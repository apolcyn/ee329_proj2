#include <msp430g2553.h>
#include<stdio.h>

#define SCALE 1
#define NUM_SAMPLES 50
#define HIGH_SQUARE 4095
#define LOW_SQUARE 100
#define VOLT_STEP ((HIGH_SQUARE - LOW_SQUARE) / NUM_SAMPLES)

#define SAWTOOTH 0
#define SQUARE 1
#define SINE 2

#define LOW_CCRO_COUNT 444
#define HZ_100 395
#define HZ_200 196
#define HZ_300 130
#define HZ_400 98
#define HZ_500 78

#define DUTY_10 (NUM_SAMPLES / 10)

void Drive_DAC(unsigned int level);

void draw_sine_wave(void);
void draw_square_wave(void);
void draw_sawtooth_wave(void);

void initBtn1(void);
void initBtn23(void);

volatile unsigned int TempDAC_Value = 0;

int sine_index = 0;

long glob_counter = 0;

int ccr0_state = HZ_100;

int sawtooth_counter = 0;
int square_counter = 0;

int square_clock_counts = DUTY_10;
int sawtooth_clock_counts = NUM_SAMPLES;

int freq_dividor = 1;

int wave_state = SAWTOOTH;

int duty_10_counts = 1;

void (*draw_wave)(void) = draw_sine_wave;

const int sine_map[] = {
		2000,
		2250,
		2497,
		2736,
		2963,
		3175,
		3369,
		3541,
		3688,
		3809,
		3902,
		3964,
		3996,
		3996,
		3964,
		3902,
		3809,
		3688,
		3541,
		3369,
		3175,
		2963,
		2736,
		2497,
		2250,
		2000,
		1749,
		1502,
		1263,
		1036,
		824,
		630,
		458,
		311,
		190,
		97,
		35,
		3,
		3,
		35,
		97,
		190,
		311,
		458,
		630,
		824,
		1036,
		1263,
		1502,
		1749,
};

void write_cmd(char cmd) {
	P1OUT &= 0xF8;

	// upper nibble
	P2OUT = (P2OUT & 0xF0) | (cmd >> 4);
	__delay_cycles(10);
	P1OUT |= BIT0;      // raise enable
	__delay_cycles(10);
	P1OUT &= ~BIT0;   // lower enable

	// lower nibble
	__delay_cycles(10);
	P2OUT = (P2OUT & 0xF0) | (0x0F & cmd);
	__delay_cycles(10);
	P1OUT |= BIT0;
	__delay_cycles(10);
	P1OUT &= ~BIT0;

    __delay_cycles(100000);  // wait a long time, allow operation to complete
    P1OUT &= 0xF8; // clear P1OUT,just keeping don't care lines low
}

void write_data(char data) {
	//	P2OUT = 0;
	P1OUT &= 0xF8;

	P1OUT |= BIT2;
//	P1OUT &= ~BIT0;

		// upper nibble
		P2OUT = (P2OUT & 0xF0) | (data >> 4);
		__delay_cycles(10);
		P1OUT |= BIT0;      // raise enable
		__delay_cycles(10);
		P1OUT &= ~BIT0;   // lower enable

		// lower nibble
		__delay_cycles(10);
		P2OUT = (P2OUT & 0xF0) | (0x0F & data);
		__delay_cycles(10);
		P1OUT |= BIT0;
		__delay_cycles(10);
		P1OUT &= ~BIT0;

	    __delay_cycles(100000);  // wait a long time, allow operation to complete
	    P1OUT &= 0xF8; // clear P1OUT,just keeping don't care lines low
}

/* Writes a string of characters to DDRAM */
write_msg(char* arr) {
	while(*arr) {
		write_data(*arr++);
	}
}


void lcd_setup() {
	/* LCD setup */
	  /* initialize */
	       __delay_cycles(1000000); // delay 1 second after power up

	      /* Write the first command to put it in nibble mode */
	          P1OUT &= 0xF8;
	          P2OUT &= 0xF0;

	          P2OUT |= BIT1;
	          __delay_cycles(10);
	          P1OUT |= BIT0;
	          __delay_cycles(10);
	          P1OUT &= ~BIT0;

	          write_cmd(BIT2 | BIT3 | BIT5); // 2 line mode, display on

	          __delay_cycles(2000000);

	          write_cmd(BIT0 | BIT1 | BIT2| BIT3); // display on, cursor on, blink on
	          __delay_cycles(2000000);

	          write_cmd(BIT0); // display clear
	          __delay_cycles(2000000);

	          write_cmd(BIT1 | BIT2); // increment mode, shift off

	          write_cmd(BIT1); // return home, set DDRAM address to 0x00

	          write_cmd(BIT2 | BIT3 | BIT5);  // function set, put in two line mode, nibble mode
}

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

  P1OUT &= 0x00;               // Shut down everything
  P1DIR |= 0x07;
  P2DIR |= 0x0f;		       // Clear P2DIR

  lcd_setup();
  write_cmd(BIT1);
  write_msg("hello");

//  initBtn1();
//  initBtn23();

  // Init Ports
//  P1DIR |= BIT4;                     // Will use BIT4 to activate /CE on the DAC

 // P1SEL  = BIT7 + BIT5;  // + BIT4;  // These two lines dedicate P1.7 and P1.5
 // P1SEL2 = BIT7 + BIT5; // + BIT4;   // for UCB0SIMO and UCB0CLK respectively

  // SPI Setup
  // clock inactive state = low,
  // MSB first, 8-bit SPI master,
  // 4-pin active Low STE, synchronous
  //
  // 4-bit mode disabled for now
//  UCB0CTL0 |= UCCKPL + UCMSB + UCMST + /* UCMODE_2 */ + UCSYNC;


//  UCB0CTL1 |= UCSSEL_2;               // UCB0 will use SMCLK as the basis for
                                      // the SPI bit clock

  // Sets SMCLK divider to 16,
  // hence making SPI SCLK equal
  // to SMCLK/16 = 1MHz
//  UCB0BR0 |= 0x10;             // (low divider byte)
//  UCB0BR1 |= 0x00;             // (high divider byte)

  // An example of creating another lower frequency SPI SCLK
  //UCB0BR0 |= 0x00;           // (low byte)  This division caused divide by 256
  //UCB0BR1 |= 0x01;           // (high byte) for an SPI SCLK of ~62.5 KHz.

  //UCB0MCTL = 0;              // No modulation => NOT REQUIRED ON B PORT,
                               // would be required if used UCA0

//  UCB0CTL1 &= ~UCSWRST;        // **Initialize USCI state machine**
                               // SPI now Waiting for something to
                               // be placed in TXBUF.

  // No interrupts in this version, so commented out
  //IE2 = UCB0TXIE;            // Enable USCI0 TX interrupt
  //IE2 = UCB0RXIE;            // Enable USCI0 RX interrupt
  //_enable_interrupts();

//  TACTL = TASSEL_2 + MC_1 + ID_3;
//  TACCR0 = HZ_100;
//  __enable_interrupt();

} // end of main

void initBtn1() {
   P1DIR |= BIT0 + BIT6;        // Set P1.0 and P1.6 to output
   P1REN |= BIT3;			   // Enable pullup resistor at P1.3
   P1OUT |= BIT3;
   P1IE |= BIT3;				   // P1.3 interrupt enable (Button 1)
   P1IES |= BIT3;			   // P1.3 Hi/lo edge
   P1IFG &= ~BIT3;			   // P1.3 IFG cleared
}

void initBtn23() {
   P2OUT |= BIT4 + BIT5;        //
   P2REN |= BIT4 + BIT5;		   // Enable pullup resistor at P2.4, P2.5
   P2IE |= BIT4 + BIT5;         // P2.4, P2.5 interrupt enable (Buttons 2, 3)
   P2IES |= BIT4 + BIT5;        // P2.4, P2.5 Hi/lo edge
   P2IFG &= ~BIT4 + ~BIT5;      // P2.4, P2.5 cleared
}


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

void draw_sine_wave() {
   Drive_DAC(sine_map[sine_index++]);
   sine_index %= NUM_SAMPLES;
}

void draw_sawtooth_wave() {
   if(++sawtooth_counter >= sawtooth_clock_counts) {
	  sawtooth_counter = 0;
	  TempDAC_Value = LOW_SQUARE;
   }
   TempDAC_Value += VOLT_STEP;

   Drive_DAC(TempDAC_Value);
}

void draw_square_wave() {
    if(++square_counter >= square_clock_counts) {
    	if(TempDAC_Value == LOW_SQUARE)
    	   TempDAC_Value = HIGH_SQUARE;
    	else
    	   TempDAC_Value = LOW_SQUARE;

    	Drive_DAC(TempDAC_Value);
    	square_clock_counts = NUM_SAMPLES - square_clock_counts;
    	square_counter = 0;
    }
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void something(void) {
	/*P1DIR |= BIT0;
	P1OUT |= BIT0;
	P1OUT &= ~BIT0;
	P1DIR &= ~BIT0;*/
	draw_wave();
	//draw_sawtooth_wave();
}

// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
   if (P1IFG & BIT3) {
	   __delay_cycles(4000000);

	   switch(wave_state) {
	      		case SAWTOOTH:
	      			draw_wave = draw_sawtooth_wave;
	      			wave_state = SINE;
	      			break;
	      		case SINE:
	      			draw_wave = draw_sine_wave;
	      			wave_state = SQUARE;
	      			break;
	      		case SQUARE:
	      			draw_wave = draw_square_wave;
	      			wave_state = SAWTOOTH;
	      			break;
	   }

	   P1IFG &= ~BIT3;
   }
}

// Port 2 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
   if (P2IFG & BIT4) {
	   __delay_cycles(4000000);

       switch (ccr0_state) {
       case HZ_100:
    	   ccr0_state = HZ_200;
    	   break;
       case HZ_200:
    	   ccr0_state = HZ_300;
    	   break;
       case HZ_300:
    	   ccr0_state = HZ_400;
    	   break;
       case HZ_400:
    	   ccr0_state = HZ_500;
    	   break;
       case HZ_500:
    	   ccr0_state = HZ_100;
    	   break;
       }

	   TACCR0 = ccr0_state;

   	   P2IFG &= 0;
   }
   else if (P2IFG & BIT5) {
	   __delay_cycles(4000000);
	   duty_10_counts = (duty_10_counts + 1) % 11;
	   square_clock_counts = duty_10_counts * DUTY_10;
	   square_counter = 0;
	   write_cmd(BIT1);
	   write_msg("asdvasdv");
	   TempDAC_Value = HIGH_SQUARE;
	   Drive_DAC(HIGH_SQUARE);
	   P2IFG &=  0;                     // P1.3 IFG cleared
   }
}

