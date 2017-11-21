/*
 * runBmp.h
 *
 *  Created on: 30 Oct 2017
 *      Author: a
 */

#ifndef APP_03_BME280_H_
#define APP_03_BME280_H_

#include "../../lpDnLib/lpDnLib.h"

typedef struct tempPressHumidResults {
   s32 g_s32ActualTemp;
   u32 g_u32ActualPress;
   u32 g_u32ActualHumity;
} tempPressHumidResults_s;

void runBme280_Init(uint8_t);
s8 readBme280 (tempPressHumidResults_s *);
void runBme280 (uint8_t);

#endif /* APP_03_BME280_H_ */
