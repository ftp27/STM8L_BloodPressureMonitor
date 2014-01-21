#include "iostm8l101f3.h"

void delay (unsigned int time) {
  for (unsigned int i=0; i<time*1000; i++);
}

void pressOnOf() {
  PB_ODR_bit.ODR0 = 1;
  delay(10000);
  PB_ODR_bit.ODR0 = 0;
}

char getData(int address) {
  char result;
  
  char addressMSB = 0xA0 + ((address << 5) >> 12); 
  char addressLSB = ((address << 8) >> 8);
  
  I2C_CR2_bit.START = 1; 
  while(!(I2C_SR1_bit.SB));
    
  I2C_DR = addressMSB+0; // For write
  while(!(I2C_SR1_bit.ADDR));
  while(!(I2C_SR1_bit.TXE));
  while(!(I2C_SR3_bit.TRA));
  
  I2C_DR = addressLSB;
  while(!(I2C_SR1_bit.TXE));
  while(!(I2C_SR1_bit.BTF));
    
  I2C_CR2_bit.START = 1; // Repeated Start
  while(!(I2C_SR1_bit.SB));
  
  I2C_DR = addressMSB+1;
  while(!(I2C_SR1_bit.ADDR));
  while(I2C_SR3_bit.TRA);
 
  while(!(I2C_SR1_bit.RXNE));  
  result = I2C_DR;  
  
  while(!(I2C_SR1_bit.BTF));
  
  I2C_CR2_bit.STOP = 1;
  
  return result;
}

void outputData(int shift) {
  unsigned address = 0x30+shift;
  for (unsigned int i=address; i<address+3; i++) {
    while(!(USART_SR_bit.TXE));
    USART_DR = getData(i);
  }
}

int main ( void )
{
  CLK_CKDIVR = 0x03;  // System clock source /8 == 2Mhz
  CLK_PCKENR |= 0x08; // I2C clock enable
  CLK_PCKENR |= 0x20; // UART clock enable
  
  // UART init
  PC_DDR_bit.DDR3 = 1;
  PC_CR1_bit.C13 = 1;
  PC_CR2_bit.C23 = 0;
  
  USART_CR1 = 0;
  USART_CR3 = 0;
  USART_CR4 = 0;
  
  USART_BRR2 = 0x00;
  USART_BRR1 = 0x0D;
  
  USART_CR2_bit.TEN = 1;
  USART_CR2_bit.REN = 1;
  USART_CR2_bit.RIEN = 1;
  
  // I2C init
  I2C_FREQR = 0x02;   // Program the peripheral input clock in I2C_FREQR Register 
                      // in order to generate correct timings.
  I2C_CCRL = 0x32;    // Configure the clock control registers
  I2C_TRISER = 0x03;  // Configure the rise time register
  I2C_CR1_bit.PE = 1; // Program the I2C_CR1 register to enable the peripheral
  
  I2C_OARL = 0xA0;
  I2C_OARH_bit.ADDCONF = 1;
  
  // GPIO config
    //Button OnOff
  PB_DDR_bit.DDR0 = 1;
  PB_CR1_bit.C10 = 1;
  PB_ODR_bit.ODR0 = 0; 
    // Valve input
  PB_DDR_bit.DDR7 = 0;
  PB_CR1_bit.C17 = 1;
  PB_CR2_bit.C27 = 1;
  EXTI_CR2_bit.P7IS = 1;
  
  asm("RIM");
  
  while (1);
}

#pragma vector=USART_R_OR_vector
__interrupt void USART_RXNE(void)
{ 
  char recive = USART_DR;
  
  if (recive == 0x01) {
    pressOnOf();
  } else {
    outputData(recive-0x02);
  }
}

#pragma vector=EXTI7_vector
__interrupt void valveOff (void) {
  pressOnOf();
  delay(100000);
  outputData(0);
  EXTI_SR1_bit.P1F = 1;
}
