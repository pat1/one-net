//! \defgroup ONE-NET_sniff_eval ONE-NET Packet Sniffer
//! \ingroup ONE-NET_eval
//! @{

/*!
    \file sniff_eval.c
    \brief The packet sniffer part of the ONE-NET evaluation project.
*/

#include "config_options.h"

#ifdef _SNIFFER_MODE

#if _DEBUG_VERBOSE_LEVEL == 0
    #error "_DEBUG_VERBOSE_LEVEL must be greater than 0 if _SNIFFER_MODE is enabled"
#endif



#include "oncli.h"
#include "oncli_port.h"
#include "uart.h"
#include "one_net_xtea.h"
#include "one_net_encode.h"
#include "oncli_str.h"
#include "one_net_timer.h"
#include "tick.h"
#include "one_net.h"
#include "one_net_port_specific.h"
#include "one_net_eval.h"
#include "hal.h"
#include "tal.h"
#include "oncli.h"
#include "cb.h"
#include "one_net_crc.h"
#include "io_port_mapping.h"



//=============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_sniff_eval_const
//! \ingroup ONE-NET_sniff_eval
//! @{


extern const char HEX_DIGIT[];
extern const UInt8 HEADER[];
extern BOOL printit;
extern UInt8 rcv_idx;
extern SInt8 prnt_idx;
extern UInt8 encoded_pkt_bytes[SNIFF_PKT_BUFFER_SIZE][ON_MAX_ENCODED_PKT_SIZE];
extern tick_t pkt_time[SNIFF_PKT_BUFFER_SIZE];
extern UInt8 pkt_size[SNIFF_PKT_BUFFER_SIZE];



//! @} ONE-NET_sniff_eval_const
//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_sniff_eval_typedefs
//! \ingroup ONE-NET_sniff_eval
//! @{

//! @} ONE-NET_sniff_eval_typedefs
//                                  TYPEDEFS END
//=============================================================================

//=============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_sniff_eval_pri_var
//! \ingroup ONE-NET_sniff_eval
//! @{


//! The channel currently being sniffed
static UInt8 sniff_channel;

//! Buffer to hold the string representation of the channel being sniffed
static char channel_format_buffer[MAX_CHANNEL_STRING_FORMAT_LENGTH];

#if _DEBUG_VERBOSE_LEVEL > 2
enum
{
    //! number of known invite keys to try for decryption.
    NUM_SNIFF_INVITE_KEYS = 2,
    
    //! number of known encryption keys to try for decryption.
    NUM_SNIFF_ENCRYPT_KEYS = 2,
};
#endif


static tick_t sniff_start_time = 0;
static tick_t sniff_duration_ms = 0;



//! @} ONE-NET_sniff_eval_pri_var
//                              PRIVATE VARIABLES END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_sniff_eval_pri_func
//! \ingroup ONE-NET_eval
//! @{

#if _DEBUG_VERBOSE_LEVEL > 2
//! Place any known invite keys in the array below
static const one_net_xtea_key_t sniff_invite_keys[NUM_SNIFF_INVITE_KEYS] =
{
    {'2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2'},
    {'3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3'}
};

//! Place any known encryption keys in the array below
static const one_net_xtea_key_t sniff_enc_keys[NUM_SNIFF_ENCRYPT_KEYS] =
{
    {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
     0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F},
     
    // use with "change key:44-44-44-44" command
    {0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,
     0x0C,0x0D,0x0E,0x0F,0x44,0x44,0x44,0x44}
};
#endif



//! @} ONE-NET_sniff_eval_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_sniff_eval_pub_func
//! \ingroup ONE-NET_sniff_eval
//! @{
    


// If sniff_time_ms is non-zero, timestamps will start at 0 and packets will
// only be siffed for a certain time period.  For example, if sniff_time_ms
// is equal to 3500, the first packet sniffed will be given a timestamp of 0.
// Any packets received more than 3.5 seconds after the first packet will not
// be displayed.
oncli_status_t oncli_reset_sniff(const UInt8 CHANNEL, tick_t sniff_time_ms)
{
    if(oncli_format_channel(CHANNEL, channel_format_buffer,
      MAX_CHANNEL_STRING_FORMAT_LENGTH) != channel_format_buffer)
    {
        // error
        return ONCLI_INTERNAL_ERR;
    }
    
    tal_set_channel(CHANNEL);
    tal_turn_on_receiver();
    in_sniffer_mode = TRUE;
    sniff_channel = CHANNEL;
    node_loop_func = &sniff_eval;
    one_net_set_channel(sniff_channel);
    sniff_duration_ms = sniff_time_ms;
    sniff_start_time = 0;
    return ONCLI_SUCCESS;
} // oncli_reset_sniff //


/*!
    \brief The packet sniffer evaluation application

    This is the main function for the packet sniffer.

    \param void

    \return void
*/
void sniff_eval(void)
{
    typedef enum
    {
        NOT_PRINTING,
        PRINT_TIMESTAMP,
        PRINT_RECEIVED_STRING,
        PRINT_NUM_BYTES,
        PRINT_BYTES_STRING,
        PRINT_HEADER,
        PRINT_BYTES
    } sniff_print_stage_t;


    static UInt16 bytes_read;
    static UInt8 index;
    static sniff_print_stage_t print_stage;
    static BOOL init = FALSE;
    static tick_t pkt_rcv_time;
    static UInt8* pkt_wo_header;

    if(!init)
    {
        init = TRUE;
        print_stage = NOT_PRINTING;
    }

    if(oncli_user_input())
    {
        return;
    } // if there has been user input //

    if(one_net_look_for_pkt(50) != ONS_SUCCESS)
    {
        if(print_stage == NOT_PRINTING && prnt_idx == -1)
        {
            return;
        }
    } // if SOF was not received //
    
    if(!printit)
    {
        return;
    }

    if(print_stage == NOT_PRINTING)
    {
        bytes_read = one_net_read(&pkt_wo_header, &pkt_rcv_time);
        if(bytes_read > 0)
        {
            print_stage = PRINT_TIMESTAMP;
            index = 0;
        }
        return;
    }
    
    
    switch(print_stage)
    {
        case PRINT_TIMESTAMP:
            oncli_send_msg("\n\n%lu", pkt_rcv_time);
            print_stage++;
            break;
        case PRINT_RECEIVED_STRING:
            oncli_send_msg(" received ");
            print_stage++;
            break;        
        case PRINT_NUM_BYTES:
            oncli_send_msg("%u", bytes_read + ONE_NET_PREAMBLE_HEADER_LEN);
            print_stage++;
            break;        
        case PRINT_BYTES_STRING:
            oncli_send_msg(" bytes:");
            print_stage++;
            index = 0;
            break;
        case PRINT_HEADER:
            if(index >= ONE_NET_PREAMBLE_HEADER_LEN)
            {
                print_stage++;
                break;
            }
            else if(index == 0)
            {
                oncli_send_msg("\n%02X", HEADER[index]);
            }
            else
            {
                oncli_send_msg(" %02X", HEADER[index]);
            }
            index++;
            break;            
        case PRINT_BYTES:
            if(index >= bytes_read + ONE_NET_PREAMBLE_HEADER_LEN)
            {
                oncli_send_msg("\n\n");
                print_stage = NOT_PRINTING;
                return;
            }
            else if(index % 26 == 0)
            {
                oncli_send_msg("\n%02X",
                  pkt_wo_header[index - ONE_NET_PREAMBLE_HEADER_LEN]);
            }
            else
            {
                oncli_send_msg(" %02X",
                  pkt_wo_header[index - ONE_NET_PREAMBLE_HEADER_LEN]);
            }
            index++;
            break;
    }
} // sniff_eval //



//! @} ONE-NET_sniff_eval_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_sniff_eval_pri_func
//! \ingroup ONE-NET_sniff_eval
//! @{



//! @} ONE-NET_sniff_eval_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

#endif

//! @} ONE_NET_sniff_eval
