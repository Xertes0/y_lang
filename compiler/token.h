#pragma once

#include <stdio.h>

enum token_type
{
    TOKEN_KEYWORD,
    TOKEN_INDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_TYPE,
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

