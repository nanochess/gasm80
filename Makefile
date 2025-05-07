# Ultra simple makefile for gasm80
# by Oscar Toledo G.
# https://github.com/nanochess/gasm80
#
build:
	@$(CC) gasm80.c -o gasm80

check: build
	-./gasm80 test/hello.asm -o /tmp/game.rom
	-./gasm80 test/test.asm -o /tmp/game.rom
	./gasm80 test/test6502.asm -o /tmp/game.rom

clean:
	@rm gasm80

love:
	@echo "...not war"
