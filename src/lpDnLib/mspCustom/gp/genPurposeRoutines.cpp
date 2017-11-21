/*
 * genPurposeRoutines.cpp
 *
 *  Created on: 27 Feb 2012
 *      Author: a
 */

int copyConstStrToBuffer(char * string, const char * constString) {
   int length = 0;
   while ((*constString != '\0')) { /* line A (see text) */
      *string++ = *constString++;
      length++;
   }
   *string = 0; // very important that we assign NULL here, string end!
   return length;
}

/* Multiplier for integer conversion in 16Bits, 65xxx being the largest possible number.
 *
 * */
int getMultiplier(int length) {
   int mult = 1;
   switch (length) {
   case 1:
      mult = 10;
      break;
   case 2:
      mult = 100;
      break;
   case 3:
      mult = 1000;
      break;
   case 4:
      mult = 10000;
   }
   return mult;
}

// base-10 itoa for positive numbers. Populates str with a null-terminated string.
// limit lets you overflow back to 1 when you hit the limit+1, 2*limit+1, ...
// make sure *str is an array that can hold the number of digits in your limit + 1.
int myItoa(unsigned int val, char *str, unsigned int limit, int endWithNull) {
   int temploc = 0;
   int digit = 0;
   int strloc = 0;
   char tempstr[5]; //16-bit number can be at most 5 ASCII digits;

   if (val > limit)
      val %= limit;

   do {
      digit = val % 10;
      tempstr[temploc++] = digit + '0';
      val /= 10;
   } while (val > 0);

   // reverse the digits back into the output string
   while (temploc > 0) {
      *str++ = tempstr[--temploc];
      strloc++;
   }
   if (endWithNull == 1) {
      *str = 0;
   }
   return strloc;
}

/* This function has no protection built in to check that buffers are not over-running.
 * Keep this in mind when passing value. Ints on the MSP430 should be 16 bit so there should
 * bee a requirement for only 6 chars including the NULL terminator.
 * */
int loadBufferWithDecimalStringRetLength(char * inputString, int integerInput) {

   // The 10 is the number base, 10 being decimal, see itoa C++ reference for more detail
   int count = myItoa(integerInput, inputString, 0xffff, 1);
   return count;
}

/* Simple method to convert a char buffer to a 16 bit integer on an MSP platform
 * */
int charBufferStringToInt(char * string, int length) {
   int value = 0;

   while (length-- > 0) {
      value += (*string++ - '0') * getMultiplier(length);
   }

   return value;
}

/* We use this routine to load a buffer at a certain point with a date time string.
 * This is a wee bit special as we need to insert leading zeros and don't want to use
 * sprintf etc! All date/times are 2 digits each in the format hhmmss & ddmmyy
 * from 2000. This is how we get them from the GPS and will standardise on these.
 *
 * */
int fillInDateTimeBufferWithInt(char * buffer, int bufLen, int bufOffset, int value, int maxBufferLength) {

   if (value > 9) {
      loadBufferWithDecimalStringRetLength(buffer + bufOffset, value);
   } else {
      loadBufferWithDecimalStringRetLength(buffer + bufOffset, 0);
      loadBufferWithDecimalStringRetLength(buffer + bufOffset + 1, value);
   }
   *(buffer + bufLen - 1) = 0; // for safety!
   return 0;

}
