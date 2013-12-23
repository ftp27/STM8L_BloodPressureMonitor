// PC0 - I2C1_SDA
// PC1 - I2C1_SCL
// PC2 - USART1_RX
// PC3 - USART1_TX
#include "iostm8l152c6.h"

char getData(int address) {
  char result;
  
  char addressMSB = 0xA0 + ((address << 5) >> 12); 
  char addressLSB = ((address << 8) >> 8);
  
  I2C1_CR2_bit.START = 1; 
  while(!(I2C1_SR1_bit.SB));
    
  I2C1_DR = addressMSB+0; // For write
  while(!(I2C1_SR1_bit.ADDR));
  while(!(I2C1_SR1_bit.TXE));
  while(!(I2C1_SR3_bit.TRA));
  
  I2C1_DR = addressLSB;
  while(!(I2C1_SR1_bit.TXE));
  while(!(I2C1_SR1_bit.BTF));
    
  I2C1_CR2_bit.START = 1; // Repeated Start
  while(!(I2C1_SR1_bit.SB));
  
  I2C1_DR = addressMSB+1;
  while(!(I2C1_SR1_bit.ADDR));
  while(I2C1_SR3_bit.TRA);
 
  while(!(I2C1_SR1_bit.RXNE));  
  result = I2C1_DR;  
  
  while(!(I2C1_SR1_bit.BTF));
  
  I2C1_CR2_bit.STOP = 1;
  
  return result;
}

void delay (int time) {
  for (int i=0; i<time*1000; i++);
}

int main ( void )
{
  CLK_CKDIVR = 0x04;           // System clock source /16 == 1Mhz
  CLK_ICKCR_bit.HSION = 1;     // Clock ready
  CLK_PCKENR1_bit.PCKEN13 = 1; // I2C clock enable
  CLK_PCKENR1_bit.PCKEN15 = 1; // UART clock enable 
  
  // UART init
  USART1_CR1 = 0;
  USART1_CR3 = 0;
  USART1_CR4 = 0;
  USART1_CR5 = 0;
  
  USART1_BRR2 = 0x08;
  USART1_BRR1 = 0x06;
  
  USART1_CR2_bit.TEN = 1;
  USART1_CR2_bit.REN = 1;
  USART1_CR2_bit.RIEN = 1;
  
  // I2C init
  I2C1_FREQR = 0x01;            // Program the peripheral input clock in I2C_FREQR Register 
                                // in order to generate correct timings.
  I2C1_CCRL = 0x32;             // Configure the clock control registers
  I2C1_TRISER = 0x02;           // Configure the rise time register
  I2C1_CR1_bit.PE = 1;          // Program the I2C_CR1 register to enable the peripheral
  
  I2C1_OARL = 0xA0;
  I2C1_OARH_bit.ADDCONF = 1;
  
  for (unsigned int i=0x30; i<0x33; i++) {
     while(!(USART1_SR_bit.TXE));
     USART1_DR = getData(i);
  }
  
  asm("RIM");
  
  while (1);
}

#pragma vector=USART_R_OR_vector
__interrupt void USART_RXNE(void)
{ 
  char recive = USART1_DR;
}
