//! \defgroup ADI ADI ADF7025 driver.
//! \ingroup TAL
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
    \file adi.c
    \brief Driver for the ADI ADF7025.

    This file implements the communications necessary for communicating with the
    ADI ADF7025.  It abstracts the actual processor pin assignments using
    definitions found in io_port_mapping.h.  This file also contains the
    implementation for the transceiver specific functions.
*/

#include "config_options.h"
#include "io_port_mapping.h"
#include "tal.h"
#include "one_net_status_codes.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ADI_const
//! \ingroup ADI
//! @{



//! @} ADI_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ADI_typedefs
//! \ingroup ADI
//! @{

//! @} ADI_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ADI_pri_var
//! \ingroup ADI
//! @{

//! @} ADI_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ADI_pri_func
//! \ingroup ADI
//! @{


static void write_reg(const UInt8 * const REG, const BOOL CLR_SLE);
static void turn_off_agc(void);
static void turn_on_agc(void);


//! @} ADI_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ADI_pub_func
//! \ingroup ADI
//! @{



/*!
    \brief Initializes the ADI transceiver.

    This function is transceiver specific. It should configure the transceiver
    for ONE-NET operation so that switching between modes (receive/transmit)
    can be accomplished with as few commands as possible.
    
    This function also makes the call to initialize the pins needed by this
    transceiver.

    \param void

    \return void
*/
void tal_init_transceiver(void)
{
} // tal_init_transceiver //


void tal_enable_transceiver(void)
{
} // tal_enable_transceiver //


void tal_disable_transceiver(void)
{
} // tal_disable_transceiver //


BOOL tal_channel_is_clear(void)
{
    return TRUE;
} // tal_channel_is_clear //


UInt16 tal_write_packet(const UInt8 * data, const UInt16 len)
{
    return 0;
}


BOOL tal_write_packet_done()
{
    return TRUE;
}


UInt16 tal_read_bytes(UInt8 * data, const UInt16 len)
{
}


one_net_status_t tal_look_for_packet(tick_t duration)
{
    return ONS_FAIL;
}


one_net_status_t tal_set_data_rate(UInt8 data_rate)
{
    return ONS_SUCCESS;
}


one_net_status_t tal_set_channel(const UInt8 channel)
{
    return ONS_SUCCESS;
}









/*!
    \brief Sets the transceiver to receive mode

    \param void

    \return void
*/
void tal_turn_on_receiver(void)
{
} // tal_turn_on_receiver //


/*!
    \brief Sets the transceiver to transmit mode

    \param void

    \return void
*/
void tal_turn_on_transmitter(void)
{
} // tal_turn_on_transmitter //


/*!
    \brief Returns the RSSI reported by the ADI in dBm.

    \param void

    \return The RSSI reading from the ADI in dBm.
*/
UInt16 read_rssi(void)
{
    return 0;
} // read_rssi //


/*!
    \brief Returns the battery status reported by the ADI

    \param void

    \return The battery status.
*/
UInt16 read_battery(void)
{
    return 0;
} // read_battery //


/*!
    \brief Returns the adc value reported by the ADI

    Returns the ADC reading on the ADCIN pin.

    \param void

    \return The adc reading.
*/
UInt16 read_adc(void)
{
    return 0;
} // read_adc //


UInt16 read_revision(void)
{
    return 0;
} // read_revision //


/*!
    \brief Reads information from the ADI.

    Write the four bytes passed in the array to the registers, then reads two
    back from the transceiver.  This function is considered protected, and is
    derived by adi_dbg_code.c.

    \param[in] MSG The information to read.

    \return The information read.
*/
UInt16 adi_7025_read(const UInt8 * const MSG)
{
    return 0;
} /* adi_7025_read */


/*!
    \brief Calculates the rssi.
    
    The value passed in should be the readback_code returned by the transceiver.
    This function will convert the read back code to a RSSI reading in dBm.

    \param[in] READBACK_CODE The value returned by the ADI.

    \return The RSSI value in dBm.
*/
UInt16 calc_rssi(const UInt16 READBACK_CODE)
{
    return 0;
} // calc_rssi //



//! @} ADI_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ADI_pri_func
//! \ingroup ADI
//! @{


/*!
    \brief Writes the 4 byte register value.
    
    Sets up the lines before and sends the register data.  After the write is
    complete, the SLE line is set.  Optionionally clears the SLE based on
    CLR_SLE.
    
    \param[in] REG The 4 bytes to write out.
    \param[in] CLR_SLE TRUE if write_reg should clear the SLE line before
                         returning.
                       FALSE if write_reg should leave SLE high

    \return void
*/
static void write_reg(const UInt8 * const REG, const BOOL CLR_SLE)
{
} // write_reg //


/*!
    \brief Turns off the agc.

    \param void

    \return void
*/
static void turn_off_agc(void)
{
} // turn_off_agc //


/*!
    \brief Turns on the agc

    \param void

    \return void
*/
static void turn_on_agc(void)
{
} // turn_on_agc //


//! @} ADI_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ADI
