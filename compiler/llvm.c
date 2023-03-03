#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "third-party/sc/map/sc_map.h"

#include "ast.h"
#include "llvm.h"
#include "token.h"

enum
{
    RETURN_VALUE_DISCARD = 0,
    RETURN_VALUE_KEEP = 1,
};

struct ast_value
{
    char *rep;
    char *type;
};

struct ast_value
get_value_from_ast(
    struct ast_base *base,
    struct llvm_context *ctx,
    FILE *stream);

void generate_call(
    struct ast_call *call_data,
    struct ast_proc *proc_data,
    struct llvm_context *ctx,
    FILE *stream,
    int keep_return_value)
{
    struct ast_value values[8] = {0};
    for(size_t arg_i=0;arg_i<call_data->arg_count;++arg_i) {
        values[arg_i] =
            get_value_from_ast(
                &call_data->args[arg_i],
                ctx, stream);
    }

    if(keep_return_value == RETURN_VALUE_KEEP) {
        fprintf(stream, "%%%zu = ", ctx->var_count++);
    }

    fprintf(stream,
            "call %s @%s(",
            proc_data->ret_type,
            proc_data->name);
    for(size_t ivar_i=0;ivar_i<proc_data->var_count;++ivar_i) {
        size_t var_i = proc_data->var_count - ivar_i - 1;
        fprintf(stream, "%c%s %s",
                ivar_i>0?',':' ',
                proc_data->vars[ivar_i].type,
                values[var_i].rep);

        free(values[var_i].rep);
    }
    fprintf(stream, ")\n");
}

struct llvm_str
parse_str_literal(struct ast_str *ast)
{
    char const *str = ast->value;
    char *rep = malloc(256 * sizeof(char));
    char *out = rep;

    size_t size = 0;

    while(*str != '\0') {
        if(*str == '\\') {
            *out++ = '\\';
            char next = *++str;
            switch (next) {
            case 'n': *out++ = '0'; *out++ = 'A'; break;
            default:
                fprintf(stderr, "Unkown char after backslash");
                exit(1);
            }
        } else {
            *out++ = *str;
        }

        str += 1;
        size += 1;
    }

    *out++ = '\\';
    *out++ = '0';
    *out++ = '0';

    size += 1;

    *out++ = '\0';

    char *type = malloc(32 * sizeof(char));
    sprintf(type, "[%zu x i8]", size);

    return (struct llvm_str){ .rep = rep, .type = type, .size = size };
}

struct ast_value
get_value_from_ast(
    struct ast_base *base,
    struct llvm_context *ctx,
    FILE *stream)
{
    struct ast_value value;
    value.rep = malloc(32 * sizeof(char));

    switch (base->type) {
    case AST_NUMBER:
    {
        sprintf(value.rep, "%i", base->number_data.value);
        value.type = base->number_data.type;

        return value;
    }
    case AST_ARTH:
    {
        struct ast_arth *data = &base->arth_data;

        struct ast_value target = get_value_from_ast(data->target, ctx, stream);
        struct ast_value other  = get_value_from_ast(data->other, ctx, stream);

        assert(strcmp(target.type, other.type) == 0);

        char *op_type = NULL;
        switch (data->type) {
        case ARTH_ADD: op_type = "add"; break;
        case ARTH_EQ: op_type = "icmp eq"; break;
        case ARTH_NE: op_type = "icmp ne"; break;
        }

        fprintf(
            stream,
            "%%%zu = %s %s %s, %s\n",
            ctx->var_count,
            op_type,
            target.type, target.rep,
            other.rep);

        free(target.rep);
        free(other.rep);

        sprintf(value.rep, "%%%zu", ctx->var_count);
        if(data->type == ARTH_EQ) {
            value.type = "i1";
        } else {
            value.type = target.type;
        }

        ctx->var_count += 1;
        return value;
    }
    case AST_TYPE:
    {
        value.type = base->vtype_data.value;
        strcpy(value.rep, "--!SHOULDNOTBEHERE!--");

        return value;
    }
    case AST_RVAR:
    {
        struct ast_var *var =
            &((struct llvm_iden*)sc_map_get_sv(&ctx->indentifier_map, base->rvar_data.name))->var_data;

        if(!sc_map_found(&ctx->indentifier_map)) {
            fprintf(stderr, "Indentifier not found %s\n", base->rvar_data.name);
            exit(1);
        }

        value.type = var->type;
        strcpy(value.rep, var->rep);

        return value;
    }
    case AST_CALL:
    {
        struct ast_call *call_data = &base->call_data;
        struct ast_proc *proc_data =
            &((struct llvm_iden*)
            sc_map_get_sv(
                &ctx->indentifier_map,
                call_data->name))->proc_data;

        if(!sc_map_found(&ctx->indentifier_map)) {
            fprintf(stderr,
                    "Unknown indentifier %s\n",
                    call_data->name);
            exit(1);
        }

        generate_call(call_data, proc_data, ctx, stream, RETURN_VALUE_KEEP);

        sprintf(value.rep, "%%%zu", ctx->var_count-1);
        value.type = proc_data->ret_type;

        return value;
    }
    case AST_STR:
    {
        struct llvm_str str = parse_str_literal(&base->str_data);
        ctx->strings[ctx->string_count] = str;

        fprintf(
            stream,
            "%%%zu = getelementptr %s,%s* @.str%zu, i64 0, i64 0\n",
            ctx->var_count,
            str.type, str.type,
            ctx->string_count);

        sprintf(value.rep, "%%%zu", ctx->var_count);
        value.type = str.type;

        ctx->string_count += 1;
        ctx->var_count += 1;

        return value;
    }
    case AST_IF:
    case AST_ASS:
    case AST_PUT:
    case AST_AT:
    case AST_PROC:
    case AST_SEP:
    case AST_RET:
        assert(0);
    }

    // To future self:
    // if you got printed you propably 'break;'ed the switch
    // instead of 'return value;'
    fprintf(stderr, "Tried to get %i\n", base->type);
    assert(0);
}

void generate_llvm(
    struct ast_base *bases,
    size_t base_count,
    struct llvm_context *ctx,
    FILE *stream)
{
    for(size_t base_i=0;base_i<base_count;++base_i) {
        switch (bases[base_i].type) {
        case AST_PROC:
        {
            struct ast_proc *data = &bases[base_i].proc_data;

            struct llvm_iden *proc_map = malloc(sizeof(struct llvm_iden));
            proc_map->type = AST_PROC;
            proc_map->proc_data = *data;
            sc_map_put_sv(
                &ctx->indentifier_map,
                data->name, proc_map);

            int definition = data->bases != NULL;

            fprintf(stream, "%s %s @%s(", definition?"define":"declare", data->ret_type, data->name);

            ctx->var_count = 0;
            for(size_t var_i=0;var_i<data->var_count;++var_i) {
                fprintf(stream, "%c%s ", var_i>0?',':' ', data->vars[var_i].type);

                if(definition) {
                    fprintf(stream, "%%%zu", ctx->var_count++);

                    struct llvm_iden *var_map = malloc(sizeof(struct llvm_iden));
                    var_map->type = AST_RVAR;
                    var_map->var_data.type = data->vars[var_i].type;
                    //var_map->var_data.name = malloc(32 * sizeof(char));
                    //sprintf(var_map->var_data.name, "%zu", ctx->var_count-1);
                    var_map->var_data.name = data->vars[var_i].name;
                    var_map->var_data.rep = malloc(32 * sizeof(char));
                    sprintf(var_map->var_data.rep, "%%%zu", ctx->var_count-1);

                    sc_map_put_sv(
                            &ctx->indentifier_map,
                            data->vars[var_i].name, var_map);
                }
            }

            fprintf(stream, ")\n");
            if(definition) {
                fprintf(stream, "{\n");
                ctx->var_count += 1;
                generate_llvm(data->bases, data->base_count, ctx, stream);
                fprintf(stream, "}\n");

                for(size_t var_i=0;var_i<data->var_count;++var_i) {
                    struct llvm_iden *iden =
                        sc_map_del_sv(
                            &ctx->indentifier_map,
                            data->vars[var_i].name);

                    free(iden);
                }
            }

            break;
        }
        case AST_RET:
        {
            struct ast_value value = get_value_from_ast(
                bases[base_i].ret_data.value,
                ctx, stream);
            if(strcmp(value.type, "void") == 0) {
                fprintf(stream, "ret %s\n", value.type);
            } else {
                fprintf(stream, "ret %s %s\n", value.type, value.rep);
            }

            free(value.rep);

            break;
        }
        case AST_CALL:
        {
            struct ast_call *call_data = &bases[base_i].call_data;
            struct ast_proc *proc_data =
                &((struct llvm_iden*)
                sc_map_get_sv(
                    &ctx->indentifier_map,
                    call_data->name))->proc_data;

            if(!sc_map_found(&ctx->indentifier_map)) {
                fprintf(stderr,
                        "Unknown indentifier %s\n",
                        call_data->name);
                exit(1);
            }
            generate_call(call_data, proc_data, ctx, stream, RETURN_VALUE_DISCARD);

            break;
        }
        case AST_IF:
        {
            struct ast_if *if_data = &bases[base_i].if_data;
            struct ast_value cond = get_value_from_ast(if_data->condition, ctx, stream);

            size_t curr_label = ctx->label_count++;
            fprintf(
                stream,
                "br i1 %s, label %%IFTRUE%zu, label %%IFFALSE%zu\n",
                cond.rep, curr_label, curr_label);
            fprintf(stream, "IFTRUE%zu:\n", curr_label);

            generate_llvm(
                if_data->true_bases,
                if_data->true_count,
                ctx, stream);

            fprintf(stream, "br label %%IF%s%zu\n",
                    if_data->false_bases==NULL?"FALSE":"END",
                    curr_label);
            fprintf(stream, "IFFALSE%zu:\n", curr_label);

            if(if_data->false_bases != NULL) {
                generate_llvm(
                    if_data->false_bases,
                    if_data->false_count,
                    ctx, stream);
                fprintf(stream, "br label %%IFEND%zu\n", curr_label);
                fprintf(stream, "IFEND%zu:\n", curr_label);
            }

            free(cond.rep);

            break;
        }
        case AST_PUT:
        {
            struct ast_put *put_data = &bases[base_i].put_data;
            fprintf(stream, "%%%s = alloca %s\n",
                    put_data->var_name, put_data->type);

            struct llvm_iden *var_map = malloc(sizeof(struct llvm_iden));
            var_map->type = AST_RVAR;
            var_map->var_data.name = put_data->var_name;
            var_map->var_data.type = put_data->type;
            var_map->var_data.rep = malloc(32 * sizeof(char));
            sprintf(var_map->var_data.rep, "%%%s", put_data->var_name);
            sc_map_put_sv(
                &ctx->indentifier_map,
                put_data->var_name, var_map);

            break;
        }
        case AST_ASS:
        case AST_AT:
        case AST_STR:
        case AST_RVAR:
        case AST_ARTH:
        case AST_SEP:
        case AST_TYPE:
        case AST_NUMBER:
            assert(0);
        }
    }
}

void generate_llvm_string_literals(
    struct llvm_context *ctx,
    FILE *stream)
{
    for(size_t str_i=0;str_i<ctx->string_count;++str_i) {
        fprintf(
            stream,
            "@.str%zu = private unnamed_addr constant %s c\"%s\"\n",
            str_i,
            ctx->strings[str_i].type,
            ctx->strings[str_i].rep);
    }
}

struct llvm_context make_llvm_context()
{
    struct llvm_context ctx;
    sc_map_init_sv(&ctx.indentifier_map, 0, 0);
    ctx.var_count = 1;
    ctx.label_count = 1;
    ctx.strings = malloc(64 * sizeof(struct llvm_str));
    ctx.string_count = 0;

    return ctx;
}

void destroy_llvm_context(struct llvm_context *ctx)
{
    struct llvm_iden *iden;
    sc_map_foreach_value(&ctx->indentifier_map, iden) {
        if(iden->type == AST_RVAR) {
            free(iden->var_data.rep);
        }
        free(iden);
    }

    for(size_t str_i=0;str_i<ctx->string_count;++str_i) {
        free(ctx->strings[str_i].rep);
        free(ctx->strings[str_i].type);
    }
    free(ctx->strings);
}

