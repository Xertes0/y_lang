#pragma once

#include <stddef.h>

enum type_type_type
{
	TYPE_ARRAY,
	TYPE_BOOL,
	TYPE_FLOATING,
	TYPE_SIGNED,
	TYPE_UNSIGNED,
	//TYPE_USERDEF,
	TYPE_VOID,
};

struct type_number
{
	unsigned bit_count;
};

struct type_array
{
	struct type_type *holds;
	size_t size;
};

struct type_type
{
	enum type_type_type type;
	unsigned ptr_count;

	union
	{
		struct type_number number_data;
		struct type_array  array_data;
	};
};

struct type_type
parse_type(char const *str);

struct type_type
copy_type(const struct type_type *type);

void destroy_type(struct type_type *type);

void print_type(struct type_type *type);

