/***************************************************************************
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 ***************************************************************************/
/********LOGs
 * 
 * Saturday, September 17, 2005:
 * I just managed to import coefficient in my desired format
 * I start writing recompression process. by acquiring quantization tables 
 *
 * A new class added to save the jpeg info plus the file stream, it is 
 * first step to optimize the access to jpeg files
 */

#ifndef C_L_JPEG_INTERFACE_H
#define C_L_JPEG_INTERFACE_H

#include "common.h"
#include "jpeglib.h"
#include "c_l_dct_coefficient.h" //The reader extract the dct coefficients
#include <stdio.h>

/**
This class is designed to under take JPEG Routine Fles Access and to finish
	the preliminaries works 
*/
class C_L_JPEG_Store
{
public:
		j_decompress_ptr m_pJPEG_DecompressionInfo;
		j_compress_ptr 		m_pJPEG_CompressionInfo;
		FILE* m_pJPEG_Stream;
		struct jpeg_error_mgr m_jerr; /*I prefer to eat jam with standard
													error handling*/
	
	/** Constructor */
	C_L_JPEG_Store();
	/** Destructor */
	~C_L_JPEG_Store();

	/** Does  the Decompression Preliminarie  and then return a pointer to 
		ready to be used decompression strucutre
	*/
	bool DecompressionStarter(char* JPEG_Filename);
	
	/** Does  the compression Preliminarie  and then return a pointer to 
		ready to be used decompression strucutre*/
	bool CompressionStarter(char* JPEG_Filename);

};

/**
This class is dealing with the ijg library and provide the crude DCT coefficient for the other classes to manipluate it also it write them back to the jpeg file when they are changed. It should also have the duty to cheat the library to stop double quantiztion and ruining the embeded data.

*/

class C_L_JPEG_Interface{
protected:
	unsigned int m_OriginalQuality;
	unsigned int m_RecompressQuality;

	//JQUANT_TBL *quant_tbl_ptrs[NUM_QUANT_TBLS];

	/** Bring the whole array of DCT coefficient in to memroy using 
		memory management rutines of libjpeg Library
	*/
	C_L_DCT_CoefficientCollection* LoadDCT_Coefficients(j_decompress_ptr pJPEG_Info);
	bool SaveDCT_Coefficients(j_compress_ptr pJPEG_Info,  j_decompress_ptr pCloneInfo, C_L_DCT_CoefficientCollection* pDCT_CoefficietnCollection);

public:
	C_L_JPEG_Interface();

    ~C_L_JPEG_Interface();
	
	/** This function first import the JPEG Header Info like Quality factor 
		and Quantization Table then Read the quantized coefficient from 
		the original Image then returns the Coefficients in a 
		C_L_DCT_CoefficientCollection
	*/
	C_L_DCT_CoefficientCollection*  ReadJPEGImage(char* JPEG_Filename);

	/** Aquire the quantziation tables with corresponding quality, 
		
		If the quality is set to zero the existed quantization tables of the JPEG_Filename returns back.
		Otherwise the a JPEG_Filename is created and such quantiztion quality will be set then the
		tables will return backs
	*/
	JQUANT_TBL* AquireQuantizationTables(char* JPEG_Filename, unsigned int RecompressionQuality);
    void WriteJPEG_Image(char* CloneImageFilename, char* NewImageFilename, unsigned int RecompressionQuality, C_L_DCT_CoefficientCollection* DCT_CoefficietnCollection, JQUANT_TBL* pRecompressQuantizationTables);

};

#endif
