# Makefile for compiling ONE_NET simulation for FC5 environment

#VARIABLES
CC = gcc

ONE_NET_MAC = ../../one_net/mac
ONE_NET_APP = ../../one_net/app

TH_DEP = .th_dep
GEN_DEP = $(CC) -E -MM

INCS = -I. -I$(ONE_NET_MAC) -I$(ONE_NET_APP)

#OBJS
ONE_NET_MAC_UTIL_OBJS = $(ONE_NET_MAC)/crc.o $(ONE_NET_MAC)/prand.o \
  $(ONE_NET_MAC)/xtea.o
ONE_NET_MASTER_MAC_OBJS = $(ONE_NET_MAC_UTIL_OBJS) mh_one_net.o \
  $(ONE_NET_MAC)/one_net_master.o
ONE_NET_CLIENT_MAC_OBJS = $(ONE_NET_MAC_UTIL_OBJS) one_net.o \
  $(ONE_NET_MAC)/one_net_client.o
ONE_NET_APP_OBJS := $(patsubst %.c, %.o, $(wildcard $(ONE_NET_APP)/*.c))

DEV_LINKS_OBJS = dev_link.o
INVITE_OBJS = invite.o
STR_TABLE_OBJS = sim_str_table.o
STRING_OBJS = strlcpy.o
SHM_OBJS = rf_shm.o
TIME_OBJS = th_time.o

LOG_OBJS = sim_log.o $(STRING_OBJS)

CLICK_OBJS = click.o $(TIME_OBJS) $(SHM_OBJS)
MASTER_SIM_OBJS = master_sim.o one_net_port_specific.o master_specific.o \
  master_sim_ona_hdlr.o $(STR_TABLE_OBJS) $(LOG_OBJS) $(SHM_OBJS) \
  $(ONE_NET_MASTER_MAC_OBJS) $(ONE_NET_APP_OBJS) $(DEV_LINKS_OBJS) \
  $(INVITE_OBJS)
CLIENT_SIM_OBJS = client_sim.o one_net_port_specific.o $(STR_TABLE_OBJS) \
  $(LOG_OBJS) $(SHM_OBJS) $(ONE_NET_CLIENT_MAC_OBJS) $(ONE_NET_APP_OBS) \
  $(DEV_LINK_OBJS) $(INVITE_OBJS)

#FLAGS
CFLAGS = $(INCS) -g -W -Wall -Wno-unused-parameter -Werror
CFLAGS += -DFC5

SIMPLE_CLIENT_FLAG = -DSIMPLE_DEV
MH_FLAG = -DMULTI_HOP
MH_REPEATER = $(MH_FLAG) -DMH_CLIENT_REPEATER
MASTER_FLAGS = -DMASTER $(MH_FLAG)

#TARGETS
all: str th_dep click master_sim client_sim

-include $(TH_DEP)

click: $(CLICK_OBJS)
	$(CC) $(CFLAGS) -o $@ $(CLICK_OBJS)

master_sim: CFLAGS += $(MASTER_FLAGS)
master_sim: $(MASTER_SIM_OBJS)
	$(CC) $(CFLAGS) -o $@ $(MASTER_SIM_OBJS)

client_sim: $(CLIENT_SIM_OBJS)
	$(CC) $(CFLAGS) -o $@ $(CLIENT_SIM_OBJS)

str:
	if [ ! -f strlstring.h ]; then \
		ln -s ../../../homeserver/snapgear/user/th_lib/strlstring.h; fi
	if [ ! -f strlcpy.c ]; then \
		ln -s ../../../homeserver/snapgear/user/th_lib/strlcpy.c; fi

th_dep:
	if [ ! -f mh_one_net.c ]; then \
    	ln -s $(ONE_NET_MAC)/one_net.c mh_one_net.c; fi
	$(GEN_DEP) $(INCS) *.c > $(TH_DEP)

clean:
	rm -f .th_dep click master_sim client_sim $(CLICK_OBJS) $(MASTER_SIM_OBJS)
