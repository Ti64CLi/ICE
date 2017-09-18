.assume adl = 1
segment data
.def _MultWithNumber

_MultWithNumber:
	ld	iy, 0
	add	iy, sp
	ld	hl, (iy+3)
	ld	de, (iy+3)
	ld	b, 0
	ld	a, 26
GetFirstBitSetLoop:
	dec	a
	add	hl, hl
	jr	nc, GetFirstBitSetLoop
GetAmountOfOpcodesNeededLoop:
	adc	a, b
	adc	hl, hl
	jr	nz, GetAmountOfOpcodesNeededLoop
	rl	b
	jr	nz, DunnoWhatThisDoes
	sub	a, 3
DunnoWhatThisDoes:
	cp	a, 10
	jr	c, DoInsertHLDE
	ld	hl, -256
	add	hl, de
	jr	c, MultiplyLarge
MultiplySmall:
	ld	a, (iy+9)
	or	a, a
	ld	a, 0EBh
	call	nz, InsertA
	ld	hl, 0CD003Eh
	ld	h, e
	ld	de, 0000150h
	ex	de, hl
	jp	InsertDEHL
MultiplyLarge:
	ld	a, (iy+9)
	or	a, a
	ld	a, 0EBh
	call	nz, InsertA
	ex	de, hl
	ld	a, 001h
	call	InsertAHL
	ld	hl, 0000154h
	jp	InsertCallHL
DoInsertHLDE:
	djnz	DontNeedPushHLPopDE
	ld	a, 0E5h
	sub	a, (iy+9)
	call	InsertA
	ld	a, 0D1h
	add	a, (iy+9)
	call	InsertA
	jr	DontInsertEXDEHL2
DontNeedPushHLPopDE:
	ld	a, (iy+9)
	or	a, a
	ld	a, 0EBh
	call	nz, InsertA
DontInsertEXDEHL2:
	ex	de, hl
	scf
GetFirstSetBit2Loop:
	adc	hl, hl
	jr	nc, GetFirstSetBit2Loop
InsertAddHLDE:
	or	a, a
	adc	hl, hl
	ret	z
	ld	a, 029h
	call	InsertA
	ld	a, 019h
	call	c, InsertA
	jr	InsertAddHLDE
    
InsertA:
	push	hl
	ld	hl, (iy+6)
	push	hl
	ld	bc, (hl)
	ld	(bc), a
	inc	bc
	pop	hl
	ld	(hl), bc
	pop	hl
	ret
InsertAHL:
	push	hl
	call	InsertA
	pop	hl
InsertHL:
	ex	de, hl
	ld	hl, (iy+6)
	push	hl
	ld	hl, (hl)
	ld	(hl), de
	inc	hl
	inc	hl
	inc	hl
	ex	de, hl
	pop	hl
	ld	(hl), de
	ret
InsertCallHL:
	ld	a, 0CDh
	jr	InsertAHL
InsertDEHL:
	push	hl
	ex	de, hl
	call	InsertHL
	pop	hl
	jr	InsertHL
