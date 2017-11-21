
#include "../../lpDnLib/lpDnLib.h"

// BME280
s32 returnRslt = 0;

typedef struct tempPressHumidResults {
   s32 g_s32ActualTemp;
   u32 g_u32ActualPress;
   u32 g_u32ActualHumity;
} tempPressHumidResults_s;

/***********************************************************
 Function: main
 */
void runBme280_Init(uint8_t uart) {
   initI2C();
   multiOneSecondDelayTimer(1);    // Wait a few secs so the user can see the red light transition

   // Enable interrupts
    __enable_interrupt();

   // Initialize bme280 sensor
   bme280_data_readout_template();
   multiOneSecondDelayTimer(1);    // Wait a few secs so the user can see the red light transition
   returnRslt = bme280_set_power_mode(BME280_SLEEP_MODE);
   multiOneSecondDelayTimer(1);    // Wait a few secs so the user can see the red light transition
   //Enable TMP, OPT, and BME sensors
   returnRslt = bme280_set_power_mode(BME280_NORMAL_MODE);
}

s8 readBme280 (tempPressHumidResults_s * results) {
   return bme280_read_pressure_temperature_humidity(&results->g_u32ActualPress, &results->g_s32ActualTemp,
         &results->g_u32ActualHumity);
}

/*
 * Dump sensor results
 * */
void runBme280(uint8_t uart) {

   tempPressHumidResults_s results;

   runBme280_Init(uart);

   while (1) {

      //Read BME environmental data
      returnRslt = bme280_read_pressure_temperature_humidity(&results.g_u32ActualPress, &results.g_s32ActualTemp,
            &results.g_u32ActualHumity);

      uartAXTxString("Results: .", uart);
      uartAXTxInt(results.g_u32ActualPress, uart);
      uartAXTxString(",", uart);
      uartAXTxInt(results.g_s32ActualTemp, uart);
      uartAXTxString(",", uart);
      uartAXTxInt(results.g_u32ActualHumity, uart);
      uartAXTxStringNR("", uart);
      multiOneSecondDelayTimer(2);    // Wait a few secs so the user can see the red light transition

   }
}
