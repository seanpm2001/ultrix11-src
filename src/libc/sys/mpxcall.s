/ SCCSID: @(#)mpxcall.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
.globl _mpxcall, cerror

_mpxcall:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),0f
	mov	6(r5),0f+2
call:
	sys	0; 9f
	bec	1f
	jmp	cerror
1:
	mov	(sp)+,r5
	rts	pc
.data
9:
	sys	56.; 0:..; ..

