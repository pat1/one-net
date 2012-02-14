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

extern "C"
{
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
};




vector<packet> valid_packets;
vector<packet> invalid_packets;
vector<xtea_key> packet::keys;
vector<xtea_key> packet::invite_keys;



packet::packet()
{
}


packet::packet(const packet& orig)
{

}


packet& packet::operator = (const packet& that)
{

}


packet::~packet()
{
    
}


bool packet::parse_app_payload(payload_t& payload)
{
    return on_parse_app_pld(&payload.decrypted_payload_bytes[ON_PLD_DATA_IDX],
        &payload.app_payload.src_unit, &payload.app_payload.dst_unit,
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


bool packet::parse_response_payload(payload_t& payload, const filter& fltr)
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
    return true;
}


bool packet::parse_invite_payload(payload_t& payload)
{
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
    return false;
}


bool packet::parse_payload(UInt8 raw_pid, UInt8* decrypted_payload_bytes,
    payload_t& payload, const filter& fltr)
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
    payload.calculated_payload_crc = one_net_compute_crc(
        &payload.decrypted_payload_bytes[1],
        payload.num_payload_bytes - 1, ON_PLD_INIT_CRC, ON_PLD_CRC_ORDER);
    payload.valid_payload_crc =
        (payload.payload_crc == payload.calculated_payload_crc);
    payload.txn_nonce = get_payload_txn_nonce(payload.decrypted_payload_bytes);
    payload.resp_nonce = get_payload_resp_nonce(payload.decrypted_payload_bytes);

    if(packet_is_ack(raw_pid) || packet_is_nack(raw_pid))
    {
        payload.ack_nack_handle =
            get_ack_nack_handle(payload.decrypted_payload_bytes);
        packet::parse_response_payload(payload, fltr);
        return packet::parse_response_payload(payload, fltr);
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
    // test timestamp
    if(!fltr.value_accepted(filter::FILTER_TIMESTAMP,
        struct_timeval_to_milliseconds(this->timestamp)))
    {
        return false;
    }

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
    if(!fltr.value_accepted(filter::FILTER_MSG_ID, msg_id))
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


bool packet::fill_in_packet_values(struct timeval timestamp, UInt8 raw_pid,
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

    if(!setup_pkt_ptr(raw_pid, enc_pkt_bytes, &pkt_ptr))
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
    

    valid_decode = true;
    UInt8 raw_nid_bytes[ON_RAW_NID_LEN];
    if(on_decode(raw_nid_bytes, *(pkt_ptr.enc_nid), ON_ENCODED_NID_LEN) !=
        ONS_SUCCESS)
    {
        valid_decode = false;
    }
    else
    {
        UInt32 tmp = one_net_byte_stream_to_int32(raw_nid_bytes);
        raw_nid = (tmp << 4) + (raw_nid_bytes[4] >> 4);
    }


    on_raw_did_t raw_did;
    if(on_decode(raw_did, *(pkt_ptr.enc_src_did), ON_ENCODED_DID_LEN) !=
        ONS_SUCCESS)
    {
        valid_decode = false;
    }
    raw_src_did = did_to_u16(&raw_did);

    if(on_decode(raw_did, *(pkt_ptr.enc_repeater_did), ON_ENCODED_DID_LEN) !=
        ONS_SUCCESS)
    {
        valid_decode = false;
    }
    raw_rptr_did = did_to_u16(&raw_did);

    if(on_decode(raw_did, *(pkt_ptr.enc_dst_did), ON_ENCODED_NID_LEN) !=
        ONS_SUCCESS)
    {
        valid_decode = false;
    }
    raw_dst_did = did_to_u16(&raw_did);

    enc_msg_crc = *(pkt_ptr.enc_msg_crc);
    msg_crc = encoded_to_decoded_byte(enc_msg_crc, TRUE);
    calculated_msg_crc = calculate_msg_crc(&pkt_ptr);

    payload.is_invite_pkt = packet_is_invite(raw_pid);
    is_ack_pkt = packet_is_ack(raw_pid);
    is_nack_pkt = packet_is_nack(raw_pid);
    is_response_pkt = is_ack_pkt || is_nack_pkt;
    is_stay_awake_pkt = packet_is_stay_awake(raw_pid);
    is_block_pkt = packet_is_block(raw_pid);
    is_stream_pkt = packet_is_stream(raw_pid);
    is_single_pkt = packet_is_single(raw_pid);
    is_data_pkt = packet_is_data(raw_pid);
    is_mh_pkt = packet_is_multihop(raw_pid);
    hops = 0;
    max_hops = 0;
    if(is_mh_pkt)
    {
        encoded_hops_field = *(pkt_ptr.enc_hops_field);
        if((decoded_hops_field = encoded_to_decoded_byte(encoded_hops_field,
            false)) = 0xFF)
        {
            valid_decode = false;
        }
        else
        {
            on_parse_hops(encoded_hops_field, &(pkt_ptr.hops),
              &(pkt_ptr.max_hops));
            max_hops = pkt_ptr.max_hops;
            hops = pkt_ptr.hops;
        }
    }

    is_large_pkt = false;
    is_extended_pkt = false;
    if(is_single_pkt || (is_response_pkt && !is_stream_pkt && !is_block_pkt))
    {
        is_large_pkt = (get_num_payload_blocks(raw_pid) == 2);
        is_extended_pkt = (get_num_payload_blocks(raw_pid) == 3);
    }

    if(on_decode(encrypted_payload_bytes, pkt_ptr.payload, encoded_payload_len)
        != ONS_SUCCESS)
    {
        valid_decode = false;
    }
    
    valid_msg_crc = (msg_crc == calculated_msg_crc);
    valid = (valid_decode && valid_msg_crc);

    payload.valid_payload_crc = false;
    bool valid_decrypt = false;

    const vector<xtea_key>* keys = fltr.get_keys(packet_is_invite(raw_pid));

    for(int i = 0; !valid_decrypt && i < keys->size(); i++)
    {
        memmove(payload.decrypted_payload_bytes, encrypted_payload_bytes,
            ON_MAX_ENCODED_PLD_LEN_WITH_TECH);
        xtea_key key = keys->at(i);

        if(on_decrypt(is_stream_pkt ? ON_STREAM : ON_SINGLE,
          payload.decrypted_payload_bytes, (one_net_xtea_key_t*) key.bytes,
          payload.num_payload_bytes + 1) != ONS_SUCCESS)
        {
            continue;
        }

        payload.payload_crc = payload.decrypted_payload_bytes[0];
        payload.calculated_payload_crc = one_net_compute_crc(
            &payload.decrypted_payload_bytes[1],
            payload.num_payload_bytes - 1, ON_PLD_INIT_CRC, ON_PLD_CRC_ORDER);
        payload.valid_payload_crc =
            (payload.payload_crc == payload.calculated_payload_crc);


        if(payload.valid_payload_crc && packet::parse_payload(raw_pid, NULL,
            payload, fltr))
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
        parse_response_payload(payload, fltr);
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


bool packet::create_packet(struct timeval timestamp, UInt8 raw_pid,
    UInt8 num_bytes, const UInt8* const bytes, const filter& fltr, packet& pkt)
{
    return pkt.fill_in_packet_values(timestamp, raw_pid, num_bytes, bytes, fltr);
}


bool packet::create_packet(string line, const filter& fltr, packet& pkt)
{
    static bool rcvd_num_bytes = false;
    static int num_bytes_rcvd;
    static int num_bytes_expected;
    static int raw_pid;
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
        if(tmp != "received")
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

        if(num_bytes_rcvd == ONE_NET_ENCODED_PID_IDX)
        {
            raw_pid = encoded_to_decoded_byte(bytes[num_bytes_rcvd], false);
            if(raw_pid >= 0x40 || get_encoded_packet_len(raw_pid, true)
                != num_bytes_expected)
            {
                rcvd_num_bytes = false;
                return false;
            }
        }
        num_bytes_rcvd++;
    }

    if(num_bytes_rcvd < num_bytes_expected)
    {
        return false;
    }

    rcvd_num_bytes = false; // packet bytes received.  Set false for next time
    return create_packet(timestamp, (UInt8) raw_pid, (UInt8) num_bytes_rcvd,
        bytes, fltr, pkt);
}


bool packet::create_packet(int fd, const filter& fltr, packet& pkt)
{
    return false;
}


bool packet::create_packet(FILE* file, const filter& fltr, packet& pkt)
{
    return false;
}


bool packet::create_packet(istream& is, const filter& fltr, packet& pkt)
{
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
            ss << (int) peer_msg.src_unit;
            ss >> tmp;
            str += tmp;
            str += " -- Peer Unit : ";
            ss << (int) peer_msg.peer_unit;
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
    ss << setfill('0') << setw(1) << hex << (int) src_unit;
    ss << " ";
    ss << setfill('0') << setw(1) << hex << (int) dst_unit;
    ss << " ";
    ss << setfill('0') << setw(4) << hex << (int) msg_class;
    ss << " ";
    ss << setfill('0') << setw(3) << hex << (int) msg_type;
    ss << " ";
    ss << setfill('0') << setw(4) << hex << (int) msg_data;
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
    switch(admin_type)
    {
        case ON_FEATURES_QUERY: return "FEATURES_QUERY";
        case ON_FEATURES_RESP: return "FEATURES_RESP";
        case ON_NEW_KEY_FRAGMENT: return "NEW_KEY_FRAGMENT";
        case ON_ADD_DEV_RESP: return "ADD_DEV_RESP";
        case ON_REMOVE_DEV_RESP: return "REMOVE_DEV_RESP";
        case ON_CHANGE_FRAGMENT_DELAY: return "CHANGE_FRAGMENT_DELAY";
        case ON_CHANGE_FRAGMENT_DELAY_RESP: return "CHANGE_FRAGMENT_DELAY_RESP";
        case ON_CHANGE_KEEP_ALIVE: return "CHANGE_KEEP_ALIVE";
        case ON_ASSIGN_PEER: return "ASSIGN_PEER";
        case ON_UNASSIGN_PEER: return "UNASSIGN_PEER";
        case ON_KEEP_ALIVE_QUERY: return "KEEP_ALIVE_QUERY";
        case ON_KEEP_ALIVE_RESP: return "KEEP_ALIVE_RESP";
        case ON_CHANGE_SETTINGS: return "CHANGE_SETTINGS";
        case ON_CHANGE_SETTINGS_RESP: return "CHANGE_SETTINGS_RESP";
        case ON_ADD_DEV: return "ADD_DEV";
        case ON_RM_DEV: return "RM_DEV";
        default: return "UNKNOWN";
    }
}


string payload_t::get_ack_nack_handle_string(bool is_ack,
    on_ack_nack_handle_t handle)
{
    string prefix_str = is_ack ? "ACK " : "NACK ";

    const char* const ACK_NACK_HANDLE_STR_ARRAY[ON_ACK_MIN_APPLICATION_HANDLE] =
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
        "ADMIN MSG",
        "KEY FRAG",
        "STATUS" // note : this one isn't valid for NACKs
    };

    if((int)handle >= ON_ACK_MIN_APPLICATION_HANDLE)
    {
        return "UNKNOWN";
    }
    if(!is_ack && (int)handle == ON_ACK_STATUS)
    {
        return "INVALID";
    }

    return prefix_str + ACK_NACK_HANDLE_STR_ARRAY[(int) handle];
}


string payload_t::get_nack_reason_string(on_nack_rsn_t nack_reason)
{
    switch(nack_reason)
    {
        case ON_NACK_RSN_NO_ERROR: return "NACK_RSN_NO_ERROR";
        case ON_NACK_RSN_NONCE_ERR: return "NACK_RSN_NONCE_ERR";
        case ON_NACK_RSN_RSRC_UNAVAIL_ERR: return "NACK_RSN_RSRC_UNAVAIL_ERR";
        case ON_NACK_RSN_INTERNAL_ERR: return "NACK_RSN_INTERNAL_ERR";
        case ON_NACK_RSN_BUSY_TRY_AGAIN: return "NACK_RSN_BUSY_TRY_AGAIN";
        case ON_NACK_RSN_BUSY_TRY_AGAIN_TIME: return "NACK_RSN_BUSY_TRY_AGAIN_TIME";
        case ON_NACK_RSN_BAD_POSITION_ERROR: return "NACK_RSN_BAD_POSITION_ERROR";
        case ON_NACK_RSN_BAD_SIZE_ERROR: return "NACK_RSN_BAD_SIZE_ERROR";
        case ON_NACK_RSN_BAD_ADDRESS_ERR: return "NACK_RSN_BAD_ADDRESS_ERR";
        case ON_NACK_RSN_INVALID_MAX_HOPS: return "NACK_RSN_INVALID_MAX_HOPS";
        case ON_NACK_RSN_INVALID_HOPS: return "NACK_RSN_INVALID_HOPS";
        case ON_NACK_RSN_INVALID_PEER: return "NACK_RSN_INVALID_PEER";
        case ON_NACK_RSN_OUT_OF_RANGE: return "NACK_RSN_OUT_OF_RANGE";
        case ON_NACK_RSN_ROUTE_ERROR: return "NACK_RSN_ROUTE_ERROR";
        case ON_NACK_RSN_INVALID_DATA_RATE: return "NACK_RSN_INVALID_DATA_RATE";
        case ON_NACK_RSN_NO_RESPONSE: return "NACK_RSN_NO_RESPONSE";
        case ON_NACK_RSN_INVALID_MSG_ID: return "NACK_RSN_INVALID_MSG_ID";
        case ON_NACK_RSN_NEED_FEATURES: return "NACK_RSN_NEED_FEATURES";
        case ON_NACK_RSN_FEATURES: return "NACK_RSN_FEATURES";
        case ON_NACK_RSN_BAD_CRC: return "NACK_RSN_BAD_CRC";
        case ON_NACK_RSN_BAD_KEY: return "NACK_RSN_BAD_KEY";
        case ON_NACK_RSN_UNSET: return "NACK_RSN_UNSET";
        case ON_NACK_RSN_GENERAL_ERR: return "NACK_RSN_GENERAL_ERR";
        case ON_NACK_RSN_INVALID_LENGTH_ERR: return "NACK_RSN_INVALID_LENGTH_ERR";
        case ON_NACK_RSN_DEVICE_FUNCTION_ERR: return "NACK_RSN_DEVICE_FUNCTION_ERR";
        case ON_NACK_RSN_UNIT_FUNCTION_ERR: return "NACK_RSN_UNIT_FUNCTION_ERR";
        case ON_NACK_RSN_INVALID_UNIT_ERR: return "NACK_RSN_INVALID_UNIT_ERR";
        case ON_NACK_RSN_MISMATCH_UNIT_ERR: return "NACK_RSN_MISMATCH_UNIT_ERR";
        case ON_NACK_RSN_BAD_DATA_ERR: return "NACK_RSN_BAD_DATA_ERR";
        case ON_NACK_RSN_TRANSACTION_ERR: return "NACK_RSN_TRANSACTION_ERR";
        case ON_NACK_RSN_MAX_FAILED_ATTEMPTS_REACHED: return "NACK_RSN_MAX_FAILED_ATTEMPTS_REACHED";
        case ON_NACK_RSN_BUSY: return "NACK_RSN_BUSY";
        case ON_NACK_RSN_NO_RESPONSE_TXN: return "NACK_RSN_NO_RESPONSE_TXN";
    }
}


string packet::get_raw_pid_string(UInt8 raw_pid)
{
    switch(raw_pid)
    {
        case ONE_NET_RAW_MASTER_INVITE_NEW_CLIENT: return "MASTER_INVITE_NEW_CLIENT";
        case ONE_NET_RAW_MH_MASTER_INVITE_NEW_CLIENT: return "MH_MASTER_INVITE_NEW_CLIENT";
        case ONE_NET_RAW_SINGLE_DATA: return "SINGLE_DATA";
        case ONE_NET_RAW_MH_SINGLE_DATA: return "MH_SINGLE_DATA";
        case ONE_NET_RAW_SINGLE_DATA_ACK: return "SINGLE_DATA_ACK";
        case ONE_NET_RAW_MH_SINGLE_DATA_ACK: return "MH_SINGLE_DATA_ACK";
        case ONE_NET_RAW_SINGLE_DATA_NACK_RSN: return "SINGLE_DATA_NACK_RSN";
        case ONE_NET_RAW_MH_SINGLE_DATA_NACK_RSN: return "MH_SINGLE_DATA_NACK_RSN";
        case ONE_NET_RAW_SINGLE_DATA_ACK_STAY_AWAKE: return "SINGLE_DATA_ACK_STAY_AWAKE";
        case ONE_NET_RAW_MH_SINGLE_DATA_ACK_STAY_AWAKE: return "MH_SINGLE_DATA_ACK_STAY_AWAKE";
        case ONE_NET_RAW_SINGLE_DATA_NACK_STAY_AWAKE: return "SINGLE_DATA_NACK_STAY_AWAKE";
        case ONE_NET_RAW_MH_SINGLE_DATA_NACK_STAY_AWAKE: return "MH_SINGLE_DATA_NACK_STAY_AWAKE";
        case ONE_NET_RAW_LARGE_SINGLE_DATA: return "LARGE_SINGLE_DATA";
        case ONE_NET_RAW_MH_LARGE_SINGLE_DATA: return "MH_LARGE_SINGLE_DATA";
        case ONE_NET_RAW_LARGE_SINGLE_DATA_ACK: return "LARGE_SINGLE_DATA_ACK";
        case ONE_NET_RAW_MH_LARGE_SINGLE_DATA_ACK: return "MH_LARGE_SINGLE_DATA_ACK";
        case ONE_NET_RAW_LARGE_SINGLE_DATA_NACK_RSN: return "LARGE_SINGLE_DATA_NACK_RSN";
        case ONE_NET_RAW_MH_LARGE_SINGLE_DATA_NACK_RSN: return "MH_LARGE_SINGLE_DATA_NACK_RSN";
        case ONE_NET_RAW_LARGE_SINGLE_DATA_ACK_STAY_AWAKE: return "LARGE_SINGLE_DATA_ACK_STAY_AWAKE";
        case ONE_NET_RAW_MH_LARGE_SINGLE_DATA_ACK_STAY_AWAKE: return "MH_LARGE_SINGLE_DATA_ACK_STAY_AWAKE";
        case ONE_NET_RAW_LARGE_SINGLE_DATA_NACK_STAY_AWAKE: return "LARGE_SINGLE_DATA_NACK_STAY_AWAKE";
        case ONE_NET_RAW_MH_LARGE_SINGLE_DATA_NACK_STAY_AWAKE: return "MH_LARGE_SINGLE_DATA_NACK_STAY_AWAKE";
        case ONE_NET_RAW_EXTENDED_SINGLE_DATA: return "EXTENDED_SINGLE_DATA";
        case ONE_NET_RAW_MH_EXTENDED_SINGLE_DATA: return "MH_EXTENDED_SINGLE_DATA";
        case ONE_NET_RAW_EXTENDED_SINGLE_DATA_ACK: return "EXTENDED_SINGLE_DATA_ACK";
        case ONE_NET_RAW_MH_EXTENDED_SINGLE_DATA_ACK: return "MH_EXTENDED_SINGLE_DATA_ACK";
        case ONE_NET_RAW_EXTENDED_SINGLE_DATA_NACK_RSN: return "EXTENDED_SINGLE_DATA_NACK_RSN";
        case ONE_NET_RAW_MH_EXTENDED_SINGLE_DATA_NACK_RSN: return "MH_EXTENDED_SINGLE_DATA_NACK_RSN";
        case ONE_NET_RAW_EXTENDED_SINGLE_DATA_ACK_STAY_AWAKE: return "EXTENDED_SINGLE_DATA_ACK_STAY_AWAKE";
        case ONE_NET_RAW_MH_EXTENDED_SINGLE_DATA_ACK_STAY_AWAKE: return "MH_EXTENDED_SINGLE_DATA_ACK_STAY_AWAKE";
        case ONE_NET_RAW_EXTENDED_SINGLE_DATA_NACK_STAY_AWAKE: return "EXTENDED_SINGLE_DATA_NACK_STAY_AWAKE";
        case ONE_NET_RAW_MH_EXTENDED_SINGLE_DATA_NACK_STAY_AWAKE: return "MH_EXTENDED_SINGLE_DATA_NACK_STAY_AWAKE";
        case ONE_NET_RAW_BLOCK_DATA: return "BLOCK_DATA";
        case ONE_NET_RAW_MH_BLOCK_DATA: return "MH_BLOCK_DATA";
        case ONE_NET_RAW_BLOCK_DATA_ACK: return "BLOCK_DATA_ACK";
        case ONE_NET_RAW_MH_BLOCK_DATA_ACK: return "MH_BLOCK_DATA_ACK";
        case ONE_NET_RAW_BLOCK_DATA_NACK_RSN: return "BLOCK_DATA_NACK_RSN";
        case ONE_NET_RAW_MH_BLOCK_DATA_NACK_RSN: return "MH_BLOCK_DATA_NACK_RSN";
        case ONE_NET_RAW_LARGE_BLOCK_DATA_ACK: return "LARGE_BLOCK_DATA_ACK";
        case ONE_NET_RAW_MH_LARGE_BLOCK_DATA_ACK: return "MH_LARGE_BLOCK_DATA_ACK";
        case ONE_NET_RAW_LARGE_BLOCK_DATA_NACK_RSN: return "LARGE_BLOCK_DATA_NACK_RSN";
        case ONE_NET_RAW_MH_LARGE_BLOCK_DATA_NACK_RSN: return "MH_LARGE_BLOCK_DATA_NACK_RSN";
        case ONE_NET_RAW_EXTENDED_BLOCK_DATA_ACK: return "EXTENDED_BLOCK_DATA_ACK";
        case ONE_NET_RAW_MH_EXTENDED_BLOCK_DATA_ACK: return "MH_EXTENDED_BLOCK_DATA_ACK";
        case ONE_NET_RAW_EXTENDED_BLOCK_DATA_NACK_RSN: return "EXTENDED_BLOCK_DATA_NACK_RSN";
        case ONE_NET_RAW_MH_EXTENDED_BLOCK_DATA_NACK_RSN: return "MH_EXTENDED_BLOCK_DATA_NACK_RSN";
        case ONE_NET_RAW_BLOCK_TERMINATE: return "BLOCK_TERMINATE";
        case ONE_NET_RAW_MH_BLOCK_TERMINATE: return "MH_BLOCK_TERMINATE";
        case ONE_NET_RAW_STREAM_DATA: return "STREAM_DATA";
        case ONE_NET_RAW_MH_STREAM_DATA: return "MH_STREAM_DATA";
        case ONE_NET_RAW_STREAM_ACK_KEEP_ALIVE: return "STREAM_ACK_KEEP_ALIVE";
        case ONE_NET_RAW_MH_STREAM_ACK_KEEP_ALIVE: return "MH_STREAM_ACK_KEEP_ALIVE";
        case ONE_NET_RAW_STREAM_NACK_KEEP_ALIVE: return "STREAM_NACK_KEEP_ALIVE";
        case ONE_NET_RAW_MH_STREAM_NACK_KEEP_ALIVE: return "MH_STREAM_NACK_KEEP_ALIVE";
        case ONE_NET_RAW_STREAM_TERMINATE: return "STREAM_TERMINATE";
        case ONE_NET_RAW_MH_STREAM_TERMINATE: return "MH_STREAM_TERMINATE";
        default: return "Invalid";
    }
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
    str += "\nACK / NACK Level (obsolete -- replace with queue level) : ";
    byte_to_hex_string(features_ack_nack_level(features), tmp);
    str += tmp;
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
                ss << response_payload.ack_nack.payload->ack_value.uint32;
                ss >> tmp;
                str += tmp;
                str += " -- 8-bit Value : ";
                ss << (int) response_payload.ack_nack.payload->ack_value.uint8;
                ss >> tmp;
                str += tmp;
            }
            else
            {
                str += " -- Value : ";
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
    }

    str += "\n";
    return true;
}


bool payload_t::detailed_payload_to_string(UInt8 raw_pid, string& str) const
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

    if(att.get_attribute(attribute::ATTRIBUTE_MSG_ID))
    {
        UInt8 enc_msg_id = *(this->pkt_ptr.enc_msg_id);
        UInt8 dec_msg_id;
        outs << "Msg ID";
        byte_to_hex_string(enc_msg_id, str);
        outs << "(Encoded -- 0x" << str << ")  ";
        dec_msg_id = encoded_to_decoded_byte(enc_msg_id, false);
        if(dec_msg_id == 0xFF || !byte_to_hex_string(dec_msg_id, str))
        {
            str = "Cannot Convert";
        }
        else
        {
            str = "0x" + str;
        }
        outs << "(Decoded -- " << str << ")";
        outs << endl;
    }

    if(att.get_attribute(attribute::ATTRIBUTE_MSG_CRC))
    {
        UInt8 enc_msg_crc = *(this->pkt_ptr.enc_msg_crc);
        UInt8 dec_msg_crc;
        outs << "Msg CRC";
        byte_to_hex_string(enc_msg_crc, str);
        outs << "(Encoded -- 0x" << str << ")  ";
        dec_msg_crc = encoded_to_decoded_byte(enc_msg_crc, true);
        if(dec_msg_crc == 0xFF || !byte_to_hex_string(dec_msg_crc, str))
        {
            str = "Cannot Convert";
        }
        else
        {
            str = "0x" + str;
        }
        outs << "(Decoded -- " << str << ")  ";

        byte_to_hex_string(this->calculated_msg_crc, str);
        outs << "(Calculated -- 0x" << str << ")";
        outs << endl;
    }

    if(att.get_attribute(attribute::ATTRIBUTE_RPTR_DID))
    {
        outs << "Rptr. DID";
        encoded_did_to_string(this->pkt_ptr.enc_repeater_did, enc_did_str);
        outs << "(Encoded -- " << enc_did_str << ")  ";

        if(!raw_did_to_string(this->raw_rptr_did, raw_did_str))
        {
            raw_did_str = "Cannot convert";
        }
        outs << "(Decoded -- " << raw_did_str << ")";
        outs << endl;
    }

    if(att.get_attribute(attribute::ATTRIBUTE_DST_DID))
    {
        outs << "Dest. DID";
        encoded_did_to_string(this->pkt_ptr.enc_dst_did, enc_did_str);
        outs << "(Encoded -- " << enc_did_str << ")  ";
        if(!raw_did_to_string(this->raw_dst_did, raw_did_str))
        {
            raw_did_str = "Cannot convert";
        }
        outs << "(Decoded -- " << raw_did_str << ")";
        outs << endl;
    }

    if(att.get_attribute(attribute::ATTRIBUTE_NID))
    {
        outs << "NID";
        encoded_nid_to_string(this->pkt_ptr.enc_nid, enc_nid_str);
        outs << "(Encoded -- " << enc_nid_str << ")  ";
        if(!raw_nid_to_string(this->raw_nid, raw_nid_str))
        {
            raw_nid_str = "Cannot convert";
        }
        outs << "(Decoded -- " << raw_nid_str << ")";
        outs << endl;
    }

    if(att.get_attribute(attribute::ATTRIBUTE_SRC_DID))
    {
        outs << "Src. DID";
        encoded_did_to_string(this->pkt_ptr.enc_src_did, enc_did_str);
        outs << "(Encoded -- " << enc_did_str << ")  ";
        if(!raw_did_to_string(this->raw_src_did, raw_did_str))
        {
            raw_did_str = "Cannot convert";
        }
        outs << "(Decoded -- " << raw_did_str << ")";
        outs << endl;
    }

    if(att.get_attribute(attribute::ATTRIBUTE_PID))
    {
        UInt8 enc_pid = *(this->pkt_ptr.enc_pid);
        outs << "PID";
        byte_to_hex_string(enc_pid, str);
        outs << "(Encoded -- 0x" << str << ")  ";
        byte_to_hex_string(payload.raw_pid, str);
        str = "0x" + str;
        string raw_pid_name_string = packet::get_raw_pid_string(
            payload.raw_pid);
        outs << "(Decoded -- " << str << " -- " << raw_pid_name_string <<
            ")";
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
        outs << "# Encoded Payload bytes = " << (int) encoded_payload_len
            << "\n";
        if(!bytes_to_hex_string(pkt_ptr.payload, encoded_payload_len, str, ' ',
            1, 24))
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
        outs << str << ")\n";
        bytes_to_hex_string(payload.decrypted_payload_bytes,
            payload.num_payload_bytes, str, ' ', 1, 24);
        outs << str << endl;
    }

    if(!payload.is_invite_pkt && payload.valid_payload_crc && att.get_attribute(
        attribute::ATTRIBUTE_NONCE))
    {
        string txn_nonce_str, resp_nonce_str;
        byte_to_hex_string(payload.txn_nonce, txn_nonce_str);
        byte_to_hex_string(payload.resp_nonce, resp_nonce_str);
        outs << "Txn Nonce : 0x" << txn_nonce_str << " -- Resp. Nonce : 0x" <<
            resp_nonce_str << endl;
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
        if(subtract)
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
