/*
 * app_00_blinkPrint.cpp
 *
 *  Created on: 19 Nov 2017
 *      Author: a
 */

#ifndef SRC_APPS_APP_00_BLINKPRINT_APP_00_BLINKPRINT_CPP_
#define SRC_APPS_APP_00_BLINKPRINT_APP_00_BLINKPRINT_CPP_

#include "../../lpDnLib/lpDnLib.h"

#define EXPERIMENT1
//#define EXPERIMENT2
//#define EXPERIMENT3

void runApp0(uint8_t uart) {
  int counter=0;

  while (1) {

#ifdef EXPERIMENT1
    led_flip_blue();
    dump_spacer(EXT_UART);                                       // as on tin
    uartAXTxString("I am LpDataNode_v1 .... goldfish---: ", EXT_UART);// a string
    uartAXTxInt(counter, EXT_UART);                              // an int
    uartAXTxStringNR("", EXT_UART);                              // New line
    multiOneSecondDelayTimer(1);
    led_flip_blue();
    counter++;
#endif

#ifdef EXPERIMENT2
    led_set_blue();
    multiNearMilliSecondDelayTimer(100);
    led_clear_blue();
    multiNearMilliSecondDelayTimer(900);
#endif

#ifdef EXPERIMENT3
    multiOneSecondDelayTimer(1);
#endif

  }

}

#endif /* SRC_APPS_APP_00_BLINKPRINT_APP_00_BLINKPRINT_CPP_ */
