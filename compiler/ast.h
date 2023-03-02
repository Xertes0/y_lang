#pragma once

#include <stddef.h>

#include "token.h"

enum ast_type
{
    AST_PROC,
    AST_NUMBER,
    AST_RET,
};

struct ast_proc
{
    char *name;

    struct ast_base *bases;
    size_t base_count;
};

struct ast_number
{
    int value;
};

struct ast_ret
{
    struct ast_base *value;
};

struct ast_base
{
    enum ast_type type;
    union
    {
        struct ast_proc    proc_data;
        struct ast_number  number_data;
        struct ast_ret     ret_data;
    };
};

/*
 * @return size_t Amount of tokens parsed.
 */
size_t build_ast_base(
    struct token *tokens, size_t token_count,
    struct ast_base **o_bases, size_t *base_count);

void destroy_ast(struct ast_base *bases, size_t base_count);

void print_ast_bases(struct ast_base *bases, size_t base_count, size_t indent);

