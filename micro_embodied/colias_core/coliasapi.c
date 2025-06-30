#include "coliasapi.h"

#include "user_terminal.h"
#include "coliasSense_board.h"
#include "coliasSense_Motion.h"
#include "debug.h" // Renyaun added, 2025.6.1
//#include "prey.h"
//#include "LCFSM.h"
#include "visuomotor_LPLC2.h"

volatile uint8_t Flag_image_ready=0;



volatile u8 testdt[4]={0x3a,0x35,0x22,0xf2};
u8 testStr[]={" Hello Wrold!\r\n"};
uint8_t Key1_psta=0;
uint8_t Key2_psta=0;
uint8_t key_sta[2]={0};


volatile uint16_t Owidth=156,Oheight=120;
volatile int32_t pixel_capture_adjust=0;
volatile uint8_t disp_mode=0x02;
u8 DEMO_mode=0;
char buffer[500]={0};
u16 a = 0x1234;
volatile u8 b , c ;
static char datetimestring[]=__DATE__" "__TIME__;

MotionController_Typedef MC1;
GlobalArgType AC1;
ColiasStatus_Typedef CINFO;


// This Init Founction Initialize the Necessary Part of aLPLC2 Running Environment.
void Colias_Init_aLPLC2(void)
{
	CoS_SenseBoard_Init(hCoS);
	CoS_UART_Enable(hCoS->hHUART1,ENABLE);
	printf("Resetting CBU.\r\n ");
	CoS_CBU_reset();
	delay_ms(200);
	UT_Init();
	
	// LED : 
	hCoS->hHCLED->Instance->CC.mode=CLED_Breath;
	hCoS->hHCLED->cledL.CC.mode=CLED_Breath; // LED1
	hCoS->hHCLED->cledM.CC.mode=CLED_Breath; // at tail
	hCoS->hHCLED->cledR.CC.mode=CLED_Breath; // LED2
	hCoS->hHCLED->cledM.Rs.freq=300;
	hCoS->hHCLED->cledM.Gs.freq=400;
	hCoS->hHCLED->cledM.Bs.freq=500;
	
	
	if (hCoS->hHCoS_Camera->iState==CoS_STATE_OK)
	{
		CoS_CAM_DMASet(hCoS->hHCoS_Camera,(uint32_t)Image,(Image_Width*Image_Height*3)/2);
		CoS_CAM_Start(hCoS->hHCoS_Camera);
	}

	// IMU : 
	//tilt_detection_init(&TiltSense);
	XIMUs_Init();
	
	delay_ms(50);
	hCoS->hHCLED->cledL.CC.mode=CLED_Freeze;
	hCoS->hHCLED->cledR.CC.mode=CLED_Freeze; // stop LED 1 and 2, only LED 4 is breathing
	CAPI_TURNOFF_BOTTOMLED();
	delay_ms(20);
	CBU_Check(hCoS->hHUART2,NULL,NULL);
	delay_ms(20);
	CAPI_UPdateCBUInfo(0);
	delay_ms(20);
	// Motion Params :
	Motion_Init() ;
	delay_ms(20);
	// FSM Params :
	FSM_aLPLC2_Init(&FSM_aLPLC2 ) ;
	delay_ms(20);
	// LPLC2 Params :
	LPLC2_Init(&hLPLC2); // Renyuan added, 2025.1.3


	

	// below is origionally in prey.c, but now moved to here : 
	/*
	DiffImg_Enable();
	// enable CBU motor and set speed 0
	CoS_StatusTypeDef sta = CoS_STATE_UNSET;
	// write motor register to colias basic protocol 78(NO ack) (reg address) (data...)
	uint8_t tx_buff[] = {0x78, MOTOR_CC_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00};
	// TX_size is not include 7d
	// gave NULL to receive buffer and receive size, so it will not receive any data
	sta = SendFrametoCBU(hCoS->hHUART2, tx_buff, 6, NULL, NULL);
	// if (sta == CoS_STATE_OK)
	// {
	// 	// printf("CBU motor enable success\r
	// }

	// turn off bottom LED on colias basic
	uint8_t tx_buff1[] = {0x78, 0x12, 0x80};
	sta = SendFrametoCBU(hCoS->hHUART2, tx_buff1, 2, NULL, NULL);
	delay_ms(100);
	*/

	BackgroundTask_Enable();

}




void Colias_loopbody_aLPLC2(void )
{
	while(1 )
	{
		if(Flag_image_ready == 1 )
		{
			Flag_image_ready = 0 ; 

			// See : 
			aLPLC2_VisualModel() ; 
			
			// And Move : 
			MC1.dir=XIMU1.gzsum ; // get current direction
			PID1.u_measure = MC1.dir ; 
			PID1.u_target = MC1.dir_set ; 
			PID_run(&PID1) ; 
			LPLC2_StateType STATE = FSM_aLPLC2_motion(&FSM_aLPLC2 ) ;

 			FSM_LED_Set() ;
		}
	}
}


void aLPLC2_VisualModel(void)
{

	static uint32_t temp = 0, interval_begin = 0 ;
	
	hLPLC2.Interval_aLPLC2 = TOCin5 - interval_begin ; // timing ends. aLPLC2 interval: 32 ms, < 33 ms.
	interval_begin = TOCin5 ; // timing starts
	
	temp = TOCin5 ;
    LPLC2_PreSynaptic(&hLPLC2); // 1.9 ms
	hLPLC2.RunTime_aLPLC2_PreSynaptic = TOCin5 - temp ;

	temp = TOCin5 ;
    LPLC2_T4_T5(&hLPLC2); // 2.1 ms
	hLPLC2.RunTime_aLPLC2_T4_T5 = TOCin5 - temp ;

	temp = TOCin5 ;
    LPLC2_Projection(&hLPLC2); // 0.0017 ms
	hLPLC2.RunTime_aLPLC2_Projection = TOCin5 - temp ;
	

	VisionToMotor(&hLPLC2);
}


/*
void Colias_loopbody_depreciated()
{
	while (1)
	{
		static u32 pn=0;
		//LGMD_MLT_test_run();
		//volatile u16 foo;

		if(Flag_image_ready==1)
		{
			
			Flag_image_ready=0;
			//transImage();
			PreyAction();
			int n;
			//n=Debug_output_format(buffer,0);
			if (n>0) 
			{	
				if (AC1.P & 1)
				;	//printf("%s",buffer);
				
			}
			//MotionControl();
		}
		 //HAL_Delay(5000);
            //Cfinder_run();
	
		pn++;
	}
}
*/


void CAPI_UPdateCBUInfo(uint8_t seq)
{
	
	CINFO.battery_level=CAPI_CBU_Read_battery();
	delay_ms(10);
	CINFO.temperature_CBU=CAPI_CBU_Read_Temperature();
	delay_ms(10);
	CINFO.ID=CAPI_CBU_ReadID();
	delay_ms(10);
	CINFO.version=CAPI_CBU_ReadVersion();
	delay_ms(10);

}

void CAPI_UpdateCBUsensors()
{
	uint16_t temp[20];
	static uint8_t seq=0;
	static uint32_t timerlog[6],ttmp;

	//read TCRT sensors
	ttmp=TOCin5;
	CAPI_CBU_Read_TCRT_Dark(temp);
	timerlog[0]=TOCin5-ttmp;
	//temp[0] [7:0]->TCRTLD[11:4], temp[1][7:4]->TCRTLD [3:0]
	CINFO.CBUsensors.TCRTL_D=(float)temp[0]/16.384;// 1.024 is the conversion factor from 10bit ADC to 1000 scale. Although 1024 is quite close to 1000, it is not exactly 1000, so we need to divide by 1.024
	CINFO.CBUsensors.TCRTM_D=(float)temp[1]/16.384;
	CINFO.CBUsensors.TCRTR_D=(float)temp[2]/16.384;
	timerlog[1]=TOCin5-ttmp;
	CAPI_CBU_Read_TCRT_Light(temp);  
	timerlog[2]=TOCin5-ttmp;	
	CINFO.CBUsensors.TCRTL_L=(float)temp[0]/16.384;
	CINFO.CBUsensors.TCRTM_L=(float)temp[1]/16.384;
	CINFO.CBUsensors.TCRTR_L=(float)temp[2]/16.384;

	//read TCS sensors
	timerlog[3]=TOCin5-ttmp;
	CAPI_CBU_Read_TCS(temp);
	timerlog[4]=TOCin5-ttmp;
	//temp[0] [7:0]->TCS_FRONT_Clear[7:0], temp[1][7:0]->TCS_FRONT_Clear [15:8]
	CINFO.CBUsensors.TCS_FRONT_Clear=(float) temp[0];
	CINFO.CBUsensors.TCS_FRONT_Red=(float) temp[1];
	CINFO.CBUsensors.TCS_FRONT_Green=(float) temp[2];
	CINFO.CBUsensors.TCS_FRONT_Blue=(float) temp[3];

	CINFO.CBUsensors.TCS_RIGHT_Clear=(float) temp[4];
	CINFO.CBUsensors.TCS_RIGHT_Red=(float) temp[5];
	CINFO.CBUsensors.TCS_RIGHT_Green=(float) temp[6];
	CINFO.CBUsensors.TCS_RIGHT_Blue=(float) temp[7];
	
	CINFO.CBUsensors.TCS_LEFT_Clear=(float) temp[8];
	CINFO.CBUsensors.TCS_LEFT_Red=(float) temp[9];
	CINFO.CBUsensors.TCS_LEFT_Green=(float) temp[10];
	CINFO.CBUsensors.TCS_LEFT_Blue=(float) temp[11];
	timerlog[5]=TOCin5-ttmp;
	//update temperature and vbat every 10 loops
	if ( seq%10==0)
	{
		CINFO.temperature_CBU=CAPI_CBU_Read_Temperature();
		CINFO.battery_level=CAPI_CBU_Read_battery();
	}
	seq++;
	

}


/*
void loop_calculateVisualModels(void)
//this function is called in the main loop to calculate the visual models
{
	if(Flag_image_ready==1)
	{
		Flag_image_ready=0;
		Diff_img_calc(hCoS->hHCoS_Camera->Instance->cam_hFrameCountount, &LGMD1CC);
		Calc_LGMD(&LGMD1CC);
		Cfinder_run();

	}
}
*/

void CAPI_UpdateVisualSensors(void)
//this function handles the calculatation of diff image, LGMD and target detection 
{
	//CINFO.LGMDResult.LGMDout=LGMD1CC.hModel

}
void CAPI_UpdateSystime(void)
{
	CINFO.Sys_time=TOCin5/10000;
}

void CAPI_UpdateIMUSensors(void)
{
	//copy data from XIMU struct into the ColiasStatus struct, see IMUSensorData_Typedef for related variables
	CINFO.IMUSensor.YawSimple=XIMU1.gzsum;
	CINFO.IMUSensor.Roll=XIMU1.roll; // used in main while loop
	CINFO.IMUSensor.Pitch=XIMU1.pitch;
	CINFO.IMUSensor.Yaw=XIMU1.yaw;
	CINFO.IMUSensor.Ax=XIMU1.ax;
	CINFO.IMUSensor.Ay=XIMU1.ay;
	CINFO.IMUSensor.Az=XIMU1.az;
	CINFO.IMUSensor.Gx=XIMU1.gx;
	CINFO.IMUSensor.Gy=XIMU1.gy;
	CINFO.IMUSensor.Gz=XIMU1.gz;
	CINFO.IMUSensor.Mx=XIMU1.mx;
	CINFO.IMUSensor.My=XIMU1.my;
	CINFO.IMUSensor.Mz=XIMU1.mz;
	CINFO.IMUSensor.Temp=XIMU1.temperature;
	//q
	CINFO.IMUSensor.q[0]=XIMU1.q[0];
	CINFO.IMUSensor.q[1]=XIMU1.q[1];
	CINFO.IMUSensor.q[2]=XIMU1.q[2];
	CINFO.IMUSensor.q[3]=XIMU1.q[3];

}

void CAPI_MotionEnable(void)
{
	AC1.en_motion = 1 ; // 2025.1.7, Renyuan added.
	CAPI_CBU_WriteByte(0x99,0x80);
}

void CAPI_MotionDisable(void)
{
	AC1.en_motion = 0 ; // 2025.1.7, Renyuan added.
	CAPI_CBU_WriteByte(0x99,0x00);
}

void CAPI_MotionSet_LR(int16_t Lpower, int16_t Rpower) // Absolute power value: -32768 ~ 32767
{
	uint8_t data[4];
	data[0]=Lpower&0xff; // 1111111101010101 & 0000000011111111 = 01010101, low 8 bit
	data[1]=Lpower>>8;	 // 1111111101010101 		>>8         = 11111111, high 8 bit
	data[2]=Rpower&0xff; 
	data[3]=Rpower>>8;   
	CAPI_CBU_WriteBytes(0x9A,4,data); // #define MOTOR_POWER_L 0x9A
}

void CAPI_MotionSet_IncLR(int8_t Lincpower, int8_t Rincpower) // Power increment: -128 ~ 127
{
	uint8_t data[2];
	data[0]=Lincpower;
	data[1]=Rincpower;
	CAPI_CBU_WriteBytes(0x9E,2,data);
}

