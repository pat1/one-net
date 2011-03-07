################################################################
#
# A Makefile for ONE_NET/processors/linux
#
#
#
################################################################

EXEC = ona_voltage_test one_net_application_test gen_data_flash_test

CFLAGS=-I./ -I../../../../one_net/mac/ -I../../../../one_net/app/ -I../common/ \
-I../../../../one_net/utility/ -I../../../../one_net/port_specific/ \
-I../../../../processors/linux/src/eval/ -I../../../../processors/renesas/src/common/ \
-I../../../../common/ -DCUNIT_TESTING -D_ONE_NET_DEBUG -Wall -Werror -g

# Tests
TEST_TARGETS = test_ona_voltage test_one_net_application test_gen_data_flash

# Objects
ONA_VOLTAGE_TEST_OBJS = ona_voltage_test.o ona_voltage.o one_net_port_specific.o rf_shm.o debug.o th_time.o
ONE_NET_APPLICATION_TEST_OBJS = one_net_application_test.o
GEN_DATA_FLASH_TEST_OBJS = gen_data_flash_test.o flash_sim.o dfi.o dfi_port_specific.o

#
# Object Targets
#

ona_voltage.o:
	gcc -c $(CFLAGS) ../../../../one_net/app/ona_voltage.c -o $@

one_net_port_specific.o:
	gcc -c $(CFLAGS) ../../../../processors/linux/src/common/one_net_port_specific.c -o $@

rf_shm.o:
	gcc -c $(CFLAGS) ../../../../processors/linux/src/common/rf_shm.c -o $@ 
debug.o:
	gcc -c $(CFLAGS) ../../../../processors/linux/src/common/debug.c -o $@

th_time.o:
	gcc -c $(CFLAGS) ../../../../processors/linux/src/common/th_time.c -o $@

client_port_specific.o:
	gcc -c $(CFLAGS) ../../../processors/renesas/src/common/client_port_specific.c -o $@

dfi.o:
	gcc -c $(CFLAGS) ../../../../common/dfi.c -o $@

dfi_port_specific.o:
	gcc -c $(CFLAGS) ../../../../processors/linux/src/common/dfi_port_specific.c -o $@


#
# Test program Targets
#
ona_voltage_test: $(ONA_VOLTAGE_TEST_OBJS)
	gcc $(CFLAGS) $(ONA_VOLTAGE_TEST_OBJS) -o $@ -lcunit

one_net_application_test: $(ONE_NET_APPLICATION_TEST_OBJS)
	gcc $(CFLAGS) $(ONE_NET_APPLICATION_TEST_OBJS) -o $@ -lcunit

gen_data_flash_test: $(GEN_DATA_FLASH_TEST_OBJS)
	gcc $(CFLAGS) $(GEN_DATA_FLASH_TEST_OBJS) -o $@ -lcunit


#
# Test targets
#
test_ona_voltage: ona_voltage_test
	./ona_voltage_test

test_one_net_application: one_net_application_test
	./one_net_application_test

test_gen_data_flash: gen_data_flash_test
	./gen_data_flash_test


test: clean $(TEST_TARGETS)


clean:
	rm -f *.o $(EXEC)
