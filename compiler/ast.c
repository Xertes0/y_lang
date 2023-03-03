#include "ast.h"

#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "token.h"

#define DEF_ARTH(TYPE) \
    assert(hist_count >= 2); \
\
    struct ast_base ast; \
    ast.type = AST_ARTH; \
    ast.arth_data.type = TYPE; \
\
    ast.arth_data.target = malloc(sizeof(struct ast_base)); \
    ast.arth_data.other  = malloc(sizeof(struct ast_base)); \
\
    *ast.arth_data.target = hist[--hist_count]; \
    *ast.arth_data.other  = hist[--hist_count]; \
\
    hist[hist_count++] = ast;

enum
{
    DECLARATION,
    DEFINITION,
};

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
                if(hist_count < 1) {
                    print_error_at(&tokens[token_i].loc, "No element in history to retrun");
                    exit(1);
                }

                base[*base_count].type = AST_RET;
                base[*base_count].ret_data.value = malloc(sizeof(struct ast_base));
                *base[*base_count].ret_data.value = hist[--hist_count];
                *base_count += 1;
            } else if(strcmp(token_str, "_end") == 0 ||
                      (strcmp(token_str, "_else") == 0 && token_i != 0)) {
                assert(hist_count == 0);
                free(hist);

                return token_i+1;
            } else if(strcmp(token_str, "_add") == 0) {
                DEF_ARTH(ARTH_ADD)
            } else if(strcmp(token_str, "_eq") == 0) {
                DEF_ARTH(ARTH_EQ)
            } else if(strcmp(token_str, "_ne") == 0) {
                DEF_ARTH(ARTH_NE)
            } else if(strcmp(token_str, "_begin") == 0) {
                if(token_i == 0) { break; }

                struct ast_if if_data;
                if_data.condition = malloc(sizeof(struct ast_base));
                *if_data.condition = hist[--hist_count];

                if_data.false_bases = NULL;
                if_data.false_count = 0;

                //token_i += 1;
                size_t used_tokens =
                    build_ast_base(tokens + token_i,
                                   token_count-token_i,
                                   &if_data.true_bases,
                                   &if_data.true_count);

                token_i += used_tokens - 1;
                print_error_at(&tokens[token_i].loc, "begin");
                printf("begin finished at %zu\n", token_i);
                if(tokens[token_i].type == TOKEN_KEYWORD &&
                   strcmp(tokens[token_i].str, "_else") == 0) {
                    //token_i += 1;
                    used_tokens =
                        build_ast_base(tokens + token_i,
                                       token_count-token_i,
                                       &if_data.false_bases,
                                       &if_data.false_count);

                    token_i += used_tokens - 1;
                    print_error_at(&tokens[token_i].loc, "else");
                    printf("else finished at %zu\n", token_i);
                }

                base[*base_count].type = AST_IF;
                base[*base_count].if_data = if_data;

                *base_count += 1;
            } else if(strcmp(token_str, "_else") == 0) {
                break;
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
                printf("%s is a function call\n", tokens[token_i].str);
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
                // Function dec/def
                int func_type = 100;
                int function_found = 0;
                for(size_t dec_i=token_i+1;dec_i<token_count;++dec_i) {
                    if(tokens[dec_i].type == TOKEN_FUNCTION) {
                        function_found = 1;
                    }
                    if(tokens[dec_i+1].type == TOKEN_KEYWORD) {
                        if(function_found == 0) {
                            func_type = DEFINITION;
                        } else {
                            func_type = DECLARATION;
                        }
                        break;
                    }
                }
                assert(func_type != 100);
                printf("%s is a function %s\n", tokens[token_i].str, func_type==DEFINITION?"definition":"declaration");

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
                        assert_token(&tokens[token_i], TOKEN_TYPE);
                        if(func_type == DEFINITION) {
                            assert_token(&tokens[token_i+1], TOKEN_VAR);
                        }

                        var_i->type = malloc(strlen(tokens[token_i].str));
                        strcpy(var_i->type, tokens[token_i].str + 1);
                        token_i += 1;

                        var_i->name = NULL;
                        if(tokens[token_i].type == TOKEN_VAR) {
                            var_i->name = malloc(strlen(tokens[token_i].str));
                            strcpy(var_i->name, tokens[token_i].str + 1);
                            token_i += 1;
                        }

                        var_i += 1;
                        data->var_count += 1;
                    }

                    // SKIP TOKEN_END
                    assert(tokens[token_i].type == TOKEN_END);
                    token_i += 1;
                }

                if(func_type == DECLARATION) {
                    // Loop will increment it back
                    token_i -= 1;
                    data->bases = NULL;
                    data->base_count = 0;
                    break;
                } else {
                    if(tokens[token_i].type != TOKEN_KEYWORD) {
                        print_error_at(&tokens[token_i].loc, "Expected '_begin' keyword");
                        assert(0);
                    }
                }

                size_t used_tokens =
                    build_ast_base(tokens + token_i,
                                   token_count-token_i,
                                   &data->bases,
                                   &data->base_count);

                // Loop will increment it back
                token_i += used_tokens - 1;
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
        {
            //destroy_ast(&hist[--hist_count], 1);
            hist_count -= 1;
            printf("Memory leak.\n");

            break;
        }
        case TOKEN_STR:
        {
            struct ast_base ast;
            ast.type = AST_STR;

            ast.str_data.value = malloc((strlen(tokens[token_i].str) + 1) * sizeof(char));
            strcpy(ast.str_data.value, tokens[token_i].str);
            hist[hist_count++] = ast;

            break;
        }
        case TOKEN_PUT:
        {
            struct ast_base ast;
            ast.type = AST_PUT;

            struct ast_base *var = &hist[--hist_count];
            assert(var->type == AST_RVAR);
            ast.put_data.var_name = var->rvar_data.name;

            struct ast_base *type = &hist[--hist_count];
            if(type->type == AST_TYPE) {
                ast.put_data.type = type->vtype_data.value;
            } else if(type->type == AST_NUMBER) {
                ast.put_data.type = malloc(32 * sizeof(char));
                sprintf(ast.put_data.type, "[%i x %s]",
                        type->number_data.value,
                        type->number_data.type);
            } else {
                assert(0);
            }

            base[*base_count] = ast;
            *base_count += 1;

            break;
        }
        case TOKEN_ASS:
        {
            struct ast_base ast;
            ast.type = AST_ASS;

            struct ast_base *var = &hist[--hist_count];
            struct ast_base *value = &hist[--hist_count];

            ast.ass_data.target = malloc(sizeof(struct ast_base));
            *ast.ass_data.target = *var;

            ast.ass_data.value = malloc(sizeof(struct ast_base));
            *ast.ass_data.value = *value;

            base[*base_count] = ast;
            *base_count += 1;

            break;
        }
        case TOKEN_AT:
        {
            struct ast_base ast;
            ast.type = AST_AT;

            struct ast_base *target = &hist[--hist_count];
            struct ast_base *value = &hist[--hist_count];

            ast.at_data.target = malloc(sizeof(struct ast_base));
            *ast.at_data.target = *target;

            ast.at_data.value = malloc(sizeof(struct ast_base));
            *ast.at_data.value = *value;

            hist[hist_count++] = ast;

            break;
        }
        case TOKEN_DEREF:
        {
            struct ast_base ast;
            ast.type = AST_DEREF;

            ast.deref_data.target = malloc(sizeof(struct ast_base));
            *ast.deref_data.target = hist[--hist_count];

            hist[hist_count++] = ast;

            break;
        }
        case TOKEN_BEGIN:
        case TOKEN_END:
        case TOKEN_SEP:
            break;
        }
    }

    assert(hist_count == 0);
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
        case AST_STR:
        {
            free(bases[base_i].str_data.value);

            break;
        }
        case AST_IF:
        {
            struct ast_if *if_data = &bases[base_i].if_data;

            destroy_ast(if_data->true_bases, if_data->true_count);
            destroy_ast(if_data->false_bases, if_data->false_count);

            destroy_ast(if_data->condition, 1);

            break;
        }
        case AST_PUT:
        {
            struct ast_put *data = &bases[base_i].put_data;
            free(data->type);

            break;
        }
        case AST_SEP:
        case AST_ASS:
        case AST_AT:
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

            printf("%s %s %s ", data->bases==NULL?"decl":"proc", data->ret_type, data->name);
            for(size_t arg_i=0;arg_i<data->var_count;++arg_i) {
                printf("%s ", data->vars[arg_i].type);

                if(data->vars[arg_i].name != NULL) {
                    printf("%s,", data->vars[arg_i].name);
                }
            }
            printf("\n");

            if(data->bases != NULL) {
                print_ast_bases(bases[base_i].proc_data.bases, bases[base_i].proc_data.base_count, indent+2);
            }

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
            char *arth_str = NULL;
            switch (bases[base_i].arth_data.type) {
            case ARTH_ADD: arth_str = "add"; break;
            case ARTH_EQ: arth_str = "eq"; break;
            case ARTH_NE: arth_str = "ne"; break;
            }
            printf("arth %s\n", arth_str);
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
        case AST_STR:
        {
            printf("str %s\n", bases[base_i].str_data.value);

            break;
        }
        case AST_IF:
        {
            struct ast_if *data = &bases[base_i].if_data;

            printf("if cond\n");
            print_ast_bases(data->condition, 1, indent+2);

            printf("if true\n");
            print_ast_bases(data->true_bases, data->true_count, indent+2);
            if(data->false_bases != NULL) {
                printf("if false\n");
                print_ast_bases(data->false_bases, data->false_count, indent+2);
            }

            break;
        }
        case AST_PUT:
        case AST_ASS:
        case AST_AT:
        case AST_SEP:
            break;
        }
    }
}

