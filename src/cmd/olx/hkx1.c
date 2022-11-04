
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)hkx1.c	3.0	4/22/86";
/*
 * ULTRIX-11 HK disk exerciser program (hkx).
 *
 * PART 1 - (hkx1.c)
 *
 *	Part 1 does the initial setup and argument processing,
 *	writes the results into a file `hkx_#.arg' (# = drive).
 *	and then calls part 2 of hkx, which is the actual exerciser.
 *	The program is split into two sections to optimize
 *	memory usage.
 *
 * Fred Canter 6/16/83
 * Bill Burns 4/84
 *	added overlapping partition check code
 *	added event flag code
 *
 *	********************************************
 *      *                                          *
 *      * This program will not function correctly *
 *      * unless the current unix monitor file is  *
 *      * named "unix".                            *
 *	*					   *
 *      ********************************************
 *
 * This program exercises the RK06 & RK07 disks
 * via the unix block and raw I/O interfaces, at
 * the user level.
 * It reports the occurrence of hard errors and/or data
 * compare errors on the standard output.
 * The detailed device error information must be obtained
 * the system error log (elp -hk).
 *
 *
 *	CHANGES FOR USER SETABLE DISK PARTITIONS -- Fred Canter 7/6/85
 *
 *	The disk partition sizes table in the currently running kernel is
 *	compared with the standard sizes table, and the operation of HKX
 * 	is modified accordingly. The standard sizes table is hard coded
 *	into the HK exerciser via "#include /usr/sys/conf/dksizes.c".
 *
 *	If the sizes tables match, then HKX allows full functionality, i.e,
 *	it knows the disk layout and can protect the user from him/her self.
 *
 *	If the sizes tables don't match, it is assumed that the user changed
 *	the partition layout for some reason, and HKX operation is modified
 *	as follows:
 *
 *	1.	If the user changed partition 6 or 7, fatal error exit.
 *		(6 for RK06, 7 for RK07)
 *
 *	2.	If the disk is the system disk (root, swap, error log) or
 *		has any mounted file systems, then the entire disk will be
 *		treated as read only.
 *
 *	3.	If the disk is not the system disk and has no mounted file
 *		systems, then partition 6 or 7 will be a free fire zone. Only
 *		partition 6 or 7 will be used.
 *
 *	4.	The -f flag is ignored.
 *
 *
 * USAGE:
 *
 *	hkx -h
 *
 *		Print the help message.
 *
 *
 *	hkx [-d#] [-f#] [-m] [-s#] [-i] [-n #] [-e #] [-z #]
 *
 *		Exercise the disk(s), using random addresses,
 *		random byte counts, and alternating worst case
 *		and random data patterns.
 *
 *	[-d#] -	Select the drive to be exercised.
 *		Only one drive may be selected.
 *
 *	[-f#] -	Select a file system to be exercised.
 *		If multiple file systems are selected
 *		only the lowest numbered one will be exercised.
 *
 *	[-m]  - Print the rk06/7 disk file system layout.
 *
 *	[-s#] -	Specify time intervals for periodic I/O statistics printouts.
 *		The time interval (#) is in minutes and can range
 *		from 1 to 720 (12 hours).
 *		The default time interval is 1/2 hour.
 *		If no time interval is specified (-s only),
 *		the I/O statistics are not printed.
 *
 *	[-i]  -	Inhibit the file system status printouts.
 *
 *	[-n #] - Specify the number (#) of data mismatch errors
 *		to be printer per failing block.
 *		The maximum is 256 and the default is 5.
 *
 *	[-e #] - Drop the disk after # hard errors,
 *		 default = 100, maximum = 1000.
 *
 *	[-z #] - event flag bit position, used to start/stop
 *		exercisers
 *
 *	note  -	The root, swap, error log, and any mounted file
 *		systems are automatically write protected.
 *		If any file systems on the selected drive
 *		are marked read only then
 *		file systems 6 & 7 are also read only,
 *		because they allow access to the entire pack.
 *
 *	Below is gone due to event flag usage:
 *	[-r kfn] - kill file name
 *
 * EXAMPLE:
 *		hkx -d1 -m
 *
 *		Print the file system map and exercise all
 *		file systems on drive 1.
 *
 *
 */

#include <sys/param.h>
#include <sys/devmaj.h>
#include <sys/mount.h>
#define	USEP	1
#define	NHK	1
#include "/usr/sys/conf/dksizes.c"
#include <stdio.h>
#include <a.out.h>
#include <signal.h>

/*
 * Structure for accessing symbols in the unix kernel
 * via the nlist subroutine and the /dev/mem driver.
 */

struct nlist nl[] =
{
	{ "_hk_size" },
	{ "_rootdev" },
	{ "_swapdev" },
	{ "_swplo" },
	{ "_nswap" },
	{ "_el_dev" },
	{ "_el_sb" },
	{ "_el_nb" },
	{ "_cputype" },
	{ "_nmount" },
	{ "_usermem" },
	{ "_hk_dt" },
	{ "_mount" },
	{ "ova" },
	{ "" },
};

/*
 * The following are the symbols who's values are obtained
 * from the unix kernel.
 * THE ORDER MUST NOT BE CHANGED !
 */

int	rootdev;
int	swapdev;
int	swplo;
int	nswap;
int	el_dev;
int	el_sb;
int	el_nb;
int	cputype;
int	nmount;
int	usermem;

/*
 * Text array used by the print file
 * system read only status code.
 */

char	*fsutab[] =
{
	"dummy",
	"root",
	"swap",
	"error log",
	"mounted",
	"overlapping",
	0
};

/*
 * Control variables for dealing with user
 * setable disk partitions.
 */

int	nsdp;	/* non-standard disk partition layout being used */
int	dpmask;	/* bitwise 1 = partition used by drive being exericsed */

/*
 * File system layout, also
 * obtained from the unix kernel.
 *
 * Sizes proto now in /usr/sys/conf/dksizes.c.
 */

struct size
{
	daddr_t nblocks;
	int	cyloff;
};

struct size hksizes[8];

/*
 * Overlapping partition array,
 * set dynamically by examining sizes table.
 */

int	ovp[8];

/*
 * Unit number.
 */

int	dn = -1;

/*
 * Drive types,
 *	0	= RK06
 *	02000	= RK07
 *	-1	= NED
 */

int	hk_dt[8];

/*
 * drive can be opened flag.
 */

int	hk_opn;

/*
 * Selected file system array.
 * Element = 0 for file system not selected.
 * Element = 1 for file system selected.
 */

char fsact[] {0, 0, 0, 0, 0, 0, 0, 0};

/*
 * File system write read status array.
 * element is zero for write/read access
 * element is non zero for read only access
 */

char	fswrs[8];

/*
 * block and raw I/O file name.
 */

char	fn[20];

/*
 * Armument file name
 */

char	afn[] = "hkx_0.arg";

/*
 * Help message.
 */

char *help[]
{
	"\n\n(hkx) - ULTRIX-11 RK06/7 disk ecerciser.",
	"\nUsage:\n",
	"\thkx [-h] [-d#] [-f#] [-m] [-s#] [-i] [-n #] [-e #]",
	"\n-h\tPrint this help message",
	"\n-d#\tSelect drive number `#' ",
	"\n-f#\tExercise only file system `#'",
	"\n-m\tPrint file system layout",
	"\n-s#\tPrint I/O statistics every `#' minutes (default = 30 minutes)",
	"\n-s\tInhibit I/O statistics printout",
	"\n-i\tInhibit file system write/read status printout",
	"\n-n #\tLimit number of data compare error printouts to `#'",
	"\n-e #\tDrop the disk after # errors, default = 100, maximum = 1000",
	"\n\n",
	0
};

/*
 * Time buffers.
 */

int	istime = 30;

#define RK6 01
#define RK7 02

char	argbuf[512];

int	fflag, mflag, sflag, iflag;

int	ndep = 5;
int	ndrop = 100;

#ifdef EFLG
char	*efbit;
char	*efids;
int	zflag;
#else
char	*killfn = "hkx.kill";
#endif

main(argc, argv)
char *argv[];
int argc;
{
	int	stop(), intr();
	register struct mount *mtp;
	register char *p;
	register int i;
	int *ap;
	int j, k;
	int fd;
	char *n;
	char c;
	char *mp;
	int mem;
	int a, b;
	int x, y;
	int	ronly;
	int	bufsiz, bcmask;
	daddr_t	i_sb, i_eb, j_sb, j_eb;

	setpgrp(0, 31111);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGQUIT, stop);
	if(argc < 2)
		goto aderr;
	for(i=1; i < argc; i++) {	/* decode arg's */
		p = argv[i];
		if(*p++ != '-') {
		aderr:
			fprintf(stderr,"\nhkx: bad arg\n");
			exit(1);
		}
		switch(*p) {
		case 'h':	/* print help message */
			for(j=0; help[j]; j++)
				fprintf(stderr,"\n%s",help[j]);
			exit();
#ifdef EFLG
		case 'z':
			zflag++;
			i++;
			efbit = argv[i++];
			efids = argv[i];
			break;
#else
		case 'r':	/* kill file name */
			i++;
			killfn = argv[i];
			break;
#endif
		case 'd':	/* select drive */
			p++;
			if(*p < '0' || *p > '7')
				goto aderr;
			dn = *p - '0';
			afn[4] = *p;	/* argument file name */
			break;
		case 'f':	/* select file system */
			fflag++;
			p++;
			if(*p < '0' || *p > '7')
				goto aderr;
			fsact[*p - '0']++;
			break;
		case 'm':	/* print file system map */
			mflag++;
			break;
		case 'n':	/* # of data errors to print */
			i++;
			ndep = atoi(argv[i]);
			if((ndep <= 0) || (ndep > 256))
				ndep = 5;
			break;
		case 'e':	/* drop disk after # errors */
			i++;
			ndrop = atoi(argv[i]);
			if((ndrop <= 0) || (ndrop > 1000))
				ndrop = 1000;
			break;
		case 'i':	/* Inhibit file system status printouts */
			iflag++;
			break;
		case 's':
			sflag++;
			p++;
			if(*p < '0' || *p >'9')
				break;
			sflag = 0;
			istime = atoi(p);
			if(istime <= 0)
				istime = 30;
			if(istime > 720)
				istime = 720;
			break;
		default:	/* bad argument */
			goto aderr;
		}
	}
	if(!zflag) {
		if(isatty(2)) {
			fprintf(stderr,"hkx: detaching... type \"sysxstop\" to stop\n");
			fflush(stderr);
		}
		if((i = fork()) == -1) {
			printf("hkx: Can't fork new copy !\n");
			exit(1);
		}
		if(i != 0)
			exit(0);
	}
	setpgrp(0, 31111);
/*
 * Check for invalid option combinatons & defaults.
 */

	if((dn < 0) && ((argc !=2) && (!mflag))) {
		fprintf(stderr, "\nhkx: must select drive\n");
		exit(1);
	}
	if(!fflag) {
		for(j=0; j<8; j++)
			fsact[j]++;
	}

/*
 * Attempt to open the first file system on selected drive,
 * if successful attempt to read first block of the file system.
 * This is done in order to force the disk driver to update
 * the hk_dt[] array with the drive type of each available unit.
 * This allows for drives to be enabled/disabled via the LAP
 * without requiring a reboot of the system.
 * This should not cause any errors to be logged.
 */

	sprintf(&fn, "/dev/hk%o0", dn);		/* file name */
	if((a = open(fn, 0)) >= 0)
		if(read(a, (char *) &argbuf, 512) == 512)
			hk_opn++;
	close(a);

/*
 * Use the nlist subroutine & /dev/mem to set the values
 * to obtain needed data from the unix kernel.
 */

	nlist("/unix", nl);
	for(i=1; i<13; i++ ) {	/* don't check size table */
		if(i == 11)	/* don't check drive type */
			continue;
		if(nl[i].n_type == 0) {
		      fprintf(stderr,"\nhkx: Can't access namelist in /unix\n");
			exit(1);
		}
	}
	if((nl[11].n_type == 0) || (nl[0].n_type == 0)) {
	    fprintf(stderr, "\nhkx: /unix not configured for RK06/7 disk\n");
	    exit(1);
	}
	if((mem = open("/dev/mem", 0)) < 0) {
		fprintf(stderr,"\nhkx: Can't open /dev/mem\n");
		exit(1);
	}
	mp = &rootdev;
	for(i=1; i<11; i++) {
		lseek(mem, (long)nl[i].n_value, 0);
		read(mem, (char *)mp, sizeof(int));
		mp += sizeof(int);
	}
/*
 * Get the disk file system layout.
 */

	lseek(mem, (long)nl[0].n_value, 0);
	read(mem, (char *)&hksizes, sizeof(hksizes));
/*
 * Get the drive types.
 */

	lseek(mem, (long)nl[11].n_value, 0);
	read(mem, (char *)&hk_dt, sizeof(hk_dt));

/*
 * Change the hk_dt values used by the hk driver
 * to the ones used by hkx.
 */
	for(i=0; i<8; i++) {
		if(hk_dt[i] == 0)
			hk_dt[i] = RK6;	/* rk06 */
		else if(hk_dt[i] == 02000)
			hk_dt[i] = RK7;	/* rk07 */
		else
			hk_dt[i] = 0;	/* NED */
	}
/*
 * Set the write and read buffer sizes
 * and transfer size limits, based on
 * the amount of user memory.
 */
	if(usermem >= 4096) {	/* 256 kb */
		bufsiz = 8192;
		bcmask = 037776;
	} else {
		bufsiz = 4096;
		bcmask = 017776;
	}
/*
 * Compare the prototype file system sizes
 * table with the real one to make sure that
 * HKX really does know about the disk layout.
 */

	nsdp = 0;
	for(i=0; i<8; i++)
		if((hksizes[i].nblocks != hk_sizes[i].nblocks)
		  ||  (hksizes[i].cyloff != hk_sizes[i].cyloff))
			nsdp++;
/*
 * If non standard partitions being used,
 * verify minimum sanity level of sizes table.
 *
 * RK06 partition 6 or RK07 partition 7 must not be changed.
 */
	if(nsdp) {
	    printf("\n****** RK0%d unit %d ", (5 + hk_dt[dn]), dn);
	    printf("non standard partition layout ******\n");
	    i = 0;
	    if(hk_dt[dn] == RK6)
		if((hksizes[6].cyloff != 0) || (hksizes[6].nblocks != 27082L))
		    i++;
	    if(hk_dt[dn] == RK7)
		if((hksizes[7].cyloff != 0) || (hksizes[7].nblocks != 53746L))
		    i++;
	    if(i) {
		fprintf(stderr, "\nhkx: fatal error - partition %d changed!\n",
		    5 + hk_dt[dn]);
		exit(1);
	    }
	}
/*
 * Set dpmask for partitions used by this
 * type of disk. For the most part, dpmask is
 * ignored if non standard partitions used.
 */
	if(hk_dt[dn] == RK6)
		dpmask = 0107;
	if(hk_dt[dn] == RK7)
		dpmask = 0213;
/*
 * Set up the overlapping partitions table.
 * Table is only used with standard partition layout.
 */
	for(i=0; i<8; i++) {		/* find overlapping partitions */
	    ovp[i] = 0;
	    if((dpmask & (1 << i)) == 0)
		continue;		/* non usable partition */
	    i_sb = hksizes[i].cyloff * 66L;	/* 66 blocks per cylinder */
	    i_eb = i_sb + hksizes[i].nblocks - 1;
	    for(j=0; j<8; j++) {
		if((dpmask & (1 << j)) == 0)
		    continue;		/* non usable partition */
		if(j == i)
		    continue;		/* can't overlap itself */
		j_sb = hksizes[j].cyloff * 66L;
		j_eb = j_sb + hksizes[j].nblocks - 1;
		if(((i_sb >= j_sb) && (i_sb <= j_eb)) ||
		   ((i_eb >= j_sb) && (i_eb <= j_eb)) ||
		   ((j_sb >= i_sb) && (j_sb <= i_eb)) ||
		   ((j_eb >= i_sb) && (j_eb <= i_eb)) ||
		   ((j_sb <= i_eb) && (j_eb >= i_sb)))
			ovp[i] |= (1 << j);
	    }
	}
/*
 * debug message
	fprintf(stderr, "\nDEBUG: ovp = ");
	for(i=0; i<8; i++)
		fprintf(stderr, "%o ", ovp[i]);
	fprintf(stderr, "\n");
*/

/*
 * Print the file system map if [-m] was specified.
 */

	if(mflag) {
		fprintf(stderr,"\n\nRK06/7 file system layout\n");
		fprintf(stderr, "\nfilsys\tcyl\tlength\n");
		for(i=0; i<8; i++) {
			if((nsdp == 0) && ((dpmask & (1 << i)) == 0))
				continue;
			if(hksizes[i].nblocks)
				fprintf(stderr, "\n%d\t%d\t%D",
				   i, hksizes[i].cyloff, hksizes[i].nblocks);
		}
		fprintf(stderr,"\n\n");
		exit(0);
	}
/*
 * Print the status of drives.
 */

	j = 0;
	printf("\nUnit %d - ", dn);
	if(hk_dt[dn] == RK6)
		printf("RK06 - ");
	if(hk_dt[dn] == RK7)
		printf("RK07 - ");
	if(hk_dt[dn] && hk_opn) {
		printf("accessible");
		j++;
	} else if(hk_dt[dn] && !hk_opn)
		printf("not accessible");
	else
		printf("non existent\n\n");
	if(j == 0)
		exit(1);

/*
 * Initialize the file system write/read status array.
 * Mark the root, swap, error log, & any mount file systems
 * read only.
 * Also mark file systems 6 & 7 read only if any other
 * file systems on the drive are read only.
 */

	/*
	 * Read the mount table from /unix.
	 *
	 * New stuff - mount table is now consistent between
	 * kernels (ov and sep I&D)  -  Bill Burns 8/13/84
	 */
	mtp = nl[12].n_value;
	lseek(mem, (long)mtp, 0);
	for(i=0; i<nmount; i++, mtp++) {
		p = &argbuf;
		p += (sizeof(struct mount) * i);
		read(mem, (char *)p, sizeof(struct mount));
	}
	close(mem);
	for(i=0; i<8; i++) {	/* check file systems for selected unit */
		a = (dn << 3) | i;	/* minor device number */
		fswrs[i] = cfs(a);	/* check file system status */
		if(fswrs[i])	/* root, swap, error log ? */
			continue;	/* yes, then read only ! */
		b = (HK_BMAJ << 8) | a;		/* maj/min device */
		for(mtp = &argbuf, j=0; j<nmount; mtp++, j++)
			if((mtp->m_bufp != NULL) && (mtp->m_dev == b)) {
				fswrs[i] = 4;	/* file system mounted */
				break;
			}
	}
/*
 * Loop to check for overlapping disk partitions
 */

	a = 0;
	for(j = 0; j < 8; j++) {
		a =+ fswrs[j];
		if((fswrs[j] > 0) && (fswrs[j] < 5)) 
			for(x = ovp[j], y = 0; y < 8; y++)
				if((x & (1 << y)) && (fswrs[y] == 0))
					fswrs[y] = 5;
	}
	ronly = a;

/*
 * Unless the iflag is set, print the list
 * of read only file systems.
 */
	if(!iflag) {
	    c = 0;
	    for(i=0; i<8; i++) {
		if(nsdp)
			break;
		if((dpmask & (1 << i)) == 0)
			continue;	/* un used partition */
		if(fswrs[i]) {
			if(!c++) {
				printf("\n\nRead only file systems.");
				printf("\n\nunit\tfile");
				printf("\nnumber\tsystem\treason");
			}
			printf("\n%d\t%d\t", dn, i);
			printf("%s",fsutab[fswrs[i]]);
			printf(" file system");
		}
	    }
	}
	if(nsdp) {
	    printf("\n****** Only using file system %d ", hk_dt[dn]+5);
	    printf("(non standard partitions used) ******\n");
	    if(ronly) {
		fswrs[6] = fswrs[7] = 5;
		printf("\n****** Unit %d read only ******\n", dn);
	    }
	}
/*
 * Write needed date into a file `hkx_#.arg' (# = drive),
 * and call part 2 of the HK exerciser (hkxr).
 */

	ap = &argbuf;
	*ap++ = dn;
	*ap++ = ndep;
	*ap++ = ndrop;
	*ap++ = fflag;
	*ap++ = sflag;
	*ap++ = istime;
	*ap++ = ronly;
	*ap++ = bufsiz;
	*ap++ = bcmask;
	*ap++ = nsdp;
	*ap++ = dpmask;
#ifdef EFLG
	*ap++ = zflag;
#endif
	p = ap;
	n = &hksizes;
	for(i=0; i<sizeof(hksizes); i++)
		*p++ = *n++;
	n = &hk_dt;
	for(i=0; i<sizeof(hk_dt); i++)
		*p++ = *n++;
	for(i=0; i<sizeof(fsact); i++)
		*p++ = fsact[i];
	for(i=0; i<sizeof(fswrs); i++)
		*p++ = fswrs[i];
	if((fd = creat(afn, 0644)) < 0) {
		fprintf(stderr, "\nhkx: Can't create %s file\n", afn);
		exit(1);
		}
	if(write(fd, (char *)&argbuf, 512) != 512) {
		fprintf(stderr, "\nhkx: %s write error\n", afn);
		exit(1);
		}
	close(fd);
	fflush(stdout);
	signal(SIGQUIT, SIG_IGN);
#ifdef EFLG
	if(zflag)
		execl("hkxr", "hkxr", afn, efbit, efids, (char *)0);
	else
		execl("hkxr", "hkxr", afn, (char *)0);
#else
	execl("hkxr", "hkxr", afn, killfn, (char *)0);
#endif
	fprintf(stderr, "\nhkx: Can't exec hkxr\n");
	exit(1);
}

/*
 * Check file system and return 0 if it is writable,
 * otherwise return 1 if it is the root file system,
 * 2 if it is the swap file system, and 3 if it
 * is the error log file system.
 */

cfs(dev)
{

	register int rm, sm, em;

	rm = (rootdev >> 8) & 0377;
	sm = (swapdev >> 8) & 0377;
	em = (el_dev >> 8) & 0377;
	if((rm == HK_BMAJ) && (dev == (rootdev & 0377)))
		return(1);
	if((sm == HK_BMAJ) && (dev == (swapdev & 0377)))
		return(2);
	if((em == HK_RMAJ) && (dev == (el_dev & 0377)))
		return(3);
	return(0);
}

stop()
{
	exit(0);
}
