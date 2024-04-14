YC = ../compiler/yc
LLC = llc
ASM = fasm
LD  = ld

LLC_FLAGS = -filetype=obj

YC_SOURCES = $(shell find -name '*.ycu')
YC_OBJECTS = $(YC_SOURCES:.ycu=.ll)

LL_SOURCES = $(YC_OBJECTS)
LL_OBJECTS = $(LL_SOURCES:.ll=.o)

ASM_SOURCES = $(shell find -name '*.asm')
ASM_OBJECTS = $(ASM_SOURCES:.asm=.o)

.PHONY: all
all: $(ASM_OBJECTS) $(LL_OBJECTS) $(YC_OBJECTS)

%.o: %.ll
	$(LLC) $(LLC_FLAGS) $< -o $@

%.o: %.asm
	$(ASM) $< $@

%.ll: %.ycu
	$(YC) $< $@

.PHONY: clean
clean:
	rm -f $(LL_OBJECTS) $(ASM_OBJECTS) $(YC_OBJECTS)
