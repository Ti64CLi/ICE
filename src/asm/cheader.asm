.assume adl = 1
segment data
.def _CHeaderData

_CHeaderData:
	ld      hl, LibLoadAppVar - $ + 0D1A881h
	call	0020320h
	ld      a, 015h
	ld      (0D005F8h), a
FindProgram:
	call	002050Ch
	jr      c, NotFound
	call	0021F98h
	jr      nz, InArc
	call	0020628h
	call	0021448h
	call	00205C4h
	jr      FindProgram
InArc:
	ex      de, hl
	ld      de, 9
	add     hl, de
	ld      e, (hl)
	add     hl, de
	inc     hl
	inc     hl
	inc     hl
	ld      de, RelocationStart - $ + 0D1A881h
	jp      (hl)
NotFound:
	call	0020814h
	call	0020828h
	ld      hl, MissingAppVar - $ + 0D1A881h
	call	00207C0h
	call	00207F0h
	jp	    00207C0h
MissingAppVar:
	db "Need"
LibLoadAppVar:
	db " LibLoad", 0
	db "tiny.cc/clibs", 0
RelocationStart:
	db 0C0h, "GRAPHX", 0, 5