#include <cstdlib>
#include <sstream>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include "one_net_xtea.h"
#include "one_net_types.h"
#include "one_net.h"
#include "on_packet.h"
#include "one_net_packet.h"
#include "one_net_encode.h"
using namespace std;




// Call this function every time a line is read from the sniffer.  This function
// will parse through the sniffer lines format and return true when a potential packet
// has been received. Keyu word here is "potential".  This function merely parses
// lines of text from sniffer output to a string usable by the on_packet constructor.
// Further parsing is needed to parse the hex string into a usable and valid packet.
bool sniffer_format_to_hex_string(std::string line, std::string& packet_hex_string,
    UInt32& timestamp_ms)
{
    static bool rcvd_num_bytes = false;
    static int num_bytes_rcvd;
    static int num_bytes_expected;
    static UInt32 timestamp_ms_static;
    static UInt8 bytes[ON_MAX_ENCODED_PKT_SIZE];
    UInt16 raw_pid, enc_pid;


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
        ss >> timestamp_ms_static;
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
            enc_pid = one_net_byte_stream_to_uint16(
              &bytes[ON_ENCODED_PID_IDX]);
            if(on_decode_uint16(&raw_pid, enc_pid) != ONS_SUCCESS)
            {
                rcvd_num_bytes = false;
                return false;
            }

            if(num_bytes_expected != (int) get_encoded_packet_len(raw_pid,
              TRUE))
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

    // So far, so good.  We seem to have received all of the bytes we need to
    // try to construct a packet.
    rcvd_num_bytes = false; // Packet bytes received.  Set false for next time.
    timestamp_ms = timestamp_ms_static;
    packet_hex_string = bytes_to_hex_string(bytes, num_bytes_expected);
    return true;
}


bool sniffer_file_to_hex_string(FILE* sniffer_file, std::string& packet_hex_string,
    UInt32& timestamp_ms)
{
    static string line = "";
    char c = fgetc(sniffer_file);
    if(c == EOF)
    {
        return false;
    }
    if(c == '\r')
    {
        return false;
    }
    else if(c != '\n')
    {
        line += c;
        return false;
    }
    bool ret = sniffer_format_to_hex_string(line, packet_hex_string, timestamp_ms);
    line = ""; // reset line for next time.
    return ret;
}


void usage()
{
    cout << "usage: ./sniff_parse verbosity [valid/invalid/both] filename_of_sniffer_text_file [output_filename]\n";
    exit(0);
}


int main(int argc, char** argv)
{
    if(argc != 4 && argc != 5)
    {
        usage();
    }

    string verbosity_str(argv[1]);
    UInt8 verbosity;
    if(!string_to_uint8(verbosity_str, verbosity, false))
    {
        std::cout << "Could not convert argv[1](" << argv[1] << ") to a decimal value "
                  << "between 0 and 255, inclusive.\n";
        usage();
    }

    bool reject_invalid = false;
    bool reject_valid = false;
    if(strcmp(argv[2], "valid") == 0)
    {
        reject_invalid = true;
    }
    else if(strcmp(argv[2], "invalid") == 0)
    {
        reject_valid = true;
    }
    else if(strcmp(argv[2], "both") == 0)
    {
    }
    else
    {
        std::cout << "argv[2](" << argv[2] << ") must be \"valid\", \"invalid\", or \"both\"";
        usage();
    }


    ofstream outs;
    if(argc == 5)
    {
        outs.open(argv[4]);
        if(!outs.good())
        {
            cout << "Could not open file " << argv[4] << " for writing.\n";
            exit(0);
        }
    }


    FILE* sniffer_file = fopen(argv[3], "r");
    if(sniffer_file == NULL)
    {
        cout << "Could not open file " << argv[3] << " for reading.\n";
        if(argc == 5)
        {
            outs.close();
        }
        exit(0);
    }


    std::string packet_hex_string;
    UInt32 timestamp_ms;
    std::string invite_key = "32323232323232323232323232323232";
    std::string network_key = "000102030405060708090A0B0C0D0E0F";

    while(!feof(sniffer_file))
    {
        if(sniffer_file_to_hex_string(sniffer_file, packet_hex_string, timestamp_ms))
        {
            on_packet new_invite_packet(packet_hex_string, invite_key);
            on_packet new_non_invite_packet(packet_hex_string, network_key);
            if(new_invite_packet.get_is_invite_pkt())
            {
                new_invite_packet.set_timestamp_ms(timestamp_ms);

                if(reject_valid && new_invite_packet.get_valid())
                {
                    continue;
                }
                if(reject_invalid && !new_invite_packet.get_valid())
                {
                    continue;
                }

                new_invite_packet.display(verbosity, NULL, argc == 5 ? outs : cout);
                outs << "\n\n\n\n\n\n";
            }
            else
            {
                new_non_invite_packet.set_timestamp_ms(timestamp_ms);

                if(reject_valid && new_non_invite_packet.get_valid())
                {
                    continue;
                }
                if(reject_invalid && !new_non_invite_packet.get_valid())
                {
                    continue;
                }

                new_non_invite_packet.display(verbosity, NULL, argc == 5 ? outs : cout);
                outs << "\n\n\n\n\n\n";
            }
        }
    }

    if(argc == 5)
    {
        outs.close();
    }

    return 0;
}
