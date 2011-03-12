#ifndef SFR_F5438
#define SFR_F5438

#include "io430.h"

#define CLK_PORT_DIR      P11DIR
#define CLK_PORT_OUT      P11OUT
#define CLK_PORT_SEL      P11SEL

#define ACLK_PIN          BIT0
#define MCLK_PIN          BIT1
#define SMCLK_PIN         BIT2

#define XT1_XTAL_DIR      P7DIR
#define XT1_XTAL_SEL      P7SEL
#define XT1_XTAL_OUT      P7OUT

#define SYSCLK_1MHZ             0
#define SYSCLK_4MHZ             1
#define SYSCLK_8MHZ             2
#define SYSCLK_12MHZ            3
#define SYSCLK_16MHZ            4
#define SYSCLK_20MHZ            5
#define SYSCLK_25MHZ            6

#define DCO_MULT_1MHZ           30
#define DCO_MULT_4MHZ           122
#define DCO_MULT_8MHZ           244
#define DCO_MULT_12MHZ          366
#define DCO_MULT_16MHZ          488
#define DCO_MULT_20MHZ          610
#define DCO_MULT_25MHZ          763

#define DCORSEL_1MHZ            DCORSEL_2
#define DCORSEL_4MHZ            DCORSEL_4
#define DCORSEL_8MHZ            DCORSEL_4
#define DCORSEL_12MHZ           DCORSEL_5
#define DCORSEL_16MHZ           DCORSEL_5
#define DCORSEL_20MHZ           DCORSEL_6
#define DCORSEL_25MHZ           DCORSEL_7

// Due to erratum FLASH28 the expected VCORE settings, as follows,
// cannot be achieved. The Vcore setting should not be changed. 
//#define VCORE_1MHZ              PMMCOREV_0
//#define VCORE_4MHZ              PMMCOREV_0
//#define VCORE_8MHZ              PMMCOREV_0
//#define VCORE_12MHZ             PMMCOREV_0
//#define VCORE_16MHZ             PMMCOREV_1
//#define VCORE_20MHZ             PMMCOREV_2
//#define VCORE_25MHZ             PMMCOREV_3
#define VCORE_1MHZ              PMMCOREV_2
#define VCORE_4MHZ              PMMCOREV_2
#define VCORE_8MHZ              PMMCOREV_2
#define VCORE_12MHZ             PMMCOREV_2
#define VCORE_16MHZ             PMMCOREV_2

// Due to erratum FLASH28 the expected VCORE settings, as follows,
// cannot be achieved. The Vcore setting should not be changed.
//#define VCORE_1_35V             PMMCOREV_0
//#define VCORE_1_55V             PMMCOREV_1
#define VCORE_1_75V             PMMCOREV_2
//#define VCORE_1_85V             PMMCOREV_3

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
