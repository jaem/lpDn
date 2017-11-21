/*
 * genPurposeRoutines.h
 *
 *  Created on: 27 Feb 2012
 *      Author: a
 */

#ifndef GENPURPOSEROUTINES_H_
#define GENPURPOSEROUTINES_H_

int copyConstStrToBuffer(char *, const char *);
int loadBufferWithDecimalStringRetLength(char *, int);
int charBufferStringToInt(char *, int);
int myItoa(unsigned int, char *, unsigned int, int);
int getMultiplier(int);
int fillInDateTimeBufferWithInt(char *, int, int, int, int);

#endif /* GENPURPOSEROUTINES_H_ */
