#include <msp430g2553.h>

#define SCALE 1
#define VOLT_STEP (150 / SCALE)
#define NUM_SAMPLES 72


void Drive_DAC(unsigned int level);
volatile unsigned int TempDAC_Value = VOLT_STEP;
int sine_index = 0;

const int sine_map[] = {
		2000,
		2174,
		2347,
		2517,
		2684,
		2845,
		3000,
		3147,
		3285,
		3414,
		3532,
		3638,
		3732,
		3812,
		3879,
		3931,
		3969,
		3992,
		4000,
		3992,
		3969,
		3931,
		3879,
		3812,
		3732,
		3638,
		3532,
		3414,
		3285,
		3147,
		3000,
		2845,
		2684,
		2517,
		2347,
		2174,
		2000,
		1825,
		1652,
		1482,
		1315,
		1154,
		1000,
		852,
		714,
		585,
		467,
		361,
		267,
		187,
		120,
		68,
		30,
		7,
		0,
		7,
		30,
		68,
		120,
		187,
		267,
		361,
		467,
		585,
		714,
		852,
		1000,
		1154,
		1315,
		1482,
		1652,
		1825,
};

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;        // Stop watchdog timer
  CCTL0 = CCIE;

  // 16Mhz SMCLK
  if (CALBC1_16MHZ==0xFF)		     // If calibration constant erased
  {
    while(1);                        // do not load, trap CPU!!
  }
  DCOCTL = 0;                        // Select lowest DCOx and MODx settings
  BCSCTL1 = CALBC1_16MHZ;            // Set range
  DCOCTL = CALDCO_16MHZ;             // Set DCO step + modulation

  // Init Ports
  P1DIR |= BIT4;                     // Will use BIT4 to activate /CE on the DAC

  P1SEL  = BIT7 + BIT5;  // + BIT4;  // These two lines dedicate P1.7 and P1.5
  P1SEL2 = BIT7 + BIT5; // + BIT4;   // for UCB0SIMO and UCB0CLK respectively

  // SPI Setup
  // clock inactive state = low,
  // MSB first, 8-bit SPI master,
  // 4-pin active Low STE, synchronous
  //
  // 4-bit mode disabled for now
  UCB0CTL0 |= UCCKPL + UCMSB + UCMST + /* UCMODE_2 */ + UCSYNC;


  UCB0CTL1 |= UCSSEL_2;               // UCB0 will use SMCLK as the basis for
                                      // the SPI bit clock

  // Sets SMCLK divider to 16,
  // hence making SPI SCLK equal
  // to SMCLK/16 = 1MHz
  UCB0BR0 |= 0x10;             // (low divider byte)
  UCB0BR1 |= 0x00;             // (high divider byte)

  // An example of creating another lower frequency SPI SCLK
  //UCB0BR0 |= 0x00;           // (low byte)  This division caused divide by 256
  //UCB0BR1 |= 0x01;           // (high byte) for an SPI SCLK of ~62.5 KHz.

  //UCB0MCTL = 0;              // No modulation => NOT REQUIRED ON B PORT,
                               // would be required if used UCA0

  UCB0CTL1 &= ~UCSWRST;        // **Initialize USCI state machine**
                               // SPI now Waiting for something to
                               // be placed in TXBUF.

  // No interrupts in this version, so commented out
  //IE2 = UCB0TXIE;            // Enable USCI0 TX interrupt
  //IE2 = UCB0RXIE;            // Enable USCI0 RX interrupt
  //_enable_interrupts();

  TACTL = TASSEL_2 + MC_1 + ID_3;
  TACCR0 = 2000 / SCALE;
  __enable_interrupt();

} // end of main


void Drive_DAC(unsigned int level){
  unsigned int DAC_Word = 0;

  DAC_Word = (0x3000) | (level & 0x0FFF);   // 0x1000 sets DAC for Write
                                            // to DAC, Gain = 1, /SHDN = 1
                                            // and put 12-bit level value
                                            // in low 12 bits.

  P1OUT &= ~BIT4;                    // Clear P1.4 (drive /CS low on DAC)
                                     // Using a port output to do this for now

  UCB0TXBUF = (DAC_Word >> 8);       // Shift upper byte of DAC_Word
                                     // 8-bits to right

  while (!(IFG2 & UCB0TXIFG));       // USCI_A0 TX buffer ready?

  UCB0TXBUF = (unsigned char)
		       (DAC_Word & 0x00FF);  // Transmit lower byte to DAC

  while (!(IFG2 & UCB0TXIFG));       // USCI_A0 TX buffer ready?
  __delay_cycles(100);               // Delay 200 16 MHz SMCLK periods
                                     // (12.5 us) to allow SIMO to complete
  P1OUT |= BIT4;                     // Set P1.4   (drive /CS high on DAC)
  return;
}

// no interrupts for now
/*
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
  // Test for valid RX and TX character
  volatile unsigned int i;
}
*/

#pragma vector=TIMER0_A0_VECTOR
__interrupt void something(void) {
	/*if(++counter >= 10 * SCALE) {
		go_up ^= 1;
		counter = 0;
	}
	if(go_up)
       TempDAC_Value += VOLT_STEP;
	else
	   TempDAC_Value -= VOLT_STEP;

	Drive_DAC(TempDAC_Value);*/
	P1DIR |= BIT0;
	P1OUT |= BIT0;
	Drive_DAC(sine_map[sine_index++]);
	sine_index %= NUM_SAMPLES;
	P1OUT &= ~BIT0;
	P1DIR &= ~BIT0;
}

