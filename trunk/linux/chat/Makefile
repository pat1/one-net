# Makefile for compiling ONE_NET code for FC5 environment

#VARIABLES
CC = gcc

ONE_NET_MAC = ../../one_net/mac
ONE_NET_APP = ../../one_net/app

TH_DEP = .th_dep
GEN_DEP = $(CC) -E -MM

INCS = -I. -I$(ONE_NET_MAC)

#OBJS
ONE_NET_OBJS =	$(ONE_NET_MAC)/xtea.o $(ONE_NET_MAC)/crc.o \
				$(ONE_NET_MAC)/prand.o $(ONE_NET_MAC)/one_net.o \
                $(ONE_NET_MAC)/one_net_master.o \
				$(ONE_NET_MAC)/one_net_client.o
ON_MH_REPEATER_OBJS =	$(ONE_NET_MAC)/xtea.o $(ONE_NET_MAC)/crc.o \
                        $(ONE_NET_MAC)/prand.o $(ONE_NET_MAC)/one_net_master.o \
                        one_net_mh_repeater.o one_net_client_mh_repeater.o \
                        one_net_port_specific.o
ON_HEAR_C1_ONLY_OBJS =	$(ONE_NET_OBJS) mh_hear_c1_only_onps.o
ON_CAN_NOT_HEAR_C2_OBJS =	$(ONE_NET_OBJS) mh_not_hear_c2_onps.o
ONE_NET_APP_OBJS := $(patsubst %.c, %.o, $(wildcard $(ONE_NET_APP)/*.c))
TIME_OBJS = th_time.o
DEBUG_OBJS = debug.o
SHM_OBJS = rf_shm.o
CHAT_OBJS = chat.o master_chat.o client_chat.o $(ONE_NET_OBJS) \
            one_net_port_specific.o $(DEBUG_OBJS) $(SHM_OBJS)
MH_REPEATER_OBJS = chat.o master_chat.o client_chat.o \
            $(ON_MH_REPEATER_OBJS) $(DEBUG_OBJS) $(SHM_OBJS)
HEAR_C1_ONLY_OBJS = chat.o master_chat.o client_chat.o \
            $(ON_HEAR_C1_ONLY_OBJS) $(DEBUG_OBJS) $(SHM_OBJS)
CAN_NOT_HEAR_C2_OBJS = chat.o master_chat.o client_chat.o \
            $(ON_CAN_NOT_HEAR_C2_OBJS) $(DEBUG_OBJS) $(SHM_OBJS)
CLICK_OBJS = click.o $(TIME_OBJS) $(SHM_OBJS) $(DEBUG_OBJS)

#FLAGS
CFLAGS = $(INCS) -g -W -Wall -Wno-unused-parameter -Werror
CFLAGS += -DFC5 -DMULTI_HOP #-DSIMPLE_DEV #enable print_debugs in ONE_NET code
#TARGETS
all: th_dep chat mh_client_repeater hear_c1_only can_not_hear_c2 click $(ONE_NET_APP_OBJS)

-include $(TH_DEP)

chat: $(CHAT_OBJS) 
	$(CC) $(CFLAGS) -o $@ $(CHAT_OBJS)

mh_client_repeater: CFLAGS += -DMH_CLIENT_REPEATER
mh_client_repeater: $(MH_REPEATER_OBJS) 
	$(CC) $(CFLAGS) -o $@ $(MH_REPEATER_OBJS)

hear_c1_only: CFLAGS += -DHEAR_C1_ONLY
hear_c1_only: $(HEAR_C1_ONLY_OBJS)
	$(CC) $(CFLAGS) -o $@ $(HEAR_C1_ONLY_OBJS)

can_not_hear_c2: CFLAGS += -DCAN_NOT_HEAR_C2
can_not_hear_c2: $(CAN_NOT_HEAR_C2_OBJS)
	$(CC) $(CFLAGS) -o $@ $(CAN_NOT_HEAR_C2_OBJS)

click: $(CLICK_OBJS)
	$(CC) $(CFLAGS) -o $@ $(CLICK_OBJS)

$(ONE_NET_APP_OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

th_dep:
	if [ ! -f one_net_mh_repeater.c ]; then \
    	ln -s $(ONE_NET_MAC)/one_net.c one_net_mh_repeater.c; fi
	if [ ! -f one_net_client_mh_repeater.c ]; then \
    	ln -s $(ONE_NET_MAC)/one_net_client.c one_net_client_mh_repeater.c; fi
	if [ ! -f mh_hear_c1_only_onps.c ]; then \
    	ln -s one_net_port_specific.c mh_hear_c1_only_onps.c; fi
	if [ ! -f mh_not_hear_c2_onps.c ]; then \
    	ln -s one_net_port_specific.c mh_not_hear_c2_onps.c; fi
	$(GEN_DEP) $(INCS) *.c > $(TH_DEP)

clean:
	rm -f chat mh_client_repeater hear_c1_only can_not_hear_c2 click \
    $(CHAT_OBJS) $(MH_REPEATER_OBJS) $(HEAR_C1_ONLY_OBJS) \
    $(CAN_NOT_HEAR_C2_OBJS) $(CLICK_OBJS) $(ONE_NET_APP_OBJS)
