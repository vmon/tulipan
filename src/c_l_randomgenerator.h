/***************************************************************************
 ***************************************************************************/
#ifndef C_L_RANDOMGENERATOR_H
#define C_L_RANDOMGENERATOR_H

/**
Any random number generator that user wishes to use in share producing process should be inherit this class to be compatible with SharableSecret Class

*/

/********LOGs
 * 
 * For now I just want that the random number generators produce an array of
 * char. Obviously if each member of array is random, the whole result will
 * random as well.
 *
 * Thursday, Septemeber 22, 2005:
 * In some usage it is inevitable to know the
 * Random size to avoid the waste of randoms
 * so an interface function added to return it back;
 *
 * Saturday, October 22, 2005,
 * I add another random generation function which supposed to be
 * faster cause it does not allocate memory.
 */

/*This is an abstrcat class and I've built it for the sake of run time
    Diversity
	Any random generator which the programmer wants to implement to 
	use with Mitra cooperativly, should be inherited from this class
	and overload its 3 functions.
*/
class C_L_RandomGenerator{
protected:
	unsigned int m_RandomSize;
public:
    C_L_RandomGenerator()
	:m_RandomSize(0)
	{
	};

    ~C_L_RandomGenerator(){};

	/*First of all user should specify the size of random block that
		is needed using this function*/
	virtual void SetRandomSize(unsigned int NewRandomSize) = 0;
	
	/*Any random number generator should be tuned by a seed
		The seed can have any arbitary size and independent of 
		random size
		
		By means of this function the user sends the seed to the generator
	*/
	virtual void SetSeed(const char* Seed, unsigned int SeedSize) = 0;
	
	/*
		Fills a  block of memory (generates random) with random value and
		returns the memory address in char*
	*/
	virtual char* GenerateRandom() = 0;

	/*!
		\fn virtual void GenerateRandomInBuffer(char* pRandomBuffer)

		This function generate random in the user given buffer. In this 
		sense it does not need to allocate the buffer and for this reason
		it is faster than its sibling.

		char* RandomBuffer: 	The memory buffer that the random
		is generated inside it. The user should provide(allocate) the 
		buffer equal to random size.

	 */
	virtual void GenerateRandomInBuffer(char* pRandomBuffer) = 0;

	virtual unsigned int GetRandomSize()
	{
		return m_RandomSize;

	}

};

#endif
