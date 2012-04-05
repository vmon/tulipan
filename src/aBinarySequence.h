/******LOGs
/*
/* I have written this class for Randomness Clinic
/* but now as I have to deal with the Bitwise problem
/* solving, which arose in Field Element evaluatioon
/* 
/* I should add a function which can get the input sequence as 
/* a character string, which is our case for Secret and Shares
/*
/* for the convenience of Sharing the Char* Secrets it is better to change
/* from DWORD to char
*/

/*I changed this constant as a modern inside-the-class constant so I can 
    makes it equal to the sizeof(char*)
*/
#ifndef ABINARYSEQUENCE_H
#define ABINARYSEQUENCE_H

//#define BLOCK_SIZE		8
//#define BYTES_IN_BLOCK  1

typedef unsigned char BLOCK;
typedef unsigned char BYTE;

/* This class present a sequence with Bit Manipulation utilities, it was 
	not originally written for Mitra, for this reason it is much more 
	sophisticated and complex than it needed, but just few functions
	of it has been used in Mitra
*/
class CaBinarySequence
{
public:
	//The constant length of block
	static const unsigned int BLOCK_SIZE;
	static const unsigned int BYTES_IN_BLOCK;
	
	void ClearBit(unsigned long BitIndex);
	bool InitializeCommittedEmpty(const unsigned long RoomSize  /*in Bit!*/);
	unsigned long EvaluateBlock(unsigned long BitIndex, unsigned long BlockSize);
	CaBinarySequence(void);
	~CaBinarySequence(void);
	// The length of Sequence in Bits, but for the sake of speed it's always multiple of 32
	// The article was suggesting a typical length between 10^4 to 10^7 so it seems more than
	// enough to specify a 4 * 10 ^ 10 variable for the bits stream size
	unsigned long m_SequenceLength;
	unsigned long m_SequenceBlockLength;
	// I have implemented my sequence in the form of 32bits blocks cause I think when I have 32bits Processor it is the optimal (fastest) solution
	BLOCK* m_pBinarySequence;
	// The only way that someone could inject a such long random sequence into the program is using files, so it's reasonable to give this task to the holder class
	bool DigestSequence(char* FileName);
	bool SerializeSequence(char* FileName);
	// //The Other way of colning an exist sequence
	CaBinarySequence& operator=(const CaBinarySequence& SourceSequence);
	//Comparing two sequence bit by bit, I wanted to send the arguement as const but the compiler didn't let me
	bool operator==(CaBinarySequence& LeftSequence);
	// It is not a such important function cause there is no matter of setting just one bit but I impelemented for the necessary cases and for having a complete set
	bool operator[](unsigned long BitIndex);
	void SetBit(unsigned long BitIndex);
	// It is a fast solution to get sequence data (in block) in compare with the bit by bit slow way, however user is restricted to walk just by BLOCK_SIZE steps
	BLOCK GetStandardBlock(unsigned long BlockIndex);
	// It lets us to retrieve blocks with any desirable size but it is not much faster than bit by bit solution
	BLOCK* GetBlock(unsigned long BitIndex, unsigned long BlockSize);
	//For secret Sharing Problem I need to write GetBlock's sister
	//too
	void SetBlock(BYTE* SourceBlock, unsigned long BitIndex, unsigned long BlockSize);
	// The Copy Constructor
	CaBinarySequence(CaBinarySequence& CopySequence);
	
	//Mitra ->
	
	/*For entering a concrete ordianary sequence, using char* instead
	of everyday-DWORD* is for the usage of the class in secret
	sharing process
	
	Gets a string of char* and store it in object of CaBinarySequence, it is 
   desirable when you have some bit job with you string */
 	void SetSequence(const BYTE* pNewSequence, unsigned long SequenceBitLength);
	
	/* Returns the sequnece stored in side of CaBinarySequence Object in the 
	form of a arbitary char* string. It is called when all bit process has been
	done on the sequence and is ready to use in other classes
	*/
	BYTE* GetSequence() const;
	
};

#endif
