/***************************************************************************

 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 ***************************************************************************/
/*********LOGs
 * 
 * Saturday, 22 October 2005:
 * I started wrting the decoder in many sense it is the same as 
 * the encoder just there is no deny in the random generator
 * and ther is no need to itterative gaussian elimination.
 * further we don't need to solve the matrix and it just replaced
 * by simple multiplication.
 */
#ifndef C_L_WETDECODER_H
#define C_L_WETDECODER_H

#include "common.h"
#include "jpeglib.h"
#include "c_l_dct_coefficient.h"
#include "c_l_jpeg_interface.h"
#include "aBinarySequence.h"

/**
Encapsulation of decoding process and retrieving the information

*/
class C_L_WetDecoder{
protected:
	C_L_WetStegoSecret m_StegoScheme;

	/*The preference of storing filename instead of file handle is 
		due to some local temporal usage of handles*/
	char* m_pSecretMessageFilename;
	char* m_pStegoObjectFilename; 

	C_L_DCT_CoefficientCollection* m_pDefectiveCoefficients;

	/* The object provide us access to the JPEG Images */
	C_L_JPEG_Interface JPEG_Interface;

	/* Fundamental Sequences*/
	CaBinarySequence m_ParitySequence;

	/* The secret Message Sequence */
	CaBinarySequence m_SecretMessage;

public:
    C_L_WetDecoder();
    ~C_L_WetDecoder();

/** The stream controler of whole retrieving  process 
	return false if the retrieving fails for any reason*/
	bool RetrieveMessage();

    void DerivateFundamentalSequences();

//Interface Functions
	/** For dencoding process we need all two secret options*/
	void SetStegoKey(S_L_StegoKey* pNewStegoKey, unsigned int HumadityThreshold, unsigned long NoOfMatrices);

	//All these should replaced with a more efficient codes
	void SetSecretMessageFilename(char* NewSecretMessageFilename)
			{
				m_pSecretMessageFilename = new char[strlen(NewSecretMessageFilename) + 1];
				strcpy(m_pSecretMessageFilename, NewSecretMessageFilename);

				//m_SecretMessage.DigestSequence(m_pSecretMessageFilename);

			}

	void SetStegoObjectFilename(char* NewStegoObjectFilename)
			{
				m_pStegoObjectFilename = new char[strlen(NewStegoObjectFilename) + 1];
				strcpy(m_pStegoObjectFilename, NewStegoObjectFilename);
			}

};

#endif
