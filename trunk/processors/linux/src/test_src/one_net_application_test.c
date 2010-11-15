/*!
    \file one_net_application_tet.c
    \brief Unit tests the varoius single packet payload put_* and get_* functions.

    The payload of the single packet messages should be constructed and parsed
    using the get_* and put_* functions defined in one_net_application.h.

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

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
/*! \brief Test one_net_application.[ch] functions.


\param void.
\return void.
*/

// this variable is referenced by one_net_write in file one_net_port_specific.c
// so we are defining it here to avoid having to link other modules we don't 
// really need for this test.
on_base_param_t base_param;

UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN];
UInt8 payload_ones[] = {0xff, 0xff, 0xff, 0xff, 0xff};

// after setting source unit to 1
UInt8 payload_src_1[] = {0x1f, 0xff, 0xff, 0xff, 0xff};

// after setting destination unit to 2
UInt8 payload_dst_2[] = {0x12, 0xff, 0xff, 0xff, 0xff};

// after setting message header (message class and type) to 0x3333
UInt8 payload_msg_hdr_3333[] = {0x12, 0x33, 0x33, 0xff, 0xff};

// after setting message data to 0x5555
UInt8 payload_msg_data_4444[] = {0x12, 0x33, 0x33, 0x44, 0x44};

// after setting message three bytes to 0x666666
UInt8 payload_msg_3bytes_556677[] = {0x55, 0x33, 0x33, 0x66, 0x77};

// data for testing get_* functions
UInt8 input_payload[] = {0x56, 0x78, 0x9a, 0xbc, 0xde};

// buffer for testing put_block_data_payload_hdr() funtion
UInt8 block_data_payload[ONE_NET_RAW_BLOCK_STREAM_DATA_LEN];

// block data payload header test case 1
UInt8 block_data_test_case_1[ONE_NET_RAW_BLOCK_STREAM_DATA_LEN] = 
  { 0x00, 0x01, 0x00, 23, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00 };
    
void test_put_functions(void)
{
    int result;
    UInt8 src_unit = 1;
    UInt8 dst_unit = 2;
    UInt16 msg_hdr = 0x3333;
    UInt16 msg_data = 0x4444;
    UInt8 msg_3bytes[] = { 0x55, 0x66, 0x77};

    //
    // test put_* functions
    //

    // initialize the paylaod buffer
    memcpy(payload, payload_ones, sizeof(payload));

    put_src_unit(src_unit, payload);
    result = memcmp(payload, payload_src_1, sizeof(payload));
    CU_ASSERT_EQUAL(result, 0);
    if (result != 0)
    {
        printf("%s: put_src_unit(%d, *) ERROR: line=%d: memcmp != 0, result=%d,\n"
          "   payload         =0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n"
          "   payload_src_1   =0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n\n",
          __FUNCTION__, src_unit, __LINE__, result, 
          payload[0], payload[1], payload[2], payload[3], payload[4],
          payload_src_1[0], payload_src_1[1], payload_src_1[2],
          payload_src_1[3], payload_src_1[4]);
    }
    
    put_dst_unit(dst_unit, payload);
    result = memcmp(payload, payload_dst_2, sizeof(payload));
    CU_ASSERT_EQUAL(result, 0);
    if (result != 0)
    {
        printf("%s: put_dst_unit(%d, *) ERROR: line=%d: memcmp != 0, result=%d,\n"
          "   payload         =0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n"
          "   payload_dst_2   =0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n\n",
          __FUNCTION__, src_unit, __LINE__, result, 
          payload[0], payload[1], payload[2], payload[3], payload[4],
          payload_dst_2[0], payload_dst_2[1], payload_dst_2[2],
          payload_dst_2[3], payload_dst_2[4]);
    }
    
    put_msg_hdr(msg_hdr, payload);
    result = memcmp(payload, payload_msg_hdr_3333, sizeof(payload));
    CU_ASSERT_EQUAL(result, 0);
    if (result != 0)
    {
        printf("%s: put_msg_hdr_3333(%d, *) ERROR: line=%d: memcmp != 0, result=%d,\n"
          "   payload                =0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n"
          "   payload_msg_hdr_3333   =0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n\n",
          __FUNCTION__, src_unit, __LINE__, result, 
          payload[0], payload[1], payload[2], payload[3], payload[4],
          payload_msg_hdr_3333[0], payload_msg_hdr_3333[1], payload_msg_hdr_3333[2],
          payload_msg_hdr_3333[3], payload_msg_hdr_3333[4]);
    }
    
    put_msg_data(msg_data, payload);
    result = memcmp(payload, payload_msg_data_4444, sizeof(payload));
    CU_ASSERT_EQUAL(result, 0);
    if (result != 0)
    {
        printf("%s: put_msg_hdr_3333(%d, *) ERROR: line=%d: memcmp != 0, result=%d,\n"
          "   payload                 =0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n"
          "   payload_msg_data_4444   =0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n\n",
          __FUNCTION__, src_unit, __LINE__, result, 
          payload[0], payload[1], payload[2], payload[3], payload[4],
          payload_msg_data_4444[0], payload_msg_data_4444[1], payload_msg_data_4444[2],
          payload_msg_data_4444[3], payload_msg_data_4444[4]);
    }
    
    put_three_message_bytes_to_payload(msg_3bytes, payload);
    result = memcmp(payload, payload_msg_3bytes_556677, sizeof(payload));
    CU_ASSERT_EQUAL(result, 0);
    if (result != 0)
    {
        printf("%s: put_msg_hdr_3333(%d, *) ERROR: line=%d: memcmp != 0, result=%d,\n"
          "   payload                   =0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n"
          "   payload_msg_3bytes_556677 =0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n\n",
          __FUNCTION__, src_unit, __LINE__, result, 
          payload[0], payload[1], payload[2], payload[3], payload[4],
          payload_msg_3bytes_556677[0], payload_msg_3bytes_556677[1], payload_msg_3bytes_556677[2],
          payload_msg_3bytes_556677[3], payload_msg_3bytes_556677[4]);
    }
    
    
} // test_put_functions //
    
void test_get_funtions(void)
{
    UInt8 src_unit;
    UInt8 dst_unit;
    UInt16 msg_hdr;
    UInt16 msg_data;

    //
    // test get_* functions
    //

    src_unit = get_src_unit(input_payload);
    CU_ASSERT_EQUAL(src_unit, 0x05);
    if (src_unit != 0x05)
    {
        printf("%s: get_src_unit(*) ERROR: line=%d: got=0x%02x, expected=0x%02x\n",
          __FUNCTION__, __LINE__, src_unit, 0x05);
    }
    
    dst_unit = get_dst_unit(input_payload);
    CU_ASSERT_EQUAL(dst_unit, 0x06);
    if (dst_unit != 0x06)
    {
        printf("%s: get_dst_unit(*) ERROR: line=%d: got=0x%02x, expected=0x%02x\n",
          __FUNCTION__, __LINE__, dst_unit, 0x06);
    }
    
    msg_hdr = get_msg_hdr(input_payload);
    CU_ASSERT_EQUAL(msg_hdr, 0x789a);
    if (msg_hdr != 0x789a)
    {
        printf("%s: get_msg_hdr(*) ERROR: line=%d: got=0x%02x, expected=0x%02x\n",
          __FUNCTION__, __LINE__, msg_hdr, 0x789a);
    }
    
    msg_data = get_msg_data(input_payload);
    CU_ASSERT_EQUAL(msg_data, 0xbcde);
    if (msg_data != 0xbcde)
    {
        printf("%s: get_msg_data(*) ERROR: line=%d: got=0x%02x, expected=0x%02x\n",
          __FUNCTION__, __LINE__, msg_data, 0xbcde);
    }
    
} // test_get_funtions //
    
void test_block_data_header_functions(void)
{
    int i;
    int result;
    UInt16 message_type;
    UInt16 block_length;
    UInt8 src_unit;
    UInt8 dst_unit;

    // initialize the whole block payload
    for (i=0; i< sizeof(block_data_payload); i++)
    {
        block_data_payload[i] = 0;

    }

    //
    // store a block date payload header into the buffer and see that it
    // was set correctly
    //
    put_block_data_payload_hdr(ONA_BLOCK_TEXT, 23, 1, 2, block_data_payload);
    result = memcmp(block_data_payload, block_data_test_case_1, 10);
    CU_ASSERT_EQUAL(result, 0);
    if (result != 0)
    {
        printf("%s: put_block_data_paylaod_hdr(*) ERROR: line=%d: \n"
          "       got=0x%02x%02x%02x%02x%02x %02x%02x%02x%02x%02x,\n"
          "  expected=0x%02x%02x%02x%02x%02x %02x%02x%02x%02x%02x\n",
          __FUNCTION__, __LINE__, 
          block_data_payload[0], block_data_payload[1], block_data_payload[2],
          block_data_payload[3], block_data_payload[4], 
          block_data_payload[5], block_data_payload[6], block_data_payload[7],
          block_data_payload[8], block_data_payload[9], 
          block_data_test_case_1[0], block_data_test_case_1[1], block_data_test_case_1[2],
          block_data_test_case_1[3], block_data_test_case_1[4],
          block_data_test_case_1[5], block_data_test_case_1[6], block_data_test_case_1[7],
          block_data_test_case_1[8], block_data_test_case_1[9]);
    }

    //
    // read back the first test case data
    //
    get_block_data_payload_hdr(&message_type, &block_length, &src_unit, &dst_unit, block_data_payload);
    CU_ASSERT_EQUAL(message_type, ONA_BLOCK_TEXT);
    if (message_type != ONA_BLOCK_TEXT)
    {
        printf("%s: get_block_data_paylaod_hdr(*) ERROR: line=%d: got=0x%04x, expected=0x%04x\n",
          __FUNCTION__, __LINE__, message_type, ONA_BLOCK_TEXT);
    }
    CU_ASSERT_EQUAL(block_length, 23);
    CU_ASSERT_EQUAL(src_unit, 1);
    CU_ASSERT_EQUAL(dst_unit, 2);

    //
    // try using the get_block_data_paylaod_hdr function to retrieve only one value
    //
    message_type = 0xffff;
    get_block_data_payload_hdr(&message_type, NULL, NULL, NULL, block_data_payload);
    CU_ASSERT_EQUAL(message_type, ONA_BLOCK_TEXT);
    if (message_type != ONA_BLOCK_TEXT)
    {
        printf("%s: get_block_data_paylaod_hdr(*) ERROR: line=%d: got=0x%04x, expected=0x%04x\n",
          __FUNCTION__, __LINE__, message_type, ONA_BLOCK_TEXT);
    }

} // test_block_data_header_funtions //

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
 
    pSuite = CU_add_suite("ONE-NET one_net_application test suite",
     init_suite, clean_suite);
    if(pSuite == NULL)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if((CU_add_test(pSuite, "test put_* functions", test_put_functions) == NULL)
     || (CU_add_test(pSuite, "test get_* functions", test_get_funtions) == NULL)
     || (CU_add_test(pSuite, "test_block_data_header_functions", test_block_data_header_functions) == NULL))
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
} // main //

