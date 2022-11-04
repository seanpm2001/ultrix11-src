
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)space.c	3.0	4/22/86
 */
extern float boty;
extern float botx;
extern float oboty;
extern float obotx;
extern float scalex;
extern float scaley;
/* float deltx 767.;
float delty 479.; */
float delta 479.;
space(x0,y0,x1,y1){
/*	botx = -384.;
	boty = -240.; */
	botx = 0.;
	boty = 0.;
	obotx = x0;
	oboty = y0;
	scalex = delta/(x1-x0);
	scaley = delta/(y1-y0);
}
