
#include "../../lpDnLib.h"

/*
 * Clock System Initialization
 */

uint8_t clockStatus;

void configure_clocks(void)
{

  PM5CTL0 &= ~LOCKLPM5;                    // Disable the GPIO power-on default high-impedance mode
                                           // to activate 1previously configured port settings

  __bis_SR_register(SCG0);                   // disable FLL
  CSCTL3 |= SELREF__REFOCLK;                 // Set REFO as FLL reference source
  CSCTL0 = 0;                                // clear DCO and MOD registers
  CSCTL1 &= ~(DCORSEL_7);                    // Clear DCO frequency select bits first
  CSCTL1 |= DCORSEL_3;                       // Set DCO = 8MHz
  CSCTL2 = FLLD_0 + 243;                     // DCODIV = 8MHz
  __delay_cycles(3);
  __bic_SR_register(SCG0);                   // enable FLL
  while(CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1)); // Poll until FLL is locked

  CSCTL4 = SELMS__DCOCLKDIV | SELA__REFOCLK; // set default REFO(~32768Hz) as ACLK source, ACLK = 32768Hz
                                             // default DCODIV as MCLK and SMCLK source

  // Configure UART pins
  P1SEL0 |= BIT4 | BIT5;                      // set 2-UART pin as second function

  // Configure UART
  UCA0CTLW0 |= UCSWRST;
  UCA0CTLW0 |= UCSSEL__SMCLK;

  // Baud Rate calculation
  // 8000000/(16*9600) = 52.083
  // Fractional portion = 0.083
  // User's Guide Table 14-4: UCBRSx = 0x49
  // UCBRFx = int ( (52.083-52)*16) = 1
  UCA0BR0 = 52;                             // 8000000/16/9600
  UCA0BR1 = 0x00;
  UCA0MCTLW = 0x4900 | UCOS16 | UCBRF_1;

    __bis_SR_register(GIE);   // Enable global interrupt

}


// NMI interrupt service routine
#pragma vector=UNMI_VECTOR
__interrupt void NMI_ISR(void)
{
    do
    {
        clockStatus = CS_clearAllOscFlagsWithTimeout(1000);
    }
    // If it still can't clear the oscillator fault flags after the timeout, trap and wait here.
    while (clockStatus != 0);
}

