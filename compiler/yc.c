#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "token.h"

int main(int argc, char *argv[])
{
    //printf("Usage: \n");
    //printf("  yc <input_file> <output_file>\n");

    FILE *src = fopen(argv[1], "r");

    struct token *tokens;
    size_t token_count;
    tokenise(src, &tokens, &token_count);

    fclose(src);

    for(size_t token_i=0;token_i<token_count;++token_i) {
        printf("%zu: %i - %s\n", token_i, tokens[token_i].type, tokens[token_i].str);
    }

    struct ast_base *bases;
    size_t base_count;
    build_ast_base(tokens, token_count, &bases, &base_count);

    print_ast_bases(bases, base_count, 0);

    FILE *ll_out = fopen(argv[2], "w");

    generate_llvm(bases, base_count, ll_out);

    fclose(ll_out);

    destroy_ast(bases, base_count);
    destroy_tokens(tokens, token_count);

    return 0;
}
