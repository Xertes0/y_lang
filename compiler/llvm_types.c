#include "llvm_types.h"

#include <assert.h>

void print_type_rep(FILE *stream, struct type_type *type)
{
	switch (type->type) {
	case TYPE_ARRAY:
	{
		fprintf(stream, "[%zu x ", type->array_data.size);
		print_type_rep(stream, type->array_data.holds);
		fprintf(stream, "]");
		break;
	}
	case TYPE_BOOL:
	{
		fprintf(stream, "i1");
		break;
	}
	case TYPE_SIGNED:
	{
		fprintf(stream, "i%u", type->number_data.bit_count);
		break;
	}
	case TYPE_UNSIGNED:
	{
		fprintf(stream, "u%u", type->number_data.bit_count);
		break;
	}
	case TYPE_FLOATING:
	{
		if(type->number_data.bit_count == 32) {
			fprintf(stream, "float");
		} else if(type->number_data.bit_count == 64) {
			fprintf(stream, "double");
		}
		break;
	}
	case TYPE_VOID:
	{
		fprintf(stream, "void");
		break;
	}
        break;
	}

	for(unsigned i=0;i<type->ptr_count;++i) {
		fprintf(stream, "*");
	}
}
