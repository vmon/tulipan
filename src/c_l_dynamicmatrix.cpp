/***************************************************************************
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 ***************************************************************************/
/*******LOGs
 * 
 * Thursday, 22 September 2005:
 *
 * Step by Step Generation of the matirix
 * 1.Setting the number of column
 * 2.  Setting the psudo random generator
 * 3. Computing Header size
 * 4. Generate the header.
 * 5. Generate Body
 * 5.5. Get the constant of the equations
 * 6. Accurate Body
 * 7. Solve the system
 * 8. return back the solution
 * 9. go to 4
 */

#include "c_l_dynamicmatrix.h"
#include "common.h"

#include <math.h>
#include <iostream>

using namespace std;

/*!
	We don't need to initialize the no of rows because the matrix is 
	dynamic in this sense and the encoding algorithm decide about it
 */
C_L_DynamicMatrix::C_L_DynamicMatrix(T_L_MatrixDimension NoOfColumns, C_L_RandomPool* pRandomGenerator, C_L_RandomGenerator* pSolutionGenerator)
 : C_L_BinaryMatrix(0, LeastGreaterMultiple(NoOfColumns, BINARY_STREAM_BLOCK_SIZE)),
  m_pMatrixGenerator(pRandomGenerator),
 m_pSolutionGenerator(pSolutionGenerator),
m_ProcessStatus(TLP_INCOMPLETE)
{

	/*When the No of Columns is known we can compute
		the header size. The header doesn't store anything
		beside the number of row in current matrix, the 
		number of rows is slightly less than the No of 
		dry columns. So we just need to set the size of header
		as much as it can hold K_avg number which is equal 
		to:

		NoOfMatrices = nr/k_avg => k_avg = nr/NoOfMatrices
		
		It is not known until two side agree on a r ratio.

		For this version we fix the header size, a good 
		k is suggested to be 250 however to remain in 
		a safe side we set the header size equal two bytes
		which could handle matrix with 65536 dry columns
	*/
	
	m_HeaderSize = 16;

	/* WARNING: In this implementation it is assumed that
		the header size is an integer multiple of 8, the no. of  bits in 
		a byte*/

}


C_L_DynamicMatrix::~C_L_DynamicMatrix()
{
}

/*/** Generate the first bunch of rows to save the size of 
	coming message chunk */
/*void C_L_DynamicMatrix::GenerateHeaderMatrix()
{
	
	   /*Hear the start point of generating matrix if any other stream is 
		associated to the matrix stream pointer should be vanish away
		because we deal with new one

	delete[] m_pMatrixStream ;
	
	m_pMatrixStream = m_pMatrixGenerator->RequestRandomStream(HeaderSize * m_NoOfColumns);

}*/

/*!
	
	\fn DrySubmatirx	()
	 
		Eliminate the columns of the matrix corresponding to the wet
	  coefficients and return the remaining submatrix to be solved

		as the gaussian elimination takes benefits of 
		integer blocks we have to cut as much as we get a
		matrix which its columns fits a integer number of
		blocks 
*/
C_L_BinaryMatrix* C_L_DynamicMatrix::DrySubmatrix(unsigned long BeginningElement)
{
	/*In this step we should delete away the column which correspond to a
		Wet Elements*/
	
	/* Preparation step to find out the submatrix dimension */
	bool* pNonDefictiveIndicator = new bool[m_NoOfColumns];
	memset(pNonDefictiveIndicator, 0, m_NoOfColumns* sizeof(bool));
	T_L_MatrixDimension NoOfHealthyElements = 0;

	for(int i = 0; i < m_NoOfColumns; i++)
		if (m_Humidity[BeginningElement + i]) //It means that the element is Dry
		{
			pNonDefictiveIndicator[i] = true;
			NoOfHealthyElements++;
			
		}

	/* The structure of guassian elimination assumes that the number of
		elements is integer multiple of BINARY_STREAM_BLOCK_SIZE*/
	T_L_MatrixDimension IntegerizedNoOfHealthyElements = LeastGreaterMultiple(NoOfHealthyElements, BINARY_STREAM_BLOCK_SIZE);

	/* The Submatrix Constraction */
	C_L_BinaryMatrix* pResult = new C_L_BinaryMatrix(m_NoOfRows, IntegerizedNoOfHealthyElements);

	for(int i = 0; i < m_NoOfRows; i++)
				//First we set the last block equal to zero
			pResult->SetBlock(i, (IntegerizedNoOfHealthyElements / BINARY_STREAM_BLOCK_SIZE)- 1, 0);

	T_L_MatrixDimension CurrentNonDefictiveColumn = 0;
	for(int j = 0; j < m_NoOfColumns; j++)
		if (pNonDefictiveIndicator[j])
		{
			for(int i = 0; i < m_NoOfRows; i++)
				pResult->SetElement(i, CurrentNonDefictiveColumn,  m_pMatrixStream[i * m_NoOfBlockColumns + j /BINARY_STREAM_BLOCK_SIZE] & Bit2BytePower[j % BINARY_STREAM_BLOCK_SIZE]);

			CurrentNonDefictiveColumn++;
		}

	delete[] pNonDefictiveIndicator;
	//pResult->PrintMatrix();
	////std::cout << "IN func" << std::endl;
	return pResult;
	
}


/*! For solving the matrix system we need the constant matrix,
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

	the estimated dryness ratio is need a periori to stegangraphy
	to estimate the header size, in this sense that the reciever 
	should know the header size as well. However in the improved
	version we first communicate these values.
*/
CaBinarySequence*  C_L_DynamicMatrix::MessageInjection(T_L_BinaryStreamBlock* Message, unsigned long MessageLength, T_L_BinaryStreamBlock* Parities, unsigned long ParitiesLength, T_L_BinaryStreamBlock* Humidity, double DrynessRatio, double EstimatedDrynessRatio)
{
	
	m_ProcessStatus = TLP_INCOMPLETE;
	//An estamte that how much of cofficients can be changed
	//These varibale are stored in the class members to be accessible by other member functions
	m_DrynessRatio = DrynessRatio;
	m_MessageLength = MessageLength;

	m_MessageSequence.SetSequence(Message, MessageLength);
	
	m_Parities.SetSequence(Parities, ParitiesLength);
	m_Humidity.SetSequence(Humidity, ParitiesLength);
	
	CaBinarySequence* pDeltaParities = new CaBinarySequence;
	pDeltaParities->InitializeCommittedEmpty(m_Parities.m_SequenceLength);

	//Computing Beta the total number of coding matrix
	m_TotalNoOfMatrices = CeillingDivide(ParitiesLength, m_NoOfColumns);

	//First we should estimate the header size as it suggested by the paper
	EstimateHeaderSize(EstimatedDrynessRatio, ParitiesLength);
	//Pay attention to deffirient meaning of Message Length
	unsigned long TotalMessageLength = MessageLength + (m_HeaderSize * m_TotalNoOfMatrices);

	/*If the message can be embed in the parties this 
		loop finish successfully*/
	unsigned long NoOfConsumedCoefficents = 0;
	unsigned long MessageDigester = 0;
	do
	{
			/*The first step is to generate the header and the default number 
				of row according to the secret scheme*/
			
			unsigned long NoOfDryCoefficients = ComputeNoOfDryCoefficients(NoOfConsumedCoefficents);
			T_L_MatrixDimension NoOfEstimatedRows = EstimateNoOfRows( NoOfDryCoefficients, TotalMessageLength, MessageLength - MessageDigester);

			//This function sets the m_NoOfRows as well
			GeneratePrimitiveMatrix(m_HeaderSize + NoOfEstimatedRows);
//			m_OriginalNoOfRows = m_NoOfRows;

/*			if (MessageDigester == 0)
			{
				char wait;
				//std::cin>> wait;
				//PrintMatrix();
				//std::cin >>wait;

			}*/

			//PrintMatrix();
			
			//char wait;
			////std::cin >> wait;

			/*Now we should eliminate the rows of the matrix until we heat
			one whose rows are indepenedent
			*/
			bool FullRowRankMatrixFound;
			do
			{
				
			 	//In each iteration we should code the header info again
				
				/*Header just shows how many rows are used to embed the
					message, q_i in the context of originial paper so
					the constant part is equal to:
	
						Constant = q_i + message_part
				*/

				/* AFTER DEBUG I am not sure but I think I should reinitialize the number of rows to the original one.
					
					2. I found it not necessary cause This is the internal loop which loop with dryness failure
				 */
				//m_NoOfRows = m_OriginalNoOfRows;
				T_L_BinaryStreamBlock* pEncodedMessage = new T_L_BinaryStreamBlock[CeillingDivide(m_NoOfRows, BINARY_STREAM_BLOCK_SIZE)] ;

				/*We are sure that m_NoOfRows is at least eqaul
				 To header size so we are allowed to do that*/
				memset(pEncodedMessage, 0, CeillingDivide(m_HeaderSize, 8));
				
				/*PORTABLITY WARNING: On the intel processor in which 
					the low significance byte stores sooner, this like should works*/
				if (MessageDigester < MessageLength)
				//There is still something to embed
				{
					T_L_MatrixDimension CurrentNoOfDataRows = m_NoOfRows - m_HeaderSize;
					memcpy(pEncodedMessage, &CurrentNoOfDataRows , CeillingDivide(m_HeaderSize, 8));
					
					memcpy(pEncodedMessage+ CeillingDivide(m_HeaderSize ,BINARY_STREAM_BLOCK_SIZE), m_MessageSequence.GetBlock(MessageDigester, CurrentNoOfDataRows), CeillingDivide(CurrentNoOfDataRows ,8));

				}
				else
				{
					//The final block is a block with the number header contains zero
					T_L_MatrixDimension FakeEstimation = 0;
					memcpy(pEncodedMessage, &FakeEstimation, CeillingDivide(m_HeaderSize, 8));

				}

				/* We should compute the constant part as follow

					ConstatnPart =m - Db

					so we should compute Db

				*/
				/* I don't understand why gcc is so handy cap in handling oveloaded operators!*/
				if (m_Parities.m_SequenceLength >= NoOfConsumedCoefficents + m_NoOfColumns)
				{
					C_L_BinaryMatrix TempMatrix(m_NoOfColumns, 1, m_Parities.GetBlock(NoOfConsumedCoefficents, m_NoOfColumns));

					//std::cout << "Raw Coeff" << std::endl;
				//	TempMatrix.PrintMatrix();

					TempMatrix = (*this) * TempMatrix;
					C_L_BinaryMatrix DeltaMatrix = C_L_BinaryMatrix(m_NoOfRows, 1, pEncodedMessage);
					//cout << m_NoOfRows << endl;

/*		JUST FOR DEBUG
			if (MessageDigester == 2816)
				{
					DeltaMatrix.PrintMatrix();
				//Just for test separated
				////std::cout << "Raw Coeff res" << std::endl;
					cout << endl;
					TempMatrix.PrintMatrix();
					cout << endl;

				}
					
				//std::cout << "Message" << std::endl;
				//DeltaMatrix.PrintMatrix();
*/
				
					DeltaMatrix = DeltaMatrix - TempMatrix;
/*				if (MessageDigester == 2816)
				{
					//std::cout << "message - raw" << std::endl;
						DeltaMatrix.PrintMatrix();
				}

				delete[] pEncodedMessage;

				//char wait;
				//std::cin >> wait;*/
				
				C_L_BinaryMatrix* pDriedMatrix = DrySubmatrix(NoOfConsumedCoefficents);
				
				//The iterative step is to run gaussian elimination until we hit a successfull one
				//pDriedMatrix->PrintMatrix();
				//	std::cin >> wait;
//					C_L_BinaryMatrix TestElim = (*pDriedMatrix);
					FullRowRankMatrixFound = pDriedMatrix->GaussianElimniation(DeltaMatrix.GetMatrixBinaryStream());
				//pDriedMatrix->PrintMatrix();
//				std::cin >> wait;
				//the solution indicate that which parities should be flipped
				//If the gaussian elimination fails the function retuns NULL
					if (!FullRowRankMatrixFound)
					{
						//pDriedMatrix->PrintMatrix();
						//std::cin >> wait;
/*						if (m_NoOfRows == 47)
						{
							int b = 100;
						}*/
						m_NoOfRows--;
						if (m_NoOfRows <= m_HeaderSize)
						{
							std::cout << "Embedding Process Failed! Not enough Dry Coefficients in Current Matrix." << std::endl;
							std::cout << "Decrease the number of matrices." << std::endl;

							m_ProcessStatus = TLP_FAILURE;
							break;
							
						}
							
						m_pMatrixGenerator->DenyCurrentRequest(m_NoOfColumns);

					}
					else
					{
						m_pMatrixGenerator->CommiteCurrentRequest();
	
						/*DEBUG
						if (MessageDigester == 2816) 
						{
							unsigned int temprow = m_NoOfRows;
							/*m_NoOfRows = 8;*/
							//this->PrintMatrix();
							/*m_NoOfRows = temprow;	
							pDriedMatrix->shouldIPrint = true;							
	
					}else
						{
							pDriedMatrix->shouldIPrint = false;
					
						}*/
	
						T_L_BinaryStreamBlock* pDeltaSolution = pDriedMatrix->FindaSolutiontoTheLinearSystem(m_pSolutionGenerator);
	
						/* JUST FOR DEBUG C_L_BinaryMatrix TempMatrix3(LeastGreaterMultiple(NoOfDryCoefficients, 8), 1, pDeltaSolution);
	
						C_L_BinaryMatrix TestSol =(* pDriedMatrix) * TempMatrix3;
						TestSol.PrintMatrix();
	
						if (MessageDigester == 2816) 
						{
							cout << "Just Dries" << endl;
							cout << NoOfDryCoefficients << endl;
							cout << "The Sole" << endl;

							TempMatrix3.PrintMatrix();
							
							cout << endl;

							CaBinarySequence TestSeq;


							C_L_BinaryMatrix TempMatrix5;
							TestSeq.InitializeCommittedEmpty(80);
							for(int i = 0; i < 8; i++)
								if (pDriedMatrix->m_pEliminatedConstants[i]) TestSeq.SetBit(i);
						
							C_L_BinaryMatrix Eliminated(8, 1, TestSeq.GetSequence());
							cout << "Eliminated Const" << endl;
							Eliminated.PrintMatrix();
							TempMatrix5 = (*pDriedMatrix) * TempMatrix3;
							cout << "Result" << endl;
							TempMatrix5.PrintMatrix();
	
							TempMatrix3 = TestElim * TempMatrix3;
							TempMatrix3.PrintMatrix();

							cout << "Before" << endl;
							TestElim.PrintMatrix();
							cout << "After" << endl;
							pDriedMatrix->PrintMatrix();

						}*/
		
						//UneliDry.PrintMatrix();
						//TempMatrix3 = UneliDry 	* TempMatrix3;
	
						//std::cout << "is still vaidl" << std::endl;
						//TempMatrix3.PrintMatrix();
					
						//char wait;
 					//std::cin >> wait;
	
						CaBinarySequence DeltaSequence;
						DeltaSequence.SetSequence(pDeltaSolution, NoOfDryCoefficients);

//					//std::cout << " 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0" << std::endl;

						//C_L_BinaryMatrix Test(LeastGreaterMultiple(NoOfDryCoefficients, BINARY_STREAM_BLOCK_SIZE), 1, pDeltaSolution);
					//((*pDriedMatrix) * Test).PrintMatrix();
					
						delete pDriedMatrix;
						delete[] pDeltaSolution;

						unsigned long i = NoOfConsumedCoefficents;
						NoOfConsumedCoefficents += m_NoOfColumns;
						//unsigned long Origin = i; //for test
						for(int DryCounter =  0; i < NoOfConsumedCoefficents; i++)
						{
								if (m_Humidity[i])
								{
									if(DeltaSequence[DryCounter])
									{
									//if (m_Parities[i]) m_Parities.ClearBit(i); else m_Parities.SetBit(i);
										pDeltaParities->SetBit(i); //for test
									//std::cout << " DELTA";
						 			}
									DryCounter++;
								//std::cout << DryCounter;
	
								}
						}

						/* JUST FOR DEBUG C_L_BinaryMatrix TempMatrix4(m_NoOfColumns, 1, pDeltaParities->GetSequence()+(NoOfConsumedCoefficents - m_NoOfColumns) / 8);
	
						C_L_BinaryMatrix TestSol =(* pDriedMatrix) * TempMatrix3;
						TestSol.PrintMatrix();
	
						if (MessageDigester == 2816) 
						{
							cout << "Result" << endl;
							cout << m_NoOfColumns << endl;
							TempMatrix3 = (*this) * TempMatrix4;
							TempMatrix3.PrintMatrix();

							
						}
			//JUST FOR TEST
						/*C_L_BinaryMatrix TempMatrix1(m_NoOfColumns, 1, m_Parities.GetBlock(Origin, m_NoOfColumns));

						//std::cout << "after delta" << std::endl;
						//TempMatrix1.PrintMatrix();

						//std::cout << "delta-res" << std::endl;
						TempMatrix1 = (*this) * TempMatrix1;
						//TempMatrix1.PrintMatrix();

						//PrintMatrix();
//						char wait;
						//std::cin >> wait;
			
					*/
	
						MessageDigester += m_NoOfRows - m_HeaderSize;
						std::cout  << MessageDigester << " bits out of " << MessageLength << " total bits of the message has been coded" << std::endl;
						//std::cout << NoOfConsumedCoefficents << "out of " << m_Parities.m_SequenceLength << std::endl;
	

					}

				//DELETE MANY THINGS
				}
				else
				{
						std::cout << "Embedding Process Failed! Run out of  Image Coefficients." << std::endl;
						std::cout << "Use another cover." << std::endl;

						m_ProcessStatus = TLP_FAILURE;
						break;

				}
				
			}while(!FullRowRankMatrixFound);

	//either fails or finished
		}while(((m_NoOfRows - m_HeaderSize) > 0) && (m_ProcessStatus != TLP_FAILURE));

	if (m_ProcessStatus == TLP_INCOMPLETE) m_ProcessStatus = TLP_SUCCESS;
	return pDeltaParities;

}

/*! This function computes the default number of rows of the underlying 
		matrix, this number is not certain and can be reduced because of 
		unsolvablity of final matrix
*/
T_L_MatrixDimension C_L_DynamicMatrix::EstimateNoOfRows(unsigned long NoOfDryCoefficients, unsigned long TotalMessageLength, unsigned long MessageRemainderLength)
{
	long Result;
	static unsigned long MaxRows = (((long)1 << m_HeaderSize) - 1);

	/* The estimate is exactly recommended by the paper, however
	there is problem with that formula proposed their that the size
	of header doesn't counted in the minization process.*/
	Result = CeillingDivide((unsigned long)(NoOfDryCoefficients * (TotalMessageLength  + 10)),(unsigned long)(m_DrynessRatio * m_Parities.m_SequenceLength));

	/* Because we computed header size independently, it should be reducted
		from the estimation */
	Result -= m_HeaderSize;
	
	//Up and down limits in the criticality order
	Result = (Result > NoOfDryCoefficients/ 2) ? Result : NoOfDryCoefficients / 2;
	Result = (Result < MessageRemainderLength) ? Result : MessageRemainderLength;
	Result = (Result < MaxRows) ? Result : MaxRows;

/*The matrix should at least be able to store the header, hear is 
	where this algorithm can collapse for a specific file and there is
	no explain about this problem in the paper, I guass that
	the algorithm can't embed a message in Totally Black JPEG Image*/
	//Result =  (Result <= m_HeaderSize) ? m_HeaderSize + 1: Result ;

	return (unsigned long) Result;

}

/* * Generate the header and the body of the random matrix
	to be the code matrix, if the matrix comes out too large to code
	the message, other functions are responsible to decrease it size
*/
void C_L_DynamicMatrix::GeneratePrimitiveMatrix(T_L_MatrixDimension NoOfRows)
{
	   /*Hear the start point of generating matrix if any other stream is 
		associated to the matrix stream pointer should be vanish away
		because we deal with new one*/

	delete[] m_pMatrixStream ;
	
	m_pMatrixStream = m_pMatrixGenerator->RequestRandomStream(NoOfRows * m_NoOfColumns);

	m_NoOfRows = NoOfRows;
		
}

/* * Just count the number of accepted coefficients
*/
unsigned long C_L_DynamicMatrix::ComputeNoOfDryCoefficients(unsigned long CurrentPositionInParities)
{
	unsigned long NoOfDryCoefficients = 0;

	for(int i = 0; i < m_NoOfColumns; i++)
		if (m_Humidity[CurrentPositionInParities + i])
			NoOfDryCoefficients++;

	return NoOfDryCoefficients;
}

CaBinarySequence*  C_L_DynamicMatrix::MessageRetrieval(CaBinarySequence* pParitySequence, double EstimatedDrynessRatio)
{
	
	CaBinarySequence MessageEstimation;
	
	//We allocate the maximum possible length for a message
	MessageEstimation.InitializeCommittedEmpty(pParitySequence->m_SequenceLength);
	T_L_MatrixDimension MessageLength = 0;

	
	m_TotalNoOfMatrices = CeillingDivide(pParitySequence->m_SequenceLength, m_NoOfColumns);

	//First we should estimate the header size as it suggested by the paper
	EstimateHeaderSize(EstimatedDrynessRatio, pParitySequence->m_SequenceLength);

	/*If the message can be embed in the parties this 
		loop finish successfully*/
	unsigned long NoOfConsumedCoefficents = 0;
	T_L_MatrixDimension NoOfCurrentMatrixRows = 0;
	do
	{
			/*The first step is to generate the header matrix 
				It tells us how many row current matrix actually have
			*/
			GeneratePrimitiveMatrix(m_HeaderSize);
			/*In retrieval Process we are always have confident about 
				our random requests*/

			m_pMatrixGenerator->CommiteCurrentRequest();

				/* JUST FOR DEBUG
				if (MessageLength == 2816)
				{
						unsigned int temprow = m_NoOfRows;
						m_NoOfRows = 8
						//this->PrintMatrix();
						cout << NoOfConsumedCoefficents << endl;
						cout 
						/*m_NoOfRows = temprow;*
				}

				if (MessageLength == 2810)
				{
					//CurrentDataVector.PrintMatrix();
					cout << NoOfConsumedCoefficents << endl;
					cout << m_NoOfColumns << endl;

					cout << pParitySequence->m_SequenceLength;
					for(long i = 0; i <m_NoOfColumns/* pParitySequence->m_SequenceLength; i++) 
					{
						cout << ((*pParitySequence)[ i+NoOfConsumedCoefficents] ? '1' : '0');
						if (!(i%100)) cout << endl;
				}

					cout << endl;
					int a = 1000;

				}*/
			C_L_BinaryMatrix CurrentDataVector(m_NoOfColumns, 1, pParitySequence->GetBlock(NoOfConsumedCoefficents, m_NoOfColumns));
			C_L_BinaryMatrix HeaderSizeVector =  (*this) * CurrentDataVector;

				/*JUST FOR DEBUG 
				if (MessageLength == 2816)
				{
					//CurrentDataVector.PrintMatrix();
			/*std::cout << "Size" << std::endl;
				//	HeaderSizeVector.PrintMatrix();
				char wait;
				std::cin >> wait;
				}*/
			BYTE pSizeDecoder[sizeof(T_L_MatrixDimension)]; 

			T_L_BinaryStreamBlock* CurrentStream = HeaderSizeVector.GetMatrixBinaryStream();
			memset(pSizeDecoder, 0, sizeof(T_L_MatrixDimension));
			memcpy(pSizeDecoder, CurrentStream, m_HeaderSize / 8);

			delete[] CurrentStream;

			NoOfCurrentMatrixRows =  *((T_L_MatrixDimension*)pSizeDecoder);
			/*DEBUG
				if (MessageLength == 2816)
				{
					//CurrentDataVector.PrintMatrix();
					cout << NoOfCurrentMatrixRows << endl;
				}*/
			/*Now the no of row in current matrix is retrieved */
			/* If there is any row*/
			/* We generate the code matrix to retrieve the message bits*/
			if (NoOfCurrentMatrixRows)
			{
				GeneratePrimitiveMatrix(NoOfCurrentMatrixRows);
				m_pMatrixGenerator->CommiteCurrentRequest();

				//And the message is retrieved
				C_L_BinaryMatrix MessageVector = (*this) * CurrentDataVector;

				//Set the  message in temporal sequence
				CurrentStream = MessageVector.GetMatrixBinaryStream();
				for(int i = 0; i < NoOfCurrentMatrixRows; i++)
					if (CurrentStream[i / BINARY_STREAM_BLOCK_SIZE] & Bit2BytePower[i % BINARY_STREAM_BLOCK_SIZE])
						MessageEstimation.SetBit(MessageLength + i);

				MessageLength += NoOfCurrentMatrixRows;

	
				std::cout << MessageLength << " bits of the message has been retrieved" << std::endl;
			
			}
		
	
/*		for(int i = 0; i < MessageLength / 8; i++)
		{
				unsigned char TestByte = MessageEstimation.GetStandardBlock(i);
				cout << hex << (unsigned int) TestByte << " ";
		}

		cout << dec << endl;*/

			NoOfConsumedCoefficents += m_NoOfColumns;
							
		}while(NoOfCurrentMatrixRows);

	CaBinarySequence* pResult = new CaBinarySequence;
	pResult->SetSequence(MessageEstimation.GetBlock(0, MessageLength), MessageLength);
	return pResult;

}

void C_L_DynamicMatrix::EstimateHeaderSize(double EstimatedDrynessRatio, unsigned long ParityLength)
{
	
	assert(m_TotalNoOfMatrices);

	m_HeaderSize = (unsigned int)(log(((EstimatedDrynessRatio * (double)ParityLength)	/ (double) m_TotalNoOfMatrices))/log(2) + 1);

	//In this implementation HeaderSize should be integer multiple of no of bytes
	m_HeaderSize = LeastGreaterMultiple(m_HeaderSize, 8);

}
