/*
 * main.c
 */
#include "msp430g2553.h"
unsigned int getDataCount = 0;
unsigned int coordX = 0;
unsigned int coordY = 0;
unsigned int pwmX = 0;
unsigned int pwmY = 0;
unsigned int zeroX = 512;
unsigned int zeroY = 512;
unsigned int zerooffsetX = 10;
unsigned int zerooffsetY = 10;
unsigned int dirX = 0;
unsigned int dirY = 0;
void MCUInit(void)
{
	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL = CALDCO_1MHZ;

	P2DIR |= BIT1;             // P2.1 to output
	P2SEL |= BIT1;             // P2.1 to TA1.1
	P2DIR |= BIT6;
	P2SEL |= BIT6;
	P2SEL2 &= ~BIT6;
	P2SEL &= ~BIT7;
	P2SEL2 &= ~BIT7;

	P1SEL |= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
	P1SEL2|= BIT6 + BIT7;
}
void TimerInit(void)
{
	TA0CCR0 = 6144;             // PWM Period12288;
	TA0CCTL1 = OUTMOD_7 + CCIE;          // CCR1 reset/set
	TA0CCR1 = 1550;                // CCR1 PWM duty cycle
	TA0CTL = TASSEL_2 + MC_1;   // SMCLK, up mode;
	TA0CCTL1 &= ~CCIE;

	TA1CCR0 = 6144;             // PWM Period12288;
	TA1CCTL1 = OUTMOD_7 + CCIE;          // CCR1 reset/set
	TA1CCR1 = 1550;                // CCR1 PWM duty cycle
	TA1CTL = TASSEL_2 + MC_1;   // SMCLK, up mode;
	TA1CCTL1 &= ~CCIE;
}
void I2C_masterInit(void)
{
	UCB0CTL1 |= UCSWRST;                      // Enable SW reset
	UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
	UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
	UCB0BR0 = 10;                             // fSCL = SMCLK/12 = ~100kHz
	UCB0BR1 = 0;
	UCB0I2CIE |= UCNACKIE;
	UCB0I2CSA = 0x70;
}

void I2C_enable(void)
{
	UCB0CTL1 &= ~UCSWRST;
}
unsigned int getData(unsigned int regAdr)
{
	unsigned int rxData = 0;
	UCB0TXBUF = 0;
	UCB0CTL1 |= UCTR;
	UCB0CTL1 |= UCTXSTT;
	while (!(IFG2 & UCB0TXIFG));
	UCB0TXBUF = regAdr;
	while (!(IFG2 & UCB0TXIFG));
	UCB0CTL1 &= ~UCTR;
	UCB0CTL1 |= UCTXSTT;
	while (UCB0CTL1 & UCTXSTT);
	rxData = UCB0RXBUF;
	UCB0CTL1 |= UCTXSTP;
	while (UCB0CTL1 & UCTXSTP);
	return rxData;
}
int main(void) {
	__bis_SR_register(GIE);
	MCUInit();
	I2C_masterInit();
	TimerInit();
	I2C_enable();
	//TA0CCTL1 |= CCIE;
	TA1CCTL1 |= CCIE;
	while (1)
	{

	}
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR (void)
{
	TA0CCTL1 &= ~CCIE;
	coordY = (getData(0x23)<<8) | getData(0x22);
	TA0CCR1 = coordY*6;
	TA0CCTL1 |= CCIE;
}

#pragma vector=TIMER1_A1_VECTOR
__interrupt void TIMER1_A1_ISR (void)
{
	TA1CCTL1 &= ~CCIE;
	coordX = (getData(0x21)<<8) | getData(0x20);
	pwmX = (coordX*coordX)/50;
	if (pwmX*6 <= TA1R) {
		if ((coordX <= zeroX + zerooffsetX) || (coordX <= zeroX - zerooffsetX)) {
			dirX = 1;
		}
		if ((coordX >= zeroX + zerooffsetX) || (coordX >= zeroX - zerooffsetX)) {
			dirX = 0;
		}
		TA1CCR1 = coordX*6;
	}
	//TA1CCR1 = coordX*6;
	coordY = (getData(0x23)<<8) | getData(0x22);
	pwmY = (coordY*coordY)/50;
	if (pwmX*6 <= TA1R) {
		if ((coordY <= zeroY + zerooffsetY) || (coordY <= zeroY - zerooffsetY)) {
			dirY = 1;
		}
		if ((coordY >= zeroY + zerooffsetY) || (coordY >= zeroY - zerooffsetY)) {
			dirY = 0;
		}
		TA0CCR1 = coordY*6;
	}
	//TA0CCR1 = coordY*6;
	TA1CCTL1 |= CCIE;
}
