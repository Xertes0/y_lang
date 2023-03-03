#pragma once

#include <stddef.h>
#include <stdio.h>

#include "third-party/sc/map/sc_map.h"

#include "ast.h"

struct llvm_str
{
    char *rep;
    char *type;
    size_t size;
};

struct llvm_context
{
    size_t var_count;
    size_t label_count;
    struct sc_map_sv indentifier_map;

    struct llvm_str *strings;
    size_t string_count;
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

struct llvm_context make_llvm_context();
void destroy_llvm_context(struct llvm_context *ctx);

void generate_llvm(
    struct ast_base *bases,
    size_t base_count,
    struct llvm_context *ctx,
    FILE *stream);

void generate_llvm_string_literals(
    struct llvm_context *ctx,
    FILE *stream);

