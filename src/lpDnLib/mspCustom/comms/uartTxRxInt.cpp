#include <msp430.h>
#include <stdint.h>

#include "../../lpDnLib.h"

EUSCI_A_UART_initParam param; // Global for initial configuration
UartRxManager_t UartRxCtrl;   //

/*
 *
 *
 *
 * */

UartRxLane_t * returnUartPtr(uint8_t uart) {
   switch (uart) {
   case 1:
      return &UartRxCtrl.u1;
   }
   return &UartRxCtrl.u0;
}

void setUartEndChar(uint8_t uart, char endType) {

   UartRxLane_t * uPtr;
   uPtr = returnUartPtr(uart);
   uPtr->endCharType = endType;

}

/*
 * char * string       String to search for
 * uint8_t startChar   Starting address in buffer to match
 * uint8_t testLength  Length of the match to perform
 * uint8_t uart        Uart to check buffer of
 *
 * */
uint16_t checkReadBufferForReply(char * string, uint8_t startChar, uint8_t testLength, uint8_t uart) {

   UartRxLane_t * uPtr;
   uint16_t returnGotMatch = 0;
   uint8_t localMatchChar = 0;
   uPtr = returnUartPtr(uart);
   while (localMatchChar < testLength) {
      if (*(string + localMatchChar) != *(uPtr->rxBuffer[uPtr->readBufferPtr] + startChar)) {
         returnGotMatch = 1;
      }
      startChar++;
      localMatchChar++;
   }
   return returnGotMatch; // Returns 1 on mismatch
}

char getReadBufferChar(uint8_t position, uint8_t uart) {

   UartRxLane_t * uPtr;
   uPtr = returnUartPtr(uart);
   return *(uPtr->rxBuffer[uPtr->readBufferPtr]);

}

void enableSingleCharBuffer(uint8_t uart) {

   UartRxLane_t * uPtr;
   uPtr = returnUartPtr(uart);
   uPtr->everyCharNewBuffer = 1;

}

void disableSingleCharBuffer(uint8_t uart) {

   UartRxLane_t * uPtr;
   uPtr = returnUartPtr(uart);
   uPtr->everyCharNewBuffer = 0;

}

void clearRxBuffers(UartRxLane_t * rxLane) {

   rxLane->readBufferCount = 0;
   rxLane->writeCharCounter = 0;
   rxLane->readBufferPtr = 0;
   rxLane->writeBufferPtr = 0;
   rxLane->writeOverflow = 0;
   rxLane->readBufferReady = 0;

}

void clearRxBufferContent(uint8_t uart) {

   UartRxLane_t * uPtr;
   uPtr = returnUartPtr(uart);
   uint8_t buffer = 0;
   uint8_t entry = 0;

   while (buffer < UART_RX_BUF_COUNT) {
      entry=0;
      while (entry < UART_RX_BUF_SIZE) {
         uPtr->rxBuffer[buffer][entry++]=0;
      }
      buffer++;
   }

   clearRxBuffers(uPtr); // Clear the counters

   uPtr->enableRx = 1; // Re-enable the lane

}

void disableUartRx(UartRxLane_t * rxLane) {

   rxLane->enableRx = 0; // disable the lane

}


void initialiseUartRxLane(UartRxLane_t * rxLane) {

   rxLane->readBufferCount = 0;
   rxLane->writeCharCounter = 0;
   rxLane->readBufferPtr = 0;
   rxLane->writeBufferPtr = 0;
   rxLane->writeOverflow = 0;
   rxLane->everyCharNewBuffer = 0;
   rxLane->endCharType = '\r';
   rxLane->enableRx = 1;

}

void initialiseUartRxStruct() {

   initialiseUartRxLane(&UartRxCtrl.u0); // A0
   initialiseUartRxLane(&UartRxCtrl.u1); // A1

}

/*
 * String/Char send methods. At the minute these effectively block by polling. Give
 * option to block VIA interrupt.
 *
 * */
void uartA0TxChar(char * aCharToSend) {
   while (!(UCA0IFG & UCTXIFG))
      ; // USCI_A0 TX buffer ready?
   UCA0TXBUF = *aCharToSend;
}
void uartA1TxChar(char * aCharToSend) {
   while (!(UCA1IFG & UCTXIFG))
      ; // USCI_A1 TX buffer ready?
   UCA1TXBUF = *aCharToSend;
}

void uartAXTxChar(char * string, uint8_t uart) {
   switch (uart) {
   case 0:
      uartA0TxChar(string);
      break;
   case 1:
      uartA1TxChar(string);
      break;
   }
}

void uartAXTxStringNR(char * string, uint8_t uart) {
   while ((*string != '\0')) { /* line A (see text) */
      uartAXTxChar(string, uart);
      string++;
   }
   uartAXTxChar("\n", uart);
#if DIALOG_UART != UART_SEL_BLE
   uartAXTxChar("\r", uart);
#endif
}

void uartAXTxString(char * string, uint8_t uart) {
   while ((*string != '\0')) { /* line A (see text) */
      uartAXTxChar(string, uart);
      string++;
   }
}

void dump_spacer(uint8_t uart) {
  uint16_t sLength = 50;
  while (sLength-- != 0) {
    uartAXTxString("*", uart);
  }
  uartAXTxStringNR("", uart);
}

// send string to the debug port. On the MSP this is Port B?
// On Windows or Linux we just print to screen
int uartAXTxInt(int value, uint8_t uart) {
   char string[6];
   int length;
   length = loadBufferWithDecimalStringRetLength(string, value);
   uartAXTxString(string, uart);
   return length;
}

void setBaudRateSm8MHz(uint32_t baudrate) {
   param.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
   param.overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;

   switch (baudrate) {
   case 9600: // UCOS16=1 UCBRx=52 UCFx=1 UCSx=0x49
      param.clockPrescalar = 52;
      param.firstModReg = 1;
      param.secondModReg = 0x49;
      break;

   case 57600: // UCOS16=1 UCBRx=8 UCFx=1 UCSx=0xF7
      param.clockPrescalar = 8;
      param.firstModReg = 11;
      param.secondModReg = 0xF7;
      break;

   case 115200: // UCwOS16=1 UCBRx=4 UCFx=5 UCSx=0x55
      param.clockPrescalar = 4;
      param.firstModReg = 5;
      param.secondModReg = 0x55;
      break;
   }
}

/*
 * UART Communication Initialization
 */
void Init_Single_UART(uint16_t baseAddr) {
   if (STATUS_FAIL == EUSCI_A_UART_init(baseAddr, &param))
      return;

   EUSCI_A_UART_enable(baseAddr);
   EUSCI_A_UART_clearInterrupt(baseAddr, EUSCI_A_UART_RECEIVE_INTERRUPT);
   EUSCI_A_UART_enableInterrupt(baseAddr, EUSCI_A_UART_RECEIVE_INTERRUPT); // Enable interrupt

}

/*
 * UART Communication Initialization
 */
void Init_UART() {
   param.parity = EUSCI_A_UART_NO_PARITY;
   param.msborLsbFirst = EUSCI_A_UART_LSB_FIRST;
   param.numberofStopBits = EUSCI_A_UART_ONE_STOP_BIT;
   param.uartMode = EUSCI_A_UART_MODE;

//   setBaudRateSm8MHz(115200);
   setBaudRateSm8MHz(9600);
   Init_Single_UART(EUSCI_A0_BASE); //
   Init_Single_UART(EUSCI_A1_BASE); //

}

/* ****************************************************************************
 * INTERRUPT SECTION
 *
 * */

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=EUSCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
#elif defined(__GNUC__)
      void __attribute__ ((interrupt(EUSCI_A0_VECTOR))) USCI_A0_ISR (void)
#else
#error Compiler not supported!
#endif
      {
   UartRxLane_t * uPtr;
   uPtr = &UartRxCtrl.u0;
   switch (__even_in_range(UCA0IV, USCI_UART_UCTXCPTIFG)) {
   case USCI_NONE:
      break;
   case USCI_UART_UCRXIFG:
      uPtr->rxBuffer[uPtr->writeBufferPtr][uPtr->writeCharCounter] = UCA0RXBUF;
      /**/
      if (uPtr->enableRx) {
         if (((uPtr->rxBuffer[uPtr->writeBufferPtr][uPtr->writeCharCounter] == uPtr->endCharType) & (uPtr->writeCharCounter != (UART_RX_BUF_SIZE - 1))) | uPtr->everyCharNewBuffer) {
            uPtr->writeCharCounter++; // Null next value
            uPtr->rxBuffer[uPtr->writeBufferPtr][uPtr->writeCharCounter] = 0; //NULL
            uPtr->writeCharCounter = 0;
            uPtr->readBufferPtr = uPtr->writeBufferPtr;
            uPtr->writeBufferPtr = (uPtr->writeBufferPtr + 1) & 0x1;
            uPtr->readBufferCount++;
         } else {
            if (uPtr->writeCharCounter == (UART_RX_BUF_SIZE - 1)) {
               uPtr->rxBuffer[uPtr->writeBufferPtr][uPtr->writeCharCounter] = 0; // Null terminate string
               uPtr->writeCharCounter = 0;
               uPtr->readBufferPtr = uPtr->writeBufferPtr;
               uPtr->writeBufferPtr = (uPtr->writeBufferPtr + 1) & 0x1;
               uPtr->readBufferCount++;
               uPtr->writeOverflow++;
            } else {
               uPtr->writeCharCounter++;
            }
         }
      }
//            __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0 on reti
      break;
   case USCI_UART_UCTXIFG:
      break;
   case USCI_UART_UCSTTIFG:
      break;
   case USCI_UART_UCTXCPTIFG:
      break;
   default:
      break;
   }
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=EUSCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
#elif defined(__GNUC__)
      void __attribute__ ((interrupt(EUSCI_A1_VECTOR))) USCI_A1_ISR (void)
#else
#error Compiler not supported!
#endif
      {
   UartRxLane_t * uPtr;
   uPtr = &UartRxCtrl.u1;
   switch (__even_in_range(UCA1IV, USCI_UART_UCTXCPTIFG)) {
   case USCI_NONE:
      break;
   case USCI_UART_UCRXIFG:
//      RXDataUA1 = UCA1RXBUF;             // Read buffer
//      UCA0TXBUF = RXDataUA1;
      uPtr->rxBuffer[uPtr->writeBufferPtr][uPtr->writeCharCounter] = UCA1RXBUF;
      if (uPtr->enableRx) {
         if (((uPtr->rxBuffer[uPtr->writeBufferPtr][uPtr->writeCharCounter] == uPtr->endCharType) & (uPtr->writeCharCounter != (UART_RX_BUF_SIZE - 1))) | uPtr->everyCharNewBuffer) {
            uPtr->writeCharCounter++; // Null next value
            uPtr->rxBuffer[uPtr->writeBufferPtr][uPtr->writeCharCounter] = 0; //NULL
            uPtr->writeCharCounter = 0;
            uPtr->readBufferPtr = uPtr->writeBufferPtr;
            uPtr->writeBufferPtr = (uPtr->writeBufferPtr + 1) & 0x1;
            uPtr->readBufferCount++;
         } else {
            if (uPtr->writeCharCounter == (UART_RX_BUF_SIZE - 1)) {
               uPtr->rxBuffer[uPtr->writeBufferPtr][uPtr->writeCharCounter] = 0; // Null terminate string
               uPtr->writeCharCounter = 0;
               uPtr->readBufferPtr = uPtr->writeBufferPtr;
               uPtr->writeBufferPtr = (uPtr->writeBufferPtr + 1) & 0x1;
               uPtr->readBufferCount++;
               uPtr->writeOverflow++;
            } else {
               uPtr->writeCharCounter++;
            }
         }
      }
//            __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0 on reti
      break;
   case USCI_UART_UCTXIFG:
      break;
   case USCI_UART_UCSTTIFG:
      break;
   case USCI_UART_UCTXCPTIFG:
      break;
   default:
      break;
   }
}

