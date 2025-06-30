#include "visuomotor_LPLC2.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
//#include <unistd.h>
#include <math.h>
//#include "sys.h"
#include "coliasapi.h"
//#include "prey.h"

// Originally in prey.c, now moved to here :
#define MOTOR_CC_ADDR 0x99
#define MOTOR_POWER_L 0x9A
#define MOTOR_POWER_R 0x9C
#define TCRT_L_ADDR 0x20
#define COLORSENSOR_F_ADDR 0x30
#define COLORSENSOR_L_ADDR 0x38
#define COLORSENSOR_R_ADDR 0x40
#define Pi 3.1415926f

FSM_LPLC2_StructTypedef FSM_aLPLC2 ; // FSM 



void VisionToMotor(LPLC2_pControlTypedef* hLPLC2 )
{
    // Pointer Passing :
    LPLC2struct_Params* Params = &(hLPLC2->Model->Params ) ;
    LPLC2struct_Layers* Layers = &(hLPLC2->Model->Layers ) ;
    LPLC2struct_Results* AF = &(hLPLC2->Model->AF[0] ) ; // For Multiple-Attention Machanism
    LPLC2struct_Visuomotor* Visuomotor = &(hLPLC2->Model->Visuomotor ) ;
    
    // Reset : 
	Visuomotor->Threat_Strength = 0 ; // reset Strength to update LED
    //Visuomotor->Threat_Direction = 0 ; // do not reset Direction, keep it to select direction of ESCAPE state 
 
    Visuomotor->Escape_Direction = 0 ;
    //Visuomotor->Escape_Speed = 0 ;

    uint64_t numerator_x = 0 ;
    uint64_t denominator_x = 0 ;

    uint64_t GlobalStrength = 0 ; // Escape Speed by Global AFs' Strength

    // Search Colliding AFs : 
    for(int n = 0; n < NUM_AF; n ++ )
    {
        if(AF[n].Vacant_AF == 1 )
            continue ;

		// Visuomotor Updates Only When Collision Happens !!!
        if(AF[n].Collision == 1 )
        {
            // If any AF is in collision, integrate global information from all extsting AFs :
            for(int n = 0; n < NUM_AF; n ++ )
            {
                if(AF[n].Vacant_AF == 1 )
                    continue ;

                // Integration for Mass Centroid Calculation along X-axis : 
                numerator_x   += AF[n].LPLC2 * AF[n].Centroid[1] ; 
                denominator_x += AF[n].LPLC2 ; 

                // Integration for Escape Speed Calculation : 
                GlobalStrength += AF[n].LPLC2 ; 
                
            }
            

            // Calculate Mass Centroid :
            float Centroid_x = (float)numerator_x / ((float)denominator_x + 1) ; // Centroid_x = 0 ~ 98
            //printf("\n\nVISUOMOTOR OUTPUT : ");
            //printf("\n%f Mass Centroid_x of AFs ", Centroid_x );

            // Threat Direction : 
            Visuomotor->Threat_Direction = (Centroid_x - Image_Width/2) * 70 / 99 ; // Centroid_x - 49, 0 ~ 98 - 49 = -49 ~ 49 (degree )
            //printf("\n%f Threat Direction", Visuomotor->Escape_Direction );


            // Escape Direction : 
            if(Visuomotor->Threat_Direction >= 0 ){
                Visuomotor->Escape_Direction = Visuomotor->Threat_Direction - 90 ; // Threat from right side, turn left.
            }else{
                Visuomotor->Escape_Direction = Visuomotor->Threat_Direction + 90 ; // Threat from left side, turn right.
            }
            //printf("\n%f Escape Direction ", Visuomotor->Escape_Direction );


            // Escape Speed :
			Visuomotor->Threat_Strength = GlobalStrength ;
            Visuomotor->Escape_Speed = 1.0 / (1.0 + exp( - 1.0*((float )GlobalStrength / 20000*NUM_AF) ) ) * MC1.Vmax ; // MC1.Vmax = 1 * 3.3 = 3.3
            //printf("\n%llu GlobalStrength input as speed", GlobalStrength );
            //printf("\n%f Escape Speed ", Visuomotor->Escape_Speed );


            return ; // Find any collision, return after integrating global information.
        }
    }

LPLC2.TIME[7] = TOCin5 ;
} // Visuomotor Over ... 




// aLPLC2 Finite State Machine (FSM ) : 
/************************************************************************************************************/
//Source file for LGMD and Colour Finder Finite State Machine

// Parameter Initialization : 
// FSM : 
int FSM_aLPLC2_Init(FSM_LPLC2_StructTypedef* FSM )
{
	FSM->hLPLC2 = &hLPLC2 ;

	FSM->STATE = STATE_WANDER ;
	FSM->STATE_Previous = STATE_LONG_TAKEOFF ; 
	FSM->COUNT_CurrentState = 0 ;
	FSM->COUNT_Safe = 0 ;
	FSM->COUNT_DirectionAlign = 0 ;

	FSM->T_Wander = 1000*AC1.tzoom ; // AC1.tzoom = 1
	FSM->T_AlignAngle = 20 ; // 10 for default. if PID.e < T, COUNT_DirectionAlign ++
	FSM->T_Aligned = 5 ; //10 for default


	return 0 ;
}

// Motion :
int Motion_Init(void)
{
	// AC: Global Arguments :
	AC1.d=0;
	AC1.p=55;
	AC1.P=0;
	AC1.m = 4.5 ; // 3.3 ; // motion gain
	AC1.l=0.5; // wander motion speed gain
	AC1.tzoom=1;
	AC1.en_motion = 0 ; // Enable Motion
	

	MC1.Vmax = MAX_SPEED * AC1.m ; // = 1.0 * 3.3 = 3.3
	MC1.Ammax = -1;
	MC1.Amax = 2 ; // 1 ;
	MC1.left = 0;
	MC1.right = 0;

	// Using frequently in motion control : 
	MC1.dir = 0 ; // COlias' current direction
	MC1.vrun = 0 ; // COlias' speed input
	MC1.vrun_default = 5 ; 
	
	// PID parameters :
	PID1.kp = 0.13;
	PID1.ki = 0.01;
	PID1.kd = 0.03;
	PID1.sum_max = 0.2;
	PID1.sum_min = -0.2;
	PID1.uc_max = 5;
	PID1.uc_min = -5;
	PID1.roundcut = 360;
	PID1.roundthreshold = 180;
	
		
	// 2025.1.11, Renyuan added (control FSM state time using time instead of direction ):
	MC1.Turn_tmax = 35 ; // Turn Time.
	MC1.Turn_left = 0 ;
	MC1.Wait_tmax = MC1.Turn_tmax/2 ; // Wait Time.
	MC1.Wait_left = 0 ;


	return 0 ;
}


// Collision Detection : 
int FSM_aLPLC2_Collision(FSM_LPLC2_StructTypedef* FSM)
{
	// if(FSM->hLPLC2->Model->AF[0].Collision == 1 )
	if(FSM->hLPLC2->Model->Visuomotor.Threat_Strength != 0 )
	{
		FSM->COUNT_Safe = 0 ;
		return 1 ;
	}
	// FSM->hLPLC2->Model->Results.Collision == 0 : 
	FSM->COUNT_Safe ++ ;

	return 0 ;
}

int FSM_aLPLC2_AlignCheck(FSM_LPLC2_StructTypedef* FSM)
{
	if (fabs(PID1.e) < FSM->T_AlignAngle ) // direction error less than 5 degree
		FSM->COUNT_DirectionAlign ++ ; // accumulate 1 at a time
	else
		FSM->COUNT_DirectionAlign = 0 ; 


	if (FSM->COUNT_DirectionAlign > FSM->T_Aligned ) // hObj->T_Aligned = 10 ; 
		return 1 ;
	else
		return 0 ;
}


// FSM State Judge :
LPLC2_StateType FSM_aLPLC2_State_Run(FSM_LPLC2_StructTypedef* FSM )
{	
	LPLC2struct_Visuomotor* Visuomotor = &(FSM->hLPLC2->Model->Visuomotor ) ; // Visuomotor results
	LPLC2struct_Params* Params = &(FSM->hLPLC2->Model->Params ) ; // Parameters

	FSM->COUNT_CurrentState ++ ; // count the time in current state

	switch (FSM->STATE )
	{
		case STATE_WANDER:
		{
			// If collision, excute long-mode escape by default : 
			if(FSM_aLPLC2_Collision(FSM) == 1 )
			{
				FSM->STATE = STATE_LONG_TAKEOFF ;
				FSM->COUNT_CurrentState = 0 ; // reset
				return STATE_LONG_TAKEOFF ;
			}

			// If meet max_wander_time, stop : 
			if(FSM->COUNT_CurrentState > FSM->T_Wander )
			{
				FSM->STATE = STATE_STOP ;
				FSM->COUNT_CurrentState = 0 ; // reset
				return STATE_STOP ;
			}


			return STATE_WANDER ;
		}
		case STATE_LONG_TAKEOFF:
		{
			// If sharp collision, excute short-mode escape :
			if(Visuomotor->Threat_Strength > Params->T_SHORT_TAKEOFF ) // T_SHORT_TAKEOFF = 1000000
			{
				FSM->STATE = STATE_SHORT_TAKEOFF ;
				FSM->COUNT_CurrentState = 0 ; // reset
				return STATE_SHORT_TAKEOFF ;
			}
			
			// If long takeoff over, go to wander state :
			if(FSM_aLPLC2_AlignCheck(FSM )/*Colias has turned to ideal direction*/ )
			{
				FSM->STATE = STATE_ESCAPE ;
				FSM->COUNT_CurrentState = 0 ; // reset

				FSM->STATE_Previous = STATE_LONG_TAKEOFF ;
				return STATE_ESCAPE ;
			}


			return STATE_LONG_TAKEOFF ;
		}
		case STATE_SHORT_TAKEOFF:
		{
			// Short tkeoff can be overridden by another short takeoff :
			if(Visuomotor->Threat_Strength > Params->T_SHORT_TAKEOFF ) // T_SHORT_TAKEOFF = 1000000
			{
				FSM->STATE = STATE_SHORT_TAKEOFF ;
				FSM->COUNT_CurrentState = 0 ; // reset --> the escaping direction will also be updated in motion control founction.
				return STATE_SHORT_TAKEOFF ;
			}


			// If short takeoff over, go to wander state :
			if(FSM_aLPLC2_AlignCheck(FSM )/*Colias has backward moved && turned to ideal direction*/ )
			{
				FSM->STATE = STATE_ESCAPE ;
				FSM->COUNT_CurrentState = 0 ; // reset
				
				FSM->STATE_Previous = STATE_SHORT_TAKEOFF ;
				return STATE_ESCAPE ;
			}


			return STATE_SHORT_TAKEOFF ;
		}
		case STATE_ESCAPE:
		{
			// If has escaped, go on to achieve wander state :
			if(FSM_aLPLC2_AlignCheck(FSM )/*Colias has turned to ideal direction*/ )
			{
				FSM->STATE = STATE_WANDER ;
				FSM->COUNT_CurrentState = 0 ; // reset
				return STATE_WANDER ;
			}

			// To be fill : 
			// if(COLLISION->LPLC2 == 1 ): Long-mode again ... 
			

			return STATE_ESCAPE ;
		}
		case STATE_STOP:
		{
			// If collision, excute long-mode escape by default : 
			if(FSM_aLPLC2_Collision(FSM) == 1 )
			{
				FSM->STATE = STATE_LONG_TAKEOFF ;
				FSM->COUNT_CurrentState = 0 ; // reset
				return STATE_LONG_TAKEOFF ;
			}
			

			return STATE_STOP ;
		}
		default:
		{
			FSM->STATE = STATE_STOP ;
			FSM->COUNT_CurrentState = 0 ; 
			return STATE_STOP ;
		}


	} // Swich Over ...
} // FSM Judge Over ...

// FSM State Motion Control : 
LPLC2_StateType FSM_aLPLC2_motion(FSM_LPLC2_StructTypedef* FSM )
{
	LPLC2struct_Visuomotor* Visuomotor = &(FSM->hLPLC2->Model->Visuomotor ) ; // Visuomotor results

	if (hCoS->hHCoS_Camera->Instance->cam_hFrameCountount <= 10 )
		return STATE_STOP ; 

	//printf("LCF running in current state ... %d  ",hObj->COUNT_CurrentState);

	
    // Get the current state :
	LPLC2_StateType STATE = FSM_aLPLC2_State_Run(FSM ) ; 
    
	// Motion control accrording to current STATE :
	switch(STATE)
	{
		case STATE_WANDER:
		{
			// First time in this state, set the speed and direction :
			if(FSM->COUNT_CurrentState == 0 ) 
			{
				MC1.dir_set = MC1.dir ; // goal = current direction
			}

			// Set speed :
			MC1.vrun = MC1.vrun_default * AC1.l ; // = 5 * 0.5 = 2.5
			
			// Motion Control :
			Motion_VB(MC1.vrun, PID1.u_control ) ; 
			
			
			return STATE_WANDER ;
		}
		case STATE_LONG_TAKEOFF: // stop and turn, then escape !
		{
			// First time in this state, set the speed and direction :
			if(FSM->COUNT_CurrentState == 0 ) 
			{
				double dir_bias ; // core
				int RANDOM = rand() ;

				// Tactic 1 : 
				// Turn (static 90 degree escape + random 0~90 degree ) :
				//dir_bias = Visuomotor->Escape_Direction + RANDOM % 90 ; 
				
				// Tactic 2 : 
				// Turn (static 180 degree escape + random -30~30 degree ) :
				
				RANDOM = RANDOM % 15 ;
				if(RANDOM % 2 == 0 ){
					if(Visuomotor->Escape_Direction >= 0){
						dir_bias = Visuomotor->Escape_Direction + 70 + RANDOM ; 
					}else{
						dir_bias = Visuomotor->Escape_Direction - 70 + RANDOM ; 
					}
				}else{
					if(Visuomotor->Escape_Direction >= 0){
						dir_bias = Visuomotor->Escape_Direction + 70 - RANDOM ; 
					}else{
						dir_bias = Visuomotor->Escape_Direction - 70 - RANDOM ; 
					}
				}
				

				MC1.dir_set = MC1.dir + dir_bias ; // set the target direction
				if(MC1.dir_set > 180 )
					MC1.dir_set -= 360 ;
				else if(MC1.dir_set < -180 )
					MC1.dir_set += 360 ; 

				//PID1.u_target=MC1.dir_set;
				
				MC1.vrun = 0 ; // long-mode posture adjustment.
			}

			// Turn Action :
			Motion_VB(MC1.vrun, PID1.u_control ) ; // input: (-2.5~2.5, -5~5 ) 


			return STATE_LONG_TAKEOFF ;
		}
		case STATE_SHORT_TAKEOFF: // backward while turning, then escape !
		{
			// First time in this state, set the speed and direction :
			if(FSM->COUNT_CurrentState == 0 ) 
			{
				// set the target direction
				// Tactic 1 : 
				// Meditation ... since it's the same as the long-mode

				// Tactic 2 :
				// Short-mode: 90 degree escape.
				MC1.dir_set = MC1.dir + Visuomotor->Escape_Direction ;
				if(MC1.dir_set > 180 )
					MC1.dir_set -= 360 ;
				else if(MC1.dir_set < -180 )
					MC1.dir_set += 360 ; 



				// Backward Speed by Looming Strength :
				MC1.vrun = - Visuomotor->Escape_Speed ; 
			}

			// Turn Action :
			Motion_VB(MC1.vrun, PID1.u_control ) ; // input: (-2.5~2.5, -5~5 ) 


			return STATE_SHORT_TAKEOFF ;
		}
		case STATE_ESCAPE: // escape from the collision !
		{
			// First time in this state, set the speed and direction :
			if(FSM->COUNT_CurrentState == 0 ) 
			{
				// set the target direction
				if(FSM->STATE_Previous == STATE_LONG_TAKEOFF){

					// Go Straight After Turning : 
					MC1.dir_set = MC1.dir ; 

				}else if(FSM->STATE_Previous == STATE_SHORT_TAKEOFF){

					// "V" Shape Escape : 
					if(Visuomotor->Threat_Direction >= 0){
						// [Threat_direction > 0] --> [Phase 1 Backward Escape_direction < 0] --> [Phase 2 Forward Escape_direction = Current_direction - 90 degree]
						MC1.dir_set = MC1.dir - 90 ;
					}else if(Visuomotor->Threat_Direction < 0 ){
						MC1.dir_set = MC1.dir + 90 ;
					}
					if(MC1.dir_set > 180 )
						MC1.dir_set -= 360 ;
					else if(MC1.dir_set < -180 )
						MC1.dir_set += 360 ; 

				}

				// Backward Speed by Looming Strength :
				MC1.vrun = Visuomotor->Escape_Speed ; 
			}

			// Turn Action :
			Motion_VB(MC1.vrun, PID1.u_control ) ; // input: (-2.5~2.5, -5~5 ) 


			return STATE_ESCAPE ;
		}
		case STATE_STOP: // stop motion !
		{
			MC1.vrun = 0 ;
			MC1.dir_set = MC1.dir ;
			Motion_VB(MC1.vrun, PID1.u_control) ;

			
			return STATE_STOP ;
		}
		default: // stop motion !
		{
			MC1.vrun = 0 ; 
			MC1.dir_set = MC1.dir ;
			Motion_VB(MC1.vrun, PID1.u_control) ;


			return STATE_STOP ;
		}
	}

}


// Motion Control : 
/************************************************************************************************************/

// 2025.03.31, Renyuan Added (motion control founction, from prey.c origenally ):
// This founction limits the speed value, for subsequent use in the motion control: 
void Motion_vset(u8 lr, double value)
{
	double* pd;
	double err;
	double newd;
	// choose left/right wheel: 
	if (lr==0){
		pd=&MC1.left; //lr==0, set left
	}else{
		pd=&MC1.right; //lr!=0, set right
	}

	// 1. limit the accelaration to Amax:
	err=value-*pd; // err = target speed - current speed
	if (err>MC1.Amax) // Amax: max A for acceleration, MC1.Amax = 1;
		err=MC1.Amax;
	if (err<MC1.Ammax) // Ammax: max A for brake, MC1.Ammax = -1;
		err=MC1.Ammax;
	newd=(*pd)+err; // newd: new speed, = current speed + err(stabilized )
	
	// 2. limit the speed to Vmax:
	if (newd>MC1.Vmax)
		newd=MC1.Vmax; // MC1.Vmax = MAX_SPEED * AC1.m(= 1 * 3.3 ) ; AC1.m=3.3; // motion gain
	if (newd<-MC1.Vmax)
		newd=-MC1.Vmax;
	
	
	*pd = newd ; // update the speed value of MC1.left/MC1.right
}

// level 1
void Motion_LR(double left, double right) // left = right = 25.5 || (-5~5 )
{
	// limit the speed and the acceleration, then set the speed of left and right wheel (MC1.left/MC1.right ).
	Motion_vset(0,left);
	Motion_vset(1,right);
	
	s16 leftS, rightS;
	if (AC1.en_motion && XIMU1.roll<150 && XIMU1.roll>-150){
		leftS=(s16)(MC1.left*100);
		rightS=(s16)(MC1.right*100);
	}else{ // the robot is flipped !
		leftS=0;
		rightS=0;
	}


	/*
	 *	//transform power of left and right wheel to Colias Basic,
	 *	// Protocol: 0x7a 0x9a(motror power register address) s16(left power[-1000~1000]) s16(right power[-1000~1000]) 0x0d 0x0a
	 *	uint8_t com[] = {0x7a, 0x9a, (u8)leftS, (u8)(leftS >> 8), (u8)rightS, (u8)(rightS >> 8), 0x0d, 0x0a};
	 *	//HAL_UART_Transmit(hCoS->huart2, com, sizeof(com), 1000);
	 */
	// transform power of left and right wheel to Colias Basic,
	// write motor register to colias basic protocol 0x78(write with NO ack) (reg address) (data...)
	uint8_t tx_buff[] = {0x78, MOTOR_CC_ADDR, 0x81, (u8)leftS, (u8)(leftS >> 8), (u8)rightS, (u8)(rightS >> 8)};
	// TX_size is not include 7d
	// assign NULL to receive buffer and receive size, so it will not receive any data
	SendFrametoCBU(hCoS->hHUART2, tx_buff, 6, NULL, NULL);
}

// level 2
void Motion_VB(double speed, double bias ) // bias is given as PID, -5 ~ 5
{
	double vl, vr ;

	vl = speed + (bias/2 ) ;
	vr = speed - (bias/2 ) ;
	Motion_LR(vl, vr ) ; // input: (-5~5, -5~5 ) 
}




// LED Control :
/************************************************************************************************************/

// |1 | 8|9 |16|17|24|
// |  R  |  G  |  B  |
// 使用 24 位表示 RGB（高 8 位为红色，中间 8 位为绿色，低 8 位为蓝色）
void LED_Set(uint8_t ID, uint32_t color)
{
	//split the input color into three individual 16-bit color data
	uint16_t red = (color & 0xff0000) >> 8;
	uint16_t green = (color & 0x00ff00) ;
	uint16_t blue = (color & 0x0000ff)<<8;

	CoS_CLED_setRGBsingle(ID, red, green, blue); // ID: ID of LED

}

/*
颜色名称	代码 (Hex)
蓝色		0x0000ff	全蓝，无红，无绿
红色		0xff0000	全红，无绿，无蓝
紫色		0xff20f0	红色、少量绿色、蓝色组合
绿色		0x00ff00	全绿，无红，无蓝
粉色		0xffefff	红色、少量绿色、少量蓝色
弱白光		0x000808	低亮度红、绿、蓝
暗红色		0x600000	中等亮度红，无绿，无蓝
*/
// Set LED color according to FSM state : 
void FSM_LED_Set(void)
{
	// FSM : 
	switch (FSM_aLPLC2.STATE )
	{
		case STATE_WANDER: // LED_1: white
		{
			LED_Set(1, 0xFFFFFF);
			break;
		}
		case STATE_LONG_TAKEOFF: // LED_1: slight red 
		{
			LED_Set(1, 0xff8080);
			break;
		}
		case STATE_SHORT_TAKEOFF: // LED_1: red
		{
			
			LED_Set(1, 0xff0000);
			break;
		}
		case STATE_ESCAPE: // LED_1: blue
		{
			LED_Set(1, 0x0000ff);
			break;
		}
		case STATE_STOP: // LED_2 off 
		{
			LED_Set(1, 0x000000);
			break;
		}
	}


	// Collison : 
	if(FSM_aLPLC2_Collision(&FSM_aLPLC2 ) == 1 ){
		
        LED_Set(2, 0xff8080); // LED_2: slight red
        
        if(FSM_aLPLC2.hLPLC2->Model->Visuomotor.Threat_Strength > FSM_aLPLC2.hLPLC2->Model->Params.T_SHORT_TAKEOFF )
            LED_Set(2, 0xff0000); // LED_2: red
	
    }else{
		LED_Set(2, 0x000000); // LED_2 off
	}
    
}
