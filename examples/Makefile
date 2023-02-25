
LLC = llc
ASM = nasm
LD  = ld

LLC_FLAGS = -filetype=obj
ASM_FLAGS = -felf64
LD_FLAGS = ../libstd/puts.o

PYTHON3 = python3
YCC = ../compiler/ycc.py

EXAMPLES = return.bin hello_world.bin

all: $(EXAMPLES)

start.o: ../start.asm
	$(ASM) $(ASM_FLAGS) $< -o $@

%.ll: %.ycu
	$(PYTHON3) $(YCC) $< $@

%.o: %.ll
	$(LLC) $(LLC_FLAGS) $< -o $@

%.bin: %.o start.o
	$(LD) $< start.o $(LD_FLAGS) -o $@

clean:
	rm -f $(EXAMPLES) ./*.o ./*.ll

.PHONY: all clean
.SECONDARY: $(EXAMPLES:.bin=.ll) $(EXAMPLES:.bin=.o)

