all: desktop_parser libonenetlib.a

onenetlib: libonenetlib.a



CFLAGS = -Wall -Werror
CPPFLAGS = -Wall -Werror



ONE_NET_LIB_PATH = -Iapplications/desktop_sniffer/desktop -Iprocessors/linux -Iprocessors/linux/common -Ione_net/app -Ione_net/utility -Ione_net/port_specific -Ione_net/mac -Itransceivers -Iprocessors/renesas/src/eval -Iprocessors/renesas/src/eval/adi

ONE_NET_LIB_OBJS = one_net_xtea.o one_net_crc.o one_net_encode.o one_net_memory.o one_net_prand.o one_net_timer.o one_net_features.o one_net_packet.o one_net_message.o one_net_peer.o one_net_application.o one_net_acknowledge.o one_net_port_specific.o one_net.o one_net_client.o one_net_master.o tick.o dummy_client_app_functions.o dummy_master_app_functions.o dummy_one_net_app_functions.o dummy_transceiver_functions.o

one_net_xtea.o: one_net/utility/one_net_xtea.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) one_net/utility/one_net_xtea.c -o one_net_xtea.o

one_net_crc.o: one_net/utility/one_net_crc.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) one_net/utility/one_net_crc.c -o one_net_crc.o

one_net_encode.o: one_net/utility/one_net_encode.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) one_net/utility/one_net_encode.c -o one_net_encode.o

one_net_memory.o: one_net/utility/one_net_memory.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) one_net/utility/one_net_memory.c -o one_net_memory.o

one_net_prand.o: one_net/utility/one_net_prand.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) one_net/utility/one_net_prand.c -o one_net_prand.o

one_net_timer.o: one_net/utility/one_net_timer.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) one_net/utility/one_net_timer.c -o one_net_timer.o

one_net_features.o: one_net/app/one_net_features.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) one_net/app/one_net_features.c -o one_net_features.o

one_net_message.o: one_net/app/one_net_message.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) one_net/app/one_net_message.c -o one_net_message.o

one_net_packet.o: one_net/app/one_net_packet.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) one_net/app/one_net_packet.c -o one_net_packet.o

one_net_peer.o: one_net/mac/one_net_peer.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) one_net/mac/one_net_peer.c -o one_net_peer.o

one_net_application.o: one_net/app/one_net_application.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) one_net/app/one_net_application.c -o one_net_application.o

one_net_acknowledge.o: one_net/app/one_net_acknowledge.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) one_net/app/one_net_acknowledge.c -o one_net_acknowledge.o

one_net_port_specific.o: processors/linux/one_net_port_specific.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) processors/linux/one_net_port_specific.c -o one_net_port_specific.o

one_net.o: one_net/mac/one_net.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) one_net/mac/one_net.c -o one_net.o

one_net_client.o: one_net/mac/one_net_client.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) one_net/mac/one_net_client.c -o one_net_client.o

one_net_master.o: one_net/mac/one_net_master.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) one_net/mac/one_net_master.c -o one_net_master.o

tick.o: processors/linux/common/tick.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) processors/linux/common/tick.c -o tick.o

dummy_client_app_functions.o: applications/dummy/dummy_client_application.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) applications/dummy/dummy_client_application.c -o dummy_client_app_functions.o

dummy_master_app_functions.o: applications/dummy/dummy_master_application.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) applications/dummy/dummy_master_application.c -o dummy_master_app_functions.o

dummy_one_net_app_functions.o: applications/dummy/dummy_one_net_application.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) applications/dummy/dummy_one_net_application.c -o dummy_one_net_app_functions.o

dummy_transceiver_functions.o: applications/dummy/dummy_transceiver_functions.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) applications/dummy/dummy_transceiver_functions.c -o dummy_transceiver_functions.o



libonenetlib.a: $(ONE_NET_LIB_OBJS)
	ar rv libonenetlib.a $(ONE_NET_LIB_OBJS)
	rm -f $(ONE_NET_LIB_OBJS)



DESKTOP_PARSER_PATH = -Iapplications/desktop_sniffer/desktop

DESKTOP_PARSER_OBJS = cpp_attribute.o cpp_chip_connection.o cpp_cli.o cpp_filter.o cpp_main.o cpp_packet.o cpp_string_utils.o cpp_time_utils.o cpp_xtea_key.o

desktop_parser: $(DESKTOP_PARSER_OBJS) libonenetlib.a
	g++ $(CPPFLAGS) $(ONE_NET_LIB_PATH) $(DESKTOP_PARSER_PATH) $(DESKTOP_PARSER_OBJS) -L. -lonenetlib -o desktop_parser

cpp_attribute.o:
	g++ -c $(CPPFLAGS) $(ONE_NET_LIB_PATH) $(DESKTOP_PARSER_PATH) applications/desktop_sniffer/desktop/attribute.cpp -o cpp_attribute.o

cpp_chip_connection.o:
	g++ -c $(CPPFLAGS) $(ONE_NET_LIB_PATH) $(DESKTOP_PARSER_PATH) applications/desktop_sniffer/desktop/chip_connection.cpp -o cpp_chip_connection.o

cpp_cli.o:
	g++ -c $(CPPFLAGS) $(ONE_NET_LIB_PATH) $(DESKTOP_PARSER_PATH) applications/desktop_sniffer/desktop/cli.cpp -o cpp_cli.o

cpp_filter.o:
	g++ -c $(CPPFLAGS) $(ONE_NET_LIB_PATH) $(DESKTOP_PARSER_PATH) applications/desktop_sniffer/desktop/filter.cpp -o cpp_filter.o

cpp_main.o:
	g++ -c $(CPPFLAGS) $(ONE_NET_LIB_PATH) $(DESKTOP_PARSER_PATH) applications/desktop_sniffer/desktop/main.cpp -o cpp_main.o

cpp_packet.o:
	g++ -c $(CPPFLAGS) $(ONE_NET_LIB_PATH) $(DESKTOP_PARSER_PATH) applications/desktop_sniffer/desktop/packet.cpp -o cpp_packet.o

cpp_string_utils.o:
	g++ -c $(CPPFLAGS) $(ONE_NET_LIB_PATH) $(DESKTOP_PARSER_PATH) applications/desktop_sniffer/desktop/string_utils.cpp -o cpp_string_utils.o

cpp_time_utils.o:
	g++ -c $(CPPFLAGS) $(ONE_NET_LIB_PATH) $(DESKTOP_PARSER_PATH) applications/desktop_sniffer/desktop/time_utils.cpp -o cpp_time_utils.o

cpp_xtea_key.o:
	g++ -c $(CPPFLAGS) $(ONE_NET_LIB_PATH) $(DESKTOP_PARSER_PATH) applications/desktop_sniffer/desktop/xtea_key.cpp -o cpp_xtea_key.o



clean:
	rm -f $(ONE_NET_LIB_OBJS) $(DESKTOP_PARSER_OBJS) libonenetlib.a desktop_parser
