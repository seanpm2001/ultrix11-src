
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)abs.c	3.0	4/22/86
 */
abs(arg)
{

	if(arg < 0)
		arg = -arg;
	return(arg);
}
