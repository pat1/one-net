README -- A more thorough README and tutorial will follow soon.
Feb. 5, 2012

This project is the 2.0.3 Beta Eval Board project specifically geared towards running a command-line program from a desktop Command-Line Interface program written in C++ rather than a "dumb" terminal program like Tera Term.  Compile the cpp_sniff_eval project on the Eval Board using Renesas HEW and attach a serial board to the Eval Board.  The program assumes it is attached to /ttySS0 (see comment below fo where to change in the code if this is invalid).

On a Linux desktop environment, assuming the following is available...

1. NetBeans with the C / C++ add-on.
2. gcc and g++ compilers are available and NetBeans is configured to use them.
3. User has permission to access to access serial port /dev/SS0.
4. stdint.h library is available and the uint64_t type is defined.

Open the one_net_lib and one_net_packet_cpp_cli projects in NetBeans.  Set one_net_packet_cpp_cli as the main project and build it.  The one_net_packet_cli project will build the one_net_lib static library and link to it.

Run the program as a console program. Type "h" at the prompt for help.

To toggle between "chip" mode and "desktop" mode, enter "chip_cli" and "cli" at the prompt (note that the prompt will tell you when you are in chip mode).

When in chip mode, enter any command that can execute on the Eval Board (i.e. "list", "sniff:US:3", etc.).  The Eval Board will execute the command and the results will write to the console.

In particular, to sniff on US channel 3, type "sniff:US:3" when in chip mode.  Packets will appear on the screen.  Copy and paste them into a file called packets.txt (or whatever you wish to call them), switch to the desktop mode, then type the following...

load packets.txt

The packets will be parsed into verbose mode.

See "help" for help on the "filter" and "attribute" commands.  You can filter out packets and select the verbosity using these commands.

A more in depth tutorial will follow soon!


Note on serial ports...

"/dev/SS0" is hard-coded as the serial port at the top of the file cli.cpp.  If you are using a differnt serial port, change that line to match.


I developed this in NetBeans on Linux, but I don't see any reason why it can't easily be ported to other IDEs or no IDE at all and also to Windows using Cygwin environment.
