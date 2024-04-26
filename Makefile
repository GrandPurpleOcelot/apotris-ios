#!/usr/bin/env make

all: gba linux w32

gba:
	@$(MAKE) --no-print-directory -C . -f make/gba_make

linux:
	@$(MAKE) --no-print-directory -f make/linux_make

w32:
	@$(MAKE) --no-print-directory -f make/w32_make

mm:
	@$(MAKE) --no-print-directory -f make/mm_make

web:
	@$(MAKE) --no-print-directory -f make/wasm_make

switch:
	@$(MAKE) --no-print-directory -C . -f make/switch_make

rgb30:
	@$(MAKE) --no-print-directory -C . -f make/rgb30_make

clean:
	@rm -rf external/Tilengine/src/build
	@rm -rf build bin
