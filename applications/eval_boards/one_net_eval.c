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
#include "tal.h"
#include "nv_hal.h"
#include "uart.h"
#include "io_port_mapping.h"
#include "oncli.h"
#include "one_net_constants.h"
#include "one_net_xtea.h"
#include "io_port_mapping.h"
#include "oncli_str.h"
#include "one_net.h"
#include "one_net_eval.h"
#ifdef _HAS_LEDS
    #include "one_net_led.h"
#endif



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


//! Master prompt
static const char* const master_prompt = "-m";

//! Client prompt
static const char* const client_prompt = "-c";

#ifdef _AUTO_MODE
//! Auto Client prompts
static const char* const auto_client_prompts[] = {"-c1", "-c2", "-c3"};
#endif

#ifdef _SNIFFER_MODE
//! Sniffer prompt
static const char* const sniffer_prompt = "-s";
#endif



//! @} ONE-NET_eval_const
//                                  CONSTANTS END
//=============================================================================



//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_eval_pub_var
//! \ingroup ONE-NET_eval
//! @{


#ifdef _AUTO_MODE
//! True if in Auto Mode
BOOL in_auto_mode = FALSE;

//! If in auto mode and a client, the index number of the client
UInt8 auto_client_index;
#endif

//! the pins on the eval board.
user_pin_t user_pin[NUM_USER_PINS];

#ifdef _SNIFFER_MODE
//! True if in Sniffer Mode
BOOL in_sniffer_mode = FALSE;
#endif

//! Pointer to the device dependent (MASTER, CLIENT, SNIFF) function that
//! should be called in the main loop
void(*node_loop_func)(void) = 0;


	
//! @} ONE-NET_eval_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================


//=============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_eval_pri_var
//! \ingroup ONE-NET_eval
//! @{
  


//! @} ONE-NET_eval_pri_var
//                              PRIVATE VARIABLES END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_eval_pri_func
//! \ingroup ONE-NET_eval
//! @{



static const char* get_prompt_string(void);
static void eval_set_modes_from_switch_positions(void);



//! @} ONE-NET_eval_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_eval_pub_func
//! \ingroup ONE-NET_eval
//! @{


/*!
    \brief Initializes the parameters used with the user pins.
    
    \param[in] USER_PIN_TYPE List containing the state of the user pins.  If
      this is 0, then the default configuration will be used.
    \param[in] USER_PIN_COUNT The number of pins to configure.  This should be
      equal to the number of user pins, or else the default configuration will
      be used.
    
    \return void
*/
void init_user_pin(const UInt8 *user_pin_type,
  UInt8 user_pin_count)
{
} // init_user_pin //


void oncli_print_prompt(void)
{   
    oncli_send_msg("ocm%s> ", get_prompt_string());
} // oncli_print_prompt //


int main(void)
{
    INIT_PORTS();
    TAL_INIT_TRANSCEIVER();
    INIT_PROCESSOR(TRUE);

    #ifdef _HAS_LEDS
        initialize_leds();
    #endif
    
    INIT_TICK();
    FLASH_ERASE_CHECK();

    uart_init(BAUD_38400, DATA_BITS_8, STOP_BITS_1, PARITY_NONE);
    ENABLE_GLOBAL_INTERRUPTS();
    
    // startup greeting
    oncli_send_msg("\n\n");
    oncli_send_msg(ONCLI_STARTUP_FMT, ONE_NET_VERSION_MAJOR,
      ONE_NET_VERSION_MINOR);
    delay_ms(10);
    oncli_send_msg(ONCLI_STARTUP_REV_FMT, ONE_NET_VERSION_REVISION,
      ONE_NET_VERSION_BUILD);   
    oncli_send_msg("\n\n");
    delay_ms(10);

    eval_set_modes_from_switch_positions();
    
#ifdef _AUTO_MODE
	// check mode switch (Auto/Serial)
	if(in_auto_mode)
	{
		oncli_send_msg("%s\n", ONCLI_AUTO_MODE_STR);
	} // if auto mode //
	else
	{
		oncli_send_msg("%s\n", ONCLI_SERIAL_MODE_STR);
	} // else serial //
#else
	oncli_send_msg("%s\n", ONCLI_SERIAL_MODE_STR);
#endif
 
    delay_ms(10);
    oncli_print_prompt();    
    while(1)
    {
        (*node_loop_func)();
        oncli();
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



/*!
    \brief returns the string to use as part of the Command-line-interface
           prompt
        
    \return string to use as part of the Command-line-interface prompt
*/
static const char* get_prompt_string(void)
{
    #ifdef _SNIFFER_MODE
    if(in_sniffer_mode)
    {
        return sniffer_prompt;
    }
    #endif
    
    #ifdef _ONE_NET_MASTER
        #ifdef _ONE_NET_CLIENT
        if(device_is_master)
        {
            return master_prompt;
        }
        #else
        return master_prompt;
        #endif
    #endif
    
    #ifndef _AUTO_MODE
    return client_prompt;
    #else
        if(!in_auto_mode)
        {
            return client_prompt;
        }
        
        return auto_client_prompts[auto_client_index];
    #endif
}


/*!
    \brief Checks the three switches to see whether the device boots in
        auto mode, whether the device is a master or a client, and,
        if in auto mode and a client, which client the device should
        be assigned.
        
    \return none
*/
static void eval_set_modes_from_switch_positions(void)
{
    #ifdef _ONE_NET_CLIENT
        // note : this includes devices which are both master and clients
        // If that is the case and we are in master mode, it will be set
        // below
        device_is_master = FALSE;
        node_loop_func = &client_eval;
    #else
        device_is_master = TRUE;
        node_loop_func = &master_eval;    
    #endif
    
    #ifdef _AUTO_MODE
	// check mode switch (Auto/Serial)
	if(SW_MODE_SELECT == 0)
	{
		in_auto_mode = TRUE;
    }
    #endif

    #ifdef _ONE_NET_MASTER
    if((SW_ADDR_SELECT1 == 0) && (SW_ADDR_SELECT2 == 0))  
    {
        device_is_master = TRUE;
        node_loop_func = &master_eval;
    }
    #endif
    
    #ifdef _AUTO_MODE
    if(!device_is_master && in_auto_mode)
    {
        if((SW_ADDR_SELECT1 == 1) && (SW_ADDR_SELECT2 == 0))
        {
            auto_client_index = 0;
        }
        else if((SW_ADDR_SELECT1 == 0) && (SW_ADDR_SELECT2 == 1))        
        {
            auto_client_index = 1;
        }
        else       
        {
            auto_client_index = 2;
        }
    }
    #endif
}



//! @} ONE-NET_eval_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} ONE_NET_eval
