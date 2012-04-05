/***************************************************************************
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 ***************************************************************************/
/******LOGs
 * 
 * Thursday, September 22, 2005:
 * I have added the number of matrix as a part of the secret key.
 * it is used to obtain the column dimension of the dynamic matrix. As the 
 * original algorithm suggests we use the ceiling of the n/\beta for the
 * number of columns
 * 
 */

#ifndef COMMON_H
#define COMMON_H
#include <memory.h>
#include <assert.h>

#include "jpeglib.h"

/** This file contains the common definitons of data types used frequently in
	the program and include the frequently used header files.
*/

//Types concerning Binary Matrix Operations
typedef unsigned char T_L_BinaryStreamBlock;		/* The building block
	of the binary stream of a matrix, a bin matrix is consisted 
	of consective blocks  of this type. the two dimensional
	definition is obtained the width and row operation. */
extern const unsigned int BINARY_STREAM_BLOCK_SIZE;
extern const unsigned int BYTES_IN_BINARY_STREAM_BLOCK;
extern unsigned char pADD_BIT_TOGETHER[];
#define BYTE_SIZE 8

typedef JDIMENSION T_L_MatrixDimension; /* This  type is used to indicate 
	the no of rows and columns of As the JPEG images are designed to support 
	up to 64Kx64K coefficients, unsigned int should be enough */
typedef unsigned long T_L_BufferDimension; /* This type is used to indicate
	the whole number of coefficient in JPEG images and corresponding process*/

//Types concerning DCT coefficients
typedef JCOEF T_L_WetDCT_Coefficient;
typedef double T_L_Humidity; /*It is a messure how a coefficient is 
													secure for being chnaged*/

typedef unsigned char T_L_Bit;
#define SELECTED_COMPONENT 0 

typedef unsigned long T_L_CoefficientIndex; /* As we refere and choose
	the coefficient using PRNG and we treat them as a series of values we need
	a tool to refere to them as one dimensional array, this type is serving as a 
	such index and maybe computer as RowNo * ImageWidth + ColNo*/

/** this class serve a container for the the Stego key
		Because the stego key is combination of three parts:
			
		1. Humidity Threshold.
		2. Decompression Quality
		3. PRNG Seed.
		
		this class manages the process for composing these parts 
		in a single string to make it a portable string.
*/	
struct S_L_StegoKey
{
	char* pPRNG_Seed; //The seed for tuning the pseudo random number generator
	unsigned int SeedSize;  //Number of Byte in the Seed
};
	
class C_L_WetStegoSecret
{
protected:
		S_L_StegoKey m_SecretKey;	/*Secret Key needed by recipant to decode 
			the secret message*/

		unsigned int m_HumidityThreshold; /* A threshold from 0..1000 which specify 
		which coefficients are subjected to be dry or suitable for embedding the
		secret message. by a linear transfomation, it specify an 0<= epsilon <= 0.5 
		if the abs distance of the frraction part of a coefficient	to 0.5 before quantization 
		is smaller than epsilon the coefficient is acceptable to be changed either to top or
		the bottom line of integers during quantization*/
		
		unsigned int m_RecompressQuality; /*The Quality used in information reducing process 0 .. 100 */

		unsigned int m_NoOfMatrix; /* In the original algorithm it does not
		treated as secret but we used it to derive more secure scheme, this
		variable is 	known as \beta in the original paper. Its value is set to 
		nr_2/k_avg = (n k/n) / k_avg = k / k_avg 

		the ratio  r can be obtain in the embed message function of the
		encoder.*/

public:
	
	/** It is used when the user know consioucely all compenets of the key and want to set one
	*/
	void SetStegoSchemeComponents(S_L_StegoKey* pSecretKey, unsigned int HumidityThreshold, unsigned int RecompressQuality, unsigned int NoOfMatrix)
	{
	
		//I don't know where I'm using it
		m_NoOfMatrix = NoOfMatrix;
	
		//The safetly rule prevent us from using a local information permanently
		m_SecretKey.SeedSize = pSecretKey->SeedSize;
		
		m_SecretKey.pPRNG_Seed = new char[m_SecretKey.SeedSize];
		memcpy(m_SecretKey.pPRNG_Seed,pSecretKey->pPRNG_Seed, m_SecretKey.SeedSize);

		m_HumidityThreshold = HumidityThreshold;
		m_RecompressQuality = RecompressQuality;
		
	}

	/** Other classes functions need the scheme components */
	S_L_StegoKey GetStegoKey()
	{
		//SAFETY FIRST 
		S_L_StegoKey Result;

		Result.SeedSize = m_SecretKey.SeedSize; 
		Result. pPRNG_Seed = new char[Result.SeedSize / 8];
		memcpy(Result.pPRNG_Seed, m_SecretKey.pPRNG_Seed, (Result.SeedSize /8)* sizeof(char));

		return Result;

	}

	unsigned int GetHumidityThreshold(){return m_HumidityThreshold;}
	unsigned int GetRecompressQuality(){return m_RecompressQuality;}
	unsigned long GetNoOfMatrices(){return m_NoOfMatrix;}

};

//GLOBAL FUNCTIONS
extern unsigned long CeillingDivide(unsigned long a, unsigned long b);
extern unsigned long LeastGreaterMultiple(unsigned long RoughValue, unsigned int BlockSize);

#endif

