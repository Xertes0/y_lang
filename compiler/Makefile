CC=cc

CC_FLAGS = -g
LD_FLAGS =

SOURCES = \
	main.c

OBJECTS = $(SOURCES:.c=.o)

EXE = ycc

all: $(EXE)

%.o: %.c
	$(CC) $(CC_FLAGS) -c $< -o $@

$(EXE): $(OBJECTS)
	$(CC) $^ $(LD_FLAGS) -o $@

clean:
	rm -f $(EXE) $(OBJECTS)

.PHONY: all clean

