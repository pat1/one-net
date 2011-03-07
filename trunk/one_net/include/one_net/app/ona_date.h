#ifndef _ONA_DATE_H
#define _ONA_DATE_H

//! \defgroup ONE-NET_APP_date ONE-NET Application Layer - Date
//! \ingroup ONE-NET_APP
//! @{

/*
    Copyright (c) 2010, Threshold Corporation
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
    \file ona_date.h
    \brief Declarations for ONE-NET date msgs

    These functions should be used to send and parse date messages.
*/

#include <one_net/one_net.h>
#include <one_net/port_specific/one_net_types.h>
#include <one_net/one_net_status_codes.h>
#include <one_net/one_net_application.h>


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_APP_date_const
//! \ingroup ONE-NET_APP_date
//! @{

//! The month
typedef enum
{
    ONA_JANUARY,
    ONA_FEBRUARY,
    ONA_MARCH,
    ONA_APRIL,
    ONA_MAY,
    ONA_JUNE,
    ONA_JULY,
    ONA_AUGUST,
    ONA_SEPTEMBER,
    ONA_OCTOBER,
    ONA_NOVEMBER,
    ONA_DECEMBER
} ona_month_t;

//! The day of the week
typedef enum
{
    ONA_MONDAY,
    ONA_TUESDAY,
    ONA_WEDNESDAY,
    ONA_THURSDAY,
    ONA_FRIDAY,
    ONA_SATURDAY,
    ONA_SUNDAY
} ona_day_of_week_t;

enum
{
    //! The year used as a basis
    ONA_BASE_YEAR = 2000,

    //! The maximum year that can be sent
    ONA_MAX_YEAR = ONA_BASE_YEAR + 0x7F,

    //! Max value a day can be
    ONA_MAX_DAY = 31
};

//! @} ONE-NET_APP_date_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_APP_date_typedefs
//! \ingroup ONE-NET_APP_date
//! @{

//! @} ONE-NET_APP_date_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_APP_date_pub_var
//! \ingroup ONE-NET_APP_date
//! @{

//! @} ONE-NET_APP_date_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_APP_date_pub_func
//! \ingroup ONE-NET_APP_date
//! @{

one_net_status_t ona_send_date_status(const UInt16 YEAR,
  const ona_month_t MONTH, const UInt8 DAY, const ona_day_of_week_t DOW,
  const one_net_raw_did_t * const RAW_DST);

one_net_status_t ona_send_date_query(const one_net_raw_did_t * const RAW_DST);

one_net_status_t ona_parse_date(const UInt8 * const MSG_DATA, const UInt8 LEN,
  UInt16 * const year, ona_month_t * const month, UInt8 * const day,
  ona_day_of_week_t * dow);

//! @} ONE-NET_APP_date_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_APP_date

#endif // _ONA_DATE_H //

