README

This configuration is designed to be used with the Desktop Sniffer program.  It therefore has the following settings...

1) Lines are handled a line at a time, not a characterat a time (HANDLE_UART_BY_LINE is defined)
2) Handling of the newlines is handled by the Desktop Sniffer (UART_CARRIAGE_RETURN_CONVERT is not defined)
3) It is generally not intended to be anything but a sniffer (though this can be changed!), so as many messages as possible are enabled, while disabling
   other things.
4) Non-volatile memory is disabled(NON_VOLATILE memory is not defined).
5) Auto-Mode is disabled(AUTO_MODE is not defined)
6) ONE_NET_CLIENT is not defined simply to make room so everything fits.
7) Block, stream, multi-hop, and peer are all defined.
8) DEBUGGING_TOOLS is defined.

Feel free to change to your particular needs.
