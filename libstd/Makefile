YC = ../compiler/yc
LLC = llc
ASM = nasm
LD  = ld

LLC_FLAGS = -filetype=obj
ASM_FLAGS = -felf64
LD_FLAGS =

YC_SOURCES = \
	std/put.ycu \
	std/strlen.ycu

YC_OBJECTS   = $(YC_SOURCES:.ycu=.ll)

LL_SOURCES = \
	$(YC_OBJECTS)

LL_OBJECTS = $(LL_SOURCES:.ll=.o)

ASM_SOURCES = \
	std/start.asm \
	std/syswrite.asm \

ASM_OBJECTS = $(ASM_SOURCES:.asm=.o)

all: $(ASM_OBJECTS) $(LL_OBJECTS) $(YC_OBJECTS)

%.o: %.ll
	$(LLC) $(LLC_FLAGS) $< -o $@

%.o: %.asm
	$(ASM) $(ASM_FLAGS) $< -o $@

%.ll: %.ycu
	$(YC) $< $@

clean:
	rm -f $(LL_OBJECTS) $(ASM_OBJECTS) $(YC_OBJECTS)

.PHONY: all clean
