#ifndef __CoSLPLC2_H
#define __CoSLPLC2_H


#ifdef __cplusplus
extern "C" {
#endif


	//#include "stm32f4xx_hal.h"
	//#include "coliasSense_def.h"
#include "LPLC2_model.h"
	//#include "coliasSense_BIO.h"
	//#include "coliasSense_Camera.h"
	//#include "coliasSense_LGMD.h"
#include <stdint.h>



// aLPLC2 Neural Network Model : 
typedef struct{
	uint32_t processCount;
	uint32_t processCountLast;
	uint32_t* hFrameCount;
	LPLC2Type* Model;
	uint8_t Enable;
	uint8_t status;
	uint8_t processRate;
	uint8_t currentImage;
	uint8_t currentDiffImage;
	uint8_t cur ; // current frame in the array, 0 ~ 9.
	//uint8_t AGC_enable_period;

	
	uint8_t Attention_Mechanism ; 
	char LPLC2_cross_shape ; // '+' shape or 'X' shape.
	uint8_t EMD_test_mode ; // 1: test EMD, Colias only see the bottom right quadrant of its visual field.


	// Run time : 
	uint32_t RunTime_aLPLC2_PreSynaptic ;
	uint32_t RunTime_aLPLC2_T4_T5 ;
	uint32_t RunTime_aLPLC2_Projection ;
	uint32_t Interval_aLPLC2 ; // Interval between two aLPLC2 process.

}LPLC2_pControlTypedef;


// Founctions for LPLC2 Calculating : 
void LPLC2_Init(LPLC2_pControlTypedef* hLPLC2);
void LPLC2_PreSynaptic(LPLC2_pControlTypedef* hLPLC2);
void LPLC2_T4_T5(LPLC2_pControlTypedef* hLPLC2);
void LPLC2_Projection(LPLC2_pControlTypedef* hLPLC2);
void VisionToMotor(LPLC2_pControlTypedef* hLPLC2 );

extern uint16_t Image[3][Height][Width];
extern int8_t Diff_Image[2][Height][Width];

extern LPLC2Type LPLC2;
extern LPLC2_pControlTypedef hLPLC2;


#ifdef __cplusplus
}
#endif

#endif