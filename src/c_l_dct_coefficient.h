/***************************************************************************
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 ***************************************************************************/
/*******LOGs
 *
 *	Tuesday, 13 September 2005:
 * I still don't know which kind of variable is suitable for storing
 * a DCT Coefficient, for now I defined C_L_WetDCT_Coefficient 
	
 */
#ifndef C_L_DCT_COEFFICIENT_H
#define C_L_DCT_COEFFICIENT_H

#include "common.h"

/**
This class is storing a single DCT coefficient with corresponding properties which help the SR to decide to involve it in quantization process or not. 

*/
/* I found that using such class is too inefficient*/
/*class C_L_DCT_Coefficient{
protected:
	C_L_WetDCT_CoefficientValue m_Value;

public:
    C_L_DCT_Coefficient();

    ~C_L_DCT_Coefficient();

//Interface functions



};*/

/**
This class store the whole collection of DCT coefficient of a JPEG Image. 
Each Block is a 64 array of C_L_DCT_Coefficients The classe is arrray of 
such array.

*/

class C_L_DCT_CoefficientCollection
{
public:
	T_L_WetDCT_Coefficient* m_pDCT_Coefficient; /* The Jpeg coefficient collection 
			can be copied in side it directly is an array of n*64 dimansion*/
	T_L_BufferDimension m_NoOfMembers; //Number of Coefficients in the Collection;

	//C_L_WetDCT_Coefficient* m_pHumidity; 	 
	/*regarding the method descirebed in PQ Paper we don't need this 
		factor*/
					
	/* The different layer of image show different capacity for steganographic 
		application, as the most Color JPEG Images are save in Luminomatic/Chromatic
		format, we should this layer differently. For the simplicity reason we 
		just embed information in luminomatic layer
	*/
};

#endif
