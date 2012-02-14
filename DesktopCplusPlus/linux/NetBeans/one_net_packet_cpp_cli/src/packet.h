#ifndef PACKET_H
#define	PACKET_H


#include <sys/time.h>
#include <vector>
#include <string>
#include <cstdio>
#include "filter.h"
#include "xtea_key.h"
#include "attribute.h"
using namespace std;


extern "C"
{
    #include "one_net_packet.h"
    #include "one_net_acknowledge.h"
    #include "one_net_features.h"
    #include "one_net_peer.h"
};


struct app_payload_t
{
    UInt8 num_bytes;
    UInt8 src_unit;
    UInt8 dst_unit;
    UInt16 msg_class;
    UInt16 msg_type;
    UInt16 msg_data;

    static string get_msg_class_string(UInt16 msg_class);
    bool detailed_app_payload_to_string(string& str) const;
};


struct add_remove_client_t
{
    on_encoded_did_t enc_did;
    UInt8 num_mh_devices;
    UInt8 num_mh_repeaters;

    bool detailed_add_remove_client_payload_to_string(string& str) const;
};


struct admin_payload_t
{
    UInt8 admin_type;
    union
    {
        union
        {
            UInt16 frag_time_low_ms;
            UInt16 frag_time_high_ms;
        };
        UInt32 time_ms;
        UInt32 value;
        on_features_t features;
        UInt8 flags;
        UInt8 data[4];
        one_net_xtea_key_fragment_t key_frag;
        add_remove_client_t add_remove_msg;
        on_peer_unit_t peer_msg;
    };

    static string get_admin_type_string(UInt8 admin_type);
    bool detailed_admin_payload_to_string(string& str) const;
};


struct invite_payload_t
{
    UInt8 version;
    UInt16 raw_did;
    UInt8 key[ONE_NET_XTEA_KEY_LEN];
    on_features_t features;
    bool detailed_invite_payload_to_string(string& str) const;
};


struct response_payload_t
{
    ack_nack_payload_t pld;
    on_ack_nack_t ack_nack;
};


struct block_payload_t
{
    UInt8 packet_number;
    UInt32 chunk_number;
};


struct payload_t
{
    UInt8 raw_pid;
    UInt8 decrypted_payload_bytes[ON_MAX_ENCODED_PLD_LEN_WITH_TECH];
    UInt8 num_payload_bytes;
    UInt8 payload_crc;
    UInt8 calculated_payload_crc;
    bool valid_payload_crc;
    UInt8 txn_nonce;
    UInt8 resp_nonce;
    bool is_app_pkt;
    bool is_admin_pkt;
    bool is_features_pkt;
    bool is_invite_pkt;
    union
    {
        UInt8 msg_type;
        UInt8 ack_nack_handle;
    };
    union
    {
        app_payload_t app_payload;
        admin_payload_t admin_payload;
        on_features_t features_payload;
        invite_payload_t invite_payload;
        response_payload_t response_payload;
        block_payload_t block_payload;
        // no elements in a stream payload other than the actual bytes.
    };

    bool detailed_payload_to_string(UInt8 raw_pid, string& str) const;
    static string get_nack_reason_string(on_nack_rsn_t nack_reason);
    static string get_ack_nack_handle_string(bool is_ack,
        on_ack_nack_handle_t handle);
    static bool detailed_data_rates_to_string(on_features_t features,
        string& str);
    static bool detailed_features_to_string(on_features_t features,
        string& str);
    bool detailed_response_payload_to_string(string& str) const;
};


class packet
{
public:
    packet();
    packet(const packet& orig);
    packet& operator = (const packet& orig);
    ~packet();
    static bool parse_app_payload(payload_t& payload);
    static bool parse_admin_payload(payload_t& payload,
        const UInt8* admin_bytes);
    static bool parse_response_payload(payload_t& payload, const filter& fltr);
    static bool parse_invite_payload(payload_t& payload);
    static bool parse_block_payload(payload_t& payload);
    static bool parse_payload(UInt8 raw_pid, UInt8* decrypted_payload_bytes,
        payload_t& payload, const filter& fltr);
    bool filter_packet(const filter& fltr) const;
    static bool create_packet(struct timeval timestamp, UInt8 raw_pid,
        UInt8 num_bytes, const UInt8* const bytes, const filter& fltr,
        packet& pkt);
    static bool create_packet(string line, const filter& fltr, packet& pkt);
    static bool create_packet(int fd, const filter& fltr, packet& pkt);
    static bool create_packet(FILE* file, const filter& fltr, packet& pkt);
    static bool create_packet(istream& is, const filter& fltr, packet& pkt);
    static string get_raw_pid_string(UInt8 raw_pid);
    static bool insert_packet(vector<packet>& packets,
        packet& new_packet);
    static void adjust_timestamps(vector<packet>& packets, struct timeval
        begin_time);
    bool display(const attribute& att, ostream& outs) const;

    static vector<xtea_key> keys;
    static vector<xtea_key> invite_keys;
    
private:
    bool fill_in_packet_values(struct timeval timestamp, UInt8 raw_pid,
        UInt8 num_bytes, const UInt8* const bytes, const filter& fltr);


    struct timeval timestamp;
    UInt8 num_bytes;
    UInt8 enc_pkt_bytes[ON_MAX_ENCODED_PKT_SIZE];
    UInt16 raw_src_did;
    UInt16 raw_rptr_did;
    UInt16 raw_dst_did;
    uint64_t raw_nid;
    on_pkt_t pkt_ptr;
    xtea_key key;
    UInt8 enc_msg_crc;
    UInt8 msg_crc;
    UInt8 calculated_msg_crc;
    UInt8 enc_msg_id;
    UInt8 msg_id;
    UInt8 rounds;
    bool valid;
    bool valid_msg_crc;
    bool valid_decode;
    bool is_large_pkt;
    bool is_extended_pkt;
    bool is_mh_pkt;
    bool is_data_pkt;
    bool is_single_pkt;
    bool is_block_pkt;
    bool is_stream_pkt;
    bool is_ack_pkt;
    bool is_nack_pkt;
    bool is_response_pkt;
    bool is_stay_awake_pkt;
    UInt8 encoded_payload_len;
    UInt8 encrypted_payload_bytes[ON_MAX_ENCODED_PLD_LEN_WITH_TECH];
    payload_t payload;
    UInt8 encoded_hops_field;
    UInt8 decoded_hops_field;
    UInt8 hops;
    UInt8 max_hops;
};





extern vector<packet> valid_packets;
extern vector<packet> invalid_packets;




#endif	/* PACKET_H */
