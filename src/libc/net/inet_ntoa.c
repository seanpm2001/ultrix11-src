
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)inet_ntoa.c	3.0	4/22/86
 * Based on	inet_ntoa.c	4.1	83/06/12
 */

/*
 * Convert network-format internet address
 * to base 256 d.d.d.d representation.
 */
#include <sys/types.h>
#include <netinet/in.h>

char *
inet_ntoa(in)
	struct in_addr in;
{
	static char b[18];
	register char *p;

	p = (char *)&in;
#define	UC(b)	(((int)b)&0xff)
	sprintf(b, "%d.%d.%d.%d", UC(p[0]), UC(p[1]), UC(p[2]), UC(p[3]));
	return (b);
}
