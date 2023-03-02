#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "third-party/sc/map/sc_map.h"

#include "llvm.h"

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

        fprintf(
            stream,
            "%%%zu = add %s %s, %s\n",
            ctx->var_count, target.type, target.rep, other.rep);

        free(target.rep);
        free(other.rep);

        sprintf(value.rep, "%%%zu", ctx->var_count);
        value.type = target.type;

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
        sprintf(value.rep, "%%%zu", var->rep);

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
    case AST_PROC:
    case AST_SEP:
    case AST_RET:
        assert(0);
    }

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

            fprintf(stream, "define %s @%s(", data->ret_type, data->name);

            ctx->var_count = 0;
            for(size_t var_i=0;var_i<data->var_count;++var_i) {
                fprintf(stream,"%c%s %%%zu", var_i>0?',':' ', data->vars[var_i].type, ctx->var_count++);

                struct llvm_iden *var_map = malloc(sizeof(struct llvm_iden));
                var_map->type = AST_RVAR;
                var_map->var_data.type = data->vars[var_i].type;
                //var_map->var_data.name = malloc(32 * sizeof(char));
                //sprintf(var_map->var_data.name, "%zu", ctx->var_count-1);
                var_map->var_data.name = data->vars[var_i].name;
                var_map->var_data.rep = ctx->var_count-1;

                sc_map_put_sv(
                        &ctx->indentifier_map,
                        data->vars[var_i].name, var_map);
            }

            fprintf(stream, ")\n{\n");
            ctx->var_count += 1;
            generate_llvm(data->bases, data->base_count, ctx, stream);
            fprintf(stream, "}\n");

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
        case AST_RVAR:
        case AST_ARTH:
        case AST_SEP:
        case AST_TYPE:
        case AST_NUMBER:
            assert(0);
        }
    }
}
