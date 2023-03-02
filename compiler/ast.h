#pragma once

#include <stddef.h>

#include "token.h"

enum ast_type
{
    AST_PROC,
    AST_NUMBER,
    AST_RET,
    AST_ARTH,
    AST_CALL,
};

struct ast_number
{
    int value;
    char *type;
};

struct ast_ret
{
    struct ast_base *value;
};

struct ast_arth
{
    struct ast_base *target;
    struct ast_base *other;
};

struct ast_var
{
    char *name;
    char *type;
};

struct ast_proc
{
    char *name;
    char *ret_type;

    struct ast_var  *vars;
    struct ast_base *bases;

    size_t var_count;
    size_t base_count;
};

struct ast_call
{
    char *name;
};

struct ast_base
{
    enum ast_type type;
    union
    {
        struct ast_proc   proc_data;
        struct ast_number number_data;
        struct ast_ret    ret_data;
        struct ast_arth   arth_data;
        struct ast_call   call_data;
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

