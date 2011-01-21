//! \defgroup ONE-NET_port_specific Application Specific ONE-NET functionality
//! \ingroup ONE-NET
//! @{

/*!
    \file one_net_port_specific.c
    \brief Application specific ONE-NET definitions.

*/

#include "one_net_port_specific.h"

#include <stdio.h>
#include <string.h>                 // for one_net_memmove

#ifdef _ONE_NET_DEBUG
#include "debug.h"
#endif
#include "rf_shm.h"

//=============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_port_specific_const
//! \ingroup ONE-NET_port_specific
//! @{

//timing constants
const UInt32 USECS_PER_TICK  = 1000;
const UInt32 USECS_PER_SEC   = 1000000;
const UInt8  BITS_PER_BYTE   = 8;
const UInt32 BITRATE         = 40000;    //40 Kbps

//debug constants
const char * const READING = "Reading";
const char * const LISTENING = "Listening...";
const char * const HEARD = "Heard";
const char * const WRITING = " Writing";

enum
{
    DEBUG_BUFFER_LEN = 256,
};

//! @} ONE-NET_port_specific_const
//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_port_specific_typedefs
//! \ingroup ONE-NET_port_specific
//! @{
    
//! @} ONE-NET_port_specific_typedefs
//                                  TYPEDEFS END
//=============================================================================

//=============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_port_specific_pub_var
//! \ingroup ONE-NET_port_specific
//! @{


//! @} ONE-NET_port_specific_pub_var
//                              PUBLIC VARIABLES END
//=============================================================================

//=============================================================================
//                              PRIVATE VARIABLES


//! "Protected" base parameters variable inherited from one_net.c.  Should not
//! be doing this, but the sending did needs to be known to test repeating MH
//! packets.
extern on_base_param_t base_param;

//! Packet buffer/index populated in one_net_look_for_pkt
static UInt8 pkt_buffer[ONE_NET_MAX_ENCODED_PKT_LEN];
static UInt8 pkt_buffer_i = 0;

//! The channel the device is on
static UInt8 channel = 0;

//! The data rate the packet was transmitted at
static UInt8 data_rate = 0;

//! The time it would take to write the data.
static tick_t write_time = 0;

//                              PRIVATE VARIABLES END
//=============================================================================

//=============================================================================
//                              PRIVATE FUNCTION DECLARATIONS

static void one_net_sleep(tick_t DURATION);

//                              PRIVATE FUNCTION DECLARATIONS END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_port_specific_pub_func
//! \ingroup ONE-NET_port_specific
//! @{

/*!
    \brief Copies LEN bytes from SRC to dst

    \param[out] dst The mem location to receive the bytes
    \param[in] SRC The bytes to copy.
    \param[in] LEN The number of bytes to copy.
    \return void
*/
void * one_net_memmove(void * dst, const void * SRC, size_t len)
{
    return(one_net_memmove(dst, SRC, len));
} // one_net_memmove //


/*!
    \brief Convert a byte stream to a 16-bit value, accounting for endianness

    \param[in] BYTE_STREAM The 2 bytes to convert to a 16-bit value, accounting
      for the endianess of the processor.

    \return The 16 bit value contained in the 2 bytes of BYTE_STREAM
*/
UInt16 one_net_byte_stream_to_int16(const UInt8 * const BYTE_STREAM)
{
    UInt16 val;

    if(!BYTE_STREAM)
    {
        return 0;
    } // if parameters are invalid //

    val = (((UInt16)BYTE_STREAM[0]) << 8) & 0xFF00;
    val |= ((UInt16)BYTE_STREAM[1]) & 0x00FF;

    return val;
} // byte_stream_to_Int16 //


/*!
    \brief Convert a 16 bit value to a MSB first stream.

    \param[in] VAL The value to convert to a byte stream
    \param[out] byte_stream The location to return the byte stream

    \return void
*/
void one_net_int16_to_byte_stream(const UInt16 VAL, UInt8 * const byte_stream)
{
    if(!byte_stream)
    {
        return;
    } // if parameters are invalid //

    byte_stream[0] = (UInt8)(VAL >> 8);
    byte_stream[1] = (UInt8)VAL;
} // one_net_int16_to_byte_stream //


/*!
    \brief Convert a byte stream to a 32-bit value, accounting for endianness

    \param[in] BYTE_STREAM The 4 bytes to convert to a 32-bit value, accounting
      for the endianess of the processor.

    \return The 32 bit value contained in the 4 bytes of BYTE_STREAM
*/
UInt32 one_net_byte_stream_to_int32(const UInt8 * const BYTE_STREAM)
{
    UInt32 val = 0;

    val = (((UInt32)BYTE_STREAM[0]) << 24) & 0xFF000000;
    val |= (((UInt32)BYTE_STREAM[1]) << 16) & 0x00FF0000;
    val |= (((UInt32)BYTE_STREAM[2]) << 8) & 0x0000FF00;
    val |= ((UInt32)BYTE_STREAM[3]) & 0x000000FF;

    return val;
} // one_net_byte_stream_to_int32 //


/*!
    \brief Convert a 32 bit value to a MSB first stream.

    \param[in] VAL The value to convert to a byte stream
    \param[out] byte_stream The location to return the byte stream

    \return void
*/
void one_net_int32_to_byte_stream(const UInt32 VAL, UInt8 * const byte_stream)
{
    byte_stream[0] = (UInt8)(VAL >> 24);
    byte_stream[1] = (UInt8)(VAL >> 16);
    byte_stream[2] = (UInt8)(VAL >> 8);
    byte_stream[3] = (UInt8)VAL;
} // one_net_int32_to_byte_stream //


/*!
    \brief Returns the number of ticks since bootup.
    
    1 Tick = 1 millisecond
    
    \param void
    \return The numer of ticks since bootup
*/
tick_t one_net_tick(void)
{
    tick_t ticks = 0;
    
    acquire_lock(TICK_SEM);
    ticks = *global_tick_ptr;      //TBD: may need a semaphore around this
    release_lock(TICK_SEM);
     
    return ticks;

} // one_net_tick //


/*!
    \brief Changes the channel the device is on

    \param[in] CHANNEL The channel to change to (0-based).

    \return void
*/
void one_net_set_channel(const UInt8 CHANNEL)
{
    channel = CHANNEL;
} // change channel //


/*!
    \brief Checks the channel to see if it is clear.

    This function performs the Carrier Sense.  It is called before a device
    transmits.

    \param void

    \return TRUE if the channel is clear
            FALSE if the channel is currently in use.
*/
BOOL one_net_channel_is_clear()
{
    if(try_lock(RFC_SEM))
    {
        if(*rf_channel_ptr == 0x00)
        {
            return TRUE;
        } // if the channel is clear //

        // channel is busy, so release the lock
        release_lock(RFC_SEM);
    } // if the lock was aquired //

    return FALSE;
} // one_net_channel_is_clear //


/*!
    \brief Changes the data rate the device is operating at.

    The ONE-NET code does not keep track if it is changing the data rate to a
    rate that is already set.  It is up to the implementer to check this.

    \param[in] DATA_RATE The data rate to set the transceiver to.  See
      data_rate_t for values.

    \return void
*/
void one_net_set_data_rate(const UInt8 DATA_RATE)
{
    data_rate = DATA_RATE;
} // one_net_set_data_rate //


/*!
    \brief Waits a specified number of ticks for receiption of a packet.
    
    \param[in] DURATION Time in ticks to look for a packet

    \return ONS_SUCCESS if a packet has been received
*/
one_net_status_t one_net_look_for_pkt(const tick_t DURATION)
{
    UInt8 i, rx_size, pid;
    UInt32 sleep_usecs = 0;
    tick_t sleep_ticks = 0;
    tick_t now, end, msg_sent, listen;
    one_net_status_t rv = ONS_TIME_OUT;
    BOOL got_pattern = FALSE;
//DEBUG
int lock_count = 0;

    //remember when we started looking
    listen = one_net_tick();
    end = listen + DURATION;

#ifdef _ONE_NET_DEBUG
    print_debug("%s\n", LISTENING);
#endif
    while(!got_pattern && (now = one_net_tick()) < end)
    {
        acquire_lock(RFC_SEM);
//DEBUG
lock_count++;

        //check timestamp
        msg_sent = *timestamp_ptr;
        if(msg_sent < listen)
        {
            release_lock(RFC_SEM);
        } //if listened after sent (you missed it)
        #ifdef HEAR_C1_ONLY
            // This is used to simulate Multi-Hop.  This device can only
            // hear packets from CLIENT 1
            else if((got_pattern = (channel == *channel_num_ptr
              && data_rate == *data_rate_ptr
              && rf_channel_ptr[0] == 0x55 && 
              rf_channel_ptr[1] == 0x55 && rf_channel_ptr[2] == 0x55 && 
              rf_channel_ptr[3] == 0x33 && rf_xmtr_ptr[0] == 0xB4 &&
              rf_xmtr_ptr[1] == 0xB3)))
        #elif defined(CAN_NOT_HEAR_C2) // ifdef HEAR_C1_ONLY //
            else if((got_pattern = (channel == *channel_num_ptr
              && data_rate == *data_rate_ptr
              && rf_channel_ptr[0] == 0x55 && 
              rf_channel_ptr[1] == 0x55 && rf_channel_ptr[2] == 0x55 && 
              rf_channel_ptr[3] == 0x33 && (rf_xmtr_ptr[0] != 0xB4 ||
              rf_xmtr_ptr[1] != 0xBA))))
        #else // elif defined(CAN_NOT_HEAR_C2) //
            else if((got_pattern = (channel == *channel_num_ptr
              && data_rate == *data_rate_ptr
              && rf_channel_ptr[0] == 0x55 && 
              rf_channel_ptr[1] == 0x55 && rf_channel_ptr[2] == 0x55 && 
              rf_channel_ptr[3] == 0x33)))
        #endif // else HEAR_C1_ONLY and CAN_NOT_HEAR_C2 are not defined //
        {
#ifdef _ONE_NET_DEBUG
            char debug_buffer[DEBUG_BUFFER_LEN+1] = {'\0'};
            size_t len = 0, size = DEBUG_BUFFER_LEN; 
   	    char * dbg_buf = debug_buffer;
#endif
            //figure out how much to read
            pid = rf_channel_ptr[ONE_NET_ENCODED_PID_IDX];
            switch(pid)
            {
                case ONE_NET_ENCODED_MASTER_INVITE_NEW_CLIENT:
                {
                    rx_size = 48;
                    break;
                } // MASTER invite new CLIENT case //

                case ONE_NET_ENCODED_MH_MASTER_INVITE_NEW_CLIENT:
                {
                    rx_size = 48 + ON_ENCODED_HOPS_SIZE;
                    break;
                } // MASTER invite new CLIENT case //

                case ONE_NET_ENCODED_SINGLE_DATA_ACK:       // fall through
                case ONE_NET_ENCODED_SINGLE_DATA_ACK_STAY_AWAKE:
                case ONE_NET_ENCODED_SINGLE_DATA_NACK:      // fall through
                case ONE_NET_ENCODED_BLOCK_DATA_ACK:        // fall through
                case ONE_NET_ENCODED_BLOCK_DATA_NACK:       // fall through
                case ONE_NET_ENCODED_STREAM_KEEP_ALIVE:
                {
                    rx_size = 17;
                    break;
                } // M_ACK/M_INVITE_REQ/M_GRANT_SINGLE

                case ONE_NET_ENCODED_MH_SINGLE_DATA_ACK:    // fall through
                case ONE_NET_ENCODED_MH_SINGLE_DATA_ACK_STAY_AWAKE:
                case ONE_NET_ENCODED_MH_SINGLE_DATA_NACK:   // fall through
                case ONE_NET_ENCODED_MH_BLOCK_DATA_ACK:     // fall through
                case ONE_NET_ENCODED_MH_BLOCK_DATA_NACK:    // fall through
                case ONE_NET_ENCODED_MH_STREAM_KEEP_ALIVE:
                {
                    rx_size = 17 + ON_ENCODED_HOPS_SIZE;
                    break;
                } // M_ACK/M_INVITE_REQ/M_GRANT_SINGLE

                case ONE_NET_ENCODED_SINGLE_TXN_ACK:        // fall through
                case ONE_NET_ENCODED_BLOCK_TXN_ACK:
                {
                    rx_size = 15;
                    break;
                } // Single transaction ack case //

                case ONE_NET_ENCODED_MH_SINGLE_TXN_ACK:     // fall through
                case ONE_NET_ENCODED_MH_BLOCK_TXN_ACK:
                {
                    rx_size = 15 + ON_ENCODED_HOPS_SIZE;
                    break;
                } // mulit-hop Single/block transaction ack case //

                case ONE_NET_ENCODED_SINGLE_DATA:           // fall through
                case ONE_NET_ENCODED_REPEAT_SINGLE_DATA:
                {
                    rx_size = 26;
                    break;
                } // Single transaction ack case //

                case ONE_NET_ENCODED_MH_SINGLE_DATA:        // fall through
                case ONE_NET_ENCODED_MH_REPEAT_SINGLE_DATA:
                {
                    rx_size = 26 + ON_ENCODED_HOPS_SIZE;
                    break;
                } // Single transaction ack case //

                case ONE_NET_ENCODED_BLOCK_DATA:            // fall through
                case ONE_NET_ENCODED_REPEAT_BLOCK_DATA:     // fall through
                case ONE_NET_ENCODED_STREAM_DATA:
                {
                    rx_size = 58;
                    break;
                } // block/repeat block case //

                case ONE_NET_ENCODED_MH_BLOCK_DATA:         // fall through
                case ONE_NET_ENCODED_MH_REPEAT_BLOCK_DATA:  // fall through
                case ONE_NET_ENCODED_MH_STREAM_DATA:
                {
                    rx_size = 58 + ON_ENCODED_HOPS_SIZE;
                    break;
                } // multi-hop (repeat)block, stream case //

                case ONE_NET_ENCODED_DATA_RATE_TEST:
                {
                    rx_size = 20;
                    break;
                } // data rate case //

                case ONE_NET_ENCODED_MH_DATA_RATE_TEST:
                {
                    rx_size = 20 + ON_ENCODED_HOPS_SIZE;
                    break;
                } // data rate case //

                default:
                {
                    release_lock(RFC_SEM);
                    return ONS_BAD_PKT_TYPE;
                    break;
                } // default ERROR
            } // end switch on pid

#ifdef _ONE_NET_DEBUG
            if(debug_level() >= 1)
            {
                //Add reading to debug buffer
                strncpy(debug_buffer, HEARD, size);
                len = strlen(HEARD);
                dbg_buf += len;
                size -= len;
        
                //add how much you're supposed to read
                len = snprintf(dbg_buf, size, "(%d): ", rx_size+4);
                dbg_buf += len;
                size -= len;

                //Add preamble & sync byte to buffer
                len = snprintf(dbg_buf, size, "%.2X %.2X %.2X %.2X ",
                  rf_channel_ptr[0], rf_channel_ptr[1], rf_channel_ptr[2], 
                  rf_channel_ptr[3]);
                dbg_buf += len;
                size -= len;
            
            } //if debug
#endif            
        
            //if we get here, then read everything else into buffer
            for(i = 0; i < rx_size; i++)
            {
                pkt_buffer[i] = rf_channel_ptr[i+4];

#ifdef _ONE_NET_DEBUG
                if(debug_level() >= 1)
                {
                    //add this byte to buffer
                    len = snprintf(dbg_buf, size, "%.2X ", rf_channel_ptr[i+4]);
                    dbg_buf += len;
                    size -= len;
                } //if debug
#endif
            }//for all data
        
            release_lock(RFC_SEM);
           
            //reset pkt_buffer index
            pkt_buffer_i = 0;
           
            //print buffer
#ifdef _ONE_NET_DEBUG
            print_debug("%s lock_count = [%d]\n", debug_buffer, lock_count);
#endif           
            rv = ONS_SUCCESS; 

            //wait for rx time
            //rx time = # bits received / bitrate * usecs per sec
            rx_size += 4;
            sleep_usecs = (rx_size * BITS_PER_BYTE) / 
                         ((Float32)BITRATE / (Float32)USECS_PER_SEC);
        
            sleep_ticks = sleep_usecs / USECS_PER_TICK;
        } //if we got pattern
        else
        {
            release_lock(RFC_SEM);
        } // else just release lock //

        if( sleep_ticks > 0 )
        {
            //"sleep" for time it took to read in data
#ifdef _ONE_NET_DEBUG
            print_debug("Sleeping for %u ticks\n", sleep_ticks);
#endif
            one_net_sleep(sleep_ticks);
#ifdef _ONE_NET_DEBUG
            print_debug("Woke up\n");
#endif
        } //sleep if read 

    } //while no pattern
    
    if( rv == ONS_TIME_OUT )
    {
#ifdef _ONE_NET_DEBUG
        print_debug("Nothing to hear\n");
#endif
    }
    return rv;
} //one_net_look_for_pkt


/*!
    \brief Reads bytes out of the rf interface.

    This function is application specific and will need to be implemented by
    the application designer.

    \param[out] data Byte array to store the receive data in.
    \param[in] LEN The number of bytes to receive (data is at least this long).
    \return The number of bytes read
*/
UInt16 one_net_read(UInt8 * data, const UInt16 LEN)
{
    UInt16 i;
#ifdef _ONE_NET_DEBUG
    char debug_buffer[DEBUG_BUFFER_LEN+1] = {'\0'};
    size_t len = 0, size = DEBUG_BUFFER_LEN; 
    char * dbg_buf = debug_buffer;

    if(debug_level() >= 1)
    {
        //Add reading to debug buffer
        strncpy(debug_buffer, READING, size);
        len = strlen(READING);
        dbg_buf += len;
        size -= len;

        //add how much you're supposed to read
        len = snprintf(dbg_buf, size, "(%d): ", LEN);
        dbg_buf += len;
        size -= len;

    } //if debug
#endif

    for(i = 0; i < LEN; i++, pkt_buffer_i++)
    {
        data[i] = pkt_buffer[pkt_buffer_i];
                
#ifdef _ONE_NET_DEBUG
        if(debug_level() >= 1)
        {
            //add this byte to buffer
            len = snprintf(dbg_buf, size, "%.2X ", pkt_buffer[pkt_buffer_i]);
            dbg_buf += len;
            size -= len;
        } //if debug
#endif
    }//read in LEN bytes

#ifdef _ONE_NET_DEBUG
    //print the buffer
    print_debug("%s\n", debug_buffer);
#endif
   
    return LEN;
} //one_net_read


/*!
    \brief Sends bytes out of the rf interface.

    This function is application specific and will need to be implemented by
    the application designer.  The rf channel must be locked before
    one_net_write is called.  one_net_write will unlock the semaphore when it
    exits.

    \param[in] DATA An array of bytes to be sent out of the rf interface
    \param[in] LEN The number of bytes to send

    \return The number of bytes sent.
*/
UInt16 one_net_write(const UInt8 * DATA, const UInt16 LEN)
{
    UInt16 i;
    UInt32 sleep_usecs = 0;

#ifdef _ONE_NET_DEBUG
    char debug_buffer[DEBUG_BUFFER_LEN+1] = {'\0'};
    size_t len = 0, size = DEBUG_BUFFER_LEN; 
    char * dbg_buf = debug_buffer;
#endif  

    //update timestamp
    *timestamp_ptr = one_net_tick();

    // set the channel
    *channel_num_ptr = channel;

    // set the data rate
    *data_rate_ptr = data_rate;

    // set the transmitter did
    one_net_memmove(rf_xmtr_ptr, &(base_param.sid[ON_ENCODED_NID_LEN]),
      ON_ENCODED_DID_LEN);

#ifdef _ONE_NET_DEBUG
    if(debug_level() >= 1)
    {
        //Add writing to debug buffer
        strncpy(debug_buffer, WRITING, size);
        len = strlen(WRITING);
        dbg_buf += len;
        size -= len;

        //Add how much you're supposed to write
        len = snprintf(dbg_buf, size, "(%d): ", LEN);
        dbg_buf += len;
        size -= len;
    } //if debug 
#endif

    //write data
    for(i = 0; i < LEN; i++)
    {
        rf_channel_ptr[i] = DATA[i];

#ifdef _ONE_NET_DEBUG
        if(debug_level() >=1)
        {
            //add this byte to buffer
            len = snprintf(dbg_buf, size ,"%.2X ", DATA[i]);
            dbg_buf += len;
            size -= len;
        } //if debug
#endif
        
    } //for all data

#ifdef _ONE_NET_DEBUG
    //print buffer
    print_debug("%s\n", debug_buffer);
#endif
        
    release_lock(RFC_SEM);
    
    //wait for tx time
    //tx time = # bits transmitted / bitrate * usecs per sec
    sleep_usecs = (LEN * BITS_PER_BYTE) / 
                 ((Float32)BITRATE / (Float32)USECS_PER_SEC);
   
    write_time = sleep_usecs / USECS_PER_TICK + one_net_tick();

#ifdef _ONE_NET_DEBUG
    // The time it would take to write the data.
    print_debug("Write time %u ticks\n", write_time);
#endif

    return LEN;
} //one_net_write


/*!
    \brief Returns TRUE if writing the data out of the rf channel is complete.

    This function also clears the data once the write time has expired.

    \param void

    \return TRUE If the device is done writing the data out of the rf channel.
            FALSE If the device is still writing the data out of the rf channel.
*/
BOOL one_net_write_done(void)
{
    if(!write_time)
    {
        return TRUE;
    } // not writing //

    if(write_time < one_net_tick())
    {
        // clear out shared memory
        acquire_lock(RFC_SEM);
#ifdef _ONE_NET_DEBUG
        print_debug("CLEARING WRITE\n");
#endif
        clear_rfc();
        release_lock(RFC_SEM);
        write_time = 0;

        return TRUE;
    } // if the write time expired //

    return FALSE;
} // one_net_write_done //


//! @} ONE-NET_port_specific_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//=============================================================================


//=============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION

/*!
    \brief Allows the device to sleep for the specified number of ticks

    The device may sleep, or it may do something else, but it is CRITICAL
    that the function returns in the correct amount of time.

    \param[in] DURATION The number of ticks to go to sleep.
    \return void
*/
static void one_net_sleep(tick_t DURATION)
{
    tick_t now = one_net_tick();
    tick_t end = now + DURATION;
    while( now < end )
    {
        now = one_net_tick();
    }
} //one_net_sleep

//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} ONE-NET_port_specific

