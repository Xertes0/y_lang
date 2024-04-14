.PHONY: all
all:
	$(MAKE) -C compiler/
	$(MAKE) -C libstd/

.PHONY: clean
clean:
	$(MAKE) -C compiler/ $@
	$(MAKE) -C libstd/ $@
