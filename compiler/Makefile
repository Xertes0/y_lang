CC := cc
LLC := llc
LD := ld

#SANITIZERS:= -fsanitize=address,leak,undefined
SANITIZERS :=

CFLAGS := \
	-MD -MP \
	$(SANITIZERS) \
	-ggdb \
	-Wall -Wextra -Wshadow -Wconversion -Wpedantic

LDFLAGS := \
	$(SANITIZERS)

EXE := yc

THRID_SOURCES := \
	./third-party/sc/map/sc_map.c

SOURCES := $(shell find -not \( -path './third-party' -prune \) -name '*.c')
OBJECTS := $(SOURCES:.c=.o) $(THRID_SOURCES:.c=.o)
DEPENDS := $(SOURCES:.c=.d)

.PHONY: all
all: $(EXE)

.PHONY: run
run: $(EXE) text.ycu
	./$(EXE) ./text.ycu ./text.ll
	$(LLC) -filetype=obj ./text.ll -o text.o
	$(LD) $(shell find ../libstd/std -name '*.o') ./text.o -o text
	./text

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(EXE): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

.PHONY: clean
clean:
	rm -Rf $(EXE) $(OBJECTS) $(DEPENDS)

-include $(DEPENDS)
