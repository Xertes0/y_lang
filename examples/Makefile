
LLC = llc
ASM = nasm
LD  = ld

LLC_FLAGS = -filetype=obj
LD_FLAGS = ../libstd/*.o

PYTHON3 = python3
YCC = ../compiler/ycc.py

EXAMPLES = \
	return.bin \
	hello_world.bin \
	array.bin \
	variable.bin \
	casting.bin \
	if.bin \
	put_i32.bin

all: $(EXAMPLES)

%.ll: %.ycu
	$(PYTHON3) $(YCC) $< $@

%.o: %.ll
	$(LLC) $(LLC_FLAGS) $< -o $@

%.bin: %.o
	$(LD) $< $(LD_FLAGS) -o $@

clean:
	rm -f $(EXAMPLES) ./*.o ./*.ll

.PHONY: all clean
.SECONDARY: $(EXAMPLES:.bin=.ll) $(EXAMPLES:.bin=.o)

