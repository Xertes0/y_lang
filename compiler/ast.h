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
    AST_SEP,
    AST_TYPE,
    AST_RVAR,
    AST_STR,
    AST_IF,
    AST_ASS,
    AST_PUT,
    AST_AT,
    AST_DEREF,
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

enum arth_type
{
    ARTH_ADD,
    ARTH_EQ,
    ARTH_NE,
};

struct ast_arth
{
    enum arth_type type;

    struct ast_base *target;
    struct ast_base *other;
};

struct ast_var
{
    char *name;
    char *type;
    char *rep;
};

struct ast_proc
{
    char *name;
    char *ret_type;

    struct ast_var  *vars;
    size_t var_count;

    struct ast_base *bases;
    size_t base_count;
};

struct ast_call
{
    char *name;

    struct ast_base  *args;
    size_t arg_count;
};

struct ast_vtype
{
    char *value;
};

struct ast_rvar
{
    char *name;
};

struct ast_str
{
    char *value;
};

struct ast_if
{
    struct ast_base *condition;

    struct ast_base *true_bases;
    size_t true_count;

    struct ast_base *false_bases;
    size_t false_count;
};

struct ast_put
{
    char *var_name;
    char *type;
};

struct ast_ass
{
    struct ast_base *value;
    struct ast_base *target;
};

struct ast_at
{
    struct ast_base *value;
    struct ast_base *target;
};

struct ast_deref
{
    struct ast_base *target;
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
        struct ast_vtype  vtype_data;
        struct ast_rvar   rvar_data;
        struct ast_str    str_data;
        struct ast_if     if_data;
        struct ast_put    put_data;
        struct ast_ass    ass_data;
        struct ast_at     at_data;
        struct ast_deref  deref_data;
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

