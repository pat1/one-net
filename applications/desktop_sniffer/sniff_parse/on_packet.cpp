#include "one_net_packet.h"
#include "one_net_message.h"
#include "one_net_application.h"
#include <string>
#include <cstring>
#include <cctype>
#include <iostream>
#include <sstream>
#include <typeinfo>
#include "on_packet.h"
#include "string_utils.h"
#include "attribute.h"
#include "config_options.h"
#include "one_net_xtea.h"
#include "on_display.h"


const unsigned int NUM_PIDS = 16;
string_int_struct raw_pid_strings[NUM_PIDS] =
{
    {"ONE_NET_RAW_SINGLE_DATA", 0x00},
    {"ONE_NET_RAW_SINGLE_DATA_ACK", 0x01},
    {"ONE_NET_RAW_SINGLE_DATA_NACK_RSN", 0x02},
    {"ONE_NET_RAW_ROUTE",      0x03},
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


on_payload::on_payload()
{
    num_bytes = 0;
    encrypted_payload = "";
    decrypted_payload = "";
    key.assign(32, '0');
    raw_pid = 0;
    valid_crc = false;
    valid = false;
    error_message = "";
}


on_payload::on_payload(UInt16 raw_pid, std::string payload, std::string key,
  bool encrypted)
{
    this->encrypted_payload = payload;
    this->key = key;
    strip_all_whitespace(this->encrypted_payload);
    this->decrypted_payload = this->encrypted_payload;
    strip_all_whitespace(this->key);
    strip_all_hyphens(this->key);
    this->raw_pid = raw_pid;
    num_bytes = get_raw_payload_len(raw_pid);
    valid_decrypt = false;
    valid_crc = false;
    valid = false;
    error_message = "";

    if(this->key.length() != 2 * ONE_NET_XTEA_KEY_LEN)
    {
        error_message = "Key is not 32 characters in length.";
        return;
    }
    if(!legal_digits(this->key, true, false))
    {
        error_message = "Key contains at least one non-hex-digit.";
        return;
    }

    // just check encrypted_payload regardless of whether we were passed
    // encrypted or decrypted
    if(this->encrypted_payload.length() % 2 == 1)
    {
        error_message = "Payload string must contain an even number of characters.";
        return;
    }
    if(!legal_digits(this->encrypted_payload, true, false))
    {
        error_message = "Payload string contains at least one non-hex-digit.";
        return;
    }
    num_bytes = this->encrypted_payload.length() / 2;
    if(num_bytes != 9 && num_bytes != 17 && num_bytes != 25 && num_bytes != 33)
    {
        error_message = "Number of payload bytes must be 9, 17, 25, or 33.";
        return;
    }

    if(packet_is_block(raw_pid) || packet_is_stream(raw_pid))
    {
        if(num_bytes != 33)
        {
            error_message = "Block and stream data packets must have payload lengths of 33";
            return;
        }
    }
    else if(packet_is_invite(raw_pid))
    {
        if(num_bytes != 25)
        {
            error_message = "Invite data packets must have payload lengths of 25";
            return;
        }
    }
    else if(num_bytes == 33)
    {
        error_message = "Only block and stream data packets can have payload lengths of 33";
        return;
    }
    else if(get_raw_payload_len(raw_pid) != num_bytes)
    {
        error_message = "Number of payload bytes does not match the number of payload bytes specified in the raw pid.";
        return;
    }

    if(!string_to_xtea_key(this->key, this->key_bytes))
    {
        error_message = "Internal Error.";
    }

    UInt8 tmp = this->num_bytes;
    if(!hex_string_to_bytes(this->decrypted_payload, this->decrypted_payload_bytes,
      tmp))
    {
        error_message = "Internal Error.";
        return;
    }
    if(this->num_bytes != tmp)
    {
        error_message = "Internal Error.";
        return;
    }

    memcpy(this->encrypted_payload_bytes, this->decrypted_payload_bytes,
      this->num_bytes);

    this->num_rounds = 32;
    if(packet_is_stream(this->raw_pid))
    {
        this->num_rounds = 8;
    }

    if(encrypted)
    {
        if(on_decrypt(packet_is_stream(this->raw_pid), decrypted_payload_bytes,
          &key_bytes, this->num_bytes) != ONS_SUCCESS)
        {
            error_message = "Invalid decryption technique.";
            return;
        }
        this->decrypted_payload = bytes_to_hex_string(decrypted_payload_bytes,
          this->num_bytes);
    }
    else
    {
        if(on_encrypt(packet_is_stream(this->raw_pid), encrypted_payload_bytes,
          &key_bytes, this->num_bytes) != ONS_SUCCESS)
        {
            error_message = "Internal Error.";
            return;
        }
        this->encrypted_payload = bytes_to_hex_string(encrypted_payload_bytes, this->num_bytes);
    }
    this->valid_decrypt = true;

    this->payload_crc = this->decrypted_payload_bytes[0];
    this->msg_id = (this->decrypted_payload_bytes[1] << 4) +
        (this->decrypted_payload_bytes[2] >> 4);

    bool ret = calculate_payload_crc(&this->calculated_payload_crc,
      this->raw_pid, decrypted_payload_bytes);
    if(!ret)
    {
        valid_crc = false;
        error_message = "Could not calculate payload CRC.";
    }
    else
    {
        this->valid_crc = (this->payload_crc == this->calculated_payload_crc);
        if(!this->valid_crc)
        {
            error_message = "Payload CRCs do not match.";
        }
    }
    this->valid = this->valid_crc;
}


on_payload::on_payload(const on_payload& pld)
{
    num_bytes = pld.num_bytes;
    encrypted_payload = pld.encrypted_payload;
    decrypted_payload = pld.encrypted_payload;
    memcpy(encrypted_payload_bytes, pld.encrypted_payload_bytes,
      sizeof(encrypted_payload_bytes));
    memcpy(decrypted_payload_bytes, pld.decrypted_payload_bytes,
      sizeof(decrypted_payload_bytes));
    key = pld.key;
    memcpy(key_bytes, pld.key_bytes, sizeof(key_bytes));
    num_rounds = pld.num_rounds;
    raw_pid = pld.raw_pid;
    calculated_payload_crc = pld.calculated_payload_crc;
    payload_crc = pld.payload_crc;
    msg_id = pld.msg_id;
    error_message = pld.error_message;
    valid_decrypt = pld.valid_decrypt;
    valid_crc = pld.valid_crc;
    valid = pld.valid;
}


on_payload::~on_payload()
{

}


void on_payload::default_display(const on_payload& obj, UInt8 verbosity,
      const attribute* att, ostream& outs)
{
    if(!att)
    {
        attribute all;
        on_payload::default_display(obj, verbosity, &all, outs);
        return;
    }

    bool need_comma = false;
    std::string str;

    if(att->get_attribute(attribute::ATTRIBUTE_KEY))
    {
        if(need_comma)
        {
            outs << ", ";
        }

        outs << "Number of Key Bytes: 16 : Number of Rounds : " << dec << (int) obj.num_rounds << ", " <<
        attribute::attribute_to_string(attribute::ATTRIBUTE_KEY, true) <<
          ": ";

        outs << format_key_string(obj.key_bytes);
        if(verbosity > 10 && packet_is_invite(obj.raw_pid))
        {
            format_invite_key_string(str, obj.key_bytes);
            outs << ", Invite Key String : " << str;
        }
        outs << "\n";
    }
    if(att->get_attribute(attribute::ATTRIBUTE_ENCRYPTED_PAYLOAD))
    {
        if(need_comma)
        {
            outs << ", ";
        }

        outs << "Number of Encrypted Payload Bytes(with technique): " << dec << (int) obj.num_bytes << ", " <<
        attribute::attribute_to_string(attribute::ATTRIBUTE_ENCRYPTED_PAYLOAD, true) <<
          ":\n";

        str = bytes_to_hex_string(obj.encrypted_payload_bytes, obj.num_bytes, ' ', 1, 24);
        outs << str << "\n";
    }
    if(att->get_attribute(attribute::ATTRIBUTE_DECRYPTED_PAYLOAD))
    {
        if(need_comma)
        {
            outs << ", ";
        }

        outs << "Number of Decrypted Payload Bytes(with technique): " << dec << (int) obj.num_bytes << ", " <<
        attribute::attribute_to_string(attribute::ATTRIBUTE_DECRYPTED_PAYLOAD, true) <<
          ":\n";

        str = bytes_to_hex_string(obj.decrypted_payload_bytes, obj.num_bytes, ' ', 1, 24);
        outs << str << "\n";
    }
    bool need_newline = false;
    if(att->get_attribute(attribute::ATTRIBUTE_PAYLOAD_CRC))
    {
        if(need_comma)
        {
            outs << ", ";
        }

        outs << attribute::attribute_to_string(attribute::ATTRIBUTE_PAYLOAD_CRC, true) <<
          ": ";
        outs << "Payload (0x" << byte_to_hex_string(obj.payload_crc);
        outs << ") ";
        outs << "Calculated (0x" << byte_to_hex_string(obj.calculated_payload_crc);
        outs << ") ";
        outs << "CRCs " << ((obj.payload_crc == obj.calculated_payload_crc) ? "" : "do not ")
             << "match.  ";
        need_newline =true;
    }
    if(att->get_attribute(attribute::ATTRIBUTE_MSG_ID))
    {
        if(need_comma)
        {
            outs << ", ";
        }

        outs << attribute::attribute_to_string(attribute::ATTRIBUTE_MSG_ID, true) <<
          ": ";
        if(obj.msg_id > 0xFFF)
            outs << "Invalid";
        else
            outs << "0x" << hex << setfill('0') << setw(3) << obj.msg_id;
        need_newline = true;
    }
    if(need_newline)
        outs << "\n";
}


static std::string data_rate_to_string(UInt8 data_rate)
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

    if(data_rate >= ONE_NET_DATA_RATE_LIMIT)
    {
        return "Invalid";
    }
    return DATA_RATE_STR[data_rate];
}


static std::string priority_to_string(UInt8 priority)
{
    // Priority Strings
    const char* const PRIORITY_STR[3] =
    {
        "None",
        "Low Priority",
        "High"
    };

    if(priority >= 3)
    {
        return "Invalid";
    }
    return PRIORITY_STR[priority];
}


std::string on_payload::detailed_data_rates_to_string(on_features_t features)
{


    std::string str;

    str = "";
    for(UInt8 i = 0; i < ONE_NET_DATA_RATE_LIMIT; i++)
    {
        BOOL dr_capable = features_data_rate_capable(features, i);
        str += "Data rate ";
        str += data_rate_to_string(i);
        if(i < 2)
        {
            str += " ";
        }
        str += " : ";
        str += (dr_capable ? "Capable" : "Not Capable");
        str += "\n";
    }
    return str;
}


std::string on_payload::detailed_features_to_string(on_features_t features, UInt8
  verbosity)
{
    std::string str, tmp;

    str = "Feature Bytes : ";
    tmp = bytes_to_hex_string((UInt8*) &features, sizeof(on_features_t), ' ', 1, 0);
    str += tmp;

    if(verbosity <= 10)
    {
        return str;
    }

    // TODO -- change to decimal
    str += "\nMax Hops : 0x";
    str += byte_to_hex_string(features_max_hops(features));
    str += "\nMax Peers : 0x";
    str += byte_to_hex_string(features_max_peers(features));
    str += "\nQueue Size : 0x";
    str += byte_to_hex_string(features_queue_size(features));
    str += "\nQueue Level : 0x";
    str += byte_to_hex_string(features_queue_level(features));
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
    str += detailed_data_rates_to_string(features);
    str += "\n";
    return str;
}







on_single_data_payload::on_single_data_payload(): on_payload()
{
}


on_single_data_payload::on_single_data_payload(UInt16 raw_pid, std::string payload, std::string key,
  bool encrypted): on_payload(raw_pid, payload, key, encrypted)
{
    if(!this->valid)
    {
        return;
    }
    this->payload_msg_type = get_payload_msg_type(this->decrypted_payload_bytes);
    is_app_pkt = false;
    is_admin_pkt = false;
    is_features_pkt = false;
    is_route_pkt = false;

    switch(this->payload_msg_type)
    {
        case ON_APP_MSG:
            is_app_pkt = true;
            break;
        case ON_ADMIN_MSG:
            is_admin_pkt = true;
            break;
        case ON_FEATURE_MSG:
            is_features_pkt = true;
            break;
        case ON_ROUTE_MSG:
            is_route_pkt = true;
            break;
        default:
            // These are user-defined packet parsing mechanisms.  If you have
            // one, add it to the cases above.  "default" case should assign this
            // as invalid.

            // TODO -- actually assign as invalid?
            break;
    }
}


on_single_data_payload::on_single_data_payload(const on_single_data_payload& pld): on_payload(pld)
{
    payload_msg_type = pld.payload_msg_type;
    is_app_pkt = pld.is_app_pkt;
    is_admin_pkt = pld.is_admin_pkt;
    is_features_pkt = pld.is_features_pkt;
    is_route_pkt = pld.is_route_pkt;
}


on_single_data_payload::~on_single_data_payload()
{

}


void on_single_data_payload::default_display(const on_single_data_payload& obj, UInt8 verbosity,
  const attribute* att, ostream& outs)
{
    // just a meaningless self-nullifying manipulation of verbosity to avoid compiler warnings.
    verbosity = ~verbosity;
    verbosity = ~verbosity;

    if(obj.valid_crc && att->get_attribute(attribute::ATTRIBUTE_PAYLOAD_DETAIL))
    {
        outs << "Msg. Type : " << (int)obj.payload_msg_type << "(" <<
          (obj.payload_msg_type == ON_APP_MSG ? "App Msg" : (obj.payload_msg_type
          == ON_ADMIN_MSG ? "Admin Msg" : (obj.payload_msg_type ==
          ON_FEATURE_MSG ? "Feature Msg" : (obj.payload_msg_type == ON_ROUTE_MSG ?
          "Route Msg" : "Unknown")))) << ")\n";
    }
}


void on_single_data_payload::display(UInt8 verbosity, const attribute* att,
  ostream& outs) const
{
    if(att == NULL)
    {
        attribute attr;
        on_single_data_payload::display(verbosity, &attr, outs);
        return;
    }

    if(on_single_data_payload::disp_single_data_pay == NULL)
    {
        return;
    }

    (*disp_single_data_pay)(*this, verbosity, att, outs);
}


void on_single_data_payload::set_display_on_single_data_pay_function(display_on_single_data_pay_func func)
{
    if(func == NULL)
    {
        return;
    }
    on_single_data_payload::disp_single_data_pay = func;
}
display_on_single_data_pay_func on_single_data_payload::disp_single_data_pay = &on_single_data_payload::default_display;







on_app_payload::on_app_payload(): on_single_data_payload()
{
}


on_app_payload::on_app_payload(UInt8 src_unit, UInt8 dst_unit, UInt8 msg_class,
      UInt8 msg_type, SInt32 msg_data)
{
    this->src_unit = src_unit;
    this->dst_unit = dst_unit;
    this->msg_class = msg_class;
    this->msg_type = msg_type;
    this->msg_data = msg_data;
}


on_app_payload::on_app_payload(UInt16 raw_pid, std::string payload, std::string key,
  bool encrypted): on_single_data_payload(raw_pid, payload, key, encrypted)
{
    if(!valid_crc)
    {
        return;
    }

    if(!on_parse_app_pld(&this->decrypted_payload_bytes[ONA_DATA_INDEX],
      payload_msg_type, &this->src_unit, &this->dst_unit, &this->msg_class,
      &this->msg_type, &this->msg_data))
    {
        error_message = "Could not parse Application payload";
        valid = false;
    }
}


on_app_payload::on_app_payload(const on_app_payload& pld): on_single_data_payload(pld)
{
    src_unit = pld.src_unit;
    dst_unit = pld.dst_unit;
    msg_class = pld.msg_class;
    msg_type = pld.msg_type;
    msg_data = pld.msg_data;
}


on_app_payload::~on_app_payload()
{

}


std::string on_app_payload::get_msg_class_string(UInt16 msg_class)
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


void on_app_payload::default_display_application_payload_info(const on_app_payload& obj,
    UInt8 verbosity, const attribute* att, ostream& outs)
{
    if(att == NULL)
    {
        attribute attr;
        on_app_payload::default_display_application_payload_info(obj, verbosity, &attr, outs);
        return;
    }

    bool need_comma = false;

    if(att->get_attribute(attribute::ATTRIBUTE_SRC_UNIT))
    {
        if(need_comma)
        {
            outs << ", ";
        }
        outs << attribute::attribute_to_string(attribute::ATTRIBUTE_SRC_UNIT, true)
             << ": " << dec << (int) obj.src_unit;
        need_comma = true;
    }
    if(att->get_attribute(attribute::ATTRIBUTE_DST_UNIT))
    {
        if(need_comma)
        {
            outs << ", ";
        }
        outs << attribute::attribute_to_string(attribute::ATTRIBUTE_DST_UNIT, true)
             << ": " << dec << (int) obj.dst_unit;
        need_comma = true;
    }
    if(att->get_attribute(attribute::ATTRIBUTE_MSG_CLASS))
    {
        if(need_comma)
        {
            outs << ", ";
        }
        outs << attribute::attribute_to_string(attribute::ATTRIBUTE_MSG_CLASS, true)
             << ": 0x" << uint16_to_hex_string(obj.msg_class);
        if(verbosity > 10)
        {
            outs << "(" + on_app_payload::get_msg_class_string(obj.msg_class) << ")";
        }
        need_comma = true;
    }
    if(att->get_attribute(attribute::ATTRIBUTE_MSG_TYPE))
    {
        if(need_comma)
        {
            outs << ", ";
        }
        outs << attribute::attribute_to_string(attribute::ATTRIBUTE_MSG_TYPE, true)
             << ": 0x" << uint16_to_hex_string(obj.msg_type);
        need_comma = true;
    }
    if(att->get_attribute(attribute::ATTRIBUTE_MSG_DATA))
    {
        if(need_comma)
        {
            outs << ", ";
        }
        outs << attribute::attribute_to_string(attribute::ATTRIBUTE_MSG_DATA, true)
             << ": 0x" << uint16_to_hex_string(obj.msg_data);
        need_comma = true;
    }

    if(need_comma)
    {
        outs << "\n";
    }
}


void on_app_payload::default_display(const on_app_payload& obj, UInt8 verbosity,
  const attribute* att, ostream& outs)
{
    on_payload::default_display(obj, verbosity, att, outs);
    on_single_data_payload::default_display(obj, verbosity, att, outs);
    outs << "App Payload : ";
    on_app_payload::default_display_application_payload_info(obj, verbosity,
      att, outs);
}


void on_app_payload::display(UInt8 verbosity, const attribute* att,
  ostream& outs) const
{
    if(att == NULL)
    {
        attribute attr;
        on_app_payload::display(verbosity, &attr, outs);
        return;
    }

    if(on_app_payload::disp_app_pay == NULL)
    {
        return;
    }

    (*disp_app_pay)(*this, verbosity, att, outs);
}


void on_app_payload::set_display_on_app_pay_function(display_on_app_pay_func func)
{
    if(func == NULL)
    {
        return;
    }
    on_app_payload::disp_app_pay = func;
}
display_on_app_pay_func on_app_payload::disp_app_pay = &on_app_payload::default_display;





on_response_payload::on_response_payload(): on_payload()
{
    ack_nack.payload = &ack_nack_payload;
}


on_response_payload::on_response_payload(UInt16 raw_pid, std::string payload, std::string key,
  bool encrypted): on_payload(raw_pid, payload, key, encrypted)
{
    if(!valid_crc)
    {
        return;
    }

    ack_nack.payload = &this->ack_nack_payload;
    if(on_parse_response_pkt(raw_pid, this->decrypted_payload_bytes, &ack_nack) !=
      ONS_SUCCESS)
    {
        valid = false;
        error_message= "Could not parse response payload";
    }
}


on_response_payload::on_response_payload(const on_response_payload& pld): on_payload(pld)
{
    ack_nack.payload = &ack_nack_payload;
    ack_nack.nack_reason = pld.ack_nack.nack_reason;
    ack_nack.handle = pld.ack_nack.handle;
    memcpy(&ack_nack.payload, &pld.ack_nack.payload, sizeof(ack_nack_payload));
}


on_response_payload::~on_response_payload()
{

}


std::string on_response_payload::get_nack_reason_string(on_nack_rsn_t nack_reason)
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


string on_response_payload::get_ack_nack_handle_string(bool is_ack,
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
        "KEY / KEY FRAG",

        // the 4 handles below are only valid for ACKs
        "BLOCK PACKETS RCVD",
        "ROUTE",
        "APP MSG",
        "ADMIN MSG",

        "USR MSG"
    };


    if((int)handle > ON_ACK_MAX_HANDLE)
    {
        return "UNKNOWN";
    }
    if(!is_ack && ((int)handle >= ON_ACK_BLK_PKTS_RCVD &&
      (int)handle <= ON_ACK_ADMIN_MSG))
    {
        return "INVALID";
    }

    return prefix_str + ACK_NACK_HANDLE_STR_ARRAY[(int) handle];
}


UInt8 on_response_payload::get_num_relevant_bytes_from_ack_nack_handle(
     const on_ack_nack_t& ack_nack, UInt8 num_xtea_blocks)
{
    bool is_ack = (ack_nack.nack_reason == ON_NACK_RSN_NO_ERROR);
    if(num_xtea_blocks < 1)
    {
        // call it an error, return 0
        return 0;
    }

    UInt8 num_raw_bytes = num_xtea_blocks * ONE_NET_XTEA_BLOCK_SIZE -
      (is_ack ? ON_PLD_DATA_IDX : ON_PLD_DATA_IDX + 1);


    if(ack_nack.handle == (int) ON_ACK_APPLICATION_HANDLE)
    {
        return num_raw_bytes;
    }

    switch(ack_nack.handle)
    {
        case ON_ACK: return 0;
        case ON_ACK_FEATURES: return sizeof(on_features_t);
        case ON_ACK_DATA: return num_raw_bytes;
        case ON_ACK_VALUE: case ON_ACK_SLOW_DOWN_TIME_MS: case ON_ACK_SPEED_UP_TIME_MS:
        case ON_ACK_PAUSE_TIME_MS: case ON_ACK_RESPONSE_TIME_MS: case ON_ACK_TIME_MS:
        case ON_ACK_TIMEOUT_MS:
            return sizeof(UInt32);
        case ON_ACK_ADMIN_MSG:
            // can't really tell, so say it is num_raw_bytes
            return num_raw_bytes;
        case ON_ACK_KEY:
            return ONE_NET_XTEA_KEY_FRAGMENT_SIZE;
        case ON_ACK_BLK_PKTS_RCVD: case ON_ACK_APP_MSG:
            return 5;
        case ON_ACK_ROUTE:
            return num_raw_bytes;
        default:
            return 0;
    }
}


void on_response_payload::default_ack_nack_display(const on_ack_nack_t& ack_nack, UInt8 verbosity,
  const attribute* att, ostream& outs, UInt8 num_xtea_blocks)
{
    if(att == NULL)
    {
        attribute attr;
        on_response_payload::default_ack_nack_display(ack_nack, verbosity, &attr, outs,
          num_xtea_blocks);
        return;
    }
    
    bool is_ack = (ack_nack.nack_reason == ON_NACK_RSN_NO_ERROR);
    UInt8 num_relevant_bytes = get_num_relevant_bytes_from_ack_nack_handle(ack_nack,
      num_xtea_blocks);

    outs << "Packet Is ACK : " << (is_ack ? "True" : "False");
    if(!is_ack)
    {
        outs << " -- Nack Reason : " << hex << "0x" << byte_to_hex_string((UInt8) ack_nack.nack_reason);

        if(att->get_attribute(attribute::ATTRIBUTE_PAYLOAD_DETAIL) && verbosity > 10)
        {
            outs << " (" << on_response_payload::get_nack_reason_string(ack_nack.nack_reason)
                 << ")";
        }
    }

    outs << " -- ACK / NACK Handle : " << hex << "0x" << byte_to_hex_string((UInt8) ack_nack.handle);
    if(att->get_attribute(attribute::ATTRIBUTE_PAYLOAD_DETAIL) && verbosity > 10)
    {
        outs << " (" << on_response_payload::get_ack_nack_handle_string(is_ack, ack_nack.handle)
             << ")";
    }
    outs << " -- Payload Bytes : " << bytes_to_hex_string(ack_nack.payload->ack_payload,
      num_relevant_bytes);
    if(ack_nack.handle == ON_ACK)
    {
        outs<< "N/A";
    }
    outs << "\n";

    switch(ack_nack.handle)
    {
        case ON_ACK:
            break;
        case ON_ACK_FEATURES:
            outs << detailed_features_to_string(ack_nack.payload->features, verbosity);
            break;
        case ON_ACK_DATA:
            outs << bytes_to_hex_string(is_ack ?
                ack_nack.payload->ack_payload :
                ack_nack.payload->nack_payload, num_relevant_bytes, ' ', 1, 0);
            break;
        case ON_ACK_VALUE:
            outs << " -- 32-bit Value : ";
            outs << dec << ack_nack.payload->ack_value;
            break;
        case ON_ACK_TIME_MS: case ON_ACK_TIMEOUT_MS:
        case ON_ACK_SLOW_DOWN_TIME_MS: case ON_ACK_SPEED_UP_TIME_MS:
        case ON_ACK_PAUSE_TIME_MS:
            outs << " -- Time ms : ";
            outs << dec << ack_nack.payload->ack_time_ms;
            break;
        case ON_ACK_ADMIN_MSG:
        {
            outs << "ACK Admin Payload...\n";
            on_admin_payload admin_payload(ack_nack.payload->admin_msg,
              sizeof(ack_nack.payload->admin_msg));
            on_admin_payload::admin_payload_details_to_stream(admin_payload, verbosity,
              att, outs);
            break;
        }
        case ON_ACK_APP_MSG:
        {
            outs << "Single App Message : ";
            
            UInt8 src_unit, dst_unit, msg_type;
            UInt8 msg_class;
            SInt32 msg_data;           
            if(!on_parse_app_pld(ack_nack.payload->app_msg, ON_APP_MSG,
              &src_unit, &dst_unit, &msg_class, &msg_type, &msg_data))
            {
                outs << "Could not parse single app message response.";
            }
            else
            {
                on_app_payload app_msg_payload(src_unit, dst_unit, msg_class,
                  msg_type, msg_data);
                on_app_payload::default_display_application_payload_info(app_msg_payload,
                  verbosity, att, outs);
            }
            break;
        }
        case ON_ACK_KEY:
        {
           outs << bytes_to_hex_string(ack_nack.payload->key_frag,
               ONE_NET_XTEA_KEY_FRAGMENT_SIZE, ' ', 1, 0);
           break;
        }
        case ON_ACK_BLK_PKTS_RCVD:
        {
            outs << "Bits : ";
            outs << bytes_to_bit_string(ack_nack.payload->ack_payload, 5);
        }
        case ON_ACK_ROUTE:
        {
            outs << on_route_payload::route_payload_info_to_string(
                ack_nack.payload->ack_payload, verbosity);
        }
        default:
            // for everything else, assume it is raw bytes.
           outs << bytes_to_hex_string(is_ack ?
                ack_nack.payload->ack_payload :
                ack_nack.payload->nack_payload, num_relevant_bytes, ' ', 1, 0);
    }

    outs << "\n";
}


void on_response_payload::display_ack_nack(const on_ack_nack_t& ack_nack, UInt8 verbosity, const attribute* att,
  ostream& outs, UInt8 num_xtea_blocks)
{
    if(att == NULL)
    {
        attribute attr;
        display_ack_nack(ack_nack, verbosity, &attr, outs, num_xtea_blocks);
        return;
    }

    (*disp_ack_nack)(ack_nack, verbosity, att, outs, num_xtea_blocks);
}


void on_response_payload::default_display(const on_response_payload& obj, UInt8 verbosity,
  const attribute* att, ostream& outs)
{
    if(att == NULL)
    {
        attribute attr;
        on_response_payload::default_display(obj, verbosity, &attr, outs);
        return;
    }

    on_payload::default_display(obj, verbosity, att, outs);
    outs << "Response Payload : ";
    on_response_payload::display_ack_nack(obj.ack_nack, verbosity, att, outs);
}


void on_response_payload::display(UInt8 verbosity, const attribute* att,
  ostream& outs) const
{
    if(on_response_payload::disp_response_pay == NULL)
    {
        return;
    }

    (*disp_response_pay)(*this, verbosity, att, outs);
}


void on_response_payload::set_display_on_ack_nack_function(display_on_ack_nack_func func)
{
    if(func == NULL)
    {
        return;
    }
    on_response_payload::disp_ack_nack = func;
}


void on_response_payload::set_display_on_response_pay_function(display_on_response_pay_func func)
{
    if(func == NULL)
    {
        return;
    }
    on_response_payload::disp_response_pay = func;
}
display_on_ack_nack_func on_response_payload::disp_ack_nack = &on_response_payload::default_ack_nack_display;
display_on_response_pay_func on_response_payload::disp_response_pay = &on_response_payload::default_display;






on_admin_payload::on_admin_payload(): on_single_data_payload()
{
}


on_admin_payload::on_admin_payload(const UInt8* bytes, UInt8 num_bytes)
{
    this->parse_bytes(bytes, num_bytes);
}


on_admin_payload::on_admin_payload(UInt16 raw_pid, std::string payload, std::string key,
  bool encrypted): on_single_data_payload(raw_pid, payload, key, encrypted)
{
    if(!this->valid)
    {
        return;
    }

    on_admin_payload::parse_bytes(&this->decrypted_payload_bytes[ON_PLD_DATA_IDX],
      21); // might be copying some extra bytes, but who cares?
}


on_admin_payload::on_admin_payload(const on_admin_payload& pld): on_single_data_payload(pld)
{
    admin_type = pld.admin_type;
    memcpy(admin_data_bytes, pld.admin_data_bytes, sizeof(admin_data_bytes));
    num_admin_bytes = pld.num_admin_bytes;
}


on_admin_payload::~on_admin_payload()
{

}



void on_admin_payload::parse_bytes(const UInt8* pld_bytes, UInt8 num_bytes)
{
    if(num_bytes > sizeof(admin_data_bytes) + 1)
    {
        this->valid = false;
        error_message = "Internal Error : Storage buffer is not large enough.";
        return;
    }

    if(pld_bytes == NULL)
    {
        this->valid = false;
        error_message = "Internal Error : NULL parameter.";
        return;
    }
    this->admin_type = pld_bytes[0];
    memcpy(this->admin_data_bytes, &pld_bytes[1], num_bytes);
    memcpy(this->bytes, &pld_bytes[1], num_bytes);

    switch(this->admin_type)
    {
        case ON_FEATURES_QUERY: num_admin_bytes = 4; break;

        case ON_FEATURES_RESP: num_admin_bytes = 4; break;

        case ON_NEW_KEY_FRAGMENT: num_admin_bytes = 4; break;

        case ON_ADD_DEV_RESP: num_admin_bytes = 4; break;

        case ON_REMOVE_DEV_RESP: num_admin_bytes = 4; break;

        case ON_CHANGE_DATA_RATE_CHANNEL: num_admin_bytes = 6;
          this->dr_channel.pause_time_ms = one_net_byte_stream_to_uint16(&bytes[2]);
          this->dr_channel.dormant_time_ms = one_net_byte_stream_to_uint16(&bytes[4]);
          break;

        case ON_REQUEST_KEY_CHANGE: num_admin_bytes = 0; break;

        // TODO -- Does ON_CHANGE_FRAGMENT_DELAY_RESP have a payload?
        case ON_CHANGE_FRAGMENT_DELAY_RESP:
        case ON_CHANGE_FRAGMENT_DELAY: num_admin_bytes = 4;
          this->change_frag_delay.frag_delay_low_ms = one_net_byte_stream_to_uint16(&bytes[ON_FRAG_LOW_IDX]);
          this->change_frag_delay.frag_delay_high_ms = one_net_byte_stream_to_uint16(&bytes[ON_FRAG_HIGH_IDX]);
          break;
        case ON_CHANGE_KEEP_ALIVE: num_admin_bytes = 4; break;
          this->value = one_net_byte_stream_to_uint32(&bytes[0]);
          break;

        // the structures of these two message pairs are identical, so treat them the same
        case ON_ADD_DEV:
        case ON_RM_DEV:
        case ON_ASSIGN_PEER:
        case ON_UNASSIGN_PEER:
        {
            num_admin_bytes = 4;
            this->add_remove_client.enc_did = one_net_byte_stream_to_uint16(&bytes[0]);
            if(on_decode_uint16(&this->add_remove_client.raw_did,
              this->add_remove_client.enc_did) != ONS_SUCCESS)
            {
                valid = false;
                error_message = "Unable to decode DID.";
            }
            break;
        }

        case ON_KEEP_ALIVE_QUERY: num_admin_bytes = 0; break;

        case ON_KEEP_ALIVE_RESP: num_admin_bytes = 4; break;

        case ON_CHANGE_SETTINGS: num_admin_bytes = 1; break;

        case ON_CHANGE_SETTINGS_RESP: num_admin_bytes = 0; break;

        case ON_REQUEST_BLOCK_STREAM: num_admin_bytes = 20;
          // much is irrelevant for stream
          this->bs_transfer_request.bs_flags =
            pld_bytes[BLOCK_STREAM_SETUP_FLAGS_IDX];
          this->bs_transfer_request.transfer_size =
            one_net_byte_stream_to_uint32(&pld_bytes[BLOCK_STREAM_SETUP_TRANSFER_SIZE_IDX]);
          this->bs_transfer_request.chunk_size =
            pld_bytes[BLOCK_STREAM_SETUP_CHUNK_SIZE_IDX];
          this->bs_transfer_request.frag_delay_ms =
            one_net_byte_stream_to_uint16(&pld_bytes[BLOCK_STREAM_SETUP_FRAG_DLY_IDX]);
          this->bs_transfer_request.chunk_pause_ms =
            one_net_byte_stream_to_uint16(&pld_bytes[BLOCK_STREAM_SETUP_CHUNK_PAUSE_IDX]);
          this->bs_transfer_request.channel =
            pld_bytes[BLOCK_STREAM_SETUP_CHANNEL_IDX];
          this->bs_transfer_request.data_rate =
            pld_bytes[BLOCK_STREAM_SETUP_DATA_RATE_IDX];
          this->bs_transfer_request.timeout_ms =
            one_net_byte_stream_to_uint16(&pld_bytes[BLOCK_STREAM_SETUP_TIMEOUT_IDX]);
          this->bs_transfer_request.enc_dst_did =
            one_net_byte_stream_to_uint16(&pld_bytes[BLOCK_STREAM_SETUP_DST_IDX]);

          if(on_decode_uint16(&this->bs_transfer_request.raw_dst_did,
            this->bs_transfer_request.enc_dst_did) != ONS_SUCCESS)
          {
              valid = false;
              error_message = "Unable to decode Dst DID.";
              return;
          }

          this->bs_transfer_request.estimated_time_ms =
            one_net_byte_stream_to_uint32(&pld_bytes[BLOCK_STREAM_SETUP_ESTIMATED_TIME_IDX]);
          break;

        case ON_REQUEST_REPEATER: num_admin_bytes = 13;
          this->reserve_repeater.enc_rptr_did = one_net_byte_stream_to_uint16(&bytes[0]);
          this->reserve_repeater.enc_src_did = one_net_byte_stream_to_uint16(&bytes[2]);
          this->reserve_repeater.enc_dst_did = one_net_byte_stream_to_uint16(&bytes[4]);
          if(on_decode_uint16(&this->reserve_repeater.raw_rptr_did,
            this->reserve_repeater.enc_rptr_did) != ONS_SUCCESS)
          {
              valid = false;
              error_message = "Unable to decode Repeater DID.";
          }
          if(on_decode_uint16(&this->reserve_repeater.raw_src_did,
            this->reserve_repeater.enc_src_did) != ONS_SUCCESS)
          {
              valid = false;
              error_message = "Unable to decode Source DID.";
          }
          if(on_decode_uint16(&this->reserve_repeater.raw_dst_did,
            this->reserve_repeater.enc_dst_did) != ONS_SUCCESS)
          {
              valid = false;
              error_message = "Unable to decode Destination DID.";
          }

          this->reserve_repeater.transfer_time_ms = one_net_byte_stream_to_uint32(&bytes[6]);
          this->reserve_repeater.channel = one_net_byte_stream_to_uint32(&bytes[10]);
          this->reserve_repeater.data_rate = one_net_byte_stream_to_uint32(&bytes[11]);
          this->reserve_repeater.priority = one_net_byte_stream_to_uint32(&bytes[12]);
          break;

        case ON_TERMINATE_BLOCK_STREAM: num_admin_bytes = 10;
          this->terminate_block_stream.enc_terminating_did = one_net_byte_stream_to_uint16(&bytes[0]);
          if(on_decode_uint16(&this->terminate_block_stream.raw_terminating_did,
            this->terminate_block_stream.enc_terminating_did) != ONS_SUCCESS)
          {
              valid = false;
              error_message = "Unable to decode Terminating DID.";
          }
          this->terminate_block_stream.response_ack_nack.payload =
              (ack_nack_payload_t*) this->terminate_block_stream.ack_nack_payload_bytes;
          memcpy(terminate_block_stream.ack_nack_payload_bytes, &bytes[5], 5);
          valid = (on_parse_response_pkt(raw_pid, bytes, &terminate_block_stream.response_ack_nack) ==
                  ONS_SUCCESS);
          break;

        default: /* Stick any other cases above.  Flag this one as invalid. */
            num_admin_bytes = 0; valid = false; return;
    }
}


std::string on_admin_payload::get_admin_type_string(UInt8 admin_type)
{
    static map<int, string> pairs =
      create_int_string_map(admin_msg_type_strings, NUM_ADMIN_MSG_TYPES);

    map<int,std::string>::iterator it = pairs.find((int) admin_type);
    if(it == pairs.end())
    {
        return "Invalid";
    }
    return it->second;
}


void on_admin_payload::admin_payload_details_to_stream(
  const on_admin_payload& obj, UInt8 verbosity, const attribute* att, ostream& outs)
{
    if(att == NULL)
    {
        attribute attr;
        on_admin_payload::admin_payload_details_to_stream(obj, verbosity, &attr, outs);
        return;
    }

    outs << "Admin Payload : ";


    std::string str;
    outs << "Admin Type : 0x";
    outs << byte_to_hex_string(obj.admin_type);
    outs << "(" << on_admin_payload::get_admin_type_string(obj.admin_type);
    outs << ") -- Num Admin Payload Bytes : ";
    outs << dec << (int) obj.num_admin_bytes << " -- ";
    str = bytes_to_hex_string(obj.admin_data_bytes, obj.num_admin_bytes, ' ', 1, 24);
    outs << str << "\n";

    if(verbosity <= 10 || !att->get_attribute(attribute::ATTRIBUTE_PAYLOAD_DETAIL))
    {
        return;
    }

    switch(obj.admin_type)
    {
        case ON_NEW_KEY_FRAGMENT: case ON_KEEP_ALIVE_RESP:
            outs << "Key Fragment : " << bytes_to_hex_string(obj.key_frag, 4, '-', 1, 0);
            break;
        case ON_FEATURES_QUERY: case ON_FEATURES_RESP:
            outs << "Features..." << detailed_features_to_string(obj.features, verbosity);
            break;
        case ON_KEEP_ALIVE_QUERY: case ON_REQUEST_KEY_CHANGE: case ON_CHANGE_SETTINGS_RESP:
            outs << "No payload";
            break;
        // TODO -- Do the cases below all have the same payload format.
        case ON_ADD_DEV: case ON_RM_DEV: case ON_ADD_DEV_RESP: case ON_REMOVE_DEV_RESP:
            outs << detailed_did_display(obj.add_remove_client.enc_did,
              obj.add_remove_client.raw_did);
            outs << " -- # Multi-Hop : ";
            outs << dec << (int) obj.add_remove_client.num_mh_devices;
            outs << " -- # Multi-Hop Repeater : ";
            outs << dec << (int) obj.add_remove_client.num_mh_repeaters;
            break;
        case ON_ASSIGN_PEER: case ON_UNASSIGN_PEER:
            outs << detailed_did_display(obj.peer_msg.enc_did,
              obj.peer_msg.raw_did);
            outs << " -- Src Unit : ";
            outs << dec << (int) obj.peer_msg.src_unit;
            outs << " -- Peer Unit : ";
            outs << dec << (int) obj.peer_msg.peer_unit;
            break;

        case ON_CHANGE_DATA_RATE_CHANNEL:
            outs << "Channel : " << dec << (int)obj.dr_channel.channel;
            outs << " -- Data Rate : " << dec << (int)obj.dr_channel.data_rate;
            outs << " -- Pause Time : " << dec << (int)obj.dr_channel.pause_time_ms << " ms";
            outs << " -- Dormant Time : " << dec << (int)obj.dr_channel.dormant_time_ms << " ms";
            break;

        case ON_CHANGE_FRAGMENT_DELAY:
        case ON_CHANGE_FRAGMENT_DELAY_RESP:
            outs << " -- Frag.Delay Low : " << dec << (int)obj.change_frag_delay.frag_delay_low_ms << " ms";
            outs << " -- Frag.Delay High : " << dec << (int)obj.change_frag_delay.frag_delay_high_ms << " ms";
            break;
        case ON_CHANGE_KEEP_ALIVE:
            outs << "Time Milliseconds : ";
            outs << obj.value;
            break;
        case ON_CHANGE_SETTINGS:
            display_flags_byte(obj.flags, outs);
            break;

        case ON_REQUEST_BLOCK_STREAM:
        {
            UInt8 priority = get_bs_priority(obj.bs_transfer_request.bs_flags);
            UInt8 hops = get_bs_hops(obj.bs_transfer_request.bs_flags);
            UInt8 data_rate = obj.bs_transfer_request.data_rate;
            bool transfer_type = get_bs_transfer_type(obj.bs_transfer_request.bs_flags);

            outs << "Block / Stream Flags : 0x";
            outs << hex << (int) obj.bs_transfer_request.bs_flags << " (Bits = "
                 << value_to_bit_string(obj.flags, 8) << ")";

            outs << " -- Hops : " << dec << (int) hops;
            outs << " -- Priority : " << dec << (int) priority
                 << "(" << priority_to_string(priority) << ")";
            outs << " -- Transfer Type : ";
            outs << ((transfer_type == ON_BLK_TRANSFER) ? "Block" : "Stream");
            outs << "\n";

            if(transfer_type == ON_BLK_TRANSFER)
            {
                outs << "Transfer Size : " << obj.bs_transfer_request.transfer_size << " -- ";
                outs << "Chunk Size : " << (int) obj.bs_transfer_request.chunk_size << " -- ";
            }

            outs << "Frag Delay : " << obj.bs_transfer_request.frag_delay_ms << " ms -- ";
            outs << "Chunk Pause : " << obj.bs_transfer_request.chunk_pause_ms << " ms -- ";
            outs << "Channel : " << (int) obj.bs_transfer_request.channel << " -- ";
            outs << "Data Rate : " << (int) data_rate
                 << "(" << data_rate_to_string(data_rate) << ") -- ";
            outs << "Timeout : " << obj.bs_transfer_request.timeout_ms << " ms -- ";
            outs << "Dest. DID -- " << detailed_did_display(obj.bs_transfer_request.enc_dst_did,
              obj.bs_transfer_request.raw_dst_did);
            outs << " -- Estimated Transfer Time : " << obj.bs_transfer_request.estimated_time_ms
                 << " ms";
            break;
        }

        case ON_REQUEST_REPEATER:
        {
            UInt8 priority = obj.reserve_repeater.priority;
            UInt8 data_rate = obj.reserve_repeater.data_rate;

            outs << "Repeater DID : " << detailed_did_display(obj.reserve_repeater.enc_rptr_did,
              obj.reserve_repeater.raw_rptr_did);
            outs << " -- Source DID : " << detailed_did_display(obj.reserve_repeater.enc_src_did,
              obj.reserve_repeater.raw_src_did);
            outs << " -- Destination DID : " << detailed_did_display(obj.reserve_repeater.enc_dst_did,
              obj.reserve_repeater.raw_dst_did);
            outs << " -- Channel : " << dec << (int) obj.reserve_repeater.channel;
            outs << "Data Rate : " << (int) data_rate
                 << "(" << data_rate_to_string(data_rate) << ")";
            outs << " -- Priority : " << dec << (int) priority
                 << "(" << priority_to_string(priority) << ")";
            outs<< "\n";
            break;
        }

        case ON_TERMINATE_BLOCK_STREAM:
            outs << "Terminating DID : " << detailed_did_display(obj.terminate_block_stream.enc_terminating_did,
              obj.terminate_block_stream.raw_terminating_did);
            outs << "Status : " << obj.terminate_block_stream.status;
            // TODO -- display ACK / NACK
            break;
        default:
            break;
    }
}


void on_admin_payload::default_display(const on_admin_payload& obj, UInt8 verbosity,
  const attribute* att, ostream& outs)
{
    if(att == NULL)
    {
        attribute attr;
        on_admin_payload::default_display(obj, verbosity, &attr, outs);
        return;
    }
    on_payload::default_display(obj, verbosity, att, outs);
    on_single_data_payload::default_display(obj, verbosity, att, outs);
    on_admin_payload::admin_payload_details_to_stream(obj, verbosity, att, outs);
}


void on_admin_payload::display(UInt8 verbosity, const attribute* att,
  ostream& outs) const
{
    if(att == NULL)
    {
        attribute attr;
        on_admin_payload::display(verbosity, &attr, outs);
        return;
    }
    if(on_admin_payload::disp_admin_pay == NULL)
    {
        return;
    }

    (*disp_admin_pay)(*this, verbosity, att, outs);
}


void on_admin_payload::set_display_on_admin_pay_function(display_on_admin_pay_func func)
{
    if(func == NULL)
    {
        return;
    }
    on_admin_payload::disp_admin_pay = func;
}
display_on_admin_pay_func on_admin_payload::disp_admin_pay = &on_admin_payload::default_display;





on_features_payload::on_features_payload(): on_single_data_payload()
{
}


on_features_payload::on_features_payload(UInt16 raw_pid, std::string payload, std::string key,
  bool encrypted): on_single_data_payload(raw_pid, payload, key, encrypted)
{
    if(!valid_crc)
    {
        return;
    }

    memcpy(&this->features, &this->decrypted_payload_bytes[ON_PLD_DATA_IDX],
      sizeof(on_features_t));
}


on_features_payload::on_features_payload(const on_features_payload& pld): on_single_data_payload(pld)
{
    features = pld.features;
}


on_features_payload::~on_features_payload()
{

}


void on_features_payload::default_display(const on_features_payload& obj, UInt8 verbosity,
  const attribute* att, ostream& outs)
{
    if(att == NULL)
    {
        attribute attr;
        on_features_payload::default_display(obj, verbosity, &attr, outs);
        return;
    }

    on_payload::default_display(obj, verbosity, att, outs);
    on_single_data_payload::default_display(obj, verbosity, att, outs);

    if(verbosity <= 10 || !att->get_attribute(attribute::ATTRIBUTE_PAYLOAD_DETAIL))
    {
        outs << "Features Payload Bytes : 0x" << bytes_to_hex_string(
            (const UInt8*) &obj.features, sizeof(obj.features)) << "\n";
    }
    else
    {
        outs << "Features...\n";
        outs << on_payload::detailed_features_to_string(obj.features, verbosity);
    }
}


void on_features_payload::display(UInt8 verbosity, const attribute* att,
  ostream& outs) const
{
    if(att == NULL)
    {
        attribute attr;
        on_features_payload::display(verbosity, &attr, outs);
        return;
    }

    if(on_features_payload::disp_features_pay == NULL)
    {
        return;
    }

    (*disp_features_pay)(*this, verbosity, att, outs);
}


void on_features_payload::set_display_on_features_pay_function(display_on_features_pay_func func)
{
    if(func == NULL)
    {
        return;
    }
    on_features_payload::disp_features_pay = func;
}
display_on_features_pay_func on_features_payload::disp_features_pay = &on_features_payload::default_display;







on_invite_payload::on_invite_payload(): on_payload()
{
}


on_invite_payload::on_invite_payload(UInt16 raw_pid, std::string payload, std::string key,
  bool encrypted): on_payload(raw_pid, payload, key, encrypted)
{
    this->version = this->decrypted_payload_bytes[ON_INVITE_VERSION_IDX];
    this->raw_did = (this->decrypted_payload_bytes[ON_INVITE_ASSIGNED_DID_IDX] << 4)
      + (this->decrypted_payload_bytes[ON_INVITE_ASSIGNED_DID_IDX+1] >> 4);
    memcpy(this->network_key_bytes, &this->decrypted_payload_bytes[ON_INVITE_KEY_IDX],
      ONE_NET_XTEA_KEY_LEN);
    memcpy((void*) &this->features_bytes, &this->decrypted_payload_bytes[ON_INVITE_FEATURES_IDX],
      sizeof(this->features_bytes));
    network_key = bytes_to_hex_string(network_key_bytes, ONE_NET_XTEA_KEY_LEN);
    {
        error_message = "Internal Error.";
    }
    this->features = bytes_to_hex_string((const UInt8*) &features_bytes,
      sizeof(this->features_bytes));
    {
        error_message = "Internal Error.";
    }
    valid = true;
}


on_invite_payload::on_invite_payload(const on_invite_payload& pld): on_payload(pld)
{
    version = pld.version;
    raw_did = pld.raw_did;
    network_key = pld.network_key;
    memcpy(network_key_bytes, pld.network_key_bytes, sizeof(network_key_bytes));
    features = pld.features;
    features_bytes = pld.features_bytes;
}


on_invite_payload::~on_invite_payload()
{

}


void on_invite_payload::default_display(const on_invite_payload& obj, UInt8 verbosity,
  const attribute* att, ostream& outs)
{
    if(!att)
    {
        attribute all;
        on_invite_payload::default_display(obj, verbosity, &all, outs);
        return;
    }

    // verbosity not used, but use it anyway to avoid compiler warning.
    verbosity++;
    verbosity--;

    on_payload::default_display(obj, verbosity, att, outs);

    bool need_comma = false;
    std::string str;

    if(att->get_attribute(attribute::ATTRIBUTE_INVITE_VERSION))
    {
        outs << (need_comma ? ", " : "");
        need_comma = true;

        outs << attribute::attribute_to_string(attribute::ATTRIBUTE_INVITE_VERSION,true) <<
          ": " << (int) obj.version;
        need_comma = true;
    }
    if(att->get_attribute(attribute::ATTRIBUTE_INVITE_DID))
    {
        outs << (need_comma ? ", " : "");
        need_comma = true;
        raw_did_to_string(obj.raw_did, str);
        outs << attribute::attribute_to_string(attribute::ATTRIBUTE_INVITE_DID, true) <<
          ": " << str;
        need_comma = true;
    }
    if(att->get_attribute(attribute::ATTRIBUTE_INVITE_KEY))
    {
        outs << (need_comma ? ", " : "");
        need_comma = true;
        outs << attribute::attribute_to_string(attribute::ATTRIBUTE_INVITE_KEY, true) <<
          ": " << format_key_string(obj.network_key_bytes);
        need_comma = true;
    }
    if(need_comma)
    {
        outs << "\n";
        need_comma = false;
    }
    if(att->get_attribute(attribute::ATTRIBUTE_INVITE_FEATURES))
    {
        outs << on_payload::detailed_features_to_string(obj.features_bytes, verbosity);
    }
}


void on_invite_payload::display(UInt8 verbosity, const attribute* att,
  ostream& outs) const
{
    if(att == NULL)
    {
        attribute attr;
        on_invite_payload::display(verbosity, &attr, outs);
        return;
    }

    if(on_invite_payload::disp_invite_pay == NULL)
    {
        return;
    }

    (*disp_invite_pay)(*this, verbosity, att, outs);
}


void on_invite_payload::set_display_on_invite_pay_function(display_on_invite_pay_func func)
{
    if(func == NULL)
    {
        return;
    }
    on_invite_payload::disp_invite_pay = func;
}
display_on_invite_pay_func on_invite_payload::disp_invite_pay = &on_invite_payload::default_display;






on_route_payload::on_route_payload(): on_single_data_payload()
{
}


on_route_payload::on_route_payload(UInt16 raw_pid, std::string payload, std::string key,
  bool encrypted): on_single_data_payload(raw_pid, payload, key, encrypted)
{
    memcpy(this->route_bytes, &this->decrypted_payload_bytes[ON_PLD_DATA_IDX],
      sizeof(this->route_bytes));
    valid = valid_crc;
}


on_route_payload::on_route_payload(const on_route_payload& pld): on_single_data_payload(pld)
{
    memcpy(route_bytes, pld.route_bytes, sizeof(route_bytes));
}


on_route_payload::~on_route_payload()
{

}


std::string on_route_payload::route_payload_info_to_string(const UInt8* route_bytes,
    UInt8 verbosity)
{
    if(route_bytes == NULL)
    {
        return "";
    }

    std::string tmp;
    std::string str = "Route Bytes: " + bytes_to_hex_string(route_bytes, 21)
        +"\n";
    if(verbosity <= 10)
    {
        return str;
    }

    str += "Route DIDs: ";
    for(unsigned int i = 0; i < 21; i += 3)
    {
        if(route_bytes[i] == 0 && route_bytes[i+1] == 0)
        {
            break;
        }
        UInt16 raw_did = (((UInt16)route_bytes[i]) << 4) +
          (((UInt16)route_bytes[i+1]) >> 4);
        if(i > 0)
        {
            str += "-->";
        }
        raw_did_to_string(raw_did, tmp);
        str += tmp;
        if((route_bytes[i+1] & 0x0F) == 0 && route_bytes[i+2] == 0)
        {
            break;
        }
        raw_did = (((UInt16)(route_bytes[i+1] & 0x0F)) << 8) + route_bytes[i+2];
        str += "-->";
        raw_did_to_string(raw_did, tmp);
        str += tmp;
    }
    str += "\n";
    return str;
}


void on_route_payload::default_display(const on_route_payload& obj, UInt8 verbosity,
  const attribute* att, ostream& outs)
{
    if(att == NULL)
    {
        attribute attr;
        on_route_payload::default_display(obj, verbosity, &attr, outs);
        return;
    }

    on_payload::default_display(obj, verbosity, att, outs);
    on_single_data_payload::default_display(obj, verbosity, att, outs);
    outs << on_route_payload::route_payload_info_to_string(obj.route_bytes,
      verbosity);
}


void on_route_payload::display(UInt8 verbosity, const attribute* att,
  ostream& outs) const
{
    if(att == NULL)
    {
        attribute attr;
        on_route_payload::display(verbosity, &attr, outs);
        return;
    }

    if(on_route_payload::disp_route_pay == NULL)
    {
        return;
    }

    (*disp_route_pay)(*this, verbosity, att, outs);
}


void on_route_payload::set_display_on_route_pay_function(display_on_route_pay_func func)
{
    if(func == NULL)
    {
        return;
    }
    on_route_payload::disp_route_pay = func;
}
display_on_route_pay_func on_route_payload::disp_route_pay = &on_route_payload::default_display;







on_block_payload::on_block_payload(): on_payload()
{
    block_pkt.data = data;
}


on_block_payload::on_block_payload(UInt16 raw_pid, std::string payload, std::string key,
  bool encrypted): on_payload(raw_pid, payload, key, encrypted)
{
    block_pkt.data = data;
    if(valid_crc)
    {
        this->block_pkt.chunk_idx = get_bs_chunk_idx(decrypted_payload_bytes);
        this->block_pkt.chunk_size = get_bs_chunk_size(decrypted_payload_bytes);
        this->block_pkt.byte_idx = get_block_byte_idx(decrypted_payload_bytes);
        memcpy(block_pkt.data, &decrypted_payload_bytes[ON_BS_DATA_PLD_IDX],
          sizeof(data));
    }
    valid = valid_crc;
}


on_block_payload::on_block_payload(const on_block_payload& pld): on_payload(pld)
{
    block_pkt = pld.block_pkt;
    block_pkt.data = data;
    memcpy(block_pkt.data, pld.block_pkt.data, sizeof(data));
}


on_block_payload::~on_block_payload()
{

}


void on_block_payload::default_display(const on_block_payload& obj, UInt8 verbosity,
  const attribute* att, ostream& outs)
{
    if(!att)
    {
        attribute all;
        on_block_payload::default_display(obj, verbosity, &all, outs);
        return;
    }

    on_payload::default_display(obj, verbosity, att, outs);

    if(!att->get_attribute(attribute::ATTRIBUTE_PAYLOAD_DETAIL))
    {
        return;
    }

    outs << "Chunk Index: " << dec << (int) obj.block_pkt.chunk_idx;
    outs << " -- Chunk Size: " << dec << (int) obj.block_pkt.chunk_size;
    outs << " -- Byte Index: " << dec << obj.block_pkt.byte_idx;
    if(verbosity > 10)
    {
        outs << " -- Data : " << bytes_to_hex_string(obj.block_pkt.data,
            sizeof(obj.data));
    }
    outs << "\n";
}


void on_block_payload::display(UInt8 verbosity, const attribute* att,
  ostream& outs) const
{
    if(att == NULL)
    {
        attribute attr;
        on_block_payload::display(verbosity, &attr, outs);
        return;
    }

    if(on_block_payload::disp_block_pay == NULL)
    {
        return;
    }

    (*disp_block_pay)(*this, verbosity, att, outs);
}


void on_block_payload::set_display_on_block_pay_function(display_on_block_pay_func func)
{
    if(func == NULL)
    {
        return;
    }
    on_block_payload::disp_block_pay = func;
}
display_on_block_pay_func on_block_payload::disp_block_pay = &on_block_payload::default_display;







on_stream_payload::on_stream_payload(): on_payload()
{
    stream_pkt.data = data;
}


on_stream_payload::on_stream_payload(UInt16 raw_pid, std::string payload, std::string key,
  bool encrypted): on_payload(raw_pid, payload, key, encrypted)
{
    stream_pkt.data = data;
    if(valid_crc)
    {
        on_parse_stream_pld(decrypted_payload_bytes, &stream_pkt);
    }
    valid = valid_crc;
}


on_stream_payload::on_stream_payload(const on_stream_payload& pld): on_payload(pld)
{
    stream_pkt = pld.stream_pkt;
    stream_pkt.data = data;
    memcpy(stream_pkt.data, pld.stream_pkt.data, sizeof(data));
}


on_stream_payload::~on_stream_payload()
{

}


void on_stream_payload::default_display(const on_stream_payload& obj, UInt8 verbosity,
  const attribute* att, ostream& outs)
{
    if(!att)
    {
        attribute all;
        on_stream_payload::default_display(obj, verbosity, &all, outs);
        return;
    }

    on_payload::default_display(obj, verbosity, att, outs);

    if(!att->get_attribute(attribute::ATTRIBUTE_PAYLOAD_DETAIL))
    {
        return;
    }

    outs << "Elapsed Time: " << dec << (int) obj.stream_pkt.elapsed_time << " ms";
    outs << " -- Response Needed: " << (obj.stream_pkt.response_needed ? "true" : "false");
    if(verbosity > 10)
    {
        outs << " -- Data : " << bytes_to_hex_string(obj.stream_pkt.data,
            sizeof(obj.data));
    }
    outs << "\n";
}


void on_stream_payload::display(UInt8 verbosity, const attribute* att,
  ostream& outs) const
{
    if(att == NULL)
    {
        attribute attr;
        on_stream_payload::display(verbosity, &attr, outs);
        return;
    }

    if(on_stream_payload::disp_stream_pay == NULL)
    {
        return;
    }

    (*disp_stream_pay)(*this, verbosity, att, outs);
}


void on_stream_payload::set_display_on_stream_pay_function(display_on_stream_pay_func func)
{
    if(func == NULL)
    {
        return;
    }
    on_stream_payload::disp_stream_pay = func;
}
display_on_stream_pay_func on_stream_payload::disp_stream_pay = &on_stream_payload::default_display;






on_packet::on_packet()
{

}


on_packet::on_packet(std::string encoded_packet, std::string key)
{
    this->encoded_packet = encoded_packet;
    strip_all_whitespace(this->encoded_packet);
    this->valid = false;
    this->valid_decode = false;
    this->valid_digits = false;
    this->valid_msg_crc = false;
    this->valid_pid = false;
    this->payload = NULL;
    this->timestamp_ms = 0;

    
    if(this->encoded_packet.length() < ON_MIN_ENCODED_PKT_SIZE * 2)
    {
        error_message = "Packet is too short.";
        return;
    }
    else if(this->encoded_packet.length() %2 == 1)
    {
        error_message = "Packet has an odd number of nibbles.";
        return;
    }
    else
    {
        unsigned int len = this->encoded_packet.length();
        this->num_encoded_bytes = len / 2;

        for(unsigned int i = 0; i < len; i++)
        {
            if(!isxdigit(this->encoded_packet[i]))
            {
                error_message = "Packet has at least one invalid hexadecimal digit";
                return;
            }
        }

        this->valid_digits = true;
    }

    string tmp;

    this->preamble_header = this->encoded_packet.substr(0, ONE_NET_PREAMBLE_HEADER_LEN * 2);
    if(preamble_header.compare("55555533") != 0)
    {
        this->error_message = "Invalid preamble/header:Should be 55555533";
        return;
    }

    tmp = this->encoded_packet.substr(ON_ENCODED_RPTR_DID_IDX * 2, ON_ENCODED_DID_LEN * 2);
    string_to_uint16(tmp, enc_rptr_did, true);
    tmp = this->encoded_packet.substr(ON_ENCODED_DST_DID_IDX * 2, ON_ENCODED_DID_LEN * 2);
    string_to_uint16(tmp, enc_dst_did, true);
    tmp = this->encoded_packet.substr(ON_ENCODED_SRC_DID_IDX * 2, ON_ENCODED_DID_LEN * 2);
    string_to_uint16(tmp, enc_src_did, true);
    tmp = this->encoded_packet.substr(ON_ENCODED_NID_IDX * 2, ON_ENCODED_NID_LEN * 2);
    string_to_uint64_t(tmp, enc_nid, true);
    tmp = this->encoded_packet.substr(ON_ENCODED_PID_IDX * 2, ON_ENCODED_PID_SIZE * 2);
    string_to_uint16(tmp, enc_pid, true);

    if(on_decode_uint16(&this->raw_rptr_did, this->enc_rptr_did) != ONS_SUCCESS)
    {
        error_message = "Rptr. DID did not decode properly";
        return;
    }
    if(on_decode_uint16(&this->raw_dst_did, this->enc_dst_did) != ONS_SUCCESS)
    {
        error_message = "Dest. DID did not decode properly";
        return;
    }
    if(on_decode_uint16(&this->raw_src_did, this->enc_src_did) != ONS_SUCCESS)
    {
        error_message = "Src. DID did not decode properly";
        return;
    }
    if(on_packet::on_decode_nid(&this->raw_nid, this->enc_nid) != ONS_SUCCESS)
    {
        error_message = "NID did not decode properly";
        return;
    }
    if(on_decode_uint16(&this->raw_pid, this->enc_pid) != ONS_SUCCESS)
    {
        error_message = "PID did not decode properly";
        return;
    }


    num_payload_blocks = get_num_payload_blocks(raw_pid);
    if(num_payload_blocks < 1 || num_payload_blocks > 4)
    {
        error_message = "Invalid number of payload blocks";
        return;
    }

    if(this->num_encoded_bytes != get_encoded_packet_len(raw_pid, TRUE))
    {
        error_message = "Number of bytes in packet does not match number of bytes specified in raw pid";
        return;
    }

    is_invite_pkt = packet_is_invite(raw_pid);
    is_ack_pkt = packet_is_ack(raw_pid);
    is_nack_pkt = packet_is_nack(raw_pid);
    is_response_pkt = is_ack_pkt || is_nack_pkt;
    is_single_data_pkt = (packet_is_single(raw_pid) && packet_is_data(raw_pid));
    is_app_pkt = false; // filled in later
    is_admin_pkt = false; // filled in later
    is_features_pkt =  false; // filled in later
    is_route_pkt =  false; // filled in later
    is_route_pkt = packet_is_route(raw_pid);
    is_block_pkt = packet_is_block(raw_pid);
    is_stream_pkt = packet_is_stream(raw_pid);
    is_multihop_pkt = packet_is_multihop(raw_pid);
    is_stay_awake_pkt = packet_is_stay_awake(raw_pid);


    on_pkt_t pkt;


    if(!hex_string_to_bytes(this->encoded_packet, encoded_packet_bytes,
      this->num_encoded_bytes))
    {
        error_message = "internal error.";
        return;
    }


    if(!setup_pkt_ptr(this->raw_pid, encoded_packet_bytes, 0, &pkt))
    {
        error_message = "Internal Error";
        return;
    }
    encoded_payload_len = pkt.payload_len;
    decoded_payload_len = get_raw_payload_len(raw_pid);
    this->calculated_msg_crc = calculate_msg_crc(&pkt);

    this->enc_msg_crc = encoded_packet_bytes[ON_ENCODED_MSG_CRC_IDX];
    this->msg_crc = encoded_to_decoded_byte(this->enc_msg_crc, TRUE);
    if(this->msg_crc == 0xFF)
    {
        error_message = "Could not decode message CRC.";
        return;
    }
    this->valid_msg_crc = (this->msg_crc == this->calculated_msg_crc);

    if(this->is_multihop_pkt)
    {
        if(on_parse_hops(&pkt, &hops, &max_hops) != ONS_SUCCESS)
        {
            error_message = "Could not decode hops field.";
            return;
        }
        encoded_hops_field = pkt.packet_bytes[ON_ENCODED_PLD_IDX];
        if(on_decode(&raw_hops_field,
          &(pkt.packet_bytes[ON_ENCODED_PLD_IDX]) + pkt.payload_len,
          ON_ENCODED_HOPS_SIZE) != ONS_SUCCESS)
        {
            error_message = "Could not decode hops field.";
            return;
        }
    }
    encoded_payload = this->encoded_packet.substr(2 * ON_ENCODED_PLD_IDX,
      2 * pkt.payload_len);
    encoded_payload_bytes = &encoded_packet_bytes[ON_ENCODED_PLD_IDX];

    if(on_decode(decoded_payload_bytes, encoded_payload_bytes,
      encoded_payload_len) != ONS_SUCCESS)
    {
        error_message = "Could not decode encoded payload.";
        return;
    }
    valid_decode = true;

    decoded_payload = bytes_to_hex_string(decoded_payload_bytes, decoded_payload_len);

    if((UInt16)(raw_pid & 0x3F) < NUM_PIDS)
    {
        valid_pid = true;
    }
    else
    {
        error_message = "Raw PID is not valid.";
        return;
    }


    if(this->is_single_data_pkt)
    {
        payload = new on_single_data_payload(raw_pid, decoded_payload, key, true);
        on_single_data_payload* osdp = reinterpret_cast<on_single_data_payload*>(payload);
        if(osdp->get_is_app_pkt())
        {
            this->is_app_pkt = true;
            delete payload;
            payload = new on_app_payload(raw_pid, decoded_payload, key, true);
        }
        else if(osdp->get_is_admin_pkt())
        {
            this->is_admin_pkt = true;
            delete payload;
            payload = new on_admin_payload(raw_pid, decoded_payload, key, true);
        }
        else if(osdp->get_is_features_pkt())
        {
            this->is_features_pkt = true;
            delete payload;
            payload = new on_features_payload(raw_pid, decoded_payload, key, true);
        }
        else if(osdp->get_is_route_pkt())
        {
            this->is_route_pkt = true;
            delete payload;
            payload = new on_route_payload(raw_pid, decoded_payload, key, true);
        }
    }
    else if(this->is_invite_pkt)
    {
        payload = new on_invite_payload(raw_pid, decoded_payload, key, true);
    }
    else if(this->is_response_pkt)
    {
        payload = new on_response_payload(raw_pid, decoded_payload, key, true);
    }
    else if(this->is_block_pkt)
    {
        payload = new on_block_payload(raw_pid, decoded_payload, key, true);
    }
    else if(this->is_stream_pkt)
    {
        payload = new on_stream_payload(raw_pid, decoded_payload, key, true);
    }

    if(!payload->get_valid())
    {
        error_message = payload->get_error_message();
    }
    else
    {
        valid = true;
    }
}


on_packet::on_packet(const on_packet& orig)
{
    timestamp_ms = orig.timestamp_ms;
    encoded_packet = orig.encoded_packet;
    encoded_payload = orig.encoded_payload;
    decoded_payload = orig.decoded_payload;
    memcpy(encoded_packet_bytes, orig.encoded_packet_bytes,
      sizeof(encoded_packet_bytes));
    memcpy(encoded_payload_bytes, orig.encoded_payload_bytes,
      sizeof(encoded_payload_bytes));
    memcpy(decoded_payload_bytes, orig.decoded_payload_bytes,
      sizeof(decoded_payload_bytes));
    preamble_header = orig.preamble_header;
    num_encoded_bytes = orig.num_encoded_bytes;
    encoded_payload_len = orig.encoded_payload_len;
    decoded_payload_len = orig.decoded_payload_len;
    raw_src_did = orig.raw_src_did;
    raw_rptr_did = orig.raw_rptr_did;
    raw_dst_did = orig.raw_dst_did;
    raw_nid = orig.raw_nid;
    enc_src_did = orig.enc_src_did;
    enc_rptr_did = orig.enc_rptr_did;
    enc_dst_did = orig.enc_dst_did;
    enc_nid = orig.enc_nid;
    enc_pid = orig.enc_pid;
    raw_pid = orig.raw_pid;
    enc_msg_crc = orig.enc_msg_crc;
    msg_crc = orig.msg_crc;
    calculated_msg_crc = orig.calculated_msg_crc;
    hops = orig.hops;
    max_hops = orig.max_hops;
    encoded_hops_field = orig.encoded_hops_field;
    raw_hops_field = orig.raw_hops_field;
    valid_digits = orig.valid_digits;
    valid_msg_crc = orig.valid_msg_crc;
    valid_decode = orig.valid_decode;
    valid_pid = orig.valid_pid;
    valid = orig.valid;
    is_invite_pkt = orig.is_invite_pkt;
    is_single_data_pkt = orig.is_single_data_pkt;
    is_response_pkt = orig.is_response_pkt;
    is_ack_pkt = orig.is_ack_pkt;
    is_nack_pkt = orig.is_nack_pkt;
    is_route_pkt = orig.is_route_pkt;
    is_block_pkt = orig.is_block_pkt;
    is_stream_pkt = orig.is_stream_pkt;
    is_app_pkt = orig.is_app_pkt;
    is_admin_pkt = orig.is_admin_pkt;
    is_features_pkt = orig.is_features_pkt;
    is_multihop_pkt = orig.is_multihop_pkt;
    is_stay_awake_pkt = orig.is_stay_awake_pkt;
    num_payload_blocks = orig.num_payload_blocks;
    error_message = orig.error_message;

    if(payload == NULL)
    {
        // should never get here?
        return;
    }

    if(this->is_single_data_pkt)
    {
        if(is_app_pkt)
        {
            on_app_payload* oap = dynamic_cast<on_app_payload*>(orig.payload);
            payload = new on_app_payload(*oap);
        }
        else if(is_admin_pkt)
        {
            on_admin_payload* oap = dynamic_cast<on_admin_payload*>(orig.payload);
            payload = new on_admin_payload(*oap);
        }
        else if(is_features_pkt)
        {
            on_features_payload* ofp = dynamic_cast<on_features_payload*>(orig.payload);
            payload = new on_features_payload(*ofp);
        }
        else if(is_route_pkt)
        {
            on_route_payload* orp = dynamic_cast<on_route_payload*>(orig.payload);
            payload = new on_route_payload(*orp);
        }
        else
        {
            on_single_data_payload* osdp = dynamic_cast<on_single_data_payload*>(orig.payload);
            payload = new on_single_data_payload(*osdp);
        }
    }
    else if(this->is_invite_pkt)
    {
        on_invite_payload* oip = dynamic_cast<on_invite_payload*>(orig.payload);
        payload = new on_invite_payload(*oip);
    }
    else if(this->is_response_pkt)
    {
        on_response_payload* orp = dynamic_cast<on_response_payload*>(orig.payload);
        payload = new on_response_payload(*orp);
    }
    else if(this->is_block_pkt)
    {
        on_block_payload* obp = dynamic_cast<on_block_payload*>(orig.payload);
        payload = new on_block_payload(*obp);
    }
    else if(this->is_stream_pkt)
    {
        on_stream_payload* osp = dynamic_cast<on_stream_payload*>(orig.payload);
        payload = new on_stream_payload(*osp);
    }
}


on_packet::~on_packet()
{
    delete payload;
}


void on_packet::display(UInt8 verbosity, const attribute* att, ostream& outs)
{
    if(att == NULL)
    {
        attribute attr;
        on_packet::display(verbosity, &attr, outs);
        return;
    }
    if(on_packet::disp_pkt == NULL)
    {
        return;
    }

    (*disp_pkt)(*this, verbosity, att, outs);
}


void on_packet::default_display(const on_packet& obj, UInt8 verbosity,
  const attribute* att, ostream& outs)
{
    if(!att)
    {
        attribute attr;
        on_packet::default_display(obj, verbosity, &attr, outs);
        return;
    }

    bool need_comma = false;
    std::string str;
    bool ret;

    if(att->get_attribute(attribute::ATTRIBUTE_TIMESTAMP))
    {
        if(need_comma)
        {
            outs << ", ";
        }
        outs << attribute::attribute_to_string(attribute::ATTRIBUTE_TIMESTAMP, true) <<
          ": " << dec << obj.timestamp_ms << " ms\n";
    }
    if(att->get_attribute(attribute::ATTRIBUTE_VALID_PKT))
    {
        outs << "Valid Digits: " << (obj.valid_digits ? "True" : "False");
        outs << ", Valid Msg CRC: " << (obj.valid_msg_crc ? "True" : "False");
        outs << ", Valid Decoding: " << (obj.valid_decode ? "True" : "False");
        outs << ", Valid PID: " << (obj.valid_pid ? "True" : "False");
        outs << ", Valid Payload Decrypt: " << ((obj.payload && obj.payload->get_valid_decrypt()) ? "True" : "False");
        outs << ", Valid Payload CRC: " << ((obj.payload && obj.payload->get_valid_crc()) ? "True" : "False");
        outs << ", Valid: " << (obj.valid ? "True" : "False") << "\n";
        if(verbosity > 10 && !obj.valid)
        {
            outs << "Error Msg: " << obj.error_message << "\n";
        }
    }
    if(att->get_attribute(attribute::ATTRIBUTE_ENCODED_BYTES))
    {
        if(need_comma)
        {
            outs << ", ";
        }

        outs << "Number of Encoded Bytes: " << dec << (int) obj.num_encoded_bytes << ", " <<
        attribute::attribute_to_string(attribute::ATTRIBUTE_ENCODED_BYTES, true) <<
          ":\n";

        str = bytes_to_hex_string(obj.encoded_packet_bytes, obj.num_encoded_bytes,
          ' ', 1, 24);
        outs << str << "\n";
    }
    if(att->get_attribute(attribute::ATTRIBUTE_HEADER))
    {
        if(need_comma)
        {
            outs << ", ";
        }

        outs << attribute::attribute_to_string(attribute::ATTRIBUTE_HEADER, true) <<
          ": " << "0x" << obj.preamble_header;
        need_comma = true;
    }
    if(att->get_attribute(attribute::ATTRIBUTE_RPTR_DID))
    {
        if(need_comma)
        {
            outs << ", ";
        }

        outs << attribute::attribute_to_string(attribute::ATTRIBUTE_RPTR_DID, true) <<
          ": ";
        if(verbosity > 10)
        {
            ret = encoded_did_to_string(obj.enc_rptr_did, str);
            outs << "Encoded (" << str;
            if(!ret)
            {
                outs << " -- Invalid";
            }
            outs << ") ";
        }
        ret = raw_did_to_string(obj.raw_rptr_did, str);
        outs << "Raw (" << str;
        if(!ret)
        {
            outs << " -- Invalid";
        }
        outs << ") ";

        need_comma = true;
    }
    if(att->get_attribute(attribute::ATTRIBUTE_MSG_CRC))
    {
        if(need_comma)
        {
            outs << ", ";
        }

        outs << attribute::attribute_to_string(attribute::ATTRIBUTE_MSG_CRC, true) <<
          ": ";
        if(verbosity > 10)
        {
            outs << "Encoded (0x" << byte_to_hex_string(obj.enc_msg_crc);
            outs << ") ";
        }
        outs << "Raw,Shifted (0x" << byte_to_hex_string(obj.msg_crc);
        outs << ") ";
        outs << "Calculated (0x" << byte_to_hex_string(obj.calculated_msg_crc);
        outs << ") ";
        outs << "CRCs " << ((obj.msg_crc == obj.calculated_msg_crc) ? "" : "do not ")
             << "match.";
    }
    if(need_comma)
    {
        outs << "\n";
    }
    need_comma = false;
    if(att->get_attribute(attribute::ATTRIBUTE_DST_DID))
    {
        if(need_comma)
        {
            outs << ", ";
        }

        outs << attribute::attribute_to_string(attribute::ATTRIBUTE_DST_DID, true) <<
          ": ";
        if(verbosity > 10)
        {
            ret = encoded_did_to_string(obj.enc_dst_did, str);
            outs << "Encoded (" << str;
            if(!ret)
            {
                outs << " -- Invalid";
            }
            outs << ") ";
        }
        ret = raw_did_to_string(obj.raw_dst_did, str);
        outs << "Raw (" << str;
        if(!ret)
        {
            outs << " -- Invalid";
        }
        outs << ") ";

        need_comma = true;
    }
    if(att->get_attribute(attribute::ATTRIBUTE_NID))
    {
        if(need_comma)
        {
            outs << ", ";
        }

        outs << attribute::attribute_to_string(attribute::ATTRIBUTE_NID, true) <<
          ": ";
        if(verbosity > 10)
        {
            ret = encoded_nid_to_string(obj.enc_nid, str);
            outs << "Encoded (" << str;
            if(!ret)
            {
                outs << " -- Invalid";
            }
            outs << ") ";
        }
        ret = raw_nid_to_string(obj.raw_nid, str);
        outs << "Raw (" << str;
        if(!ret)
        {
            outs << " -- Invalid";
        }
        outs << ") ";

        need_comma = true;
    }
    if(att->get_attribute(attribute::ATTRIBUTE_SRC_DID))
    {
        if(need_comma)
        {
            outs << ", ";
        }

        outs << attribute::attribute_to_string(attribute::ATTRIBUTE_SRC_DID, true) <<
          ": ";
        if(verbosity > 10)
        {
            ret = encoded_did_to_string(obj.enc_src_did, str);
            outs << "Encoded (" << str;
            if(!ret)
            {
                outs << " -- Invalid";
            }
            outs << ") ";
        }
        ret = raw_did_to_string(obj.raw_src_did, str);
        outs << "Raw (" << str;
        if(!ret)
        {
            outs << " -- Invalid";
        }
        outs << ") ";
    }
    if(need_comma)
    {
        outs << "\n";
        need_comma = false;
    }
    if(att->get_attribute(attribute::ATTRIBUTE_PID))
    {
        if(need_comma)
        {
            outs << ", ";
        }

        outs << attribute::attribute_to_string(attribute::ATTRIBUTE_PID, true) <<
          ": ";
        if(verbosity > 10)
        {
            outs << "Encoded (0x" << uint16_to_hex_string(obj.enc_pid);
            if(!obj.valid_pid)
            {
                outs << " -- Invalid";
            }
            outs << ") ";
        }
        outs << "Raw (0x" << uint16_to_hex_string(obj.raw_pid);
        if(!obj.valid_pid)
        {
            outs << " -- Invalid";
        }
        outs << ") ";

        if(verbosity > 10 && obj.valid_pid)
        {

            outs << " : 12 Raw Bits : ";
            outs << value_to_bit_string(obj.raw_pid, 12) << ")\n";

            UInt16 num_blocks_part = ((obj.raw_pid & ONE_NET_RAW_PID_SIZE_MASK) >>
              ONE_NET_RAW_PID_SIZE_SHIFT);
            UInt16 mh_part = ((obj.raw_pid & ONE_NET_RAW_PID_MH_MASK) >>
              ONE_NET_RAW_PID_MH_SHIFT);
            UInt16 sa_part = ((obj.raw_pid & ONE_NET_RAW_PID_STAY_AWAKE_MASK) >>
              ONE_NET_RAW_PID_STAY_AWAKE_SHIFT);
            UInt16 packet_type_part = (obj.raw_pid &
              ONE_NET_RAW_PID_PACKET_TYPE_MASK);

            outs << "(Raw PID Bits 11 - 8(# XTEA blocks):" << value_to_bit_string(
              num_blocks_part, 4) << ": " << num_blocks_part << ")\n";
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
    if(att->get_attribute(attribute::ATTRIBUTE_HOPS))
    {
        outs << "Multi-Hop : " << (obj.is_multihop_pkt ? "true" : "false");
        if(obj.is_multihop_pkt)
        {
            outs << byte_to_hex_string(obj.encoded_hops_field);
            outs << " -- Encoded : " << str << " -- ";
            outs << byte_to_hex_string(obj.raw_hops_field);
            outs << " -- Decoded : " << str << " -- ";
            outs << "Hops : " << (int) obj.hops << " -- Max Hops : " << (int)obj.max_hops;
        }
        outs << "\n";
    }
    if(att->get_attribute(attribute::ATTRIBUTE_ENCODED_PAYLOAD))
    {
        if(need_comma)
        {
            outs << ", ";
        }

        outs << "Number of Encoded Payload Bytes: " << dec << (int) obj.encoded_payload_len << ", " <<
        attribute::attribute_to_string(attribute::ATTRIBUTE_ENCODED_PAYLOAD, true) <<
          ":\n";

        str = bytes_to_hex_string(obj.encoded_payload_bytes, obj.encoded_payload_len,
          ' ', 1, 24);
        outs << str << "\n";
    }

    if(obj.payload)
    {
        obj.payload->display(verbosity, att, outs);
    }
}


void on_packet::set_display_on_packet_function(display_on_packet_func func)
{
    if(func == NULL)
    {
        return;
    }
    on_packet::disp_pkt = func;
}


string on_packet::get_raw_pid_string(UInt16 raw_pid)
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


one_net_status_t on_packet::on_encode_uint32(UInt32* encoded, UInt32 decoded)
{
    if(!encoded || decoded > 0xFFFFFF)
    {
        return ONS_BAD_PARAM;
    }
    UInt16 low_decoded, high_decoded, low_encoded, high_encoded;
    low_decoded = decoded & 0xFFF;
    high_decoded = decoded >> 12;

    if(on_encode_uint16(&low_encoded, low_decoded) != ONS_SUCCESS)
    {
        return ONS_BAD_PARAM;
    }
    if(on_encode_uint16(&high_encoded, high_decoded) != ONS_SUCCESS)
    {
        return ONS_BAD_PARAM;
    }

    *encoded = high_encoded;
    *encoded <<= 16;
    *encoded += low_encoded;
    return ONS_SUCCESS;
}


one_net_status_t on_packet::on_decode_uint32(UInt32* decoded, UInt32 encoded)
{
    if(!decoded)
    {
        return ONS_BAD_PARAM;
    }

    UInt16 low_encoded, high_encoded, low_decoded, high_decoded;
    low_encoded = encoded & 0xFFFF;
    high_encoded = encoded>> 16;

    if(on_decode_uint16(&low_decoded, low_encoded) != ONS_SUCCESS)
    {
        return ONS_BAD_PARAM;
    }
    if(on_decode_uint16(&high_decoded, high_encoded) != ONS_SUCCESS)
    {
        return ONS_BAD_PARAM;
    }

    *decoded = (high_decoded << 12) + low_decoded;
    return ONS_SUCCESS;
}


one_net_status_t on_packet::on_encode_nid(uint64_t* encoded_nid, uint64_t decoded_nid)
{
    uint64_t max_decoded_nid = 0xFFFFFFFFFLL;
    if(!encoded_nid || decoded_nid > max_decoded_nid)
    {
        return ONS_BAD_PARAM;
    }

    UInt32 low_encoded, low_decoded;
    UInt16 high_encoded, high_decoded;
    low_decoded = decoded_nid & 0xFFFFFF;
    high_decoded = decoded_nid >> 24;

    if(on_encode_uint32(&low_encoded, low_decoded) != ONS_SUCCESS)
    {
        return ONS_BAD_PARAM;
    }
    if(on_encode_uint16(&high_encoded, high_decoded) != ONS_SUCCESS)
    {
        return ONS_BAD_PARAM;
    }

    *encoded_nid = high_encoded;
    *encoded_nid <<= 32;
    *encoded_nid += low_encoded;
    return ONS_SUCCESS;
}


one_net_status_t on_packet::on_decode_nid(uint64_t* decoded_nid, uint64_t encoded_nid)
{
    uint64_t max_encoded_nid = 0xFFFFFFFFFFFFLL;
    if(!decoded_nid || encoded_nid > max_encoded_nid)
    {
        return ONS_BAD_PARAM;
    }

    UInt32 low_encoded, low_decoded;
    UInt16 high_encoded, high_decoded;

    low_encoded = encoded_nid & 0xFFFFFFFF;
    high_encoded = encoded_nid >> 32;

    if(on_decode_uint32(&low_decoded, low_encoded) != ONS_SUCCESS)
    {
        return ONS_BAD_PARAM;
    }
    if(on_decode_uint16(&high_decoded, high_encoded) != ONS_SUCCESS)
    {
        return ONS_BAD_PARAM;
    }

    *decoded_nid = high_decoded;
    *decoded_nid <<= 24;
    *decoded_nid += low_decoded;
    return ONS_SUCCESS;
}
display_on_packet_func on_packet::disp_pkt = &on_packet::default_display;
