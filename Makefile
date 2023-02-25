LLC = llc
ASM = nasm
LD  = ld

LLC_FLAGS = -filetype=obj
ASM_FLAGS = -felf64
LD_FLAGS =

LL_SOURCES = \
	hello.ll

LL_OBJECTS = $(LL_SOURCES:.ll=.o)

ASM_SOURCES = \
	start.asm \
	libstd/puts.asm

ASM_OBJECTS = $(ASM_SOURCES:.asm=.o)

EXE = hello

all: $(EXE)

%.o: %.ll
	$(LLC) $(LLC_FLAGS) $< -o $@

%.o: %.asm
	$(ASM) $(ASM_FLAGS) $< -o $@

$(EXE): $(LL_OBJECTS) $(ASM_OBJECTS)
	$(LD) $^ -o $@ $(LD_FLAGS)

clean:
	rm -f $(EXE) $(LL_OBJECTS) $(ASM_OBJECTS)

.PHONY: all clean
