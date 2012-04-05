/***************************************************************************
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 ***************************************************************************/
/*******LOGs
 *
 * Friday, April 26, 2006:
 * If the first element of the row-reduced matrix (0,0) be zero the
 * the pSolution[0] was not set and would damage the other solutions 
 * in the step we combine them to constructs the bytes from bits.
 * I have solved the problem by set pSolution with 0 default value.
 */
#include "c_l_binarymatrix.h"

#include <iostream>

using namespace std;
const unsigned char C_L_BinaryMatrix::Bit2BytePower [8]={1,2,4,8,16,32,64,128};

C_L_BinaryMatrix::C_L_BinaryMatrix(T_L_MatrixDimension NoOfRows, T_L_MatrixDimension NoOfColumns, T_L_BinaryStreamBlock* InitialMatrix)
: m_pEliminatedConstants(NULL)
{
	m_NoOfRows = NoOfRows;
	m_NoOfColumns = NoOfColumns;
	m_NoOfBlockColumns = CeillingDivide(NoOfColumns, BINARY_STREAM_BLOCK_SIZE);

	if ((m_NoOfRows) && (m_NoOfColumns)) //both dimension is set validly
	{
		m_pMatrixStream = new T_L_BinaryStreamBlock[CeillingDivide((m_NoOfRows * m_NoOfColumns) , BINARY_STREAM_BLOCK_SIZE) ];
		
		if (InitialMatrix)
			memcpy(m_pMatrixStream, InitialMatrix, CeillingDivide((m_NoOfRows * m_NoOfColumns) , BINARY_STREAM_BLOCK_SIZE) * BYTES_IN_BINARY_STREAM_BLOCK);
	}
	else
		m_pMatrixStream  = NULL;
	
}

C_L_BinaryMatrix::~C_L_BinaryMatrix()
{
	delete[] m_pMatrixStream;	
}

/* * The General matirx multiplication operator 
	WARNING THIS JUST WORKS FOR m x n * n x 1
	AND n MUST BE INTEGER MULTIPLE OF BLOCK SIZE
*/
C_L_BinaryMatrix& C_L_BinaryMatrix::operator*(C_L_BinaryMatrix& LeftMatrix)
{
	//The multiplication should be possible
	assert(m_NoOfColumns == LeftMatrix.m_NoOfRows);
	assert(m_NoOfBlockColumns == m_NoOfColumns / BINARY_STREAM_BLOCK_SIZE);
	assert(LeftMatrix.m_NoOfColumns == 1);

	C_L_BinaryMatrix* pResultat = new C_L_BinaryMatrix(m_NoOfRows, 1);

	/*Because we work with OR operator it is essential to set the matrix
		initially to zero
	*/
	for(int i = 0; i < CeillingDivide(m_NoOfRows, BINARY_STREAM_BLOCK_SIZE); i++)
		pResultat->m_pMatrixStream[i] = 0;

	for(int i = 0; i < m_NoOfRows; i++)
	{
		//for(int j = 0; j < LeftMatrix.m_NoOfColumns / BINARY_STREAM_BLOCK_SIZE; j++) //there isjust one column
			unsigned char CurrentBit = 0;
			for(int k = 0; k < m_NoOfBlockColumns; k++)
			{
				T_L_BinaryStreamBlock ModulationBuffer = m_pMatrixStream[i * m_NoOfBlockColumns + k] & LeftMatrix.m_pMatrixStream[k/*+ j*/];
				CurrentBit ^= pADD_BIT_TOGETHER[ModulationBuffer / 16] ^  pADD_BIT_TOGETHER[ModulationBuffer % 16];
			}
			
			pResultat->m_pMatrixStream[i / BINARY_STREAM_BLOCK_SIZE] |= CurrentBit << (i %  BINARY_STREAM_BLOCK_SIZE);

	}

	return *pResultat;

}

/* * The General matirx addition operator */
C_L_BinaryMatrix& C_L_BinaryMatrix::operator+(C_L_BinaryMatrix& LeftMatrix)
{
	//The addition should be possible
	assert((m_NoOfRows == LeftMatrix.m_NoOfRows ) && (m_NoOfColumns == LeftMatrix.m_NoOfColumns));

	/*I use pointers to have variable in heap and I can return it as reference
		consequently avoid extra copy*/
	C_L_BinaryMatrix*  pResultat =  new C_L_BinaryMatrix(m_NoOfRows, m_NoOfColumns);

	for(int i = 0; i < CeillingDivide(m_NoOfRows * m_NoOfColumns,  BINARY_STREAM_BLOCK_SIZE); i++)
				pResultat ->m_pMatrixStream[i] = m_pMatrixStream[i] ^ LeftMatrix.m_pMatrixStream[i];

	return *pResultat;

}

/* * The General matirx subtraction operator */
C_L_BinaryMatrix& C_L_BinaryMatrix::operator-(C_L_BinaryMatrix& LeftMatrix)
{
	
	//There is no difference between + and - in GF(2)
	return operator+(LeftMatrix);
	//The operation should be possible
	/*if ((m_NoOfRows != LeftMatrix.m_NoOfRows ) || (m_NoOfColumns != LeftMatrix.m_NoOfColumns))
	{
		throw("Operation Not Allowed");
		return NULL;

	}

	C_L_BinaryMatrix* pResultat  =   new C_L_BinaryMatrix(m_NoOfRows, m_NoOfColumns);

	for(int i = 0; i < (m_NoOfRows * m_NoOfColumns) / BINARY_STREAM_BLOCK_SIZE; i++)
				pResultat->m_Matrix[i] = m_Matrix[i] ^ LeftMatrix.m_Matrix[i];

	C_L_BinaryMatrix& Resultat = *pResultat;
	return Resultat;*/

}

/*!

	\fn bool C_L_BinaryMatrix::GaussianElimniation(T_L_BinaryStreamBlock* ConstantsCoefficients)
	
	Gaussian Elimination returning bool, when the matrix is full row rank
	it returns true, if it is not full row rank matrix return false. It also
	runs the gaussian elimination tasks on the constant coefficients 
	ConstantsCoefficients, so the solver does not need to run another 
	elimination to find the solution when this function returns true.
	
*/
bool C_L_BinaryMatrix::GaussianElimniation(T_L_BinaryStreamBlock* pConstantsCoefficients)
{
	int NewRow, CurrentRow=0;
	
	/*I use this varaible for combining matrix elimination with constant 
		elimination, but for the sake of the speed of byte operation
		in compare to bit operation, I add a complete byte to ensure that
		we can keep on our byte operations.
	*/
	
	T_L_BinaryStreamBlock* pExtendedMatrixStream = new T_L_BinaryStreamBlock[m_NoOfRows * (m_NoOfBlockColumns + 1)];

	/* Becasue of the structure of new matrix we have to copy the previous one 
		row by row.
	*/
	T_L_BinaryStreamBlock* pSourceRowIndicator = m_pMatrixStream;
	T_L_BinaryStreamBlock* pTargetRowIndicator = pExtendedMatrixStream;

	for(unsigned long i = 0; i < m_NoOfRows; i++)
	{
		memcpy(pTargetRowIndicator, pSourceRowIndicator, m_NoOfBlockColumns * BYTES_IN_BINARY_STREAM_BLOCK);

		pSourceRowIndicator +=m_NoOfBlockColumns;
		pTargetRowIndicator += m_NoOfBlockColumns;

		(*pTargetRowIndicator) = ((*(pConstantsCoefficients + (i / BINARY_STREAM_BLOCK_SIZE))) & Bit2BytePower[i% BINARY_STREAM_BLOCK_SIZE]) ? 1 : 0;
		pTargetRowIndicator++;

	}

	T_L_MatrixDimension ExtendedNoOfBlockColumns = m_NoOfBlockColumns + 1;

	//std::cout << m_NoOfColumns << std::endl;
/*	for(int i = 0; i < m_NoOfRows; i++) {

		for(int j = 0; j < m_NoOfColumns + 1; j++);
			//std::cout << (((pExtendedMatrixStream[( i*(m_NoOfColumns + 8)+j)/8] &Bit2BytePower[(i*(m_NoOfColumns + 8)+j) % 8])) ? "1 " : "0 ");

		//std::cout << std::endl;

}*/
	/*We should just count till the number of columns because we don't need
		to consider the row holding the constant*/
	for (int j=0; j<m_NoOfColumns; j++)
	{	
		if(CurrentRow == m_NoOfRows-1) break;
		for (int i = CurrentRow; i < m_NoOfRows; i++)
			{
			if ((pExtendedMatrixStream[(i*(m_NoOfColumns + 8)+j)/8] &Bit2BytePower[(i*(m_NoOfColumns + 8)+j) % 8]) != 0)
				{
				if (CurrentRow != i)
				{
					NewRow=i;
					for (int k=0; k < ExtendedNoOfBlockColumns; k++)
						{
						unsigned char Temp = pExtendedMatrixStream[(CurrentRow*(ExtendedNoOfBlockColumns))+k];
						pExtendedMatrixStream[(CurrentRow*(ExtendedNoOfBlockColumns ))+k] =  pExtendedMatrixStream[(NewRow *(ExtendedNoOfBlockColumns))+k];
						pExtendedMatrixStream[(NewRow*(ExtendedNoOfBlockColumns))+k] = Temp;
					}
				}
		
				for (int l=CurrentRow+1; l < m_NoOfRows ; l++)
				{
					//Just XOR when it is not zero 			
						if ((pExtendedMatrixStream[(l *(m_NoOfColumns + 8)+j)/8] &Bit2BytePower[(l*(m_NoOfColumns + 8)+j) % 8]) != 0)
						{
							for (int k=0; k < ExtendedNoOfBlockColumns; k++)
							{	
								(pExtendedMatrixStream[(l*ExtendedNoOfBlockColumns)+k])= (pExtendedMatrixStream[(l*ExtendedNoOfBlockColumns)+k]) ^ (pExtendedMatrixStream[CurrentRow*ExtendedNoOfBlockColumns + k]);
							}	
						}	
					}
/*				for(int p = 0; p < m_NoOfRows; p++) {

		for(int q = 0; q < m_NoOfColumns + 1; q++)
			//std::cout << (((pExtendedMatrixStream[( p*(m_NoOfColumns + 8)+q)/8] &Bit2BytePower[(p*(m_NoOfColumns + 8)+q) % 8])) ? "1 " : "0 ");

		//std::cout << std::endl;
	}
	char wait1;
	std::cin >> wait1;*/

				CurrentRow++;
				break; //If we find our row we go to next row
				
			}
		}
	}

/*	std::cout << m_NoOfColumns << std::endl;
	for(int i = 0; i < m_NoOfRows; i++) {

		for(int j = 0; j < m_NoOfColumns + 1; j++)
			std::cout << (((pExtendedMatrixStream[( i*(m_NoOfColumns + 8)+j)/8] &Bit2BytePower[(i*(m_NoOfColumns + 8)+j) % 8])) ? "1 " : "0 ");

		std::cout << std::endl;
	
		}

	char wait;
	std::cin >> wait;
	//std::cout << "ELIMNITATED CONST:"<< std::endl;*/

	for (int n=0; n< m_NoOfBlockColumns; n++)
	{	
		if (pExtendedMatrixStream[(m_NoOfRows-1)*ExtendedNoOfBlockColumns + n] != 0 )
		//The matrix is appeared to be full-row rank 
		{
			delete m_pEliminatedConstants;
			m_pEliminatedConstants = new T_L_Bit[m_NoOfRows];

			pSourceRowIndicator = pExtendedMatrixStream;
			pTargetRowIndicator = m_pMatrixStream; 

			for(int i = 0; i < m_NoOfRows; i++)
			{
				memcpy(pTargetRowIndicator, pSourceRowIndicator, m_NoOfBlockColumns * BYTES_IN_BINARY_STREAM_BLOCK);

				pSourceRowIndicator +=m_NoOfBlockColumns;
				pTargetRowIndicator += m_NoOfBlockColumns;

				m_pEliminatedConstants[i] = *pSourceRowIndicator;
				pSourceRowIndicator++;

				//std::cout << (int)m_pEliminatedConstants[i] << std::endl;

			}
					
			return true;

		}
	}

	return false;

}

/*!
	This function is solving a system of linear equation in GF(2)

	According the current configuration, this function is called after the 	
	matix is	gaussian elminated successfully. So, the sole job remains is 
	to compute the vaiable. 

	the function use its random generator to handle the extra variabls
*/ 
T_L_BinaryStreamBlock* C_L_BinaryMatrix::FindaSolutiontoTheLinearSystem(C_L_RandomGenerator* pSolutionGenerator)
{

	T_L_Bit* pSolutions = new T_L_Bit[m_NoOfColumns];

	//AFTER DEBUG
	memset(pSolutions, 0, m_NoOfColumns * sizeof(T_L_Bit));

	//Debug
	//PrintMatrix();

	unsigned int FirstEvaluatedVariable = m_NoOfColumns;
	for(int i = m_NoOfRows - 1; i >= 0; i--)
    {
		//The first step is to find the first free variable
		// The first time that I found a variable I can set it 
 		//according to the constant and set the others randomly
		//Just I should keep the track that I don't set them for the next time
		for(int j = 0; j < m_NoOfBlockColumns; j++)
			if (m_pMatrixStream[i * m_NoOfBlockColumns + j])
			/*The search should find a solution before reaching to the 
				FirstEvaluatedVariable because the gaussian elimination has
				been succeeded
			*/
			{
				T_L_MatrixDimension CurrentBlock = i * m_NoOfBlockColumns + j;
				for(int k = 0; k < BINARY_STREAM_BLOCK_SIZE; k++)
					if (m_pMatrixStream[CurrentBlock] & Bit2BytePower[k])
					/* The first no  n-zero bit of the block */
					{
						/* We keep this first variable for consistancy of equation
						    and evaluate all other randomly
						*/
						T_L_MatrixDimension ConsistencyVariable = j * BINARY_STREAM_BLOCK_SIZE + k;
						
						for(int l = ConsistencyVariable + 1; l < FirstEvaluatedVariable; l++)
						{
							char pRandomBuffer[64];
							pSolutionGenerator->GenerateRandomInBuffer(pRandomBuffer);
							pSolutions[l] = pRandomBuffer[0] & 1; //Should be changed in next version

						}
						
						//Now we should evaluate the consistency variablef
						pSolutions[ConsistencyVariable] = m_pEliminatedConstants[i];

						for(int l = ConsistencyVariable + 1; l < m_NoOfColumns; l++)
							if (m_pMatrixStream[i * m_NoOfBlockColumns + l /8] & Bit2BytePower[l % 8])
								pSolutions[ConsistencyVariable] ^= pSolutions[l];

						/* FOR DEBUG
							if (shouldIPrint) cout << "Conistantcy Variable of Row" << i << ": " << ConsistencyVariable << "with value " << (pSolutions[ConsistencyVariable] ? 1 : 0)  << "for constant " << (m_pEliminatedConstants[i] ? 1 : 0)  << endl;
						*/

	////std::cout << "SolutionsMid:" <<  std::endl;
	unsigned char Resulti = 0;
	for(int k = 0; k < m_NoOfColumns; k++)
		 //if (i == 0) //std::cout << (int)pSolutions[k] << std::endl;
		if (pSolutions[k] && (m_pMatrixStream[i * m_NoOfBlockColumns + k /8] & Bit2BytePower[k % 8]))
			Resulti ^= 1;

	//std::cout << i << ":"<<(int)Resulti << std::endl;

			//char wait;
//			std::cin >> wait;
						FirstEvaluatedVariable = ConsistencyVariable;
						break;

					}

					break;
				}
				
		}

	/*
		Just FOR DEBUG
	if (shouldIPrint) cout << "Conistantcy Variable of Row" << 4 << ": " << 5 << "with value " << (pSolutions[5] ? 1 : 0)  << endl;

	std::cout << "SolutionsMid:" <<  std::endl;
	for(int i = 0; i < m_NoOfColumns; i++)
		 //std::cout << (int)pSolutions[i] << std::endl;*/

	//Now we should organize the solutions in a bit stream
	T_L_BinaryStreamBlock* pResult = new T_L_BinaryStreamBlock[m_NoOfBlockColumns];
	memset(pResult, 0, BYTES_IN_BINARY_STREAM_BLOCK * m_NoOfBlockColumns);

/*	//std::cout << "SolutionsMid:" <<  std::endl;
	for(int i = 0; i < m_NoOfColumns; i++)
		 //std::cout << (int)pSolutions[i] << std::endl;*/

	/* JUST FOR DEBUG
			if (shouldIPrint) cout << "blockcol"<< m_NoOfColumns;
				cout << hex;
	*/
	for(int i = 0; i < m_NoOfBlockColumns; i++)
	{
		/*Pay attention to the order of variable to understand 
			the logic just think that solution number zero should 
			in zero bit of the zero block*/
		//So it should be the highest bit in the block
		pResult[i] = pSolutions[(i + 1) * BINARY_STREAM_BLOCK_SIZE - 1];
		for(int j = BINARY_STREAM_BLOCK_SIZE - 2; j >= 0; j--)
		{
			pResult[i] <<= 1;
			pResult[i] |= pSolutions[i * BINARY_STREAM_BLOCK_SIZE + j];

/*	JUST FOR DEBUG		if ((shouldIPrint) && (i == 0)) cout << "1.Iteration" << j << " Result " << (int)pResult[i] << " Sole: " << (pSolutions[j] ? 1 :0) <<  endl;
			if ((shouldIPrint) && (i == 0)) cout << "2.Iteration" << j << " Result " << (int)pResult[i] << " Sole: " << (pSolutions[j] ? 1 :0) <<  endl;
			if ((shouldIPrint) && (i == 0)) cout << "3.Iteration" << j << " Result " << (int)pResult[i] << " Sole: " << (pSolutions[j] ? 1 :0) <<  endl;

			for(int k = 0; k < m_NoOfColumns; k++)
		 		//std::cout << ((pResult[k / 8]  & Bit2BytePower[k%8]) ? "1" : "0") ;

				//std::cout  << std::endl;
				char wait;
				std::cin >> wait;*/
		}
	}
	//std::cout << "Solutions:" <<  std::endl;
	//for(int i = 0; i < m_NoOfColumns; i++)
		 //std::cout << ((pResult[i / 8]  & Bit2BytePower[i%8]) ? "1 " : "0 ") << (int) pSolutions[i] << std::endl;

	//std::cout << "Straight solution multi" << std::endl;
	//C_L_BinaryMatrix TempMatrix1(m_NoOfColumns, 1, pResult);

	//for(int i = 0; i < m_NoOfColumns; i++);
		 //std::cout << ((pResult[i / 8]  & Bit2BytePower[i%8]) ? "1 " : "0 ") << (int) pSolutions[i] << 
//							((TempMatrix1.m_pMatrixStream[i / 8]  & Bit2BytePower[i%8]) ? " 1 " :  " 0 ") << std::endl;

	//TempMatrix1.PrintMatrix();

	//std::cout << "f"<< std::endl;
	//TempMatrix1 = (*this) * TempMatrix1;

	//TempMatrix1.PrintMatrix();
	/*if (shouldIPrint) cout << "Conistantcy Variable of Row" << 4 << ": " << 5 << "with value " << ((pResult[5 / 8]  & Bit2BytePower[5%8])  ? 1 : 0)  << endl;*/

	delete[] pSolutions;

	/*//std::cout << "Original:" <<  std::endl;
	for(int i = 0; i < m_NoOfRows; i++)
		 //std::cout << (int)m_pEliminatedConstants[i] << std::endl;

	char wait;
	std::cin >> wait;*/
	
	return pResult;

}

void C_L_BinaryMatrix::PrintMatrix()
{
	//std::cout << m_NoOfColumns << std::endl;
	for(int i = 0; i < m_NoOfRows; i++) {
		std::cout << i % 10 << ": ";
		for(int j = 0; j < m_NoOfColumns; j++)
			std::cout << (((m_pMatrixStream[( i*(m_NoOfColumns)+j)/8] &Bit2BytePower[(i*(m_NoOfColumns)+j) % 8])) ? "1 " : "0 ");

		std::cout << std::endl;
	}

}

C_L_BinaryMatrix::C_L_BinaryMatrix(const C_L_BinaryMatrix& TheMatrix)
:m_NoOfRows(TheMatrix.m_NoOfRows),
 m_NoOfColumns(TheMatrix.m_NoOfColumns),
m_NoOfBlockColumns(TheMatrix.m_NoOfBlockColumns)
 {

	if (m_NoOfRows * m_NoOfBlockColumns)
	{
		m_pMatrixStream = new T_L_BinaryStreamBlock[CeillingDivide((m_NoOfRows * m_NoOfColumns) , BINARY_STREAM_BLOCK_SIZE) ];
		memcpy(m_pMatrixStream, TheMatrix.m_pMatrixStream,CeillingDivide((m_NoOfRows * m_NoOfColumns) , BINARY_STREAM_BLOCK_SIZE) * BYTES_IN_BINARY_STREAM_BLOCK);

	}
	else
		m_pMatrixStream = NULL;
	
}

C_L_BinaryMatrix
C_L_BinaryMatrix::operator=(const C_L_BinaryMatrix& LeftMatrix)
{
 	m_NoOfRows = LeftMatrix.m_NoOfRows;
 	m_NoOfColumns = LeftMatrix.m_NoOfColumns;
 	m_NoOfBlockColumns = LeftMatrix.m_NoOfBlockColumns;

	if (m_NoOfRows * m_NoOfColumns)
	{
		delete[] m_pMatrixStream;
		m_pMatrixStream = new T_L_BinaryStreamBlock[CeillingDivide((m_NoOfRows * m_NoOfColumns) , BINARY_STREAM_BLOCK_SIZE) ];
		memcpy(m_pMatrixStream, LeftMatrix.m_pMatrixStream,CeillingDivide((m_NoOfRows * m_NoOfColumns) , BINARY_STREAM_BLOCK_SIZE) * BYTES_IN_BINARY_STREAM_BLOCK);

	}
	else
		m_pMatrixStream = NULL;

	return *this;

}

