/***************************************************************************
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 ***************************************************************************/

/********LOGs
 * 
 * Monday, 12 September 2005:
 * In the name of God, I am starting the implementation. the first step is to
 * load a jpeg file. Actually we don't need the resulted pixels but during 
 * the process of compression/decompression the access to DCT Coefficients
 * is of real important for us. So the purpose is to intervening during DCT
 * operation, a vague sample of such operation is represented in OutGuess
 * implementation but it is not enough for us as we should aquire and inject
 * the information during Quantization process.
 * 
 * For experimental purpose the first step is to soak in the JPEG Group 
 * Official Implementation in Tulipan and check which Forward DCT 
 * function is used to obtain the DCT Coefficients.
 *
 * I'm not sure which files from JPEG implementatation is necessary, for this reason 
 * I have to import all and see if I can compile them or not.
 * 
 *  A good guide is to compile the example JPEG Encoder and The JPEG DECODER
 * and see which files are need to finish a complete jpeg operation.
 * 
 * Tuesday, 13 September, 2005:
 * The jpeg library gives me the ability to read the coefficient meanwhile
 * also it permits to specfy the quantization table manually what would I 
 * is to read the coefficient and then quantize them manually after it I
 * represent the library a all 1 elements matrix as a quantization table
 * to prevent it from intervening the quantization process in this sense
 * I don't have to import the source functions of the library and I can
 * simply linked to libjpeg.a
 * 
 * It is the time that one should decide about the structure of project.
 * I predicate the necessity of following classes:
 *
 *		C_L_DCT_Coefficient
 *		C_L_Message
 *
 *  	C_L_JPEG_Interface
 *
 *		C_L_WetEncoder
 *		C_L_WetDecoder
 *		C_L_Selector
 *		C_L_Quantizer
 *		
 *		C_L_WetSteganographer
 *
 *		Today I'm implementing C_L_JPEGInterfacce
 * 
 *	Wednesday, 14 September 2005:
 * Successfully C_L_JPEG_Interfacce has imported the DCT Coefficient and
 * wrote them with different quantization table, it means that libjpeg satisfies
 * our need and we don't have to hack into the code.
 * 
 * Obviously, we need a PRNG, I use the one that I have implemented for Mitra,
 * base on SHA-1, though it may not be so secure and we have to change it next
 * improvement.
 * 
 * The algorithm for the fist release is so simple:
 *   1. PRNG arrange all thecoefficients.
 *   2. PRNG Generate the whole encoding matrix.
 *	 3. The encoding algorithm in 3.4 applied.
 *
 * Thursday, September 15, 2005:
 * Started testing WetSteganographer.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <cstdlib>

#include "common.h"
#include <stdio.h>
#include "c_l_wetsteganographer.h"
using namespace std;


/*!
	\fn bool CommandLineInterpreter(int argc, char *argv[])
	
	This function get the command line parameter from the main and 
	call the approprate functions to handle user request 

	If the interpretation process failed for any reason the function 
	return FALSE.
*/
bool  CommandLineInterpreter(int argc, char *argv[])
{

	C_L_WetSteganographer WetSteganographer;

	S_L_StegoKey StegoKey;
	unsigned int HumidityThreshold;
	unsigned int RecompressionQuality;
	unsigned long NoOfMatrices;

/*	unsigned int ArgumentCounter;
	unsigned int CommandCounter;
	unsigned int ParameterCounter, ParameterCount; /*Counts the paramter 
								   of a single command*/

	if (argc == 1) return false;

/*	for(ArgumentCounter = 1; ArgumentCounter < argc; ArgumentCounter++)
	{*/
		/*When the intepretation process reach this point, an "-"
			sign before the argument expected. */

		if (argv[1][0] != '-')
		{
			//PushError(UC_ERROR_SYNTAX);
			return false; //User's synax error 

		}
		
//		ParameterCounter = ArgumentCounter + 1;
		/*Find the number of paramter which is need for 
		  allocating proper memory
		 */
		
/*		while(ParameterCounter < argc && argv[ParameterCounter][0] != '-')
			ParameterCounter++;
		
		if (ParameterCounter <= argc)
			ParameterCount = ParameterCounter - ArgumentCounter - 1;
		else
			ParameterCount = 0;*/

	

		/*Search for the command in the command pool*/
		//for(CommandCounter = 0; CommandCounter < COMMAND_COUNT; CommandCounter++)
		
		if (!strcmp("encode", &(argv[1][1])))
		{
			/*Command Found call the corresponding fucntion*/
				if (argc < 9) return false;
			
				StegoKey.SeedSize = strlen(argv[5]) * 8;
				StegoKey.pPRNG_Seed = new char[StegoKey.SeedSize + 1];
				strcpy(StegoKey.pPRNG_Seed, argv[5]);

				HumidityThreshold = atoi(argv[6]);
				RecompressionQuality = atoi(argv[7]);
				NoOfMatrices = atol(argv[8]);

				WetSteganographer.InitializeStegoScheme(HumidityThreshold, RecompressionQuality, &StegoKey, NoOfMatrices);

				//cout << argv[0] << ", " << argv[4] << " , " << HumidityThreshold << ", " << RecompressionQuality << ", " << NoOfMatrices << endl;

				if (!WetSteganographer.EmbedMessageInCoverMedia(argv[2], argv[3], argv[4])) 
					cout << "Embeding Process Failed!" << endl;
				else
					cout << "The message is embeded successfully" << endl;

				return true;	
			
		}
		else if  (!strcmp("decode", &(argv[1][1])))
		{
				if (argc < 7) return false;
			
				StegoKey.SeedSize = strlen(argv[4]) * 8;
				StegoKey.pPRNG_Seed = new char[StegoKey.SeedSize + 1];
				strcpy(StegoKey.pPRNG_Seed, argv[4]);

				HumidityThreshold = atoi(argv[5]);
				NoOfMatrices = atol(argv[6]);

				WetSteganographer.InitializeStegoScheme(HumidityThreshold, 0, &StegoKey, NoOfMatrices);

				//cout << argv[0] << ", " << argv[4] << " , " << HumidityThreshold << ", " << RecompressionQuality << ", " << NoOfMatrices << endl;

				if (!WetSteganographer.RetrieveMessage(argv[2], argv[3])) 
					cout << "Retrieving Process Failed!" << endl;
				else
					cout << "The message is retrieved successfully" << endl;

				return true;	
		}
		else 
			return false;


}
void ShowUsage()
{
	cout << "Tulipan version 0.0.1" << endl;
	cout << "Usage:" << endl;
	cout << "./tulipan -encode  <Message filename> <Cover image filename> <Output image filename> <key> <Humidity threshold> <Recompression quality> <No of matrices>" << endl;
	cout << endl;
	cout << "./tulipan -decode <Stego image filename> <Output message filename> <key> <Humidity threshold> <No of matrices>";
	cout << endl;

}

int main(int argc, char *argv[])
{
	if (!CommandLineInterpreter(argc, argv))
		ShowUsage();

  return EXIT_SUCCESS;

}
