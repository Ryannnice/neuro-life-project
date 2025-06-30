/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2024 VisoMorphic
  * All rights reserved.
  * 
  * 
  * 
  * 
  *
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "coliasapi.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <arm_math.h>

 


float vspeed=300;
float diff_sup=20;
float diff_light,l,r;

int main(void)
{
	  //Colias_Init();
    Colias_Init_aLPLC2();
    //CAPI_MotionEnable();

    
    Colias_loopbody_aLPLC2();
  



    //Colias_loopbody_depreciated();

    
    //CAPI_MotionSet_LR(30000,-30000);// -32768~32767

    /*
	  while(1)
	  {
	  	  l=CINFO.CBUsensors.TCS_LEFT_Clear;
	  	  r=CINFO.CBUsensors.TCS_RIGHT_Clear; // right Light --> right TCS value small
	  	  diff_light=(l-r)/diff_sup; // right TCS value small --> diff_light big
	  	  if (fabs(CINFO.IMUSensor.Roll)>90)
	  	  {
	  	      CAPI_MotionSet_LR(0,0);//stop motor running if flipped, for the noise 
	  	  }
	  	  else{
	  	  	  CAPI_MotionSet_LR(vspeed-diff_light,vspeed+diff_light); // diff_light big --> right power big --> move leftward
	  	  }
          
        delay_ms(30);

	  }
    */
  
  
  

}
