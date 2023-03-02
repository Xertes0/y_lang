#include <assert.h>
#include <stdlib.h>

#include "llvm.h"

struct llvm_context
{
    size_t var_count;
};

char *
get_value_from_ast(
    struct ast_base *base,
    struct llvm_context *ctx,
    FILE *stream)
{
    switch (base->type) {
    case AST_NUMBER:
    {
        char *value = malloc(32 * sizeof(char));
        sprintf(value, "%i", base->number_data.value);

        return value;
    }
    case AST_ARTH:
    {
        struct ast_arth *data = &base->arth_data;

        // TODO assert types
        char *target = get_value_from_ast(data->target, ctx, stream);
        char *other  = get_value_from_ast(data->other, ctx, stream);

        fprintf(
            stream,
            "%%%zu = add i32 %s, %s\n",
            ctx->var_count, target, other);

        free(target);
        // Reuse 'outher's memory

        sprintf(other, "%%%zu", ctx->var_count);

        ctx->var_count += 1;
        return other;
    }
    case AST_PROC:
    case AST_RET:
        assert(0);
    }

    return "";
}

void generate_llvm(
    struct ast_base *bases,
    size_t base_count,
    FILE *stream)
{
    struct llvm_context ctx;
    ctx.var_count = 1;

    for(size_t base_i=0;base_i<base_count;++base_i) {
        switch (bases[base_i].type) {
        case AST_PROC:
        {
            struct ast_proc *data = &bases[base_i].proc_data;
            fprintf(stream, "define i32 @%s()\n{\n", data->name);
            generate_llvm(data->bases, data->base_count, stream);
            fprintf(stream, "}\n");

            break;
        }
        case AST_RET:
        {
            char *value = get_value_from_ast(
                bases[base_i].ret_data.value,
                &ctx, stream);
            fprintf(stream, "ret i32 %s\n", value);

            free(value);

            break;
        }
        case AST_CALL:
        {
            break;
        }
        case AST_ARTH:
        case AST_NUMBER:
            assert(0);
        }
    }
}
