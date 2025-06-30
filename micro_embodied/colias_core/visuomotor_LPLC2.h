#ifndef VISUOMOTOR_LPLC2_H
#define VISUOMOTOR_LPLC2_H


#include "main.h"
//#include "common_defs.h"
//#include "coliasSense_LGMD.h"
#include "ePID.h"
//#include "Color_finder.h"
#include "coliasSense_LPLC2.h"


typedef enum{
	STATE_STOP = 0, 
	STATE_WANDER = 0x01, 
	STATE_LONG_TAKEOFF = 0x02, // decide direction 
	STATE_SHORT_TAKEOFF = 0x03, // decide backward speed
	STATE_ESCAPE = 0x04

}LPLC2_StateType;

typedef struct
{
	LPLC2_pControlTypedef* hLPLC2 ; // aLPLC2 model here

	// Finite State Machine (FSM) :
	LPLC2_StateType STATE ; // 1: wander, 2: long_takeoff, 3: short_takeoff, 4: escape, 5: stop.
	LPLC2_StateType STATE_Previous ; // previous state, used for STATE_ESCAPE.

	// Count for FSM :
	uint16_t COUNT_CurrentState ; // current state duration
	uint16_t COUNT_Safe ; // non-collision duration
	uint16_t COUNT_DirectionAlign ; // direction aligned, TURN state completed.

	// Time Limit : 
	uint16_t T_Wander ; // maximum wander time
	float T_AlignAngle ; // 10 for default. if PID.e < T, COUNT_DirectionAlign ++ 
	uint16_t T_Aligned ; //10 for default. if COUNT_DirectionAlign > T, TURN state over.


}FSM_LPLC2_StructTypedef;


extern FSM_LPLC2_StructTypedef FSM_aLPLC2 ;


void VisionToMotor(LPLC2_pControlTypedef* hLPLC2);

void LED_Set(uint8_t ID, uint32_t color);
void FSM_LED_Set(void);

int Motion_Init(void);
int FSM_aLPLC2_Init(FSM_LPLC2_StructTypedef* FSM);
int FSM_aLPLC2_Collision(FSM_LPLC2_StructTypedef* FSM);
int SM_aLPLC2_AlignCheck(FSM_LPLC2_StructTypedef* FSM);
LPLC2_StateType FSM_aLPLC2_State_Run(FSM_LPLC2_StructTypedef* FSM );
LPLC2_StateType FSM_aLPLC2_motion(FSM_LPLC2_StructTypedef* FSM );


// Basic Motion Control : 
void Motion_vset(u8 lr, double value); 
void Motion_LR(double left, double right); // level 1
void Motion_VB(double speed, double bias); // level 2

#endif /* VISUOMOTOR_LPLC2_H */
