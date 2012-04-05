/***************************************************************************
 ***************************************************************************/
#include <assert.h>
#include "c_l_sharandomgenerator.h"

/********LOGs
 * it is the code that George has suggested to use the SHA program
 *
			//The array that will receive the final result (32bytes for SHA256)
			char acDigest[33];
			//Declare an object
			CSHA oSHA(CSHA::SHA256);
			//Add data repeatedly
			SHA.AddData("string1", 7);
			SHA.AddData("string2", 7);
			SHA.AddData("string3", 7);
			//Conclude the operation, after that the result is in acDigest
			SHA.FinalDigest(acDigest);
			
 * As a random generator we feed the seed by means of AddData function, and 
 * we use FinalDigest to generatre random string as many as we need.
 */

C_L_SHARandomGenerator::C_L_SHARandomGenerator()
/* Constructor: Just tune the low level generator on 512 bits which seems more 
	secure
*/
:m_SHAAgent(C_L_SHA::SHA512)
{
//We believe that 512 process is more secure than 2 consective 256 processes
//so we always use SHA-512

//Hirarichally creation sets m_RandomSize = 0

}


C_L_SHARandomGenerator::~C_L_SHARandomGenerator()
{
}

void C_L_SHARandomGenerator::SetRandomSize(unsigned int NewRandomSize)
/*Set the random string size as needed by the user, in this program
	it should be set according to the number of bit that uses in the Finite Field
	
	New RandomSize	The size of random block in number of BITs
*/
{
	//Setting is metter when the algorithm wants to let out the final 
	//random number, also the seed is expected to have that size
	//however it doesn't have any effect on memory allocation
	
	m_RandomSize = NewRandomSize;
	
}
void C_L_SHARandomGenerator::SetSeed(const char* Seed, unsigned int SeedSize)
/*
	Set the random generator state on the seed
	
	Seed				Block Memory that constains
	SeedSize 		The size of the Seed in number of BITs
*/
{
	assert(Seed);
	m_SHAAgent.AddData(Seed, SeedSize / (8 * sizeof(char)));
	
}

char* C_L_SHARandomGenerator::GenerateRandom()
/*
	Generates a random block of memory with m_RandomSize bits and return it 
	as char* pointer
*/
{
	//each time we finalize the data we get a new random object,
	//the fundamental question is that if these random strings are independent
	//or they are volunrable, we should this question our next meeting
	
	//first of All we should generate random as much as we need then we 
	//should return them. As Dr. Eghlidos believes that SHA-512 is the 
	//most secure and reliable algorithm, we unexceptionaly use it. it has 
	//set in the constructor
	
	//512 / 8 = 64

	unsigned int NoOfDigest = (m_RandomSize / 512) + ((m_RandomSize %  512) ? 1 : 0);
	
	char* Result = new char[NoOfDigest * 64]; /**BYTEPORT!*/
	assert(Result);
	
	for(int i = 0; i < NoOfDigest; i++)
	{
		m_SHAAgent.FinalDigest(&Result[i * 64]);
		m_SHAAgent.AddData(&Result[i * 64],  64);
		/*This step is need to prevent the random generator to be restarted 
			next time
		*/
		
	}
	//You see that I generate always more than or equal bit 
	//to the amount user requires
	
	return Result;
	
}


	/*!
		\fn virtual GenerateRandomInBuffer(char* pRandomBuffer)

		This function generate random in the user given buffer. In this 
		sense it does not need to allocate the buffer and for this reason
		it is faster than its sibling.

		char* RandomBuffer: 	The memory buffer that the random
		is generated inside it. The user should provide(allocate) the 
		buffer equal to random size.

  */
void C_L_SHARandomGenerator::GenerateRandomInBuffer(char* pRandomBuffer)
{
	assert(pRandomBuffer);

	unsigned int NoOfDigest = (m_RandomSize / 512) + ((m_RandomSize %  512) ? 1 : 0);

	for(int i = 0; i < NoOfDigest; i++)
	{
		m_SHAAgent.FinalDigest(&pRandomBuffer[i * 64]);
		m_SHAAgent.AddData(&pRandomBuffer[i * 64],  64);
		/*This step is need to prevent the random generator to be restarted 
			next time
		*/
	}

}
