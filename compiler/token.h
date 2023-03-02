#pragma once

#include <stdio.h>

enum token_type
{
    TOKEN_KEYWORD,
    TOKEN_FUNCTION,
    TOKEN_NUMBER,
    TOKEN_TYPE,
    TOKEN_VAR,
    TOKEN_BEGIN,
    TOKEN_END,
    TOKEN_SEP,
    TOKEN_DISC,
};

struct token
{
    enum token_type type;
    char *str;
};

void assert_token(struct token *token, enum token_type type);

enum token_type
parse_token_type(const char *str);

void tokenise(
    FILE *stream,
    struct token **tokens,
    size_t *token_count);

void destroy_tokens(struct token *tokens, size_t token_count);

