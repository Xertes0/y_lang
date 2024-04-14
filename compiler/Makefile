CC=cc

#SANITIZERS= -fsanitize=address,leak,undefined
SANITIZERS=

CFLAGS= \
	-MD -MP \
	$(SANITIZERS) \
	-ggdb \
	-Wall -Wextra -Wshadow -Wconversion -Wpedantic

LDFLAGS= \
	$(SANITIZERS)

EXE=yc

THRID_SOURCES= \
	./third-party/sc/map/sc_map.c

SOURCES=$(shell find -not \( -path "./third-party" -prune \) -name '*.c')
OBJECTS=$(SOURCES:.c=.o) $(THRID_SOURCES:.c=.o)
DEPENDS=$(SOURCES:.c=.d)

.PHONY: all
all: $(EXE)

.PHONY: run
run: $(EXE)
	./$(EXE) ./text.ycu ./text.ll

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(EXE): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

.PHONY: clean
clean:
	rm -Rf $(EXE) $(OBJECTS) $(DEPENDS)

-include $(DEPENDS)
