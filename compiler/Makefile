CC=cc

CC_FLAGS= \
	-fsanitize=address,leak,undefined \
	-ggdb \
	-Wall -Wextra -Wshadow -Wconversion -Wpedantic

LD_FLAGS= \
	-fsanitize=address,leak,undefined

EXE=yc

THRID_SOURCES= \
	./third-party/sc/map/sc_map.c

SOURCES= \
	ast.c token.c llvm.c yc.c

OBJECTS=$(SOURCES:.c=.o) $(THRID_SOURCES:.c=.o)

all: yc

%.o: %.c
	$(CC) $(CC_FLAGS) -c $< -o $@

$(EXE): $(OBJECTS)
	$(CC) $(LD_FLAGS) $^ -o $@

clean:
	rm $(EXE) $(OBJECTS)

.PHONY: all clean
