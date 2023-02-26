
LLC = llc
ASM = nasm
LD  = ld

LLC_FLAGS = -filetype=obj
LD_FLAGS = \
	../libstd/std/start.o \
	../libstd/std/put.o \
	../libstd/std/strlen.o \
	../libstd/std/syswrite.o

PYTHON3 = python3
YCC = ../compiler/ycc.py

STD_PATH = ../libstd/

EXAMPLES = \
	return.bin \
	hello_world.bin \
	array.bin \
	variable.bin \
	casting.bin \
	if.bin \
	put_i32.bin \
	macros.bin

all: $(EXAMPLES)

%.ll: %.ycu
	$(PYTHON3) $(YCC) $< $(STD_PATH)

%.o: %.ll
	$(LLC) $(LLC_FLAGS) $< -o $@

%.bin: %.o
	$(LD) $< $(LD_FLAGS) -o $@

clean:
	rm -f ./*.bin ./*.o ./*.ll

.PHONY: all clean
.SECONDARY: $(EXAMPLES:.bin=.ll) $(EXAMPLES:.bin=.o)

