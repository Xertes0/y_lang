#include <stddef.h>
#include <stdio.h>

#include "ast.h"

void generate_llvm(
    struct ast_base *bases,
    size_t base_count,
    FILE *stream);
