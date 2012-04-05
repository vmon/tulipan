/***************************************************************************
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 ***************************************************************************/

/* This file contains the body of the global functions and the 
		valuation of global constants.
*/
#include "common.h"

const unsigned int BINARY_STREAM_BLOCK_SIZE = sizeof(T_L_BinaryStreamBlock) * 8;
const unsigned int BYTES_IN_BINARY_STREAM_BLOCK =  sizeof(T_L_BinaryStreamBlock);
/*																		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, a, b, c, d, e,f */
unsigned char pADD_BIT_TOGETHER[] = {  0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 };

/*! Simply returns back the ceilling of a / b when both a and b are integers*/
unsigned long CeillingDivide(unsigned long a, unsigned long b)
{
	return (a % b) ? a / b + 1 : a / b;
	
}

/*! This function returns the least multiple of BlockSize which is greater or 
	equal to RoughValue 
*/
unsigned long LeastGreaterMultiple(unsigned long RoughValue, unsigned int BlockSize)
{
	unsigned long Remainder =  RoughValue % BlockSize;
	return RoughValue + ((Remainder) ? BlockSize - Remainder : 0);

}
