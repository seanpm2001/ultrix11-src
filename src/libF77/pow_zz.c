
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)pow_zz.c	3.0	4/22/86
 */
#include "complex"

pow_zz(r,a,b)
dcomplex *r, *a, *b;
{
double logr, logi, x, y;
double log(), exp(), cos(), sin(), atan2(), cabs();

logr = log( cabs(a->dreal, a->dimag) );
logi = atan2(a->dimag, a->dreal);

x = exp( logr * b->dreal - logi * b->dimag );
y = logr * b->dimag + logi * b->dreal;

r->dreal = x * cos(y);
r->dimag = x * sin(y);
}
