/ SCCSID: @(#)zaptty.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C library -- zaptty

/ error = zaptty();

.globl	_zaptty, cerror
.zaptty = 69.

_zaptty:
	mov	r5,-(sp)
	mov	sp,r5
	sys	.zaptty
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc
