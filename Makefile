.PHONY: all
all:
	$(MAKE) -C compiler/
	$(MAKE) -C libstd/

.PHONY: run
run: all
	$(MAKE) -C compiler/ run

.PHONY: clean
clean:
	$(MAKE) -C compiler/ $@
	$(MAKE) -C libstd/ $@
