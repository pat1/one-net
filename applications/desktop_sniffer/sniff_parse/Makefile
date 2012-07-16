all: sniff_parse libonenetlib.a

onenetlib: libonenetlib.a



CFLAGS = -Wall -Werror
CPPFLAGS = -Wall -Werror



ONE_NET_LIB_PATH = -I../../../applications/desktop_sniffer/desktop -I../../../processors/linux -I../../../processors/linux/common -I../../../one_net/app -I../../../one_net/utility -I../../../one_net/port_specific -I../../../one_net/mac -I../../../transceivers -I../../../processors/renesas/src/eval -I../../../processors/renesas/src/eval/adi

ONE_NET_LIB_OBJS = one_net_xtea.o one_net_crc.o one_net_encode.o one_net_memory.o one_net_prand.o one_net_timer.o one_net_features.o one_net_packet.o one_net_message.o one_net_peer.o one_net_application.o one_net_acknowledge.o one_net_port_specific.o one_net.o one_net_client.o one_net_master.o tick.o dummy_client_app_functions.o dummy_master_app_functions.o dummy_one_net_app_functions.o dummy_transceiver_functions.o

one_net_xtea.o: ../../../one_net/utility/one_net_xtea.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) ../../../one_net/utility/one_net_xtea.c -o one_net_xtea.o

one_net_crc.o: ../../../one_net/utility/one_net_crc.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) ../../../one_net/utility/one_net_crc.c -o one_net_crc.o

one_net_encode.o: ../../../one_net/utility/one_net_encode.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) ../../../one_net/utility/one_net_encode.c -o one_net_encode.o

one_net_memory.o: ../../../one_net/utility/one_net_memory.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) ../../../one_net/utility/one_net_memory.c -o one_net_memory.o

one_net_prand.o: ../../../one_net/utility/one_net_prand.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) ../../../one_net/utility/one_net_prand.c -o one_net_prand.o

one_net_timer.o: ../../../one_net/utility/one_net_timer.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) ../../../one_net/utility/one_net_timer.c -o one_net_timer.o

one_net_features.o: ../../../one_net/app/one_net_features.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) ../../../one_net/app/one_net_features.c -o one_net_features.o

one_net_message.o: ../../../one_net/app/one_net_message.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) ../../../one_net/app/one_net_message.c -o one_net_message.o

one_net_packet.o: ../../../one_net/app/one_net_packet.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) ../../../one_net/app/one_net_packet.c -o one_net_packet.o

one_net_peer.o: ../../../one_net/mac/one_net_peer.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) ../../../one_net/mac/one_net_peer.c -o one_net_peer.o

one_net_application.o: ../../../one_net/app/one_net_application.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) ../../../one_net/app/one_net_application.c -o one_net_application.o

one_net_acknowledge.o: ../../../one_net/app/one_net_acknowledge.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) ../../../one_net/app/one_net_acknowledge.c -o one_net_acknowledge.o

one_net_port_specific.o: ../../../processors/linux/one_net_port_specific.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) ../../../processors/linux/one_net_port_specific.c -o one_net_port_specific.o

one_net.o: ../../../one_net/mac/one_net.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) ../../../one_net/mac/one_net.c -o one_net.o

one_net_client.o: ../../../one_net/mac/one_net_client.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) ../../../one_net/mac/one_net_client.c -o one_net_client.o

one_net_master.o: ../../../one_net/mac/one_net_master.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) ../../../one_net/mac/one_net_master.c -o one_net_master.o

tick.o: ../../../processors/linux/common/tick.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) ../../../processors/linux/common/tick.c -o tick.o

dummy_client_app_functions.o: ../../../applications/dummy/dummy_client_application.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) ../../../applications/dummy/dummy_client_application.c -o dummy_client_app_functions.o

dummy_master_app_functions.o: ../../../applications/dummy/dummy_master_application.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) ../../../applications/dummy/dummy_master_application.c -o dummy_master_app_functions.o

dummy_one_net_app_functions.o: ../../../applications/dummy/dummy_one_net_application.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) ../../../applications/dummy/dummy_one_net_application.c -o dummy_one_net_app_functions.o

dummy_transceiver_functions.o: ../../../applications/dummy/dummy_transceiver_functions.c
	gcc -c $(CFLAGS) $(ONE_NET_LIB_PATH) ../../../applications/dummy/dummy_transceiver_functions.c -o dummy_transceiver_functions.o



libonenetlib.a: $(ONE_NET_LIB_OBJS)
	ar rv libonenetlib.a $(ONE_NET_LIB_OBJS)
	rm -f $(ONE_NET_LIB_OBJS)



SNIFF_PARSE_PATH = -I.

SNIFF_PARSE_OBJS = cpp_attribute.o cpp_sniff_parse.o cpp_packet.o cpp_string_utils.o cpp_xtea_key.o cpp_filter.o

sniff_parse: $(SNIFF_PARSE_OBJS) libonenetlib.a
	g++ $(CPPFLAGS) $(ONE_NET_LIB_PATH) $(SNIFF_PARSE_PATH) $(SNIFF_PARSE_OBJS) -L. -lonenetlib -o sniff_parse

cpp_attribute.o:
	g++ -c $(CPPFLAGS) $(ONE_NET_LIB_PATH) $(SNIFF_PARSE_PATH) attribute.cpp -o cpp_attribute.o

cpp_sniff_parse.o:
	g++ -c $(CPPFLAGS) $(ONE_NET_LIB_PATH) $(SNIFF_PARSE_PATH) sniff_parse.cpp -o cpp_sniff_parse.o

cpp_packet.o:
	g++ -c $(CPPFLAGS) $(ONE_NET_LIB_PATH) $(SNIFF_PARSE_PATH) on_packet.cpp -o cpp_packet.o

cpp_string_utils.o:
	g++ -c $(CPPFLAGS) $(ONE_NET_LIB_PATH) $(SNIFF_PARSE_PATH) string_utils.cpp -o cpp_string_utils.o

cpp_xtea_key.o:
	g++ -c $(CPPFLAGS) $(ONE_NET_LIB_PATH) $(SNIFF_PARSE_PATH) xtea_key.cpp -o cpp_xtea_key.o

cpp_filter.o:
	g++ -c $(CPPFLAGS) $(ONE_NET_LIB_PATH) $(SNIFF_PARSE_PATH) filter.cpp -o cpp_filter.o


clean:
	rm -f $(ONE_NET_LIB_OBJS) $(SNIFF_PARSE_OBJS) libonenetlib.a sniff_parse *.o *~
