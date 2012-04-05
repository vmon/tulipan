/***************************************************************************
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 ***************************************************************************/
#ifndef C_L_SELECTORRULER_H
#define C_L_SELECTORRULER_H

#include "common.h"
#include "c_l_dct_coefficient.h"

/**
This class get the Both version of dct coefficient array and decide which coefficient meets the criteria to be a DRY Coefficient.

*/
class C_L_SelectorRuler
{
protected:	
	
	C_L_DCT_CoefficientCollection m_FullInfoCoefficients; /* Represent
	the value of the DCT coefficitnet before the image bears the information
	reduction process*/
	C_L_DCT_CoefficientCollection m_InformationReducedCoefficients;
	 /* Represent the value of the DCT coefficitnet after the image bears the 
	informationreduction process*/

	double m_SelectionThreshold;

public:
    C_L_SelectorRuler();

    ~C_L_SelectorRuler();
	/** Giving the DCT info to the class */
	void SetFullInfoDCTData(C_L_DCT_CoefficientCollection* pFullInfoCoefficients);
	void SetInformaationReducedCoefficients(C_L_DCT_CoefficientCollection* pInformationReducedCoefficients);

	/** This function get a specified collection of cefficient it compute the
		humidity (the possibility of changing) of them and return back
		an array informing their status */
	T_L_Humidity* SelectionRule(T_L_CoefficientIndex* pProposedCoefficients);

};

#endif
