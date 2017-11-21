/*
 * leds.cpp
 *
 *  Created on: 9 Apr 2017
 *      Author: andy
 *
 *  LED control functions.
 *  Basic Set/Clear/Flip
 *
 */

#include <msp430.h>

#define LED_BLUE_BIT BIT0

void led_flip_blue(void) {
   P1OUT ^= LED_BLUE_BIT;
}

void led_set_blue(void) {
   P1OUT |= LED_BLUE_BIT;
}

void led_clear_blue(void){
   P1OUT &= ~LED_BLUE_BIT;
}
