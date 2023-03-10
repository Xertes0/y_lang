CC=cc

#SANITIZERS= -fsanitize=address,leak,undefined
SANITIZERS=

CC_FLAGS= \
	$(SANITIZERS) \
	-ggdb \
	-Wall -Wextra -Wshadow -Wconversion -Wpedantic

LD_FLAGS= \
	$(SANITIZERS)

EXE=yc

THRID_SOURCES= \
	./third-party/sc/map/sc_map.c

SOURCES= \
	ast.c token.c types.c llvm.c llvm_types.c yc.c

OBJECTS=$(SOURCES:.c=.o) $(THRID_SOURCES:.c=.o)

all: yc

%.o: %.c
	$(CC) $(CC_FLAGS) -c $< -o $@

$(EXE): $(OBJECTS)
	$(CC) $(LD_FLAGS) $^ -o $@

clean:
	rm $(EXE) $(OBJECTS)

.PHONY: all clean
