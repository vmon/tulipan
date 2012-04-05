/***************************************************************************


  ***************************************************************************/
#ifndef C_L_SHARANDOMGENERATOR_H
#define C_L_SHARANDOMGENERATOR_H

#include "c_l_randomgenerator.h"
#include "SHA.h"
/**
This class encapsulates the SHA-2 hash algorithm in as a random generator core, it performes all SHA actions in the context and just gives the random number to the user 

*/
class C_L_SHARandomGenerator  : public C_L_RandomGenerator 
{
protected:
	/*The handy SHA agent, serves this 
		class as a low level implementation of the core of the random generation
		procedure
	*/
	C_L_SHA m_SHAAgent;
	
public:
    C_L_SHARandomGenerator();

    ~C_L_SHARandomGenerator();

public:
 	
	//Virtual redifinitions
	
	/*Set the random string size as needed by the user, in this program
		it should be set according to the number of bit that uses in the Finite Field
	*/
	virtual void SetRandomSize(unsigned int NewRandomSize);
	
	//	Set the random generator state on the seed
	virtual void SetSeed(const char* Seed, unsigned int SeedSize);
	
	/*	Generates a random block of memory with m_RandomSize bits and return it 
		as char* pointer
	*/
	virtual char* GenerateRandom();

	/*!
		\fn virtual void GenerateRandomInBuffer(char* pRandomBuffer)

		This function generate random in the user given buffer. In this 
		sense it does not need to allocate the buffer and for this reason
		it is faster than its sibling.

		char* RandomBuffer: 	The memory buffer that the random
		is generated inside it. The user should provide(allocate) the 
		buffer equal to random size.

	 */
	virtual void GenerateRandomInBuffer(char* pRandomBuffer);


};

#endif
