/***************************************************************************
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 ***************************************************************************/
#include "c_l_wetdecoder.h"
#include "c_l_sharandomgenerator.h"
#include "c_l_dynamicmatrix.h"
#include "common.h"

#include <iostream> //For Debug

C_L_WetDecoder::C_L_WetDecoder()
{
}


C_L_WetDecoder::~C_L_WetDecoder()
{
}



/** The stream controler of whole retreiving process 
	return false if the retreiving fails for any reason*/
bool C_L_WetDecoder::RetrieveMessage()
{
	
	//Step -2, Obtaining the coefficient of the StegoObject
	//The very first step is to import JPEG Image Info
	m_pDefectiveCoefficients = JPEG_Interface.ReadJPEGImage(m_pStegoObjectFilename);
	
	//deriving the parities 
	DerivateFundamentalSequences();

	/* In this step I shoud prepare the Random Pool */
	/*RANDOM POOL HERE*/
	C_L_SHARandomGenerator  MatrixGenerator;

	//Initilizing the random generators
 	S_L_StegoKey MatrixSeed = m_StegoScheme.GetStegoKey();
	MatrixGenerator.SetSeed(MatrixSeed.pPRNG_Seed, MatrixSeed.SeedSize);
	
	/*I have set it just to set it to something the optimal value 
		should be derive in further researches

		It is the least value that no random bit is wasted out	
	*/
	MatrixGenerator.SetRandomSize(512);
	
	/* Because of the nature of dynamic encoding we should gaurd our 
		random generated numbers in a pool. */
	C_L_RandomPool MatrixRandomPool(&MatrixGenerator);

	/*In this step we start the Dynamix Matix cycle, the parameters
		of this step are treated as secret and should be set in secret scheeme.

		No of Columns of each matrix should be computed as suggested in the
		paper. It is n/beta 
	*/
	unsigned long NoOfColumns = m_pDefectiveCoefficients->m_NoOfMembers / m_StegoScheme.GetNoOfMatrices();

	C_L_DynamicMatrix CodeMatrix(NoOfColumns, &MatrixRandomPool, NULL);

	/*I dervie a rough estimation as follow:
		In a typical JPEG File 3/4 of coefficients are
		zero and unusable so if the humidity threshold is
		100 the ratio is about 1/4 if it is 0 it is 0
		I scale it linearly
	*/
	double EstimatedDrynessRatio = (double)m_StegoScheme.GetHumidityThreshold() / (double)400;

	//The scheme should be set for the CodeMatrix Here
	CaBinarySequence* pSecretMessage =  CodeMatrix.MessageRetrieval(&m_ParitySequence, EstimatedDrynessRatio);

	return pSecretMessage->SerializeSequence(m_pSecretMessageFilename);

}

void C_L_WetDecoder::DerivateFundamentalSequences()
{
	m_ParitySequence.InitializeCommittedEmpty(m_pDefectiveCoefficients->m_NoOfMembers);
 	
	for(unsigned long i = 0; i < m_pDefectiveCoefficients->m_NoOfMembers; i++)
	{
		if (m_pDefectiveCoefficients->m_pDCT_Coefficient[i]% 2)
				m_ParitySequence.SetBit(i);

	}

}

void C_L_WetDecoder::SetStegoKey(S_L_StegoKey* pNewStegoKey, unsigned int HumidityThreshold, unsigned long NoOfMatrices)
{
			m_StegoScheme.SetStegoSchemeComponents(pNewStegoKey, HumidityThreshold, 0, NoOfMatrices);

}
