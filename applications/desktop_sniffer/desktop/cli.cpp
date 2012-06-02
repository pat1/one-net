#include <iostream>
#include <termios.h>
#include <fstream>
#include <string>
#include <sstream>
#include <cctype>
#include <fcntl.h>
#include "packet.h"
#include "string_utils.h"
#include "cli.h"
#include "xtea_key.h"
#include "chip_connection.h"
#include "attribute.h"
#include "filter.h"
using namespace std;


extern "C"
{
    #include "one_net_xtea.h"
}

const speed_t DEFAULT_BAUD = B115200;
const string DEFAULT_DEVICE = "/dev/ttyS0";

const int NUM_HELP_STRINGS = 46;
bool chip_cli_mode = false;
chip_connection* chip_con = NULL;



char command_buffer[10000];
int command_buffer_index = 0;


static attribute att;
static filter pkt_filter;


bool logging = false;
string log_filename = "log.txt";
filebuf log_buf;
ostream* log_file = NULL;

vector <packet> packets;



static const char* const HELP_STRINGS[] =
{
    "help -- explanation of commands.",
    "clear -- removes all packets from memory.",
    "load a.txt -- loads packets from a.txt into memory.  Removes all existing "
        "packets from memory.",
    "remove a.txt -- removes all packets from a.txt from memory.",
    "add a.txt -- adds all packets from a.txt from memory.",
    "save a.txt -- saves all packets in memory to a.txt.",
    "save a.txt verbose -- saves all packets in memory in verbose fashion.",
    "filter display -- displays the packet filer criteria",
    "filter remove all -- no packets are filtered (i.e. all are shown).",
    "filter add all -- all packets are filtered (i.e. none are shown).",
    "filter remove pid -- no filter on the pid.",
    "filter remove pid 0 1 2 -- pids 0, 1, and 2 will not be displayed.",
    "filter add pid 0 1 2 -- pids 0, 1, and 2 will be displayed.",
    "filter add src_did 3 5 -- Source DIDs 003 and 005 will be displayed.",
    "filter add keys 2 -- add the second key from the list to the invite key list.  Type 'display keys' for a list.",
    "filter add invite_keys 2 -- add the second invite key from the list to the invite key list.  Type 'display invite_keys' for a list.",
    "filter remove keys 2 -- removes the second key from the list to the invite key list.  Type 'display keys' for a list.",
    "filter remove invite_keys 2 -- removes the second invite key from the list to the invite key list.  Type 'display invite_keys' for a list.",
    "filter display keys -- display keys in the filter",
    "filter display invite_keys -- display invite keys in the filter",
    "filter add msg_crc_match true -- filter out all packets where the message crc does not match",
    "filter add payload_crc_match false -- filter out all packets where the message crc matches",
    "filter add valid true -- filter out any invalid packets",
    "attribute display -- shows what attributes are displayed",
    "attribute rptr_did src_did pid bytes -- sets to the filter to show the repeater did, the source did, the pid, and the packet bytes.  Execute the \"attribute display\" command to show options.",
    "attribute add nid -- adds nid to the list if not already there",
    "attribute remove payload_crc -- removes payload_crc from list if there.",
    "attribute removal all -- remove all attributes from list.",
    "attribute add all -- adds all attributes from list.",
    "keys remove all -- remove all keys.",
    "invite_keys remove all -- remove all invite keys.",
    "keys remove 00-01-02-03-04-05-06-07-08-09-0A-0B-0C-0D-0E-0F -- remove key.",
    "invite_keys remove 2222-2222 -- remove 2222-2222 as an invite key.",
    "keys add 00-01-02-03-04-05-06-07-08-09-0A-0B-0C-0D-0E-0F -- add key.",
    "invite_keys add 2222-2222 -- add 2222-2222 as an invite key.",
    "keys display -- displays all non-invite keys.",
    "invite_keys display -- displays all invite keys.",
    "chip_cli -- sets mode to the microchip command line interface",
    "cli -- sets mode to the desktop command line interface",
    "log log.txt -- sets log file to log.txt.",
    "log on -- turns logging on",
    "log off -- logging is off",
    "log clear -- clear the log file",
    "log display -- display log file",
    "log save a.txt -- saves the log to a.txt",
    "exit -- exits the program"
};


void cli_print_prompt(ostream& outs)
{
    outs << "\n\nEnter Command (" << (logging ? "Logging, " : "Not Logging, ")
        << "Logfile = " << log_filename << ", \"chip_cli\" for chip commands, "
        << "\"help\" for help) >\n";
}


void chip_cli_print_prompt(ostream& outs)
{
    outs << "\n\nEnter Chip Command (" << (logging ? "Logging, " : "Not Logging, ")
        << "Logfile = " << log_filename << ", \"cli\" to return to desktop CLI) >\n";
}


void cli_print_error(ostream& outs)
{
    outs << "Could not execute or parse command.\n";
}


bool open_log_file(bool append_mode)
{
    if(log_file)
    {
        delete log_file;
    }

    log_file = NULL;

    if(log_buf.is_open())
    {
        log_buf.close();
    }

    if(append_mode)
    {
        if(!log_buf.open(log_filename.c_str(), ios::out | ios::app))
        {
            logging = false;
            return false;
        }
    }
    else
    {
        if(!log_buf.open(log_filename.c_str(), ios::out))
        {
            logging = false;
            return false;
        }
    }

    try
    {
        log_file = new ostream(&log_buf);
    }
    catch(...)
    {
        logging = false;
        log_buf.close();
        log_file = NULL;
        return false;
    }

    if(!log_file->good())
    {
        logging = false;
        delete log_file;
        log_file = NULL;
        log_buf.close();
        return false;
    }

    return true;
}


void cli_execute_help(ostream& outs)
{
    string print_string;
    const int MULTI_LINE_PAD = 3;
    const string pad = "   ";
    const int HELP_LINE_LENGTH = 80;
    for(int i = 0; i < NUM_HELP_STRINGS; i++)
    {
        print_string = HELP_STRINGS[i];
        string remaining;
        int line_length = HELP_LINE_LENGTH;
        while(1)
        {
            strip_leading_and_trailing_whitespace(print_string);

            if(line_length < HELP_LINE_LENGTH)
            {
                outs << pad;
            }

            if((int) print_string.length() <= line_length)
            {
                outs << print_string << endl;
                break;
            }

            while(!isspace(print_string[line_length - 1]))
            {
                line_length--;
            }

            remaining = print_string.substr(line_length);
            print_string = print_string.substr(0, line_length);
            strip_leading_and_trailing_whitespace(remaining);
            outs << print_string << endl;
            print_string = remaining;
            line_length = HELP_LINE_LENGTH - MULTI_LINE_PAD;
        }
    }
}


bool cli_execute_keys(bool invite_key, string command_line)
{
    string command, args;

    split_string(command_line, command, args);
    str_tolower(command);

    ADD_REMOVE_DISPLAY add_remove_display = parse_add_remove_display(command);
    if(add_remove_display == ADD_REMOVE_ERROR)
    {
        return false;
    }

    if(add_remove_display == DISPLAY)
    {
        display_keys(cout, invite_key, invite_key ? packet::invite_keys :
            packet::keys);
        if(logging)
        {
            display_keys(*log_file, invite_key, invite_key ? packet::invite_keys
                : packet::keys);
        }
        return true;
    }

    if(args == "")
    {
        return false;
    }

    xtea_key key;

    if(invite_key)
    {
        if(!parse_invite_key_string(args, key))
        {
            return false;
        }
    }
    else
    {
        if(!parse_key_string(args, key))
        {
            return false;
        }
    }

    if(add_remove_display == ADD)
    {
        if(invite_key)
        {
            xtea_key::insert_key(packet::invite_keys, key, false);
        }
        else
        {
            xtea_key::insert_key(packet::keys, key, false);
        }
    }
    else
    {
        if(invite_key)
        {
            xtea_key::delete_key(packet::invite_keys, key);
        }
        else
        {
            xtea_key::delete_key(packet::keys, key);
        }
    }

    return true;
}


bool cli_execute_filter(string command_line)
{
    string command, args;

    split_string(command_line, command, args);
    str_tolower(command);

    ADD_REMOVE_DISPLAY add_remove_display = parse_add_remove_display(command);
    if(add_remove_display == ADD_REMOVE_ERROR)
    {
        return false;
    }

    if(add_remove_display == DISPLAY)
    {
        pkt_filter.display(cout);
        if(logging)
        {
            pkt_filter.display(*log_file);
        }
        return true;
    }

    if(args == "")
    {
        return false;
    }

    bool add = false;
    if(add_remove_display == ADD)
    {
        add = true;
    }
    else
    {
        if(args == "all")
        {
            pkt_filter.remove_filter();
            return true;
        }
    }

    string ft_string;
    split_string(args, ft_string, args);

    filter::FILTER_TYPE ft = filter::string_to_filter_type(ft_string);
    if(ft == filter::FILTER_INVALID)
    {
        return false;
    }

    if(args == "")
    {
        if(add)
        {
            return false;
        }

        return pkt_filter.remove_filter(ft);
    }

    bool range = false;
    if(ft == filter::FILTER_KEY_FRAGMENT)
    {
        strip_all_hyphens(args);
        strip_all_whitespace(args);
        uint64_t key_frag_value;
        if(!string_to_uint64_t(args, key_frag_value, true))
        {
            return false;
        }

        if(add)
        {
            pkt_filter.accept_value(ft, key_frag_value);
        }
        else
        {
            pkt_filter.reject_value(ft, key_frag_value);
        }

        return true;
    }
    else
    {
        // count the hyphens.  There can be 0 or 1.
        size_t index = args.find('-', 0);
        if(index != string::npos)
        {
            args[index] = ' ';
            range = true;

            // there had better not be another one.
            if(args.find_first_of('-', index + 1) != string::npos)
            {
                return false; // too many hyphens
            }
        }

        if(ft == filter::FILTER_PAYLOAD_CRC_MATCH || ft ==
            filter::FILTER_MSG_CRC_MATCH || ft == filter::FILTER_VALID_MATCH ||
            ft == filter::FILTER_VALID_MATCH)
        {
            bool val;
            if(!string_to_bool(args, val))
            {
                return false;
            }

            filter::FILTER_MATCH fm = (val == 0 ? filter::MUST_NOT_MATCH :
                filter::MUST_MATCH);

            return pkt_filter.accept_value(ft, (uint64_t) fm);
        }


        vector <uint64_t> values = string_to_uint64_t_vector(args);
        if((ft == filter::FILTER_INVITE_KEYS || ft == filter::FILTER_KEYS) &&
            values.size() != 1)
        {
            return false;
        }
        else if(values.size() == 0)
        {
            return false;
        }
        else if(range)
        {
            if(values.size() != 2)
            {
                return false;
            }

            uint64_t low = values[0];
            uint64_t high = values[1];

            if(add)
            {
                return pkt_filter.accept_range(ft, low, high);
            }
            else
            {
                return pkt_filter.reject_range(ft, low, high);
            }
        }
        else
        {
            if(add)
            {
                return pkt_filter.accept_values(ft, values);
            }
            else
            {
                return pkt_filter.reject_values(ft, values);
            }

            return true;
        }
    }

    return false;
}


bool cli_execute_attribute(string command_line)
{
    string command, args;

    split_string(command_line, command, args);
    str_tolower(command);

    ADD_REMOVE_DISPLAY add_remove_display = parse_add_remove_display(command);
    if(add_remove_display == ADD_REMOVE_ERROR)
    {
        return false;
    }

    if(add_remove_display == DISPLAY)
    {
        att.display_attributes(cout);
        if(logging)
        {
            att.display_attributes(*log_file);
        }
        return true;
    }

    if(args == "")
    {
        return false;
    }


    bool add = false;
    if(add_remove_display == ADD)
    {
        add = true;
    }

    if(args.compare("all") == 0)
    {
        att.set_attributes(add);
        return true;
    }

    return att.set_attribute(args, add);
}


bool cli_execute_load(string command_line, const filter& fltr)
{
    bool use_log_file = false;
    bool ret_value = true;
    string filename = command_line;

    if(command_line == "")
    {
        use_log_file = true;
        filename = log_filename;
        if(log_file)
        {
            delete log_file;
        }
        log_file = NULL;
        if(log_buf.is_open())
        {
            log_buf.close();
        }
    }

    packet pkt;

    ifstream ins;
    ins.open(filename.c_str());

    if(!ins.is_open())
    {
        ret_value = false;
    }
    else
    {
        string line;
        packets.clear();
        while(ins)
        {
            getline(ins, line, '\n');
            if(ins.good())
            {
                if(packet::create_packet(line, fltr, pkt))
                {
                    if(pkt.filter_packet(fltr))
                    {
                        packet::insert_packet(packets, pkt);
                    }
                }
            }
        }

        ins.close();
        struct timeval start_time = {0,0};
        packet::adjust_timestamps(packets, start_time);
        packet::display(packets, att, cout);
    }

    if(use_log_file)
    {
        return (ret_value && open_log_file(true));
    }

    return ret_value;
}


bool cli_execute_log(string& command_line)
{
    bool clear_log = false;
    bool display_log = false;
    bool save_log = false;
    string save_filename;
    bool error = false;

    if(command_line.compare("off") == 0)
    {
        logging = false;
        return true;
    }

    if(command_line.compare("clear") == 0)
    {
        clear_log = true;
    }
    else if(command_line.find(".txt") != string::npos)
    {
        string command;
        split_string(command_line, command, save_filename);
        if(command != "save")
        {
            log_filename = command_line;
            clear_log = true;
            logging = true;
        }
        else
        {
            save_log = true;
        }
    }
    else if(command_line.compare("on") == 0)
    {
        if(logging)
        {
            return true;
        }

        logging = true;
        return open_log_file(true);
    }
    else if(command_line.compare("display") == 0)
    {
        display_log = true;
    }
    else
    {
        return false;
    }

    if(!log_buf.is_open() || !log_file || !(log_file->good()))
    {
        clear_log = true;
    }

    if(clear_log)
    {
        if(!open_log_file(false))
        {
            return false;
        }
    }

    if(display_log || save_log)
    {
        if(logging)
        {
            delete log_file;
            log_file = NULL;
            log_buf.close();
        }

        ofstream outs;
        if(save_log)
        {
            outs.open(save_filename.c_str());
            error = error || !outs.is_open() || !outs.good();
        }

        ifstream ins;
        ins.open(log_filename.c_str());
        string line;

        if(!save_log)
        {
            cout.flush();
        }
        while(ins.good() && !error)
        {
            getline(ins, line, '\n');
            if(save_log)
            {
                outs << line << endl;
                error = error || !outs.is_open() || !outs.good();
            }
            else
            {
                cout << line << endl;
            }
        }

        ins.close();
        if(save_log)
        {
            outs << endl;
            outs.close();
        }
        else
        {
            cout << endl;
        }

        if(logging)
        {
            return open_log_file(true);
        }

        if(error)
        {
            return false;
        }
    }

    return true;
}


bool cli_execute_command(string& command_line)
{
    string command, args;

    strip_leading_and_trailing_whitespace(command_line);
    string original_command = command_line;

    split_string(command_line, command, args);
    str_tolower(command);
    bool valid_parse = true;

    if(command.compare("cli") == 0)
    {
        chip_cli_mode = false;
    }
    else if(command.compare("chip_cli") == 0)
    {
        if(chip_con == NULL)
        {
            chip_con = new chip_connection(DEFAULT_DEVICE, DEFAULT_BAUD);
            if(chip_con->get_chip_fd() < 0)
            {
                delete chip_con;
                return false;
            }
        }

        chip_cli_mode = true;
        chip_con->set_console(false);
    }
    else if(chip_cli_mode)
    {
        return chip_con->send_bytes_to_chip(original_command, true);
    }
    else if(command.compare("h") == 0 || command.compare("help") == 0)
    {
        cli_execute_help(cout);
        if(logging)
        {
            cli_execute_help(*log_file);
        }
    }
    else if(command.compare("keys") == 0)
    {
        cli_execute_keys(false, args);
    }
    else if(command.compare("invite_keys") == 0)
    {
        cli_execute_keys(true, args);
    }
    else if(command.compare("filter") == 0)
    {
        valid_parse = cli_execute_filter(args);
    }
    else if(command.compare("attribute") == 0)
    {
        valid_parse = cli_execute_attribute(args);
    }
    else if(command.compare("log") == 0)
    {
        valid_parse = cli_execute_log(args);
    }
    else if(command.compare("load") == 0)
    {
        valid_parse = cli_execute_load(args, pkt_filter);
    }
    else if(command.compare("exit") == 0)
    {
        delete chip_con;
        return false;
    }
    else
    {
        valid_parse = false;
    }

    if(!valid_parse)
    {
        cli_print_error(cout);
        if(logging)
        {
            cli_print_error(*log_file);
        }
    }

    return true;
}


bool cli()
{
    static bool initialized = false;
    if(!initialized)
    {
        const UInt8 DEFAULT_KEY_BYTES[4][ONE_NET_XTEA_KEY_LEN] =
        {
            {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
            {4,5,6,7,8,9,10,11,12,13,14,15, 0x44,0x44,0x44, 0x44},
            {'2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2'},
            {'3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3'}
        };

        xtea_key default_key1(DEFAULT_KEY_BYTES[0]);
        xtea_key default_key2(DEFAULT_KEY_BYTES[1]);
        xtea_key default_key3(DEFAULT_KEY_BYTES[2]);
        xtea_key default_key4(DEFAULT_KEY_BYTES[3]);
        xtea_key::insert_key(packet::keys, default_key1, true);
        xtea_key::insert_key(packet::keys, default_key2, true);
        xtea_key::insert_key(packet::invite_keys, default_key3, true);
        xtea_key::insert_key(packet::invite_keys, default_key4, true);
        pkt_filter.accept_value(filter::FILTER_KEYS, 1);
        pkt_filter.accept_value(filter::FILTER_KEYS, 2);
        pkt_filter.accept_value(filter::FILTER_INVITE_KEYS, 1);
        pkt_filter.accept_value(filter::FILTER_INVITE_KEYS, 2);

        initialized = true;
    }


    string command_line;
    if(!chip_cli_mode)
    {
        cli_print_prompt(cout);
        if(logging)
        {
            cli_print_prompt(*log_file);
        }
        getline(cin, command_line, '\n');
        if(logging)
        {
            *log_file << command_line << "\n";
        }
        bool ret = cli_execute_command(command_line);
        if(!ret)
        {
            if(log_buf.is_open())
            {
                log_buf.close();
            }

            if(log_file)
            {
                delete log_file;
            }
        }

        return ret;
    }
    else
    {
        chip_cli_print_prompt(cout);
        if(logging)
        {
            chip_cli_print_prompt(*log_file);
        }
        command_buffer_index = 0;
        bool newline_input_rcvd = false;

        while(!newline_input_rcvd)
        {
            struct timeval timeout = {0,0}; // poll, no pause
            fd_set set;
            FD_ZERO(&set);
            FD_SET(STDIN_FILENO, &set);


            int rv = select(STDIN_FILENO + 1, &set, NULL, NULL, &timeout);
            if(rv < -1)
            {
                continue; // Error.  Not sure what to do here.  Abort?
            }
            else if(rv == 0)
            {
                // get any output from the chip
                string chip_output = chip_con->read_bytes(timeout);
                cout << chip_output;
                if(logging)
                {
                    *log_file << chip_output;
                }
            }
            else
            {
                char byte;
                int bytes_read = read(STDIN_FILENO, &byte, 1);
                if(bytes_read)
                {
                    write(STDOUT_FILENO, &byte, 1); // echo the byte
                    if(logging)
                    {
                        *log_file << byte;
                    }
                    if(byte == '\r' || byte == '\n')
                    {
                        newline_input_rcvd =  true;
                    }
                    else
                    {
                        if(command_buffer_index >= ((int) sizeof(command_buffer)
                          - 1))
                        {
                            continue; // prevent seg fault.
                        }
                        
                        command_buffer[command_buffer_index] = byte;
                        command_buffer_index++;
                    }
                }
            }
        }

        command_buffer[command_buffer_index] = 0;
        string command_line_string(command_buffer);
        return cli_execute_command(command_line_string);
    }
}
