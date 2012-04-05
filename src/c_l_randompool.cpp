/***************************************************************************
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 ***************************************************************************/
/******LOGs
 * 
 * Thursday, September 2005,
 * We should be sure that the RandomSize is a integer multiple of 
 * Number of bits in a T_L_BinaryStreamBlock, but it would be 
 * by high probability because a valid random size is 512 bits.
 *
 */
#include "c_l_randompool.h"
#include "common.h"

C_L_RandomPool::C_L_RandomPool(C_L_RandomGenerator *pRandomGenerator)
:m_pRandomGenerator(pRandomGenerator), //The Generator is assumed to be tunned
m_RequestInProcess(false),
m_PoolSize(0),
m_PoolHead(0),
m_PoolContentSize(0),
m_pRandomBlock(NULL),
m_pTempoPool(0)
{

	assert(pRandomGenerator);
	assert(pRandomGenerator->GetRandomSize());
	/*For the sake of speed I just allocate this buffer once and use it many time
		in this sense it is CRITICAL that the user does not change the random generator 
		during the operation of random pool
	*/
	m_RandomBlockSize = pRandomGenerator->GetRandomSize() / 8;
	m_pRandomBlock = new char[m_RandomBlockSize];
	
	assert(m_pRandomBlock);

}


C_L_RandomPool::~C_L_RandomPool()
{
	delete m_pRandomBlock;
}

/** when user require a bunch of random numbers but not commit them should 
	call this function. The function assess the amount of available random numbers
	if it's not enouch it generate more and store them in to the memory.  The size of
	requested stream is in bits.

	The function fails if there is still a request in process by returning NULL.
*/
T_L_BinaryStreamBlock* C_L_RandomPool::RequestRandomStream(unsigned long RequestSize)
{

	assert( m_pRandomGenerator);

	//Just one request per commite or deny is allowed	
	if (m_RequestInProcess) return NULL;
	m_RequestInProcess = true;
	//So We are sure that the m_PoolHead is at zero

	if (RequestSize > m_PoolContentSize) /*if the request size is greater
		than generated pool we need more resource*/
	{
		//makes a bigger temporal memory
		//The generator should be completely initialized
			unsigned long RealizedRequestSize = LeastGreaterMultiple(RequestSize - m_PoolContentSize, m_RandomBlockSize);

		/*Anyway the other content of pool are rubish and can
			be overwritten. If even adding them we need more
			space we should allocate it again.*/
		T_L_BinaryStreamBlock* pGrownPool;
		if (m_PoolSize - m_PoolContentSize < RealizedRequestSize)
		{
			unsigned long NewPoolSize = RealizedRequestSize + m_PoolContentSize;
			pGrownPool = new T_L_BinaryStreamBlock[NewPoolSize  / BINARY_STREAM_BLOCK_SIZE]; 

			//This buffer has to be a integer multiple of RandomSize
			memcpy(pGrownPool, m_pTempoPool, CeillingDivide(m_PoolContentSize, BINARY_STREAM_BLOCK_SIZE) * BYTES_IN_BINARY_STREAM_BLOCK) ;
		
			delete[] m_pTempoPool;
	
			m_pTempoPool = pGrownPool;
			m_PoolSize = NewPoolSize;

	}
	
		//I'm sure that the m_PoolSize is an integer multiple of the random size
		T_L_BinaryStreamBlock* PoolIndicator = m_pTempoPool;
		PoolIndicator += m_PoolContentSize /BINARY_STREAM_BLOCK_SIZE;

		unsigned int NoOfGaintStep = (RealizedRequestSize) /m_RandomBlockSize;
		for(int i = 0; i < NoOfGaintStep; i ++, PoolIndicator +=  m_RandomBlockSize / BINARY_STREAM_BLOCK_SIZE)
		{
			//We should delete the random block to avoid leak of memory
			m_pRandomGenerator->GenerateRandomInBuffer(m_pRandomBlock);
			memcpy(PoolIndicator , m_pRandomBlock, (m_RandomBlockSize / BINARY_STREAM_BLOCK_SIZE) * BYTES_IN_BINARY_STREAM_BLOCK);

		}

		/* In new sterategy there is no baby step
			//Now the one baby step
		
			m_pRandomGenerator->GenerateRandomInBuffer(m_pRandomBlock);
			memcpy(PoolIndicator , m_pRandomBlock, (((RequestSize - m_PoolContentSize) % RandomSize) / BINARY_STREAM_BLOCK_SIZE) * BYTES_IN_BINARY_STREAM_BLOCK);

		*/
	
		m_PoolHead = RequestSize;
		m_PoolContentSize = RealizedRequestSize + m_PoolContentSize;
	}
	else
	//We have already enough random contents
	{
		m_PoolHead = RequestSize;

	}

	//Integrality assumption in the first release
	T_L_BinaryStreamBlock* pResult = new T_L_BinaryStreamBlock[CeillingDivide(RequestSize, BINARY_STREAM_BLOCK_SIZE)];
	
	memcpy(pResult, m_pTempoPool , CeillingDivide(RequestSize, BINARY_STREAM_BLOCK_SIZE) * BYTES_IN_BINARY_STREAM_BLOCK);

	return pResult;

}

/** If a part of random sequence is not used, this function free it back to
	the C_L_RandomPool for future usage, NOT A BIT OF RANDOM POOL 
	SHOULD BE WASTE or the secret isn't decodable.  
	
	if there is no current request function fails with returning false.
*/
bool C_L_RandomPool::DenyCurrentRequest(unsigned long Size )
{
	if (!m_RequestInProcess) return false;

	/*ATTENTION: Because the structure of first version size should be integer
		multiple of BINARY_STREAM_BLOCK_SIZE
	*/
	m_PoolHead -= Size;
	return true;
	
}

/** After that the actual size of the used random stream was understood
	,this function is used to commit it and send it out of the pool

	if there is no current request function fails with returning false.
*/
bool C_L_RandomPool::CommiteCurrentRequest()
{
	//One need a Request to commit
	if (!m_RequestInProcess) return false;

	/*Commiting a part of pool means that we don't need to store that part
		in the pool
	*/
	memcpy(m_pTempoPool, m_pTempoPool + m_PoolHead / BINARY_STREAM_BLOCK_SIZE, ((m_PoolContentSize - m_PoolHead) /BINARY_STREAM_BLOCK_SIZE) * BYTES_IN_BINARY_STREAM_BLOCK);

	m_PoolContentSize -= m_PoolHead;
	m_PoolHead = 0;

	m_RequestInProcess = false;

	return true;
	
}


