/*!
 *     \file sniffer_raw.c
 *     \brief Reads binary data from a serial port where the data is supplied by a sniffer front end.
 *
 *     The eval_adi code is being modified and called sniffer_adi to make a ONE-NET sniffer
 *     front end. The ONE-NET sniffer front end will listen for ONE-NET messages at a low
 *     level (as the bits arrive from the transceiver) and send the messages byte by byte
 *     out the serial port (UART). The data will be raw binary bytes as they are read
 *     from the transceiver. All decoding, decrypting, and packet analysis will be performed
 *     by this ONE-NET sniffer backend.
 *
 *     This program is desiged to run on a Linux system.
 *
 *     \author Ruairi Long
 *     \note Threshold Corporation
 *
 */

/***************************************************************************
 *   Copyright (C) 2008 by Integration Lab   *
 *   ilab@thguest.threshold   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>              /* Standard input/output definitions */
#include <string.h>             /* String function definitions */
#include <unistd.h>             /* UNIX standard function definitions */
#include <fcntl.h>              /* File control definitions */
#include <errno.h>              /* Error number definitions */
#include <termios.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <termios.h>
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "fcntl.h"
#include "one_net_types.h"
#include "one_net.h"
#include "one_net_encode.h"
#include "one_net_xtea.h"
#include "one_net_crc.h"

enum
{
    MAX_PACKET_TYPE_NAME = 20,
    MAX_PCON_FIELD_SIZE = 50,
    PACKET_FIXED_PART_SIZE = 11,
    PACKET_TYPE_OFFSET = 10,
    MESSAGE_BUFFER_SIZE = 256
};

struct timeval tv_start;
struct timeval tv_now;
struct timeval tv_last_char_rcvd;

time_t ms_last = 0;
time_t ms_now;
time_t ms_diff;

time_t ms_char_last = 0;
time_t ms_char_now;
time_t ms_char_diff;

unsigned char buffer[MESSAGE_BUFFER_SIZE];

one_net_xtea_key_t key = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                          0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

//
// WARNING: Temporary Hack
// The following one_net_* functions were copied from one_net_port_specific.c
// rather than being linked in because there were too many other symbols
// that would have had to be included and I did not have the time.
//


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
      \brief Handle processing the packet contents up through the PTYP field.

      This function is called when we have received the first 11 bytes of a 
      packet. These bytes contain the destination DID(2), NID(6), source DID(2), and
      PTYP(1) fields.

      \param buffer A pointer to the buffer holding the first 11 bytes of the packet
      \param ms Millicesonds since the last message was received
      \param fp File pointer to log file to be written to.
      \return expected size of the packet based on the PTYP
*/
int handle_packet_ptyp(unsigned char * packet, int bytes_read, time_t ms, FILE *fp)
{
    unsigned char decoded_buffer[ONE_NET_MAX_ENCODED_PKT_LEN];
    unsigned char packet_type_name[MAX_PACKET_TYPE_NAME+1];
    int expected_packet_size;

    if (bytes_read < PACKET_FIXED_PART_SIZE)
    {
        strcpy(packet_type_name, "<<SHORT MSG>>");
        expected_packet_size = bytes_read;
    }
    else
    {
        switch (buffer[PACKET_TYPE_OFFSET])
        {
            case ONE_NET_ENCODED_MASTER_INVITE_NEW_CLIENT:
            {
                strcpy(packet_type_name, "INVITE");
                expected_packet_size = 48-4; // does not incl preamble(3), SOM(1)
                break;
            }

            case ONE_NET_ENCODED_SINGLE_DATA_ACK:
            {
                strcpy(packet_type_name, "ACK-SNGLD");
                expected_packet_size = 17-4; // does not incl preamble(3), SOM(1)
                break;
            }

            case ONE_NET_ENCODED_SINGLE_DATA_ACK_STAY_AWAKE:
            {
                strcpy(packet_type_name, "ACK-SA-SNGLD");
                expected_packet_size = 17-4; // does not incl preamble(3), SOM(1)
                break;
            }
          
            case ONE_NET_ENCODED_SINGLE_DATA_NACK:
            {
                strcpy(packet_type_name, "NACK-SNGLD");
                expected_packet_size = 18-5; // does not incl preamble(3), SOM(1), NAK reason
                break;
            }
          
            case ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN:
            {
                strcpy(packet_type_name, "NACK-SNGLD-RSN");
                expected_packet_size = 19-5; // does not incl preamble(3), SOM(1), NAK reason
                break;
            }
          
            case ONE_NET_ENCODED_SINGLE_TXN_ACK:
            {
                strcpy(packet_type_name, "TXN-ACK-SNGLD");
                expected_packet_size = 15-4; // does not incl preamble(3), SOM(1)
                break;
            }
          
            case ONE_NET_ENCODED_SINGLE_DATA:
            {
                strcpy(packet_type_name, "SNGLD");
                expected_packet_size = 26-4; // does not incl preamble(3), SOM(1)
                break;
            }
          
            case ONE_NET_ENCODED_REPEAT_SINGLE_DATA:
            {
                strcpy(packet_type_name, "REPT-SNGLD");
                expected_packet_size = 26-4; // does not incl preamble(3), SOM(1)
                break;
            }
          
            case ONE_NET_ENCODED_BLOCK_DATA:
            {
                strcpy(packet_type_name, "BLK-DATA");
                expected_packet_size = 58-4; // does not incl preamble(3), SOM(1)
                break;
            }
          
            case ONE_NET_ENCODED_REPEAT_BLOCK_DATA:
            {
                strcpy(packet_type_name, "REPT-BLK-DATA");
                expected_packet_size = 58-4; // does not incl preamble(3), SOM(1)
                break;
            }
          
            case ONE_NET_ENCODED_BLOCK_DATA_ACK:
            {
                strcpy(packet_type_name, "ACK-BLK");
                expected_packet_size = 17-4; // does not incl preamble(3), SOM(1)
                break;
            }
          
            case ONE_NET_ENCODED_BLOCK_DATA_NACK:
            {
                strcpy(packet_type_name, "NACK-BLK");
                expected_packet_size = 17-4; // does not incl preamble(3), SOM(1)
                break;
            }
          
            case ONE_NET_ENCODED_BLOCK_TXN_ACK:
            {
                strcpy(packet_type_name, "TXN-ACK-BLK");
                expected_packet_size = 15-4; // does not incl preamble(3), SOM(1)
                break;
            }
          
            default: 
            {
                sprintf(packet_type_name, "UNKNOWN=0x%02x", buffer[PACKET_TYPE_OFFSET]);
                expected_packet_size = 15-4; 
            }
        } // switch //
    }

    fprintf(fp, "%8d: %-15s >", ms, packet_type_name);
    on_decode(decoded_buffer, buffer, expected_packet_size);

    fprintf(fp, "%02x%02x <%02x%02x ", decoded_buffer[0], decoded_buffer[1]&0xf0,
      decoded_buffer[6], decoded_buffer[7]&0xf0);

    if (expected_packet_size == 13)
    {
        fprintf(fp, "[%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x]\n",
          decoded_buffer[0], decoded_buffer[1], decoded_buffer[2], decoded_buffer[3], 
          decoded_buffer[4], decoded_buffer[5], decoded_buffer[6], decoded_buffer[7], 
          decoded_buffer[8], decoded_buffer[9]);
    }
    else if (expected_packet_size == 14)
    {
        fprintf(fp, "[%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x]\n",
          decoded_buffer[0], decoded_buffer[1], decoded_buffer[2], decoded_buffer[3], 
          decoded_buffer[4], decoded_buffer[5], decoded_buffer[6], decoded_buffer[7], 
          decoded_buffer[8], decoded_buffer[9], decoded_buffer[10]);
    }
    else
    {
        fprintf(fp, "\n");
    }

    if ((buffer[PACKET_TYPE_OFFSET] == ONE_NET_ENCODED_SINGLE_TXN_ACK) ||
        (buffer[PACKET_TYPE_OFFSET] == ONE_NET_ENCODED_BLOCK_TXN_ACK))
    {
        fprintf(fp, "\n");
    }

    return expected_packet_size;

} // handle_packet_ptyp //
 

/*!
      \brief Calculate expected packet size and provide a name for the packet type.

      This function is called when we have received the first 11 bytes of a 
      packet. These bytes contain the destination DID(2), NID(6), source DID(2), and
      PTYP(1) fields.

      \param buffer A pointer to the buffer holding the first 11 bytes of the packet.
      \param ptyp_name A pointer to a char buffer for storing the name of the ptyp.
      \param ptyp_name_length The length of the buffer pointed to by ptyp_name.
      \return expected size of the packet based on the PTYP
*/
int ptyp_size_and_name(unsigned char * packet, unsigned char *ptyp_name, int ptyp_name_length)
{
    unsigned char packet_type_name[MAX_PACKET_TYPE_NAME+1];
    int expected_packet_size;

    switch (buffer[PACKET_TYPE_OFFSET])
    {
        case ONE_NET_ENCODED_MASTER_INVITE_NEW_CLIENT:
        {
            strncpy(ptyp_name, "INVITE", ptyp_name_length-1);
            ptyp_name[ptyp_name_length-1] = '\0';
            expected_packet_size = 48-4; // does not incl preamble(3), SOM(1)
            break;
        }

        case ONE_NET_ENCODED_SINGLE_DATA_ACK:
        {
            strncpy(ptyp_name, "ACK-SNGLD", ptyp_name_length-1);
            ptyp_name[ptyp_name_length-1] = '\0';
            expected_packet_size = 17-4; // does not incl preamble(3), SOM(1)
            break;
        }

        case ONE_NET_ENCODED_SINGLE_DATA_ACK_STAY_AWAKE:
        {
            strncpy(ptyp_name, "ACK-SA-SNGLD", ptyp_name_length-1);
            ptyp_name[ptyp_name_length-1] = '\0';
            expected_packet_size = 17-4; // does not incl preamble(3), SOM(1)
            break;
        }
      
        case ONE_NET_ENCODED_SINGLE_DATA_NACK:
        {
            strncpy(ptyp_name, "NACK-SNGLD", ptyp_name_length-1);
            ptyp_name[ptyp_name_length-1] = '\0';
            expected_packet_size = 18-5; // does not incl preamble(3), SOM(1), NAK reason
            break;
        }
      
        case ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN:
        {
            strncpy(ptyp_name, "NACK-SNGLD-RSN", ptyp_name_length-1);
            ptyp_name[ptyp_name_length-1] = '\0';
            expected_packet_size = 19-5; // does not incl preamble(3), SOM(1), NAK reason
            break;
        }
      
        case ONE_NET_ENCODED_SINGLE_TXN_ACK:
        {
            strncpy(ptyp_name, "TXN-ACK-SNGLD", ptyp_name_length-1);
            ptyp_name[ptyp_name_length-1] = '\0';
            expected_packet_size = 15-4; // does not incl preamble(3), SOM(1)
            break;
        }
      
        case ONE_NET_ENCODED_SINGLE_DATA:
        {
            strncpy(ptyp_name, "SNGLD", ptyp_name_length-1);
            ptyp_name[ptyp_name_length-1] = '\0';
            expected_packet_size = 26-4; // does not incl preamble(3), SOM(1)
            break;
        }
      
        case ONE_NET_ENCODED_REPEAT_SINGLE_DATA:
        {
            strncpy(ptyp_name, "REPT-SNGLD", ptyp_name_length-1);
            ptyp_name[ptyp_name_length-1] = '\0';
            expected_packet_size = 26-4; // does not incl preamble(3), SOM(1)
            break;
        }
      
        case ONE_NET_ENCODED_BLOCK_DATA:
        {
            strncpy(ptyp_name, "BLK-DATA", ptyp_name_length-1);
            ptyp_name[ptyp_name_length-1] = '\0';
            expected_packet_size = 58-4; // does not incl preamble(3), SOM(1)
            break;
        }
      
        case ONE_NET_ENCODED_REPEAT_BLOCK_DATA:
        {
            strncpy(ptyp_name, "REPT-BLK-DATA", ptyp_name_length-1);
            ptyp_name[ptyp_name_length-1] = '\0';
            expected_packet_size = 58-4; // does not incl preamble(3), SOM(1)
            break;
        }
      
        case ONE_NET_ENCODED_BLOCK_DATA_ACK:
        {
            strncpy(ptyp_name, "ACK-BLK", ptyp_name_length-1);
            ptyp_name[ptyp_name_length-1] = '\0';
            expected_packet_size = 17-4; // does not incl preamble(3), SOM(1)
            break;
        }
      
        case ONE_NET_ENCODED_BLOCK_DATA_NACK:
        {
            strncpy(ptyp_name, "NACK-BLK", ptyp_name_length-1);
            ptyp_name[ptyp_name_length-1] = '\0';
            expected_packet_size = 17-4; // does not incl preamble(3), SOM(1)
            break;
        }
      
        case ONE_NET_ENCODED_BLOCK_TXN_ACK:
        {
            strncpy(ptyp_name, "TXN-ACK-BLK", ptyp_name_length-1);
            ptyp_name[ptyp_name_length-1] = '\0';
            expected_packet_size = 15-4; // does not incl preamble(3), SOM(1)
            break;
        }
      
        default: 
        {
            sprintf(packet_type_name, "UNKNOWN=0x%02x", buffer[PACKET_TYPE_OFFSET]);
            expected_packet_size = 15-4; 
        }
    }

    return expected_packet_size;

} // ptyp_size_and_name //
 

/*!
      \brief Handle the PCON portion of a message

      Handle the variable portion a message (aka as PCON).

      \param buffer A pointer to the buffer holding bytes_read bytes of the packet
      \param expected_packet_size expected size of the packet in buffer
      \param bytes_read the number of bytes in this message
      \param fp File pointer to log file to be written to.
*/
void handle_packet_pcon(
  unsigned char * packet,
  int expected_packet_size,
  int bytes_read,
  FILE *fp)
{
    int pcon_size;
    int rounds = 32; // ON_XTEA_32_ROUNDS;
    unsigned char nonce1, nonce2, nack_reason, pcon_msg_type;
    unsigned char decoded_pcon[MAX_PCON_FIELD_SIZE];
    unsigned char decrypted_pcon[MAX_PCON_FIELD_SIZE];
    unsigned char calculated_crc;

    pcon_size = expected_packet_size - PACKET_FIXED_PART_SIZE;

    if (bytes_read < PACKET_FIXED_PART_SIZE)
    {
        int i;

        //
        // handle the case where we did not receive the full fixed
        // part of a packet. I not sure how this can happen, but it 
        // appeared to happen during one test run. It may be that there
        // was a collision (two devices transmitting) and what was heard
        // by the sniffer backend happened to look like a control character from 
        // the sniffer fron end.
        //
        fprintf(fp, "          [", nonce1, nonce2);
        for (i=0; i< bytes_read; i++)
        {
            fprintf(fp, "%02x ", buffer[i]);
        }
        fprintf(fp, "]\n\n", nonce1, nonce2);
    }
    else if ((buffer[PACKET_TYPE_OFFSET] == ONE_NET_ENCODED_SINGLE_DATA_ACK) ||
        (buffer[PACKET_TYPE_OFFSET] == ONE_NET_ENCODED_BLOCK_DATA_ACK) ||
        (buffer[PACKET_TYPE_OFFSET] == ONE_NET_ENCODED_SINGLE_DATA_ACK_STAY_AWAKE) ||
        (buffer[PACKET_TYPE_OFFSET] == ONE_NET_ENCODED_SINGLE_DATA_NACK) ||
        (buffer[PACKET_TYPE_OFFSET] == ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN) ||
        (buffer[PACKET_TYPE_OFFSET] == ONE_NET_ENCODED_BLOCK_DATA_NACK) ||
        (buffer[PACKET_TYPE_OFFSET] == ONE_NET_ENCODED_BLOCK_DATA_NACK_RSN))
    {
        //
        // process nonce fields, these are not encrypted
        //
        if ((buffer[PACKET_TYPE_OFFSET] != ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN) &&
            (buffer[PACKET_TYPE_OFFSET] != ONE_NET_ENCODED_BLOCK_DATA_NACK_RSN))
        {
            //
            // handle the regular NACK/ACK types (ones without a reason field)
            //
            on_decode(decoded_pcon, &buffer[0], expected_packet_size);
            nonce1 = decoded_pcon[8] & 0x3f; 
            nonce2 = (decoded_pcon[9] >> 2) & 0x3f;

            fprintf(fp, "          (tn=%02x,rn=%02x) \n", nonce1, nonce2);
        }
        else
        {
            //
            // handle the ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN type
            // and ONE_NET_ENCODED_BLOCK_DATA_NACK_RSN type
            //
            on_decode(decoded_pcon, &buffer[0], expected_packet_size);
            nonce1 = decoded_pcon[8] & 0x3f; 
            nonce2 = (decoded_pcon[9] >> 2) & 0x3f;
            nack_reason = (((decoded_pcon[9] & 0x03) << 4) | 
              ((decoded_pcon[10] >> 4) & 0x0f)) & 0x3f;

            fprintf(fp, "          (tn=%02x,rn=%02x,nr=%02x) \n", nonce1, nonce2, nack_reason);
        }
    }
    else if ((buffer[PACKET_TYPE_OFFSET] == ONE_NET_ENCODED_SINGLE_DATA) ||
        (buffer[PACKET_TYPE_OFFSET] == ONE_NET_ENCODED_REPEAT_SINGLE_DATA))
    {
        //
        // decrypt and then process the nonce feilds
        //
        on_decode(decoded_pcon ,&buffer[PACKET_TYPE_OFFSET+1], pcon_size);

        memcpy(decrypted_pcon, decoded_pcon, pcon_size);
        one_net_xtea_decipher(rounds, decrypted_pcon, (const one_net_xtea_key_t const *) &key);

        nonce1 = (decrypted_pcon[1]>>2) & 0x3f; 
        nonce2 = (((decrypted_pcon[1]&0x03) << 4) | (decrypted_pcon[2] >> 4)) & 0x3f;
        pcon_msg_type = decrypted_pcon[2] & 0x0f;

        fprintf(fp, "          (tn=%02x,rn=%02x) msg_type=%02x %02x%02x%02x%02x%02x",
          nonce1, nonce2, pcon_msg_type,
          decrypted_pcon[ON_PLD_DATA_IDX],
          decrypted_pcon[ON_PLD_DATA_IDX+1],
          decrypted_pcon[ON_PLD_DATA_IDX+2],
          decrypted_pcon[ON_PLD_DATA_IDX+3],
          decrypted_pcon[ON_PLD_DATA_IDX+4]);

        //
        // check the CRC field within the PCON section
        //
        calculated_crc = (UInt8) one_net_compute_crc(&(decrypted_pcon[ON_PLD_TXN_NONCE_IDX]),
          ONE_NET_RAW_SINGLE_DATA_LEN + ON_RAW_PLD_HDR_SIZE - ON_PLD_CRC_SIZE, ON_PLD_INIT_CRC,
          ON_PLD_CRC_ORDER);

        if (calculated_crc == decrypted_pcon[ON_PLD_CRC_IDX])
        {
            fprintf(fp, " CRC=OK\n");
        }
        else
        {
            fprintf(fp, " CRC=NG(r=%02x,c=%02x)\n", decrypted_pcon[ON_PLD_CRC_IDX], calculated_crc);
        }

        //
        // print some additional information about this message
        //
        if (pcon_msg_type == 0)
        {
            //
            // application message
            //
            fprintf(fp, "            appl msg\n");
        }
        else if (pcon_msg_type == 1)
        {
            //
            // basic admin message
            //
            fprintf(fp, "            basic admin: ");
            if (decrypted_pcon[ON_PLD_DATA_IDX] == 0x00)
            {
                fprintf(fp, "   Status Query\n");
            }
            else if (decrypted_pcon[ON_PLD_DATA_IDX] == 0x01)
            {
                fprintf(fp, "   Status Response\n");
            }
            else if (decrypted_pcon[ON_PLD_DATA_IDX] == 0x02)
            {
                fprintf(fp, "   Settings Query\n");
            }
            else if (decrypted_pcon[ON_PLD_DATA_IDX] == 0x03)
            {
                fprintf(fp, "   Settings Response\n");
            }
            else
            {
                fprintf(fp, "   msg id = 0x%02x\n", decrypted_pcon[ON_PLD_DATA_IDX]);
            }
        }
        else if (pcon_msg_type == 2)
        {
            //
            // extended admin message
            //
            if ((decrypted_pcon[ON_PLD_DATA_IDX] == 0x0f) && 
              (decrypted_pcon[ON_PLD_DATA_IDX+1] == 0x00) &&
              (decrypted_pcon[ON_PLD_DATA_IDX+2] == 0x00))
            {
                fprintf(fp, "            extended admin: Initiate Send BLock Low - Update Stream Key - "
                  "Length = 0x%02x%02x\n", decrypted_pcon[ON_PLD_DATA_IDX+3], 
                  decrypted_pcon[ON_PLD_DATA_IDX+4]);
            }
            else
            {
                fprintf(fp, "            extended admin: Unrecongnized format\n");
            }
        }
    }
    else if ((buffer[PACKET_TYPE_OFFSET] == ONE_NET_ENCODED_BLOCK_DATA) ||
        (buffer[PACKET_TYPE_OFFSET] == ONE_NET_ENCODED_REPEAT_BLOCK_DATA))
    {
        int i;

        //
        // decrypt and then process the nonce feilds
        //
        on_decode(decoded_pcon ,&buffer[PACKET_TYPE_OFFSET+1], pcon_size);

        memcpy(decrypted_pcon, decoded_pcon, pcon_size);
        one_net_xtea_decipher(rounds, decrypted_pcon, (const one_net_xtea_key_t const *) &key);
        one_net_xtea_decipher(rounds, decrypted_pcon+8, (const one_net_xtea_key_t const *) &key);
        one_net_xtea_decipher(rounds, decrypted_pcon+16, (const one_net_xtea_key_t const *) &key);
        one_net_xtea_decipher(rounds, decrypted_pcon+24, (const one_net_xtea_key_t const *) &key);

        nonce1 = (decrypted_pcon[1]>>2) & 0x3f; 
        nonce2 = (((decrypted_pcon[1]&0x03) << 4) | (decrypted_pcon[2] >> 4)) & 0x3f;
        pcon_msg_type = decrypted_pcon[2] & 0x0f;

        fprintf(fp, "          (tn=%02x,rn=%02x) msg_type=%02x\n", nonce1, nonce2, pcon_msg_type);
                        
        //
        // display the block data
        //
        fprintf(fp, "            block data: len=%d\n", pcon_size-3);
        fprintf(fp, "            [");
        for (i=3; i<pcon_size; i++)
        {
            if (((i-3) > 0) && (((i-3) % 16) == 0))
            {
                fprintf(fp, "\n             ");
            }
            fprintf(fp, "%02x ", decrypted_pcon[i]);
        }
        fprintf(fp, "]\n");
    }
    else if ((buffer[PACKET_TYPE_OFFSET] == ONE_NET_ENCODED_SINGLE_TXN_ACK) ||
          (buffer[PACKET_TYPE_OFFSET] == ONE_NET_ENCODED_BLOCK_TXN_ACK))
    {
        //
        // no more to do for the end of a transaction, just add a blank line
        //
        fprintf(fp, "\n");
    }
    else
    {
        //
        // need to handle other messaage types, except for transaction ACKs which have
        // no more data to decode
        //
        {
            fprintf(fp, "            <message type decode not implemented yet>\n");
        }
    }
} // handle_packet_pcon //
 


/*!
      \brief Write a message block to the binary data file being used to hold data from 
      ONE-NET sniffer front end.

      The format of each message block is LLtDDDDDDDD.... where LL is a 16 byte length of
      the remaining data (including the t field), t is a one byte message type, and 
      DDDDD... are data bytes. The meaning of DDDD.... depends on the message type t.
      \param 
      \return 
*/
 
void write_msg_data(
  int sniffer_fe_msg_type,
  unsigned char * buffer,
  int expected_packet_size,
  time_t ms_diff)
{
    static int raw_data_log = -1;

    if (raw_data_log == -1)
    {
        //
        // open the file for writing
    }
    else
    {
        //
        // write this data to the file
        //
    }
} // write_msg_data //

void clear_buffer(unsigned char *bp, int length)
{
    int i;

    for (i=0; i<length; i++)
    {
        bp[i] = '\0';
    }
}

int main(int argc, char *argv[])
{
    int fd, itmp, action;
    unsigned char chandigit1;
    unsigned char chandigit2;
    FILE *fp;

    // ASSIGN SOME LOCALS
    int cur_char;
    unsigned short int i = 0;
    unsigned short int value;
    unsigned short int sniffer_fe_msg_type;
    unsigned short int value10;
    unsigned short int code, channel;
    int ierr, ControlCharactersCount;
    unsigned short int length[16], length2[16];
    int expected_packet_size;
    int byte_in_msg = 0;
    int pcon_handled;

    printf("sniffer_raw starting\n");

    fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NDELAY);
    if(fd == -1)
    {
        printf("%s: ERROR: open /dev/ttys0 failed, errno=%d \n", __FILE__, errno);
        exit(1);
    }

    fp = fopen("sniffer_raw.log", "a+");
    if (fp == NULL)
    {
        printf("%s: ERROR: log file open failed, errno=%d\n", __FILE__, errno);
        exit(1);
    }


    //
    // initialize the serial port
    //
    struct termios attribs;
    itmp = tcgetattr(fd, &attribs);     // initialize termios struct
    itmp = isatty(fd);          //  make sure it is a terminal
    if(itmp != 1)
    {
        printf("Device name is not a terminal\n");
        exit(1);
    }

    attribs.c_cc[VMIN] = 0;     // THIS PARAMETER IS VERY IMPORTANT
    attribs.c_cc[VTIME] = 1;    // ALSO VERY IMPORTANT
    attribs.c_lflag = 0;        // clear ICANON flag among others
    attribs.c_oflag = 0;
    itmp = cfsetispeed(&attribs, B115200);      // set receive baud rate
    itmp = cfsetospeed(&attribs, B115200);      // set receive baud rate
    itmp = tcsetattr(fd, TCSANOW, &attribs);
    itmp = tcsetattr(fd, TCSAFLUSH, &attribs);

    // THE FOLLOWING ARE THE LENGTHS IN BYTES OF EACH FRAME, NOT INCLUDING 
    // THE START BYTES OR CHECKSUM BYTES

    printf("\n%s: %s: Reading bytes ...........\n", __FILE__, "v0.5");

    itmp = tcflush(fd, TCIOFLUSH);
    itmp = tcflush(fd, ONLCR);

    gettimeofday(&tv_start, NULL);

    while (1)                   // read and process data
    {

        while (read(fd, &value10, 1) == -1)  // read single byte from serial port
        {
            if (errno != EAGAIN)
            {
                printf("%s: ERROR read from /dev/ttyS0 failed, errno=%d\n", __FILE__, errno);
                exit(1);
            }

            //
            // see if it looks like we have read the last character in the current message
            // based on the time that has gone by since the last character.
            //
            gettimeofday(&tv_now, NULL);
            if (tv_now.tv_usec < tv_last_char_rcvd.tv_usec)
            {
                ms_char_now = ((tv_now.tv_sec-1) - tv_last_char_rcvd.tv_sec) * 1000;
                ms_char_now += ((tv_now.tv_usec+1000000) - tv_last_char_rcvd.tv_usec) / 1000;
            }
            else
            {
                ms_char_now = (tv_now.tv_sec - tv_last_char_rcvd.tv_sec) * 1000;
                ms_char_now += (tv_now.tv_usec - tv_last_char_rcvd.tv_usec) / 1000;
            }

            if (ms_char_last == 0)
            {
                ms_char_diff = 0;
            }
            else
            {
                ms_char_diff = ms_char_now - ms_char_last;
            }

            if (ms_char_diff > 500)
            {
                value10 = 0xff;
                break;
            }
        }

        ms_char_last = ms_char_now;

        // remember the time of the last character received
        gettimeofday(&tv_last_char_rcvd, NULL);

        value = value10 & 255;      // extract lower 8 bits that were actually read

        if((value & 0xf0) == 0xf0)
        {
            //
            // handle start of message character
            //
            gettimeofday(&tv_now, NULL);
            if (tv_now.tv_usec < tv_start.tv_usec)
            {
                ms_now = ((tv_now.tv_sec-1) - tv_start.tv_sec) * 1000;
                ms_now += ((tv_now.tv_usec+1000000) - tv_start.tv_usec) / 1000;
            }
            else
            {
                ms_now = (tv_now.tv_sec - tv_start.tv_sec) * 1000;
                ms_now += (tv_now.tv_usec - tv_start.tv_usec) / 1000;
            }

            if (ms_last == 0)
            {
                ms_diff = 0;
            }
            else
            {
                ms_diff = ms_now - ms_last;
            }

            if (value != 0xff)
            {
                ms_last = ms_now;
            }
            
            //
            // process the last message since we are seeing a new one
            // TODO: RWM maybe do this if the amount of time since the 
            // last character is greater than some small value since
            // there should not bew much delay.
            //
            if ( (byte_in_msg > 0) && (pcon_handled != 1))
            {
                expected_packet_size = handle_packet_ptyp(buffer, byte_in_msg, ms_diff, fp);
                handle_packet_pcon(buffer, expected_packet_size, byte_in_msg, fp);
                write_msg_data(sniffer_fe_msg_type, buffer, expected_packet_size, ms_diff);
                clear_buffer(buffer, sizeof(buffer));
            }

            if (value != 0xff)
            {
                printf("\n%02x %8ld: ", value, ms_diff);
            }
            byte_in_msg = 0;
            pcon_handled = 0;
            sniffer_fe_msg_type = value;
        }
        else
        {
            //
            // handle the next character in the message
            //
            buffer[byte_in_msg] = value;
            byte_in_msg++;

            if(byte_in_msg == PACKET_FIXED_PART_SIZE)
            {
                printf(" [%02x]", value);
            }
            else
            {
                printf(" %02x", value);
            }
        }

#if 0
        if (expected_packet_size > PACKET_FIXED_PART_SIZE)
        {
            if (byte_in_msg == expected_packet_size)
            {
                if (pcon_handled == 0)
                {
                    handle_packet_pcon(buffer, expected_packet_size, byte_in_msg, fp);
                    write_msg_data(sniffer_fe_msg_type, buffer, expected_packet_size, ms_diff);
                    clear_buffer(buffer, sizeof(buffer));
                    pcon_handled = 1;
                }
            }
            else if (byte_in_msg > expected_packet_size)
            {
                if (pcon_handled == 0)
                {
                    handle_packet_pcon(buffer, expected_packet_size, byte_in_msg, fp);
                    write_msg_data(sniffer_fe_msg_type, buffer, expected_packet_size, ms_diff);
                    clear_buffer(buffer, sizeof(buffer));
                    pcon_handled = 1;
                }
                // TODO: RWM: how handle extra characters?
            }
        }
#endif

        fflush(NULL);

    } // end main while loop


    close(fd);
    fclose(fp);
    return 0;
}
