/*******************************************************************************
*
* _print 
*
* Copyright(C) 1999(2000-2004). Renesas Technology Corp.
* and Renesas Solutions Corp., All rights reserved.
*
*
* $Date: 2005/10/14 02:00:51 $
* $Revision: 1.26 $
*
*
*     : Version 0.00.00
*	    Version 2.00.00
*		%n        %g ANSI      
*		0         ANSI       
*		g     ANSI            
*		                (%G   )
*
*
*        _print --               
*
*        format = _print( func, format, arg, total );
*
*           #include <ctype.h>
*           int  (*func)();                 
*           char *format;                   
*           union {                           
*               int  *i_p;          int      d,i,o,u,x,X 
*               long *l_p;          long      l? 
*               long long *ll_p;    long long      ll? 
*               double *d_p;        double     (e,E,f,g,G)
*               int  **i_pp;        int           n 
*               long **l_pp;        long           ln 
*               long long **ll_pp;  long long           lln 
*               char **c_pp;        char           s,p 
*           }*arg;                             
*           int  *total;                            
*
*   format                           []       
*
*            %[FLAGS][WIDTH][.PRECISION][l or h]TYPE
*            %-05.8ld
*
*
*                     
*
*         [FLAGS] 
*
*             ----                           
*                              
*             ----                     
*                                  
*       blank' '  ----                         
*                             blank' '     
*       #           ----  o             x X          
*                          0x 0X     
*                        e E f              
*                        g G                        
*
*              [WIDTH] 
*
*                                          
*                                          
*                     blank' '                 
*                   '0'          
*                                         
*                   blank' '    
*                                          
*                                          
*                 
*
*        [.PRECISION] 
*
*       '.'              '.'                    
*                                
*
*        d i o u x X     
*
*                                       '0'  
*                                          
*                                          
*                              
*                                           
*
*        s     
*
*                                          
*                           
*
*         e E f     
*
*             n        
*
*        g G     
*       n               
*
*                                          
*                               
*
*                 
*
*            d i o u x X n   long int   u_long int   
*              
*
*             d i o u x X n   
*       long long int   unsigned long long int          
*
*            d i o u x X n   short int   u_short int  
*               
*
*              d i o u x X n             
*                
*
*            e E f g G   long double   
*              C77V3.00   long double double         
*              double         
*
*         TYPE 
*
*       d i --                    
*       u   --                    
*       o   --                   
*       x   --                    
*                 0AH 0FH     "abcdef"    
*       X   --                    
*                 0AH 0FH     "ABCDEF"    
*       e     --  double                  
*                 [-]d.dddddde+/-dd                      
*                               (     )      
*                                  
*       E     --  e              e     E    
*       f     --  double    
*                 [-]d.dddddd          
*                                      
*                           (     )      
*                                  
*       g     --  double    e   f             
*                    f        e          -4       
*                                   
*       G     --   g      E   f             
*       c   --           ASCII        
*       s   --                  '\0'         
*                         
*       p   --                               
*                                    00:1205 
*
*       n    -- n                  n           
*                                              
*                      n    int       
*                           l    long       
*                           ll    long long      
*                                   
*
*
*                                         
*                               
*
*        format       arg               buff   
*              
*
*******************************************************************************/


#include <one_net/port_specific/config_options.h>


#ifdef _ONE_NET_EVAL
    #pragma section program program_high_rom
#endif // ifdef _R8C_TINY //


// This was copied from the nc30 lib source directory.  Needed to go above
// 48k, so had to use the nc30lib.lib.  This caused a few function variables
// to be declared static, which used too much ram.  A beneficial side effect
// is that this also saves several k of ROM.

/*****************************************************************************
 *           
 ****************************************************************************/
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#define LIB_USE
#include <float.h>
/*****************************************************************************
 *          
 ****************************************************************************/
/**       **/
#define M_LEFT		0x0001
#define M_SIGN		0x0002
#define M_SHRP		0x0004
#define M_BLNK		0x0008
#define M_MINUS		0x0010
#define M_EFUGO		0x0020
#define M_G		0x0040
#define M_EFG		0x0080
#define M_FUGOU		0x0100
#define M_GONYU		0x0200
#define M_KETA		0x0400
/**         **/
#define CHK_LEFT	(flag & M_LEFT)
#define CHK_SIGN	(flag & M_SIGN)
#define CHK_SHRP	(flag & M_SHRP)
#define CHK_BLNK	(flag & M_BLNK)
#define CHK_MINUS	(flag & M_MINUS)
#define CHK_EFUGO	(flag & M_EFUGO)
#define CHK_G		(flag & M_G)
#define CHK_EFG		(flag & M_EFG)
#define CHK_FUGOU	(flag & M_FUGOU)
#define CHK_GONYU	(flag & M_GONYU)
#define CHK_KETA	(flag & M_KETA)
/**        **/
#define SET_LEFT	(flag |= M_LEFT)
#define SET_SIGN	(flag |= M_SIGN)
#define SET_SHRP	(flag |= M_SHRP)
#define SET_BLNK	(flag |= M_BLNK)
#define SET_MINUS	(flag |= M_MINUS)
#define SET_EFUGO	(flag |= M_EFUGO)
#define SET_G		(flag |= M_G)
#define SET_EFG		(flag |= M_EFG)
#define SET_FUGOU	(flag |= M_FUGOU)
#define SET_GONYU	(flag |= M_GONYU)
#define SET_KETA	(flag |= M_KETA)
/**        **/
#define CLR_LEFT	(flag &= ~M_LEFT)
#define CLR_SIGN	(flag &= ~M_SIGN)
#define CLR_SHRP	(flag &= ~M_SHRP)
#define CLR_BLNK	(flag &= ~M_BLNK)
#define CLR_MINUS	(flag &= ~M_MINUS)
#define CLR_EFUGO	(flag &= ~M_EFUGO)
#define CLR_G		(flag &= ~M_G)
#define CLR_EFG		(flag &= ~M_EFG)
#define CLR_FUGOU	(flag &= ~M_FUGOU)
#define CLR_GONYU	(flag &= ~M_GONYU)
#define CLR_KETA	(flag &= ~M_KETA)

#define STRMAX	30
#define TRUE	1
#define ZERO	0L
#define OCT	8L	/* 8          */
#define DEC	10L	/* 10         */
#define HEX	16L	/* 16         */
#define ZERO_LL	0LL
#define OCT_LL	8LL	/* 8         (long long ) */
#define DEC_LL	10LL	/* 10         (long long ) */
#define HEX_LL	16LL	/* 16         (long long ) */
#define LONG_LONG 2	/*      ll             */
#define FALSE	0

/**           */
#define BFS 64
#define DBM 4
#define DBS BFS+DBM
#define BLEN 16
#define DIV (DTYP)10000
#define BMSK (DTYP)0x8000
#define DMSK (DTYP)0xffff
#define UNIT 4
#define DIG 6
#define EMAX 4

typedef u_short DTYP;
typedef union ptr {
	int  *i_p;	/* int      */
	long *l_p;	/* long      */
	long long *ll_p;	/* long long      */
	double *d_p;	/* double      */
	int  **i_pp;	/* int           */
	long **l_pp;	/* long           */
	long long **ll_pp;	/* long long           */
	char **c_pp;	/* char           */
} UNIONPTR;


/*****************************************************************************
 *          
 ****************************************************************************/
int _f8prn( int(*)(int), double, int, char,  int, char, int, int *, u_int) ;
void henkan1( double *, FLOATP );
double henkan2( FLOATP );
int _pri( int(*)(int), char, char *, char *, char *, int, int, int, int, int, u_int );
void _f8geti( DTYP *, FLOATP );
int _inmod( DTYP *, DTYP *, DTYP, int * );

const char * _local_print( int (*func)(int), const char *format,
  int **ap, int *total )
{
	int i, j;
	UNIONPTR *arg;
	u_int flag;
	unsigned char p_flg;	/*       			*/
	int lh_flg;		/* long     short int    	*/
	char pad;		/*      '0' ' '     	*/
	int width;		/*               	*/
	int pres;		/*               	*/
	long get;		/*            	*/
	long long get_ll;	/*            (long long ) */
#if !defined (__R8C__) && !defined(__NO_FP__)
	double db;		/*       (      )	*/
#endif
	u_long data = 0L;		/*       			*/
	unsigned long long data_ll;	/*       (long long ) */
	char str[STRMAX+1];	/*           		*/
	char hex_char;		/*     0xa      	*/
	char *top = 0;		/*               	*/
	int num;		/*            	*/
	int t_num;		/*    0x 0X     	*/
	int cnt = 0;		/* 10   			*/
	int ret;		/*        		*/
#if !defined (__R8C__) && !defined(__NO_FP__)
	DBL_FLTP dp = (DBL_FLTP)&db;	/*           	*/
#endif

	/********************************************
	*
	*               
	*
	********************************************/

	/* (*func)()                 */
	_asm("	.call	$_pc,G");
	_asm("	.call	$_sp,G");
	_asm("	.call	$_fp,G");

	arg = (UNIONPTR *)ap;
	flag = lh_flg = 0;
	while ( *format == '-' || *format == '+'
         || *format == ' ' || *format == '#' ) {
		/*         */
		switch ( *format++ ) {
		case '-': SET_LEFT;
			  break;
		case '+': SET_SIGN;
			  break;
		case '#': SET_SHRP;
			  break;
		case ' ': SET_BLNK;
			  break;
		}
	}

	/********************************************
	*
	*                
	*
	********************************************/

	pad = ' ';			/*          */
	width = 0;			/*              */
	if ( isdigit(*format) ) {	/*             */
		if ( *format == '0' ) {
			/*             '0'     */
			format++;
			pad = '0';
		}
		if( *format == '*' ){
                        /* 0      '*'      */
                        format++;
                        width = *(arg->i_p)++;          /*           */
                        if ( width < 0 ) {
                                /*        */
                                SET_LEFT;               /*          */
                                width = -width;         /*           */
                        }
		}
		else{
			/*         */
			while ( isdigit( *format ) )
				width = ( width * 10 ) + ( *format++ & 0xf );
		}
	} else if ( *format == '*' ) {
		/* '*'        */
		format++;
		width = *(arg->i_p)++;		/*           */
		if ( width < 0 ) {
			/*        */
			SET_LEFT;		/*          */
			width = -width;		/*           */
		}
	}

	/********************************************
	*
	*          
	*
	********************************************/

	pres = -1;			/*        */
	if ( *format == '.' ) {		/*         */
		format++;
		if ( *format == '*' ) {
			/* '*'        */
			format++;
			pres = *(arg->i_p)++;	/*           */
			if ( pres < 0 )		/*        */
				pres = -1;	/*             */
		} else {
			pres = 0;
			while ( isdigit( *format ) )
				pres = ( pres * 10 ) + ( *format++ & 0xf );
		}
	}

	/********************************************
	*
	*            
	*
	********************************************/

	if ( *format == 'l' || *format == 'h' ){
		/*             */
		lh_flg = *format++;
		if ( (*(format - 1) == 'l') && (*format == 'l') ){
			/*          */
			lh_flg = LONG_LONG;
			format++;
		}
	}
	else if ( *format == 'L' )
		/* L           */
		*format++;

	/********************************************
	*
	*          
	*
	********************************************/

	/*
	 * "%g"   "%f"                       
	 */
	p_flg = *format;

	/*           */
	switch ( p_flg ) {
	case 'd':
	case 'i':
		if ( lh_flg == 'l' ) {
			/*    long    */
			get = *(arg->l_p)++;
			if ( get < 0L ) {
  				SET_MINUS;
   				get = -get;
  			}
   			data = ( u_long )( get );
		} else if ( lh_flg == LONG_LONG ) {
			/*    long long     */
			get_ll = *(arg->ll_p)++;
			if ( get_ll < 0LL ) {
				SET_MINUS;
				get_ll = -get_ll;
			}
			data_ll = ( unsigned long long )( get_ll);
		} else {
			get = ( long )( *(arg->i_p)++ );
			if ( get < 0L ) {
  				SET_MINUS;
   				get = -get;
  			}
   			data = ( u_long )( get ) & 0xffffL;
		}
		break;
	case 'u':
	case 'o':
	case 'x':
	case 'X':
		if ( lh_flg == 'l' )
			/*    long    */
			data = ( u_long )( *(arg->l_p)++ );
		else if ( lh_flg == LONG_LONG )
			/*    long long     */
			data_ll = ( unsigned long long )( *(arg->ll_p)++ );
		else
			data = ( u_long )( *(arg->i_p)++ ) & 0xffffL;
		break;
#if !defined (__R8C__) && !defined(__NO_FP__)
	case 'e':
	case 'E':
	case 'f':
		db = *(arg->d_p)++ ;
		if (dp->sign) {
			/* db       */
  			SET_MINUS;
			dp->sign = 0;
		}
		break;
	case 'g':
	case 'G':
		db = *(arg->d_p) ;	/*              */
		if (dp->sign) {
			/* db       */
  			SET_MINUS;
			dp->sign = 0;
		}
		break;
#endif
	}

	/********************************************
	*
	*       0      ANSI   
	*
	********************************************/

	switch ( p_flg ) {
	case 'd':
	case 'i':
	case 'o':
	case 'u':
	case 'x':
	case 'X':
		if ( pres != -1 || (CHK_LEFT) )
			pad = ' ';
	}
	/********************************************
	*
	*                
	*
	********************************************/

	i = STRMAX; /*                */
	t_num = 0; /*    0x 0X        */

	/*           */
	switch ( p_flg ) {
	case 'd':
	case 'i':
	case 'u':
		if ( lh_flg == LONG_LONG ){
			/* long long     */
			if ( pres == 0 && data_ll == ZERO_LL ) {
				/*              (LL) */
				num = 0;
				break;
			}
			/*         */
			do {
				str[i--] = ( char )( data_ll % DEC_LL ) + '0';
								/* (LL) */
				data_ll /= DEC_LL;		/* (LL) */
			} while ( data_ll != ZERO_LL );		/* (LL) */
		}else{
			if ( pres == 0 && data == ZERO ) {
				/*              (L) */
				num = 0;
				break;
			}
			/*         */
			do {
				str[i--] = ( char )( data % DEC ) + '0';
								/* (L) */
				data /= DEC;			/* (L) */
			} while ( data != ZERO );		/* (L) */
		}
		num = STRMAX - i;
		while ( num < pres ) {
			/*               */
			str[i--] = '0';	/*   '0'    */
			num++;
		}
		if ( CHK_MINUS || CHK_SIGN || CHK_BLNK ){
			/*                */
			num++;	/*        */
			t_num = 1;
		}
		if ( CHK_MINUS ) /*      */
			str[i] = '-';
		else if ( CHK_SIGN ) /*          */
			str[i] = '+';
		else if ( CHK_BLNK ) /* blank        */
			str[i] = ' ';
		else
			i++;
		top = &str[i];	/*             */
		break;
	case 'x':
	case 'X':
		if ( lh_flg == LONG_LONG ){
			/* long long     */
			if ( pres == 0 && data_ll == ZERO_LL ) {
				/*             (LL) */
				num = 0;
				break;
			}
			/*            */
			if ( p_flg == 'X' )
				hex_char = 'A';
			else
				hex_char = 'a';
			if ( !data_ll )
				/*     0     #          */
				CLR_SHRP;
			/*         */
			do {
				str[i] = ( char )( data_ll % HEX_LL );
								/* (LL) */
				if( str[i] >= 0xa )
					str[i] = str[i] - 0xa + hex_char;
				else
	   				str[i] = str[i] + '0';
				i--;
				data_ll /= HEX_LL;		/* (LL) */
			} while ( data_ll != ZERO_LL );		/* (LL) */
		}else{
			if ( pres == 0 && data == ZERO ) {
				/*             (L) */
				num = 0;
				break;
			}
			/*            */
			if ( p_flg == 'X' )
				hex_char = 'A';
			else
				hex_char = 'a';
			if ( !data )
				/*     0     #          */
				CLR_SHRP;
			/*         */
			do {
				str[i] = ( char )( data % HEX );
								/* (L) */
				if( str[i] >= 0xa )
					str[i] = str[i] - 0xa + hex_char;
				else
		   			str[i] = str[i] + '0';
				i--;
				data /= HEX;			/* (L) */
			} while ( data != ZERO );		/* (L) */
		}
		num = STRMAX - i;
		if ( CHK_SHRP ) {
			/*         (L) */
			num += 2;
			t_num = 2;
		}
		while ( num < pres ) {
			str[i--] = '0';
			num++;
		}
		if ( CHK_SHRP ) {
			/*         (L) */
			str[i--] = p_flg;
			str[i] = '0';
		} else 
			i++;
		top = &str[i];
		break;
	case 'o':
		if ( lh_flg == LONG_LONG ){
			/* long long     */
			if ( pres == 0 && data_ll == ZERO_LL ) {
				/*             (LL) */
				num = 0;
				break;
			}
			if ( !data_ll )
				/*     0     #          */
				CLR_SHRP;
			/*        */
			do {
				str[i--] = ( char )( data_ll % OCT_LL ) + '0';
								/* (LL) */
				data_ll /= OCT_LL;		/* (LL) */
			} while ( data_ll != ZERO_LL );		/* (LL) */
		}else{
			if ( pres == 0 && data == ZERO ) {
				/*             (L) */
				num = 0;
				break;
			}
			if ( !data )
				/*     0     #          */
				CLR_SHRP;
			/*        */
			do {
				str[i--] = ( char )( data % OCT ) + '0';
								/* (L) */
				data /= OCT;			/* (L) */
			} while ( data != ZERO );		/* (L) */
		}
		num = STRMAX - i;
		if ( CHK_SHRP )			
			/*          */
			num++;
		while ( num < pres ) {
			str[i--] = '0';
			num++;
		}
		if ( CHK_SHRP )
			/*          */
			str[i] = '0';
		else
			i++;
		top = &str[i];
		break;
#if !defined (__R8C__) && !defined(__NO_FP__)
	case 'e':
	case 'E':
	case 'f':
	case 'g':
	case 'G':
		/*     */
		ret = FALSE;
		SET_EFG;
		if (dp->exp == DBL_INFEXP) {
			/* Infinity            */
			char *p = "-Infinity";
			if (!(CHK_MINUS))
				p++;
			while (*p != '\0') {
				if ((*func)((int)(*p++)) == EOF)
					return  (char *)FALSE;
				*total += 1;
			}
			if ( p_flg == 'g' || p_flg == 'G' )
				*(arg->d_p)++; 
			break;
		}
		if ( pres == 0 && db == 0.0 ){
			/*              */
			num = 0;
		}
		if( pres == -1 )
			pres = 6;	/*        */
		if ( p_flg != 'f' && db != 0.0 ) {
			if ( db < 1.0 ) {
				/* 0.ddd...   */
		 		while ( db < 1.0 ) {
					db = db * 10;
					cnt++;	/* 10        */
		 		}
		 		SET_EFUGO; /*           */
			} else {
				/* ddd.dd....   */
		 		while ( db >= 10.0 ) {
					db = db / 10;
					cnt++;
		 		}
		 		CLR_EFUGO; /*           */
			}	
		}
		if ( p_flg == 'g' || p_flg == 'G' ) {
			SET_G;	/* %g        */
			if( pres == 0 )
				pres = 1;
			/* 10    -4            %e  
			        %f      */
			if ( (CHK_EFUGO && cnt > 4) || (!CHK_EFUGO && cnt >= pres ) ){
		 		if ( p_flg == 'g' )
					/* e     */
					p_flg = ( char )'e';
		 		else if ( p_flg == 'G' )
					/* E    */
					p_flg = ( char )'E';
		 		pres -= 1;	/* %g pres      */
			} else {
				/* f    */
		 		p_flg = ( char )'f';
		 		db = *(arg->d_p) ;			
				if (dp->sign)
					dp->sign = 0;	/* db       */
				if ( !CHK_EFUGO ) /* |db| > 1.0 */
					pres -= (cnt + 1); /* %g pres      */
				else if (CHK_EFUGO)  /* |db| < 1.0 */
					pres -= ( (-1)*cnt + 1 );
			}
			*(arg->d_p)++ ; 
		}
		/*            */
		_asm("	.call	$_pc,G");
		_asm("	.call	$_sp,G");
		_asm("	.call	$_fp,G");
		if (ret = _f8prn(
		   func, db, pres, p_flg, cnt, pad, width, total, flag))
			return  (char *)FALSE;
		break;
#endif
	case 'c':
		pad = ' ';	/* c        blank */
		str[i] = *(arg->i_p)++;
		num = 1;
		top = &str[i];
		break;
	case 'p':
		pad = ' ';	/* p        blank */
		data = ( u_long )( *(arg->c_pp) );
		data &= 0xffffL;	/*               */
		for ( j = 0 ; j < 4 ; j++ ) {
			str[i] = ( char )( data & 0xf );	/* (L) */
			if( str[i] >= 0xa )
				str[i] = str[i] - 0xa + 'A';
			else
				str[i] = str[i] + '0';
			i--;
			data /= HEX;
		}
		str[i--] = ':';	/*           */
		data = ( u_long )( *(arg->c_pp)++ );
		data = ( data >> 16 ) & 0xff;	/*          (L) */
		for ( j = 0 ; j < 2 ; j++ ) {
			str[i] = ( char )( data & 0xf );	/* (L) */
			if( str[i] >= 0xa )
				str[i] = str[i] - 0xa + 'A';
			else
				str[i] = str[i] + '0';
			i--;
			data /= 16;
		}
		num = STRMAX - i;
		top = &str[i+1];
		break;
	case 'n':
		if( lh_flg == 'l' ) {
			**(arg->l_pp) = *total;
			*(arg->l_pp)++ ;	
		} else if ( lh_flg == LONG_LONG ) {
			**(arg->ll_pp) = *total;
			*(arg->ll_pp)++;
		} else{
			**(arg->i_pp) = *total;
			*(arg->i_pp)++;
		}
		num = 0;
		return ( format + 1 );
	case 's':
		pad = ' ';	/* s        blank */
		top = *(arg->c_pp)++;
		for ( num = 0 ; num != pres && top[num] ; num++ ) ;
		break;
	default:
		return format;
	}

	/********************************************
	*
	*             
	*
	********************************************/

	if ( !CHK_EFG ) {
	    /* %e,E,f,g,G        */
	    if ( num < width ) {
		/*              */
		if ( CHK_LEFT ) {
			/*        */
			for ( i = 0 ; i < num ; i++ ) {
				if ( ( *func )( (int)(*top++) ) == EOF )
					return  ( char * )FALSE;
			}
			for ( i = 0 ; i < ( width - num ) ; i++ ) {
				if ( ( *func )( ' ' ) == EOF )
					return  ( char * )FALSE;
			}
		} else {
			if ( t_num && pad == '0' ) {
				/*    0x 0X   0       */
				for ( i = 0 ; i < t_num ; i++ ) {
					if ( ( *func )( (int)(*top++) ) == EOF )
						return  ( char * )FALSE;
				}
				for ( i = 0 ; i < ( width - num ) ; i++ ) {
					if ( ( *func )( '0' ) == EOF )
						return  ( char * )FALSE;
				}
				for ( i = 0 ; i < ( num - t_num ) ; i++ ) {
					if ( ( *func )( (int)(*top++) ) == EOF )
						return  ( char * )FALSE;
				}
			} else {
				for ( i = 0 ; i < ( width - num ) ; i++ ) {
					if ( ( *func )( (int)pad ) == EOF )
						return  ( char * )FALSE;
				}
				for ( i = 0 ; i < num ; i++ ) {
					if ( ( *func )( (int)(*top++) ) == EOF )
						return  ( char * )FALSE;
				}
			}
		}
		num = width;
	    } else {
		for ( i = 0 ; i < num ; i++ ) {
			if ( ( *func )( (int)(*top++) ) == EOF )
				return  ( char * )FALSE;
		}
	    }
	    *total += num;
	}
	return ( format + 1 );
}

/*****************************************************************************
 * @(#)     : _f8prn
 *       : F8                        
 *      : int
 *       : int (*func)() :            
 *	   : double db     :        F8   
 *	   : int pres      :   
 *	   : char p_flg    :       
 *	   : int cnt       :      
 *	   : char pad      :        
 *	   : int width     :       
 *	   : int *total    :                
 *	   : u_int flag    :    
 *       : 1992/07/30   
 *****************************************************************************/

#if !defined (__R8C__) && !defined(__NO_FP__)
int _f8prn( int (*func)(int), double db, int pres, char p_flg,
	int cnt, char pad, int width, int *total, u_int flag )
{
#ifdef NC30
	static DTYP b[DBS];
	static char inte[DBL_MAX_10_EXP+1];	/*           */
	static char deci[DBL_MAX_10_EXP+1];	/*           */
#else
	DTYP b[DBS];
	char inte[DBL_MAX_10_EXP+1];	
	char deci[DBL_MAX_10_EXP+1];
#endif
	DTYP sp;
	FLOAT f;
	char estr[EMAX+1];		/* %e,E      */
	int x, y, i, seisu,r;		/*      */
	int size;
	int last;			/* %g          */
	int strcnt;			/*       */
	int unit;			/*               */

	/* (*func)()             */
	_asm("	.call	$_pc,G");
	_asm("	.call	$_sp,G");
	_asm("	.call	$_fp,G");

	last = strcnt = FALSE;
	henkan1(&db,&f);		/* _f8rtol(&db, &f) */
	if (f.exp == DBL_INFEXP) {
		/*           */
		return 1;
	}

	/*           */
	for (r=0; r < DBL_MAX_10_EXP+1; )
		inte[r++] = '0';		/*         */

	for (r=0; r < DBL_MAX_10_EXP+1; )
			deci[r++] = '0';	/*         */

	/*        */
	_f8geti(b, &f);			/* input  | f:          */
					/* output | b:     f:     */

	/*           */
	db = henkan2(&f);				/* _f8ltor(&f) */
	for ( r = 0; r < pres+1; r++ )		/*   +1          */
		db *= (double)10;
	henkan1(&db,&f);			/* _f8rtol(&db, &f) */
		/* f:                         */

	/*                  inte     */
	for (size=BFS, seisu=0; ; ) {
		/*      sp = b[] % DIV */
		/* output | b:   */
		if (!_inmod(b, &sp, DIV, &size))
			break;
		for (y = 0; y < UNIT; y++) {
		/*     4             */
			inte[seisu++] = (char)( (sp % (DTYP)10) + '0');
			sp /= (DTYP)10;
		}
	}

	/*        */
	_f8geti(b, &f);
				/* output | b:              */
	for (size=BFS, x=0; ; ) {
		/*      sp = b[] % DIV */
		if (!_inmod(b, &sp, DIV, &size))
			break;

		/*   +1          */
		if ( !x ) {
			if ( ( sp % (DTYP)10) >= 5 )
				SET_GONYU;		/*         */
			sp /= (DTYP)10;			/*         */
			unit = UNIT - 1;		/*      */
		} else
			unit = UNIT;		
		/* UNIT  10    */
		for (y = 0; y < unit; y++) {
			deci[x++] = (char)( (sp % (DTYP)10) + '0');
			sp /= (DTYP)10;
		}
	}
	/*          */
	if ( CHK_GONYU ) {
		for ( r=0; r<pres; r++ ){
			if ( deci[r] == '9' )
				deci[r] = '0';
			else {
				++deci[r];
				break;
			}
		}
		if ( r==pres )		/*              */
			SET_KETA;
	}			
	/* #       %g,G                */
	if ( CHK_G && !CHK_SHRP ) {   
		for ( last=0; last<=x; last++ ){
			if ( deci[last] != '0' )
				break;
		}
		if ( (last - 1) == x ) {	/*             */
			x = last = 0;
		}
	}
	strcnt += (pres - last );		/*         */

	/*                      */
	if ( CHK_KETA ) {
		if ( (p_flg == 'e' || p_flg == 'E') && inte[0]=='9' ) {
		/* %e       9    */
			inte[0] = '1';		/*          */
			if ( CHK_EFUGO ) {
				/*             */
				cnt--;
				if ( !cnt )
					/*                  */
					CLR_EFUGO;
			} else
				cnt++;
		} else {
			for ( r=0; r<seisu; r++ ) {
				if ( inte[r] == '9' ) 
					inte[r] = '0';
				else {
					++inte[r];
					break;
				}
			}
			if ( r==seisu && r!=0) {
				inte[seisu] = '1';
				seisu++;
			}
			else if(seisu == 0){
				inte[seisu] = '1';
				seisu++;
			}
		}
	}	
			 
	if (seisu) {
		/* 0       */
		for (--seisu; seisu >= 0; seisu--) {
			if (inte[seisu] != '0')
				break;
		}
	}
	strcnt += (seisu + 1);		/*         */
	/*               */
	if ( CHK_MINUS || CHK_SIGN || CHK_BLNK ){
		strcnt ++;
		SET_FUGOU;
	}
	if ( CHK_MINUS )
		inte[++seisu] = '-';
	else if ( CHK_SIGN )
		inte[++seisu] = '+';
	else if ( CHK_BLNK )
		inte[++seisu] = ' ';

	/*                */
	if (pres>0 && CHK_G && x== 0 && (!CHK_SHRP));
		/*          g                  */
	else if ( pres>0 || (pres<=0 && CHK_SHRP))
		strcnt += 1;

	if ( p_flg == 'e' || p_flg == 'E' ) {
		/* +/-dd */
		i = EMAX ;
		do {
			estr[i--] = ( char )( cnt % 10 ) + '0';
			cnt /= 10;
		} while ( cnt != 0 );
		if ( i == EMAX - 1 )
			estr[i--] = '0';
		if ( CHK_EFUGO )
			estr[i--] = '-';
		else
			estr[i--] = '+';
		if ( p_flg == 'e' )
			estr[i] = 'e';	/* d.dd...e+dd */
		else if ( p_flg == 'E' )
			estr[i] = 'E';	/* d.dd...E+dd */
		strcnt += (EMAX - i + 1); /*       e+dd      */
   	 } 	

	/*************************
	*    
	**************************/

	if ( strcnt < width ) {
		if ( CHK_LEFT ){
			/*    */
			if ( _pri(func, p_flg, inte, deci, estr,
			    seisu, x, last, i, pres, flag) )
				return  1;	/*           */
			for ( x = 0; x < (width-strcnt); x++ ) {
				/*          */
				if ( (*func)(' ') == EOF )
					return 1;
			}
		} else {
			if ( (CHK_FUGOU) && pad == '0' ) {
				if ( ( *func )( (int)inte[seisu--] ) == EOF )
					/*      */
					return  1;
				for ( x=0; x < (width-strcnt); x++ ) {
					/*         */
					if ( (*func)('0') == EOF )
					return 1;
				}
				/*    */
				if ( _pri(func, p_flg, inte, deci, estr,
				    seisu, x, last, i, pres, flag) )
					return  1; /*           */
			} else {
				for ( x=0; x < (width-strcnt) ;x++ ) {
					/*          */
					if( (*func)( (int)pad ) == EOF ) 
						return 1;
				}
				/*    */
 				if ( _pri(func, p_flg, inte, deci, estr, seisu,
				    x, last, i, pres, flag) )
					return  1; /*           */
			}
		}
		strcnt = width;
	} else {
		/*    */
		if ( _pri(func, p_flg, inte, deci, estr, seisu,
		    x, last, i, pres, flag) )
			return  1;	/*           */
	}
	*total += strcnt;
	return 0;
}
#endif	//__R8C__


#if !defined (__R8C__) && !defined(__NO_FP__)

/*****************************************************************************
 * @(#)     : henkan1  (_f8rtol.c   )
 *           :                           (   )
 *          : void
 *           : double * db : (I)                   
 *             : FLOATP f  : (O)                   
 *           : 1991/11/22   
 *****************************************************************************/
void henkan1( double * db, FLOATP f )
{
        DBL_FLTP real;
 
        real = (DBL_FLTP)db;
        f->class = DBL_CLS;
        f->sign = real->sign;
        f->exp = real->exp;
        f->mant.ml = real->mant;
        f->mant2.ml = real->mant2;
}

#endif
/*****************************************************************************
 * @(#)     : henkan2  (_f8ltor.c   )
 *           :                           
 *                       (   )
 *          : double :                
 *           : FLOATP f : (I)                   
 *           : 1991/11/22   
 *****************************************************************************/
#if !defined (__R8C__) && !defined(__NO_FP__)
double henkan2( FLOATP f )
{
        DBL_FLT r;      /*               */
        double *p;
 
        r.sign = f->sign;
        r.exp = f->exp;
        r.mant = f->mant.ml;
        r.mant2 = f->mant2.ml;
        p = (double *)&r;
        return *p;
}
#endif
/*****************************************************************************
 * @(#)     : pri
 *       :              
 *      : int
 *       : int (*func)() :            
 *	   : char p_flg    :       
 *	   : char *inte    :               
 *	   : char *deci    :               
 *	   : char *estr    : %e                
 *	   : int seisu     :               
 *	   : int x	   :               
 *	   : int last	   : %g           
 *	   : int i	   : %e                
 *	   : int pres	   :   
 *	   : u_int flag    :    
 *      :  I.MAYUMI
 *       : 1992/08/20   
 *****************************************************************************/
#if !defined (__R8C__) && !defined(__NO_FP__)
int _pri( int (*func)(int), char p_flg, char *inte, char *deci,
	char *estr,int seisu, int x, int last, int i,  int pres, u_int flag)
{
	/* (*func)()             */
	_asm("	.call	$_pc,G");
	_asm("	.call	$_sp,G");
	_asm("	.call	$_fp,G");

	/*       */
	if (seisu || (!seisu && inte[0] != '0') ) {
		for (; seisu >= 0; seisu--) {
			if ( ( *func )( (int)inte[seisu] ) == EOF )
				return  1;
		}
	} else if ( CHK_KETA ) {
		if ( ( *func )( '1' ) == EOF )
			return  1;
	} else if ( ( *func )( '0' ) == EOF )
		return  1;

	/*        */
	if ( pres>0 && CHK_G && x==0 && (!CHK_SHRP) ){
		/*          g                  */
		return 0;	/*     (        ) */
	}else if ( pres>0 || (pres<=0 && CHK_SHRP) ) {
		if ( ( *func )( '.' ) == EOF )
			return  1;
	}

	/*       */
	if ( pres > 0 ) {
		for (x = pres-1; x >= last; x--) {
			if ( ( *func )( (int)deci[x] ) == EOF )
				return  1;
		}
	}
  	if ( p_flg == 'e' || p_flg == 'E' ) {/* +/-dd */
		for ( ; i<=EMAX ; i++ ) {
			if ( (*func)( (int)(estr[i]) ) == EOF )
				return 1;
		}
   	} 
	return  0;	/*        */
}
#endif
/*****************************************************************************
 * @(#)     : _f8geti
 *       :                  
 *      : void
 *       : INT *b   : (O)            
 *	   : FLOATP f : (I)                
 *	              :	(O)        
 *       : 1992/07/30   
 *****************************************************************************/
#if !defined (__R8C__) && !defined(__NO_FP__)
void _f8geti( DTYP *b, FLOATP f )
{
	int x;
	DTYP c1, c2, m;
	u_long cl1, cl2;
	int j, k;

	for (x = 0; x < DBS; )
		b[x++] = 0;

	/* 0.0     1.0       0       */
	if ((!f->exp) && (!f->mant.ml) && (!f->mant2.ml))
		return;
	if (f->exp < 0x3ff)
		return;

	/*       MSB        */
	b[BFS-1] = 1L;
	/*          */
	b[BFS+0] = (DTYP)((f->mant.ml & 0xffff0L) >> 4);
	b[BFS+1] = (DTYP)((f->mant.ml & 0xfL) << 12);
	b[BFS+1] |= (DTYP)((f->mant2.ml & 0xfff00000L) >> 20);
	b[BFS+2] = (DTYP)((f->mant2.ml & 0xffff0L) >> 4);
	b[BFS+3] = (DTYP)((f->mant2.ml & 0xfL) << 12);

	/*       */
	/*               */
	j = (f->exp - 0x3ff) / BLEN;
	if (j) {
		for (x=0; x+j < DBS; x++)
			b[x] = b[x+j];
		for (; x < DBS; x++)
			b[x] = 0;
		f->exp -= (j * BLEN);
	}
	/*              */
	j = f->exp - 0x3ff;
	f->exp = 0x3ff;
	if (j) {
		k = BLEN - j;
		m = DMSK << k;
		for (c1 = 0, x = DBS-1; x >= 0; x--) {
			c2 = ((DTYP)(b[x] & m) >> k);
			b[x] <<= j;
			b[x] |= c1;
			c1 = c2;
		}
	}

	/*       */
	cl2 = (u_long)b[BFS+3] >> 12;
	cl2 |= (u_long)b[BFS+2] << 4;
	cl2 |= (u_long)b[BFS+1] << 20;
	cl1 = (u_long)b[BFS+1] >> 12;
	cl1 |= (u_long)b[BFS+0] << 4;
	f->mant.ml = cl1;
	f->mant2.ml = cl2;

	if ((! f->mant.ml) && (! f->mant2.ml)) {
		f->exp = 0;
		return;
	}
	/*       MSB              */
	for ( ; ; ) {
		f->exp--;
		cl1 = (u_long)f->mant.ml & (u_long)0x80000L;
		cl2 = (((u_long)f->mant2.ml & (u_long)0x80000000L) ? 1L : 0L);
		f->mant2.ml = (u_long)f->mant2.ml << 1;
		f->mant.ml = (u_long)f->mant.ml << 1;
		f->mant.ml = (u_long)f->mant.ml | cl2;
		if (cl1)
			break;
	}
}
#endif
/*****************************************************************************
 * @(#)     : _inmod
 *       : BFS*2           
 *      : int 1 :         
 *	   : int 0 :       
 *       : DTYP *w0  : (I)             
 *	               : (O)   
 *	   : DTYP *sp  : (O)            
 *	   : DTYP w1   : (I)  
 *	   : int *size : (I)         
 *	               : (O)           
 *       : 1992/07/30   
 *****************************************************************************/
int _inmod( DTYP *w0, DTYP *sp, DTYP w1, int *size )
{
	register int x;		/*        */
	static DTYP qt[BFS];		/*  :quotient */
	DTYP c;			/*         */
	int st;			/*         */
	int bfs;		/*           */
	int blen;		/*        */
	DTYP msk;		/*     */
	DTYP seb;		/*           */
	DTYP *wp, *qp;		/*          */
	DTYP ssp;		/*           */
	DTYP wd;		/*            */
	int sz;			/*               */

	sz = *size;
	/*  /          */
	qp = qt;
	wp = w0;
	for (msk = 0, x = 0; x < sz; x++) {
		*qp++ = 0;
		msk |= *wp++;
	}
	if (!msk)
		return 0;

	/*              */
	for (wp = w0, st = 0; st < sz; st++) {
		if (*wp++)
			break;
	}
	/*          ,        */
	bfs = sz - st;
	blen = bfs * BLEN;

	/*      */
	ssp = (DTYP)0;
	msk = BMSK;
	wp = &w0[st];
	wd = *wp;
	seb = (DTYP)1;
	qp = &qt[st];
	qp--;

	while (1) {
	D_LP1:
		c = 0;
	D_LP2:
		/*         */
		if (c)
			*qp |= seb;
		if ((--blen) < 0)
			break;
		seb >>= 1;
		if (!seb) {
			/*          */
			seb = BMSK;
			qp++;
		}
		if (!msk) {
			/*              */
			msk = BMSK;
			wp++;
			wd = *wp;
		}

		/*            */
		c = ((wd & msk) ? 1 : 0);
		msk >>= 1;

		/*    << 1 */
		ssp <<= 1;
		ssp |= c;
		/* if (   >   ) goto D_LP1 */
		if (w1 > ssp)
			goto D_LP1;
		/*    =    -    */
		ssp = ssp - w1;

		c = 1;
		goto D_LP2;
	}

	/*    (   ) */
	qp = &qt[st];
	wp = w0;
	for (x = 0; x < bfs; x++)
		*wp++ = *qp++;
	*size = bfs;
	*sp = ssp;
	return 1;
}

/*******************************************************************************
*
* _print 
*
 * Copyright(C) 1999(2000-2004). Renesas Technology Corp.
 * and Renesas Solutions Corp., All rights reserved.
 *
*
*******************************************************************************/
