/ SCCSID: @(#)killpg.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C library -- killpg

.globl	_killpg, cerror

_killpg:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(sp),r0
	mov	6(sp),8f
	neg	8f		/ kill with - signo is killpg
	sys	0; 9f
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc

.data
9:
	sys	kill; 8:..
