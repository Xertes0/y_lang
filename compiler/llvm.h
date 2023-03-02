#pragma once

#include <stddef.h>
#include <stdio.h>

#include "third-party/sc/map/sc_map.h"

#include "ast.h"

struct llvm_context
{
    size_t var_count;
    struct sc_map_sv indentifier_map;
};

struct llvm_iden
{
    enum ast_type type;
    union
    {
        struct ast_proc proc_data;
        struct ast_var  var_data;
    };
};

void generate_llvm(
    struct ast_base *bases,
    size_t base_count,
    struct llvm_context *ctx,
    FILE *stream);
