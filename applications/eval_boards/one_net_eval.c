//! \addtogroup ONE-NET_eval ONE_NET Evaluation
//! @{

/*!
    \file one_net_eval.c
    \brief The ONE-NET evaluation project.

    This is the application that runs on the ONE-NET evaluation boards.
*/



#include "config_options.h"
#include "tick.h"
#include "pal.h"
#include "hal.h"
#include "uart.h"
#include "io_port_mapping.h"
#include "oncli.h"
#include "one_net_constants.h"
#include "one_net_xtea.h"
#include "io_port_mapping.h"
#include "oncli_str.h"



//=============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_eval_typedefs
//! \ingroup ONE-NET_eval
//! @{


enum
{
    //! The time in milliseconds to leave the LEDs on. 50ms
    EVAL_LED_ON_TIME = 50,
    
    #ifdef _AUTO_MODE
    //! The number of clients in AUTO mode.
    NUM_AUTO_CLIENTS = 3
    #endif
};


//! @} ONE-NET_eval_typedefs
//                                  TYPEDEFS END
//=============================================================================


//=============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_eval_const
//! \ingroup ONE-NET_eval
//! @{



#ifdef _AUTO_MODE
	//! The raw CLIENT DIDs for auto mode
	static const on_raw_did_t RAW_AUTO_CLIENT_DID[NUM_AUTO_CLIENTS] =
	{
	    {0x00, 0x20}, {0x00, 0x30}, {0x00, 0x40}
	};
#endif

//! The key used in the evaluation network ("protected")
const one_net_xtea_key_t EVAL_KEY = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
  0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

#ifdef _STREAM_MESSAGES_ENABLED
//! The key to use for stream transactions in the eval network ("protected")
const one_net_xtea_key_t EVAL_STREAM_KEY = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
  0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
#endif

//! Default invite key to use if no manufacturing data (SID and invite key) segment
//! is found in data flash.
const UInt8 DEFAULT_INVITE_KEY[] = { '2', '2', '2', '2',   '2', '2', '2', '2',
                                     '2', '2', '2', '2',   '2', '2', '2', '2'};

//! Default SID to use if no manufacturing data (SID and invite key) segment
//! is found in data flash.
const UInt8 DEFAULT_RAW_SID[] =        { 0x00, 0x00, 0x00, 0x00, 0x10, 0x01 };



//! @} ONE-NET_eval_const
//                                  CONSTANTS END
//=============================================================================



//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_eval_pub_var
//! \ingroup ONE-NET_eval
//! @{

	
//! @} ONE-NET_eval_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================


//=============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_eval_pri_var
//! \ingroup ONE-NET_eval
//! @{
  
    
#ifdef _AUTO_MODE
//! True if in Auto Mode
static BOOL in_auto_mode;

//! If in auto mode and a client, the index number of the client
static UInt8 auto_client_index;
#endif


//! @} ONE-NET_eval_pri_var
//                              PRIVATE VARIABLES END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_eval_pri_func
//! \ingroup ONE-NET_eval
//! @{



//! @} ONE-NET_eval_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_eval_pub_func
//! \ingroup ONE-NET_eval
//! @{



void oncli_print_prompt(void)
{
    oncli_send_msg("ocm> ");
} // oncli_print_prompt //


int main(void)
{
    UInt8 i;
    const char* const message = "Hello World!";
    
    INIT_PROCESSOR(TRUE);
    INIT_PORTS_LEDS();
    uart_init(BAUD_38400, DATA_BITS_8, STOP_BITS_1, PARITY_NONE);
    ENABLE_GLOBAL_INTERRUPTS();
    
#ifdef _AUTO_MODE
	// check mode switch (Auto/Serial)
	if(SW_MODE_SELECT == 0)
	{
		in_auto_mode = TRUE;
		oncli_send_msg("%s\n", ONCLI_AUTO_MODE_STR);
	} // if auto mode //
	else
	{
		in_auto_mode = FALSE;
		oncli_send_msg("%s\n", ONCLI_SERIAL_MODE_STR);
	} // else serial //
#else
	oncli_send_msg("%s\n", ONCLI_SERIAL_MODE_STR);
#endif    
    
    
    
    
    USER_PIN0_DIR = OUTPUT;
    USER_PIN1_DIR = OUTPUT;
    USER_PIN2_DIR = OUTPUT;
    USER_PIN3_DIR = OUTPUT;
    
    USER_PIN0 = 0;
    USER_PIN1 = 0;
    USER_PIN2 = 0;
    USER_PIN3 = 0;
    delay_ms(3000);
    
    {
        UInt16 len = 12;
        uart_write(message, len);
        delay_ms(2000);
    }
    
    
    for(i = 0; i < 10; i++)
    {
        TOGGLE (USER_PIN0);
        delay_ms(200);
        TOGGLE (USER_PIN1);
        delay_ms(200);
        TOGGLE (USER_PIN2);
        delay_ms(200);
        TOGGLE (USER_PIN3);
        delay_ms(1000);
    }
    
    oncli_send_msg("\n:Done:  String=%s, i=%d", message, i);
    delay_ms(3000);
    
    while(1)
    {
        oncli();
        delay_ms(25);
    }
    
    
    EXIT();
	return 0;
} // main //



//! @} ONE-NET_eval_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_eval_pri_func
//! \ingroup ONE-NET_eval
//! @{


//! @} ONE-NET_eval_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} ONE_NET_eval
