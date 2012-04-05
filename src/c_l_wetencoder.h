/***************************************************************************
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 ***************************************************************************/
#ifndef C_L_WETENCODER_H
#define C_L_WETENCODER_H

#include "common.h"
#include "jpeglib.h"
#include "c_l_dct_coefficient.h"
#include "c_l_jpeg_interface.h"
#include "aBinarySequence.h"

#if BITS_IN_JSAMPLE == 8
typedef int DCTELEM;		/* 16 or 32 bits is fine */
#else
typedef INT32 DCTELEM;		/* must have 32 bits */
#endif

/**
Encapsulation of Encoding and Embeding process

*/
class C_L_WetEncoder{
protected:
	C_L_WetStegoSecret m_StegoScheme;

	/*The preference of storing filename instead of file handle is 
		due to some local temporal usage of handles*/
	char* m_pCoverMediaFilename;
	char* m_pSecretMessageFilename;
	char* m_pStegoObjectFilename; 

	C_L_DCT_CoefficientCollection* m_pOriginalCoefficients;
	C_L_DCT_CoefficientCollection m_WetQuantizedCoefficients;

	/* The object provide us access to the JPEG Images */
	C_L_JPEG_Interface JPEG_Interface;

	/* The storage of quantization table before and after recompression*/
	JQUANT_TBL* m_pFullInfoQuantizationTables;
	JQUANT_TBL* m_pReducedInfoQuantizationTables;

	/* Fundamental Sequences*/
	CaBinarySequence m_ParitySequence;
	CaBinarySequence m_HumiditySequence;

	/* The secret Message Sequence */
	CaBinarySequence m_SecretMessage;

public:
    C_L_WetEncoder();
    ~C_L_WetEncoder();

/** The stream controler of whole embing process 
	return false if the embeding fails for any reason*/
	bool EmbedSecretMessage();

    unsigned long DerivateFundamentalSequences();

//Interface Functions
	/** For encoding process we need all three secret options*/
	void SetStegoKey(unsigned int HumidityThreshold, unsigned int RecompressionQuality, S_L_StegoKey* pNewStegoKey, unsigned long NoOfMatrices);

	//All these should replaced with a more efficient codes
	void SetCoverMediaFilename(char* NewCoverImageFilename) 
			{	
				m_pCoverMediaFilename = new char[strlen(NewCoverImageFilename) + 1];
				strcpy(m_pCoverMediaFilename, NewCoverImageFilename);
			}

	void SetSecretMessageFilename(char* NewSecretMessageFilename)
			{
				m_pSecretMessageFilename = new char[strlen(NewSecretMessageFilename) + 1];
				strcpy(m_pSecretMessageFilename, NewSecretMessageFilename);

				m_SecretMessage.DigestSequence(m_pSecretMessageFilename);

			}

	void SetStegoObjectFilename(char* NewStegoObjectFilename)
			{
				m_pStegoObjectFilename = new char[strlen(NewStegoObjectFilename) + 1];
				strcpy(m_pStegoObjectFilename, NewStegoObjectFilename);
			}


};

#endif
