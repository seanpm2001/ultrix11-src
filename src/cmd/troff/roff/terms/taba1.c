
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*	SCCSID: @(#)taba1.c	3.0	4/22/86	*/
/*	(SYSTEM 5.0)	taba1.c	1.1	*/
#define INCH 240
/*
DASI450
nroff driving tables
width and code tables
*/

struct termtable ta1 = {
/*bset*/	0,
/*breset*/	054,
/*Hor*/		INCH/60,
/*Vert*/	INCH/48,
/*Newline*/	INCH/6,
/*Char*/	INCH/10,
/*Em*/		INCH/10,
/*Halfline*/	INCH/12,
/*Adj*/		INCH/10,
/*twinit*/	"\033R",
/*twrest*/	"\033R",
/*twnl*/	"\015\n",
/*hlr*/		"\033S",
/*hlf*/		"\033B",
/*flr*/		"\033\n",
/*bdon*/	"",
/*bdoff*/	"",
/*iton*/	"",
/*itoff*/	"",
/*ploton*/	"\0334",
/*plotoff*/	"\033R",
/*up*/		"\005",
/*down*/	"\n",
/*right*/	" ",
/*left*/	"\b",
/*codetab*/
#include "code.300"
