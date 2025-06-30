/* aLPLC2 Structure Capsulation */
#ifndef __LPLC2_PC_H
#define __LPLC2_PC_H


#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>


#define Width 99
#define Height 72
// #define Delay_Length 2 
#define DELAY_WINDOW 10
#define NUM_AF 2



// LPLC2 Neural Network Model with Attention Mechanism
typedef struct
{
	//uint8_t L1[Height][Width]; // Lamina Cells 1: Luminance increasment
	//uint8_t L2[Height][Width]; // Lamina Cells 2: Luminance decreasment

	uint8_t ON[DELAY_WINDOW][Height][Width]; // t and t-1(the delayed ) Signal in ON Channels
	uint8_t OFF[DELAY_WINDOW][Height][Width];


	//uint8_t T4[4][Height][Width];
	//uint8_t T5[4][Height][Width];
	// 2025.04.07 Renyuan Removed for Memory Space: 
	//int16_t T[Height][Width]; // Directional Selective Signal Centrifugally Integrated in 4 Quadrants


	uint8_t IS_AF[Height][Width] ; // whether this pixel is already in an existing AF. 1: currently occupied, 0: currently vacant


	// Test : 
	// Test EMD :													    _____________
	//int16_t EMD[4][Height]; // 1: Right; 2: Left; 3: Down; 4: Up. 	  " |	  |     | "
	// 'Height' length pixel in the central line of the visual field,   |  -------  |   shape.
	//																    |	  |     |
	//                                                                  `````````````

}LPLC2struct_Layers;

typedef struct
{
	// ON/OFF Channel :
	uint8_t W_ON; // Weight of ON Channel Contribution
	uint8_t W_OFF; // Weight of OFF Channel Contribution
	uint8_t r; // Radius of Mean-Value Filter
	uint8_t t; // Threshold after Mean-Value Filter

	// Elementary Movement Detector :
	uint8_t Step ; // Step Length in for loop, used for downsampling.
	uint8_t d ; // Distance between two adjacent input posion in EMD.
	uint8_t Nc ; // The number of connected neurons
	uint8_t Delay ; // Delay time in FRAME, n: delay for n frames.
	uint16_t Bias ; // Bias Coefficient for HRC
	uint8_t AF_Radius ; // Radius of Attention Field.
	uint8_t AF_Radius_Exclusive ; // Field That No AF is Allowed to Establish.
 	int32_t AF_Threshold ; // Threshold for Conserving an AF.

	// LPLC2 Projection : 
	int64_t T_cardinal; // Threshold for Each Quadrant's Output
	int64_t T_LPLC2; // Threshold for LPLC2 Projection

	float tau_m ; // Membrane time constant
	int8_t V_rest ; // Resting potential
	int8_t V_threshold ; // Threshold potential for spiking
	int8_t V_reset ; // Reset Potential
	int8_t V_spike ; // Spiking Potential
	float R ; // Membrane Resistance
	uint32_t max_input ;


	uint16_t n ; // Spike Number Shreshold to Detect Collision

}LPLC2struct_Params;


typedef struct
{
	uint8_t Vacant_AF ; // Whether this AF is active or vacant. If vacant one, all results below doesn't make sense.

	uint8_t Centroid[2]; // Cooradinate of Current AF Center. Centroid[0]: Ordinate; Centroid[1]: Abscissa.

	// int32_t Q1_R ;
	// int32_t Q1_D ;
	// int32_t Q2_L ;
	// int32_t Q2_D ;
	// int32_t Q3_L ;
	// int32_t Q3_U ;
	// int32_t Q4_R ;
	// int32_t Q4_U ;

	int64_t Quadrant[4] ; // Quadrant[0]: Upper Right; Quadrant[1]: Upper Left; Quadrant[2]: Bottom Left;
	int64_t Output[4] ; // Consider Recent 4 Frames' Output
	int64_t LPLC2 ; // Final Output of LPLC2
	double I ; // LPLC2 Arbor's Current to LIF Model
	float OUTPUT[30] ; // Input LPLC2, LIF Output
	uint16_t Spike; // Spike Number 
	uint8_t Collision; // 0/1


	// // EMD test:
	// // use up right quadrant 1 (bottom right in Colias' view ) to test EMD:
	// int16_t R_sum;
	// int16_t L_sum;
	// int16_t HS ; // R - L
	// 
	// int16_t D_sum;
	// int16_t U_sum;
	// int16_t VS ; // D - U
	// 
	// int16_t Q1_b_r ;
	// int16_t Q1_u_l ; // Q1_b_r reversed.

}LPLC2struct_Results;


typedef struct
{
	float Escape_Direction ; // Colias' Escape Direction by existing AFs.
	float Escape_Speed ; // Colias' Escape Speed by existing AFs.

}LPLC2struct_Visuomotor;


typedef struct
{
	LPLC2struct_Layers Layers ;
	LPLC2struct_Params Params ;

	LPLC2struct_Results AF[NUM_AF] ; // Used for multiple attention mechanism.
	// LPLC2struct_Results Results ; // Used for single attention mechanism.
	// int16_t* pImg ;
	// int8_t* pDiffImg ;

	LPLC2struct_Visuomotor Visuomotor ; // Visual Results Indicating Motor Input.

}LPLC2Type;


#ifdef __cplusplus
}
#endif


#endif
