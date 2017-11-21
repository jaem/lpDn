// Test

// Load the library for LpDn
#include "lpDnLib/lpDnLib.h"

// APPS, you can remove these simple test ones
#include "apps/app_00_blinkPrint/app_00_blinkPrint.h"
#include "apps/app_03_bme280/app_03_bme280.h"

void main(void) {
  // Stop Watchdog Timer
  WDT_A_hold(WDT_A_BASE);

  // Setup clock configuration
  configure_clocks();

  Init_GPIO();
  Init_UART();
  Init_UartGpio(UARTA0_PORT, UARTA0_TXPIN, UARTA0_RXPIN, GPIO_PRIMARY_MODULE_FUNCTION);
  Init_UartGpio(UARTA1_PORT, UARTA1_TXPIN, UARTA1_RXPIN, GPIO_PRIMARY_MODULE_FUNCTION);

  // Disable the GPIO power-on default high-impedance mode to activate
  // previously configured port settings
  PM5CTL0 &= ~LOCKLPM5;

  // Enable interrupts
  __enable_interrupt();

  // Select your app to run
  runApp0(EXT_UART);
//  runBme280(EXT_UART);

}

#if 0
// Enable this if you have a lot of objects that need to be initialised.
// Otherwise watchdog will trigger.
int _system_pre_init(void)
{
  // stop WDT
  WDTCTL = WDTPW + WDTHOLD;

  // Perform C/C++ global data initialization
  return 1;
}
#endif
