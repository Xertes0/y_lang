#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"

/**
 * @brief Parse bit_count and ptr_count of primitive type str.
 *
 * @param str String to the whole type eg. `i32`, `u32`.
 */
void parse_number_ptr_count(
    struct type_type *type,
    char const* str)
{
    // TODO Validate input.
    type->ptr_count = 0;

    while(*str != '\0'){
        if(*str == '*') {
            type->ptr_count += 1;
        }
        str += 1;
    }
}

struct type_type
parse_type(char const *str)
{
    struct type_type type;

    if(*str == 's') {
        type.type = TYPE_SIGNED;
        type.number_data.bit_count = (unsigned)atoi(str+1);
        parse_number_ptr_count(&type, str);
    } else if(*str == 'u') {
        type.type = TYPE_UNSIGNED;
        type.number_data.bit_count = (unsigned)atoi(str+1);
        parse_number_ptr_count(&type, str);
    } else if(*str == 'f') {
        type.type = TYPE_FLOATING;
        type.number_data.bit_count = (unsigned)atoi(str+1);
        parse_number_ptr_count(&type, str);
    } else if(strcmp(str, "bool") == 0) {
        type.type = TYPE_BOOL;
        parse_number_ptr_count(&type, str);
    } else if(strcmp(str, "void") == 0) {
        type.type = TYPE_VOID;
        type.ptr_count = 0;
    } else if(isdigit(*str) != 0) {
        type.type = TYPE_ARRAY;
        type.ptr_count = 0;
        char const* colon = strchr(str, ':');
        if(colon == NULL) {
            assert(0); // not an array
        } else {
            type.array_data.size = (size_t)atoi(str);
            type.array_data.holds = malloc(sizeof(struct type_type));
            *type.array_data.holds = parse_type(colon+1);
        }
    } else {
        // TODO Meaningful message.
        fprintf(stderr, "Could not parse %s as a type\n", str);
        assert(0);
    }

    return type;
}

struct type_type
copy_type(const struct type_type *type)
{
    struct type_type copy = *type;
    if(copy.type == TYPE_ARRAY) {
        copy.array_data.holds = malloc(sizeof(struct type_type));
        *copy.array_data.holds = copy_type(type->array_data.holds);
    }

    return copy;
}

void destroy_type(struct type_type *type)
{
    if(type == NULL) { return; }
    if(type->type == TYPE_ARRAY) {
        if(type->array_data.holds == NULL) { return; }
        destroy_type(type->array_data.holds);
        free(type->array_data.holds);
        type->array_data.holds = NULL;
    }
}

void print_type(struct type_type *type)
{
    switch (type->type) {
    case TYPE_ARRAY:
    {
        printf("%zu:", type->array_data.size);
        print_type(type->array_data.holds);
        break;
    }
    case TYPE_BOOL:
    {
        printf("bool");
        break;
    }
    case TYPE_SIGNED:
    {
        printf("s%u", type->number_data.bit_count);
        break;
    }
    case TYPE_UNSIGNED:
    {
        printf("u%u", type->number_data.bit_count);
        break;
    }
    case TYPE_FLOATING:
    {
        printf("f%u", type->number_data.bit_count);
        break;
    }
    case TYPE_VOID:
    {
        printf("void");
        break;
    }
        break;
    }

    for(unsigned i=0;i<type->ptr_count;++i) {
        printf("*");
    }
}

