/ SCCSID: @(#)setreuid.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
.globl	cerror
.globl  _setreuid

setreuid = 107.

_setreuid:
	mov	r5,-(sp)
	mov	sp,r5
	mov     4(r5),r0
	mov     6(r5),r1
	sys	setreuid
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc
