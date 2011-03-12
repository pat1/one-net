#ifndef SFR_X613X
#define SFR_X613X

#include "cc430x613x.h"

/*-------------------------------------------------------------------------
 *   DIGITAL I/O Port1/2 Pull up / Pull down Resistors
 *-------------------------------------------------------------------------*/


__no_init volatile union
{
  unsigned char P1IN_byte;  /* Port 1 Input */ 
  
  struct
  {
    unsigned char P1IN_0         : 1; 
    unsigned char P1IN_1         : 1; 
    unsigned char P1IN_2         : 1; 
    unsigned char P1IN_3         : 1; 
    unsigned char P1IN_4         : 1; 
    unsigned char P1IN_5         : 1; 
    unsigned char P1IN_6         : 1; 
    unsigned char P1IN_7         : 1; 
  } P1IN_bit;  
} @ 0x0200;




__no_init volatile union
{
  unsigned char P1OUT_byte;  /* Port 1 Output */ 
  
  struct
  {
    unsigned char P1OUT_0        : 1; 
    unsigned char P1OUT_1        : 1; 
    unsigned char P1OUT_2        : 1; 
    unsigned char P1OUT_3        : 1; 
    unsigned char P1OUT_4        : 1; 
    unsigned char P1OUT_5        : 1; 
    unsigned char P1OUT_6        : 1; 
    unsigned char P1OUT_7        : 1; 
  } P1OUT_bit;  
} @ 0x0202;


enum {
  P1OUT_0             = 0x0001,
  P1OUT_1             = 0x0002,
  P1OUT_2             = 0x0004,
  P1OUT_3             = 0x0008,
  P1OUT_4             = 0x0010,
  P1OUT_5             = 0x0020,
  P1OUT_6             = 0x0040,
  P1OUT_7             = 0x0080,
};


__no_init volatile union
{
  unsigned char P1DIR_byte;  /* Port 1 Direction */ 
  
  struct
  {
    unsigned char P1DIR_0        : 1; 
    unsigned char P1DIR_1        : 1; 
    unsigned char P1DIR_2        : 1; 
    unsigned char P1DIR_3        : 1; 
    unsigned char P1DIR_4        : 1; 
    unsigned char P1DIR_5        : 1; 
    unsigned char P1DIR_6        : 1; 
    unsigned char P1DIR_7        : 1; 
  } P1DIR_bit;  
} @ 0x0204;


enum {
  P1DIR_0             = 0x0001,
  P1DIR_1             = 0x0002,
  P1DIR_2             = 0x0004,
  P1DIR_3             = 0x0008,
  P1DIR_4             = 0x0010,
  P1DIR_5             = 0x0020,
  P1DIR_6             = 0x0040,
  P1DIR_7             = 0x0080,
};


__no_init volatile union
{
  unsigned char P1REN_byte;  /* Port 1 Resistor Enable */ 
  
  struct
  {
    unsigned char P1REN_0        : 1; 
    unsigned char P1REN_1        : 1; 
    unsigned char P1REN_2        : 1; 
    unsigned char P1REN_3        : 1; 
    unsigned char P1REN_4        : 1; 
    unsigned char P1REN_5        : 1; 
    unsigned char P1REN_6        : 1; 
    unsigned char P1REN_7        : 1; 
  } P1REN_bit;  
} @ 0x0206;


enum {
  P1REN_0             = 0x0001,
  P1REN_1             = 0x0002,
  P1REN_2             = 0x0004,
  P1REN_3             = 0x0008,
  P1REN_4             = 0x0010,
  P1REN_5             = 0x0020,
  P1REN_6             = 0x0040,
  P1REN_7             = 0x0080,
};


__no_init volatile union
{
  unsigned char P1DS_byte;  /* Port 1 Resistor Drive Strenght */ 
  
  struct
  {
    unsigned char P1DS_0         : 1; 
    unsigned char P1DS_1         : 1; 
    unsigned char P1DS_2         : 1; 
    unsigned char P1DS_3         : 1; 
    unsigned char P1DS_4         : 1; 
    unsigned char P1DS_5         : 1; 
    unsigned char P1DS_6         : 1; 
    unsigned char P1DS_7         : 1; 
  } P1DS_bit;  
} @ 0x0208;


enum {
  P1DS_0              = 0x0001,
  P1DS_1              = 0x0002,
  P1DS_2              = 0x0004,
  P1DS_3              = 0x0008,
  P1DS_4              = 0x0010,
  P1DS_5              = 0x0020,
  P1DS_6              = 0x0040,
  P1DS_7              = 0x0080,
};


__no_init volatile union
{
  unsigned char P1SEL_byte;  /* Port 1 Selection */ 
  
  struct
  {
    unsigned char P1SEL_0        : 1; 
    unsigned char P1SEL_1        : 1; 
    unsigned char P1SEL_2        : 1; 
    unsigned char P1SEL_3        : 1; 
    unsigned char P1SEL_4        : 1; 
    unsigned char P1SEL_5        : 1; 
    unsigned char P1SEL_6        : 1; 
    unsigned char P1SEL_7        : 1; 
  } P1SEL_bit;  
} @ 0x020A;


enum {
  P1SEL_0             = 0x0001,
  P1SEL_1             = 0x0002,
  P1SEL_2             = 0x0004,
  P1SEL_3             = 0x0008,
  P1SEL_4             = 0x0010,
  P1SEL_5             = 0x0020,
  P1SEL_6             = 0x0040,
  P1SEL_7             = 0x0080,
};


__no_init volatile union
{
  unsigned char P1IES_byte;  /* Port 1 Interrupt Edge Select */ 
  
  struct
  {
    unsigned char P1IES_0        : 1; 
    unsigned char P1IES_1        : 1; 
    unsigned char P1IES_2        : 1; 
    unsigned char P1IES_3        : 1; 
    unsigned char P1IES_4        : 1; 
    unsigned char P1IES_5        : 1; 
    unsigned char P1IES_6        : 1; 
    unsigned char P1IES_7        : 1; 
  } P1IES_bit;  
} @ 0x0218;


enum {
  P1IES_0             = 0x0001,
  P1IES_1             = 0x0002,
  P1IES_2             = 0x0004,
  P1IES_3             = 0x0008,
  P1IES_4             = 0x0010,
  P1IES_5             = 0x0020,
  P1IES_6             = 0x0040,
  P1IES_7             = 0x0080,
};


__no_init volatile union
{
  unsigned char P1IE_byte;  /* Port 1 Interrupt Enable */ 
  
  struct
  {
    unsigned char P1IE_0         : 1; 
    unsigned char P1IE_1         : 1; 
    unsigned char P1IE_2         : 1; 
    unsigned char P1IE_3         : 1; 
    unsigned char P1IE_4         : 1; 
    unsigned char P1IE_5         : 1; 
    unsigned char P1IE_6         : 1; 
    unsigned char P1IE_7         : 1; 
  } P1IE_bit;  
} @ 0x021A;


enum {
  P1IE_0              = 0x0001,
  P1IE_1              = 0x0002,
  P1IE_2              = 0x0004,
  P1IE_3              = 0x0008,
  P1IE_4              = 0x0010,
  P1IE_5              = 0x0020,
  P1IE_6              = 0x0040,
  P1IE_7              = 0x0080,
};


__no_init volatile union
{
  unsigned char P1IFG_byte;  /* Port 1 Interrupt Flag */ 
  
  struct
  {
    unsigned char P1IFG_0        : 1; 
    unsigned char P1IFG_1        : 1; 
    unsigned char P1IFG_2        : 1; 
    unsigned char P1IFG_3        : 1; 
    unsigned char P1IFG_4        : 1; 
    unsigned char P1IFG_5        : 1; 
    unsigned char P1IFG_6        : 1; 
    unsigned char P1IFG_7        : 1; 
  } P1IFG_bit;  
} @ 0x021C;


enum {
  P1IFG_0             = 0x0001,
  P1IFG_1             = 0x0002,
  P1IFG_2             = 0x0004,
  P1IFG_3             = 0x0008,
  P1IFG_4             = 0x0010,
  P1IFG_5             = 0x0020,
  P1IFG_6             = 0x0040,
  P1IFG_7             = 0x0080,
};


__no_init volatile union
{
  unsigned char P2IN_byte;  /* Port 2 Input */ 
  
  struct
  {
    unsigned char P2IN_0         : 1; 
    unsigned char P2IN_1         : 1; 
    unsigned char P2IN_2         : 1; 
    unsigned char P2IN_3         : 1; 
    unsigned char P2IN_4         : 1; 
    unsigned char P2IN_5         : 1; 
    unsigned char P2IN_6         : 1; 
    unsigned char P2IN_7         : 1; 
  } P2IN_bit;  
} @ 0x0201;


enum {
  P2IN_0              = 0x0001,
  P2IN_1              = 0x0002,
  P2IN_2              = 0x0004,
  P2IN_3              = 0x0008,
  P2IN_4              = 0x0010,
  P2IN_5              = 0x0020,
  P2IN_6              = 0x0040,
  P2IN_7              = 0x0080,
};


__no_init volatile union
{
  unsigned char P2OUT_byte;  /* Port 2 Output */ 
  
  struct
  {
    unsigned char P2OUT_0        : 1; 
    unsigned char P2OUT_1        : 1; 
    unsigned char P2OUT_2        : 1; 
    unsigned char P2OUT_3        : 1; 
    unsigned char P2OUT_4        : 1; 
    unsigned char P2OUT_5        : 1; 
    unsigned char P2OUT_6        : 1; 
    unsigned char P2OUT_7        : 1; 
  } P2OUT_bit;  
} @ 0x0203;


enum {
  P2OUT_0             = 0x0001,
  P2OUT_1             = 0x0002,
  P2OUT_2             = 0x0004,
  P2OUT_3             = 0x0008,
  P2OUT_4             = 0x0010,
  P2OUT_5             = 0x0020,
  P2OUT_6             = 0x0040,
  P2OUT_7             = 0x0080,
};


__no_init volatile union
{
  unsigned char P2DIR_byte;  /* Port 2 Direction */ 
  
  struct
  {
    unsigned char P2DIR_0        : 1; 
    unsigned char P2DIR_1        : 1; 
    unsigned char P2DIR_2        : 1; 
    unsigned char P2DIR_3        : 1; 
    unsigned char P2DIR_4        : 1; 
    unsigned char P2DIR_5        : 1; 
    unsigned char P2DIR_6        : 1; 
    unsigned char P2DIR_7        : 1; 
  } P2DIR_bit;  
} @ 0x0205;


enum {
  P2DIR_0             = 0x0001,
  P2DIR_1             = 0x0002,
  P2DIR_2             = 0x0004,
  P2DIR_3             = 0x0008,
  P2DIR_4             = 0x0010,
  P2DIR_5             = 0x0020,
  P2DIR_6             = 0x0040,
  P2DIR_7             = 0x0080,
};


__no_init volatile union
{
  unsigned char P2REN_byte;  /* Port 2 Resistor Enable */ 
  
  struct
  {
    unsigned char P2REN_0        : 1; 
    unsigned char P2REN_1        : 1; 
    unsigned char P2REN_2        : 1; 
    unsigned char P2REN_3        : 1; 
    unsigned char P2REN_4        : 1; 
    unsigned char P2REN_5        : 1; 
    unsigned char P2REN_6        : 1; 
    unsigned char P2REN_7        : 1; 
  } P2REN_bit;  
} @ 0x0207;


enum {
  P2REN_0             = 0x0001,
  P2REN_1             = 0x0002,
  P2REN_2             = 0x0004,
  P2REN_3             = 0x0008,
  P2REN_4             = 0x0010,
  P2REN_5             = 0x0020,
  P2REN_6             = 0x0040,
  P2REN_7             = 0x0080,
};


__no_init volatile union
{
  unsigned char P2DS_byte;  /* Port 2 Resistor Drive Strenght */ 
  
  struct
  {
    unsigned char P2DS_0         : 1; 
    unsigned char P2DS_1         : 1; 
    unsigned char P2DS_2         : 1; 
    unsigned char P2DS_3         : 1; 
    unsigned char P2DS_4         : 1; 
    unsigned char P2DS_5         : 1; 
    unsigned char P2DS_6         : 1; 
    unsigned char P2DS_7         : 1; 
  } P2DS_bit;  
} @ 0x0209;


enum {
  P2DS_0              = 0x0001,
  P2DS_1              = 0x0002,
  P2DS_2              = 0x0004,
  P2DS_3              = 0x0008,
  P2DS_4              = 0x0010,
  P2DS_5              = 0x0020,
  P2DS_6              = 0x0040,
  P2DS_7              = 0x0080,
};


__no_init volatile union
{
  unsigned char P2SEL_byte;  /* Port 2 Selection */ 
  
  struct
  {
    unsigned char P2SEL_0        : 1; 
    unsigned char P2SEL_1        : 1; 
    unsigned char P2SEL_2        : 1; 
    unsigned char P2SEL_3        : 1; 
    unsigned char P2SEL_4        : 1; 
    unsigned char P2SEL_5        : 1; 
    unsigned char P2SEL_6        : 1; 
    unsigned char P2SEL_7        : 1; 
  } P2SEL_bit;  
} @ 0x020B;


enum {
  P2SEL_0             = 0x0001,
  P2SEL_1             = 0x0002,
  P2SEL_2             = 0x0004,
  P2SEL_3             = 0x0008,
  P2SEL_4             = 0x0010,
  P2SEL_5             = 0x0020,
  P2SEL_6             = 0x0040,
  P2SEL_7             = 0x0080,
};


__no_init volatile union
{
  unsigned char P2IES_byte;  /* Port 2 Interrupt Edge Select */ 
  
  struct
  {
    unsigned char P2IES_0        : 1; 
    unsigned char P2IES_1        : 1; 
    unsigned char P2IES_2        : 1; 
    unsigned char P2IES_3        : 1; 
    unsigned char P2IES_4        : 1; 
    unsigned char P2IES_5        : 1; 
    unsigned char P2IES_6        : 1; 
    unsigned char P2IES_7        : 1; 
  } P2IES_bit;  
} @ 0x0219;


enum {
  P2IES_0             = 0x0001,
  P2IES_1             = 0x0002,
  P2IES_2             = 0x0004,
  P2IES_3             = 0x0008,
  P2IES_4             = 0x0010,
  P2IES_5             = 0x0020,
  P2IES_6             = 0x0040,
  P2IES_7             = 0x0080,
};


__no_init volatile union
{
  unsigned char P2IE_byte;  /* Port 2 Interrupt Enable  */ 
  
  struct
  {
    unsigned char P2IE_0         : 1; 
    unsigned char P2IE_1         : 1; 
    unsigned char P2IE_2         : 1; 
    unsigned char P2IE_3         : 1; 
    unsigned char P2IE_4         : 1; 
    unsigned char P2IE_5         : 1; 
    unsigned char P2IE_6         : 1; 
    unsigned char P2IE_7         : 1; 
  } P2IE_bit;  
} @ 0x021B;


enum {
  P2IE_0              = 0x0001,
  P2IE_1              = 0x0002,
  P2IE_2              = 0x0004,
  P2IE_3              = 0x0008,
  P2IE_4              = 0x0010,
  P2IE_5              = 0x0020,
  P2IE_6              = 0x0040,
  P2IE_7              = 0x0080,
};


__no_init volatile union
{
  unsigned char P2IFG_byte;  /* Port 2 Interrupt Flag */ 
  
  struct
  {
    unsigned char P2IFG_0        : 1; 
    unsigned char P2IFG_1        : 1; 
    unsigned char P2IFG_2        : 1; 
    unsigned char P2IFG_3        : 1; 
    unsigned char P2IFG_4        : 1; 
    unsigned char P2IFG_5        : 1; 
    unsigned char P2IFG_6        : 1; 
    unsigned char P2IFG_7        : 1; 
  } P2IFG_bit;  
} @ 0x021D;


enum {
  P2IFG_0             = 0x0001,
  P2IFG_1             = 0x0002,
  P2IFG_2             = 0x0004,
  P2IFG_3             = 0x0008,
  P2IFG_4             = 0x0010,
  P2IFG_5             = 0x0020,
  P2IFG_6             = 0x0040,
  P2IFG_7             = 0x0080,
};

 
/*-------------------------------------------------------------------------
 *   DIGITAL I/O Port3/4 Pull up / Pull down Resistors
 *-------------------------------------------------------------------------*/

__no_init volatile union
{
  unsigned char P3IN_byte;  /* Port 3 Input */ 
  
  struct
  {
    unsigned char P3IN_0         : 1; 
    unsigned char P3IN_1         : 1; 
    unsigned char P3IN_2         : 1; 
    unsigned char P3IN_3         : 1; 
    unsigned char P3IN_4         : 1; 
    unsigned char P3IN_5         : 1; 
    unsigned char P3IN_6         : 1; 
    unsigned char P3IN_7         : 1; 
  } P3IN_bit;  
} @ 0x0220;




__no_init volatile union
{
  unsigned char P3OUT_byte;  /* Port 3 Output */ 
  
  struct
  {
    unsigned char P3OUT_0        : 1; 
    unsigned char P3OUT_1        : 1; 
    unsigned char P3OUT_2        : 1; 
    unsigned char P3OUT_3        : 1; 
    unsigned char P3OUT_4        : 1; 
    unsigned char P3OUT_5        : 1; 
    unsigned char P3OUT_6        : 1; 
    unsigned char P3OUT_7        : 1; 
  } P3OUT_bit;  
} @ 0x0222;


enum {
  P3OUT_0             = 0x0001,
  P3OUT_1             = 0x0002,
  P3OUT_2             = 0x0004,
  P3OUT_3             = 0x0008,
  P3OUT_4             = 0x0010,
  P3OUT_5             = 0x0020,
  P3OUT_6             = 0x0040,
  P3OUT_7             = 0x0080,
};


__no_init volatile union
{
  unsigned char P3DIR_byte;  /* Port 3 Direction */ 
  
  struct
  {
    unsigned char P3DIR_0        : 1; 
    unsigned char P3DIR_1        : 1; 
    unsigned char P3DIR_2        : 1; 
    unsigned char P3DIR_3        : 1; 
    unsigned char P3DIR_4        : 1; 
    unsigned char P3DIR_5        : 1; 
    unsigned char P3DIR_6        : 1; 
    unsigned char P3DIR_7        : 1; 
  } P3DIR_bit;  
} @ 0x0224;


enum {
  P3DIR_0             = 0x0001,
  P3DIR_1             = 0x0002,
  P3DIR_2             = 0x0004,
  P3DIR_3             = 0x0008,
  P3DIR_4             = 0x0010,
  P3DIR_5             = 0x0020,
  P3DIR_6             = 0x0040,
  P3DIR_7             = 0x0080,
};


__no_init volatile union
{
  unsigned char P3REN_byte;  /* Port 3 Resistor Enable */ 
  
  struct
  {
    unsigned char P3REN_0        : 1; 
    unsigned char P3REN_1        : 1; 
    unsigned char P3REN_2        : 1; 
    unsigned char P3REN_3        : 1; 
    unsigned char P3REN_4        : 1; 
    unsigned char P3REN_5        : 1; 
    unsigned char P3REN_6        : 1; 
    unsigned char P3REN_7        : 1; 
  } P3REN_bit;  
} @ 0x0226;


enum {
  P3REN_0             = 0x0001,
  P3REN_1             = 0x0002,
  P3REN_2             = 0x0004,
  P3REN_3             = 0x0008,
  P3REN_4             = 0x0010,
  P3REN_5             = 0x0020,
  P3REN_6             = 0x0040,
  P3REN_7             = 0x0080,
};


__no_init volatile union
{
  unsigned char P3DS_byte;  /* Port 3 Resistor Drive Strenght */ 
  
  struct
  {
    unsigned char P3DS_0         : 1; 
    unsigned char P3DS_1         : 1; 
    unsigned char P3DS_2         : 1; 
    unsigned char P3DS_3         : 1; 
    unsigned char P3DS_4         : 1; 
    unsigned char P3DS_5         : 1; 
    unsigned char P3DS_6         : 1; 
    unsigned char P3DS_7         : 1; 
  } P3DS_bit;  
} @ 0x0228;


enum {
  P3DS_0              = 0x0001,
  P3DS_1              = 0x0002,
  P3DS_2              = 0x0004,
  P3DS_3              = 0x0008,
  P3DS_4              = 0x0010,
  P3DS_5              = 0x0020,
  P3DS_6              = 0x0040,
  P3DS_7              = 0x0080,
};


__no_init volatile union
{
  unsigned char P3SEL_byte;  /* Port 3 Selection */ 
  
  struct
  {
    unsigned char P3SEL_0        : 1; 
    unsigned char P3SEL_1        : 1; 
    unsigned char P3SEL_2        : 1; 
    unsigned char P3SEL_3        : 1; 
    unsigned char P3SEL_4        : 1; 
    unsigned char P3SEL_5        : 1; 
    unsigned char P3SEL_6        : 1; 
    unsigned char P3SEL_7        : 1; 
  } P3SEL_bit;  
} @ 0x022A;


enum {
  P3SEL_0             = 0x0001,
  P3SEL_1             = 0x0002,
  P3SEL_2             = 0x0004,
  P3SEL_3             = 0x0008,
  P3SEL_4             = 0x0010,
  P3SEL_5             = 0x0020,
  P3SEL_6             = 0x0040,
  P3SEL_7             = 0x0080,
};


__no_init volatile union
{
  unsigned char P4IN_byte;  /* Port 4 Input */ 
  
  struct
  {
    unsigned char P4IN_0         : 1; 
    unsigned char P4IN_1         : 1; 
    unsigned char P4IN_2         : 1; 
    unsigned char P4IN_3         : 1; 
    unsigned char P4IN_4         : 1; 
    unsigned char P4IN_5         : 1; 
    unsigned char P4IN_6         : 1; 
    unsigned char P4IN_7         : 1; 
  } P4IN_bit;  
} @ 0x0221;


enum {
  P4IN_0              = 0x0001,
  P4IN_1              = 0x0002,
  P4IN_2              = 0x0004,
  P4IN_3              = 0x0008,
  P4IN_4              = 0x0010,
  P4IN_5              = 0x0020,
  P4IN_6              = 0x0040,
  P4IN_7              = 0x0080,
};


__no_init volatile union
{
  unsigned char P4OUT_byte;  /* Port 4 Output */ 
  
  struct
  {
    unsigned char P4OUT_0        : 1; 
    unsigned char P4OUT_1        : 1; 
    unsigned char P4OUT_2        : 1; 
    unsigned char P4OUT_3        : 1; 
    unsigned char P4OUT_4        : 1; 
    unsigned char P4OUT_5        : 1; 
    unsigned char P4OUT_6        : 1; 
    unsigned char P4OUT_7        : 1; 
  } P4OUT_bit;  
} @ 0x0223;


enum {
  P4OUT_0             = 0x0001,
  P4OUT_1             = 0x0002,
  P4OUT_2             = 0x0004,
  P4OUT_3             = 0x0008,
  P4OUT_4             = 0x0010,
  P4OUT_5             = 0x0020,
  P4OUT_6             = 0x0040,
  P4OUT_7             = 0x0080,
};


__no_init volatile union
{
  unsigned char P4DIR_byte;  /* Port 4 Direction */ 
  
  struct
  {
    unsigned char P4DIR_0        : 1; 
    unsigned char P4DIR_1        : 1; 
    unsigned char P4DIR_2        : 1; 
    unsigned char P4DIR_3        : 1; 
    unsigned char P4DIR_4        : 1; 
    unsigned char P4DIR_5        : 1; 
    unsigned char P4DIR_6        : 1; 
    unsigned char P4DIR_7        : 1; 
  } P4DIR_bit;  
} @ 0x0225;


enum {
  P4DIR_0             = 0x0001,
  P4DIR_1             = 0x0002,
  P4DIR_2             = 0x0004,
  P4DIR_3             = 0x0008,
  P4DIR_4             = 0x0010,
  P4DIR_5             = 0x0020,
  P4DIR_6             = 0x0040,
  P4DIR_7             = 0x0080,
};


__no_init volatile union
{
  unsigned char P4REN_byte;  /* Port 4 Resistor Enable */ 
  
  struct
  {
    unsigned char P4REN_0        : 1; 
    unsigned char P4REN_1        : 1; 
    unsigned char P4REN_2        : 1; 
    unsigned char P4REN_3        : 1; 
    unsigned char P4REN_4        : 1; 
    unsigned char P4REN_5        : 1; 
    unsigned char P4REN_6        : 1; 
    unsigned char P4REN_7        : 1; 
  } P4REN_bit;  
} @ 0x0227;


enum {
  P4REN_0             = 0x0001,
  P4REN_1             = 0x0002,
  P4REN_2             = 0x0004,
  P4REN_3             = 0x0008,
  P4REN_4             = 0x0010,
  P4REN_5             = 0x0020,
  P4REN_6             = 0x0040,
  P4REN_7             = 0x0080,
};


__no_init volatile union
{
  unsigned char P4DS_byte;  /* Port 4 Resistor Drive Strenght */ 
  
  struct
  {
    unsigned char P4DS_0         : 1; 
    unsigned char P4DS_1         : 1; 
    unsigned char P4DS_2         : 1; 
    unsigned char P4DS_3         : 1; 
    unsigned char P4DS_4         : 1; 
    unsigned char P4DS_5         : 1; 
    unsigned char P4DS_6         : 1; 
    unsigned char P4DS_7         : 1; 
  } P4DS_bit;  
} @ 0x0229;


enum {
  P4DS_0              = 0x0001,
  P4DS_1              = 0x0002,
  P4DS_2              = 0x0004,
  P4DS_3              = 0x0008,
  P4DS_4              = 0x0010,
  P4DS_5              = 0x0020,
  P4DS_6              = 0x0040,
  P4DS_7              = 0x0080,
};


__no_init volatile union
{
  unsigned char P4SEL_byte;  /* Port 4 Selection */ 
  
  struct
  {
    unsigned char P4SEL_0        : 1; 
    unsigned char P4SEL_1        : 1; 
    unsigned char P4SEL_2        : 1; 
    unsigned char P4SEL_3        : 1; 
    unsigned char P4SEL_4        : 1; 
    unsigned char P4SEL_5        : 1; 
    unsigned char P4SEL_6        : 1; 
    unsigned char P4SEL_7        : 1; 
  } P4SEL_bit;  
} @ 0x022B;


enum {
  P4SEL_0             = 0x0001,
  P4SEL_1             = 0x0002,
  P4SEL_2             = 0x0004,
  P4SEL_3             = 0x0008,
  P4SEL_4             = 0x0010,
  P4SEL_5             = 0x0020,
  P4SEL_6             = 0x0040,
  P4SEL_7             = 0x0080,
};

/*------------------------------------------------------
  Port P1 register (output)
------------------------------------------------------*/
#define    p1            P1OUT

#define    p1_0          P1OUT_bit.P1OUT_0        /* Port P10 bit */
#define    p1_1          P1OUT_bit.P1OUT_1        /* Port P11 bit */
#define    p1_2          P1OUT_bit.P1OUT_2        /* Port P12 bit */
#define    p1_3          P1OUT_bit.P1OUT_3        /* Port P13 bit */
#define    p1_4          P1OUT_bit.P1OUT_4        /* Port P14 bit */
#define    p1_5          P1OUT_bit.P1OUT_5        /* Port P15 bit */
#define    p1_6          P1OUT_bit.P1OUT_6        /* Port P16 bit */
#define    p1_7          P1OUT_bit.P1OUT_7        /* Port P17 bit */

/*------------------------------------------------------
  Port P1 register (input)
------------------------------------------------------*/
#define    pi1            P1IN

#define    pi1_0          P1IN_bit.P1IN_0         /* Port P10 bit */
#define    pi1_1          P1IN_bit.P1IN_1         /* Port P11 bit */
#define    pi1_2          P1IN_bit.P1IN_2         /* Port P12 bit */
#define    pi1_3          P1IN_bit.P1IN_3         /* Port P13 bit */
#define    pi1_4          P1IN_bit.P1IN_4         /* Port P14 bit */
#define    pi1_5          P1IN_bit.P1IN_5         /* Port P15 bit */
#define    pi1_6          P1IN_bit.P1IN_6         /* Port P16 bit */
#define    pi1_7          P1IN_bit.P1IN_7         /* Port P17 bit */

/*------------------------------------------------------
  Port P1 direction register
------------------------------------------------------*/
#define    pd1           P1DIR

#define    pd1_0         P1DIR_bit.P1DIR_0        /* Port P10 direction bit */
#define    pd1_1         P1DIR_bit.P1DIR_1        /* Port P11 direction bit */
#define    pd1_2         P1DIR_bit.P1DIR_2        /* Port P12 direction bit */
#define    pd1_3         P1DIR_bit.P1DIR_3        /* Port P13 direction bit */
#define    pd1_4         P1DIR_bit.P1DIR_4        /* Port P14 direction bit */
#define    pd1_5         P1DIR_bit.P1DIR_5        /* Port P15 direction bit */
#define    pd1_6         P1DIR_bit.P1DIR_6        /* Port P16 direction bit */
#define    pd1_7         P1DIR_bit.P1DIR_7        /* Port P17 direction bit */

/*------------------------------------------------------
  Port P1 func. selection register
------------------------------------------------------*/
#define    ps1           P1SEL

#define    ps1_0         P1SEL_bit.P1SEL_0
#define    ps1_1         P1SEL_bit.P1SEL_1
#define    ps1_2         P1SEL_bit.P1SEL_2
#define    ps1_3         P1SEL_bit.P1SEL_3
#define    ps1_4         P1SEL_bit.P1SEL_4
#define    ps1_5         P1SEL_bit.P1SEL_5
#define    ps1_6         P1SEL_bit.P1SEL_6
#define    ps1_7         P1SEL_bit.P1SEL_7

/*-----------------------------------------------------
  Port P1 pull-up resistor enable register
------------------------------------------------------*/
#define    pre1          P1REN

#define    pre1_0        P1REN_bit.P1REN_0
#define    pre1_1        P1REN_bit.P1REN_1
#define    pre1_2        P1REN_bit.P1REN_2
#define    pre1_3        P1REN_bit.P1REN_3
#define    pre1_4        P1REN_bit.P1REN_4
#define    pre1_5        P1REN_bit.P1REN_5
#define    pre1_6        P1REN_bit.P1REN_6
#define    pre1_7        P1REN_bit.P1REN_7

/*-----------------------------------------------------
  Port P1 interrupt enable register
------------------------------------------------------*/
#define    pie1          P1IE

#define    pie1_0        P1IE_bit.P1IE_0
#define    pie1_1        P1IE_bit.P1IE_1
#define    pie1_2        P1IE_bit.P1IE_2
#define    pie1_3        P1IE_bit.P1IE_3
#define    pie1_4        P1IE_bit.P1IE_4
#define    pie1_5        P1IE_bit.P1IE_5
#define    pie1_6        P1IE_bit.P1IE_6
#define    pie1_7        P1IE_bit.P1IE_7

/*-----------------------------------------------------
  Port P1 interrupt flag register
------------------------------------------------------*/
#define    pif1          P1IFG

#define    pif1_0        P1IFG_bit.P1IFG_0
#define    pif1_1        P1IFG_bit.P1IFG_1
#define    pif1_2        P1IFG_bit.P1IFG_2
#define    pif1_3        P1IFG_bit.P1IFG_3
#define    pif1_4        P1IFG_bit.P1IFG_4
#define    pif1_5        P1IFG_bit.P1IFG_5
#define    pif1_6        P1IFG_bit.P1IFG_6
#define    pif1_7        P1IFG_bit.P1IFG_7

/*-----------------------------------------------------
  Port P1 interrupt edge select register
------------------------------------------------------*/
#define    pis1          P1IES

#define    pis1_0        P1IES_bit.P1IES_0
#define    pis1_1        P1IES_bit.P1IES_1
#define    pis1_2        P1IES_bit.P1IES_2
#define    pis1_3        P1IES_bit.P1IES_3
#define    pis1_4        P1IES_bit.P1IES_4
#define    pis1_5        P1IES_bit.P1IES_5
#define    pis1_6        P1IES_bit.P1IES_6
#define    pis1_7        P1IES_bit.P1IES_7

/*------------------------------------------------------
  Port P2 register (output)
------------------------------------------------------*/
#define    p2            P2OUT

#define    p2_0          P2OUT_bit.P2OUT_0        /* Port P20 bit */
#define    p2_1          P2OUT_bit.P2OUT_1        /* Port P21 bit */
#define    p2_2          P2OUT_bit.P2OUT_2        /* Port P22 bit */
#define    p2_3          P2OUT_bit.P2OUT_3        /* Port P23 bit */
#define    p2_4          P2OUT_bit.P2OUT_4        /* Port P24 bit */
#define    p2_5          P2OUT_bit.P2OUT_5        /* Port P25 bit */
#define    p2_6          P2OUT_bit.P2OUT_6        /* Port P26 bit */
#define    p2_7          P2OUT_bit.P2OUT_7        /* Port P27 bit */

/*------------------------------------------------------
  Port P2 register (input)
------------------------------------------------------*/
#define    pi2            P2IN

#define    pi2_0          P2IN_bit.P2IN_0         /* Port P20 bit */
#define    pi2_1          P2IN_bit.P2IN_1         /* Port P21 bit */
#define    pi2_2          P2IN_bit.P2IN_2         /* Port P22 bit */
#define    pi2_3          P2IN_bit.P2IN_3         /* Port P23 bit */
#define    pi2_4          P2IN_bit.P2IN_4         /* Port P24 bit */
#define    pi2_5          P2IN_bit.P2IN_5         /* Port P25 bit */
#define    pi2_6          P2IN_bit.P2IN_6         /* Port P26 bit */
#define    pi2_7          P2IN_bit.P2IN_7         /* Port P27 bit */

/*------------------------------------------------------
  Port P2 direction register
------------------------------------------------------*/
#define    pd2           P2DIR

#define    pd2_0         P2DIR_bit.P2DIR_0        /* Port P20 direction bit */
#define    pd2_1         P2DIR_bit.P2DIR_1        /* Port P21 direction bit */
#define    pd2_2         P2DIR_bit.P2DIR_2        /* Port P22 direction bit */
#define    pd2_3         P2DIR_bit.P2DIR_3        /* Port P23 direction bit */
#define    pd2_4         P2DIR_bit.P2DIR_4        /* Port P24 direction bit */
#define    pd2_5         P2DIR_bit.P2DIR_5        /* Port P25 direction bit */
#define    pd2_6         P2DIR_bit.P2DIR_6        /* Port P26 direction bit */
#define    pd2_7         P2DIR_bit.P2DIR_7        /* Port P27 direction bit */

/*------------------------------------------------------
  Port P2 func. selection register
------------------------------------------------------*/
#define    ps2           P2SEL

#define    ps2_0         P2SEL_bit.P2SEL_0
#define    ps2_1         P2SEL_bit.P2SEL_1
#define    ps2_2         P2SEL_bit.P2SEL_2
#define    ps2_3         P2SEL_bit.P2SEL_3
#define    ps2_4         P2SEL_bit.P2SEL_4
#define    ps2_5         P2SEL_bit.P2SEL_5
#define    ps2_6         P2SEL_bit.P2SEL_6
#define    ps2_7         P2SEL_bit.P2SEL_7

/*-----------------------------------------------------
  Port P2 pull-up resistor enable register
------------------------------------------------------*/
#define    pre2          P2REN

#define    pre2_0        P2REN_bit.P2REN_0
#define    pre2_1        P2REN_bit.P2REN_1
#define    pre2_2        P2REN_bit.P2REN_2
#define    pre2_3        P2REN_bit.P2REN_3
#define    pre2_4        P2REN_bit.P2REN_4
#define    pre2_5        P2REN_bit.P2REN_5
#define    pre2_6        P2REN_bit.P2REN_6
#define    pre2_7        P2REN_bit.P2REN_7

/*-----------------------------------------------------
  Port P2 interrupt enable register
------------------------------------------------------*/
#define    pie2          P2IE

#define    pie2_0        P2IE_bit.P2IE_0
#define    pie2_1        P2IE_bit.P2IE_1
#define    pie2_2        P2IE_bit.P2IE_2
#define    pie2_3        P2IE_bit.P2IE_3
#define    pie2_4        P2IE_bit.P2IE_4
#define    pie2_5        P2IE_bit.P2IE_5
#define    pie2_6        P2IE_bit.P2IE_6
#define    pie2_7        P2IE_bit.P2IE_7

/*-----------------------------------------------------
  Port P2 interrupt flag register
------------------------------------------------------*/
#define    pif2          P2IFG

#define    pif2_0        P2IFG_bit.P2IFG_0
#define    pif2_1        P2IFG_bit.P2IFG_1
#define    pif2_2        P2IFG_bit.P2IFG_2
#define    pif2_3        P2IFG_bit.P2IFG_3
#define    pif2_4        P2IFG_bit.P2IFG_4
#define    pif2_5        P2IFG_bit.P2IFG_5
#define    pif2_6        P2IFG_bit.P2IFG_6
#define    pif2_7        P2IFG_bit.P2IFG_7

/*-----------------------------------------------------
  Port P2 interrupt edge select register
------------------------------------------------------*/
#define    pis2          P2IES

#define    pis2_0        P2IES_bit.P2IES_0
#define    pis2_1        P2IES_bit.P2IES_1
#define    pis2_2        P2IES_bit.P2IES_2
#define    pis2_3        P2IES_bit.P2IES_3
#define    pis2_4        P2IES_bit.P2IES_4
#define    pis2_5        P2IES_bit.P2IES_5
#define    pis2_6        P2IES_bit.P2IES_6
#define    pis2_7        P2IES_bit.P2IES_7

/*------------------------------------------------------
  Port P3 register (output)
------------------------------------------------------*/
#define    p3            P3OUT

#define    p3_0          P3OUT_bit.P3OUT_0        /* Port P30 bit */
#define    p3_1          P3OUT_bit.P3OUT_1        /* Port P31 bit */
#define    p3_2          P3OUT_bit.P3OUT_2        /* Port P32 bit */
#define    p3_3          P3OUT_bit.P3OUT_3        /* Port P33 bit */
#define    p3_4          P3OUT_bit.P3OUT_4        /* Port P34 bit */
#define    p3_5          P3OUT_bit.P3OUT_5        /* Port P35 bit */
#define    p3_6          P3OUT_bit.P3OUT_6        /* Port P36 bit */
#define    p3_7          P3OUT_bit.P3OUT_7        /* Port P37 bit */

/*------------------------------------------------------
  Port P3 register (input)
------------------------------------------------------*/
#define    pi3            P3IN

#define    pi3_0          P3IN_bit.P3IN_0         /* Port P30 bit */
#define    pi3_1          P3IN_bit.P3IN_1         /* Port P31 bit */
#define    pi3_2          P3IN_bit.P3IN_2         /* Port P32 bit */
#define    pi3_3          P3IN_bit.P3IN_3         /* Port P33 bit */
#define    pi3_4          P3IN_bit.P3IN_4         /* Port P34 bit */
#define    pi3_5          P3IN_bit.P3IN_5         /* Port P35 bit */
#define    pi3_6          P3IN_bit.P3IN_6         /* Port P36 bit */
#define    pi3_7          P3IN_bit.P3IN_7         /* Port P37 bit */

/*------------------------------------------------------
  Port P3 direction register
------------------------------------------------------*/
#define    pd3           P3DIR

#define    pd3_0         P3DIR_bit.P3DIR_0        /* Port P30 direction bit */
#define    pd3_1         P3DIR_bit.P3DIR_1        /* Port P31 direction bit */
#define    pd3_2         P3DIR_bit.P3DIR_2        /* Port P32 direction bit */
#define    pd3_3         P3DIR_bit.P3DIR_3        /* Port P33 direction bit */
#define    pd3_4         P3DIR_bit.P3DIR_4        /* Port P34 direction bit */
#define    pd3_5         P3DIR_bit.P3DIR_5        /* Port P35 direction bit */
#define    pd3_6         P3DIR_bit.P3DIR_6        /* Port P36 direction bit */
#define    pd3_7         P3DIR_bit.P3DIR_7        /* Port P37 direction bit */

/*------------------------------------------------------
  Port P3 func. selection register
------------------------------------------------------*/
#define    ps3           P3SEL

#define    ps3_0         P3SEL_bit.P3SEL_0
#define    ps3_1         P3SEL_bit.P3SEL_1
#define    ps3_2         P3SEL_bit.P3SEL_2
#define    ps3_3         P3SEL_bit.P3SEL_3
#define    ps3_4         P3SEL_bit.P3SEL_4
#define    ps3_5         P3SEL_bit.P3SEL_5
#define    ps3_6         P3SEL_bit.P3SEL_6
#define    ps3_7         P3SEL_bit.P3SEL_7

/*-----------------------------------------------------
  Port P3 pull-up resistor enable register
------------------------------------------------------*/
#define    pre3          P3REN

#define    pre3_0        P3REN_bit.P3REN_0
#define    pre3_1        P3REN_bit.P3REN_1
#define    pre3_2        P3REN_bit.P3REN_2
#define    pre3_3        P3REN_bit.P3REN_3
#define    pre3_4        P3REN_bit.P3REN_4
#define    pre3_5        P3REN_bit.P3REN_5
#define    pre3_6        P3REN_bit.P3REN_6
#define    pre3_7        P3REN_bit.P3REN_7

/*------------------------------------------------------
  Port P4 register (output)
------------------------------------------------------*/
#define    p4            P4OUT

#define    p4_0          P4OUT_bit.P4OUT_0        /* Port P40 bit */
#define    p4_1          P4OUT_bit.P4OUT_1        /* Port P41 bit */
#define    p4_2          P4OUT_bit.P4OUT_2        /* Port P42 bit */
#define    p4_3          P4OUT_bit.P4OUT_3        /* Port P43 bit */
#define    p4_4          P4OUT_bit.P4OUT_4        /* Port P44 bit */
#define    p4_5          P4OUT_bit.P4OUT_5        /* Port P45 bit */
#define    p4_6          P4OUT_bit.P4OUT_6        /* Port P46 bit */
#define    p4_7          P4OUT_bit.P4OUT_7        /* Port P47 bit */

/*------------------------------------------------------
  Port P4 register (input)
------------------------------------------------------*/
#define    pi4           P4IN

#define    pi4_0         P4IN_bit.P4IN_0         /* Port P40 bit */
#define    pi4_1         P4IN_bit.P4IN_1         /* Port P41 bit */
#define    pi4_2         P4IN_bit.P4IN_2         /* Port P42 bit */
#define    pi4_3         P4IN_bit.P4IN_3         /* Port P43 bit */
#define    pi4_4         P4IN_bit.P4IN_4         /* Port P44 bit */
#define    pi4_5         P4IN_bit.P4IN_5         /* Port P45 bit */
#define    pi4_6         P4IN_bit.P4IN_6         /* Port P46 bit */
#define    pi4_7         P4IN_bit.P4IN_7         /* Port P47 bit */

/*------------------------------------------------------
  Port P4 direction register
------------------------------------------------------*/
#define    pd4           P4DIR

#define    pd4_0         P4DIR_bit.P4DIR_0        /* Port P40 direction bit */
#define    pd4_1         P4DIR_bit.P4DIR_1        /* Port P41 direction bit */
#define    pd4_2         P4DIR_bit.P4DIR_2        /* Port P42 direction bit */
#define    pd4_3         P4DIR_bit.P4DIR_3        /* Port P43 direction bit */
#define    pd4_4         P4DIR_bit.P4DIR_4        /* Port P44 direction bit */
#define    pd4_5         P4DIR_bit.P4DIR_5        /* Port P45 direction bit */
#define    pd4_6         P4DIR_bit.P4DIR_6        /* Port P46 direction bit */
#define    pd4_7         P4DIR_bit.P4DIR_7        /* Port P47 direction bit */

/*------------------------------------------------------
  Port P4 func. selection register
------------------------------------------------------*/
#define    ps4           P4SEL

#define    ps4_0         P4SEL_bit.P4SEL_0
#define    ps4_1         P4SEL_bit.P4SEL_1
#define    ps4_2         P4SEL_bit.P4SEL_2
#define    ps4_3         P4SEL_bit.P4SEL_3
#define    ps4_4         P4SEL_bit.P4SEL_4
#define    ps4_5         P4SEL_bit.P4SEL_5
#define    ps4_6         P4SEL_bit.P4SEL_6
#define    ps4_7         P4SEL_bit.P4SEL_7

/*-----------------------------------------------------
  Port P4 pull-up resistor enable register
------------------------------------------------------*/
#define    pre4          P4REN

#define    pre4_0        P4REN_bit.P4REN_0
#define    pre4_1        P4REN_bit.P4REN_1
#define    pre4_2        P4REN_bit.P4REN_2
#define    pre4_3        P4REN_bit.P4REN_3
#define    pre4_4        P4REN_bit.P4REN_4
#define    pre4_5        P4REN_bit.P4REN_5
#define    pre4_6        P4REN_bit.P4REN_6
#define    pre4_7        P4REN_bit.P4REN_7

#endif
