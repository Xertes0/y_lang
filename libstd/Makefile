PYTHON3 = python3
YCC = ../compiler/ycc.py
LLC = llc
ASM = nasm
LD  = ld

LLC_FLAGS = -filetype=obj
ASM_FLAGS = -felf64
LD_FLAGS =

YC_SOURCES = \
	put.ycu \
	strlen.ycu

YC_OBJECTS = $(YC_SOURCES:.ycu=.ll)

LL_SOURCES = \
	$(YC_OBJECTS)

LL_OBJECTS = $(LL_SOURCES:.ll=.o)

ASM_SOURCES = \
	start.asm \
	syswrite.asm \

ASM_OBJECTS = $(ASM_SOURCES:.asm=.o)

EXE =

all: $(ASM_OBJECTS) $(LL_OBJECTS) $(YC_OBJECTS)

%.o: %.ll
	$(LLC) $(LLC_FLAGS) $< -o $@

%.o: %.asm
	$(ASM) $(ASM_FLAGS) $< -o $@

%.ll: %.ycu
	$(PYTHON3) $(YCC) $< $@

#$(EXE): $(LL_OBJECTS) $(ASM_OBJECTS)
#	$(LD) $^ -o $@ $(LD_FLAGS)

clean:
	rm -f $(EXE) $(LL_OBJECTS) $(ASM_OBJECTS) $(YC_OBJECTS)

.PHONY: all clean
