  /***************************************************************************
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 ***************************************************************************/
/*******LOGs
 *
 *
 * Wednesday, 14 September 2005:
 * the most important part of this class is gaussian elimination.
 *  The structure for the sake of speed is a 
 *  simple array of byte although I can use the binary sequence, it is not 
 * optimize for such operations
 *
 */
#ifndef C_L_BINARYMATRIX_H
#define C_L_BINARYMATRIX_H

#include "common.h"
#include "c_l_randomgenerator.h"

/**
This class is an efficient implementation of a binary matrix which support guassian elimination and consequently solving a system of linear equations with non-unique solution.

	The general equation which should be solved using this matrix class is of form
	
	Hv = m - Db

	In which the columns of H is just those correspond to dry coefficients. 
	Matrix v is equal to:

		v = b - b'

	And after solving the equation contains the change needed to be applied to
	dry elements.

	m - Db is a q x 1 matrix lik Hv, so we put the m - Db as the last column of Hv and 
	we apply the gaussian elimniation. After it it is straight forward to find out the 
	solutions.

*/

extern unsigned char Bit2BytePower[8];

class C_L_BinaryMatrix
{
protected:
	T_L_BinaryStreamBlock* m_pMatrixStream; //Memory storage of the matrix 
	
	T_L_MatrixDimension m_NoOfRows; //No of BIT Rows of the matrix
	T_L_MatrixDimension m_NoOfColumns; //No of BIT Columns of the matrix
	
	T_L_MatrixDimension m_NoOfBlockColumns; /*The colmun of T_BinaryStreamBlock 
			of the Matrix, it does not mean anything if the no of columns of a matrix is not a
			multiple of Block size,  it does not have any meaning to have NoOfRowBlocks, it
			it is always equal to the m_NoOfRows*/

	/* When a Gaussian Elimination is undertaken
		on a system of linear equation the eliminated consatant
		is saved in this varaible
	*/
//This line is just for debug reason!!!!!!!!!!!!!!!!!!!!!
public:
	T_L_Bit* m_pEliminatedConstants;

	static unsigned int m_sSizeOfBlock; /*For changing a bunch of bits to a 
		block or vice-versa we need frequently the size of block so I decided 
		to store it in a static varibale*/
	

	static const unsigned char Bit2BytePower[8];

public:
    /* * constructor build up a complete matrix at once */
	C_L_BinaryMatrix(T_L_MatrixDimension NoOfRows = 0, T_L_MatrixDimension NoOfColumns = 0, T_L_BinaryStreamBlock* InitialMatrix = NULL);
	//I don't see any necessity for changing the elements of the matrix after it has been stablished.
    ~C_L_BinaryMatrix();

	C_L_BinaryMatrix(const C_L_BinaryMatrix& TheMatrix); 
	C_L_BinaryMatrix operator=(const C_L_BinaryMatrix& LeftMatrix);

/*! This is to perform gaussian elimination on the matrix. the function 
			returns true if the matrix is found out to be full rank regarding 
			its rows (q independendt rows), it return false 
			(failure if the rows were not so);
	*/
	bool GaussianElimniation(T_L_BinaryStreamBlock* ConstantsCoefficients);

	/*!
		
		\fn 
		This function runs a gaussian elimination on the matrix, if it was successful, find a solution
		to the system, because we don't deal with square matrix, there may be infinite solution
		a RNG is needed to select solution, however the RNG should be different from one 
		which is shared with the recipent, because there is no synchornization method.
	
		If the function failed to solve the system due to elimination failure it returns NULL;
	*/
	T_L_BinaryStreamBlock* FindaSolutiontoTheLinearSystem(C_L_RandomGenerator* pSolutionGenerator/*, T_L_BinaryStreamBlock* ConstantsCoefficients*/);

//Matrix Arithmetic Operations	
	/* * The General matirx multiplication operator */
	C_L_BinaryMatrix& operator*(C_L_BinaryMatrix& LeftMatrix);

	/*! The General matirx addition operator */
	C_L_BinaryMatrix& operator+(C_L_BinaryMatrix& LeftMatrix);
	C_L_BinaryMatrix& operator-(C_L_BinaryMatrix& LeftMatrix);

	/*!
			Sets an element in the binary matrix. The beauty of the functions
			is that it does not matter what value, Value has. as long as it is true
			the function set true values.
	*/
	void SetElement(T_L_MatrixDimension i, T_L_MatrixDimension  j, T_L_Bit Value)
	{
		if (Value)
			m_pMatrixStream[i * m_NoOfBlockColumns + j / BINARY_STREAM_BLOCK_SIZE] |= Bit2BytePower[j % BINARY_STREAM_BLOCK_SIZE];
		else
		{
			m_pMatrixStream[i * m_NoOfBlockColumns + j / BINARY_STREAM_BLOCK_SIZE] &= ~Bit2BytePower[j % BINARY_STREAM_BLOCK_SIZE];
		}

	}

	void SetBlock(T_L_MatrixDimension i, T_L_MatrixDimension jOfBlock, T_L_BinaryStreamBlock Value)
	{
		m_pMatrixStream[i * m_NoOfBlockColumns + jOfBlock] = Value;

	}

	/*!
		\fn C_L_BinaryStreamBlock* GetMatrixBinaryStream()
	
		This function return back the bit stream memroy of the matrix
		which stores the matrix bits. When the individual element of 
		the matrix is need, in gaussian elimiantion for example, it appears 
		useful. 

		For the reason of safety, it does not return back the pointer to the 
		buffer but it make another copy of the buffer in the heap.

		return value: A copy of the matrix element buffer
	*/
	T_L_BinaryStreamBlock* GetMatrixBinaryStream()
	{
		T_L_BinaryStreamBlock* ImmitateMatrix = new T_L_BinaryStreamBlock[CeillingDivide(m_NoOfRows * m_NoOfColumns, BINARY_STREAM_BLOCK_SIZE)];
		memcpy(ImmitateMatrix, m_pMatrixStream, (CeillingDivide(m_NoOfRows * m_NoOfColumns ,BINARY_STREAM_BLOCK_SIZE)) *BYTES_IN_BINARY_STREAM_BLOCK);

		return ImmitateMatrix;

	}

	void PrintMatrix();

	bool shouldIPrint;

};
#endif
