
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: "@(#)sleep.c	3.0	4/22/86"
 */
/*	@(#)sleep.c	2.1	SCCS id keyword	*/
#include <signal.h>
#include <setjmp.h>

static jmp_buf jmp;

sleep(n)
unsigned n;
{
	int sleepx();
	unsigned altime;
	int (*alsig)() = SIG_DFL;

	if (n==0)
		return;
	altime = alarm(1000);	/* time to maneuver */
	if (setjmp(jmp)) {
		sigset(SIGALRM, alsig);
		alarm(altime);
		return;
	}
	if (altime) {
		if (altime > n)
			altime -= n;
		else {
			n = altime;
			altime = 1;
		}
	}
	alsig = sigset(SIGALRM, sleepx);
	alarm(n);
	for(;;)
		pause();
	/*NOTREACHED*/
}

#ifndef C_OVERLAY
static
#endif
sleepx()
{
	longjmp(jmp, 1);
}
