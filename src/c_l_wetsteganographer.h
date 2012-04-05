/***************************************************************************
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 ***************************************************************************/
/*******LOGs
 *
 *
 * Wednesday, 14 September 2005:
 * As the first step I designed the memory structure needed 
 * by the algortihm.
 * Actually, The four important structures are:
 *		1. The pseudo random order of usage of dct coefficients.
 *		2. The progressive pseudo random matrix.
 *		3. The solvable matrix.
 * 	    4. The container of the dct coefficient and their properties
 *	    5. The secret message.
 *
 * 	the pseudo random order of dct usage need not to be stored
 * and can be computed on the fly.
 * 
 * For the secret message I used my tradiational binary sequence class.
 */

#ifndef C_L_WETSTEGANOGRAPHER_H
#define C_L_WETSTEGANOGRAPHER_H

#include "common.h"
#include "c_l_wetencoder.h"
#include "c_l_wetdecoder.h"

/**
The whole encoding/decoding manager of the programm. It runs every step of encoding/decoding algorithms.

*/

class C_L_WetSteganographer{
protected:
	/* Encoder/Decoder */
	C_L_WetEncoder	m_WetEncoder;
	C_L_WetDecoder m_WetDecoder;
	
public:
    C_L_WetSteganographer();
    ~C_L_WetSteganographer();

	/** Key Injection Routine */
	void InitializeStegoScheme(unsigned int HumidityThreshold, unsigned int RecompressionQuality, S_L_StegoKey* pNewStegoKey, unsigned long NoOfMatrices);
	
	/**  Embeds The message file in Cover File */
	bool EmbedMessageInCoverMedia(char* MessageFilename, char* CoverImageFilename, char* StegoFilename);
	
	/** Retrievement Procedure */
	bool RetrieveMessage(char* StegoFilename, char* RetrievedMessageFile);

};

#endif
