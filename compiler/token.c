#include "token.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>

void assert_token(struct token *token, enum token_type type)
{
    if(token->type == type) return;

    fprintf(stderr,
            "ERROR: Expected token of type %i, got %i\n",
            type, token->type);
    exit(1);
}

enum token_type
parse_token_type(const char *str)
{
    if(*str == '_') {
        return TOKEN_KEYWORD;
    } else if(*str == ':') {
        return TOKEN_TYPE;
    } else if(*str == '$') {
        return TOKEN_VAR;
    } else if(*str == '@') {
        return TOKEN_FUNCTION;
    } else if(*str == '[') {
        return TOKEN_BEGIN;
    } else if(*str == ']') {
        return TOKEN_END;
    } else if(*str == '|') {
        return TOKEN_SEP;
    } else if(*str == '!') {
        return TOKEN_DISC;
    } else if(isdigit(*str)) {
        // TODO type
        return TOKEN_NUMBER;
    } else {
        fprintf(stderr, "Unknown token type %s\n", str);
        exit(1);
    }
}

void tokenise(
    FILE *stream,
    struct token **tokens,
    size_t *token_count)
{
    *tokens = malloc(1024 * sizeof(struct token));

    struct token *token_i = *tokens;
    char *token_str_i = NULL;

    struct location curr_loc;
    curr_loc.line = 1;
    curr_loc.at   = 1;

    bool parsing_token = false;

    while(!feof(stream)) {
        char c = (char)getc(stream);

        // Parsing
        if(!isgraph(c) && parsing_token) {
            // Finished reading the token
            *token_str_i++ = '\0';
            token_i->type = parse_token_type(token_i->str);
            token_i->loc = curr_loc;
            token_i += 1;
            parsing_token = false;
        } else if(isgraph(c)) {
            if(!parsing_token) {
                token_i->str = malloc(64 * sizeof(char));
                token_str_i = token_i->str;
            }
            parsing_token = true;
            *token_str_i++ = c;
        }

        // Location
        if(c == '\n') {
            curr_loc.line += 1;
            curr_loc.at = 1;
        } else {
            curr_loc.at += 1;
        }
    }

    *token_count = (size_t)(token_i - *tokens);
}

void destroy_tokens(struct token *tokens, size_t token_count)
{
    for(size_t token_i=0;token_i<token_count;++token_i) {
        free(tokens[token_i].str);
    }
    free(tokens);
}

void print_error_at(struct location *loc, const char *msg)
{
    fprintf(stderr, "ERROR at %zu,%zu: %s\n", loc->line, loc->at, msg);
}

