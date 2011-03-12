#include "RF1A.h"
#include "cc430x613x.h"

unsigned char packetReceived = 0;
unsigned char packetTransmit = 0; 

unsigned char Strobe(unsigned char strobe)
{
  unsigned char statusByte;
  
  // Check for valid strobe command 
  if((strobe == 0xBD) || (strobe >= 0x30) && (strobe <= 0x3D))
  {
    RF1AIFCTL1 &= ~(RFSTATIFG);             // Clear the status read flag
    
    while( !(RF1AIFCTL1 & RFINSTRIFG)  );   // Wait for INSTRIFG
    RF1AINSTRB = strobe;                    // Write the strobe command    
    
    if(strobe != 0x30) while( !(RF1AIFCTL1 & RFSTATIFG) ); 
    statusByte = RF1ASTATB;         
  }  
  else
    return 0;                               // Invalid strobe was sent
  
  return statusByte;
}

unsigned char ReadSingleReg(unsigned char addr)
{
  unsigned char data_out;
  
  // Check for valid configuration register address, 0x3E refers to PATABLE 
  if ((addr <= 0x2E) || (addr == 0x3E))
    // Send address + Instruction + 1 dummy byte (auto-read)
    RF1AINSTR1B = (addr | RF_SNGLREGRD);    
  else
    // Send address + Instruction + 1 dummy byte (auto-read)
    RF1AINSTR1B = (addr | RF_STATREGRD);    
  
  while (!(RF1AIFCTL1 & RFDOUTIFG) );
  data_out = RF1ADOUT1B;                    // Read data and clears the RFDOUTIFG

  return data_out;
}

void WriteSingleReg(unsigned char addr, unsigned char value)
{   
  while (!(RF1AIFCTL1 & RFINSTRIFG));       // Wait for the Radio to be ready for next instruction
  RF1AINSTRB = (addr | RF_REGWR);			// Send address + Instruction

  RF1ADINB = value; 						// Write data in 

  __no_operation(); 
}
        

void ReadBurstReg(unsigned char addr, unsigned char *buffer, unsigned char count)
{
  unsigned int i;
  
  while (!(RF1AIFCTL1 & RFINSTRIFG));       // Wait for INSTRIFG
  RF1AINSTR1B = (addr | RF_REGRD);          // Send addr of first conf. reg. to be read 
                                            // ... and the burst-register read instruction
  for (i = 0; i < (count-1); i++)
  {
    while (!(RFDOUTIFG&RF1AIFCTL1));        // Wait for the Radio Core to update the RF1ADOUTB reg
    buffer[i] = RF1ADOUT1B;                 // Read DOUT from Radio Core + clears RFDOUTIFG
                                            // Also initiates auo-read for next DOUT byte
  }
  buffer[count-1] = RF1ADOUT0B;             // Store the last DOUT from Radio Core
}  

void WriteBurstReg(unsigned char addr, unsigned char *buffer, unsigned char count)
{  
  // Write Burst works wordwise not bytewise - known errata
  unsigned char i;

  while (!(RF1AIFCTL1 & RFINSTRIFG));       // Wait for the Radio to be ready for next instruction
  RF1AINSTRW = ((addr | RF_REGWR)<<8 ) + buffer[0]; // Send address + Instruction

  for (i = 1; i < count; i++)
  {
    RF1ADINB = buffer[i];                   // Send data
    while (!(RFDINIFG & RF1AIFCTL1));       // Wait for TX to finish
  } 
  i = RF1ADOUTB;                            // Reset RFDOUTIFG flag which contains status byte
}

void WriteSmartRFReg(const unsigned char SmartRFSetting[][2], unsigned char size)
{
  unsigned char i;
  for (i=0; i < (size); i++)
    WriteSingleReg(SmartRFSetting[i][0], SmartRFSetting[i][1]);
}

void ResetRadioCore (void)
{
  Strobe(RF_SRES);                          // Reset the Radio Core
  Strobe(RF_SNOP);                          // Reset Radio Pointer
}


void WritePATable(void)
{
  unsigned char valueRead = 0;
  while(valueRead != 0xc0)
//  while(valueRead != 0x51)
  {
    /* Write the power output to the PA_TABLE and verify the write operation.  */
    unsigned char i = 0; 

    /* wait for radio to be ready for next instruction */
    while( !(RF1AIFCTL1 & RFINSTRIFG));
//    RF1AINSTRW = 0x7E51; // PA Table write (burst)
    RF1AINSTRW = 0x7Ec0; // PA Table write (burst)
    
    /* wait for radio to be ready for next instruction */
    while( !(RF1AIFCTL1 & RFINSTRIFG));
      RF1AINSTR1B = RF_PATABRD;                 // -miguel read & write
    
    // Traverse PATABLE pointers to read 
    for (i = 0; i < 7; i++)
    {
      while( !(RF1AIFCTL1 & RFDOUTIFG));
      valueRead  = RF1ADOUT1B;     
    }
    while( !(RF1AIFCTL1 & RFDOUTIFG));
    valueRead  = RF1ADOUTB;
  }
}

/*
void Transmit(unsigned char *buffer, unsigned char length)
{
  RF1AIES |= BIT9;                          
  RF1AIFG &= ~BIT9;                         // Clear pending interrupts
  RF1AIE |= BIT9;                           // Enable RFIFG9 TX end-of-packet interrupts
  
  // RF1AIN_9 => Rising edge indicates SYNC sent/received and
  //             Falling edge indicates end of packet.
  //             Configure it to interrupt on falling edge.
  //    
  WriteBurstReg(RF_TXFIFOWR, buffer, length);     
  
  Strobe( RF_STX );                         // Strobe STX   
}
*/
/*
void ReceiveOn(void)
{  
  RF1AIFG &= ~BIT4;                         // Clear a pending interrupt
  RF1AIE  |= BIT4;                          // Enable the interrupt 
  
  Strobe( RF_SIDLE );
  Strobe( RF_SRX );                      
}
*/
/*
void ReceiveOff(void)
{
  RF1AIE &= ~BIT4;                          // Disable RX interrupts
  RF1AIFG &= ~BIT4;                         // Clear pending IFG

  Strobe( RF_SIDLE );
  Strobe( RF_SFRX);      // Flush the receive FIFO of any residual data
}
*/
// Called if an interface error has occured. No interface errors should 
// exist in application code, so this is intended to be used for debugging
// or to catch errant operating conditions. 
/*
static void RF1A_interface_error_handler(void)
{
  switch(__even_in_range(RF1AIFERRV,8))
  {
    case 0: break;                          // No error
    case 2:                                 // Low core voltage error     
      P1OUT &= ~BIT0;						// 00 = on LED's [D2,D1]
      P3OUT &= ~BIT6; 
      __no_operation();
      break; 
    case 4:                                 // Operand Error
      P1OUT |= BIT0;						// 01 = on LED's [D2,D1]
      P3OUT &= ~BIT6; 
      __no_operation();
      break;  
    case 6:                                 // Output data not available error 
      P1OUT &= ~BIT0;						// 10 = on LED's [D2,D1]
      P3OUT |= BIT6; 
      __no_operation();
      break;
    case 8:                                 // Operand overwrite error
      P1OUT |= BIT0;						// 11 = on LED's [D2,D1]
      P3OUT |= BIT6; 
      __no_operation();
      break; 
  }
}
*/

/*
// If RF1A_interrupt_handler is called, an interface interrupt has occured.
static void RF1A_interrupt_handler(void)
{
  // RF1A interrupt is pending
  switch(__even_in_range(RF1AIFIV,14))
  {
    case  0: break;                         // No interrupt pending
    case  2:                                // RFERRIFG 
      RF1A_interface_error_handler();
    case  4: break;                         // RFDOUTIFG
    case  6: break;                         // RFSTATIFG
    case  8: break;                         // RFDINIFG
    case 10: break;                         // RFINSTRIFG
    case 12: break;                         // RFRXIFG
    case 14: break;                         // RFTXIFG
  }
}
*/
/*
#pragma vector=CC1101_VECTOR
__interrupt void CC1101_ISR(void)
{
  switch(__even_in_range(RF1AIV,32))        // Prioritizing Radio Core Interrupts 
  {
    case  0:                                // No RF core interrupt pending                                            
      RF1A_interrupt_handler();             // means RF1A interface interrupt pending
      break; 
    case  2: break;                         // RFIFG0 
    case  4: break;                         // RFIFG1
    case  6: break;                         // RFIFG2
    case  8: break;                         // RFIFG3
    case 10:                                // RFIFG4 - RX end-of-packet
      packetReceived = 1;    
      break;
    case 12: break;                         // RFIFG5
    case 14: break;                         // RFIFG6          
    case 16: break;                         // RFIFG7
    case 18: break;                         // RFIFG8
    case 20:                                // RFIFG9 - TX end-of-packet
      packetTransmit = 1;                   
      break;
    case 22: break;                         // RFIFG10
    case 24: break;                         // RFIFG11
    case 26: break;                         // RFIFG12
    case 28: break;                         // RFIFG13
    case 30: break;                         // RFIFG14
    case 32: break;                         // RFIFG15
  }  
  __bic_SR_register_on_exit(LPM3_bits);     
}
*/