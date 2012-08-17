#include "one_net_xtea.h"
#include "one_net_types.h"
#include "string_utils.h"
#include <fstream>
#include <iostream>
#include <string>



bool ParseArgsForKey(int argc, char* argv[], one_net_xtea_key_t& key,
  std::string& error)
{
    std::string key_string;
    error = "";

    for(int i = 0; i < argc - 1; i++)
    {
        if(strcmp(argv[i], "-kf") == 0)
        {
            std::ifstream ins;
            ins.open(argv[i+1]);
            if(!ins)
            {
                error = "Could not open " + std::string(argv[i+1]) + " for reading.";
                return false;
            }

            getline(ins, key_string, '\n');
            strip_all_whitespace(key_string);
            strip_all_hyphens(key_string);
            ins.close();
            
        }
        else if(strcmp(argv[i], "-k") == 0)
        {
            key_string += std::string(argv[i+1]);
        }
        else
        {
            continue;
        }

        if(key_string.length() < 32)
        {
            error = "Key string " + key_string + " is too short for an XTEA key string.\n";
            return false;
        }
        if(key_string.substr(0, 2) == "0x")
        {
            key_string = key_string.substr(2);
        }

        if(!string_to_xtea_key(key_string, key))
        {
            error = "Could not parse string " + key_string + " into an XTEA key.";
            return false;
        }
        return true;
    }
    return false; // key not found
}


bool ParseArgsForInputArray(int argc, char* argv[], UInt8* bytes,
  UInt8& num_bytes, std::string& error)
{
    std::string array_string;
    error = "";

    for(int i = 0; i < argc - 1; i++)
    {
        if(strcmp(argv[i], "-if") == 0)
        {
            std::ifstream ins;
            ins.open(argv[i+1]);
            if(!ins)
            {
                error = "Could not open " + std::string(argv[i+1]) + " for reading.";
                return false;
            }

            getline(ins, array_string, '\n');
            strip_all_whitespace(array_string);
            strip_all_hyphens(array_string);
            ins.close();            
        }
        else if(strcmp(argv[i], "-i") == 0)
        {
            array_string += string(argv[i+1]);
        }
        else
        {
            continue;
        }

        if(array_string.substr(0, 2) == "0x")
        {
            array_string = array_string.substr(2);
        }

        if(!hex_string_to_bytes(array_string, bytes, num_bytes))
        {
            error = "Could not parse input string into valid hex array.";
            return false;
        }
        return true;
    }

    error = "No -i argument found.";
    return false; // not found
}


bool ParseArgsForSize(int argc, char* argv[], UInt8& size, std::string& error)
{
    std::string value_string;
    error = "";

    for(int i = 0; i < argc - 1; i++)
    {
        if(strcmp(argv[i], "-size") == 0)
        {
            value_string += string(argv[i+1]);
        }
        else
        {
            continue;
        }

        bool hex = false;
        if(value_string.substr(0, 2) == "0x")
        {
            value_string = value_string.substr(2);
            hex = true;
        }

        return string_to_uint8(value_string, size, hex);
    }

    error = "-size argument not found.";
    return false; // not found
}


bool ParseArgsForRounds(int argc, char* argv[], UInt8& rounds, std::string& error)
{
    std::string value_string;
    error = "";

    for(int i = 0; i < argc - 1; i++)
    {
        if(strcmp(argv[i], "-rounds") == 0)
        {
            value_string += string(argv[i+1]);
        }
        else
        {
            continue;
        }

        bool hex = false;
        if(value_string.substr(0, 2) == "0x")
        {
            value_string = value_string.substr(2);
            hex = true;
        }

        return string_to_uint8(value_string, rounds, hex);
    }

    error = "-rounds argument not found.";
    return false; // not found
}


bool ParseArgsForInputValue(int argc, char* argv[], uint64_t& value,
  std::string& error)
{
    std::string value_string;
    error = "";

    for(int i = 0; i < argc - 1; i++)
    {
        if(strcmp(argv[i], "-if") == 0)
        {
            std::ifstream ins;
            ins.open(argv[i+1]);
            if(!ins)
            {
                error = "Could not open " + std::string(argv[i+1]) + " for reading.";
                return false;
            }
            ins >> value_string;
            ins.close();
            
        }
        else if(strcmp(argv[i], "-i") == 0)
        {
            value_string += string(argv[i+1]);
        }
        else
        {
            continue;
        }

        bool hex = false;
        if(value_string.substr(0, 2) == "0x")
        {
            value_string = value_string.substr(2);
            hex = true;
        }

        return string_to_uint64_t(value_string, value, hex);
    }

    error = "-i argument not found.";
    return false; // not found
}


bool ParseArgsForOutputStream(int argc, char* argv[], std::ostream** outs,
  std::string& error)
{
    std::string value_string;
    error = "";


    for(int i = 0; i < argc - 1; i++)
    {
        if((strcmp(argv[i], "-o") == 0) || (strcmp(argv[i], "-of") == 0))
        {
            std::ofstream* outfile = new ofstream;
            outfile->open(argv[i+1]);
            if(!(*outfile).good())
            {
                *outs = NULL;
                outfile->close();
                delete outfile;
                error = "Could not open " + std::string(argv[i+1]) + " for writing.";
                return false;
            }

            *outs = reinterpret_cast<std::ostream*>(outfile);
            return true;
        }
    }
    *outs = &std::cout;
    return true; // cout used
}


void close_stream(std::ostream* outs)
{
    if(outs == NULL || outs == &std::cout)
    {
        return;
    }

    std::ofstream* outfile = dynamic_cast<std::ofstream*>(outs);
    outfile->close();
    delete outfile;
}


void close_stream(std::istream* ins)
{
    if(ins == NULL || ins == &std::cin)
    {
        return;
    }

    std::ifstream* infile = dynamic_cast<std::ifstream*>(ins);
    infile->close();
    delete infile;
}
