/***************************************************************************
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 ***************************************************************************/
/*******LOGs
 * Wednesday, September 21,2005<
 * For now I compute the humditiy on the fly in the improved version 
 * a pre-computed table will be used for the sake of speed.
 *
 * One should notify that the algorithm does not differ between
 * Zero and none zero DCT coefficient so n as it suggested in the original 
 * paper is the total number of DCT Coefficients.
*/
#include "c_l_wetencoder.h"
#include "c_l_sharandomgenerator.h"
#include "c_l_dynamicmatrix.h"
#include "common.h"

#include <math.h>
#include <iostream> //For Debug

C_L_WetEncoder::C_L_WetEncoder()
{
}

C_L_WetEncoder::~C_L_WetEncoder()
{
}

/** The stream controler of whole embing process 
	return false if the embeding fails for any reason*/
bool C_L_WetEncoder::EmbedSecretMessage()
{
	
	//Step -2, Obtaining the raw coefficient of the first object
	//The very first step is to import JPEG Image Info
	m_pOriginalCoefficients = JPEG_Interface.ReadJPEGImage(m_pCoverMediaFilename);
	
	/*The new set of coefficient which contains the code has the same size 
	as the original one and now can be initialized*/
	
	m_WetQuantizedCoefficients.m_NoOfMembers = m_pOriginalCoefficients->m_NoOfMembers;
	m_WetQuantizedCoefficients.m_pDCT_Coefficient = new T_L_WetDCT_Coefficient[m_WetQuantizedCoefficients.m_NoOfMembers ];

	/*The encoding algorithm starts actually after two main steps
		1. Lossy Recompression.
		2. Applying SR and finding the available coefficients.
		
		It is actually what algorithm suggest but I prefer to 
		postepone the SR operation after the dynamic matrix 
		generated, in this sense one can select more wisely
	*/
	
 	/*Step -1, The Lossy Recompression.
		The Recompression is Lossy to Quantazaion Table
		What have to do is to requantize my table coefficient
		with larger quantizer coefficients. But it is essential
		to obtain an standard quantization table because an
		odd quantations table smells so good for blind
		universal steganalyzers
	*/
	m_pFullInfoQuantizationTables = JPEG_Interface.AquireQuantizationTables(m_pCoverMediaFilename, 0);
	//The reduction quality is treated as a secret.
	m_pReducedInfoQuantizationTables = JPEG_Interface.AquireQuantizationTables(m_pCoverMediaFilename, m_StegoScheme.GetRecompressQuality());

	//This is a place for the improvement of pre-computed humidity.
	
	//Totally inefficient way of deriving the parities and humidities
	unsigned long TotalNoOfDryElements = DerivateFundamentalSequences();
	double DrynessRatio = ((double)TotalNoOfDryElements) / ((double) m_WetQuantizedCoefficients.m_NoOfMembers);
	
	/* In this step I shoud prepare the Random Pool */
	/*RANDOM POOL HERE*/
	C_L_SHARandomGenerator SolutionGenerator, MatrixGenerator;

	//Initilizing the random generators
 	S_L_StegoKey MatrixSeed = m_StegoScheme.GetStegoKey();
	MatrixGenerator.SetSeed(MatrixSeed.pPRNG_Seed, MatrixSeed.SeedSize);
	
	/*I have set it just to set it to something the optimal value 
		should be derive in further researches

		It is the least value that no random bit is wasted out	
	*/
	MatrixGenerator.SetRandomSize(512);
	SolutionGenerator.SetRandomSize(512);

	/*The solution generator actually should use the entropy seed of system 
		but for this version we use the same seed for it*/
	SolutionGenerator.SetSeed(MatrixSeed.pPRNG_Seed, MatrixSeed.SeedSize);
		
	/* Because of the nature of dynamic encoding we should gaurd our 
		random generated numbers in a pool. */
	C_L_RandomPool MatrixRandomPool(&MatrixGenerator);
	/*In this step we start the Dynamix Matix cycle, the parameters
		of this step are treated as secret and should be set in secret scheeme.

		No of Columns of each matrix should be computed as suggested in the
		paper. It is n/beta 
	*/
	unsigned long NoOfColumns = m_pOriginalCoefficients->m_NoOfMembers / m_StegoScheme.GetNoOfMatrices();

	C_L_DynamicMatrix CodeMatrix(NoOfColumns, &MatrixRandomPool, &SolutionGenerator);

	/*I dervie a rough estimation as follow:
		In a typical JPEG File 3/4 of coefficients are
		zero and usable so if the humidity threshold is
		100 the ratio is about 1/4 if it is 0 it is 0
		I scale it linearly
	*/
	double EstimatedDrynessRatio = (double)m_StegoScheme.GetHumidityThreshold() / (double)400;

	//The scheme should be set for the CodeMatrix Here
	CaBinarySequence* pDeltaParities =  CodeMatrix.MessageInjection(m_SecretMessage.GetSequence(), m_SecretMessage.m_SequenceLength, m_ParitySequence.GetSequence(), m_ParitySequence.m_SequenceLength, m_HumiditySequence.GetSequence(), DrynessRatio, EstimatedDrynessRatio);

	if (CodeMatrix.GetStatus() != TLP_SUCCESS) return false;

	for(unsigned long i = 0; i < m_WetQuantizedCoefficients.m_NoOfMembers; i++)
	{
		//Update coefficients
		if ((*pDeltaParities)[i])
		{	
			//std::cout << m_WetQuantizedCoefficients.m_pDCT_Coefficient[i] << ", ";
			m_WetQuantizedCoefficients.m_pDCT_Coefficient[i] ^= 1; //(m_WetQuantizedCoefficients.m_pDCT_Coefficient[i] / 2 )  * 2 + //(m_WetQuantizedCoefficients.m_pDCT_Coefficient[i] % 2) ? 0 : 1;
			//std::cout << m_WetQuantizedCoefficients.m_pDCT_Coefficient[i] << std::endl;
		}

	}

JPEG_Interface.WriteJPEG_Image(m_pCoverMediaFilename, m_pStegoObjectFilename, m_StegoScheme.GetRecompressQuality(), &m_WetQuantizedCoefficients, m_pReducedInfoQuantizationTables);

	return true;

}

/*! For encoding process we need all three secret options*/
void C_L_WetEncoder::SetStegoKey(unsigned int HumidityThreshold, unsigned int RecompressionQuality, S_L_StegoKey* pNewStegoKey, unsigned long NoOfMatrices)
{
	m_StegoScheme.SetStegoSchemeComponents( pNewStegoKey, HumidityThreshold, RecompressionQuality, NoOfMatrices);
}


/*!
    \fn C_L_WetEncoder::DerivateFundamentalSequences()
	This function take the first bit of each coefficients and
	put them in a sequence.
	Also it analysis the Humidity of each coefficients and builds up a
	sequence corresponding to each parity there is a bit which shows
	if the parity is dry enough to conterbutes in embeding process or
	not.

	the function return back the no of dry elements this number is
	critical in approximating the dynamic matrix dimensions.
 */
unsigned long C_L_WetEncoder::DerivateFundamentalSequences()
{
	m_ParitySequence.InitializeCommittedEmpty(m_pOriginalCoefficients->m_NoOfMembers);
 	m_HumiditySequence.InitializeCommittedEmpty(m_pOriginalCoefficients->m_NoOfMembers);

	DCTELEM temp, qval;
	unsigned long TotalNoOfDryElements = 0;

	/*The driest element have humidity 0, the wetest element has humidity 0.5
		therefore in this step we should change the 0 - 100 system to the  0 - 0.5
		system.
	*/
	double AcceptableTreshold = (((double)m_StegoScheme.GetHumidityThreshold()) * 0.5)/ 100.00;

	unsigned long testcounter = 0;
	for(unsigned long i = 0; i < m_pOriginalCoefficients->m_NoOfMembers; i++)
	{
		
		/*I'm using the same code to devide by a/b as the 
			 JPEG Library to ensure that no characterstic
			 remains for the Blind Steganalysis.*/
		temp = m_pOriginalCoefficients->m_pDCT_Coefficient[i] * m_pFullInfoQuantizationTables[SELECTED_COMPONENT].quantval[i % DCTSIZE2];

		/* For including a coefficient in the embeding process it should qualify
			two test, first its fraction part should be near 0.5 and secondly it 
			should not be a zero variable. the second criteria is bit strange but
			it mentioned explicitly in the corresponding paper.
		*/
		/* the fact that 3/4 of the coefficient is zero made me to write this 
			if supperior to all*/
		if (temp)
		{
			qval = m_pReducedInfoQuantizationTables[SELECTED_COMPONENT].quantval[i % DCTSIZE2];
			#ifdef FAST_DIVIDE
				#define DIVIDE_BY(a,b)	a /= b
			#else
				#define DIVIDE_BY(a,b)	if (a >= b) a /= b; else a = 0
			#endif
			if (temp < 0) {
	  			temp = -temp;
	  			temp += qval>>1;	/* for rounding */
	  			DIVIDE_BY(temp, qval);
	  			temp = -temp;
			} else {
	  			temp += qval>>1;	/* for rounding */
	  			DIVIDE_BY(temp, qval);
			}

		//Setting Parity Sequence
			if (temp % 2)
				m_ParitySequence.SetBit(i);

			//Setting Humidity Sequence
			//Temp may become zero in quantization process
			if (temp)
			{
				double LossLessReductedElement = (				((double)(m_pOriginalCoefficients->m_pDCT_Coefficient[i] * m_pFullInfoQuantizationTables[SELECTED_COMPONENT].quantval[i % DCTSIZE2]))
		 		/ ((double)  m_pReducedInfoQuantizationTables[SELECTED_COMPONENT].quantval[i % DCTSIZE2]));

				if (((double)0.5 - fabs(LossLessReductedElement - (double)temp)) <  AcceptableTreshold)
				{
						//if (i < 100) std::cout <<" "<< i << ", ";
						m_HumiditySequence.SetBit(i);
						TotalNoOfDryElements++;

				}

			}

		}

		m_WetQuantizedCoefficients.m_pDCT_Coefficient[i] = temp;

	}

	/*std::cout << std::endl << std::endl << TotalNoOfDryElements << std::endl;
	std::cout << std::endl << m_pOriginalCoefficients->m_NoOfMembers  << std::endl;*/

	return TotalNoOfDryElements;

}
