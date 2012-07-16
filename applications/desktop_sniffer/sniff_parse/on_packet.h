#ifndef ON_PACKET_H
#define	ON_PACKET_H

#include <string>
#include <stdint.h>
#include <iomanip>
#include "one_net_types.h"
#include "one_net_acknowledge.h"
#include "one_net_message.h"
#include "one_net_packet.h"
#include "one_net_features.h"
#include "one_net_peer.h"
#include "attribute.h"
#include "string_utils.h"


extern const unsigned int NUM_PIDS;
extern string_int_struct raw_pid_strings[];



class on_payload
{
public:
    on_payload();
    on_payload(UInt16 raw_pid, std::string payload, std::string key,
      bool encrypted);
    on_payload(const on_payload& pld);
    virtual ~on_payload();
    static void default_display(const on_payload& obj, UInt8 verbosity,
      const attribute* att, ostream& outs = cout);
    virtual void display(UInt8 verbosity, const attribute* att, ostream& outs) const = 0;
    static std::string detailed_data_rates_to_string(on_features_t features);
    static std::string detailed_features_to_string(on_features_t features, UInt8 verbosity);


    std::string get_error_message(){return error_message;}
    bool get_valid(){return valid;}
    bool get_valid_decrypt(){return valid_decrypt;}
    bool get_valid_crc(){return valid_crc;}
protected:
    SInt8 num_bytes;
    std::string encrypted_payload;
    std::string decrypted_payload;
    UInt8 encrypted_payload_bytes[ON_MAX_RAW_PLD_LEN_WITH_TECH];
    UInt8 decrypted_payload_bytes[ON_MAX_RAW_PLD_LEN_WITH_TECH];
    std::string key;
    one_net_xtea_key_t key_bytes;
    unsigned int num_rounds;
    UInt16 raw_pid;
    UInt8 calculated_payload_crc;
    UInt8 payload_crc;
    UInt16 msg_id;
    std::string error_message;
    bool valid_decrypt;
    bool valid_crc;
    bool valid;
};


class on_single_data_payload;
typedef void(*display_on_single_data_pay_func)(const on_single_data_payload&, UInt8,
  const attribute* att, ostream& outs);
class on_single_data_payload: public on_payload
{
public:
    on_single_data_payload();
    on_single_data_payload(UInt16 raw_pid, std::string payload, std::string key,
      bool encrypted);
    on_single_data_payload(const on_single_data_payload& pld);
    virtual ~on_single_data_payload();
    static void default_display(const on_single_data_payload& obj,
      UInt8 verbosity, const attribute* att, ostream& outs = cout);
    void display(UInt8 verbosity, const attribute* att, ostream& outs = cout) const;
    static void set_display_on_single_data_pay_function(display_on_single_data_pay_func func);

    UInt8 get_pld_msg_type(){return payload_msg_type;}
    bool get_is_app_pkt(){return is_app_pkt;}
    bool get_is_admin_pkt(){return is_admin_pkt;}
    bool get_is_features_pkt(){return is_features_pkt;}
    bool get_is_route_pkt(){return is_route_pkt;}
protected:
    UInt8 payload_msg_type;
    bool is_app_pkt;
    bool is_admin_pkt;
    bool is_features_pkt;
    bool is_route_pkt;

    static display_on_single_data_pay_func disp_single_data_pay;
};


class on_app_payload;
typedef void(*display_on_app_pay_func)(const on_app_payload&, UInt8,
  const attribute* att, ostream&);
class on_app_payload: public on_single_data_payload
{
public:
    on_app_payload();
    on_app_payload(UInt8 src_unit, UInt8 dst_unit, ona_msg_class_t msg_class,
      UInt8 msg_type, SInt32 msg_data);
    on_app_payload(UInt16 raw_pid, std::string payload, std::string key,
      bool encrypted);
    on_app_payload(const on_app_payload& pld);
    ~on_app_payload();
    void display(UInt8 verbosity, const attribute* att, ostream& outs = cout) const;
    static void default_display_application_payload_info(const on_app_payload& obj,
      UInt8 verbosity, const attribute* att, ostream& outs);
    static void default_display(const on_app_payload& obj, UInt8 verbosity,
      const attribute* att, ostream& outs = cout);
    static void set_display_on_app_pay_function(display_on_app_pay_func func);
    static std::string get_msg_class_string(UInt16 msg_class);


private:
    UInt8 src_unit;
    UInt8 dst_unit;
    ona_msg_class_t msg_class;
    UInt8 msg_type;
    SInt32 msg_data;

    static display_on_app_pay_func disp_app_pay;
};


class on_response_payload;
typedef void(*display_on_response_pay_func)(const on_response_payload&, UInt8,
  const attribute* att, ostream&);
typedef void(*display_on_ack_nack_func)(const on_ack_nack_t& ack_nack, UInt8 verbosity,
  const attribute* att, ostream&, UInt8 num_xtea_blocks);



class on_response_payload: public on_payload
{
public:
    on_response_payload();
    on_response_payload(UInt16 raw_pid, std::string payload, std::string key,
      bool encrypted);
    on_response_payload(const on_response_payload& pld);
    ~on_response_payload();
    static std::string get_nack_reason_string(on_nack_rsn_t nack_reason);
    static std::string get_ack_nack_handle_string(bool is_ack,
      on_ack_nack_handle_t handle);
    static void display_ack_nack(const on_ack_nack_t& ack_nack, UInt8 verbosity,
      const attribute* att, ostream& outs = cout, UInt8 num_xtea_blocks = 1);
    void display(UInt8 verbosity, const attribute* att, ostream& outs = cout) const;
    static void default_ack_nack_display(const on_ack_nack_t& ack_nack, UInt8 verbosity,
      const attribute* att, ostream& outs = cout, UInt8 num_xtea_blocks = 1);
    static void default_display(const on_response_payload& obj, UInt8 verbosity,
      const attribute* att, ostream& outs = cout);
    static void set_display_on_ack_nack_function(display_on_ack_nack_func func);
    static void set_display_on_response_pay_function(display_on_response_pay_func func);
    static UInt8 get_num_relevant_bytes_from_ack_nack_handle(
      const on_ack_nack_t& ack_nack, UInt8 num_xtea_blocks = 1);

private:
    on_ack_nack_t ack_nack;
    ack_nack_payload_t ack_nack_payload;

    static display_on_ack_nack_func disp_ack_nack;
    static display_on_response_pay_func disp_response_pay;
};



// add_remove_client_t and peer_msg_t are different but will fit into the struct
// exactly the same

// add / remove client message.
struct add_remove_client_t
{
    on_encoded_did_t enc_did_bytes;
    UInt8 num_mh_devices;
    UInt8 num_mh_repeaters;
    UInt16 enc_did;
    UInt16 raw_did;
};

// Peer message
struct peer_msg_t
{
    on_encoded_did_t enc_did_bytes;
    UInt8 src_unit;
    UInt8 peer_unit;
    UInt16 enc_did;
    UInt16 raw_did;
};


// data rate / channel change
struct dr_channel_t
{
    UInt8 channel;
    UInt8 data_rate;
    UInt16 pause_time_ms;
    UInt16 dormant_time_ms;
};


// block /stream transfer request
struct bs_transfer_request_t
{
    // much of this is irrelevant for stream
    UInt8 bs_flags;
    UInt32 transfer_size;
    UInt8 chunk_size;
    UInt16 frag_delay_ms;
    UInt16 chunk_pause_ms;
    UInt8 channel;
    UInt8 data_rate;
    UInt16 timeout_ms;
    on_encoded_did_t dst_did_bytes;
    UInt32 estimated_time_ms;
    UInt16 enc_dst_did;
    UInt16 raw_dst_did;
};


// reserve repeater request
struct reserve_repeater_t
{
    on_encoded_did_t rptr_did_bytes;
    on_encoded_did_t src_did_bytes;
    on_encoded_did_t dst_did_bytes;
    UInt32 transfer_time_ms;
    UInt8 channel;
    UInt8 data_rate;
    UInt8 priority;
    UInt16 enc_rptr_did;
    UInt16 raw_rptr_did;
    UInt16 enc_src_did;
    UInt16 raw_src_did;
    UInt16 enc_dst_did;
    UInt16 raw_dst_did;
};


// change fragment delay
struct change_frag_delay_t
{
    UInt16 frag_delay_low_ms;
    UInt16 frag_delay_high_ms;
};


// terminate block stream message
struct terminate_block_stream_t
{
    on_encoded_did_t terminating_did_bytes;
    on_message_status_t status;
    on_ack_nack_t response_ack_nack;
    UInt8 ack_nack_payload_bytes[5];
    UInt16 enc_terminating_did;
    UInt16 raw_terminating_did;
};



class on_admin_payload;
typedef void(*display_on_admin_pay_func)(const on_admin_payload&, UInt8,
  const attribute* att, ostream&);
class on_admin_payload: public on_single_data_payload
{
public:
    on_admin_payload();
    on_admin_payload(const UInt8* bytes, UInt8 num_bytes = 5);
    on_admin_payload(UInt16 raw_pid, std::string payload, std::string key,
      bool encrypted);
    on_admin_payload(const on_admin_payload& pld);
    ~on_admin_payload();
    void parse_bytes(const UInt8* bytes, UInt8 num_bytes = 5);
    static std::string get_admin_type_string(UInt8 admin_type);
    void display(UInt8 verbosity, const attribute* att, ostream& outs = cout) const;
    static void admin_payload_details_to_stream(const on_admin_payload& obj, UInt8 verbosity,
      const attribute* att, ostream& outs = cout);
    static void default_display(const on_admin_payload& obj, UInt8 verbosity,
      const attribute* att, ostream& outs = cout);
    static void set_display_on_admin_pay_function(display_on_admin_pay_func func);


private:
    UInt8 admin_type;
    UInt8 admin_data_bytes[20]; // constant for 20?
    UInt8 num_admin_bytes;

    union
    {
        UInt8 bytes[20]; // not used, but this guarantees that we have space!
        one_net_xtea_key_t xtea_key;
        one_net_xtea_key_fragment_t key_frag;
        on_features_t features;
        UInt8 flags;
        UInt32 value;
        peer_msg_t peer_msg;
        add_remove_client_t add_remove_client;
        dr_channel_t dr_channel;
        bs_transfer_request_t bs_transfer_request;
        reserve_repeater_t reserve_repeater;
        change_frag_delay_t change_frag_delay;
        terminate_block_stream_t terminate_block_stream;
    };

    static display_on_admin_pay_func disp_admin_pay;
};


class on_features_payload;
typedef void(*display_on_features_pay_func)(const on_features_payload&, UInt8,
  const attribute* att, ostream&);
class on_features_payload: public on_single_data_payload
{
public:
    on_features_payload();
    on_features_payload(UInt16 raw_pid, std::string payload, std::string key,
      bool encrypted);
    on_features_payload(const on_features_payload& pld);
    ~on_features_payload();
    void display(UInt8 verbosity, const attribute* att, ostream& outs = cout) const;
    static void default_display(const on_features_payload& obj, UInt8 verbosity,
      const attribute* att, ostream& outs = cout);
    static void set_display_on_features_pay_function(display_on_features_pay_func func);


private:
    on_features_t features;

    static display_on_features_pay_func disp_features_pay;
};


class on_route_payload;
typedef void(*display_on_route_pay_func)(const on_route_payload&, UInt8,
  const attribute* att, ostream&);
class on_route_payload: public on_single_data_payload
{
public:
    on_route_payload();
    on_route_payload(UInt8* route_bytes);
    on_route_payload(UInt16 raw_pid, std::string payload, std::string key,
      bool encrypted);
    on_route_payload(const on_route_payload& pld);
    ~on_route_payload();
    static std::string route_payload_info_to_string(const UInt8* route_bytes,
      UInt8 verbosity);
    void display(UInt8 verbosity, const attribute* att, ostream& outs = cout) const;
    static void default_display(const on_route_payload& obj, UInt8 verbosity,
      const attribute* att, ostream& outs = cout);
    static void set_display_on_route_pay_function(display_on_route_pay_func func);


private:
    UInt8 route_bytes[21];

    static display_on_route_pay_func disp_route_pay;
};




class on_invite_payload;
typedef void(*display_on_invite_pay_func)(const on_invite_payload&, UInt8,
  const attribute* att, ostream&);
class on_invite_payload: public on_payload
{
public:
    on_invite_payload();
    on_invite_payload(UInt16 raw_pid, std::string payload, std::string key,
      bool encrypted);
    on_invite_payload(const on_invite_payload& pld);
    ~on_invite_payload();
    void display(UInt8 verbosity, const attribute* att, ostream& outs = cout) const;
    static void default_display(const on_invite_payload& obj, UInt8 verbosity,
      const attribute* att, ostream& outs = cout);
    static void set_display_on_invite_pay_function(display_on_invite_pay_func func);


private:
    UInt8 version;
    UInt16 raw_did;
    std::string network_key;
    one_net_xtea_key_t network_key_bytes;
    std::string features;
    on_features_t features_bytes;


    static display_on_invite_pay_func disp_invite_pay;
};




class on_block_payload;
typedef void(*display_on_block_pay_func)(const on_block_payload&, UInt8,
  const attribute* att, ostream&);
class on_block_payload: public on_payload
{
public:
    on_block_payload();
    on_block_payload(UInt16 raw_pid, std::string payload, std::string key,
      bool encrypted);
    on_block_payload(const on_block_payload& pld);
    ~on_block_payload();
    void display(UInt8 verbosity, const attribute* att, ostream& outs = cout) const;
    static void default_display(const on_block_payload& obj, UInt8 verbosity,
      const attribute* att, ostream& outs = cout);
    static void set_display_on_block_pay_function(display_on_block_pay_func func);


private:
    block_pkt_t block_pkt;
    UInt8 data[25];

    static display_on_block_pay_func disp_block_pay;
};




class on_stream_payload;
typedef void(*display_on_stream_pay_func)(const on_stream_payload&, UInt8,
  const attribute* att, ostream&);
class on_stream_payload: public on_payload
{
public:
    on_stream_payload();
    on_stream_payload(UInt16 raw_pid, std::string payload, std::string key,
      bool encrypted);
    on_stream_payload(const on_stream_payload& pld);
    ~on_stream_payload();
    void display(UInt8 verbosity, const attribute* att, ostream& outs = cout) const;
    static void default_display(const on_stream_payload& obj, UInt8 verbosity,
      const attribute* att, ostream& outs = cout);
    static void set_display_on_stream_pay_function(display_on_stream_pay_func func);


private:
    stream_pkt_t stream_pkt;
    UInt8 data[25];

    static display_on_stream_pay_func disp_stream_pay;
};



class on_packet;
typedef void(*display_on_packet_func)(const on_packet&, UInt8,
  const attribute* att, ostream&);
class on_packet
{
public:
    on_packet();
    on_packet(std::string encoded_bytes, std::string key);
    on_packet(const on_packet& orig);
    virtual ~on_packet();

    void display(UInt8 verbosity = 255, const attribute* att = NULL, ostream& outs = cout);
    static void default_display(const on_packet& obj, UInt8 verbosity,
      const attribute* att, ostream& outs = cout);
    static void set_display_on_packet_function(display_on_packet_func func);
    static string get_raw_pid_string(UInt16 raw_pid);


    static one_net_status_t on_encode_uint32(UInt32* encoded, UInt32 decoded);
    static one_net_status_t on_decode_uint32(UInt32* decoded, UInt32 encoded);
    static one_net_status_t on_encode_nid(uint64_t* encoded_nid, uint64_t decoded_nid);
    static one_net_status_t on_decode_nid(uint64_t* decoded_nid, uint64_t encoded_nid);

    std::string get_error_message(){return error_message;}
    bool get_valid(){return valid;}
    bool get_is_invite_pkt(){return is_invite_pkt;}
    UInt32 get_timestamp_ms(){return timestamp_ms;}
    void set_timestamp_ms(UInt32 timestamp_ms){this->timestamp_ms = timestamp_ms;}

private:
    UInt32 timestamp_ms;
    std::string encoded_packet;
    std::string encoded_payload;
    std::string decoded_payload;
    UInt8 encoded_packet_bytes[ON_MAX_ENCODED_PKT_SIZE];
    UInt8* encoded_payload_bytes;
    UInt8 decoded_payload_bytes[ON_MAX_RAW_PLD_LEN_WITH_TECH];


    std::string preamble_header;
    UInt8 num_encoded_bytes;
    UInt8 encoded_payload_len;
    UInt8 decoded_payload_len;
    UInt16 raw_src_did;
    UInt16 raw_rptr_did;
    UInt16 raw_dst_did;
    uint64_t raw_nid;
    UInt16 enc_src_did;
    UInt16 enc_rptr_did;
    UInt16 enc_dst_did;
    uint64_t enc_nid;
    UInt16 enc_pid;
    UInt16 raw_pid;
    UInt8 enc_msg_crc;
    UInt8 msg_crc;
    UInt8 calculated_msg_crc;
    UInt8 hops;
    UInt8 max_hops;
    UInt8 encoded_hops_field;
    UInt8 raw_hops_field;
    bool valid_digits;
    bool valid_msg_crc;
    bool valid_decode;
    bool valid_pid;
    bool valid;
    bool is_invite_pkt;
    bool is_single_data_pkt;
    bool is_response_pkt;
    bool is_ack_pkt;
    bool is_nack_pkt;
    bool is_route_pkt;
    bool is_block_pkt;
    bool is_stream_pkt;
    bool is_app_pkt;
    bool is_admin_pkt;
    bool is_features_pkt;
    bool is_multihop_pkt;
    bool is_stay_awake_pkt;
    SInt8 num_payload_blocks;
    std::string error_message;
    on_payload* payload;
    static display_on_packet_func disp_pkt;
};


#endif	/* ON_PACKET_H */
