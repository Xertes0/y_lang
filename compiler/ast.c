#include "ast.h"

#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "token.h"

size_t build_ast_base(
    struct token *tokens, size_t token_count,
    struct ast_base **o_bases, size_t *base_count)
{
    *o_bases = malloc(1024 * sizeof(struct ast_base));
    struct ast_base *base = *o_bases;
    *base_count = 0;

    struct ast_base *hist = malloc(1024 * sizeof(struct ast_base));
    size_t hist_count = 0;

    for(size_t token_i=0;token_i<token_count;++token_i) {
        char *token_str = tokens[token_i].str;
        switch (tokens[token_i].type) {
        case TOKEN_KEYWORD:
        {
            if(strcmp(token_str, "_ret") == 0) {
                assert(hist_count >= 1);

                base[*base_count].type = AST_RET;
                base[*base_count].ret_data.value = malloc(sizeof(struct ast_base));
                *base[*base_count].ret_data.value = hist[--hist_count];
                *base_count += 1;
            } else if(strcmp(token_str, "_end") == 0) {
                free(hist);
                return token_i;
            } else if(strcmp(token_str, "_add") == 0) {
                assert(hist_count >= 2);

                struct ast_base ast;
                ast.type = AST_ARTH;

                ast.arth_data.target = malloc(sizeof(struct ast_base));
                ast.arth_data.other  = malloc(sizeof(struct ast_base));

                *ast.arth_data.target = hist[--hist_count];
                *ast.arth_data.other  = hist[--hist_count];

                hist[hist_count++] = ast;
            } else if(strcmp(token_str, "_begin") == 0) {
                //assert(base[(*base_count)-1].type == AST_PROC);
            } else {
                fprintf(stderr, "Unknown keyword %s\n", token_str);
                exit(1);
            }
            break;
        }
        case TOKEN_NUMBER:
        {
            struct ast_base ast;
            ast.type = AST_NUMBER;
            ast.number_data.value = atoi(tokens[token_i].str);
            ast.number_data.type = "i32";
            hist[hist_count++] = ast;

            break;
        }
        case TOKEN_FUNCTION:
        {
            if(tokens->type == TOKEN_KEYWORD) {
                // Function call
                struct ast_call data;

                size_t name_len = strlen(tokens[token_i].str);
                data.name = malloc(name_len * sizeof(char));
                strcpy(data.name, tokens[token_i].str+1);

                if(hist_count != 0 && hist[hist_count - 1].type != AST_SEP) {
                    data.args = malloc(hist_count * sizeof(struct ast_base));

                    size_t arg_count = 0;
                    while(hist_count != 0) {
                        if(hist[hist_count - 1].type == AST_SEP) { break; }
                        data.args[arg_count++] = hist[--hist_count];
                    }
                    data.arg_count = arg_count;
                } else {
                    data.args = NULL;
                    data.arg_count = 0;
                }

                if(tokens[token_i+1].type == TOKEN_DISC) {
                    token_i += 1;

                    base[*base_count].type = AST_CALL;
                    base[*base_count].call_data = data;
                    *base_count += 1;
                } else {
                    hist[hist_count].type = AST_CALL;
                    hist[hist_count].call_data = data;
                    hist_count += 1;
                }
            } else {
                // Function def
                base[*base_count].type = AST_PROC;
                struct ast_proc *data = &base[*base_count].proc_data;
                *base_count += 1;

                size_t name_len = strlen(tokens[token_i].str);
                data->name = malloc(name_len * sizeof(char));
                strcpy(data->name, tokens[token_i].str + 1);

                token_i += 1;
                if(tokens[token_i].type == TOKEN_TYPE) {
                    // Has non-void return type
                    size_t type_len = strlen(tokens[token_i].str);
                    data->ret_type = malloc(type_len * sizeof(char));
                    strcpy(data->ret_type, tokens[token_i].str+1);

                    token_i += 1;
                } else {
                    // Has void return type
                    data->ret_type = malloc(5 * sizeof(char));
                    strcpy(data->ret_type, "void");
                }

                if(tokens[token_i].type != TOKEN_BEGIN) {
                    // Function with no arguments
                    data->vars = NULL;
                    data->var_count = 0;
                } else {
                    // Function with arguments
                    token_i += 1;
                    data->vars = malloc(8 * sizeof(struct ast_var));
                    data->var_count = 0;
                    struct ast_var *var_i = data->vars;
                    while(tokens[token_i].type != TOKEN_END) {
                        assert_token(&tokens[token_i],   TOKEN_TYPE);
                        assert_token(&tokens[token_i+1], TOKEN_VAR);

                        var_i->type = malloc(strlen(tokens[token_i].str));
                        var_i->name = malloc(strlen(tokens[token_i+1].str));

                        strcpy(var_i->type, tokens[token_i].str   + 1);
                        strcpy(var_i->name, tokens[token_i+1].str + 1);

                        var_i += 1;
                        data->var_count += 1;
                        token_i += 2;
                    }

                    // TOKEN_END
                    token_i += 1;
                }

                size_t used_tokens =
                    build_ast_base(tokens + token_i,
                                   token_count-token_i,
                                   &data->bases,
                                   &data->base_count);

                token_i += used_tokens;
            }

            break;
        }
        case TOKEN_TYPE:
        {
            struct ast_base ast;
            ast.type = AST_TYPE;

            size_t value_len = strlen(tokens[token_i].str);
            ast.vtype_data.value = malloc(value_len * sizeof(char));
            strcpy(ast.vtype_data.value, tokens[token_i].str + 1);
            hist[hist_count++] = ast;

            break;
        }
        case TOKEN_VAR:
        {
            struct ast_base ast;
            ast.type = AST_RVAR;
            size_t name_len = strlen(tokens[token_i].str);
            ast.rvar_data.name = malloc(name_len * sizeof(char));
            strcpy(ast.rvar_data.name, tokens[token_i].str + 1);
            hist[hist_count++] = ast;

            break;
        }
        case TOKEN_DISC:
        case TOKEN_BEGIN:
        case TOKEN_END:
        case TOKEN_SEP:
            break;
        }
    }

    free(hist);
    return token_count;
}

void destroy_vars(struct ast_var *vars, size_t var_count)
{
    for(size_t var_i=0;var_i<var_count;++var_i) {
        free(vars[var_i].name);
        free(vars[var_i].type);
    }

    free(vars);
}

void destroy_ast(struct ast_base *bases, size_t base_count)
{
    for(size_t base_i=0;base_i<base_count;++base_i) {
        switch (bases[base_i].type) {
        case AST_PROC:
        {
            free(bases[base_i].proc_data.name);
            free(bases[base_i].proc_data.ret_type);

            destroy_ast(bases[base_i].proc_data.bases, bases[base_i].proc_data.base_count);
            destroy_vars(bases[base_i].proc_data.vars,  bases[base_i].proc_data.var_count);

            break;
        }
        case AST_RET:
        {
            destroy_ast(bases[base_i].ret_data.value, 1);

            break;
        }
        case AST_ARTH:
        {
            destroy_ast(bases[base_i].arth_data.target, 1);
            destroy_ast(bases[base_i].arth_data.other, 1);

            break;
        }
        case AST_CALL:
        {
            free(bases[base_i].call_data.name);
            destroy_ast(bases[base_i].call_data.args, bases[base_i].call_data.arg_count);

            break;
        }
        case AST_TYPE:
        {
            free(bases[base_i].vtype_data.value);

            break;
        }
        case AST_RVAR:
        {
            free(bases[base_i].rvar_data.name);

            break;
        }
        case AST_SEP:
        case AST_NUMBER:
            break;
        }
    }

    free(bases);
}

void print_ast_bases(struct ast_base *bases, size_t base_count, size_t indent)
{
    for (size_t base_i=0;base_i<base_count;++base_i) {
        for(size_t indent_i=0;indent_i<indent/2;++indent_i) {
            printf("| ");
        }

        switch (bases[base_i].type) {
        case AST_PROC:
        {
            struct ast_proc *data = &bases[base_i].proc_data;
            printf("proc %s ", data->name);
            for(size_t arg_i=0;arg_i<data->var_count;++arg_i) {
                printf("%s %s,", data->vars[arg_i].name, data->vars[arg_i].type);
            }
            printf("\n");
            print_ast_bases(bases[base_i].proc_data.bases, bases[base_i].proc_data.base_count, indent+2);

            break;
        }
        case AST_NUMBER:
        {
            printf("number %i\n", bases[base_i].number_data.value);

            break;
        }
        case AST_RET:
        {
            printf("ret\n");
            print_ast_bases(bases[base_i].ret_data.value, 1, indent+2);

            break;
        }
        case AST_ARTH:
        {
            printf("arth\n");
            print_ast_bases(bases[base_i].arth_data.target, 1, indent+2);
            print_ast_bases(bases[base_i].arth_data.other, 1, indent+2);

            break;
        }
        case AST_CALL:
        {
            printf("call %s\n", bases[base_i].call_data.name);
            print_ast_bases(bases[base_i].call_data.args, bases[base_i].call_data.arg_count, indent+2);

            break;
        }
        case AST_TYPE:
        {
            printf("type %s\n", bases[base_i].vtype_data.value);

            break;
        }
        case AST_RVAR:
        {
            printf("var %s\n", bases[base_i].rvar_data.name);

            break;
        }
        case AST_SEP:
            break;
        }
    }
}
