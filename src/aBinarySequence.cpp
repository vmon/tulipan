
#include "aBinarySequence.h"
#include <memory.h>
#include <assert.h>
#include <fstream>
/*******LOGs
 *
 * Friday, September 16, 2005:
 * I just changed this class in a way it works for standard c++
 * not only for MFC and VC. This lack of portability was due 
 * to using CFile class of MFC which I replace by ifstream
 * to read sequences from file. However, yet I've not test it.
 *
 */
using namespace std;

//Const statics definitons
extern const unsigned char TwoPower[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x040, 0x80};

const unsigned int CaBinarySequence::BLOCK_SIZE = sizeof(char) * 8;
const unsigned int CaBinarySequence::BYTES_IN_BLOCK = sizeof(char);

CaBinarySequence::CaBinarySequence(void)
: m_SequenceLength(0)
, m_pBinarySequence(0)
{
}

CaBinarySequence::~CaBinarySequence(void)
{
	delete[] m_pBinarySequence;
	m_pBinarySequence = 0;

}


// The only way that someone could inject a such long random sequence into the program is using files, so it's reasonable to give this task to the holder class
bool CaBinarySequence::DigestSequence(char* FileName)
{
	ifstream SequenceStorage;
	
	SequenceStorage.open(FileName, ios::in | ios::binary);
	if (!SequenceStorage.good())
	{
		throw("Error in opening File");
		return false;
	}
	
	streampos current = SequenceStorage.tellg();
	streampos end = SequenceStorage.seekg(0, ios::end).tellg();
	SequenceStorage.seekg(current);
	
	m_SequenceBlockLength = ((end-current) +  (long) BYTES_IN_BLOCK - (long)1)/ BYTES_IN_BLOCK;
	m_SequenceLength = (long) ((end-current)  * 8); //In bits

	//Alocating Memory for bring whole sequence in to the heap for later analysis
	//If there is another sequence is in the memory free I have to free it first to avoid 
	//memory leak
	
	if (m_pBinarySequence) delete[] m_pBinarySequence;
	if (!(m_pBinarySequence = new BLOCK [m_SequenceBlockLength] ))
	{
		throw("Out of memory!");
		return false;
	}

	//If we find room, we can load the sequence from file
		//The Read count argument is of type UINT (32bit int) so if is regriously has been chosen
		//The is no worry to read whole the sequence in one step
	SequenceStorage.read((char *)m_pBinarySequence, m_SequenceLength / 8);

	return true;
		
}

// //The Other way of colning an exist sequence
CaBinarySequence& CaBinarySequence::operator=(const CaBinarySequence& SourceSequence)
{

	//Alocating Memory for bring whole sequence in to the heap for later analysis
	//If there is another sequence is in the memory free I have to free it first to avoid 
	//memory leak
	
	m_SequenceLength = 0;
	m_SequenceBlockLength = 0;
	if (m_pBinarySequence) delete[] m_pBinarySequence;
	if (!(m_pBinarySequence = new BLOCK [SourceSequence.m_SequenceBlockLength] ))
	{
		//Error
		return *this;

	}
	m_SequenceLength = SourceSequence.m_SequenceLength;
	m_SequenceBlockLength = SourceSequence.m_SequenceBlockLength;

	//If we find room, we can load the sequence from file
	//I don't know if a new technology has been invented to optimize 
	//memory imaging so I use the arcade ways for it

	memcpy(m_pBinarySequence, SourceSequence.m_pBinarySequence, (m_SequenceBlockLength) * BYTES_IN_BLOCK);
	
	return *this;
}

// Just for read access (The write access using reference operator needs explisite equality
bool CaBinarySequence::operator[](unsigned long BitIndex)
{
	return ((bool)(m_pBinarySequence[BitIndex / BLOCK_SIZE] & TwoPower[BitIndex % BLOCK_SIZE]));
}

// It is not a such important function cause there is no matter of setting just one bit but I impelemented for the necessary cases and for having a complete set
void CaBinarySequence::SetBit(unsigned long BitIndex)
{
	m_pBinarySequence[BitIndex / BLOCK_SIZE] |= TwoPower[BitIndex % BLOCK_SIZE];

}

// It is a fast solution to get sequence data (in block) in compare with the bit by bit slow way, however user is restricted to walk just by BLOCK_SIZE steps
BLOCK CaBinarySequence::GetStandardBlock(unsigned long BlockIndex)
{
	return m_pBinarySequence[BlockIndex];
}

// It lets us to retrieve blocks with any desirable size but it is not much faster than bit by bit solution
BLOCK* CaBinarySequence::GetBlock(unsigned long BitIndex, unsigned long BlockSize)
{
	BLOCK* Result = NULL;

	unsigned long BlockSizeInBlocks = BlockSize + BLOCK_SIZE - 1;
	BlockSizeInBlocks /= BLOCK_SIZE;

	//if Bit Index is a multiple of BLOCK_SIZE everything fells in brand-new super fast procedure
	if (BitIndex % BLOCK_SIZE == 0)
	{
		Result = new BLOCK[BlockSizeInBlocks];
		//It is necessary here to check if the computer give us requested memory or not,
		//but for the sake of the speed we could use our ignorance toward this problem, why not?
		memcpy(Result, m_pBinarySequence + (BitIndex / BLOCK_SIZE), (BlockSizeInBlocks) * BYTES_IN_BLOCK);
		
		//It is very important to delete extra bits on the last char, because may the user
		//treat the char as a number and the exact value is needed
		if (BlockSizeInBlocks * BLOCK_SIZE > BlockSize)
			Result[BlockSizeInBlocks - 1] &= TwoPower[BlockSize % BLOCK_SIZE] - 1; 

	}
	//Unfortunately if that golden rule isn't applicable, we should involve ourselves in 
	// a exhustic operation of "digesting the long worm"
	else
	{
		//If we have BlockSize % 32 = 31 for example 63 so we have to allocate 
		//a Block more
		Result = new BLOCK [BlockSizeInBlocks];
		//It is necessary here to check if the computer give us requested memory or not,
		//but for the sake of the speed we could use our ignorance toward this problem, why not?

		//Because I've used logical OR operator to set bits, I need to unset them at the first
		//The One Extra BLOCK that we have allocated, is needed to set to zero too even if 
		//it has extra bits, because these extra bits have influnce on the number interpetation
		memset(Result, 0, (BlockSizeInBlocks) * sizeof(BLOCK));

		BLOCK Buffer = m_pBinarySequence[BitIndex / BLOCK_SIZE];
		unsigned long DeltaPhase = BitIndex % BLOCK_SIZE;
		Buffer >>= DeltaPhase;
		
		for(unsigned int i = BitIndex; i < BitIndex + BlockSize; i++)
		{
			if ((i % BLOCK_SIZE) == 0)
				Buffer = m_pBinarySequence[i / BLOCK_SIZE];

			Result[(i - BitIndex) / BLOCK_SIZE] |= (Buffer % 2) * TwoPower[(i - DeltaPhase) % BLOCK_SIZE];
			Buffer >>= 1;

		}

		//Yet it isn't regoriously proved that this algorithm work I have to check it later
		//I have check it, It is proven now.

	}
	
	return Result;
}

void CaBinarySequence::SetBlock(BLOCK* SourceBlock, unsigned long BitIndex, unsigned long BlockSize)
{

	//this function  can be optimized in the same way I have delt with 
	// GetBlock but I postpone the task for futur.
	
	CaBinarySequence Buffer;
	Buffer.SetSequence(SourceBlock, BlockSize);
	
	for(unsigned  long i = 0; i < BlockSize; i++)
		Buffer[i] ? SetBit(BitIndex + i) : ClearBit(BitIndex + i);
		
}

// The Copy Constructor
CaBinarySequence::CaBinarySequence(CaBinarySequence& SourceSequence)
: m_SequenceLength(0)
{
	
	//It is a copy constructor, so I don't have to be worried about
	//pre-allocated memories
	if (!(m_pBinarySequence = new BLOCK [m_SequenceBlockLength] ))
	{
		//Error
		return;

	}
	
	m_SequenceLength = SourceSequence.m_SequenceLength;
	m_SequenceBlockLength = SourceSequence.m_SequenceBlockLength;
	//If we find room, we can load the sequence from file
	//I don't know if a new technology has been invented to optimize 
	//memory imaging so I use the arcade ways for it

	memcpy(m_pBinarySequence, SourceSequence.m_pBinarySequence, (m_SequenceBlockLength ) * BYTES_IN_BLOCK);
	
}

bool CaBinarySequence::operator ==(CaBinarySequence &LeftSequence)
{
	//If they don't have same length they are not equal and there is no need 
	//to carry out further testes
	if (m_SequenceLength != LeftSequence.m_SequenceLength) return false;

	//Then we check the complete Double Words together
	for(unsigned long i = 0; i < (m_SequenceLength / BLOCK_SIZE); i++)
	{
		if (m_pBinarySequence != LeftSequence.m_pBinarySequence) return false;

	}

	//At last those final bits which don't build complete Double Word
	for(unsigned long i = m_SequenceLength - (m_SequenceLength % BLOCK_SIZE); i < m_SequenceLength; i++)
		if (operator[](i) != ((CaBinarySequence)LeftSequence)[i]) return false;

	return true;

}

unsigned long CaBinarySequence::EvaluateBlock(unsigned long BitIndex, unsigned long BlockSize)
{
	//I want  to support the Evaluation of less than 32 byte if I decided to evaluate 
	//Bigger Blocks, the problem complexty would grow dramaticaly
	if (((BitIndex + BlockSize) > m_SequenceLength) || (BlockSize > BLOCK_SIZE)) return false;
	BLOCK* FormalBlock = GetBlock(BitIndex, BlockSize);
	
	char Result = *FormalBlock;
/*	if (Result == 1)
	{
		char temp[64];
		_itoa(BitIndex, temp, 10);
		TRACE(temp);
		TRACE(", ");

	}*/
	delete[] FormalBlock;

	return Result;

}

bool CaBinarySequence::InitializeCommittedEmpty(const unsigned long RoomSize  /* !in Bit!*/)
{

	//If there is another sequence is in the memory free I have to free it first to avoid 
	//memory leak
	
	m_SequenceLength = 0;

	if (m_pBinarySequence) delete[] m_pBinarySequence;
	
	//Roundoff Problem
	m_SequenceBlockLength =RoomSize + BLOCK_SIZE - 1;
	m_SequenceBlockLength /= BLOCK_SIZE;
	if (!(m_pBinarySequence = new BLOCK[m_SequenceBlockLength ] ))
	{
		//Error
		return false;

	}

	m_SequenceLength = RoomSize;
	//Intuatively, It assumes that the empty sequence is a ALL-ZERO
	//sequence so I respect this assumtion too
	memset(m_pBinarySequence, 0, (m_SequenceBlockLength) * BYTES_IN_BLOCK );
	//It isn't my guilt but I had to use this ugly sizeof(char) expression
	//It is because memset isn't a DWORD oriented function
	
	return true;

}

void CaBinarySequence::ClearBit(unsigned long BitIndex)
{
	m_pBinarySequence[BitIndex / BLOCK_SIZE] &= ~(TwoPower[BitIndex % BLOCK_SIZE]);

}

//These functions are added for usage in Mitra

void CaBinarySequence::SetSequence(const BLOCK* pNewSequence, unsigned long SequenceBitLength)
/*Gets a string of char* and store it in object of CaBinarySequence, it is 
   desirable when you have some bit job with you string 
   
   Parameters:
   pNewSequence	The Character String
   SequenceBitLength	The length of Character String in the sense of the
   number of BITS
*/
{

	/*Alocating Memory for bring whole sequence in to the heap for later analysis
	   If there is another sequence is in the memory free I have to free it first to avoid 
	   memory leak*/
	
	m_SequenceLength = 0;
	if (m_pBinarySequence) delete[] m_pBinarySequence;

	m_SequenceBlockLength = (SequenceBitLength + BLOCK_SIZE - 1) / BLOCK_SIZE;
	m_pBinarySequence = new BLOCK[m_SequenceBlockLength];
	//Out of Memory Check Point
	assert(m_pBinarySequence);

	m_SequenceLength = SequenceBitLength;

	memcpy(m_pBinarySequence, pNewSequence, (m_SequenceBlockLength) * BYTES_IN_BLOCK);
	
}

BYTE* CaBinarySequence::GetSequence() const
/* Returns the sequnece stored in side of CaBinarySequence Object in the 
	form of a arbitary char* string. It is called when all bit process has been
	done on the sequence and is ready to use in other classes
*/
{
	//Just for the sake of safety, The programmer should be cautious 
	//about deleting the result.
	BYTE* Result = (BYTE*) new BLOCK [m_SequenceBlockLength] ;
	//Out of Memory Check Point
	assert(Result);
	
	memcpy(Result, m_pBinarySequence, (m_SequenceBlockLength) * BYTES_IN_BLOCK);

	return Result;

}

bool CaBinarySequence::SerializeSequence(char* FileName)
{
	ofstream SequenceStorage;
	
	SequenceStorage.open(FileName, ios::out | ios::binary);
	if (!SequenceStorage.good())
	{
		throw("Error in opening File");
		return false;
	}
	

	//If we find room, we can load the sequence from file
		//The Read count argument is of type UINT (32bit int) so if is regriously has been chosen
		//The is no worry to read whole the sequence in one step
	SequenceStorage.write((char *)m_pBinarySequence, m_SequenceBlockLength * sizeof(BLOCK));

	return true;
		
}
