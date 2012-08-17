#include "packet.h"
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <sys/time.h>
#include <string>
#include <cstdio>
#include "attribute.h"
#include "filter.h"
#include "string_utils.h"
#include "time_utils.h"
using namespace std;


const unsigned int NUM_PIDS = 16;
string_int_struct raw_pid_strings[NUM_PIDS] =
{
    {"ONE_NET_RAW_SINGLE_DATA", 0x00},
    {"ONE_NET_RAW_SINGLE_DATA_ACK", 0x01},    {"ONE_NET_RAW_SINGLE_DATA_NACK_RSN", 0x02},    {"ONE_NET_RAW_ROUTE",      0x03},
    {"ONE_NET_RAW_ROUTE_ACK",  0x04},
    {"ONE_NET_RAW_ROUTE_NACK", 0x05},
    {"ONE_NET_RAW_BLOCK_DATA", 0x06},
    {"ONE_NET_RAW_BLOCK_DATA_ACK", 0x07},
    {"ONE_NET_RAW_BLOCK_DATA_NACK_RSN", 0x08},
    {"ONE_NET_RAW_BLOCK_TERMINATE", 0x09},
    {"ONE_NET_RAW_STREAM_DATA", 0x0A},
    {"ONE_NET_RAW_STREAM_DATA_ACK", 0x0B},
    {"ONE_NET_RAW_STREAM_DATA_NACK_RSN", 0x0C},
    {"ONE_NET_RAW_STREAM_TERMINATE", 0x0D},
    {"ONE_NET_RAW_MASTER_INVITE_NEW_CLIENT", 0x0E},
    {"ONE_NET_RAW_CLIENT_REQUEST_INVITE", 0x0F}
};


const unsigned int NUM_NACK_REASONS = 49;
string_int_struct raw_nack_reason_strings[NUM_NACK_REASONS] =
{
    {"ON_NACK_RSN_NO_ERROR", ON_NACK_RSN_NO_ERROR},
    {"ON_NACK_RSN_RSRC_UNAVAIL_ERR", ON_NACK_RSN_RSRC_UNAVAIL_ERR},
    {"ON_NACK_RSN_INTERNAL_ERR", ON_NACK_RSN_INTERNAL_ERR},
    {"ON_NACK_RSN_BUSY_TRY_AGAIN", ON_NACK_RSN_BUSY_TRY_AGAIN},
    {"ON_NACK_RSN_BUSY_TRY_AGAIN_TIME", ON_NACK_RSN_BUSY_TRY_AGAIN_TIME},
    {"ON_NACK_RSN_BAD_POSITION_ERROR", ON_NACK_RSN_BAD_POSITION_ERROR},
    {"ON_NACK_RSN_BAD_SIZE_ERROR", ON_NACK_RSN_BAD_SIZE_ERROR},
    {"ON_NACK_RSN_BAD_ADDRESS_ERR", ON_NACK_RSN_BAD_ADDRESS_ERR},
    {"ON_NACK_RSN_INVALID_MAX_HOPS", ON_NACK_RSN_INVALID_MAX_HOPS},
    {"ON_NACK_RSN_INVALID_HOPS", ON_NACK_RSN_INVALID_HOPS},
    {"ON_NACK_RSN_INVALID_PEER", ON_NACK_RSN_INVALID_PEER},
    {"ON_NACK_RSN_OUT_OF_RANGE", ON_NACK_RSN_OUT_OF_RANGE},
    {"ON_NACK_RSN_ROUTE_ERROR", ON_NACK_RSN_ROUTE_ERROR},
    {"ON_NACK_RSN_INVALID_DATA_RATE", ON_NACK_RSN_INVALID_DATA_RATE},
    {"ON_NACK_RSN_NO_RESPONSE", ON_NACK_RSN_NO_RESPONSE},
    {"ON_NACK_RSN_INVALID_MSG_ID", ON_NACK_RSN_INVALID_MSG_ID},
    {"ON_NACK_RSN_INVALID_MSG_ID", ON_NACK_RSN_INVALID_MSG_ID},
    {"ON_NACK_RSN_FEATURES", ON_NACK_RSN_FEATURES},
    {"ON_NACK_RSN_BAD_CRC", ON_NACK_RSN_BAD_CRC},
    {"ON_NACK_RSN_BAD_KEY", ON_NACK_RSN_BAD_KEY},
    {"ON_NACK_RSN_ALREADY_IN_PROGRESS", ON_NACK_RSN_ALREADY_IN_PROGRESS},
    {"ON_NACK_RSN_NOT_ALREADY_IN_PROGRESS", ON_NACK_RSN_NOT_ALREADY_IN_PROGRESS},
    {"ON_NACK_RSN_INVALID_CHANNEL", ON_NACK_RSN_INVALID_CHANNEL},
    {"ON_NACK_RSN_INVALID_CHUNK_SIZE", ON_NACK_RSN_INVALID_CHUNK_SIZE},
    {"ON_NACK_RSN_INVALID_CHUNK_DELAY", ON_NACK_RSN_INVALID_CHUNK_DELAY},
    {"ON_NACK_RSN_INVALID_BYTE_INDEX", ON_NACK_RSN_INVALID_BYTE_INDEX},
    {"ON_NACK_RSN_INVALID_FRAG_DELAY", ON_NACK_RSN_INVALID_FRAG_DELAY},
    {"ON_NACK_RSN_INVALID_PRIORITY", ON_NACK_RSN_INVALID_PRIORITY},
    {"ON_NACK_RSN_PERMISSION_DENIED_NON_FATAL", ON_NACK_RSN_PERMISSION_DENIED_NON_FATAL},
    {"ON_NACK_RSN_UNSET", ON_NACK_RSN_UNSET},
    {"ON_NACK_RSN_GENERAL_ERR", ON_NACK_RSN_GENERAL_ERR},
    {"ON_NACK_RSN_INVALID_LENGTH_ERR", ON_NACK_RSN_INVALID_LENGTH_ERR},
    {"ON_NACK_RSN_DEVICE_FUNCTION_ERR", ON_NACK_RSN_DEVICE_FUNCTION_ERR},
    {"ON_NACK_RSN_UNIT_FUNCTION_ERR", ON_NACK_RSN_UNIT_FUNCTION_ERR},
    {"ON_NACK_RSN_INVALID_UNIT_ERR", ON_NACK_RSN_INVALID_UNIT_ERR},
    {"ON_NACK_RSN_MISMATCH_UNIT_ERR", ON_NACK_RSN_MISMATCH_UNIT_ERR},
    {"ON_NACK_RSN_BAD_DATA_ERR", ON_NACK_RSN_BAD_DATA_ERR},
    {"ON_NACK_RSN_TRANSACTION_ERR", ON_NACK_RSN_TRANSACTION_ERR},
    {"ON_NACK_RSN_MAX_FAILED_ATTEMPTS_REACHED", ON_NACK_RSN_MAX_FAILED_ATTEMPTS_REACHED},
    {"ON_NACK_RSN_BUSY", ON_NACK_RSN_BUSY},
    {"ON_NACK_RSN_NO_RESPONSE_TXN", ON_NACK_RSN_NO_RESPONSE_TXN},
    {"ON_NACK_RSN_UNIT_IS_INPUT", ON_NACK_RSN_UNIT_IS_INPUT},
    {"ON_NACK_RSN_UNIT_IS_OUTPUT", ON_NACK_RSN_UNIT_IS_OUTPUT},
    {"ON_NACK_RSN_DEVICE_NOT_IN_NETWORK", ON_NACK_RSN_DEVICE_NOT_IN_NETWORK},
    {"ON_NACK_RSN_DEVICE_IS_THIS_DEVICE", ON_NACK_RSN_DEVICE_IS_THIS_DEVICE},
    {"ON_NACK_RSN_SENDER_AND_DEST_ARE_SAME", ON_NACK_RSN_SENDER_AND_DEST_ARE_SAME},
    {"ON_NACK_RSN_PERMISSION_DENIED_FATAL", ON_NACK_RSN_PERMISSION_DENIED_FATAL},
    {"ON_NACK_RSN_ABORT", ON_NACK_RSN_ABORT},
    {"ON_NACK_RSN_FATAL_ERR", ON_NACK_RSN_FATAL_ERR}
};


const unsigned int NUM_ADMIN_MSG_TYPES = 21;
string_int_struct admin_msg_type_strings[NUM_ADMIN_MSG_TYPES] =
{
    {"ON_FEATURES_QUERY", ON_FEATURES_QUERY},
    {"ON_FEATURES_RESP", ON_FEATURES_RESP},
    {"ON_NEW_KEY_FRAGMENT", ON_NEW_KEY_FRAGMENT},
    {"ON_ADD_DEV_RESP", ON_ADD_DEV_RESP},
    {"ON_REMOVE_DEV_RESP", ON_REMOVE_DEV_RESP},
    {"ON_CHANGE_DATA_RATE_CHANNEL", ON_CHANGE_DATA_RATE_CHANNEL},
    {"ON_REQUEST_KEY_CHANGE", ON_REQUEST_KEY_CHANGE},
    {"ON_CHANGE_FRAGMENT_DELAY", ON_CHANGE_FRAGMENT_DELAY},
    {"ON_CHANGE_FRAGMENT_DELAY_RESP", ON_CHANGE_FRAGMENT_DELAY_RESP},
    {"ON_CHANGE_KEEP_ALIVE", ON_CHANGE_KEEP_ALIVE},
    {"ON_ASSIGN_PEER", ON_ASSIGN_PEER},
    {"ON_UNASSIGN_PEER", ON_UNASSIGN_PEER},
    {"ON_KEEP_ALIVE_QUERY", ON_KEEP_ALIVE_QUERY},
    {"ON_KEEP_ALIVE_RESP", ON_KEEP_ALIVE_RESP},
    {"ON_CHANGE_SETTINGS", ON_CHANGE_SETTINGS},
    {"ON_CHANGE_SETTINGS_RESP", ON_CHANGE_SETTINGS_RESP},
    {"ON_REQUEST_BLOCK_STREAM", ON_REQUEST_BLOCK_STREAM},
    {"ON_REQUEST_REPEATER", ON_REQUEST_REPEATER},
    {"ON_TERMINATE_BLOCK_STREAM", ON_TERMINATE_BLOCK_STREAM},
    {"ON_ADD_DEV", ON_ADD_DEV},
    {"ON_RM_DEV", ON_RM_DEV}
};


vector<xtea_key> packet::keys;
vector<xtea_key> packet::invite_keys;
const UInt8 packet::INVALID_CRC = 0xFF;
const UInt16 packet::INVALID_DID = 0xFFFF;
const uint64_t packet::INVALID_NID = 0xFFFFFFFFFFFFll;
const UInt16 packet::INVALID_PID = 0xFFFF;




#include "one_net.h"
#include "one_net_encode.h"
#include "one_net_application.h"
#include "one_net_message.h"
#include "one_net_peer.h"
#include "one_net_packet.h"
#include "one_net_crc.h"
#include "one_net_types.h"
#include "one_net_port_specific.h"
#include "one_net_data_rate.h"
#include "one_net_features.h"




packet::packet()
{
}


packet::packet(const packet& orig, const filter& fltr)
{
    fill_in_packet_values(orig.timestamp, orig.payload.raw_pid, orig.num_bytes,
        orig.enc_pkt_bytes, fltr);
}


packet::~packet()
{
    
}


bool packet::parse_app_payload(payload_t& payload)
{
    // TODO -- do not assume ON_APP_MSG.  Could be ON_APP_MSG_TYPE_2 or
    // something else

    return on_parse_app_pld(&payload.decrypted_payload_bytes[ON_PLD_DATA_IDX],
        ON_APP_MSG, &payload.app_payload.src_unit, &payload.app_payload.dst_unit,
        &payload.app_payload.msg_class, &payload.app_payload.msg_type,
        &payload.app_payload.msg_data);
}


bool packet::parse_admin_payload(payload_t& payload, const UInt8* admin_bytes)
{
    payload.admin_payload.admin_type = admin_bytes[0];
    admin_bytes++;

    switch(payload.admin_payload.admin_type)
    {
        case ON_CHANGE_FRAGMENT_DELAY: case ON_CHANGE_FRAGMENT_DELAY_RESP:
            payload.admin_payload.frag_time_low_ms = (admin_bytes[0] << 8) +
                    admin_bytes[1];
            payload.admin_payload.frag_time_high_ms = (admin_bytes[2] << 8) +
                    admin_bytes[3];
            return true;
        case ON_CHANGE_KEEP_ALIVE:
            payload.admin_payload.time_ms = (admin_bytes[0] << 24) +
                (admin_bytes[1] << 16) + (admin_bytes[2] << 8) + admin_bytes[3];
            return true;
        default:
            memcpy(payload.admin_payload.data, admin_bytes, sizeof(
                payload.admin_payload.data));
            return true;
    }
}


bool packet::parse_response_payload(payload_t& payload)
{
    on_ack_nack_t* ack_nack = &payload.response_payload.ack_nack;
    UInt8 tmp_bytes[sizeof(payload.decrypted_payload_bytes)];
    memcpy(tmp_bytes, payload.decrypted_payload_bytes,
        sizeof(payload.decrypted_payload_bytes));
    bool ret = (on_parse_response_pkt(payload.raw_pid, tmp_bytes, ack_nack)
        == ONS_SUCCESS);
    memcpy(&payload.response_payload.pld, ack_nack->payload,
        sizeof(ack_nack_payload_t));
    ack_nack->payload = &payload.response_payload.pld;

    if(payload.response_payload.ack_nack.handle == ON_ACK_ADMIN_MSG)
    {
        return parse_admin_payload(payload,
            &payload.decrypted_payload_bytes[ON_PLD_ADMIN_TYPE_IDX]);
    }

    return ret;
}


bool packet::parse_invite_payload(payload_t& payload)
{
    #ifdef COMPILE_WO_WARNINGS
    // meaningless code to avoid compile warnings
    if(sizeof(payload) == 12)
    {
        // do nothing
    }
    #endif

    payload.invite_payload.version =
        payload.decrypted_payload_bytes[ON_INVITE_VERSION_IDX];
    payload.invite_payload.raw_did = did_to_u16((const on_raw_did_t*)
        &payload.decrypted_payload_bytes[ON_INVITE_ASSIGNED_DID_IDX]);
    memcpy(payload.invite_payload.key,
        &payload.decrypted_payload_bytes[ON_INVITE_KEY_IDX],
        ONE_NET_XTEA_KEY_LEN);
    memcpy(&payload.invite_payload.features,
        &payload.decrypted_payload_bytes[ON_INVITE_FEATURES_IDX],
        sizeof(on_features_t));
    return true;
}


bool packet::parse_block_payload(payload_t& payload)
{
    #ifdef COMPILE_WO_WARNINGS
    // meaningless code to avoid compile warnings
    if(sizeof(payload) == 12)
    {
        // do nothing
    }
    #endif

    return false;
}


bool packet::parse_payload(UInt16 raw_pid, UInt8* decrypted_payload_bytes,
    payload_t& payload)
{
    SInt8 raw_pld_len_including_tech = get_raw_payload_len(raw_pid);
    payload.raw_pid = raw_pid;
    if(raw_pld_len_including_tech < 0)
    {
        return false;
    }
    payload.num_payload_bytes = raw_pld_len_including_tech - 1;
    if(decrypted_payload_bytes)
    {
        memmove(payload.decrypted_payload_bytes, decrypted_payload_bytes,
            payload.num_payload_bytes);
    }

    payload.payload_crc = payload.decrypted_payload_bytes[0];
    //payload.calculated_payload_crc = one_net_compute_crc(
    //    &payload.decrypted_payload_bytes[1],
    //    payload.num_payload_bytes - 1, ON_PLD_INIT_CRC, ON_PLD_CRC_ORDER);
    payload.calculated_payload_crc = 7;
    payload.valid_payload_crc =
        (payload.payload_crc == payload.calculated_payload_crc);
    payload.msg_id = get_payload_msg_id(payload.decrypted_payload_bytes);

    if(packet_is_ack(raw_pid) || packet_is_nack(raw_pid))
    {
        payload.ack_nack_handle =
            get_ack_nack_handle(payload.decrypted_payload_bytes);
        packet::parse_response_payload(payload);
        return packet::parse_response_payload(payload);
    }
    else if(packet_is_data(raw_pid) && packet_is_single(raw_pid))
    {
        payload.msg_type = get_payload_msg_type(payload.decrypted_payload_bytes);
        switch(payload.msg_type)
        {
            case ON_APP_MSG:
                payload.is_app_pkt = true;
                return packet::parse_app_payload(payload);
            case ON_ADMIN_MSG:
                payload.is_admin_pkt = true;
                return packet::parse_admin_payload(payload,
                    &payload.decrypted_payload_bytes[ON_PLD_DATA_IDX]);
            case ON_FEATURE_MSG:
                payload.is_features_pkt = true;
                memcpy(&payload.features_payload,
                    &payload.decrypted_payload_bytes[ON_PLD_DATA_IDX],
                    sizeof(on_features_t));
                return true;
            default:
                return false;
        }
    }
    else if(packet_is_invite(raw_pid))
    {
        return packet::parse_invite_payload(payload);
    }
    else
    {
        // TODO -- handle block and stream
        return false;
    }
}


bool packet::filter_packet(const filter& fltr) const
{
cout << "p1\n";
    // test timestamp
    if(!fltr.value_accepted(filter::FILTER_TIMESTAMP,
        struct_timeval_to_milliseconds(this->timestamp)))
    {
        return false;
    }
cout << "p2\n";

    // test source did
    if(!fltr.value_accepted(filter::FILTER_SRC_DID, raw_src_did))
    {
        return false;
    }

    // test repeater did
    if(!fltr.value_accepted(filter::FILTER_RPTR_DID, raw_rptr_did))
    {
        return false;
    }

    // test nid
    if(!fltr.value_accepted(filter::FILTER_NID, raw_nid))
    {
        return false;
    }

    // test pid
    if(!fltr.value_accepted(filter::FILTER_PID, payload.raw_pid))
    {
        return false;
    }

    // test encryption key
    if(!fltr.value_accepted(payload.is_invite_pkt ? filter::FILTER_INVITE_KEYS :
        filter::FILTER_KEYS, key))
    {
        return false;
    }

    // test message crc
    if(!fltr.value_accepted(filter::FILTER_MSG_CRC, msg_crc))
    {
        return false;
    }
    if(!fltr.match_value_accepted(filter::FILTER_MSG_CRC_MATCH, valid_msg_crc))
    {
        return false;
    }

    // test payload crc
    if(!fltr.value_accepted(filter::FILTER_PLD_CRC, payload.payload_crc))
    {
        return false;
    }
    if(!fltr.match_value_accepted(filter::FILTER_PAYLOAD_CRC_MATCH,
        payload.valid_payload_crc))
    {
        return false;
    }

    // test message id
    if(!fltr.value_accepted(filter::FILTER_MSG_ID, payload.msg_id))
    {
        return false;
    }

    // test valid decode and valid
    if(!fltr.match_value_accepted(filter::FILTER_VALID_DECODE_MATCH,
        valid_decode))
    {
        return false;
    }
    if(!fltr.match_value_accepted(filter::FILTER_VALID_MATCH, valid))
    {
        return false;
    }

    // test hops and max hops
    if(!fltr.value_accepted(filter::FILTER_HOPS, hops))
    {
        return false;
    }
    if(!fltr.value_accepted(filter::FILTER_MAX_HOPS, max_hops))
    {
        return false;
    }

    // test admin message type
    bool test_admin_type = false;
    UInt8 admin_type;
    if(is_data_pkt && is_single_pkt)
    {
        test_admin_type = true;
        admin_type = payload.admin_payload.admin_type;
    }
    else if(this->is_response_pkt && payload.admin_payload.admin_type ==
        ON_ACK_ADMIN_MSG)
    {
        test_admin_type = true;
        admin_type = payload.admin_payload.admin_type;
    }
    if(test_admin_type && !fltr.value_accepted(filter::FILTER_ADMIN_TYPE,
        admin_type))
    {
        return false;
    }

    return true;
}


bool packet::fill_in_packet_values(struct timeval timestamp, UInt16 raw_pid,
    UInt8 num_bytes, const UInt8* const bytes, const filter& fltr)
{
    payload.is_app_pkt = false;
    payload.is_admin_pkt = false;
    payload.is_features_pkt = false;
    payload.is_invite_pkt = false;


    valid = false;
    valid_decode = false;
    valid_msg_crc = false;
    payload.valid_payload_crc = false;
    this->payload.raw_pid = raw_pid;
    this->timestamp = timestamp;
    this->num_bytes = num_bytes;
    if(!enc_pkt_bytes || num_bytes < ON_SINGLE_ENCODED_PKT_LEN || num_bytes >
        ON_MAX_ENCODED_PKT_SIZE)
    {
        return false;
    }
    
    memmove(enc_pkt_bytes, bytes, num_bytes);

    if(!setup_pkt_ptr(raw_pid, enc_pkt_bytes, 0, &pkt_ptr))
    {
        return false;
    }

    SInt8 tmp;
    if((tmp = get_encoded_payload_len(raw_pid)) < 0)
    {
        return false;
    }
    encoded_payload_len = tmp;
    if((tmp = get_raw_payload_len(raw_pid)) < 0)
    {
        return false;
    }
    payload.num_payload_bytes = tmp - 1;


    enc_rptr_did = one_net_byte_stream_to_uint16(
      &enc_pkt_bytes[ON_ENCODED_RPTR_DID_IDX]);
    enc_dst_did = one_net_byte_stream_to_uint16(
      &enc_pkt_bytes[ON_ENCODED_DST_DID_IDX]);
    enc_src_did = one_net_byte_stream_to_uint16(
      &enc_pkt_bytes[ON_ENCODED_SRC_DID_IDX]);


    valid_decode = true;
    UInt8 raw_nid_bytes[ON_RAW_NID_LEN];
    if(on_decode(raw_nid_bytes, &enc_pkt_bytes[ON_ENCODED_NID_IDX],
      ON_ENCODED_NID_LEN) != ONS_SUCCESS)
    {
        valid_decode = false;
    }
    else
    {
        UInt32 tmp = one_net_byte_stream_to_uint32(raw_nid_bytes);
        raw_nid = (tmp << 4) + (raw_nid_bytes[4] >> 4);
        enc_nid = (enc_pkt_bytes[ON_ENCODED_NID_IDX]) << 8;
        enc_nid += enc_pkt_bytes[ON_ENCODED_NID_IDX+1];
        enc_nid <<= 32;
        enc_nid += one_net_byte_stream_to_uint32(
          &enc_pkt_bytes[ON_ENCODED_NID_IDX + 2]);
    }

    on_raw_did_t raw_did;
    if(on_decode(raw_did, &enc_pkt_bytes[ON_ENCODED_SRC_DID_IDX],
      ON_ENCODED_DID_LEN) != ONS_SUCCESS)
    {
        valid_decode = false;
    }
    raw_src_did = did_to_u16(&raw_did);

    if(on_decode(raw_did, &enc_pkt_bytes[ON_ENCODED_RPTR_DID_IDX],
      ON_ENCODED_DID_LEN) != ONS_SUCCESS)
    {
        valid_decode = false;
    }
    raw_rptr_did = did_to_u16(&raw_did);

    if(on_decode(raw_did, &enc_pkt_bytes[ON_ENCODED_DST_DID_IDX],
      ON_ENCODED_DID_LEN) != ONS_SUCCESS)
    {
        valid_decode = false;
    }
    raw_dst_did = did_to_u16(&raw_did);

    enc_msg_crc = enc_pkt_bytes[ON_ENCODED_MSG_CRC_IDX];
    msg_crc = encoded_to_decoded_byte(enc_msg_crc, TRUE);
    calculated_msg_crc = calculate_msg_crc(&pkt_ptr);

    payload.is_invite_pkt = packet_is_invite(raw_pid);
    is_ack_pkt = packet_is_ack(raw_pid);
    is_nack_pkt = packet_is_nack(raw_pid);
    is_response_pkt = is_ack_pkt || is_nack_pkt;
    is_stay_awake_pkt = ((raw_pid & ONE_NET_RAW_PID_STAY_AWAKE_MASK) > 0);
    is_block_pkt = packet_is_block(raw_pid);
    is_stream_pkt = packet_is_stream(raw_pid);
    is_single_pkt = packet_is_single(raw_pid);
    is_data_pkt = packet_is_data(raw_pid);
    is_mh_pkt = ((raw_pid & ONE_NET_RAW_PID_MH_MASK) > 0);
    hops = 0;
    max_hops = 0;
    if(is_mh_pkt)
    {
        encoded_hops_field = enc_pkt_bytes[ON_ENCODED_PLD_IDX + pkt_ptr.payload_len];
        if((decoded_hops_field = encoded_to_decoded_byte(encoded_hops_field,
            false)) = 0xFF)
        {
            valid_decode = false;
        }
        else
        {
            on_parse_hops(&pkt_ptr, &(pkt_ptr.hops), &(pkt_ptr.max_hops));
            max_hops = pkt_ptr.max_hops;
            hops = pkt_ptr.hops;
        }
    }

    if(on_decode(encrypted_payload_bytes, &enc_pkt_bytes[ON_ENCODED_PLD_IDX],
      encoded_payload_len) != ONS_SUCCESS)
    {
        valid_decode = false;
    }
    
    valid_msg_crc = (msg_crc == calculated_msg_crc);
    valid = (valid_decode && valid_msg_crc);

    payload.valid_payload_crc = false;
    bool valid_decrypt = false;

    const vector<xtea_key>* keys = fltr.get_keys(packet_is_invite(raw_pid));

    for(unsigned int i = 0; !valid_decrypt && i < keys->size(); i++)
    {
        memmove(payload.decrypted_payload_bytes, encrypted_payload_bytes,
            ON_MAX_ENCODED_PLD_LEN_WITH_TECH);
        xtea_key key = keys->at(i);

        if(on_decrypt(is_stream_pkt,
          payload.decrypted_payload_bytes, (one_net_xtea_key_t*) key.bytes,
          payload.num_payload_bytes + 1) != ONS_SUCCESS)
        {
            continue;
        }

        payload.payload_crc = payload.decrypted_payload_bytes[0];
        payload.calculated_payload_crc = 8;
        payload.valid_payload_crc =
            (payload.payload_crc == payload.calculated_payload_crc);


        if(payload.valid_payload_crc && packet::parse_payload(raw_pid, NULL,
            payload))
        {
            valid_decrypt = true;
            this->key = key;
        }
    }

    valid = (valid && valid_decrypt);


    if(is_single_pkt && is_data_pkt)
    {
        payload.msg_type = get_payload_msg_type(
            payload.decrypted_payload_bytes);

        switch(payload.msg_type)
        {
            case ON_APP_MSG:
                payload.is_app_pkt = true;
                parse_app_payload(payload);
                break;
            case ON_ADMIN_MSG:
                payload.is_admin_pkt = true;
                parse_admin_payload(payload,
                    &payload.decrypted_payload_bytes[ON_PLD_DATA_IDX]);
                break;
            case ON_FEATURE_MSG:
                payload.is_features_pkt = true;
                break;
            default:
                // TODO -- set valid false?
                break;
        }
    }
    else if(is_single_pkt && is_response_pkt)
    {
        parse_response_payload(payload);
    }
    else if(payload.is_invite_pkt)
    {
        parse_invite_payload(payload);
    }
    else
    {
        // TODO -- block and stream
    }
 
    return true;
}


bool packet::create_packet(struct timeval timestamp, UInt16 raw_pid,
    UInt8 num_bytes, const UInt8* const bytes, const filter& fltr, packet& pkt)
{
    return pkt.fill_in_packet_values(timestamp, raw_pid, num_bytes, bytes, fltr);
}


bool packet::create_packet(string line, const filter& fltr, packet& pkt)
{
    static bool rcvd_num_bytes = false;
    static int num_bytes_rcvd;
    static int num_bytes_expected;
    UInt32 timestamp_ms;
    static UInt8 bytes[ON_MAX_ENCODED_PKT_SIZE];
    static struct timeval timestamp;


    if(!rcvd_num_bytes)
    {
        // first count
        stringstream ss(line);
        int num_strings = 0;
        string tmp;
        while(ss >> tmp)
        {
            num_strings++;
        }


        if(num_strings != 4)
        {
            return false;
        }

        ss.clear();
        ss.seekg(ios_base::beg);
        ss >> timestamp_ms;
        timestamp.tv_sec = timestamp_ms / 1000;
        timestamp.tv_usec = (timestamp_ms % 1000) * 1000;
        ss >> tmp;
        if(tmp != "received" && tmp != "sending" && tmp != "sent")
        {
            return false;
        }
        ss >> num_bytes_expected;

        if(num_bytes_expected < ON_MIN_ENCODED_PKT_SIZE || num_bytes_expected >
            ON_MAX_ENCODED_PKT_SIZE)
        {
            return false;
        }

        ss >> tmp;
        if(tmp != "bytes:")
        {
            return false;
        }
        if(!ss)
        {
            return false;
        }
        num_bytes_rcvd = 0;
        rcvd_num_bytes = true;
        return false;
    }

    string tmp;
    stringstream ss(line);
    while(ss >> tmp)
    {
        if(num_bytes_rcvd >= num_bytes_expected)
        {
            rcvd_num_bytes = false;
            return false;
        }

        if(!string_to_uint8(tmp, bytes[num_bytes_rcvd], true))
        {
            rcvd_num_bytes = 0;
            return false;
        }

        if(num_bytes_rcvd == ON_ENCODED_PLD_IDX - 1)
        {
            pkt.enc_pid = one_net_byte_stream_to_uint16(
              &bytes[ON_ENCODED_PID_IDX]);
            UInt8 raw_pid_bytes[ON_ENCODED_PID_SIZE];
            pkt.raw_pid = 0xFFFF; // just make it invalid
            if(on_decode(raw_pid_bytes, &bytes[ON_ENCODED_PID_IDX],
              ON_ENCODED_PID_SIZE) == ONS_SUCCESS)
            {
                pkt.raw_pid = (one_net_byte_stream_to_uint16(raw_pid_bytes)) >> 4;
            }

            if(num_bytes_expected != (int) get_encoded_packet_len(pkt.raw_pid,
              TRUE))
            {
                rcvd_num_bytes = false;
                return false;
            }
            pkt.payload.raw_pid = pkt.raw_pid;
        }
        num_bytes_rcvd++;
    }

    if(num_bytes_rcvd < num_bytes_expected)
    {
        return false;
    }

    rcvd_num_bytes = false; // packet bytes received.  Set false for next time
    return create_packet(timestamp, pkt.raw_pid, (UInt8) num_bytes_rcvd,
        bytes, fltr, pkt);
}


bool packet::create_packet(int fd, const filter& fltr, packet& pkt)
{
    #ifdef COMPILE_WO_WARNINGS
    // meaningless code to avoid compile warnings
    if(sizeof(fd) + sizeof(fltr) + sizeof(pkt) == 12)
    {
        // do nothing
    }
    #endif

    return false;
}


bool packet::create_packet(FILE* file, const filter& fltr, packet& pkt)
{
    #ifdef COMPILE_WO_WARNINGS
    // meaningless code to avoid compile warnings
    if(sizeof(file) + sizeof(fltr) + sizeof(pkt) == 12)
    {
        // do nothing
    }
    #endif

    return false;
}


bool packet::create_packet(istream& is, const filter& fltr, packet& pkt)
{
    #ifdef COMPILE_WO_WARNINGS
    // meaningless code to avoid compile warnings
    if(sizeof(is) + sizeof(fltr) + sizeof(pkt) == 12)
    {
        // do nothing
    }
    #endif

    return false;
}


string app_payload_t::get_msg_class_string(UInt16 msg_class)
{
    switch(msg_class)
    {
        case ONA_STATUS: return "STATUS";
        case ONA_STATUS_CHANGE: return "STATUS_CHANGE";
        case ONA_STATUS_QUERY_RESP: return "STATUS_QUERY_RESP";
        case ONA_STATUS_FAST_QUERY_RESP: return "STATUS_FAST_QUERY_RESP";
        case ONA_STATUS_COMMAND_RESP: return "STATUS_COMMAND_RESP";
        case ONA_COMMAND: return "COMMAND";
        case ONA_QUERY: return "QUERY";
        case ONA_FAST_QUERY: return "FAST_QUERY";
        default: return "UNKNOWN";
    }
}


bool admin_payload_t::detailed_admin_payload_to_string(string& str) const
{
    string tmp;
    stringstream ss;
    str = "Admin Type : 0x";
    byte_to_hex_string(admin_type, tmp);
    str += tmp;
    str += "(";
    str += admin_payload_t::get_admin_type_string(admin_type);
    str += ") -- ";

    switch(admin_type)
    {
        case ON_NEW_KEY_FRAGMENT: case ON_KEEP_ALIVE_RESP:
            str += "Key Fragment : ";
            bytes_to_hex_string(key_frag, 4, tmp, '-', 1, 0);
            str += tmp;
            break;
        case ON_FEATURES_QUERY: case ON_FEATURES_RESP:
            str += "Features...";
            if(!payload_t::detailed_features_to_string(features, tmp))
            {
                return false;
            }
            str += tmp;
            break;
        case ON_ADD_DEV_RESP: case ON_REMOVE_DEV_RESP:
        case ON_CHANGE_SETTINGS_RESP:
            str += "No payload";
            break;
        case ON_ADD_DEV: case ON_RM_DEV:
            str += "Encoded DID : 0x";
            bytes_to_hex_string(add_remove_msg.enc_did, 2, tmp, ' ', 0, 0);
            str += tmp;
            str += " -- # Multi-Hop : ";
            ss.clear();
            ss << (int) add_remove_msg.num_mh_devices;
            ss >> tmp;
            str += tmp;
            str += " -- # Multi-Hop Repeater : ";
            ss.clear();
            ss << (int) add_remove_msg.num_mh_repeaters;
            ss >> tmp;
            str += tmp;
            break;
        case ON_ASSIGN_PEER: case ON_UNASSIGN_PEER:
            str += "Encoded Peer DID : 0x";
            bytes_to_hex_string(peer_msg.peer_did, 2, tmp, ' ', 0, 0);
            str += tmp;
            str += " -- Src Unit : ";
            ss << uppercase << (int) peer_msg.src_unit;
            ss >> tmp;
            str += tmp;
            str += " -- Peer Unit : ";
            ss << uppercase << (int) peer_msg.peer_unit;
            ss >> tmp;
            str += tmp;
            break;
        case ON_CHANGE_FRAGMENT_DELAY:
        case ON_CHANGE_FRAGMENT_DELAY_RESP:
            str += " -- Frag Low ms. : ";
            ss << frag_time_low_ms;
            ss >> tmp;
            str += tmp;
            str += " -- Frag High ms. : ";
            ss << frag_time_high_ms;
            ss >> tmp;
            str += tmp;
            break;
        case ON_CHANGE_KEEP_ALIVE:
            str += "Time Milliseconds : ";
            ss << time_ms;
            ss >> tmp;
            str += tmp;
            break;
        case ON_CHANGE_SETTINGS:
            str += "Settings : 0x";
            ss << (int) flags;
            str += " -- JOINED : ";
            str += ((flags & ON_JOINED) ? "true" : "false");
            str += " -- SEND TO MASTER : ";
            str += ((flags & ON_SEND_TO_MASTER) ? "true" : "false");
            break;
        default:
            break;
    }

    str += "\n";
    return true;
}


bool app_payload_t::detailed_app_payload_to_string(string& str) const
{
    string tmp;
    stringstream ss;
    ss << setfill('0') << uppercase << setw(1) << hex << (int) src_unit;
    ss << " ";
    ss << setfill('0') << uppercase << setw(1) << hex << (int) dst_unit;
    ss << " ";
    ss << setfill('0') << uppercase << setw(4) << hex << (int) msg_class;
    ss << " ";
    ss << setfill('0') << uppercase << setw(2) << hex << (int) msg_type;
    ss << " ";
    ss << setfill('0') << uppercase << setw(5) << hex << (int) msg_data;
    str = " -- Source Unit : 0x";
    ss >> tmp;
    str += tmp;
    str += " -- Dest. Unit : 0x";
    ss >> tmp;
    str += tmp;
    str += " -- Msg Class : 0x";
    ss >> tmp;
    str += tmp;
    str += "(";
    str += app_payload_t::get_msg_class_string(msg_class);
    str += ") -- Msg Type : 0x";
    ss >> tmp;
    str += tmp;
    str += " -- Msg. Data : 0x";
    ss >> tmp;
    str += tmp;
    str += "\n";
    return true;
}


string admin_payload_t::get_admin_type_string(UInt8 admin_type)
{
    static map<int, string> pairs = 
      create_int_string_map(admin_msg_type_strings, NUM_ADMIN_MSG_TYPES);

    map<int,string>::iterator it = pairs.find((int) admin_type);
    if(it == pairs.end())
    {
        return "Invalid";
    }
    return it->second;
}


string payload_t::get_ack_nack_handle_string(bool is_ack,
    on_ack_nack_handle_t handle)
{
    string prefix_str = is_ack ? "ACK " : "NACK ";
    static const char* const ACK_NACK_HANDLE_STR_ARRAY[ON_ACK_MAX_HANDLE+1] =
    {
        "NO PAYLOAD",
        "FEATURES",
        "DATA",
        "VALUE",
        "TIME MS",
        "TIMEOUT MS",
        "SLOW DOWN TIME MS",
        "SPEED UP TIME MS",
        "PAUSE TIME MS",
        "RESPONSE TIME MS",
        "KEY FRAG / KEY",

        // the 4 handles bleow are only valid for ACKs
        "BLOCK PACKETS RCVD",
        "ROUTE",
        "APP MSG",
        "ADMIN MSG",

        "USR/APP-CODE MSG"
    };

    if((int)handle == ON_ACK_APPLICATION_HANDLE)
    {
        return "UNKNOWN";
    }
    if(!is_ack && ((int)handle <= ON_ACK_BLK_PKTS_RCVD &&
      (int)handle <= ON_ADMIN_MSG))
    {
        return "INVALID";
    }

    return prefix_str + ACK_NACK_HANDLE_STR_ARRAY[(int) handle];
}


string payload_t::get_nack_reason_string(on_nack_rsn_t nack_reason)
{
    static map<int, string> pairs = 
      create_int_string_map(raw_nack_reason_strings, NUM_NACK_REASONS);

    map<int,string>::iterator it = pairs.find((int) nack_reason);
    if(it == pairs.end())
    {
        return "";
    }
    return it->second;
}


string packet::get_raw_pid_string(UInt16 raw_pid)
{
    static map<int, string> pairs = 
      create_int_string_map(raw_pid_strings, NUM_PIDS);

    map<int,string>::iterator it = pairs.find((int) (raw_pid & 0x3F));
    if(it == pairs.end())
    {
        return "Invalid";
    }
    return it->second;
}


bool payload_t::detailed_data_rates_to_string(on_features_t features,
    string& str)
{
    // Data Rate Strings
    const char* const DATA_RATE_STR[ONE_NET_DATA_RATE_LIMIT] =
    {
        "38,400",
        "76,800",
        "115,200",
        "153,600",
        "192,000",
        "230,400"
    };

    str = "";
    for(int i = 0; i < ONE_NET_DATA_RATE_LIMIT; i++)
    {
        BOOL dr_capable = features_data_rate_capable(features, i);
        str += "Data rate ";
        str += DATA_RATE_STR[i];
        if(i < 2)
        {
            str += " ";
        }
        str += " : ";
        str += (dr_capable ? "Capable" : "Not Capable");
        str += "\n";
    }

    return true;
}


bool payload_t::detailed_features_to_string(on_features_t features, string& str)
{
    string tmp;
    str = "Feature Bytes : ";
    bytes_to_hex_string((UInt8*) &features, sizeof(on_features_t), tmp, ' ', 1,
        0);
    str += tmp;
    str += "\nMax Hops : ";
    byte_to_hex_string(features_max_hops(features), tmp);
    str += tmp;
    str += "\nMax Peers : ";
    byte_to_hex_string(features_max_peers(features), tmp);
    str += tmp;
    str += "\nQueue Size : ";
    byte_to_hex_string(features_queue_size(features), tmp);
    str += tmp;
    str += "\nQueue Level : ";
    byte_to_hex_string(features_queue_level(features), tmp);
    str += tmp;

    str += "\nData Rate / Channel Change : ";
    str += (features_dr_channel_capable(features) ? "Capable" : "Not Capable");
    str += "\nExtended Single : ";
    str += (features_extended_single_capable(features) ? "Capable" : "Not Capable");
    str += "\nRoute : ";
    str += (features_route_capable(features) ? "Capable" : "Not Capable");
    str += "\nMulti-Hop : ";
    str += (features_mh_capable(features) ? "Capable" : "Not Capable");
    str += "\nMulti-Hop Repeat : ";
    str += (features_mh_repeat_capable(features) ? "Capable" : "Not Capable");
    str += "\nBlock Capable : ";
    str += (features_block_capable(features) ? "Capable" : "Not Capable");
    str += "\nStream Capable : ";
    str += (features_stream_capable(features) ? "Capable" : "Not Capable");
    str += "\nDevice Sleeps : ";
    str += (features_device_sleeps(features) ? "True" : "False");

    str += "\n\nData Rates...\n\n";
    detailed_data_rates_to_string(features, tmp);
    str += tmp;
    str += "\n";
    return true;
}


bool invite_payload_t::detailed_invite_payload_to_string(string& str) const
{
    stringstream ss;
    string tmp;
    str = "Version : ";
    ss << (int) this->version;
    ss >> tmp;
    str += tmp;
    str += "\nKey : ";
    bytes_to_hex_string(this->key, ONE_NET_XTEA_KEY_LEN, tmp, '-', 1, 0);
    str += tmp;
    str += "\nRaw DID : ";
    if(!raw_did_to_string(raw_did, tmp))
    {
        return false;
    }
    str += tmp;
    str += "\n";
    if(!payload_t::detailed_features_to_string(features, tmp))
    {
        return false;
    }
    str += "Features\n";
    str += tmp;
    str += "\n";
    return true;
}


bool payload_t::detailed_response_payload_to_string(string& str) const
{
    string tmp;
    stringstream ss;
    bool is_ack = (this->response_payload.ack_nack.nack_reason ==
        ON_NACK_RSN_NO_ERROR);
    str = "ACK / NACK : ";
    str += (is_ack ? "ACK" : "NACK");
    str += " -- ";
    if(!is_ack)
    {
        str += " NACK Reason : 0x";
        byte_to_hex_string(response_payload.ack_nack.nack_reason, tmp);
        str += tmp;
        str += "(";
        str += payload_t::get_nack_reason_string(
            response_payload.ack_nack.nack_reason);
        str += ") --";
    }

    str += "Handle : 0x";
    byte_to_hex_string(response_payload.ack_nack.handle, tmp);
    str += tmp;
    str += "(";
    str += payload_t::get_ack_nack_handle_string(is_ack,
        response_payload.ack_nack.handle);
    str += ")";

    switch(response_payload.ack_nack.handle)
    {
        case ON_ACK:
            return true;
        case ON_ACK_FEATURES:
            if(!detailed_features_to_string(
                response_payload.ack_nack.payload->features, tmp))
            {
                return false;
            }
            str += tmp;
            break;
        case ON_ACK_DATA:
            if(!bytes_to_hex_string(is_ack ?
                response_payload.ack_nack.payload->ack_payload :
                response_payload.ack_nack.payload->nack_payload, 4, tmp, ' ', 1, 0))
            {
                return false;
            }
            str += tmp;
            break;
        case ON_ACK_VALUE:
            if(is_ack)
            {
                str += " -- 32-bit Value : ";
                ss << response_payload.ack_nack.payload->ack_value;
                ss >> tmp;
                str += tmp;
            }
            else
            {
                str += " -- 32-bit Value : ";
                ss << response_payload.ack_nack.payload->nack_value;
                ss >> tmp;
                str += tmp;
            }
            break;
        case ON_ACK_TIME_MS: case ON_ACK_TIMEOUT_MS:
        case ON_ACK_SLOW_DOWN_TIME_MS: case ON_ACK_SPEED_UP_TIME_MS:
        case ON_ACK_PAUSE_TIME_MS:
            str += " -- Time ms : ";
            ss << response_payload.ack_nack.payload->ack_time_ms;
            ss >> tmp;
            str += tmp;
            break;
        case ON_ACK_ADMIN_MSG:
            str += "ACK Admin Payload...\n";
            if(!admin_payload.detailed_admin_payload_to_string(tmp))
            {
                return false;
            }
            str += tmp;
            break;
        default:
            break;
    }

    str += "\n";
    return true;
}


bool payload_t::detailed_payload_to_string(UInt16 raw_pid, string& str) const
{
    str = "";

    if(packet_is_single(raw_pid) && (packet_is_ack(raw_pid) ||
        packet_is_nack(raw_pid)))
    {
        return this->detailed_response_payload_to_string(str);
    }
    if(this->is_app_pkt)
    {
        return app_payload.detailed_app_payload_to_string(str);
    }

    else if(this->is_admin_pkt)
    {
        return admin_payload.detailed_admin_payload_to_string(str);
    }
    else if(this->is_features_pkt)
    {
        return detailed_features_to_string(features_payload, str);
    }
    else if(this->is_invite_pkt)
    {
        return invite_payload.detailed_invite_payload_to_string(str);
    }

    return true;
}


bool packet::display(const attribute& att, ostream& outs) const
{
    string str;
    string raw_did_str, raw_nid_str, enc_did_str, enc_nid_str;

    if(att.get_attribute(attribute::ATTRIBUTE_TIMESTAMP))
    {
        struct_timeval_to_string(timestamp, str);
        outs << "Timestamp : " << str << " seconds\n";
    }
    if(att.get_attribute(attribute::ATTRIBUTE_VALID_PKT))
    {
        outs << "Valid Decode : " << (this->valid_decode ? "true" : "false");
        outs << " -- Valid Msg CRC : " << (this->valid_msg_crc ?
            "true" : "false");
        outs << " -- Valid Payload CRC : " << (this->payload.valid_payload_crc ?
            "true" : "false");
        outs << " -- Valid Packet : " << (this->valid ? "true" : "false") <<
            "\n";
    }
    if(att.get_attribute(attribute::ATTRIBUTE_ENCODED_BYTES))
    {
        outs << "# of Encoded bytes = " << (int) this->num_bytes << endl;
        if(!bytes_to_hex_string(this->enc_pkt_bytes, this->num_bytes,
            str, ' ', 1, 24))
        {
            return false;
        }
        outs << str << endl;
    }

    if(att.get_attribute(attribute::ATTRIBUTE_RPTR_DID))
    {
        outs << left << setw(15) << setfill(' ') << "Rptr. DID";
        outs << " -- (Encoded --         0x" << setw(4) << hex << uppercase <<
          setfill('0') << right << enc_rptr_did << ")  (Decoded --       ";

        if(raw_rptr_did == INVALID_DID)
        {
            outs << "Cannot convert";
        }
        else
        {
            outs << "0x" << setw(3) << hex << uppercase <<
              setfill('0') << raw_rptr_did;
        }
        outs << ")" << endl;
    }

    if(att.get_attribute(attribute::ATTRIBUTE_MSG_CRC))
    {
        outs << left << setw(15) << setfill(' ') << "Msg. CRC";
        outs << " -- (Encoded --           0x" << setw(2) << hex << uppercase <<
          setfill('0') << right << (int) enc_msg_crc << ")  (Decoded --        ";

        if(msg_crc == INVALID_CRC)
        {
            outs << "Cannot convert)";
        }
        else
        {
            outs << "(0x" << setw(2) << hex << uppercase <<
              setfill('0') << (int) msg_crc << ")";
            outs << "  (Calc. -- 0x" << setw(2) << hex << uppercase <<
              setfill('0') << (int) calculated_msg_crc << ")";
            if(msg_crc == calculated_msg_crc)
            {
                outs << "  (Msg. CRCs match)"; 
            }
        }
        outs << endl;
    }


    if(att.get_attribute(attribute::ATTRIBUTE_DST_DID))
    {
        outs << left << setw(15) << setfill(' ') << "Dst. DID";
        outs << " -- (Encoded --         0x" << setw(4) << hex << uppercase <<
          setfill('0') << right << enc_dst_did << ")  (Decoded --       ";

        if(raw_dst_did == INVALID_DID)
        {
            outs << "Cannot convert";
        }
        else
        {
            outs << "0x" << setw(3) << hex << uppercase <<
              setfill('0') << raw_dst_did;
        }
        outs << ")" << endl;
    }

    if(att.get_attribute(attribute::ATTRIBUTE_NID))
    {
        outs << left << setw(15) << setfill(' ') << "NID";
        outs << " -- (Encoded -- 0x" << setw(12) << hex << uppercase <<
          setfill('0') << right << enc_nid << ")  (Decoded -- ";

        if(raw_nid == INVALID_NID)
        {
            outs << "Cannot convert";
        }
        else
        {
            outs << "0x" << setw(9) << hex << uppercase <<
              setfill('0') << raw_nid;
        }
        outs << ")" << endl;
    }

    if(att.get_attribute(attribute::ATTRIBUTE_SRC_DID))
    {
        outs << left << setw(15) << setfill(' ') << "Src. DID";
        outs << " -- (Encoded --         0x" << setw(4) << hex << uppercase <<
          setfill('0') << right << enc_src_did << ")  (Decoded --       ";

        if(raw_src_did == INVALID_DID)
        {
            outs << "Cannot convert";
        }
        else
        {
            outs << "0x" << setw(3) << hex << uppercase <<
              setfill('0') << raw_src_did;
        }
        outs << ")" << endl;
    }

    if(att.get_attribute(attribute::ATTRIBUTE_PID))
    {
        outs << left << setw(15) << setfill(' ') << "PID";
        outs << " -- (Encoded --         0x" << setw(4) << hex << uppercase <<
          setfill('0') << right << enc_pid << ")  (Decoded --       ";

        if(raw_pid == INVALID_PID)
        {
            outs << "Cannot convert)";
        }
        else
        {
            outs << "0x" << setw(3) << hex << uppercase <<
              setfill('0') << raw_pid << " : 12 Bits : ";
            outs << value_to_bit_string(raw_pid, 12) << ")\n";

            UInt16 num_blocks_part = ((raw_pid & ONE_NET_RAW_PID_SIZE_MASK) >>
              ONE_NET_RAW_PID_SIZE_SHIFT);
            UInt16 mh_part = ((raw_pid & ONE_NET_RAW_PID_MH_MASK) >>
              ONE_NET_RAW_PID_MH_SHIFT);
            UInt16 sa_part = ((raw_pid & ONE_NET_RAW_PID_STAY_AWAKE_MASK) >>
              ONE_NET_RAW_PID_STAY_AWAKE_SHIFT);
            UInt16 packet_type_part = (raw_pid &
              ONE_NET_RAW_PID_PACKET_TYPE_MASK);

            outs << "(Raw PID Bits 11 - 8(# XTEA blocks):" << value_to_bit_string(
              num_blocks_part, 4) << ": 0x" << hex << uppercase <<
              num_blocks_part << ")\n";
            outs << "(Raw PID Bit 7(MH Bit):" << value_to_bit_string(sa_part, 1) <<
              ": 0x" << hex << uppercase << mh_part << ": Multi-Hop? " <<
              (mh_part ? "Yes" : "No") << ")\n";
            outs << "(Raw PID Bit 6(SA Bit):" << value_to_bit_string(mh_part, 1) <<
              ": 0x" << hex << uppercase << mh_part << ": Stay-Awake? " <<
              (sa_part ? "Yes" : "No") << ")\n";
            outs << "(Raw PID Bits 5 - 0(Packet Type):" << value_to_bit_string(
              packet_type_part, 6) << ": 0x" << hex << setw(2) << setfill('0') <<
              uppercase << packet_type_part <<
              ": Packet Type -- " << get_raw_pid_string(packet_type_part) << ")";
        }
        outs << endl;
    }

    if(att.get_attribute(attribute::ATTRIBUTE_HOPS))
    {
        outs << "Multi-Hop : " << (is_mh_pkt ? "true" : "false");
        if(is_mh_pkt)
        {
            byte_to_hex_string(encoded_hops_field, str);
            outs << " -- Encoded : " << str << " -- ";
            byte_to_hex_string(decoded_hops_field, str);
            outs << " -- Decoded : " << str << " -- ";
            outs << "Hops : " << hops << " -- Max Hops : " << max_hops;
        }
        outs << "\n";
    }

    if(att.get_attribute(attribute::ATTRIBUTE_ENCODED_PAYLOAD))
    {
        outs << dec << "# Encoded Payload Bytes = " << (int) encoded_payload_len
            << "\n";
        if(!bytes_to_hex_string(&enc_pkt_bytes[ON_ENCODED_PLD_IDX],
          encoded_payload_len, str, ' ', 1, 24))
        {
            return false;
        }
        outs << str << endl;
    }

    if(att.get_attribute(attribute::ATTRIBUTE_ENCRYPTED_PAYLOAD))
    {
        outs << "Encrypted Payload (Key = ";
        bytes_to_hex_string(this->key.bytes, ONE_NET_XTEA_KEY_LEN, str, '-',
            1, 0);
        outs << str << ")\n";
        bytes_to_hex_string(this->encrypted_payload_bytes,
            payload.num_payload_bytes, str, ' ', 1, 24);
        outs << str << endl;
    }

    if(att.get_attribute(attribute::ATTRIBUTE_DECRYPTED_PAYLOAD))
    {
        outs << "Decrypted Payload (Key = ";
        bytes_to_hex_string(this->key.bytes, ONE_NET_XTEA_KEY_LEN, str, '-',
            1, 0);
        outs << str << ") -- Pkt. Pld CRC : 0x";
        byte_to_hex_string(payload.payload_crc, str);
        outs << str << " -- Calc. Pld CRC = 0x";
        byte_to_hex_string(payload.calculated_payload_crc, str);
        outs << str;
        if(payload.payload_crc == payload.calculated_payload_crc)
        {
            outs << " (CRCs match)"; 
        }
        outs << endl;
        bytes_to_hex_string(payload.decrypted_payload_bytes,
            payload.num_payload_bytes, str, ' ', 1, 24);
        outs << str << endl;
    }

    if(payload.valid_payload_crc && att.get_attribute(
        attribute::ATTRIBUTE_PAYLOAD_DETAIL))
    {
        if(is_single_pkt && is_data_pkt)
        {
            outs << "Msg. Type : " << (int) payload.msg_type << "(" <<
                (payload.msg_type == ON_APP_MSG ? "App Msg." : (payload.msg_type
                == ON_ADMIN_MSG ? "Admin Msg" : (payload.msg_type ==
                ON_FEATURE_MSG ? "Feature Msg" : "Unknown"))) << ")";
        }
        if(payload.detailed_payload_to_string(payload.raw_pid, str))
        {
            outs << str;
        }
    }

    return true;
}



void packet::display(const vector<packet>& packets, const attribute& att,
        ostream& outs)
{
    const int size = packets.size();
    outs << "\n\n# of packets : " << size << "\n\n";
    for(int i = 0; i < size; i++)
    {
        outs << "\n\nPacket " << i + 1 << "\n\n";
        packets[i].display(att, outs);
    }
}


bool packet::insert_packet(vector<packet>& packets, packet& new_packet)
{
    int num_packets = packets.size();

    if(num_packets == 0 || timeval_compare(new_packet.timestamp,
        packets.at(num_packets - 1).timestamp) > 0)
    {
        packets.push_back(new_packet);
        return true;
    }


    vector<packet>::iterator it = packets.end();
    it--;
    while(1)
    {
        int comp = timeval_compare(new_packet.timestamp, it->timestamp);
        if(comp == 0)
        {
            return false;
        }
        if(comp > 0)
        {
            packets.insert(it, new_packet);
            return true;
        }
        if(it == packets.begin())
        {
            packets.insert(it, new_packet);
            return true;
        }
    }
}


void packet::adjust_timestamps(vector<packet>& packets,
    struct timeval begin_time)
{
    int num_packets = packets.size();
    if(num_packets == 0)
    {
        return;
    }

    bool subtract = false;
    if(timeval_compare(begin_time, packets[0].timestamp) < 0)
    {
        subtract = true;
    }

    struct timeval time_diff = (subtract ? subtract_timeval(
        packets[0].timestamp, begin_time) : subtract_timeval(begin_time,
        packets[0].timestamp));

    for(int i = 0; i < num_packets; i++)
    {
        if(!subtract)
        {
            packets[i].timestamp = add_timeval(packets[i].timestamp,
                time_diff);
        }
        else
        {
            packets[i].timestamp = subtract_timeval(packets[i].timestamp,
                time_diff);
        }
    }
}
