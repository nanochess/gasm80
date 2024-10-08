# Gasm80: Generic Z80/6502 assembler
### by Oscar Toledo G. Mar/03/2024

[https://nanochess.org/](https://nanochess.org/)

[https://github.com/nanochess/gasm80](https://github.com/nanochess/gasm80)

Gasm80 is a small assembler for Z80/6502 programs. It was developed to support my new CVBasic compiler for Colecovision, in order to have a complete toolchain that can work in macOS and Linux.

It was developed in a single day based on my tinyasm 8088 assembler available at [https://github.com/nanochess/tinyasm](https://github.com/nanochess/tinyasm)

It uses a command line syntax similar to nasm:

    gasm80 game.asm -o game.rom -l game.lst

There is also the -d option for defining labels:

    -dLABEL
    -dANOTHER_LABEL=1

It returns a non-zero error code when the assembled file generates errors.

### Supported directives

Labels are started with letter, number sign, underscore or period. Case is insensitive.

Local labels are supported, and should start with period. The local labels final name is derived from concatenation of the last global label (not starting with period) and the local label.

There is only support for Z80 and 6502 processors, and only are implemented the following directives:

    IFDEF
    IFNDEF
    IF
    ELSE
    ENDIF
    INCLUDE
    INCLINE
    INCBIN
    TIMES
    CPU Z80
    CPU 6502
    EQU
    DB
    DW
    RB

The following operators are implemented:

	|	Binary OR
	^	Binary XOR
	&	Binary AND
	<<	Shift to left
	>>	Shift to right
	+	Addition 
	-	Subtraction 
	* 	Multiplication 
	/	Division (unsigned 16-bit)
	%	Modulo operator
	(expr)	Parenthesis
	-	Unary negation

The following numbers are implemented:

	0b0000_0100	Binary, you can use underscore (it will be ignored)
	0xabcd		Hexadecimal.
	$0abcd		Hexadecimal (after $ the first digit must be a number)
	'a'		Character constant.
	10		Decimal.
	$$		Start address.
	$		Current address.

This assembler won't win a speed test ;) because the internal implementation uses a linear search for the instruction set, and it is also implemented as a kind of regular expression subset for easier coding.


### Building the assembler

The assembler is easily built in any platform because it uses standard C:

    gcc gasm80.c -o gasm80

I've included test cases in the 'test' subdirectory that includes the complete Z80 and 6502 instruction set.
