#ifndef CLI_H
#define	CLI_H


#include <iostream>
#include <fstream>
#include <string>
using namespace std;


extern const int NUM_HELP_STRINGS;


void cli_print_prompt(ostream& outs);
void chip_cli_print_prompt(ostream& outs);
void cli_execute_help(ostream& outs);
bool cli_execute_command(ostream& outs, string& command_line);
bool cli();


#endif	/* CLI_H */

