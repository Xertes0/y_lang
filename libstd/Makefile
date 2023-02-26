LLC = llc
ASM = nasm
LD  = ld

LLC_FLAGS = -filetype=obj
ASM_FLAGS = -felf64
LD_FLAGS =

LL_SOURCES =

LL_OBJECTS = $(LL_SOURCES:.ll=.o)

ASM_SOURCES = \
	start.asm \
	puts.asm

ASM_OBJECTS = $(ASM_SOURCES:.asm=.o)

EXE =

all: $(ASM_OBJECTS) $(LL_OBJECTS)

%.o: %.ll
	$(LLC) $(LLC_FLAGS) $< -o $@

%.o: %.asm
	$(ASM) $(ASM_FLAGS) $< -o $@

#$(EXE): $(LL_OBJECTS) $(ASM_OBJECTS)
#	$(LD) $^ -o $@ $(LD_FLAGS)

clean:
	rm -f $(EXE) $(LL_OBJECTS) $(ASM_OBJECTS)

.PHONY: all clean
