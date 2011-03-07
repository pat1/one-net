//! \defgroup ONE-NET_port_specific Application Specific ONE-NET functionality
//! \ingroup ONE-NET
//! @{

/*!
    \file one_net_port_specific.c
    \brief Application specific ONE-NET definitions.

*/

#include "one_net_port_specific.h"
#include "sim_port_specific_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>                 // for one_net_memmove

#include "rf_shm.h"
#include "sim_log.h"

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

//! Unique id of this device.
static UInt16 uid;

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

//! number of devices that this device can hear
static unsigned int num_dev_in_range = 0;

//! The unique device ids that this device can hear
static UInt16 dev_in_range[MAX_DEVICES];

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
    one_net_memmove(dst, SRC, len);
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
} // one_net_set_channel //


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

    \return SUCCESS if a packet has been received
*/
one_net_status_t one_net_look_for_pkt(const tick_t DURATION)
{
    unsigned int i, rx_size;
    UInt8 pid;
    UInt32 sleep_usecs = 0;
    tick_t sleep_ticks = 0;
    tick_t now, end, msg_sent, listen;
    one_net_status_t rv = ONS_TIME_OUT;

    //remember when we started looking
    listen = one_net_tick();
    end = listen + DURATION;

    while((now = one_net_tick()) < end)
    {
        acquire_lock(RFC_SEM);

        //check timestamp
        msg_sent = *timestamp_ptr;
        if(msg_sent < listen)
        {
            release_lock(RFC_SEM);
        } //if listened after sent (you missed it)

        if(channel == *channel_num_ptr && data_rate == *data_rate_ptr
          && rf_channel_ptr[0] == 0x55 && rf_channel_ptr[1] == 0x55
          && rf_channel_ptr[2] == 0x55 && rf_channel_ptr[3] == 0x33)
        {
            for(i = 0; i < num_dev_in_range; i++)
            {
                if(dev_in_range[i] == *rf_xmtr_ptr)
                {
                    break;
                } // if the device can be heard //
            } // loop to see if sending device can be heard by this device //

            if(i && i >= num_dev_in_range)
            {
                continue;
            } // if the device can't be heard //

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
                case ON_MH_BLOCK_DATA_ACK:                  // fall through
                case ON_MH_BLOCK_DATA_NACK:                 // fall through
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
                } // Single transaction ack case //

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

                case ON_MH_BLOCK_DATA:                      // fall through
                case ONE_NET_ENCODED_MH_REPEAT_BLOCK_DATA:  // fall through
                case ONE_NET_ENCODED_MH_STREAM_DATA:
                {
                    rx_size = 58 + ON_ENCODED_HOPS_SIZE;
                    break;
                } // block/repeat block case //

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

            //if we get here, then read everything else into buffer
            for(i = 0; i < rx_size; i++)
            {
                pkt_buffer[i] = rf_channel_ptr[i+4];
            }//for all data
        
            release_lock(RFC_SEM);
           
            //reset pkt_buffer index
            pkt_buffer_i = 0;

            rv = ONS_SUCCESS; 

            //wait for rx time
            //rx time = # bits received / bitrate * usecs per sec
            rx_size += 4;
            sleep_usecs = (rx_size * BITS_PER_BYTE) / 
                         ((Float32)BITRATE / (Float32)USECS_PER_SEC);
        
            sleep_ticks = sleep_usecs / USECS_PER_TICK;
            break;
        } //if we got pattern
        else
        {
            release_lock(RFC_SEM);
        } // else just release lock //

        if( sleep_ticks > 0 )
        {
            one_net_sleep(sleep_ticks);
        } //sleep if read 
    } //while no pattern
    
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
  
    for(i = 0; i < LEN; i++, pkt_buffer_i++)
    {
        data[i] = pkt_buffer[pkt_buffer_i];
    }//read in LEN bytes

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

    //update timestamp
    *timestamp_ptr = one_net_tick();

    // set the channel
    *channel_num_ptr = channel;

    // set the data rate
    *data_rate_ptr = data_rate;

    // set the transmitter did
    *rf_xmtr_ptr = uid;

    //write data
    for(i = 0; i < LEN; i++)
    {
        rf_channel_ptr[i] = DATA[i];
    } //for all data

    release_lock(RFC_SEM);
    
    //wait for tx time
    //tx time = # bits transmitted / bitrate * usecs per sec
    sleep_usecs = (LEN * BITS_PER_BYTE) / 
                 ((Float32)BITRATE / (Float32)USECS_PER_SEC);
   
    write_time = sleep_usecs / USECS_PER_TICK + one_net_tick();

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
        clear_rfc();
        release_lock(RFC_SEM);
        write_time = 0;

        return TRUE;
    } // if the write time expired //

    return FALSE;
} // one_net_write_done //


/*!
    \brief Sets the uid

    \param[in] UID The unique id to set uid to

    \return void
*/
void set_uid(const UInt16 UID)
{
    uid = UID;
} // set_uid //


/*!
    \brief Sets the devices in the device list that indicate which devices
      this device can hear.

    If more devices are passed in than can be handled, this function will
    cause the program to exit.

    \param[in] DEV_LIST List of devices this device can hear.
    \param[in] DEV_COUNT The number of devices in the list.

    \return void
*/
void set_dev_in_range(const UInt16 * const DEV_LIST,
  const unsigned int DEV_COUNT)
{
    unsigned int i;

    if(DEV_COUNT && !DEV_LIST)
    {
        sim_log("Bad parameters passed to set_dev_in_range!\nExiting!\n");
        exit(EXIT_FAILURE);
    } // if the parmeters are invalid //
    else if(DEV_COUNT > MAX_DEVICES)
    {
        sim_log("More devices (%u) passed in than can be handled (%u)!\n",
          DEV_COUNT, MAX_DEVICES);
        sim_log("Exiting!\n");
        exit(EXIT_FAILURE);
    } // else if more devices than can be handled //

    num_dev_in_range = DEV_COUNT;
    one_net_memmove(dev_in_range, DEV_LIST, DEV_COUNT);

    // Output the change in the device in range list to the log file
    sim_log("Updating list of devices in range.\nNum devices in range: %u",
      num_dev_in_range);
    for(i = 0; i < num_dev_in_range; i++)
    {
        sim_log("\t%04X", dev_in_range[i]);
    } // loop to log the new list of devices in range //
} // set_dev_in_range //


/*!
    \brief Returns if two raw dids are equal

    \param[in] LHS The did that would be the left-hand-side of the equation
    \param[in] RHS The did that would be the right-hand-side of the equation

    \return TRUE if the addresses are equal
            FALSE if the addresses are not equal
*/
BOOL raw_did_equal(const one_net_raw_did_t * const LHS,
  const one_net_raw_did_t * const RHS)
{
    unsigned int i;

    if(!LHS || !RHS)
    {
        return FALSE;
    } // if the parameters are invalid //

    if(LHS == RHS)
    {
        return TRUE;
    } // if the pointers are equal //

    for(i = 0; i < ONE_NET_RAW_DID_LEN; i++)
    {
        if(LHS[i] != RHS[i])
        {
            return FALSE;
        } // if the bytes don't match //
    } // loop through the address bytes //

    return TRUE;
} // raw_did_equal //

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

