/ SCCSID: @(#)srt2.s	3.0	4/21/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ Startup code for RABADS standalone program
/ to be loaded and run via `boot'.
/ Fred Canter

/ non-UNIX instructions
mfpi	= 6500^tst
stst	= 170300^tst
mtpi	= 6600^tst
mfpd	= 106500^tst
mtpd	= 106600^tst
spl	= 230
ldfps	= 170100^tst
stfps	= 170200^tst
wait	= 1
rtt	= 6
reset	= 5
halt	= 0
/ trap	= 104400

PS	= 177776

.globl	_end
.globl	_main, __rtt
.globl	_edata
.globl	_devsw
	jmp	start

/ trap vectors

	trap;340	/ bus error
	trap;341	/ illegal instruction
	trap;342	/ BPT
	trap;343	/ IOT
	trap;344	/ POWER FAIL
	trap;345	/ EMT
tvec:
	start;346	/ TRAP

.=100^.
	_devsw		/ address of devsw[] table for passing device
			/ CSR addresses from the Boot: program.
			/ replaced by stray vector catcher.

.=114^.
	trap;352	/ Memory parity

.=240^.
	trap;347	/ Programmed interrupt request
	trap;350	/ Floating point
	trap;351	/ M/M segmentation

.=1000^.

tpc:	0	/ Save PS & PC from trap, used to start
tps:	0	/ program, and use for return to boot.
saveps: 0	/ save PS after trap (PS is trap type)

/ Argument passing buffer,
/ args placed here by sdload program.
/ If argflag = 0, argument buffer is ignored.
/ If argflag = 1, gets() & getchar() read from the argument
/ buffer instead of the console terminal keyboard.
/ Location rtnstat holds exit status of standalone program, sdload
/ gets status from rtnstat via mpfi().

.=1010^.
.globl	_argflag, _argbuf, _rtnstat

_rtnstat: 0
_argflag: 0
_argbuf: .=.+126.

.text

start:
	mov	$340,*$PS
	mov	(sp)+,tpc
	mov	(sp)+,tps
	mov	$trap,tvec

/ Load stray vector catchers in unused vector locations

	clr	r0
4:
	mov	$trap,(r0)+
	mov	$357,(r0)+
	br	2f
1:
	tst	(r0)+
	tst	(r0)+
2:
	cmp	r0,$1000
	bge	3f
	tst	2(r0)
	bne	1b
	br	4b
3:

/ Clear bss area of program.

	mov	$157776,sp
	mov	$_edata,r0
	mov	$_end,r1
	sub	r0,r1
	inc	r1
	clc
	ror	r1
1:
	clr	(r0)+
	sob	r1,1b
	jsr	pc,_main

/ fix up stack to point at trap ps-pc pair
/ so we can return to the bootstrap
__rtt:
	mov	$157776,sp
	mov	tps,-(sp)
	mov	tpc,-(sp)
	rtt				/ we hope!
	br	.


.globl	_trap
trap:
	mov	*$PS,saveps
	mov	r0,-(sp)
	mov	r1,-(sp)
	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	r4,-(sp)
	mov	r5,-(sp)
	mov	sp,-(sp)
	sub	$4,(sp)
	mov	saveps,-(sp)
	jsr	pc,_trap
	halt		/ _trap never returns
	br	.

/ mtpi(word,addr)
.globl	_mtpi
_mtpi:
	mov	4(sp),r0
	mov	2(sp),-(sp)
	mtpi	(r0)+
	rts	pc

/ mfpi(addr), word returned in r0
.globl _mfpi
_mfpi:
	mov	2(sp),r1
	mfpi	(r1)
	mov	(sp)+,r0
	rts	pc

