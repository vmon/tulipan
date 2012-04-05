/***************************************************************************
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 ***************************************************************************/
/*******LOGs
 * 
 * Thursday, September 15, 2005:
 * I started officially the implementation of the first function
 */
#include "c_l_wetsteganographer.h"

#define MYDEBUG

#ifdef MYDEBUG
#include <iostream>
using namespace std;
#endif

C_L_WetSteganographer::C_L_WetSteganographer()
{
}


C_L_WetSteganographer::~C_L_WetSteganographer()
{
}


/** Key Injection Routine */
void C_L_WetSteganographer::InitializeStegoScheme(unsigned int HumidityThreshold, unsigned int RecompressionQuality, S_L_StegoKey* pNewStegoKey, unsigned long NoOfMatrices)
{
	/* Here should both Encoder and Decoder Keys initialized however for 
		the first stage I just concentrate on Encoder.
	*/
	m_WetEncoder.SetStegoKey(HumidityThreshold, RecompressionQuality, pNewStegoKey, NoOfMatrices);
	m_WetDecoder.SetStegoKey(pNewStegoKey, HumidityThreshold, NoOfMatrices);

}

/**  Embeds The message file in Cover File */
bool C_L_WetSteganographer::EmbedMessageInCoverMedia(char* MessageFilename, char* CoverImageFilename, char* StegoFilename)
{
	//In this stage we just tune the Encoder and then the encoder takes the whole responsiblity
	m_WetEncoder.SetCoverMediaFilename(CoverImageFilename);
	m_WetEncoder.SetSecretMessageFilename(MessageFilename);
	m_WetEncoder.SetStegoObjectFilename(StegoFilename);

	return m_WetEncoder.EmbedSecretMessage();
	
}


bool C_L_WetSteganographer::RetrieveMessage(char* StegoFilename, char* RetrievedMessageFile)
{
	//In this stage we just tune the Encoder and then the encoder takes the whole responsiblity
	m_WetDecoder.SetStegoObjectFilename(StegoFilename);
	m_WetDecoder.SetSecretMessageFilename(RetrievedMessageFile);

	return m_WetDecoder.RetrieveMessage();
	
}
