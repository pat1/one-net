/*!
    \file gen_data_flash_test.c
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
#include "flash.h"
#include "dfi.h"

//#define DUMP_FLASH_SIM

UInt8 test_segment_0[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                          0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00 };

UInt8 test_segment_1[] = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18 };

UInt8 test_segment_2[] = {0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
                          0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x20,
                          0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
                          0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x20 };

UInt8 test_segment_3[] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
                          0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x30,
                          0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
                          0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x30,
                          0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38 };

UInt8 test_segment_4[] = {0x41};

UInt8 test_segment_5[] = {0x51, 0x52};

UInt8 test_segment_6[] = {0x61, 0x62, 0x63};

UInt8 test_segment_7[] = {0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
                          0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x70,
                          0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88,
                          0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 0x80,
                          0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
                          0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0x90,
                          0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8,
                          0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xa0,
                          0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8,
                          0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xb0,
                          0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8,
                          0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xc0,
                          0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8,
                          0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xd0,
                          0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8,
                          0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xe0 };

extern UInt8 sim_data_flash[];

UInt8 mfg_segment_type_list[] =
{
    DFI_ST_DEVICE_MFG_DATA
};

//! the number of entries in dfi_segment_types_used
UInt8 mfg_segment_type_list_count= sizeof(mfg_segment_type_list);



// functions that are not part of the public dfi interface
int dfi_current_block(void);


void dump_flash_sim(char * label)
{
    dfi_segment_hdr_t *sh;
    UInt8 *next_segment;
    UInt8 *end;
    int ff_count;
    int current_block;

    if (sim_data_flash[0] == DFI_ST_UNUSED_FLASH_DATA)
    {
        // first block is current
        next_segment = &sim_data_flash[DF_BLOCK_SIZE];
        end = &sim_data_flash[(DF_BLOCK_SIZE*2)-1];
        current_block = 1;
    }
    else
    {
        // first block is current
        next_segment = &sim_data_flash[0];
        end = &sim_data_flash[DF_BLOCK_SIZE-1];
        current_block = 0;
    }
    printf("%s: LABEL: %s, current block is %d\n", __FUNCTION__, label, current_block);

    while(next_segment < end)
    {
        sh = (dfi_segment_hdr_t *) next_segment;
        printf("%s: 0x%04x: type=0x%02x len=%d\n", __FUNCTION__,
          (UInt16) DFI_ADDR_TO_UINT16(next_segment),
          sh->type, sh->len);
        if (sh->type == DFI_ST_UNUSED_FLASH_DATA)
        {
            // count number of 0xFF bytes
            ff_count = 0;
            while(next_segment < end)
            {
                if (*next_segment++ == DFI_ST_UNUSED_FLASH_DATA)
                {
                    ff_count++;
                }
            }
            printf("%s: free byte count is %d\n", __FUNCTION__, ff_count);
            break;
        }
        else
        {
            next_segment += sizeof(dfi_segment_hdr_t) + sh->len;
        }
    }
}

int data_flash_sim_erased_starting_at(UInt8 *addr)
{
    int i, start;
    
    start = (void *)addr - (void *)DF_BLOCK_A_START;
    for (i=start; i<DF_BLOCK_SIZE*2; i++)
    {
        if (sim_data_flash[i] != 0xff)
        {
            return(0);
        }
    }
    return(1);
}

void test_data_flash_sim(void)
{
    BOOL result;
    UInt16 bytes_written;
    UInt16 total_bytes_written = 0;
    int iresult;

    //
    // test the basic data flash simulation functions (in flash_sim.c)
    //

    // erase data flash
    result = erase_data_flash(DF_BLOCK_A_START);
    CU_ASSERT_EQUAL(result, TRUE);

    result = erase_data_flash(DF_BLOCK_B_START);
    CU_ASSERT_EQUAL(result, TRUE);

    iresult = data_flash_sim_erased_starting_at((UInt8 *)DF_BLOCK_A_START);
    CU_ASSERT_EQUAL(iresult, 1);

    // write one segment
    bytes_written = write_data_flash(DF_BLOCK_A_START, test_segment_1, sizeof(test_segment_1));
    CU_ASSERT_EQUAL(bytes_written, sizeof(test_segment_1));

    iresult = data_flash_sim_erased_starting_at((UInt8 *)(DF_BLOCK_A_START+sizeof(test_segment_1)));
    CU_ASSERT_EQUAL(iresult, 1);

    iresult = memcmp(test_segment_1, &sim_data_flash[total_bytes_written], sizeof(test_segment_1));
    CU_ASSERT_EQUAL(iresult, 0);

    total_bytes_written += bytes_written;
    
    // write a second segment
    bytes_written = write_data_flash(DF_BLOCK_A_START+total_bytes_written,
      test_segment_2, sizeof(test_segment_2));
    CU_ASSERT_EQUAL(bytes_written, sizeof(test_segment_2));

    iresult = data_flash_sim_erased_starting_at((UInt8 *)(DF_BLOCK_A_START+
      sizeof(test_segment_1)+sizeof(test_segment_2)));
    CU_ASSERT_EQUAL(iresult, 1);

    iresult = memcmp(test_segment_1, &sim_data_flash[0], sizeof(test_segment_1));
    CU_ASSERT_EQUAL(iresult, 0);
    
    iresult = memcmp(test_segment_2, &sim_data_flash[total_bytes_written], sizeof(test_segment_2));
    CU_ASSERT_EQUAL(iresult, 0);

    total_bytes_written += bytes_written;
    
    // write a third segment
    bytes_written = write_data_flash(DF_BLOCK_A_START+total_bytes_written,
      test_segment_3, sizeof(test_segment_3));
    CU_ASSERT_EQUAL(bytes_written, sizeof(test_segment_3));

    iresult = data_flash_sim_erased_starting_at((UInt8 *)(DF_BLOCK_A_START+
      sizeof(test_segment_1)+sizeof(test_segment_2)+sizeof(test_segment_3)));
    CU_ASSERT_EQUAL(iresult, 1);

    iresult = memcmp(test_segment_1, &sim_data_flash[0], sizeof(test_segment_1));
    CU_ASSERT_EQUAL(iresult, 0);
    
    iresult = memcmp(test_segment_2, &sim_data_flash[sizeof(test_segment_1)], sizeof(test_segment_2));
    CU_ASSERT_EQUAL(iresult, 0);
    
    iresult = memcmp(test_segment_3, &sim_data_flash[total_bytes_written], sizeof(test_segment_3));
    CU_ASSERT_EQUAL(iresult, 0);

    total_bytes_written += bytes_written;
    
    // write a fourth segment
    bytes_written = write_data_flash(DF_BLOCK_A_START+total_bytes_written,
      test_segment_4, sizeof(test_segment_4));
    CU_ASSERT_EQUAL(bytes_written, sizeof(test_segment_4));

    iresult = data_flash_sim_erased_starting_at((UInt8 *)(DF_BLOCK_A_START+
      sizeof(test_segment_1)+sizeof(test_segment_2)+sizeof(test_segment_3)+
      sizeof(test_segment_4)));
    CU_ASSERT_EQUAL(iresult, 1);

    iresult = memcmp(test_segment_1, &sim_data_flash[0], sizeof(test_segment_1));
    CU_ASSERT_EQUAL(iresult, 0);
    
    iresult = memcmp(test_segment_2, &sim_data_flash[sizeof(test_segment_1)], sizeof(test_segment_2));
    CU_ASSERT_EQUAL(iresult, 0);
    
    iresult = memcmp(test_segment_3, &sim_data_flash[sizeof(test_segment_1)+
      sizeof(test_segment_2)], sizeof(test_segment_3));
    CU_ASSERT_EQUAL(iresult, 0);
    
    iresult = memcmp(test_segment_4, &sim_data_flash[total_bytes_written], sizeof(test_segment_4));
    CU_ASSERT_EQUAL(iresult, 0);

    total_bytes_written += bytes_written;
    
    // write a fifth segment
    bytes_written = write_data_flash(DF_BLOCK_A_START+total_bytes_written,
      test_segment_5, sizeof(test_segment_5));
    CU_ASSERT_EQUAL(bytes_written, sizeof(test_segment_5));

    iresult = data_flash_sim_erased_starting_at((UInt8 *)(DF_BLOCK_A_START+
      sizeof(test_segment_1)+sizeof(test_segment_2)+sizeof(test_segment_3)+
      sizeof(test_segment_4)+sizeof(test_segment_5)));
    CU_ASSERT_EQUAL(iresult, 1);

    iresult = memcmp(test_segment_1, &sim_data_flash[0], sizeof(test_segment_1));
    CU_ASSERT_EQUAL(iresult, 0);
    
    iresult = memcmp(test_segment_2, &sim_data_flash[sizeof(test_segment_1)], sizeof(test_segment_2));
    CU_ASSERT_EQUAL(iresult, 0);
    
    iresult = memcmp(test_segment_3, &sim_data_flash[sizeof(test_segment_1)+
      sizeof(test_segment_2)], sizeof(test_segment_3));
    CU_ASSERT_EQUAL(iresult, 0);
    
    iresult = memcmp(test_segment_4, &sim_data_flash[sizeof(test_segment_1)+
      sizeof(test_segment_2)+sizeof(test_segment_3)], sizeof(test_segment_4));
    CU_ASSERT_EQUAL(iresult, 0);
    
    iresult = memcmp(test_segment_5, &sim_data_flash[total_bytes_written], sizeof(test_segment_5));
    CU_ASSERT_EQUAL(iresult, 0);

    total_bytes_written += bytes_written;
    
    // write a sixth segment
    bytes_written = write_data_flash(DF_BLOCK_A_START+total_bytes_written,
      test_segment_6, sizeof(test_segment_6));
    CU_ASSERT_EQUAL(bytes_written, sizeof(test_segment_6));

    iresult = data_flash_sim_erased_starting_at((UInt8 *)(DF_BLOCK_A_START+
      sizeof(test_segment_1)+sizeof(test_segment_2)+sizeof(test_segment_3)+
      sizeof(test_segment_4)+sizeof(test_segment_5)+sizeof(test_segment_6)));
    CU_ASSERT_EQUAL(iresult, 1);

    iresult = memcmp(test_segment_1, &sim_data_flash[0], sizeof(test_segment_1));
    CU_ASSERT_EQUAL(iresult, 0);
    
    iresult = memcmp(test_segment_2, &sim_data_flash[sizeof(test_segment_1)], sizeof(test_segment_2));
    CU_ASSERT_EQUAL(iresult, 0);
    
    iresult = memcmp(test_segment_3, &sim_data_flash[sizeof(test_segment_1)+
      sizeof(test_segment_2)], sizeof(test_segment_3));
    CU_ASSERT_EQUAL(iresult, 0);
    
    iresult = memcmp(test_segment_4, &sim_data_flash[sizeof(test_segment_1)+
      sizeof(test_segment_2)+sizeof(test_segment_3)], sizeof(test_segment_4));
    CU_ASSERT_EQUAL(iresult, 0);
    
    iresult = memcmp(test_segment_5, &sim_data_flash[sizeof(test_segment_1)+
      sizeof(test_segment_2)+sizeof(test_segment_3)+sizeof(test_segment_4)],
      sizeof(test_segment_5));
    CU_ASSERT_EQUAL(iresult, 0);
    
    iresult = memcmp(test_segment_6, &sim_data_flash[total_bytes_written], sizeof(test_segment_6));
    CU_ASSERT_EQUAL(iresult, 0);

    total_bytes_written += bytes_written;

    // try some error conditions
    // write that starts OK, but end outside of data flash area
    bytes_written = write_data_flash(DF_BLOCK_A_START+DF_BLOCK_SIZE*2-128,
      test_segment_1, 255);
    CU_ASSERT_EQUAL(bytes_written, 0);

    // write that starts after data flash area
    bytes_written = write_data_flash(DF_BLOCK_A_START+DF_BLOCK_SIZE*2+1,
      test_segment_1, 2);
    CU_ASSERT_EQUAL(bytes_written, 0);

    // write that starts before data flash area
    bytes_written = write_data_flash(DF_BLOCK_A_START-1,
      test_segment_1, 2);
    CU_ASSERT_EQUAL(bytes_written, 0);


} // test_data_flash_sim //
    
void test_dfi_functions(void)
{

    BOOL bresult;
    int current_block;
    UInt8 * ptr_result;
    int result;
    int i;

    //
    // setup simulated flash and test dfi_current_block()
    //
    // OK, so these next few unit tests should not exist because
    // the function tested is not part of the public interface
    // and should be declared static. But come now, aren't these
    // unit tests like our children and would you kill one
    // of your children just because they were illegitimate?
    //
    sim_data_flash[0] = DFI_ST_UNUSED_FLASH_DATA;
    sim_data_flash[DF_BLOCK_SIZE] = DFI_ST_UNUSED_FLASH_DATA;
    current_block = dfi_current_block();
    CU_ASSERT_EQUAL(current_block, 0);

    sim_data_flash[0] = DFI_ST_ONE_NET_MASTER_SETTINGS;
    sim_data_flash[DF_BLOCK_SIZE] = DFI_ST_UNUSED_FLASH_DATA;
    current_block = dfi_current_block();
    CU_ASSERT_EQUAL(current_block, 0);
    
    // erase data flash
    bresult = erase_data_flash(DF_BLOCK_A_START);
    CU_ASSERT_EQUAL(bresult, TRUE);

    bresult = erase_data_flash(DF_BLOCK_B_START);
    CU_ASSERT_EQUAL(bresult, TRUE);

    //
    // add the first segment
    //
    ptr_result = dfi_write_segment_of_type(DFI_ST_APP_DATA_1, test_segment_1,
      sizeof(test_segment_1));
    CU_ASSERT_NOT_EQUAL(ptr_result, (UInt8 *) 0);

    ptr_result = dfi_find_last_segment_of_type(DFI_ST_APP_DATA_1);
    CU_ASSERT_NOT_EQUAL(ptr_result, (UInt8 *) 0);

#ifdef DUMP_FLASH_SIM
    dump_flash_sim("after first segment add");
#endif

    if (ptr_result != (UInt8 *) 0)
    {
        result = memcmp(ptr_result+sizeof(dfi_segment_hdr_t), test_segment_1, sizeof(test_segment_1));
        CU_ASSERT_EQUAL(result, 0);
    }
   
    // 
    // add a second segment of the same type as the first, but with different data
    //
    ptr_result = dfi_write_segment_of_type(DFI_ST_APP_DATA_1, test_segment_2,
      sizeof(test_segment_2));
    CU_ASSERT_NOT_EQUAL(ptr_result, (UInt8 *) 0);

    ptr_result = dfi_find_last_segment_of_type(DFI_ST_APP_DATA_1);
    CU_ASSERT_NOT_EQUAL(ptr_result, (UInt8 *) 0);

    if (ptr_result != (UInt8 *) 0)
    {
        result = memcmp(ptr_result+sizeof(dfi_segment_hdr_t), test_segment_2, sizeof(test_segment_2));
        CU_ASSERT_EQUAL(result, 0);
    }

#ifdef DUMP_FLASH_SIM
    dump_flash_sim("after second segment add");
#endif
   
    // 
    // add a third segment of the same type as the first and second, but with different data
    //
    ptr_result = dfi_write_segment_of_type(DFI_ST_APP_DATA_1, test_segment_3,
      sizeof(test_segment_3));
    CU_ASSERT_NOT_EQUAL(ptr_result, (UInt8 *) 0);

    ptr_result = dfi_find_last_segment_of_type(DFI_ST_APP_DATA_1);
    CU_ASSERT_NOT_EQUAL(ptr_result, (UInt8 *) 0);

    if (ptr_result != (UInt8 *) 0)
    {
        result = memcmp(ptr_result+sizeof(dfi_segment_hdr_t), test_segment_3, sizeof(test_segment_3));
        CU_ASSERT_EQUAL(result, 0);
    }
   
#ifdef DUMP_FLASH_SIM
    dump_flash_sim("after third segment add");
#endif

    // 
    // add a third segment of different type as the first, second, and third 
    //
    ptr_result = dfi_write_segment_of_type(DFI_ST_APP_DATA_2, test_segment_7,
      sizeof(test_segment_7));
    CU_ASSERT_NOT_EQUAL(ptr_result, (UInt8 *) 0);

    ptr_result = dfi_find_last_segment_of_type(DFI_ST_APP_DATA_2);
    CU_ASSERT_NOT_EQUAL(ptr_result, (UInt8 *) 0);

    if (ptr_result != (UInt8 *) 0)
    {
        result = memcmp(ptr_result+sizeof(dfi_segment_hdr_t), test_segment_7, sizeof(test_segment_7));
        CU_ASSERT_EQUAL(result, 0);
    }

    ptr_result = dfi_find_last_segment_of_type(DFI_ST_APP_DATA_1);
    CU_ASSERT_NOT_EQUAL(ptr_result, (UInt8 *) 0);

    if (ptr_result != (UInt8 *) 0)
    {
        result = memcmp(ptr_result+sizeof(dfi_segment_hdr_t), test_segment_3, sizeof(test_segment_3));
        CU_ASSERT_EQUAL(result, 0);
    }

#ifdef DUMP_FLASH_SIM
    dump_flash_sim("after fourth segment add");
#endif
   
    //
    // write another instance of the DFI_ST_APP_DATA_2 segment with different data
    //
    ptr_result = dfi_write_segment_of_type(DFI_ST_APP_DATA_2, test_segment_2,
      sizeof(test_segment_2));
    CU_ASSERT_NOT_EQUAL(ptr_result, (UInt8 *) 0);

    ptr_result = dfi_find_last_segment_of_type(DFI_ST_APP_DATA_2);
    CU_ASSERT_NOT_EQUAL(ptr_result, (UInt8 *) 0);

    if (ptr_result != (UInt8 *) 0)
    {
        result = memcmp(ptr_result+sizeof(dfi_segment_hdr_t), test_segment_2, sizeof(test_segment_2));
        CU_ASSERT_EQUAL(result, 0);
    }

    ptr_result = dfi_find_last_segment_of_type(DFI_ST_APP_DATA_1);
    CU_ASSERT_NOT_EQUAL(ptr_result, (UInt8 *) 0);

    if (ptr_result != (UInt8 *) 0)
    {
        result = memcmp(ptr_result+sizeof(dfi_segment_hdr_t), test_segment_3, sizeof(test_segment_3));
        CU_ASSERT_EQUAL(result, 0);
    }

#ifdef DUMP_FLASH_SIM
    dump_flash_sim("after fifth segment add");
#endif
   
    //
    // write many segments of a third type, forcing a switch to the other block
    //
    for (i=0; i<10; i++)
    {
        ptr_result = dfi_write_segment_of_type(DFI_ST_APP_DATA_3, test_segment_7,
          sizeof(test_segment_7));
        CU_ASSERT_NOT_EQUAL(ptr_result, (UInt8 *) 0);

        ptr_result = dfi_find_last_segment_of_type(DFI_ST_APP_DATA_3);
        CU_ASSERT_NOT_EQUAL(ptr_result, (UInt8 *) 0);

        if (ptr_result != (UInt8 *) 0)
        {
            result = memcmp(ptr_result+sizeof(dfi_segment_hdr_t), test_segment_7, sizeof(test_segment_7));
            CU_ASSERT_EQUAL(result, 0);
        }
        else
        {
            printf("%s: %s: ERROR: i=%d\n", __FILE__, __FUNCTION__, i);
        }

#ifdef DUMP_FLASH_SIM
        dump_flash_sim("after nth segment add");
#endif
    }

    //
    // make sure the other segments were copied correctly
    //
    ptr_result = dfi_find_last_segment_of_type(DFI_ST_APP_DATA_2);
    CU_ASSERT_NOT_EQUAL(ptr_result, (UInt8 *) 0);

    if (ptr_result != (UInt8 *) 0)
    {
        result = memcmp(ptr_result+sizeof(dfi_segment_hdr_t), test_segment_2, sizeof(test_segment_2));
        CU_ASSERT_EQUAL(result, 0);
    }

    ptr_result = dfi_find_last_segment_of_type(DFI_ST_APP_DATA_1);
    CU_ASSERT_NOT_EQUAL(ptr_result, (UInt8 *) 0);

    if (ptr_result != (UInt8 *) 0)
    {
        result = memcmp(ptr_result+sizeof(dfi_segment_hdr_t), test_segment_3, sizeof(test_segment_3));
        CU_ASSERT_EQUAL(result, 0);
    }

   
} // test_dfi_functions //
    
void test_dfi_delete(void)
{
    UInt8 * ptr_result;
    int result;

    //
    // test dfi_delete_segments_except_for function
    //
#ifdef DUMP_FLASH_SIM
    dump_flash_sim("at the beginning of test_dfi_delete");
#endif

    //
    // make sure we already have segments in data flash
    //
    ptr_result = dfi_find_last_segment_of_type(DFI_ST_APP_DATA_3);
    CU_ASSERT_NOT_EQUAL(ptr_result, (UInt8 *) 0);

    if (ptr_result != (UInt8 *) 0)
    {
        result = memcmp(ptr_result+sizeof(dfi_segment_hdr_t), test_segment_7, sizeof(test_segment_7));
        CU_ASSERT_EQUAL(result, 0);
    }

    ptr_result = dfi_find_last_segment_of_type(DFI_ST_APP_DATA_2);
    CU_ASSERT_NOT_EQUAL(ptr_result, (UInt8 *) 0);

    if (ptr_result != (UInt8 *) 0)
    {
        result = memcmp(ptr_result+sizeof(dfi_segment_hdr_t), test_segment_2, sizeof(test_segment_2));
        CU_ASSERT_EQUAL(result, 0);
    }

    ptr_result = dfi_find_last_segment_of_type(DFI_ST_APP_DATA_1);
    CU_ASSERT_NOT_EQUAL(ptr_result, (UInt8 *) 0);

    if (ptr_result != (UInt8 *) 0)
    {
        result = memcmp(ptr_result+sizeof(dfi_segment_hdr_t), test_segment_3, sizeof(test_segment_3));
        CU_ASSERT_EQUAL(result, 0);
    }

    //
    // add a segment that we want presevered 
    //
    ptr_result = dfi_write_segment_of_type(DFI_ST_DEVICE_MFG_DATA, test_segment_0,
      sizeof(test_segment_0));
    CU_ASSERT_NOT_EQUAL(ptr_result, (UInt8 *) 0);

    //
    // now delete all segments except for the one we want to preserve
    //
    dfi_delete_segments_except_for(mfg_segment_type_list, mfg_segment_type_list_count);

    ptr_result = dfi_find_last_segment_of_type(DFI_ST_APP_DATA_3);
    CU_ASSERT_EQUAL(ptr_result, (UInt8 *) 0);

    ptr_result = dfi_find_last_segment_of_type(DFI_ST_APP_DATA_2);
    CU_ASSERT_EQUAL(ptr_result, (UInt8 *) 0);

    ptr_result = dfi_find_last_segment_of_type(DFI_ST_APP_DATA_1);
    CU_ASSERT_EQUAL(ptr_result, (UInt8 *) 0);

    ptr_result = dfi_find_last_segment_of_type(DFI_ST_DEVICE_MFG_DATA);
    CU_ASSERT_NOT_EQUAL(ptr_result, (UInt8 *) 0);

    if (ptr_result != (UInt8 *) 0)
    {
        result = memcmp(ptr_result+sizeof(dfi_segment_hdr_t), test_segment_0, sizeof(test_segment_0));
        CU_ASSERT_EQUAL(result, 0);
    }

#ifdef DUMP_FLASH_SIM
    dump_flash_sim("at the end of test_dfi_delete");
#endif
    
} // test_dfi_delete //
    

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
 
    pSuite = CU_add_suite("generic data flash test suite",
     init_suite, clean_suite);
    if(pSuite == NULL)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if((CU_add_test(pSuite, "test save basic flash simulation functions", test_data_flash_sim) == NULL)
     || (CU_add_test(pSuite, "test dfi write/read functions", test_dfi_functions) == NULL)
     || (CU_add_test(pSuite, "test dfi delete function", test_dfi_delete) == NULL))
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

