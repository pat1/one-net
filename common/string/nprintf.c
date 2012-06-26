//! \addtogroup nprintf
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
    \file nprintf.c
    \brief Contains the implementation for Xnprintf functions (ie snprint,
      vsprintf, etc).

    \Note These functions are not thread safe.  snprintf and vsprintf cannot be
      called from different threads at the same time.
*/

#include "config_options.h"


// TODO -- this is a bit messy.  Find a better #define test.
#if defined(_R8C_TINY) && !defined(QUAD_OUTPUT)
    #pragma section program program_high_rom
#endif // if _R8C_TINY and not a 16K chip //


#include "nprintf.h"

#include <stdarg.h>
#include <stdio.h>


//==============================================================================
//								CONSTANTS
//! \defgroup nprintf_const
//! \ingroup nprintf
//! @{

//! @} nprintf_const
//								CONSTANTS END
//==============================================================================

//==============================================================================
//								TYPEDEFS
//! \defgroup nprintf_typedefs
//! \ingroup nprintf
//! @{

//! @} nprintf_typedefs
//								TYPEDEFS END
//==============================================================================

//==============================================================================
//							PRIVATE VARIABLES
//! \defgroup nprintf_pri_var
//! \ingroup nprintf
//! @{

//! Pointer to the current location in the output string to store the result.
//! This is global (to this file) due to the callback to store the format ch.
static char * result_str = 0;

//! The maximum size in bytes (not including the NULL termination)
static unsigned int max_result_len = 0;

//! The current length of the result string (result string may actually be less
//! than this value if it was not big enough to hold the entire string).
static unsigned int result_len = 0;

//! @} nprintf_pri_var
//							PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//						PRIVATE FUNCTION DECLARATIONS
//! \defgroup vsnprintf_pri_func
//! \ingroup vsnprintf
//! @{

// from _print
const char * _local_print(int (*func)(int), const char *format,
  int **ap, int *total);

static int append_ch(int c);

//! @} vsnprintf_pri_func
//						PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//						PUBLIC FUNCTION IMPLEMENTATION 
//! \defgroup vsnprintf_pub_func
//! \ingroup vsnprintf
//! @{

/*!
    \brief Returns a string based on the format provided.
    
    \Note snprintf is not thread safe.
    
    \param[out] out_str Location to store the resulting string.
    \param[in] SIZE The size in bytes of the location to receive the resulting
      string.
    \param[in] FMT The format string used to create the resulting string.
    
    \return The number of bytes that would have been copied.
*/
int snprintf(char * out_str, const size_t SIZE,
  const char * const FMT, ...)
{
    int rv;
    va_list ap;
    
    va_start(ap, FMT);
    rv = vsnprintf(out_str, SIZE, FMT, ap);
    va_end(ap);
    
    return rv;
} // snprintf //


/*!
    \brief Returns a string based on the format provided.

    This function is Renesas specific and is based on the Renesas provided
    vsprintf function.
    
    \Note vsnprintf is not thread safe
    
    \param[out] out_str Location to store the resulting string.
    \param[in] SIZE The size in bytes of the location to receive the resulting
      string.
    \param[in] FMT The format string used to create the resulting string
    \param[in] ap variable length argument list to use with the placeholders in
      FMT.

    \return The number of bytes that would have been copied.  
*/
int vsnprintf(char * out_str, const size_t SIZE,
  const char * FMT, va_list ap)
{
    if(!out_str || !SIZE || !FMT)
    {
        return -1;
    } // if any of the parameters are invalid //

    result_str = out_str;
    result_len = 0;

    // -1 to save 1 byte for NULL termination.
    max_result_len = SIZE - 1;

    while(FMT && *FMT)
    {
        if(*FMT == '%')
        {
            FMT++;
			if(*FMT == '%')
            {
                append_ch(*FMT);
                FMT++;
			} // if a '%' should be added //
            else
            {
                int bytes_added;
                
                FMT = _local_print(append_ch, FMT, (int **)&ap, &bytes_added);
            } // else fill in the place holder //
        } // if a '%' //
        else
        {
            append_ch(*FMT++);
		} // else it's not '%' //
	} // while characters remain //

	*result_str = '\0';
    result_str = 0;                 // no longer operating on that data location
    max_result_len = 0;
	return result_len;
} // vsnprintf //

//! @} vsnprintf_pub_func
//						PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//						PRIVATE FUNCTION IMPLEMENTATION 
//! \addtogroup vsnprintf_pri_func
//! \ingroup vsnprintf
//! @{

/*!
    \brief Appends a character to the resulting string in the vsnprintf
      operation.
    
    This is also the callback for _print to append the character to the
    resulting string.
    
    \Note append_ch is not thread safe
    
    \param[in] c The character to add.
    
    \return The number of bytes added.
*/
static int append_ch(int c)
{
    if(!result_str || !max_result_len)
    {
        return EOF;
    } // if the variables have not been set up //

    if(result_len < max_result_len)
    {
	    *result_str++ = c;
    } // if room to add a character //
    
    result_len++;
	return 1;
} // append_ch //

//! @} vsnprintf_pri_func
//						PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} vsnprintf
