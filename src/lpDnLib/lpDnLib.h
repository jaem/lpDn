/*
 * lpDnLib.h
 *
 *  Created on: 18 Nov 2017
 *      Author: a
 */

#ifndef SRC_LPDNLIB_LPDNLIB_H_
#define SRC_LPDNLIB_LPDNLIB_H_

#define FR2433_LP
//#define LPDN_V1_0

#include <msp430.h>
#include <stdint.h>

#include "MSP430FR2xx_4xx/driverlib.h"

// lpDn custom functions
#include "mspCustom/gp/genPurposeRoutines.h"
#include "mspCustom/clocks/clocks_lpdn.h"
#include "mspCustom/comms/i2c_driver.h"

// Sensors, this should maybe in another header
#include "sensors/bme280/bme280_support.h"

// rev this for versions.
#include "lpDnLib_v1.h"

/*
 * Sleep functions, interrupt driven for low power
 * */
uint8_t multiOneSecondDelayTimer(uint16_t);
uint8_t multiNearMilliSecondDelayTimer(uint16_t);
// Timer based on ACLK for data sampling
uint8_t setupAccurateTimer(uint16_t, uint16_t);
uint8_t setupPulseGenTimer();

void Init_GPIO(void);

/*
 * Leds calls
 * */
void led_flip_blue(void);
void led_set_blue(void);
void led_clear_blue(void);

/*
 * UART functions
 * */
void setBaudRateSm8MHz(uint32_t);
void Init_Single_UART(uint16_t);

void Init_UART();
void dump_spacer(uint8_t);

void InitGpioBank(uint8_t, uint16_t, uint16_t);
void Init_UartGpio(uint8_t, uint8_t, uint16_t, uint8_t);
void set_GPIO_LpMode();
void uartA0TxChar(char *);
void uartAXTxChar(char *, uint8_t);
void uartAXTxStringNR(char *, uint8_t);
void uartAXTxString(char *, uint8_t);
int uartAXTxInt(int, uint8_t);
void uartA1TxChar(char *);
void initialiseUartRxStruct();
void setUartEndChar(uint8_t, char);
void enableSingleCharBuffer(uint8_t);
void disableSingleCharBuffer(uint8_t);
uint16_t checkReadBufferForReply(char *, uint8_t, uint8_t, uint8_t);


/*
 * UART RX control
 *
 * */
#define UART_RX_BUF_COUNT 2
#define UART_RX_BUF_SIZE  80

typedef struct {
   char rxBuffer[UART_RX_BUF_COUNT][UART_RX_BUF_SIZE];
//   const uint8_t maxBufferSize = UART_RX_BUF_SIZE; // programmatically, wastes a byte
   uint8_t writeCharCounter; // This must be big enough to accommodate UART_RX_BUF_SIZE
   uint8_t readBufferPtr;
   uint8_t writeBufferPtr;
   uint8_t readBufferCount;
   uint8_t readBufferReady;
   uint8_t writeOverflow;
   uint8_t everyCharNewBuffer;
   uint8_t enableRx;
   char endCharType;

} UartRxLane_t;

/*
 * Redefine this struct based on no of UARTS
 *
 * */
typedef struct UartRxManager_t {
   UartRxLane_t u0;
   UartRxLane_t u1;
} UartRxManager_t;

/*
 * UART Application methods
 *
 * */
uint16_t checkReadBufferForReply(char *, uint8_t, uint8_t);
UartRxLane_t * returnUartPtr(uint8_t);
void clearRxBuffers(UartRxLane_t *);
void clearRxBufferContent(uint8_t);
void disableUartRx(UartRxLane_t *);
char getReadBufferChar(uint8_t, uint8_t);

/*
 * RTC
 *
 * */
void setupRtc(void);


#endif /* SRC_LPDNLIB_LPDNLIB_H_ */
