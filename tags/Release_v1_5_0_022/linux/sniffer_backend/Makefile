# Makefile for compiling sniffer_backend programs

#VARIABLES
CC = gcc

ONE_NET_MAC = ../../one_net/mac
ONE_NET_APP = ../../one_net/app
ONE_NET_INC = ../../processors/renesas/src/common
ONE_NET_UTIL = ../../one_net/utility
ONE_NET_PORT_SPEC = ../../one_net/port_specific
ONE_NET_PROC_LINUX_SRC_EVAL = ../../processors/linux/src/eval
ONE_NET_PROC_LINUX_SRC_COMMON = ../../processors/linux/src/common

TH_DEP = .th_dep
GEN_DEP = $(CC) -E -MM

INCS = -I. -I$(ONE_NET_MAC) -I$(ONE_NET_INC) -I$(ONE_NET_UTIL) -I$(ONE_NET_PORT_SPEC) \
-I$(ONE_NET_PROC_LINUX_SRC_EVAL) -I$(ONE_NET_PROC_LINUX_SRC_COMMON) 

#OBJS
ONE_NET_OBJS =	$(ONE_NET_UTIL)/one_net_xtea.o $(ONE_NET_MAC)/crc.o \
				$(ONE_NET_MAC)/prand.o $(ONE_NET_MAC)/one_net.o \
                $(ONE_NET_MAC)/one_net_master.o \
				$(ONE_NET_MAC)/one_net_client.o
SNIFFER_BACKEND_OBJS = sniffer_backend.o sniffer_raw.o
TIME_OBJS = th_time.o
DEBUG_OBJS = debug.o
SHM_OBJS = rf_shm.o
CHAT_OBJS = chat.o master_chat.o client_chat.o $(ONE_NET_OBJS) \
            one_net_port_specific.o $(DEBUG_OBJS) $(SHM_OBJS)

#FLAGS
CFLAGS = $(INCS) -g -W -Wno-unused-parameter
#CFLAGS = $(INCS) -g -W -Wall -Wno-unused-parameter -Werror

#TARGETS
all: sniffer_backend sniffer_raw $(SNIFFER_BACKEND_OBJS)

-include $(TH_DEP)

one_net_port_specific.o:
	$(CC) $(CFLAGS) -c -o $@ $(ONE_NET_PROC_LINUX_SRC_COMMON)/one_net_port_specific.c

one_net_crc.o:
	$(CC) $(CFLAGS) -c -o $@ $(ONE_NET_UTIL)/one_net_crc.c

one_net_encode.o:
	$(CC) $(CFLAGS) -c -o $@ $(ONE_NET_UTIL)/one_net_encode.c

one_net_xtea.o:
	$(CC) $(CFLAGS) -c -o $@ $(ONE_NET_UTIL)/one_net_xtea.c

#sniffer_backend: sniffer_backend.o one_net_encode.o one_net_xtea.o 
#	$(CC) $(CFLAGS) -o $@ sniffer_backend.o one_net_encode.o one_net_xtea.o 
sniffer_backend: sniffer_backend.o 
	$(CC) $(CFLAGS) -o $@ sniffer_backend.o 

sniffer_raw: sniffer_raw.o one_net_encode.o one_net_xtea.o one_net_crc.o
	$(CC) $(CFLAGS) -o $@ sniffer_raw.o one_net_encode.o one_net_xtea.o one_net_crc.o

$(SNIFFER_BACKEND_OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

th_dep:
#	if [ ! -f one_net_mh_repeater.c ]; then \
#    	ln -s $(ONE_NET_MAC)/one_net.c one_net_mh_repeater.c; fi
#	if [ ! -f one_net_client_mh_repeater.c ]; then \
#    	ln -s $(ONE_NET_MAC)/one_net_client.c one_net_client_mh_repeater.c; fi
#	if [ ! -f mh_hear_c1_only_onps.c ]; then \
#    	ln -s one_net_port_specific.c mh_hear_c1_only_onps.c; fi
#	if [ ! -f mh_not_hear_c2_onps.c ]; then \
#    	ln -s one_net_port_specific.c mh_not_hear_c2_onps.c; fi
#	$(GEN_DEP) $(INCS) *.c > $(TH_DEP)

clean:
	rm -f sniffer_backend sniffer_raw *.o
