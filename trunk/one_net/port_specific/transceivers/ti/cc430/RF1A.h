
/* ------------------------------------------------------------------------------------------------
 *                                          Defines
 * ------------------------------------------------------------------------------------------------
 */

/* Define whether or not to check for RF1AERROR flags during debug */ 
//#define DEBUG 1              // For debug of error flags ONLY

/* Define frequency of operation by commenting the symbol, SELECT_MHZ_FOR_OPERATION,
 * then uncommenting the appropriate frequency symbol (868 or 915)*/ 
//#define SELECT_MHZ_FOR_OPERATION  1 
//#define MHZ_868   1            // Select either 868 or 915MHz
#define MHZ_915   1

#ifdef SELECT_MHZ_FOR_OPERATION
  #error: "Please define frequency of operation in RF1A.h" 
#endif

#define CONF_REG_SIZE	 47	   /* There are 47 8-bit configuration registers */ 

unsigned char Strobe(unsigned char strobe);
unsigned char ReadSingleReg(unsigned char addr);
void WriteSingleReg(unsigned char addr, unsigned char value);
void ReadBurstReg(unsigned char addr, unsigned char *buffer, unsigned char count);
void WriteBurstReg(unsigned char addr, unsigned char *buffer, unsigned char count);
void ResetRadioCore (void);
void WritePATable(void);
//void Transmit(unsigned char *buffer, unsigned char length);
//void ReceiveOn(void);
//void ReceiveOff(void);
//void WriteSmartRFReg(const unsigned char SmartRFSetting[][2], unsigned char size);
