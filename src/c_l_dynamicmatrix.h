/***************************************************************************
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 ***************************************************************************/
/******LOGs
 *

 * Thursday, September 2005, 22
 * We need a temporary memory to save the not-used column of the matirx, 
 * A good strategi is to generate the matirix in the temp memory and whenever,
 * it was needed one could get coefficients from there.
 * the temporal matrix is self managed and if there is no coefficient anymore
 * it reproduce them. For this reason I design new class
 *
 *  Wednesday, Octobre 19, 2005:
 * I have finished the first stage implementation of the encoder!
 *
 * Tuesday, April 18, 2006
 * I have added the m_OriginalNoOfRows for the reason
 * of returning back of the m_NoOfRows after decrease process.
 * 
 * Wednesday, April 19. 2006,
 * I found it is not necessary to add that variable. I also added a 
 * Status which shows the status of embedding/retrieval process
 */
#ifndef C_L_DYNAMICMATRIX_H
#define C_L_DYNAMICMATRIX_H

#include "c_l_binarymatrix.h"
#include "c_l_randompool.h"
#include "aBinarySequence.h"


enum TLP_EMBRET_STATUS
{
	TLP_INCOMPLETE,
	TLP_SUCCESS,
	TLP_FAILURE
};
/**
It is the encapsulation of D matrix in PQ algorithm. Because each time it is generated, it should be generated in two steps and the H matrix is deduced from it by a selection criterion, it needs more dynamic structure and we separate it from its paranete tt, the C_L_BinaryMatrix

In this sense we should deal with two important operations:
	1.	Dynamic Generation, the matrix should be generated row by row without 
		losing any Ranodm Element because it should be generatable in recipient side.
		First the log_2[n] row of it should be generated to store the size of data, 
		then remaining rows.
	
	2.	It should let out a submatrix of itself with rows just correspond to the DRY 
		Coefficients.

An essential duty of this class is that if the Final Dry Linear System was not solvable,
it should reduces the size of matrix, but should not wape out the remaining rows,
instead they should be used for the next stage.

*/
class C_L_DynamicMatrix : public C_L_BinaryMatrix
{
protected:	
	T_L_MatrixDimension m_HeaderSize; //No of rows spent on the rows.
	/*T_L_MatrixDimension m_OriginalNoOfRows; /*As the Number of Rows devrease
						in existance of high density of humid coefficients we have to 
						return it back for the next matrix */
	TLP_EMBRET_STATUS m_ProcessStatus;

	//For dynamic forward/backward generation
	T_L_MatrixDimension m_NoOfConsumedRows;

	C_L_RandomPool* m_pMatrixGenerator; /*The buffer which keeps track of 
		random elements*/
	
	/* Random Generator which generate random solution for free extra 
		variables, in this version this random generator is tuned by the user
		key but it should be tuned by the system random entropy
	*/
	C_L_RandomGenerator* m_pSolutionGenerator;

	CaBinarySequence m_MessageSequence;
	CaBinarySequence m_Parities;
	CaBinarySequence m_Humidity; //If an element is humid should be 0

	unsigned long m_MessageLength; /*It should be saved here because it is used by
		many class member functions*/
	double m_DrynessRatio;

	/* This number has a fundamental role in PQ algorithm
		and show how many submatrix we are allowed to generate*/
	unsigned long m_TotalNoOfMatrices;

	/** Generate the header and the body of the random matrix
		to be the code matrix, if the matrix comes out too large to code
		the message, other functions are responsible to decrease it size
	*/
	void GeneratePrimitiveMatrix(T_L_MatrixDimension NoOfRows);

	/** Just count the number of accepted coefficients
	*/
	unsigned long ComputeNoOfDryCoefficients(unsigned long CurrentPositionInParities);

	/*! This function computes the default number of rows of the underlying 
		matrix, this number is not certain and can be reduced because of 
		unsolvablity of final matrix
	*/
	T_L_MatrixDimension EstimateNoOfRows(unsigned long NoOfDryCoefficients, unsigned long TotalMessageLength, unsigned long MessageRemainderLength);

public:
    C_L_DynamicMatrix(T_L_MatrixDimension NoOfColumns = 0, C_L_RandomPool* pMatrixGenerator = NULL, C_L_RandomGenerator* pSolutionGenerator = NULL);

    ~C_L_DynamicMatrix();
	
	/** Generate the first bunch of rows to save the size of 
		coming message chunk */
	/*void GenerateHeaderMatrix();
		There is no reason to generate the header and
		the body in two steps*/
	
	/** Eliminate the columns of the matrix corresponding to the wet
		  coefficients and return the remaining submatrix to be solved
	*/
	C_L_BinaryMatrix* DrySubmatrix(unsigned long BeginningElement);
	
	/** This function is a message to dynamic matrix
		to shrink it size in hope to the smaller set of rows
		become linearly indepenedent.
	*/
	void FailBack(T_L_MatrixDimension ReductionSize);

	/** For solving the matrix system we need the constant matrix,
		according to Wet Code the constant matrix is:
		
			m - Db
		for computing this parts, one need the message and the 
		PARITY of the coefficient which the manager should prepare for this
		class using this function.

		It is worthy of noting that if the matrix D is found out to be too 
		large to be full row rank, the message size decrease automatically
		and the manager does not need to inform any thing to the
		C_L_DaynamicMatrix. However the next time it should not send
		the overlapping informations.
	*/
	CaBinarySequence* MessageInjection(T_L_BinaryStreamBlock* Message, unsigned long MessageLength, T_L_BinaryStreamBlock* Parities, unsigned long ParitiesLength, T_L_BinaryStreamBlock* Humidity, double DrynessRatio, double EstimatedDrynessRatio);

	CaBinarySequence* MessageRetrieval(CaBinarySequence* pParitySequence, double EstimatedDrynessRatio);

	/** Inteface Functions*/
	//The size of header is known from the secret info not any more*/
	inline void SetHeaderSize(T_L_MatrixDimension HeaderSize)
	{
		m_HeaderSize = HeaderSize;

	}

	/**
	 *    Get the status of the last embedding retrieval process
	 * @return The status
	 */
	inline TLP_EMBRET_STATUS GetStatus()
	{
		return m_ProcessStatus;
	}
	
	void EstimateHeaderSize(double EstimatedDrynessRation, unsigned long ParityLength);

};

#endif
