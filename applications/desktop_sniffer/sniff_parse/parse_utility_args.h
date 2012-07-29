#ifndef PARSE_UTILITY_ARGS_H
#define PARSE_UTILITY_ARGS_H

#include "one_net_xtea.h"
#include "one_net_types.h"
#include <fstream>
#include <ostream>


bool ParseArgsForKey(int argc, char* argv[], one_net_xtea_key_t& key,
  std::string& error);
bool ParseArgsForInputArray(int argc, char* argv[], UInt8* bytes,
  UInt8& num_bytes, std::string& error);
bool ParseArgsForSize(int argc, char* argv[], UInt8& size, std::string& error);
bool ParseArgsForRounds(int argc, char* argv[], UInt8& size, std::string& error);
bool ParseArgsForInputValue(int argc, char* argv[], uint64_t& value,std::string& error);
bool ParseArgsForOutputStream(int argc, char* argv[], std::ostream** outs,
  std::string& error);
void close_stream(std::ostream* outs);
void close_stream(std::istream* ins);


#endif
