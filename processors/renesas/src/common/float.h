/*******************************************************************************
*
* constants for floating point values header file
*
* float.h -- Version 2.00.00
* $Date: 2005/09/21 02:50:05 $
* $Revision: 1.14 $
*
* Copyright(C) 1999(2000-2004). Renesas Technology Corp.
* and Renesas Solutions Corp., All rights reserved.
*
*
*******************************************************************************/


#if defined(NC30)		/* NCxx */
	/* dummy */
#elif defined(NC77)
	#error "NC77 not supported"
#elif defined(NC79)
	#error "NC79 not supported"
#else
	#error "NC30, NC77, NC79 not defined"
#endif				/* NCxx */


#ifdef NEED_SJMP_FOR_LIB
_asm("	.SJMP	OFF");
#endif
#ifndef __FLOAT_H
#define __FLOAT_H

#define FLT_GUARD	1
#define FLT_NORMALIZE	1
#define DBL_DIG 	15
#define DBL_EXP_DIG	11
#define DBL_POS_EPS	1.1102230246251567e-16
#define DBL_NEG_EPS	5.5511151231257834e-17
#define DBL_POS_EPS_EXP	-52
#define DBL_NEG_EPS_EXP	-53
#define DBL_EPSILON	2.2204460492503131e-16
#define DBL_MANT_DIG	53
#define DBL_MAX 	1.7976931348623157e+308
#define DBL_MAX_10_EXP	308
#define DBL_MAX_EXP	1024
#define DBL_MIN 	2.2250738585072014e-308
#define DBL_MIN_10_EXP	(-307)
#define DBL_MIN_EXP	(-1021)

#define FLT_DIG 	6
#define FLT_EXP_DIG	8
#define FLT_POS_EPS	5.9604644886412935e-8F
#define FLT_NEG_EPS	2.9802322443206468e-8F
#define FLT_POS_EPS_EXP	-23
#define FLT_NEG_EPS_EXP	-24
#define FLT_EPSILON	1.19209290e-07F
#define FLT_MANT_DIG	24
#define FLT_MAX 	3.40282347e+38F
#define FLT_MAX_10_EXP	38
#define FLT_MAX_EXP	128
#define FLT_MIN 	1.17549435e-38F
#define FLT_MIN_10_EXP	(-37)
#define FLT_MIN_EXP	(-125)
#define FLT_RADIX	2
#define FLT_ROUNDS	1

#define LDBL_DIG	DBL_DIG
#define LDBL_POS_EPS	DBL_POS_EPS
#define LDBL_NEG_EPS	DBL_NEG_EPS
#define LDBL_POS_EPS_EXP	DBL_POS_EPS_EXP
#define LDBL_NEG_EPS_EXP	DBL_NEG_EPS_EXP
#define LDBL_EXP_DIG	DBL_EXP_DIG
#define LDBL_EPSILON	DBL_EPSILON
#define LDBL_MANT_DIG	DBL_MANT_DIG
#define LDBL_MAX	DBL_MAX
#define LDBL_MAX_10_EXP	DBL_MAX_10_EXP
#define LDBL_MAX_EXP	DBL_MAX_EXP
#define LDBL_MIN	DBL_MIN
#define LDBL_MIN_10_EXP	DBL_MIN_10_EXP
#define LDBL_MIN_EXP	DBL_MIN_EXP

/*
 * definitions below need to be defined
 * ONLY when compiling libraries:lib*.c
 */
#ifdef LIB_USE
#ifndef _LIBUSE_DEF
#define _LIBUSE_DEF

#ifdef OLD_TYPE
typedef char s_char;
typedef short s_short;
typedef int s_int;
typedef long s_long;
typedef long long s_llong;
#else	/* OLD_TYPE */
typedef signed char s_char;
typedef signed short s_short;
typedef signed int s_int;
typedef signed long s_long;
typedef signed long long s_llong;
#endif	/* OLD_TYPE */
typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;
typedef unsigned long long u_llong;
#ifdef OLD_TYPE
#define PROTO_TYPE(x) ()
#else
#define PROTO_TYPE(x) x
#endif

/*
 * please DON'T USE macro SEQUENCE(x,y) !!
 * this macro will be removed in future.
 *
 * for NC30, SEQUENCE(x,y) is defined as y,x (reversed)
 */
#ifdef NORMAL_CALLING_CONVENTION
/* only for test/debug use */
#define SEQUENCE(x,y) x,y
#else
/* for NC30 */
#define SEQUENCE(x,y) x,y
#endif

/* Definition of Single-precision floating point macro */
#define SGL_MBSZ	5
#define SGL_CLS		1
#define	SGL_INFEXP	0xff		/* Infinity exponent */
#define	SGL_INFMANT	0L		/* Infinity mantissa */
#define	SGL_NANEXP	0xff		/* Not A Number exponent */
#define	SGL_NANMANT	0x7fffffL	/* Not A Number mantissa */
#define	SGL_UFLLMT	24		/* Under-flow limit */
#define SGL_UFLEXP	1		/* Under-flow exponent */
#define SGL_UFLMANT	0L		/* Under-flow mantissa */
#define SGL_OFLEXP	0xfe		/* Over-flow exponent */
#define SGL_OFLMANT	SGL_NANMANT	/* Over-flow mantissa */
#define SGL_MANT_MSK	0x7fffffL
#define SGL_EXP_MSK	0xff
#define SGL_MANT_MSB	0x400000L

/* Definition of Double-precision floating point macro */
#define DBL_MBSZ	9
#define DBL_CLS		2
#define	DBL_INFEXP	0x7ff		/* Infinity exponent */
#define	DBL_INFMANT	0L		/* Infinity mantissa */
#define	DBL_NANEXP	0x7ff		/* Not A Number exponent */
#define	DBL_NANMANT	0xfffffL	/* Not A Number mantissa */
#define	DBL_NANMANT2	0xffffffffL	/* Not A Number mantissa-2 */
#define	DBL_UFLLMT	53		/* Under-flow limit */
#define DBL_UFLEXP	1		/* Under-flow exponent */
#define DBL_UFLMANT	0L		/* Under-flow mantissa */
#define DBL_OFLEXP	0x7fe		/* Over-flow exponent */
#define DBL_OFLMANT	DBL_NANMANT	/* Over-flow mantissa */
#define DBL_OFLMANT2	DBL_NANMANT2	/* Over-flow mantissa-2 */
#define DBL_MANT_MSK	0xfffffL
#define DBL_MANT2_MSK	0xffffffffL
#define DBL_EXP_MSK	0x7ff
#define DBL_MANT_MSB	0x80000L

/* calculation kind */
#define	OPE_ADD		1
#define	OPE_SUB		2
#define	OPE_MUL		3
#define	OPE_DIV		4

/* Definition Single-precision floating point struct */
typedef struct {
#ifdef M16C
	unsigned long mant : 23;	/* mantissa */
	unsigned long exp  : 8;		/* exponent */
	unsigned long sign : 1;		/* sign */
#else /* 68000 */
	unsigned long sign : 1;		/* sign */
	unsigned long exp  : 8;		/* exponent */
	unsigned long mant : 23;	/* mantissa */
#endif
} SGL_FLT;

/* Definition Double-precision floating point struct */
typedef struct {
#ifdef M16C
	unsigned long mant2;		/* mantissa2 */
	unsigned long mant : 20;	/* mantissa */
	unsigned long exp  : 11;	/* exponent */
	unsigned long sign : 1;		/* sign */
#else /* 68000 */
	unsigned long sign : 1;		/* sign */
	unsigned long exp  : 11;	/* exponent */
	unsigned long mant : 20;	/* mantissa */
	unsigned long mant2;		/* mantissa2 */
#endif
} DBL_FLT;

/* mantissa union */
typedef union {
#ifdef OLD_TYPE
	long ml;
#else
	unsigned long ml;
#endif
	u_short ms[2];
	u_char mc[4];
} MANT;

/* Definition of floating point struct for calculate */
typedef struct {
	char class;	/* precision 1:Single 2:Double */
	char sign;
#ifdef OLD_TYPE
	short exp;
#else
	signed short exp;
#endif
	MANT mant;
	MANT mant2;
} FLOAT;

typedef FLOAT * FLOATP;
typedef SGL_FLT * SGL_FLTP;
typedef DBL_FLT * DBL_FLTP;

/* qualtet long ( for 128bit divide ) */
typedef union {
	u_short s[8];
	u_long l[4];
} QLONG;
typedef QLONG * QLONGP;

#ifndef UNUSED_PROTO_TYPE
/*****************************************************************************
 *  Function prototype
 ****************************************************************************/
/*******************************************
 * Single-precision floating point module
 ******************************************/

/* the four rules of arithmetic. */
float _f4add PROTO_TYPE((float sa, float sw));
float _f4sub PROTO_TYPE((float sa, float sw));
float _f4mul PROTO_TYPE((float sa, float sw));
float _f4div PROTO_TYPE((float sw, float sa));

/* comparison */
int _f4eq PROTO_TYPE((float sa, float sw));
int _f4ne PROTO_TYPE((float sa, float sw));
int _f4ge PROTO_TYPE((float sa, float sw));
int _f4gt PROTO_TYPE((float sa, float sw));
int _f4le PROTO_TYPE((float sa, float sw));
int _f4lt PROTO_TYPE((float sa, float sw));

/* convert */
float   _i4tof4 PROTO_TYPE((long i4));
float   _i4Utof4 PROTO_TYPE((u_long i4));
long    _f4toi4 PROTO_TYPE((float f4));
u_long  _f4toi4U PROTO_TYPE((float f4));
double  _f4tof8 PROTO_TYPE((float f4));
float   _i8tof4 PROTO_TYPE((s_llong i8));
float   _i8Utof4 PROTO_TYPE((u_llong i8));
s_llong _f4toi8 PROTO_TYPE((float f4));
u_llong _f4toi8U PROTO_TYPE((float f4));

/* sub-module */
void  _f4rtol PROTO_TYPE((float *r, FLOATP l));
float _f4ltor PROTO_TYPE((FLOATP l));
void  _f4mpy PROTO_TYPE((FLOATP a, FLOATP w, u_short *mp, u_short *dp));
void  _f4round PROTO_TYPE((u_char *mp));
void  _f4dvs PROTO_TYPE((u_short  *mp, u_int dp));
int   _f4cmp PROTO_TYPE((float  *sa, float *sw));

/*******************************************
 * Double-precision floating point module
 ******************************************/

/* the four rules of arithmetic. */
double _f8add PROTO_TYPE((double da, double dw));
double _f8sub PROTO_TYPE((double da, double dw));
double _f8mul PROTO_TYPE((double da, double dw));
double _f8div PROTO_TYPE((double da, double dw));

/* comparison */
int _f8eq PROTO_TYPE((double da, double dw));
int _f8ne PROTO_TYPE((double da, double dw));
int _f8ge PROTO_TYPE((double da, double dw));
int _f8gt PROTO_TYPE((double da, double dw));
int _f8le PROTO_TYPE((double da, double dw));
int _f8lt PROTO_TYPE((double da, double dw));

/* convert */
double  _i4tof8 PROTO_TYPE((long i4));
double  _i4Utof8 PROTO_TYPE((u_long i4));
long    _f8toi4 PROTO_TYPE((double f8));
u_long  _f8toi4U PROTO_TYPE((double f8));
float   _f8tof4 PROTO_TYPE((double f8));
double  _i8tof8 PROTO_TYPE((s_llong i8));
double  _i8Utof8 PROTO_TYPE((u_llong i8));
s_llong _f8toi8 PROTO_TYPE((double f8));
u_llong _f8toi8U PROTO_TYPE((double f8));

/* sub-module */
void   _f8rtol PROTO_TYPE((double *r, FLOATP l));
double _f8ltor PROTO_TYPE((FLOATP l));
void   _f8addmant PROTO_TYPE((FLOATP a, FLOATP w));
void   _f8mpy PROTO_TYPE((FLOATP a, FLOATP w, u_short  *mp, u_long *dp));
void   _f8round PROTO_TYPE((u_char  *mp));
void   _f8dvs PROTO_TYPE((u_short  *mp, u_long *dp));
int    _f8cmp PROTO_TYPE((double  *da, double *dw));

/*********************************
 *  Single/Double common module
 ********************************/

int _chkdata PROTO_TYPE((FLOATP a, FLOATP w, int ope));
int _cmpexp PROTO_TYPE((FLOATP a, FLOATP w, int  *ds));
void _ngt PROTO_TYPE((FLOATP l));
void _asr PROTO_TYPE((FLOATP l, int  *cf));
void _nor PROTO_TYPE((FLOATP l));

#endif	/* UNUSED_PROTO_TYPE */
#endif	/* _LIBUSE_DEF */
#endif	/* LIB_USE */
#endif	/* __FLOAT_H */
/*******************************************************************************
*
* float.h -- Version 2.00.00
*
* Copyright(C) 1999(2000-2004). Renesas Technology Corp.
* and Renesas Solutions Corp., All rights reserved.
*
*
*******************************************************************************/
