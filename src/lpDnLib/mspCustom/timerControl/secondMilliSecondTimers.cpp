/*
 * secondMilliSecondTimers.cpp
 *
 *  Created on: 16 Aug 2013
 *      Author: a
 */

#include "../../lpDnLib.h"

#define TIMER_LP_LEVEL LPM3_bits
//
#define SECOND_TIMER_COUNT   4096
#define NEARMS_TIMER_COUNT   4
#define SECOND_TIMER_MULTMAX 16

uint8_t timerA0_decrement;

/* Setup our timer
 *
 * */
uint8_t setupAccurateTimer(uint16_t period, uint16_t counterVal) {

   if (period > SECOND_TIMER_MULTMAX) {
      TA0CCR0 = counterVal * 1; // just in case
   } else {
      TA0CCR0 = counterVal * period;
   }

   timerA0_decrement = 0;
   TA0CTL = TASSEL_1 + MC__UP + ID_3 + TACLR; // ACLK, count up, div 8, clear the timer!
   TA0CTL = TASSEL_1 + MC__UP + ID_3;         // ACLK, count up, div 8!
   TA0CCTL0 = CCIE;                           // TACCR0 interrupt enabled
   return 0;
}

/* This allows us to have a 2 ^ 16 delay
 * (65,536 seconds, 1092 minutes, 18 hours!)
 * */
uint8_t multiOneSecondDelayTimer(uint16_t seconds) {

   while (seconds-- > 0) {
      setupAccurateTimer(1, SECOND_TIMER_COUNT);

      __bis_SR_register(TIMER_LP_LEVEL + GIE);
      __no_operation();
   }
   return 0;
}

/* MS timer
 *
 * */
uint8_t multiNearMilliSecondDelayTimer(uint16_t milliSeconds) {

   while (milliSeconds-- > 0) {
      setupAccurateTimer(1, NEARMS_TIMER_COUNT);

      while (timerA0_decrement == 0) {
         __bis_SR_register(TIMER_LP_LEVEL + GIE);
         __no_operation();

      }
   }
   return 0;
}

// Timer A1 interrupt service routine, when this goes off we have not got a reply when
// expected. So we stop and deal with the error
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0_ISR(void) {

   TA0CTL = TASSEL_1 + MC__STOP + ID_3; // ACLK, count STOP, div 8
   timerA0_decrement++;
   __bic_SR_register_on_exit(TIMER_LP_LEVEL);
   __no_operation();
}


uint8_t setupPulseGenTimer() {

   P2DIR  |=  BIT6;
   P2SEL0 &= ~BIT6;
   P2SEL1 &= ~BIT6;

   TA1CCR0 = 512;
   TA1CTL = TASSEL_1 + MC__UP + ID_1 + TACLR; // ACLK, count up, div 2, clear the timer!
   TA1CTL = TASSEL_1 + MC__UP + ID_1;         // ACLK, count up, div 2!
   TA1CCTL0 = CCIE;                           // TACCR0 interrupt enabled
   return 0;
}

// Timer A1 interrupt service routine, when this goes off we have not got a reply when
// expected. So we stop and deal with the error
#pragma vector = TIMER1_A0_VECTOR
__interrupt void Timer1_A0_ISR(void) {

   TA1CTL = TASSEL_1 + MC__UP + ID_1 + TACLR; // ACLK, count up, div 2, clear the timer!
   TA1CTL = TASSEL_1 + MC__UP + ID_1;         // ACLK, count up, div 2!
   P2OUT ^= BIT6;
//   __bic_SR_register_on_exit(TIMER_LP_LEVEL);
//   __no_operation();
}

