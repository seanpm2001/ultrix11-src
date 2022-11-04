/ SCCSID: @(#)readlink.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C library -- readlink

/ error = readlink(path, buf, bufsiz);
/	  char *path, *buf;
/	  int bufsiz;

.globl  _readlink
.globl	cerror
.readlink = 110.

_readlink:
	mov	r5,-(sp)
	mov	sp,r5
	mov     4(r5),r0
	mov     6(r5),0f
	mov	010(r5),0f+2
	sys	0; 9f
	bec	1f
	jmp	cerror
1:
	mov	(sp)+,r5
	rts	pc
.data
9:
	sys	.readlink; 0:..; ..
