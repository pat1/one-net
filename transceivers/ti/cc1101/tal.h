#ifndef _TAL_H
#define _TAL_H

//! \defgroup TAL Transceiver Abstraction Layer
//! @{

/*
    Copyright (c) 2011, Threshold Corporation
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

        * Redistributions of source code must retain the above copyright notice,
          this list of conditions, and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in the
          documentation and/or other materials provided with the distribution.
        * Neither the name of Threshold Corporation (trustee of ONE-NET) nor the
          names of its contributors may be used to endorse or promote products
          derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
    CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
    BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
    OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
    BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
    NEGLIGENCE OR OTHEWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*!
    \file tal.h (for TI)
    \brief Defines the transceiver specific layer for ONE-NET code.

    You must create a transceiver specific version of this file for each
    transceiver.  The tal.h file in each transceiver directory defines the
    interface that should be used to access all transceiver specific
    functionality. 
*/
//==============================================================================
//                                  CONSTANTS
//! \defgroup TAL_const
//! \ingroup TAL
//! @{

//! @} TAL_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup TAL_type_defs
//! \ingroup TAL
//! @{

//! @} TAL_type_defs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup TAL_pub_var
//! \ingroup TAL
//! @{

//! @} TAL_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                          PUBLIC FUNCTION DECLARATIONS
//! \defgroup TAL_pub_func
//! \ingroup TAL
//! @{

void tal_init_ports(void);

void tal_init_transceiver(void);

/*!
    \brief Initialize (bit by bit) the i/o ports between the processor and the
      transceiver.

    \param void

    \return void
*/
#define TAL_INIT_PORTS() tal_init_ports()


/*!
    \brief Initialize the transceiver. 

    This function should perform any one of a kind transceiver initialization.
    There are other functions for turning on the transmitter and receiver.
    However, any initialization that does not need to be repeated when turning
    on the transmitter or recevier should be done in this function so that less
    time is needed to turn on the transmitter or receiver.

    \param void

    \return void
*/
#define TAL_INIT_TRANSCEIVER() tal_init_transceiver()


#ifdef _CHIP_ENABLE
    /*!
        \brief Enables the transceiver

        \param void

        \return void
    */
    #define ENABLE_TRANSCEIVER() CHIP_ENABLE = 1
    
    /*!
        \brief Disables the transceiver

        \param void

        \return void
    */
    #define DISABLE_TRANSCEIVER() CHIP_ENABLE = 0
#else // ifdef _CHIP_ENABLE //
    /*!
        \brief Enables the transceiver

        \param void

        \return void
    */
    #define ENABLE_TRANSCEIVER()
    
    /*!
        \brief Disables the transceiver

        \param void

        \return void
    */
    #define DISABLE_TRANSCEIVER()
#endif // else _CHIP_ENABLE is not defined //


/*!
    \brief Initialize interrupts needed for ADI 7025 operation.

    \param void

    \return void
*/
#define INIT_RF_INTERRUPTS()


/*!
    \brief Turn on the transceiver's receiver.

    This function tells the transceiver to turn on its receiver. If the receiver
    needs some time to come up to full operation, this function should wait that
    amount of time so that upon exit the receiver is ready.

    \param void

    \return void
*/
#define TAL_TURN_ON_RECEIVER() tal_turn_on_receiver()


/*
 *  This macro is for use by other macros to form a fully valid C statement.
 *  Without this, the if/else conditionals could show unexpected behavior.
 *
 *  For example, use...
 *    #define SET_REGS()  st( ioreg1 = 0; ioreg2 = 0; )
 *  instead of ...
 *    #define SET_REGS()  { ioreg1 = 0; ioreg2 = 0; }
 *  or
 *    #define  SET_REGS()    ioreg1 = 0; ioreg2 = 0;
 *  The last macro would not behave as expected in the if/else construct.
 *  The second to last macro will cause a compiler error in certain uses
 *  of if/else construct
 *
 *  It is not necessary, or recommended, to use this macro where there is
 *  already a valid C statement.  For example, the following is redundant...
 *    #define CALL_FUNC()   st(  func();  )
 *  This should simply be...
 *    #define CALL_FUNC()   func()
 *
 * (The while condition below evaluates false without generating a
 *  constant-controlling-loop type of warning on most compilers.)
 */
#define st(x)      do { x } while (__LINE__ == -1)

//! @} TAL_pub_func
//!                         PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} TAL

#endif

