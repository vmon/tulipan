/***************************************************************************
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 ***************************************************************************/
/*******LOGs

 * Tuesday, 13 September 2005:
 * Successfully I could open a JPEG file, now two steps and 
 * I'll concentrate on wet code first I should be able to read
 * JPEG Coefficients, then I should be able to change the 
 * Quantization Table. If I could do, 

#include "c_l_jpeg_interface.h"
#include <stdio.h>

/*
 * Includes file for users of JPEG library.
 * You will need to have included system headers that define at least
 * the typedefs FILE and size_t before you can include jpeglib.h.
 * (stdio.h is sufficient on ANSI-conforming systems.)
 * You may also wish to include "jerror.h".
 */

#include "c_l_jpeg_interface.h"
#include <stdio.h> //for the sake of size_t
#include <iostream>
#include "jpeg-6b/jmemsys.h" //it was creasy decision
#include "jerror.h"
#include "jpegint.h"

#undef MYDEBUG

/*For the reason of simplicty in this version we just
						keep information in the first color component*/

using namespace std;

/*just for debug purpose
	struct jvirt_barray_control {
 //JBLOCKARRAY mem_buffer;	/* => the in-memory buffer */
 //JDIMENSION rows_in_array;	/* total virtual array height */
  //JDIMENSION blocksperrow;	/* width of array (and of memory buffer) */
  //JDIMENSION maxaccess;		/* max rows accessed by access_virt_barray */
  //JDIMENSION rows_in_mem;	/* height of memory buffer */
  //JDIMENSION rowsperchunk;	/* allocation chunk size in mem_buffer */
  //JDIMENSION cur_start_row;	/* first logical row # in the buffer */
  //JDIMENSION first_undef_row;	/* row # of first uninitialized row */
  //boolean pre_zero;		/* pre-zero mode requested? */
 //boolean dirty;		/* do current buffer contents need written? */
  //boolean b_s_open;		/* is backing-store data valid? */
  //jvirt_barray_ptr next;	/* link to next virtual barray control block */
  //backing_store_info b_s_info;	/* System-dependent control info */
//};

// C_L_JPEG_Store Implementation 

	/** Constructor */
C_L_JPEG_Store::C_L_JPEG_Store()
: m_pJPEG_Stream(NULL), 
m_pJPEG_DecompressionInfo(NULL),
m_pJPEG_CompressionInfo(NULL)
{
}

/** Destructor */
C_L_JPEG_Store::~C_L_JPEG_Store()
{
  /* Step 7: Finish decompression */

  	if (m_pJPEG_DecompressionInfo)  {
		/*If the user is aborting before a complete jpeg operation
			We should not call Finish */
		if (m_pJPEG_DecompressionInfo->global_state == DSTATE_STOPPING) 
			(void) jpeg_finish_decompress(m_pJPEG_DecompressionInfo);
  		/* We can ignore the return value since suspension is not possible
   		* with the stdio data source.
   		*/

 		 /* Step 8: Release JPEG decompression object */
  		/* This is an important step since it will release a good deal of memory. */
  		jpeg_destroy_decompress(m_pJPEG_DecompressionInfo);
  		/* After finish_decompress, we can close the input file.
   		 * Here we postpone it until after no more JPEG errors are possible,
   		 * so as to simplify the setjmp error logic above.  (Actually, I don't
   		* think that jpeg_destroy can do an error exit, but why assume anything...)
   		*/
		
		fclose(m_pJPEG_Stream);
	}
	
	if (m_pJPEG_CompressionInfo) {
		/*If the user is aborting before a complete jpeg operation
			We should not call Finish */
			if ((m_pJPEG_CompressionInfo->global_state == DSTATE_STOPPING) ||
					(m_pJPEG_CompressionInfo->global_state == CSTATE_WRCOEFS) )
					 jpeg_finish_compress(m_pJPEG_CompressionInfo);
  			/* After finish_compress, we can close the output file. */
  		fclose(m_pJPEG_Stream);

  		/* release JPEG compression object */

  		/* This is an important step since it will release a good deal of memory. */
 		 jpeg_destroy_compress(m_pJPEG_CompressionInfo);
	}

}

/** Doess  the Decompression Preliminarie  and then return a pointer to 
	ready to be used decompression strucutre
*/
bool C_L_JPEG_Store::DecompressionStarter(char* JPEG_Filename)
{
	
  /* This struct contains the JPEG decompression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   */
	m_pJPEG_DecompressionInfo = new jpeg_decompress_struct; 
  /* We use our private extension JPEG error handler.
   * Note that this struct must live as long as the main JPEG parameter
   * struct, to avoid dangling-pointer problems.
   */
  //struct my_error_mgr jerr;
  /* More stuff */

  if ((m_pJPEG_Stream = fopen(JPEG_Filename, "rb")) == NULL) {
		throw("Error in Opening File");
    	return false; //Failure
  }
  /* Step 1: allocate and initialize JPEG decompression object */

	m_pJPEG_DecompressionInfo->err = jpeg_std_error(&m_jerr);

  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(m_pJPEG_DecompressionInfo );

  /* Step 2: specify data source (eg, a file) */
  jpeg_stdio_src(m_pJPEG_DecompressionInfo , m_pJPEG_Stream);

  /* Step 3: read file parameters with jpeg_read_header() */
	(void) jpeg_read_header(m_pJPEG_DecompressionInfo , TRUE);
  /* We can ignore the return value from jpeg_read_header since
   *   (a) suspension is not possible with the stdio data source, and
   *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
   * See libjpeg.doc for more info.
   */
	return true;
}

/** Does  the compression Preliminarie  and then return a pointer to 
	ready to be used decompression strucutre*/
bool C_L_JPEG_Store::CompressionStarter(char* JPEG_Filename)
{
  /* This struct contains the JPEG compression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   * It is possible to have several such structures, representing multiple
   * compression/decompression processes, in existence at once.  We refer
   * to any one struct (and its associated working data) as a "JPEG object".
   */
	m_pJPEG_CompressionInfo = new jpeg_compress_struct;
  /* More stuff */

  /* Step 1: allocate and initialize JPEG compression object */

  /* We have to set up the error handler first, in case the initialization
   * step fails.  (Unlikely, but it could happen if you are out of memory.)
   * This routine fills in the contents of struct jerr, and returns jerr's
   * address which we place into the link field in cinfo.
   */
  m_pJPEG_CompressionInfo->err = jpeg_std_error(&m_jerr);
  /* Now we can initialize the JPEG compression object. */
  jpeg_create_compress(m_pJPEG_CompressionInfo);

  /* Step 2: specify data destination (eg, a file) */
  /* Note: steps 2 and 3 can be done in either order. */

  /* Here we use the library-supplied code to send compressed data to a
   * stdio stream.  You can also write your own code to do something else.
   * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
   * requires it in order to write binary files.
   */
  if ((m_pJPEG_Stream = fopen(JPEG_Filename, "wb")) == NULL) {
    throw( "can't open %s\n");
    return false;
  }
  jpeg_stdio_dest(m_pJPEG_CompressionInfo, m_pJPEG_Stream);

}

//C_L_JPEG_Interface Implementation
C_L_JPEG_Interface::C_L_JPEG_Interface()
{
}

C_L_JPEG_Interface::~C_L_JPEG_Interface()
{
}

/*
 * Sample routine for JPEG decompression.  We assume that the source file name
 * is passed in.  We want to return 1 on success, 0 on error.
 */

C_L_DCT_CoefficientCollection*  C_L_JPEG_Interface::ReadJPEGImage(char* JPEG_Filename)
{

  	//The preliminaries is encapsulated in DecompressionStarter
	//later I move all in another class C_L_JPEG_Stor
	C_L_JPEG_Store Decompression_Store;
	Decompression_Store.DecompressionStarter(JPEG_Filename);
	/* Step 4: set parameters for decompression */
	/*jpeg_copy_critical_parameters(&JPEG_Info, &DoubleCompressedJPEG_Info);(/
  /* In this example, we don't need to change any of the defaults set by
   * jpeg_read_header(), so we do nothing here.
   */

	/* In this step I'm going to change the quantization tables*/

	
/*		for(int j = 0; j  < 2; j++)
			for(int i = 0; i < DCTSIZE2; i++) {
				cout << JPEG_Info.quant_tbl_ptrs[j]->quantval[i] << "|";

				DoubleCompressedJPEG_Info.quant_tbl_ptrs[j]->quantval[i] /= 25;
				cout << DoubleCompressedJPEG_Info.quant_tbl_ptrs[j]->quantval[i] << ", ";
		}*/
		
  /* Step 5: Start decompressor */
	
	/* Because we just read the DCT Coefficients directly we don't need
		to go through decompression preprocess and we get directly the
		quantized coefficients. The real value of the coefficient can
		be obtained only after DeQunatization step
	*/

	
	/* We have much memory than 1998 so we can read the whole coefficient array
		in the memory and then process them faster and easier
	*/
	return LoadDCT_Coefficients(Decompression_Store.m_pJPEG_DecompressionInfo);

	//The JPEG_Store automatically destroy everything
	
//jpeg_write_coefficients(&DoubleCompressedJPEG_Info, DCT_CoefficientBuffer);
  
  /* At this point you may want to check to see whether any corrupt-data
   * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
   */

  /* And we're done! */
}

	/** Bring the whole array of DCT coefficient in to memroy using 
		memory management rutines of libjpeg Library
	*/
C_L_DCT_CoefficientCollection* C_L_JPEG_Interface::LoadDCT_Coefficients(j_decompress_ptr pJPEG_Info)
{
	
	/* First we read the Quantized coefficient out of the image */
	jvirt_barray_ptr* pDCT3D_CoefficientBuffer = jpeg_read_coefficients(pJPEG_Info);
	//jvirt_barray_control TestStruct = *pDCT3D_CoefficientBuffer[SELECTED_COMPONENT];

	/*The jpeg has a restrict control on its memory object which is undesirable
		for us, so first, I transfer all of information once in my memory space
	*/
 	/* For the simplicity in this version we just deal with the first 
		layer which luminace informaiton 
	*/
	/* The JPEG Library needs to define a strucuter to 
		access a virtual array, as much as I understand.
	*/
		
	//First we obtain the block dimansion instead of pixel dimansion
	JDIMENSION NoColumns = pJPEG_Info->comp_info[SELECTED_COMPONENT].width_in_blocks;
	JDIMENSION NoRows = pJPEG_Info->comp_info[SELECTED_COMPONENT].height_in_blocks;

	/*I wanted to read whole of object in one access but the memory manager does
		not allow me to read more than few line by a call (although the 
		virtual coefficient array isn't virtual any more and is completely 
		loaded into memory, so I out to read it by a call for each line.

		Also becasue the structure of the array is not two dimensiona,instead
		it is a an array of one dimensional arrays I need a loop to embed it in
		to our C_L_DCT_CoefficientCollection, there for I combine these 
		two loop operation in following one
	*/
	/* We can keep the coefficients in the order the libjpeg offer but because
		our PRNG does not care for this order, if we preserved the order we 
		would have an overhead computation of placement for each access to 
		the buffer. for this reason I damage the structure and uniformize the 
		coefficient then we wanted to write them to stego object we arrange 
		them again.
		
		Rearrangement Process:
	*/

 	C_L_DCT_CoefficientCollection* pCurrentImageCoefficients = new C_L_DCT_CoefficientCollection;
	pCurrentImageCoefficients->m_NoOfMembers = NoRows * NoColumns * DCTSIZE2;

	pCurrentImageCoefficients->m_pDCT_Coefficient = new T_L_WetDCT_Coefficient[pCurrentImageCoefficients->m_NoOfMembers];
	T_L_WetDCT_Coefficient* pDCTCollectionIndicator = pCurrentImageCoefficients->m_pDCT_Coefficient;

	for(JDIMENSION RowCounter = 0; RowCounter < NoRows; RowCounter++)
	{
		JBLOCKARRAY pCoefficientBlocks =  (*pJPEG_Info->mem->access_virt_barray)((j_common_ptr)pJPEG_Info,
					pDCT3D_CoefficientBuffer[SELECTED_COMPONENT],
					    RowCounter, //Starting Line
						  1, //No of lines to be read it could be more but 1 is always safe
					    false); //Do we want to write on them? NO

		for(JDIMENSION ColumnCounter = 0; ColumnCounter < NoColumns; ColumnCounter++)
		{
			/*This operation is just allow when T_L_WetCoefficient and JCOEF have the same
				size*/
			/*Because in each call we take just a row of the coefficient array
				the first index is always 0*/
			memcpy(pDCTCollectionIndicator, (pCoefficientBlocks [0])[ColumnCounter], sizeof(JCOEF) * DCTSIZE2);
			pDCTCollectionIndicator  += DCTSIZE2;

		}

	}

#ifdef MYDEBUG
// for(int i = 0; i < DCTSIZE2; i++)
//			cout << pDCTCollectionIndicator[i - DCTSIZE2] << ", ";
//		cout << endl;
	
#endif 

	/*JBLOCKARRAY pCoefficientBlocks = access_virt_barray(pJPEG_Info,
					    pDCT3D_CoefficientBuffer[0],
					    0,
					    pDCT3D_CoefficientBuffer[0].rows_in_array,
					    false));*/

//Just to be informed of the structure
//struct jvirt_barray_control {
  //JBLOCKARRAY mem_buffer;	/* => the in-memory buffer */
  //JDIMENSION rows_in_array;	/* total virtual array height */
  //JDIMENSION blocksperrow;	/* width of array (and of memory buffer) */
  //JDIMENSION maxaccess;		/* max rows accessed by access_virt_barray */
  //JDIMENSION rows_in_mem;	/* height of memory buffer */
  //JDIMENSION rowsperchunk;	/* allocation chunk size in mem_buffer */
  //JDIMENSION cur_start_row;	/* first logical row # in the buffer */
  //JDIMENSION first_undef_row;	/* row # of first uninitialized row */
  //boolean pre_zero;		/* pre-zero mode requested? */
  //boolean dirty;		/* do current buffer contents need written? */
  //boolean b_s_open;		/* is backing-store data valid? */
  //jvirt_barray_ptr next;	/* link to next virtual barray control block */
  //backing_store_info b_s_info;	/* System-dependent control info */
//};

//typedef JCOEF JBLOCK[DCTSIZE2];	/* one block of coefficients */
//typedef JBLOCK FAR *JBLOCKROW;	/* pointer to one row of coefficient blocks */
//typedef JBLOCKROW *JBLOCKARRAY;		/* a 2-D array of coefficient blocks */
//typedef JBLOCKARRAY *JBLOCKIMAGE;	/* a 3-D array of coefficient blocks */

			
		return pCurrentImageCoefficients;	
			
}

JQUANT_TBL* C_L_JPEG_Interface::AquireQuantizationTables(char* JPEG_Filename, unsigned int RecompressionQuality)
{
	
	/*If the quaility factor is 0, it menas that an existing quantization 
		table is demanded.*/
	C_L_JPEG_Store ExistingJPEG_Store; //Just for aquiring Quantization tables
	/*in both cases decompression is needed
	   So we need to undergo a starting phase of a decompression process*/
	ExistingJPEG_Store.DecompressionStarter(JPEG_Filename);
	if (!RecompressionQuality)
	{
		JQUANT_TBL*  pQuantizationTables = new JQUANT_TBL[NUM_QUANT_TBLS];
		for(int i = 0; i < NUM_QUANT_TBLS; i++)
			if (!ExistingJPEG_Store.m_pJPEG_DecompressionInfo->quant_tbl_ptrs[i])
				 //The quantization table is not available
				memset(&pQuantizationTables[i], 0, sizeof(JQUANT_TBL));
			else
				memcpy(&pQuantizationTables[i], ExistingJPEG_Store.m_pJPEG_DecompressionInfo->quant_tbl_ptrs[i], sizeof(JQUANT_TBL));
#ifdef MYDEBUG
	cout << pQuantizationTables[0].quantval[0];
#endif 

		return pQuantizationTables;

	}
	else
	{
			/* This case is the harder job of this function, to generate a 
				a quantization table with desirable quality*/
			
			/* First open a file for compression, the new image should have
				exactly the same properties as the existing one but with 
				different quality, so we should open new file for compression
				and immitate the properties of the first one
			*/

			C_L_JPEG_Store TempJPEG_Store;
			
			char* TempFilename = "/tmp/tmpImage.jpg";	
			TempJPEG_Store.CompressionStarter(TempFilename);
			
			/* We should choose Temp name for our file */
 	
  			/* Now use the library's routine to set default compression parameters.
   			* (You must set at least cinfo.in_color_space before calling this,
   			* since the defaults depend on the source color space.)
   			*/
  			//jpeg_set_defaults(&cinfo);
			//instead
			jpeg_copy_critical_parameters(ExistingJPEG_Store.m_pJPEG_DecompressionInfo, TempJPEG_Store.m_pJPEG_CompressionInfo);
  
			/* Now you can set any non-default parameters you wish to.
   			* Here we just illustrate the use of quality (quantization table) scaling:
   			*/
  			jpeg_set_quality(TempJPEG_Store.m_pJPEG_CompressionInfo, RecompressionQuality, TRUE /* limit to baseline-JPEG values */);

			JQUANT_TBL*  pQuantizationTables = new JQUANT_TBL[NUM_QUANT_TBLS];
			for(int i = 0; i < NUM_QUANT_TBLS; i++)
				if (!TempJPEG_Store.m_pJPEG_CompressionInfo->quant_tbl_ptrs[i])
					 //The quantization table is not available
					memset(&pQuantizationTables[i], 0, sizeof(JQUANT_TBL));
				else
					memcpy(&pQuantizationTables[i], TempJPEG_Store.m_pJPEG_CompressionInfo->quant_tbl_ptrs[i], sizeof(JQUANT_TBL));

#ifdef MYDEBUG
	cout << pQuantizationTables[0].quantval[0];
#endif 

		return pQuantizationTables;

	}		

}


/*!
    \fn C_L_JPEG_Interface::WriteJPEG_Image(char* ImageFilename, JQUANT_TBL* QuantizationTables, C_L_WetCoefficientCollection* DCT_CoefficietnCollection)
 */
void C_L_JPEG_Interface::WriteJPEG_Image(char* CloneImageFilename, char* NewImageFilename, unsigned int RecompressionQuality, C_L_DCT_CoefficientCollection* pDCT_CoefficietnCollection, JQUANT_TBL* pRecompressQuantizationTables)
 {
		/*If the quaility factor is 0, it menas that an existing quantization 
		table is demanded.*/
		C_L_JPEG_Store ExistingJPEG_Store; //Just for aquiring Quantization tables
		/*in both cases decompression is needed
	   So we need to undergo a starting phase of a decompression process*/
		ExistingJPEG_Store.DecompressionStarter(CloneImageFilename);
			
		/* First open a file for compression, the new image should have
			exactly the same properties as the existing one but with 
			different quality, so we should open new file for compression
			and immitate the properties of the first one
		*/

		//It should be a pointer to give me the ability to delete it manualy
		C_L_JPEG_Store* pNewJPEG_Store = new C_L_JPEG_Store;
			
		pNewJPEG_Store->CompressionStarter(NewImageFilename);
			
		/* We should choose Temp name for our file */
 	
  		/* Now use the library's routine to set default compression parameters.
   		 * (You must set at least cinfo.in_color_space before calling this,
   		* since the defaults depend on the source color space.)
   			*/
  			//jpeg_set_defaults(&cinfo);
			//instead
		jpeg_copy_critical_parameters(ExistingJPEG_Store.m_pJPEG_DecompressionInfo, pNewJPEG_Store->m_pJPEG_CompressionInfo);

		//JDIMENSION NoColumns = NewJPEG_Store.m_pJPEG_CompressionInfo->comp_info[SELECTED_COMPONENT].width_in_blocks;

		/* Now you can set any non-default parameters you wish to.
   		 * Here we just illustrate the use of quality (quantization table) scaling:
   		 */
 		//jpeg_set_quality(pNewJPEG_Store->m_pJPEG_CompressionInfo, RecompressionQuality, TRUE /* limit to baseline-JPEG values */)
		memcpy(pNewJPEG_Store->m_pJPEG_CompressionInfo->quant_tbl_ptrs[SELECTED_COMPONENT], &pRecompressQuantizationTables[SELECTED_COMPONENT],  sizeof(JQUANT_TBL));
	
		if (!SaveDCT_Coefficients(pNewJPEG_Store->m_pJPEG_CompressionInfo, ExistingJPEG_Store.m_pJPEG_DecompressionInfo, pDCT_CoefficietnCollection))
			throw("Error!");

	delete pNewJPEG_Store;

}

/*!
	\fn bool C_L_JPEG_Interface::SaveDCT_Coefficients(j_compress_ptr pJPEG_Info, jvirt_barray_ptr* pCloneDCT3D_CoefficientBuffer, C_L_WetCoefficientCollection* DCT_CoefficietnCollection)

	uses the array of coefficients of the cover media and changes it according to the new DCT collection then write it to the new file.
*/
bool C_L_JPEG_Interface::SaveDCT_Coefficients(j_compress_ptr pJPEG_Info,  j_decompress_ptr pCloneInfo, C_L_DCT_CoefficientCollection* pDCT_CoefficietnCollection)
{
		jvirt_barray_ptr* pCloneDCT3D_CoefficientBuffer = jpeg_read_coefficients(pCloneInfo);

	/*The jpeg has a restrict control on its memory object which is undesirable
		for us, so first, I transfer all of information once in my memory space
	*/
 	/* For the simplicity in this version we just deal with the first 
		layer which luminace informaiton 
	*/

	/* The JPEG Library needs to define a strucuter to 
		access a virtual array, as much as I understand.
	*/
		
	//First we obtain the block dimansion instead of pixel dimansion
	JDIMENSION NoColumns = pCloneInfo->comp_info[SELECTED_COMPONENT].width_in_blocks;
	JDIMENSION NoRows = pCloneInfo->comp_info[SELECTED_COMPONENT].height_in_blocks;

	/*pCurrentImageCoefficients->m_pDCT_Coefficient = new T_L_WetDCT_Coefficient[pCurrentImageCoefficients->m_Size];*/
	T_L_WetDCT_Coefficient* pDCTCollectionIndicator = pDCT_CoefficietnCollection->m_pDCT_Coefficient;

	/*First I read the coefficient from the clone files  then I change the coefficients*/
	for(JDIMENSION RowCounter = 0; RowCounter < NoRows; RowCounter++)
	{
		JBLOCKARRAY pCoefficientBlocks =  (*pCloneInfo->mem->access_virt_barray)((j_common_ptr)pCloneInfo,
					pCloneDCT3D_CoefficientBuffer[SELECTED_COMPONENT],
					RowCounter, //Starting Line
					1, //No of lines to be read it could be more but 1 is always safe
					true); //We want to change the coefficients

		for(JDIMENSION ColumnCounter = 0; ColumnCounter < NoColumns; ColumnCounter++)
		{
			/* In this step we changes the coefficients according to the coefficient collection
				Pay attention that in the first version we just put info in luminace component.*/
			
			memcpy((pCoefficientBlocks[0])[ColumnCounter], pDCTCollectionIndicator, sizeof(JCOEF) * DCTSIZE2);
			pDCTCollectionIndicator  += DCTSIZE2;

		}

	}
	/*JBLOCKARRAY pCoefficientBlocks = access_virt_barray(pJPEG_Info,
					    pDCT3D_CoefficientBuffer[0],
					    0,
					    pDCT3D_CoefficientBuffer[0].rows_in_array,
					    false));*/

//Just to be informed of the structure
//struct jvirt_barray_control {
  //JBLOCKARRAY mem_buffer;	/* => the in-memory buffer */
  //JDIMENSION rows_in_array;	/* total virtual array height */
  //JDIMENSION blocksperrow;	/* width of array (and of memory buffer) */
  //JDIMENSION maxaccess;		/* max rows accessed by access_virt_barray */
  //JDIMENSION rows_in_mem;	/* height of memory buffer */
  //JDIMENSION rowsperchunk;	/* allocation chunk size in mem_buffer */
  //JDIMENSION cur_start_row;	/* first logical row # in the buffer */
  //JDIMENSION first_undef_row;	/* row # of first uninitialized row */
  //boolean pre_zero;		/* pre-zero mode requested? */
  //boolean dirty;		/* do current buffer contents need written? */
  //boolean b_s_open;		/* is backing-store data valid? */
  //jvirt_barray_ptr next;	/* link to next virtual barray control block */
  //backing_store_info b_s_info;	/* System-dependent control info */
//};

//typedef JCOEF JBLOCK[DCTSIZE2];	/* one block of coefficients */
//typedef JBLOCK FAR *JBLOCKROW;	/* pointer to one row of coefficient blocks */
//typedef JBLOCKROW *JBLOCKARRAY;		/* a 2-D array of coefficient blocks */
//typedef JBLOCKARRAY *JBLOCKIMAGE;	/* a 3-D array of coefficient blocks */
			

	jpeg_write_coefficients(pJPEG_Info, pCloneDCT3D_CoefficientBuffer);

	/*We have to force that the stego store closes sooner than 
		the Cover Store because the actual write process will be done
	 	when finish is called*/
	//delete pJPEG_Info;

	return true;	

}
