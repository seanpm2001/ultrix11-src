
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/* SCCSID: @(#)logo.c	3.0	4/22/86 */
main()
{
openpl();
erase();
line( 20, 30, 20, 60);
line( 20, 60, 40, 60);
line( 40, 60, 40, 45);
line( 40, 45, 20, 45);
line( 50, 30, 50, 60);
line( 50, 60, 70, 60);
line( 70, 60, 70, 45);
line( 70, 45, 50, 45);
line( 60, 45, 70, 30);
line( 80, 30, 80, 60);
line( 80, 60,100, 60);
line(100, 60,100, 30);
line(100, 30, 80, 30);
line(110, 30,130, 60);
line(140, 60,150, 30);
line(150, 30,160, 60);
line(170, 60,190, 60);
line(190, 60,170, 30);
line(200, 30,200, 60);
line(200, 60,210, 45);
line(210, 45,220, 60);
line(220,60,220,30);
closepl();
}
