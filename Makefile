#!/usr/bin/env make

all: gba linux w32

gba:
	@make --no-print-directory -C . -f make/gba_make

linux:
	@make --no-print-directory -f make/linux_make

w32:
	@make --no-print-directory -f make/w32_make

mm:
	@make --no-print-directory -f make/mm_make

clean:
	@rm -rf build bin
