PYTHON3 = python3
YCC = ../compiler/ycc.py
LLC = llc
ASM = nasm
LD  = ld

LLC_FLAGS = -filetype=obj
ASM_FLAGS = -felf64
LD_FLAGS =

STD_PATH = .

YC_SOURCES = \
	std/put.ycu \
	std/strlen.ycu

YC_OBJECTS   = $(YC_SOURCES:.ycu=.ll)
YC_SECONDARY = $(YC_SOURCES:.ycu=.ypu)

LL_SOURCES = \
	$(YC_OBJECTS)

LL_OBJECTS = $(LL_SOURCES:.ll=.o)

ASM_SOURCES = \
	std/start.asm \
	std/syswrite.asm \

ASM_OBJECTS = $(ASM_SOURCES:.asm=.o)

EXE =

all: $(ASM_OBJECTS) $(LL_OBJECTS) $(YC_OBJECTS)

std/put.ll: std/strlen.ll

%.o: %.ll
	$(LLC) $(LLC_FLAGS) $< -o $@

%.o: %.asm
	$(ASM) $(ASM_FLAGS) $< -o $@

%.ll: %.ycu
	$(PYTHON3) $(YCC) $< $(STD_PATH)

#$(EXE): $(LL_OBJECTS) $(ASM_OBJECTS)
#	$(LD) $^ -o $@ $(LD_FLAGS)

clean:
	rm -f $(EXE) $(LL_OBJECTS) $(ASM_OBJECTS) $(YC_OBJECTS) $(YC_SECONDARY)

.PHONY: all clean
.SECONDARY: $(YC_SECONDARY)
