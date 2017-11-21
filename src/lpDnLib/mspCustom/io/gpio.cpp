/*
 * gpio.c
 *
 *  Created on: 27 Jan 2017
 *      Author: a
 */
#include <msp430.h>
#include <stdint.h>

#include "../../lpDnLib.h"

void InitGpioBank(uint8_t bank, uint16_t outMask, uint16_t inMask) {
  GPIO_setOutputLowOnPin(bank, outMask);
  GPIO_setAsOutputPin(bank, outMask);
}

/*
 * Fast entry into total LP mode. Further savings may be avaiable on the
 * P1.0 P1.1 & P3.2
 * */
void Init_UartGpio(uint8_t bank, uint8_t txMask, uint16_t rxMask, uint8_t mf) {
  GPIO_setOutputLowOnPin(bank, txMask);
  GPIO_setAsOutputPin(bank, txMask);
  GPIO_setAsPeripheralModuleFunctionOutputPin(bank, txMask, mf);
  GPIO_setAsPeripheralModuleFunctionInputPin(bank, rxMask, mf);
}

void Init_GPIO() {
  //-------------------------------------------------------------------------
  // Initial PORT defines
  //-------------------------------------------------------------------------
  InitGpioBank(GPIO_PORT_P1, P1_OUT_DEF, P1_IN_DEF);
  InitGpioBank(GPIO_PORT_P2, P2_OUT_DEF, P2_IN_DEF);
  InitGpioBank(GPIO_PORT_P3, P3_OUT_DEF, P3_IN_DEF);
#ifdef USE_LP_XTAL
  // Set Px.x and Px.x as Primary Module Function Input, LFXT.
//   GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_PJ, GPIO_PIN4 + GPIO_PIN5, GPIO_PRIMARY_MODULE_FUNCTION);
#endif

  // Disable the GPIO power-on default high-impedance mode
  // to activate previously configured port settings
  PMM_unlockLPM5();
}

void set_GPIO_LpMode() {
  //-------------------------------------------------------------------------
  // Initial PORT defines
  //-------------------------------------------------------------------------
  InitGpioBank(GPIO_PORT_P1, P1_OUT_DEF_LP, 0x0);
  InitGpioBank(GPIO_PORT_P2, P2_OUT_DEF_LP, 0x0);
  InitGpioBank(GPIO_PORT_P3, P3_OUT_DEF_LP, 0x0);
}

void Init_UARTS_GPIO() {
  Init_UartGpio(UARTA0_PORT, UARTA0_TXPIN, UARTA0_RXPIN, GPIO_SECONDARY_MODULE_FUNCTION);
  Init_UartGpio(UARTA1_PORT, UARTA1_TXPIN, UARTA1_RXPIN, GPIO_PRIMARY_MODULE_FUNCTION);
}
