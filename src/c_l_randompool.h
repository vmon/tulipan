/***************************************************************************
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 ***************************************************************************/
#ifndef C_L_RANDOMPOOL_H
#define C_L_RANDOMPOOL_H

#include "c_l_randomgenerator.h"
#include "common.h"

/**
This class represents a pool of random Numbers one can request as much as she estimate she needs and after it she can denies her request or commits it.

Pay attention that a request can be denied many times but can be commited just once.
*/
class C_L_RandomPool{
protected:
	C_L_RandomGenerator* m_pRandomGenerator; /* it should be a tunned random
	Generator and C_L_RandomPool is not responsible for it. */

	T_L_BinaryStreamBlock* m_pTempoPool; /* It store the Generated Random Numbers
	until they are used commitedly*/

	unsigned long  m_PoolHead; /*A pointer tofirst  undemanded BIT of pool
	when user send a request the pPoolHead is progressing to the bit after the 
	last request bit, if the user denies the request, it comes back agian, so
	just one request is possible and others are invalide until the user either
	denies or commite her current one.*/
	
	unsigned long m_PoolSize; //the ACTUAL Size of the pool in memory
	unsigned long m_PoolContentSize; /*It shows how many bits after 
		after the pool head is unused and can be used for other purposes
	in this sense m_PoolSize =< m_Pool Head + m_PoolContentSize
	*/

	bool m_RequestInProcess;

	/*To avoid repetetive time consuming allocation of random block
		I use this permanent buffer
	 */
	char* m_pRandomBlock;
	unsigned long m_RandomBlockSize;

public:
    C_L_RandomPool(C_L_RandomGenerator *pRandomnGenrator);

    ~C_L_RandomPool();

	/** when user require a bunch of random numbers but not commit them should 
		call this function. The function assess the amount of available random numbers
		if it's not enouch it generate more and store them in to the memory.  The size of
		requested stream is in bits.

		The function fails if there is still a request in process by returning NULL.
	*/
	T_L_BinaryStreamBlock* RequestRandomStream(unsigned long Size);
	//In the first release the size should be multiple of BINARY_STREAM_BLOCK_SIZE

	/** If a part of random sequence is not used, this function free it back to
	the C_L_RandomPool for future usage, NOT A BIT OF RANDOM POOL 
	SHOULD BE WASTE or the secret isn't decodable.  
	
	if there is no current request function fails with returning false.
	*/
	bool DenyCurrentRequest(unsigned long Size);

	/** After that the actual size of the used random stream was understood
			,this function is used to commit it and send it out of the pool

	if there is no current request function fails with returning false.
	*/
	bool CommiteCurrentRequest();

};

#endif
