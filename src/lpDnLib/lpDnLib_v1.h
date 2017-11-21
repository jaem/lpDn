/*
 * lpDnLib_v1.h
 *
 *  Created on: 19 Nov 2017
 *      Author: a
 */

#ifndef SRC_LPDNLIB_LPDNLIB_V1_H_
#define SRC_LPDNLIB_LPDNLIB_V1_H_

#define P1_OUT_DEF 0xff
#define P1_IN_DEF  ~P1_OUT_DEF
#define P1_OUT_DEF_LP 0x00

#define P2_OUT_DEF 0xff
#define P2_IN_DEF  ~P2_OUT_DEF
#define P2_OUT_DEF_LP 0x00

#define P3_OUT_DEF 0xff
#define P3_IN_DEF  ~P3_OUT_DEF
#define P3_OUT_DEF_LP 0x00

/*
 * Define the UART function pins
 *
 * */
#define UARTA0_PORT  1
#define UARTA0_TXPIN BIT4
#define UARTA0_RXPIN BIT5

#define UARTA1_PORT  2
#define UARTA1_TXPIN BIT6
#define UARTA1_RXPIN BIT5

/*
 * UART function to physical mapping
 * */
#define EXT_UART      0
#define LORA_BLE_UART 1

#endif /* SRC_LPDNLIB_LPDNLIB_V1_H_ */
