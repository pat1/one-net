/*!
    \file ona_voltage_test.c
    \brief Unit tests the varoius voltage message parse and send utility functions.

    The payload of the voltage messages does not follow the normal format. Instead of 
    containing a byte with the source and destination units and a 16 bit data value, 
    there are 24 bits of data (2 bit voltage source, 11 bit battery voltage in volts,
    and 11 bit external voltage in volts, See the ONE-NET Device Payload Format 
    specification for more information on the format of the voltage message types
    (ONA_VOLTAGE_VOLTS, ONA_VOLTAGE_10THS_VOLTS, ONA_VOLTAGE_100THS_VOLTS).

    \author Roger
    \note Threshold Corporation
*/

#include <stdio.h>
// #include <sys/types.h>
// #include <time.h>
// #include <unistd.h>

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <CUnit/Automated.h>

#include "one_net.h"
#include "one_net_application.h"
#include "ona_voltage.h"

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
/*! \brief Test ona_voltage.[ch] functions.


\param void.
\return void.
*/

// this variable is referenced by one_net_write in file one_net_port_specific.c
// so we are defining it here to avoid having to link other modules we don't 
// really need for this test.
on_base_param_t base_param;

UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN];

UInt8 payload_sent[ONE_NET_RAW_SINGLE_DATA_LEN * 2];

UInt8 voltage_source_1;
UInt16 battery_voltage_1, external_voltage_1;
UInt8 test_send_case_1[ONE_NET_RAW_SINGLE_DATA_LEN] = {
    0x40, 0x00, 0x13, 0x28, 0x01
};

UInt8 voltage_source_2;
UInt16 battery_voltage_2, external_voltage_2;
UInt8 test_send_case_2[ONE_NET_RAW_SINGLE_DATA_LEN] = {
    0x81, 0x00, 0x14, 0x28, 0x08
};

UInt8 voltage_source_3;
UInt16 battery_voltage_3, external_voltage_3;
UInt8 test_send_case_3[ONE_NET_RAW_SINGLE_DATA_LEN] = {
    0x8e, 0x00, 0x15, 0x10, 0x32
};

UInt8 voltage_source_4;
UInt16 battery_voltage_4, external_voltage_4;
UInt8 test_send_case_4[ONE_NET_RAW_SINGLE_DATA_LEN] = {
    0x81, 0x00, 0x14, 0x38, 0x00
};

const one_net_raw_did_t raw_did = {0x00, 0x02};

one_net_send_single_func_t one_net_send_single;

one_net_status_t fake_one_net_client_send_single(UInt8 *data,
  UInt8 DATA_LEN, const BOOL send_to_peer_list, UInt8 PRIORITY,
  const one_net_raw_did_t *RAW_DST, UInt8 SRC_UNIT,
  tick_t* send_time_from_now, tick_t* expire_time_from_now)
{
    one_net_status_t status;

    // this fake send will just copy the payload to a string where
    // a test program can exmine it.
    memcpy(payload_sent, data, DATA_LEN);

    status = ONS_SUCCESS;
    return status;
    
} // fake_one_net_client_send_single //
    
void test_send_voltage(void)
{
    one_net_status_t status;
    int result;

    // set it up so that we get called instead of ONE-NET to send a message
    one_net_send_single = fake_one_net_client_send_single;

    //
    // test_send_case_1
    //
    voltage_source_1 = ONA_VOLTAGE_EXTERNAL_STATUS /*0x40*/;
    battery_voltage_1 = 5;
    external_voltage_1 = 1;

    status = ona_send_voltage_volts_status(voltage_source_1, battery_voltage_1,
      external_voltage_1, &raw_did);

    CU_ASSERT_EQUAL(status, ONS_SUCCESS);
    if ( status != ONS_SUCCESS )
    {
        printf("%s: line=%d: ona_send_voltage_volts_status failed, status=%d\n\n", __FUNCTION__, 
          __LINE__, status);
    } 
    
    result = memcmp(test_send_case_1, payload_sent, ONE_NET_RAW_SINGLE_DATA_LEN);
    CU_ASSERT_EQUAL(result, 0);
    if (result != 0)
    {
        printf("%s: ona_send_voltage_volts_status: line=%d: memcmp != 0, result=%d,\n"
          "  status=0x%02x, bat=%d, ext=%d\n"
          "   payload_sent    =0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n"
          "   test_send_case_1=0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n\n", __FUNCTION__,
          __LINE__, result, voltage_source_1, battery_voltage_1, external_voltage_1, 
          payload_sent[0], payload_sent[1], payload_sent[2], payload_sent[3], payload_sent[4],
          test_send_case_1[0], test_send_case_1[1], test_send_case_1[2], test_send_case_1[3],
          test_send_case_1[4]);
    }
    
    //
    // test_send_case_2
    //
    voltage_source_2 = ONA_VOLTAGE_BATTERY_STATUS /*0x80*/;
    battery_voltage_2 = 37;
    external_voltage_2 = 8;

    status = ona_send_voltage_10ths_volts(voltage_source_2, battery_voltage_2,
      external_voltage_2, &raw_did);

    CU_ASSERT_EQUAL(status, ONS_SUCCESS);
    if ( status != ONS_SUCCESS )
    {
        printf("%s: line=%d: ona_send_voltage_10ths_volts failed, status=%d\n\n", __FUNCTION__, 
          __LINE__, status);
    } 
    
    result = memcmp(test_send_case_2, payload_sent, ONE_NET_RAW_SINGLE_DATA_LEN);
    CU_ASSERT_EQUAL(result, 0);
    if (result != 0)
    {
        printf("%s: ona_send_voltage_10ths_volts: line=%d: memcmp != 0, result=%d,\n"
          "  status=0x%02x, bat=%d, ext=%d\n"
          "   payload_sent    =0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n"
          "   test_send_case_2=0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n\n", __FUNCTION__,
          __LINE__, result, voltage_source_2, battery_voltage_2, external_voltage_2, 
          payload_sent[0], payload_sent[1], payload_sent[2], payload_sent[3], payload_sent[4],
          test_send_case_2[0], test_send_case_2[1], test_send_case_2[2], test_send_case_2[3],
          test_send_case_2[4]);
    }

    //
    // try parsing the last send case 
    //
    status = ona_parse_voltage_10ths_volts(test_send_case_2, sizeof(test_send_case_1),
      &voltage_source_2, &battery_voltage_2, &external_voltage_2);

    CU_ASSERT_EQUAL(status, ONS_SUCCESS);
    if ( status != ONS_SUCCESS )
    {
        printf("%s: line=%d: ona_parse_voltage_10ths_volts_status failed, status=%d\n\n", __FUNCTION__, 
          __LINE__, status);
    } 
    
    result = (voltage_source_2 == ONA_VOLTAGE_BATTERY_STATUS /*0x80*/);
    CU_ASSERT_EQUAL(result, 1);
    if (result != 1)
    {
        printf("%s: test_parse_case_1: line=%d: voltage_source_1(%d) != (%d),\n"
          , __FUNCTION__, __LINE__, voltage_source_2, ONA_VOLTAGE_BATTERY_STATUS);
    }
    
    result = (battery_voltage_2 == 37);
    CU_ASSERT_EQUAL(result, 1);
    if (result != 1)
    {
        printf("%s: test_parse_case_1: line=%d: battery_voltage_2(%d) != (%d),\n"
          , __FUNCTION__, __LINE__, battery_voltage_2, 37);
    }

    result = (external_voltage_2 == 8);
    CU_ASSERT_EQUAL(result, 1);
    if (result != 1)
    {
        printf("%s: test_parse_case_1: line=%d: battery_voltage_2(%d) != (%d),\n"
          , __FUNCTION__, __LINE__, external_voltage_2, 1);
    }

    //
    // test_send_case_3
    //
    voltage_source_3 = ONA_VOLTAGE_BATTERY_STATUS /*0x80*/;
    battery_voltage_3 = 450;
    external_voltage_3 = 50;

    status = ona_send_voltage_100ths_volts(voltage_source_3, battery_voltage_3,
      external_voltage_3, &raw_did);

    CU_ASSERT_EQUAL(status, ONS_SUCCESS);
    if ( status != ONS_SUCCESS )
    {
        printf("%s: line=%d: ona_send_voltage_100ths_voltsfailed, status=%d\n\n", __FUNCTION__, 
          __LINE__, status);
    } 
    
    result = memcmp(test_send_case_3, payload_sent, ONE_NET_RAW_SINGLE_DATA_LEN);
    CU_ASSERT_EQUAL(result, 0);
    if (result != 0)
    {
        printf("%s: ona_send_voltage_100ths_volts: line=%d: memcmp != 0, result=%d,\n"
          "  status=0x%02x, bat=%d, ext=%d\n"
          "   payload_sent    =0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n"
          "   test_send_case_3=0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n\n", __FUNCTION__,
          __LINE__, result, voltage_source_3, battery_voltage_3, external_voltage_3, 
          payload_sent[0], payload_sent[1], payload_sent[2], payload_sent[3], payload_sent[4],
          test_send_case_3[0], test_send_case_3[1], test_send_case_3[2], test_send_case_3[3],
          test_send_case_3[4]);
    }
    
} // test_send_voltage //
    
void test_parse_voltage(void)
{
    one_net_status_t status;
    int result;

    //
    // test_parse_case_1
    //
    status = ona_parse_voltage_volts(test_send_case_1, sizeof(test_send_case_1),
      &voltage_source_1, &battery_voltage_1, &external_voltage_1);

    CU_ASSERT_EQUAL(status, ONS_SUCCESS);
    if ( status != ONS_SUCCESS )
    {
        printf("%s: line=%d: ona_parse_voltage_volts_status failed, status=%d\n\n", __FUNCTION__, 
          __LINE__, status);
    } 
    
    result = (voltage_source_1 == ONA_VOLTAGE_EXTERNAL_STATUS /*0x40*/);
    CU_ASSERT_EQUAL(result, 1);
    if (result != 1)
    {
        printf("%s: test_parse_case_1: line=%d: voltage_source_1(%d) != (%d),\n"
          , __FUNCTION__, __LINE__, voltage_source_1, ONA_VOLTAGE_EXTERNAL_STATUS);
    }
    
    result = (battery_voltage_1 == 5);
    CU_ASSERT_EQUAL(result, 1);
    if (result != 1)
    {
        printf("%s: test_parse_case_1: line=%d: battery_voltage_1(%d) != (%d),\n"
          , __FUNCTION__, __LINE__, battery_voltage_1, 5);
    }

    result = (external_voltage_1 == 1);
    CU_ASSERT_EQUAL(result, 1);
    if (result != 1)
    {
        printf("%s: test_parse_case_1: line=%d: battery_voltage_1(%d) != (%d),\n"
          , __FUNCTION__, __LINE__, external_voltage_1, 1);
    }

    //
    // test_parse_case_2
    //
    battery_voltage_2 = 37;
    external_voltage_2 = 1;

    status = ona_parse_voltage_volts(test_send_case_2, sizeof(test_send_case_2),
      &voltage_source_2, &battery_voltage_2, &external_voltage_2);

    CU_ASSERT_EQUAL(status, ONS_SUCCESS);
    if ( status != ONS_SUCCESS )
    {
        printf("%s: line=%d: ona_parse_voltage_volts_status failed, status=%d\n\n", __FUNCTION__, 
          __LINE__, status);
    } 
    
    result = (voltage_source_2 == ONA_VOLTAGE_BATTERY_STATUS /*0x80*/);
    CU_ASSERT_EQUAL(result, 1);
    if (result != 1)
    {
        printf("%s: test_parse_case_2: line=%d: voltage_source_2(%d) != (%d),\n"
          , __FUNCTION__, __LINE__, voltage_source_2, ONA_VOLTAGE_BATTERY_STATUS);
    }
    
    result = (battery_voltage_2 == 37);
    CU_ASSERT_EQUAL(result, 1);
    if (result != 1)
    {
        printf("%s: test_parse_case_2: line=%d: battery_voltage_2(%d) != (%d),\n"
          , __FUNCTION__, __LINE__, battery_voltage_2, 37);
    }

    result = (external_voltage_2 == 8);
    CU_ASSERT_EQUAL(result, 1);
    if (result != 1)
    {
        printf("%s: test_parse_case_2: line=%d: battery_voltage_2(%d) != (%d),\n"
          , __FUNCTION__, __LINE__, external_voltage_2, 8);
    }

    //
    // test_parse_case_3
    //

    status = ona_parse_voltage_100ths_volts(test_send_case_3, sizeof(test_send_case_3),
      &voltage_source_3, &battery_voltage_3, &external_voltage_3);

    CU_ASSERT_EQUAL(status, ONS_SUCCESS);
    if ( status != ONS_SUCCESS )
    {
        printf("%s: line=%d: ona_parse_voltage_100ths_voltsfailed, status=%d\n\n", __FUNCTION__, 
          __LINE__, status);
    } 
    
    result = (voltage_source_3 == ONA_VOLTAGE_BATTERY_STATUS /*0x80*/);
    CU_ASSERT_EQUAL(result, 1);
    if (result != 1)
    {
        printf("%s: test_parse_case_3: line=%d: voltage_source_3(%d) != (%d),\n"
          , __FUNCTION__, __LINE__, voltage_source_3, ONA_VOLTAGE_EXTERNAL_STATUS);
    }
    
    result = (battery_voltage_3 == 450);
    CU_ASSERT_EQUAL(result, 1);
    if (result != 1)
    {
        printf("%s: test_parse_case_3: line=%d: battery_voltage_3(%d) != (%d),\n"
          , __FUNCTION__, __LINE__, battery_voltage_3, 5);
    }

    result = (external_voltage_3 == 50);
    CU_ASSERT_EQUAL(result, 1);
    if (result != 1)
    {
        printf("%s: test_parse_case_3: line=%d: battery_voltage_3(%d) != (%d),\n"
          , __FUNCTION__, __LINE__, external_voltage_3, 1);
    }

    //
    // test_parse_case_4
    //

    status = ona_parse_voltage_10ths_volts(test_send_case_4, sizeof(test_send_case_4),
      &voltage_source_4, &battery_voltage_4, &external_voltage_4);

    CU_ASSERT_EQUAL(status, ONS_SUCCESS);
    if ( status != ONS_SUCCESS )
    {
        printf("%s: line=%d: ona_parse_voltage_10ths_voltsfailed, status=%d\n\n", __FUNCTION__, 
          __LINE__, status);
    } 
    
    result = (voltage_source_4 == ONA_VOLTAGE_BATTERY_STATUS /*0x80*/);
    CU_ASSERT_EQUAL(result, 1);
    if (result != 1)
    {
        printf("%s: test_parse_case_4: line=%d: voltage_source_4(%d) != (%d),\n"
          , __FUNCTION__, __LINE__, voltage_source_4, ONA_VOLTAGE_EXTERNAL_STATUS);
    }
    
    result = (battery_voltage_4 == 39);
    CU_ASSERT_EQUAL(result, 1);
    if (result != 1)
    {
        printf("%s: test_parse_case_4: line=%d: battery_voltage_4(%d) != (%d),\n"
          , __FUNCTION__, __LINE__, battery_voltage_4, 39);
    }

    result = (external_voltage_4 == 0);
    CU_ASSERT_EQUAL(result, 1);
    if (result != 1)
    {
        printf("%s: test_parse_case_4: line=%d: battery_voltage_4(%d) != (%d),\n"
          , __FUNCTION__, __LINE__, external_voltage_4, 1);
    }

} // test_parse_voltage //
    

int init_suite( void )
{
    return 0;
}

int clean_suite( void )
{
    return 0;
}

   
int main(int argc, char * argv[])
{
    CU_pSuite pSuite = NULL;

#if CUNIT_EXPERIMENTAL
    printf("%s: starting\n", __FILE__);
#endif

    if(CU_initialize_registry() != CUE_SUCCESS)
    {
        return CU_get_error();
    }
 
    pSuite = CU_add_suite("ONE-NET ona_voltage test suite",
     init_suite, clean_suite);
    if(pSuite == NULL)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if((CU_add_test(pSuite, "test ona_send_voltage", test_send_voltage) == NULL)
     || (CU_add_test(pSuite, "test ona_parse_voltage", test_parse_voltage) == NULL))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }
 
#ifndef CUNIT_EXPERIMENTAL
    //
    // normal (old) style CUnit running and reporting output.
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    // must calc. this before the registry cleanup
    const int num_failed = CU_get_number_of_tests_failed();
#else // CUNIT_EXPERIMENTAL
    //
    // trying a different style of CUnit output, one that separates
    // CUnit reporting output from printf's used to show what happended
    // when asserts fail.
    //
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_automated_run_tests();

    // must calc. this before the registry cleanup
    const int num_failed = CU_get_number_of_failures();
    const int num_asserts = CU_get_number_of_asserts();
    printf("%s: ending, with %d failures out of %d asserts\n\n", __FILE__,
      num_failures, num_asserts);
     
    CU_basic_show_failures(CU_get_failure_list());
#endif
    CU_cleanup_registry(); 

    // return a non zero (failure) if any test failed or if a CUnit error occured
    return num_failed | CU_get_error();
}

