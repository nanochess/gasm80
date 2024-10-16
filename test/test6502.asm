	;
	; 6502 instruction set test for GASM80
	;
	; by Oscar Toledo G.
	; https://nanochess.org/
	;
	; Creation date: Aug/02/2024.
	;

	CPU 6502

	BRX
	BPL $+5
	JSR $aabb
	BMI $+5
	RTI
	BVC $+5
	RTS
	BVS $+5
	BCC $+5
	LDY #$55
	LDX #$55
	BCS $+5
	CPY #$55
	BNE $+5
	CPX #$55
	BEQ $+5
	ORA ($66,X)
	ORA ($66),Y
	AND ($66,X)
	AND ($66),Y
	EOR ($66,X)
	EOR ($66),Y
	ADC ($66,X)
	ADC ($66),Y
	STA ($66,X)
	STA ($66),Y
	LDA ($66,X)
	LDA ($66),Y
	CMP ($66,X)
	CMP ($66),Y
	SBC ($66,X)
	SBC ($66),Y
	BIT $66
	STY $66
	STY $66,X
	LDY $66
	LDY $66,X
	CPY $66
	CPX $66
	ORA $66
	ORA $66,X
	AND $66
	AND $66,X
	EOR $66
	EOR $66,X
	ADC $66
	ADC $66,X
	STA $66
	STA $66,X
	LDA $66
	LDA $66,X
	CMP $66
	CMP $66,X
	SBC $66
	SBC $66,X
	ASL $66
	ASL $66,X
	ROL $66
	ROL $66,X
	LSR $66
	LSR $66,X
	ROR $66
	ROR $66,X
	STX $66
	STX $66,Y
	LDX $66
	LDX $66,Y
	DEC $66
	DEC $66,X
	INC $66
	INC $66,X
	PHP
	CLC
	PLP
	SEC
	PHA
	CLI
	PLA
	SEI
	DEY
	TYA
	TAY
	CLV
	INY
	CLD
	INX
	SED
	ORA #$55
	ORA $aabb,Y
	AND #$55
	AND $aabb,Y
	EOR #$55
	EOR $aabb,Y
	ADC #$55
	ADC $aabb,Y
	STA $aabb,Y
	LDA #$55
	LDA $aabb,Y
	CMP #$55
	CMP $aabb,Y
	SBC #$55
	SBC $aabb,Y
	ASL A
	ROL A
	LSR A
	ROR A
	TXA
	TXS
	TAX
	TSX
	DEX
	NOP
	BIT $aabb
	JMP ($aabb)
	JMP $aabb
	STY $aabb
	LDY $aabb
	LDY $aabb,X
	CPY $aabb
	CPX $aabb
	ORA $aabb
	ORA $aabb,X
	AND $aabb
	AND $aabb,X
	EOR $aabb
	EOR $aabb,X
	ADC $aabb
	ADC $aabb,X
	STA $aabb
	STA $aabb,X
	LDA $aabb
	LDA $aabb,X
	CMP $aabb
	CMP $aabb,X
	SBC $aabb
	SBC $aabb,X
	ASL $aabb
	ASL $aabb,X
	ROL $aabb
	ROL $aabb,X
	LSR $aabb
	LSR $aabb,X
	ROR $aabb
	ROR $aabb,X
	STX $aabb
	LDX $aabb
	LDX $aabb,Y
	DEC $aabb
	DEC $aabb,X
	INC $aabb
	INC $aabb,X

	; Undocumented instructions
	SLO ($66,X)
	SLO ($66),Y
	RLA ($66,X)
	RLA ($66),Y
	SRE ($66,X)
	SRE ($66),Y
	RRA ($66,X)
	RRA ($66),Y
	SAX ($66,X)
	SHA ($66),Y
	LAX ($66,X)
	LAX ($66),Y
	DCP ($66,X)
	DCP ($66),Y
	ISC ($66,X)
	ISC ($66),Y
	SLO $66
	SLO $66,X
	RLA $66
	RLA $66,X
	SRE $66
	SRE $66,X
	RRA $66
	RRA $66,X
	SAX $66
	SAX $66,Y
	LAX $66
	LAX $66,Y
	DCP $66
	DCP $66,X
	ISC $66
	ISC $66,X
	ANC #$55
	SLO $aabb,Y
	RLA $aabb,Y
	ALR #$55
	SRE $aabb,Y
	ARR #$55
	RRA $aabb,Y
	ANE #$55
	TAS $aabb,Y
	LXA #$55
	LAS $aabb,Y
	SBX #$55
	DCP $aabb,Y
	USBC #$55
	ISC $aabb,Y
	SHY $aabb,X
	SHX $aabb,Y
	SLO $aabb
	SLO $aabb,X
	RLA $aabb
	RLA $aabb,X
	SRE $aabb
	SRE $aabb,X
	RRA $aabb
	RRA $aabb,X
	SAX $aabb
	SHA $aabb,Y
	LAX $aabb
	LAX $aabb,Y
	DCP $aabb
	DCP $aabb,X
	ISC $aabb
	ISC $aabb,X
