#include "coliasSense_LPLC2.h"
//#include "coliasSense_board.h"
//#include "delay.h"
#include <math.h>
//#include <arm_math.h>
#include <stdlib.h>

//#include "coliasapi.h"




//hLPLC2->hFrameCount = &hCoS->hHCoS_Camera->Instance->cam_hFrameCountount;

#define Image_Width 99
#define Image_Height 72
//#define NUM_AF 3 // moved to LPLC2_model.h

// Use a packed structure to ensure that Image and Diff_Image are stored contiguously
//#pragma pack(1)  // Ensure no padding bytes are added in structure

uint16_t Image[3][Image_Height][Image_Width]; // uint16_t
int8_t Diff_Image[2][Image_Height][Image_Width]; // int8_t
// LPLC2Type LPLC2 ; // moved to CCMRAM_DEF.c // On-line version
LPLC2Type LPLC2 ; // Off-line version
LPLC2_pControlTypedef hLPLC2 ;

//LPLC2struct_Results AF[NUM_AF] ; // moved to LPLC2_model.h



void LPLC2_Init(LPLC2_pControlTypedef* hLPLC2)
{
    // Parameters Initialization :

    LPLC2.Params.W_ON = 0 ;
    LPLC2.Params.W_OFF = 1 ;
    LPLC2.Params.r = 1 ;
    LPLC2.Params.t = 30 ;
    LPLC2.Params.Bias = 7 ; // >> 7 bit // EMD Bias. Biger value means biger substraction.
    LPLC2.Params.AF_Radius = Height / 4 ;
    LPLC2.Params.AF_Radius_Exclusive = Height / 3 ;



    // Essential to EMD :
    ///////////////////////////////////////////////////////////////////////////////////
    LPLC2.Params.Step = 1 ; // for_loop step length
    LPLC2.Params.Nc = 3 ; // connected neuron number
    LPLC2.Params.d = 1 ; // distance between two input position in EMD
    LPLC2.Params.Delay = 1 ; // delay between two adjacent T4/T5 neurons
    LPLC2.Params.T_cardinal = 10000 ; // 20 /* Binary Case */ ; 
    ///////////////////////////////////////////////////////////////////////////////////


    LPLC2.Params.T_LPLC2 = LPLC2.Params.T_cardinal*40*2 ; // T_cardinal + T_cardinal + T_cardinal + T_cardinal
    LPLC2.Params.n = 1 ; // Spike number threshold for collision 
    LPLC2.Params.AF_Threshold = 50000 ;

    // LIF Parameters: 
    /*
    float tau_m ; // Membrane time constant
    int V_rest ; // Resting potential
    int V_threshold ; // Threshold potential for spiking
    int V_reset ; // Reset Potential
    int R ; // Membrane resistance
    */
    LPLC2.Params.tau_m = 0.1 ;
    LPLC2.Params.V_rest = -60 ;
    LPLC2.Params.V_threshold = -50 ;
    LPLC2.Params.V_reset = -70 ;
    LPLC2.Params.V_spike = 10 ;
    LPLC2.Params.R = 0.000003 ;
    LPLC2.Params.max_input = 3000000 ;



/*
    // Results Initialzation : 
    LPLC2.Results.Centroid[0] = Height / 2 ;
    LPLC2.Results.Centroid[1] = Width / 2 ;

    LPLC2.Results.Quadrant[0] = 0 ;
    LPLC2.Results.Quadrant[1] = 0 ;
    LPLC2.Results.Quadrant[2] = 0 ;
    LPLC2.Results.Quadrant[3] = 0 ;

    LPLC2.Results.LPLC2 = 0 ;
    LPLC2.Results.I = 0 ;
    //LPLC2.Results.OUTPUT = LPLC2.Params.V_rest ;
    for(int i = 0; i < 30; i ++ )
    {
        LPLC2.Results.OUTPUT[i] = LPLC2.Params.V_rest ;
    }
    LPLC2.Results.Collision = 0 ;
*/

    // AFs Initialization for Multiple-Attention Mechanism : 
    for(int j = 0; j < NUM_AF; j ++ ) 
    {
        LPLC2.AF[j].Vacant_AF = 1 ;

        // NUM_AF Times Replication of Above ... 
        LPLC2.AF[j].Centroid[0] = Height / 2 ;
        LPLC2.AF[j].Centroid[1] = Width / 2 ;

        LPLC2.AF[j].Quadrant[0] = 0 ;
        LPLC2.AF[j].Quadrant[1] = 0 ;
        LPLC2.AF[j].Quadrant[2] = 0 ;
        LPLC2.AF[j].Quadrant[3] = 0 ;

        LPLC2.AF[j].LPLC2 = 0 ;
        LPLC2.AF[j].I = 0 ;
        //LPLC2.Results.OUTPUT = LPLC2.Params.V_rest ;
        for(int i = 0; i < 30; i ++ )
        {
            LPLC2.AF[j].OUTPUT[i] = LPLC2.Params.V_rest ;
        }
        LPLC2.AF[j].Collision = 0 ;
    }

    // Initialize AF : 
    for(int i = 0; i < Height; i ++ )
    {
        for(int j = 0; j < Width; j ++ )
        {
            LPLC2.Layers.IS_AF[i][j] = 0 ; 
        }
    }


    // hLPLC2 Handle Initialization :
    hLPLC2->currentImage = 0 ;
    hLPLC2->currentDiffImage = 1 ;
    hLPLC2->Enable = 1 ;
    hLPLC2->processCount = 1 ;
    hLPLC2->processCountLast = 1 ;
    hLPLC2->status = 0 ;
    hLPLC2->processRate = 30 ;
    //hLPLC2->AGC_enable_period = 0 ;
    //hLPLC2->hFrameCount = &hCoS->hHCoS_Camera->Instance->cam_hFrameCountount; // Off-line version: updated in for-loop

    // Put initilaized Model into hLPLC2 Handle :
    hLPLC2->Model = &LPLC2 ; // LPLC2 is declared in CCMRAM_DEF.c

    hLPLC2->Attention_Mechanism = 2 ; // 0: attention is static, at the center of RF.

    hLPLC2->LPLC2_cross_shape = 'X' ; // 'X' shape or '+' shape.

    hLPLC2->EMD_test_mode = 0 ; // 0: LPLC2 mode; 1: test EMD in the bottom right quadrant of visual field.
}


void LPLC2_PreSynaptic(LPLC2_pControlTypedef* hLPLC2)
{
    uint8_t ImgArr_Cur[3] = {2, 0, 1 } ;
    uint8_t ImgArr_Pre[3] = {1, 2, 0 } ;

    uint16_t *Img_Pixel_Cur, *Img_Pixel_Pre ;
    int8_t *DiffImg_Pixel_Cur ;

    // Two consecutive frame delay : 
    hLPLC2->currentDiffImage = !(hLPLC2->currentDiffImage) ; // jump between 1 and 0
    uint8_t cur = hLPLC2->currentDiffImage ;
    
    // DELAY_WINDOW length delay : 
    uint8_t cur_window = *hLPLC2->hFrameCount % DELAY_WINDOW ; // 0, 1, 2, 3, 4, 5, 6, 7, 8, 9. 
    hLPLC2->cur = cur_window ; // 0, 1, 2, 3, 4, 5, 6, 7, 8, 9. 
    
    Img_Pixel_Cur = (uint16_t* )((uint8_t* )&Image[ImgArr_Cur[(*(hLPLC2->hFrameCount))%3]][0][0] + 1 );
    Img_Pixel_Pre = (uint16_t* )((uint8_t* )&Image[ImgArr_Pre[(*(hLPLC2->hFrameCount))%3]][0][0] + 1 );
    // Image[][][]: 1111111100000000  -->  Img_Pixel_Pre: 0000000011111111, we've got Img_Pixel_Cur: 255 now
    DiffImg_Pixel_Cur = &Diff_Image[cur][0][0] ;
    
    LPLC2struct_Params* Params = &(hLPLC2->Model->Params) ;
    //LPLC2struct_Results* Results = &(hLPLC2->Model->Results) ;
    LPLC2struct_Results* AF = &(hLPLC2->Model->AF[0] ) ; // AFs 
    LPLC2struct_Layers* Layers = &(hLPLC2->Model->Layers) ;


    uint8_t R = Params->r ;
    uint8_t t = Params->t ;
    uint8_t AF_Radius = Params->AF_Radius ;
    uint8_t AF_Radius_Exclusive = Params->AF_Radius_Exclusive ;

    uint8_t x = 0 ;
    uint8_t y = 0 ;
    uint16_t i, j ;

/*
    if(hLPLC2->Attention_Mechanism == 1 ){
        // Use MAX as AF Center : 
        uint16_t MAX = 0 ;
        // Use Centroid Calculation Formula to Find AF Center : 
        // centroid_x = numerator_x / denominator_x 
        uint64_t numerator_y = 0 ;
        uint64_t denominator_y = 0 ;
        uint64_t numerator_x = 0 ;
        uint64_t denominator_x = 0 ;
        for (i = 0 ; i < (Width * Height) ; i ++ )
        {
            //int16_t Diff = ((*Img_Pixel_Cur) - (*Img_Pixel_Pre)) >> 9 ;
            int16_t Diff = ((*Img_Pixel_Cur) - (*Img_Pixel_Pre)) ;
            if(Diff > 127)
                Diff = 127 ;
            if(Diff < -127)
                Diff = -127 ;
            *DiffImg_Pixel_Cur = Diff ;

            y = i / Width ;
            x = i % Width ;

            if(Diff >= 0){
                // ON Channel
                Layers->ON[cur_window][y][x] = Diff ; 
                Layers->OFF[cur_window][y][x] = 0 ;
                
                // This operation is moved to post-filter part: 
                // Integration in Centroid Calculating Formula :
                //numerator_y   += Params->W_ON * Diff * y ;
                //denominator_y += Diff ;
                //numerator_x   += Params->W_ON * Diff * x ;
                //denominator_x += Diff ;
                
            }else{
                // OFF Channel
                Layers->OFF[cur_window][y][x] = -Diff ; 
                Layers->ON[cur_window][y][x] = 0 ; 
                
                // This operation is moved to post-filter part: 
                // Integration in Centroid Calculating Formula :
                //numerator_y   += Params->W_OFF * (-Diff) * y ;
                //denominator_y += (-Diff) ;
                //numerator_x   += Params->W_OFF * (-Diff) * x ;
                //denominator_x += (-Diff) ;
                
            }
            
            // Use MAX as AF Center : 
            //if(abs(Diff) > MAX )
            //{
            //  MAX = abs(Diff) ; 
            //  Results->Centroid[0] = i / Width ; // Round Down
            //  Results->Centroid[1] = i % Width ; 
            //}
            

            // move on 
            Img_Pixel_Cur ++ ;
            Img_Pixel_Pre ++ ;
            DiffImg_Pixel_Cur ++ ;
        }

        // Mean-Value Filter : 
        for(y = r; y < Height - r; y ++ )
        {
            for(x = r; x < Width - r; x ++ )
            {
                Layers->OFF[cur_window][y][x] =(Layers->OFF[cur_window][y-1][x-1] + Layers->OFF[cur_window][y-1][x] + Layers->OFF[cur_window][y-1][x+1]
                                              + Layers->OFF[cur_window][y][x-1]   + Layers->OFF[cur_window][y][x]   + Layers->OFF[cur_window][y][x+1]
                                              + Layers->OFF[cur_window][y+1][x-1] + Layers->OFF[cur_window][y+1][x] + Layers->OFF[cur_window][y+1][x+1] )
                                              / 9 ;
                Layers->OFF[cur_window][y][x] = Layers->OFF[cur_window][y][x] > t ? Layers->OFF[cur_window][y][x] : 0 ; // Threshold


                // Integration in Centroid Calculating Formula :
                numerator_y   += Layers->OFF[cur_window][y][x] * y ;
                denominator_y += Layers->OFF[cur_window][y][x] ;
                numerator_x   += Layers->OFF[cur_window][y][x] * x ;
                denominator_x += Layers->OFF[cur_window][y][x] ;
            }
        } // We've Got OFF Layer after Filtering. 


        // Update Centroid as AF Center :
        Results->Centroid[0] = numerator_y / (denominator_y + 1) ;
        Results->Centroid[1] = numerator_x / (denominator_x + 1) ;


    }else if(hLPLC2->Attention_Mechanism == 0 ){ // Colias's eye is stuck ... 
        for (i = 0 ; i < (Width * Height) ; i ++ )
        {
            // '>>9' is a redundant operation to line 101 & 102 '(uint16_t* )((uint8_t* )'
            //int16_t Diff = ((*Img_Pixel_Cur) - (*Img_Pixel_Pre)) >> 9 ;
            int16_t Diff = ((*Img_Pixel_Cur) - (*Img_Pixel_Pre)) ; 
            if(Diff > 127)
                Diff = 127 ;
            if(Diff < -127)
                Diff = -127 ;
            *DiffImg_Pixel_Cur = Diff ; // 1111 1111


            y = i / Width ;
            x = i % Width ;

            if(*DiffImg_Pixel_Cur > 0){
                // ON Channel
                Layers->ON[cur_window][y][x] = *DiffImg_Pixel_Cur ; // 1 ; 
                Layers->OFF[cur_window][y][x] = 0 ;
            }else if(*DiffImg_Pixel_Cur < 0 ){
                // OFF Channel
                Layers->OFF[cur_window][y][x] = -*DiffImg_Pixel_Cur ; // 1 ; 
                Layers->ON[cur_window][y][x] = 0 ; 
            }else{
                // No Luminance Change
                Layers->ON[cur_window][y][x] = 0 ; 
                Layers->OFF[cur_window][y][x] = 0 ; 
            }

            // move on 
            Img_Pixel_Cur ++ ;
            Img_Pixel_Pre ++ ;
            DiffImg_Pixel_Cur ++ ;
        }
    }else{ 
*/
        // multiple-attention mechanism !!!


        // PHASE 1: Global Information :        

        // Use MAX as AF Center : 
        int16_t MAX_OFF = 0 ;
        uint8_t MAX_y = 0, MAX_x = 0 ;
        for (i = 0 ; i < (Width * Height) ; i ++ )
        {
            y = i / Width ;
            x = i % Width ;

            //int16_t Diff = ((*Img_Pixel_Cur) - (*Img_Pixel_Pre) ) >> 9 ;
            int16_t Diff = ((*Img_Pixel_Cur) - (*Img_Pixel_Pre) ) ;
            
            if(Diff > 127 )
                Diff = 127 ;
            if(Diff < -127 )
                Diff = -127 ;
            *DiffImg_Pixel_Cur = Diff ;

            if(Diff >= 0){
                // ON Channel
                Layers->ON[cur_window][y][x] = Diff ; 
                Layers->OFF[cur_window][y][x] = 0 ; 
            }else{
                // OFF Channel
                Layers->OFF[cur_window][y][x] = -Diff ; 
                Layers->ON[cur_window][y][x] = 0 ; 
            }

            // move on 
            Img_Pixel_Cur ++ ;
            Img_Pixel_Pre ++ ;
            DiffImg_Pixel_Cur ++ ;
        
        }

        // Mean-Value Filter : 
        for(y = R; y < Height - R; y ++ )
        {
            for(x = R; x < Width - R; x ++ )
            {
                Layers->OFF[cur_window][y][x] =(Layers->OFF[cur_window][y-1][x-1] + Layers->OFF[cur_window][y-1][x] + Layers->OFF[cur_window][y-1][x+1]
                                              + Layers->OFF[cur_window][y][x-1]   + Layers->OFF[cur_window][y][x]   + Layers->OFF[cur_window][y][x+1]
                                              + Layers->OFF[cur_window][y+1][x-1] + Layers->OFF[cur_window][y+1][x] + Layers->OFF[cur_window][y+1][x+1] )
                                              / 9 ;
                Layers->OFF[cur_window][y][x] = Layers->OFF[cur_window][y][x] > t ? Layers->OFF[cur_window][y][x] : 0 ; // Threshold
            }
        } // We've Got OFF Layer after Filtering. 

        // Search MaxPoint : 
        for (i = 0 ; i < (Width * Height) ; i ++ )
        {
            y = i / Width ;
            x = i % Width ;

            // Multiple-Attention Mechanism : 
            // Find MaxPoint Beyond All Existing AFs:
            if(Layers->IS_AF[y][x] == 0 ){ // Only If All Bits are 0, Layers->IS_AF[y][x] == 0, which means no any AF exists here.
                if(Layers->OFF[cur_window][y][x] > MAX_OFF ){
                    // Find MaxPoint in OFF Channel
                    MAX_OFF = Layers->OFF[cur_window][y][x] ; 
                    // Update Maxpoint's Coordination, for new AF establishment
                    MAX_y = y ;
                    MAX_x = x ;
                    printf("MAX_OFF: %d, New AFC: [%d, %d] \n",MAX_OFF, MAX_y, MAX_x ) ;
                }
            }

        } // We've Got MaxPoint's (y, x) Coordination now.

          



        // PHASE 2: Local Luminance Change Information in AFs:

        int16_t col, row, r, l, d, u ; 
        // Traversal All Existing Attention Fields: Is there any redundant AF ? 
        for(i = 0; i < NUM_AF; i ++ )
        {

         //   // Monitor Existing AFs: 
         //   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
         //   printf("\n\nAF %d: Active: %d Centroid: [%d, %d] ",i , !AF[i].Vacant_AF, AF[i].Centroid[0], AF[i].Centroid[1] ) ;
         //   
         //   //printf("   [%d] \n", AF_y_top) ;
         //   //printf("[%d]  [%d] <-- AF \n", AF_x_left, AF_x_right) ;
         //   //printf("   [%d] \n", AF_y_bottom) ;
         //   printf("\n[ Q2_L: %d, Q2_D: %d ] | [ Q1_R: %d, Q1_D: %d ] \n", AF[i].Q2_L, AF[i].Q2_D, AF[i].Q1_R, AF[i].Q1_D ) ;
         //   printf("------------------[%d,%d]----------------------", AF[i].Centroid[0], AF[i].Centroid[1] ) ;
         //   printf("\n[ Q3_L: %d, Q3_U: %d ] | [ Q4_R: %d, Q4_U: %d ] \n", AF[i].Q3_L, AF[i].Q3_U, AF[i].Q4_R, AF[i].Q4_U ) ;
         //   printf("\nQ[0]: %d, Q[1]: %d, Q[2]: %d, Q[3]: %d \n", AF[i].Quadrant[0], AF[i].Quadrant[1], AF[i].Quadrant[2], AF[i].Quadrant[3] ) ;
         //   
         //
         //   printf("\n %d  Current LPLC2 Output, %d ", AF[i].Output[(*(hLPLC2->hFrameCount)-1)%4], (*(hLPLC2->hFrameCount)-1)%4 ) ;
         //   printf("\n %d  Recent LPLC2 Output ", AF[i].LPLC2 ) ;
         //   printf("\n %f  I ", AF[i].I ) ;
         //   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


            // Is there any redundant AF ? 
            if(AF[i].Vacant_AF == 0 ){
                if(AF[i].LPLC2 < Params->AF_Threshold ){

                    // Redundant AF Range : 
                    d = AF[i].Centroid[0] + AF_Radius_Exclusive ;
                    u = AF[i].Centroid[0] - AF_Radius_Exclusive ;
                    r = AF[i].Centroid[1] + AF_Radius_Exclusive ;
                    l = AF[i].Centroid[1] - AF_Radius_Exclusive ;
                    // Drop This AF: 
                    AF[i].LPLC2 = 0 ;
                    for(int j = 0; j < 4; j ++ )
                    {
                        AF[i].Quadrant[j] = 0 ;
                        AF[i].Output[j] = 0 ;
                    }
                    for(row = u; row < d; row ++ )
                    {
                        if(row < 0 || row >= Height )
                            continue ;
                        for(col = l; col < r; col ++ )
                        {
                            if(col < 0 || col >= Width )
                                continue ;

                            //Layers->IS_AF[row][col] = 0 ; // Vacant this area !
                            Layers->IS_AF[row][col] &= ~(1 << i) ;

                        }

                    }


                    AF[i].Vacant_AF = 1 ;
                    printf(" AF[%d] OUT !!! ", i ) ; 
                }
            }
            
        }


        // Traversal All Existing Attention Fields: Is there any vacant position to establish a new AF ? 
        for(i = 0; i < NUM_AF; i ++ )
        {
            if(AF[i].Vacant_AF == 1 ){
                // Establish New AF at Vacant Position :
                AF[i].Vacant_AF = 0 ;
                printf("\n\nAF[%d] ESTABLISHED !!! ", i ) ; 

                AF[i].Centroid[0] = MAX_y ;
                AF[i].Centroid[1] = MAX_x ;

                //printf("\nMAX_OFF: %d (%d, %d)\n",MAX_OFF, MAX_y, MAX_x ) ;
                printf("AFC: [%d, %d] ", AF[i].Centroid[0], AF[i].Centroid[1] ) ;

                d = AF[i].Centroid[0] + AF_Radius_Exclusive ;
                u = AF[i].Centroid[0] - AF_Radius_Exclusive ;
                r = AF[i].Centroid[1] + AF_Radius_Exclusive ;
                l = AF[i].Centroid[1] - AF_Radius_Exclusive ;
                printf("\nRange: y: %d~%d x: %d~%d \n", u, d-1, l, r-1 ) ;

                for(row = u; row < d; row ++ )
                {
                    if(row < 0 || row >= Height ) 
                        continue ;
                    for(col = l; col < r; col ++ )
                    {
                        if(col < 0 || col >= Width )
                            continue ;
                        //Layers->IS_AF[row][col] = 1 ; // Occupy this area !
                        Layers->IS_AF[row][col] |= (1 << i) ; 
                    }
                }

                break ; // Generate only 1 new AF for a time.
            } // else: keep on monitoring current existing AFs ... 
            
        }


        // Update AF Center: 
        for(i = 0; i < NUM_AF; i ++ )
        {
            // Update AFC of Existing AF : 
            if(AF[i].Vacant_AF == 0 ){
                // Reset AF:
                d = AF[i].Centroid[0] + AF_Radius_Exclusive ; // AF_Radius ; // This is wrong since it only up the inner AF status below(line 524 ).
                u = AF[i].Centroid[0] - AF_Radius_Exclusive ; // AF_Radius ; // This is wrong since it only up the inner AF status below(line 524 ).
                r = AF[i].Centroid[1] + AF_Radius_Exclusive ; // AF_Radius ; // This is wrong since it only up the inner AF status below(line 524 ).
                l = AF[i].Centroid[1] - AF_Radius_Exclusive ; // AF_Radius ; // This is wrong since it only up the inner AF status below(line 524 ).
                for(row = u; row < d; row ++ )
                {
                    if(row < 0 || row >= Height )
                        continue ; 
                    for(col = l; col < r; col ++ )
                    {
                        if(col < 0 || col >= Width )
                            continue ;
                        // Clear Occupation:
                        //Layers->IS_AF[row][col] = 0 ; 
                        Layers->IS_AF[row][col] &= ~(1 << i) ;

                        // // Integration in Centroid Calculating Formula :
                        // numerator_y   += Layers->OFF[cur_window][row][col] * row ;
                        // denominator_y += Layers->OFF[cur_window][row][col] ;
                        // numerator_x   += Layers->OFF[cur_window][row][col] * col ;
                        // denominator_x += Layers->OFF[cur_window][row][col] ;
                    }
                }

                // Find Mass Center : 
                d = AF[i].Centroid[0] + AF_Radius ; // Update Mass Center by Inner AF.
                u = AF[i].Centroid[0] - AF_Radius ; // Update Mass Center by Inner AF.
                r = AF[i].Centroid[1] + AF_Radius ; // Update Mass Center by Inner AF.
                l = AF[i].Centroid[1] - AF_Radius ; // Update Mass Center by Inner AF.
                uint64_t numerator_y = 0 ;
                uint64_t denominator_y = 0 ;
                uint64_t numerator_x = 0 ;
                uint64_t denominator_x = 0 ;
                for(row = u; row < d; row ++ )
                {
                    if(row < 0 || row >= Height )
                        continue ; 
                    for(col = l; col < r; col ++ )
                    {
                        if(col < 0 || col >= Width )
                            continue ;
                        // Integration in Centroid Calculating Formula :
                        numerator_y   += Layers->OFF[cur_window][row][col] * row ;
                        denominator_y += Layers->OFF[cur_window][row][col] ;
                        numerator_x   += Layers->OFF[cur_window][row][col] * col ;
                        denominator_x += Layers->OFF[cur_window][row][col] ;
                    }
                } // Now we've got the mass center of inner AF.


                // Use Mass Centroid Calculation Formula to Find New AFC:  
                // centroid_x = numerator_x / denominator_x 
                AF[i].Centroid[0] = numerator_y / (denominator_y + 1) ;
                AF[i].Centroid[1] = numerator_x / (denominator_x + 1) ;
                printf("\nShifted AFC of AF[%d]: [%d, %d] ", i, AF[i].Centroid[0], AF[i].Centroid[1] ) ;
                // Now we've got AFC.

                // If AFC Go into Any Existing AFs: Drop Tt !
                if(Layers->IS_AF[AF[i].Centroid[0] ][AF[i].Centroid[1] ] == 1 ){
                    // Drop This AF: 
                    AF[i].LPLC2 = 0 ;
                    for(int j = 0; j < 4; j ++ )
                    {
                        AF[i].Quadrant[j] = 0 ;
                        AF[i].Output[j] = 0 ;
                    }
                    AF[i].Vacant_AF = 1 ;
                    printf("\nShifted AFC of AF[%d] goes into some existing AF! Drop it ! \n", i ) ;
                }else{
                    // Update AF Occupation: 
                    d = AF[i].Centroid[0] + AF_Radius_Exclusive ;
                    u = AF[i].Centroid[0] - AF_Radius_Exclusive ;
                    r = AF[i].Centroid[1] + AF_Radius_Exclusive ;
                    l = AF[i].Centroid[1] - AF_Radius_Exclusive ; // Using Updated AFC.
                    printf("\nRange: y: %d~%d x: %d~%d \n", u, d-1, l, r-1 ) ;
                    for(row = u; row < d; row ++ )
                    {
                        if(row < 0 || row >= Height )
                            continue ;
                        for(col = l; col < r; col ++ )
                        {
                            if(col < 0 || col >= Width )
                                continue ;
                            // Clear Occupation:
                            //Layers->IS_AF[row][col] = 1 ;
                            Layers->IS_AF[row][col] |= (1 << i) ; 

                        }
                    }

                }


                
            }

        } // Traversal AFs Over ...



        


    // } // End of Multiple-Attention Mechanism.


} // Pre-synaptic Calculation Over ...


uint16_t HRC(uint8_t here_cur, uint8_t far_cur, uint8_t here_pre, uint8_t far_pre, uint8_t bias)
{
    return (here_cur*far_pre - here_pre*far_cur) >> bias ;
}// deprecated. redundant founction call.


void LPLC2_T4_T5(LPLC2_pControlTypedef* hLPLC2)
{
    LPLC2struct_Params* Params = &(hLPLC2->Model->Params ) ;
    LPLC2struct_Layers* Layers = &(hLPLC2->Model->Layers ) ;
    // LPLC2struct_Results* Results = &(hLPLC2->Model->Results ) ;

    // Multiple-Attention Fields : 
    LPLC2struct_Results* AF = &(hLPLC2->Model->AF[0] ) ; // For Multiple-Attention Machanism


    // Params : 
    uint8_t step = Params->Step ;
    uint8_t d = Params->d ;
    uint8_t n = Params->Nc ;
    uint8_t delay = Params->Delay ;
    uint8_t bias = Params->Bias ;
    uint8_t AF_Radius = Params->AF_Radius ;


    //uint8_t cur = hLPLC2->currentDiffImage ;
    //uint8_t pre = !cur ;
    uint8_t cur = hLPLC2->cur ;
    uint8_t pre = (cur-delay) >= 0 ? (cur-delay) : ((cur-delay)+DELAY_WINDOW ) ;

    int16_t y, x, i, N ;


    // Local Motion :
    /*
    int8_t Right = 0 ;
    int8_t Left = 0 ;
    int8_t Down = 0 ;
    int8_t Up = 0 ;
    */
    int32_t Right = 0 ;
    int32_t Left = 0 ;
    int32_t Down = 0 ;
    int32_t Up = 0 ;

    // Results->Quadrant[0] = 0 ;
    // Results->Quadrant[1] = 0 ;
    // Results->Quadrant[2] = 0 ;
    // Results->Quadrant[3] = 0 ;


    // '+' shaped EMD in 4 directions(in EMD test mode ) : 
    // int32_t Horizontal = 0 ;
    // int32_t Vertical = 0 ;
    // int32_t R_sum = 0 ;
    // int32_t L_sum = 0 ;
    // int32_t D_sum = 0 ;
    // int32_t U_sum = 0 ;
    //    
    // int32_t Q1_b_r = 0 ;
    // int32_t Q1_u_l = 0 ;

    // 2025.04.07 Renyuan Removed for Memory Space: 
    // for(y = 0; y < Height; y ++ )
    // {
    //     for(x = 0; x < Width; x ++ )
    //     {
    //         Layers->T[y][x] = 0 ;
    //     }
    // }


/*
    if(hLPLC2->EMD_test_mode )
    {
        // Single Attention Machanism:
        uint8_t AF_y = Results->Centroid[0] ; // ordinate of AF center
        uint8_t AF_x = Results->Centroid[1] ; // abscissa of AF center

        // Test 4 directions in right bottom quadrant of Colias' view : [ vertical direction is reversed, While horizontal direction is normal.]

        // Quadrant 1 : | |1| upper right
        //              | | |

        // Colias view: | | |   [ vertical direction is reversed, While horizontal direction is normal.]
        //              | |1|
        for (y = n*d ; y < AF_y - n*d ; y += step )
        {
            for (x = AF_x + n*d ; x < AF_x + 36 - n*d ; x += step )
            {
                Right = Layers->ON[cur][y][x] * Layers->ON[pre][y][x - d]
                      -(Layers->ON[pre][y][x] * Layers->ON[cur][y][x - d]) >> bias
                      + Layers->OFF[cur][y][x] * Layers->OFF[pre][y][x - d]
                      -(Layers->OFF[pre][y][x] * Layers->OFF[cur][y][x - d]) >> bias;

                Left  = Layers->ON[cur][y][x] * Layers->ON[pre][y][x + d]
                      -(Layers->ON[pre][y][x] * Layers->ON[cur][y][x + d]) >> bias
                      + Layers->OFF[cur][y][x] * Layers->OFF[pre][y][x + d]
                      -(Layers->OFF[pre][y][x] * Layers->OFF[cur][y][x + d]) >> bias;
                      
                Down  = Layers->ON[cur][y][x] * Layers->ON[pre][y + d][x]
                      -(Layers->ON[pre][y][x] * Layers->ON[cur][y + d][x]) >> bias
                      + Layers->OFF[cur][y][x] * Layers->OFF[pre][y + d][x]
                      -(Layers->OFF[pre][y][x] * Layers->OFF[cur][y + d][x]) >> bias;
                      
                Up    = Layers->ON[cur][y][x] * Layers->ON[pre][y - d][x]
                      -(Layers->ON[pre][y][x] * Layers->ON[cur][y - d][x]) >> bias
                      + Layers->OFF[cur][y][x] * Layers->OFF[pre][y - d][x]
                      -(Layers->OFF[pre][y][x] * Layers->OFF[cur][y - d][x]) >> bias;


                R_sum += Right ;
                L_sum += Left ;
                Horizontal += (Right - Left) ;

                D_sum += Down ;
                U_sum += Up ;
                Vertical += (Down - Up) ;

                // test bottom right quadrant output :
                Q1_b_r += (int16_t)sqrt(Right * Right + Down * Down) ;
                Q1_u_l += (int16_t)sqrt(Left * Left + Up * Up) ;


                Layers->T[y][x] = (int16_t)sqrt(Right * Right + Up * Up);

                Results->Quadrant[0] += Layers->T[y][x];
            }
        }

        Results->R_sum = R_sum;
        Results->L_sum = L_sum;
        Results->D_sum = D_sum;
        Results->U_sum = U_sum;

        Results->HS = Horizontal ;
        Results->VS = Vertical ;


        // HS and VS Test : 
        
        //if((Results->R_sum < LPLC2.Params.T_cardinal) && (Results->R_sum > (-LPLC2.Params.T_cardinal)))
        //{
        //  Results->HS = 0 ;
        //  wb_led_set(2, 0); 
        //  wb_led_set(1, 0);
        //}else if(Results->R_sum >= LPLC2.Params.T_cardinal){
        //  // Rightward :
        //  wb_led_set(2, 0xff0000); 
        //  wb_led_set(1, 0x0000ff); // Rightward: LED2 red, LED1 blue
        //}else{
        //  // Leftward :
        //  wb_led_set(1, 0xff0000);
        //  wb_led_set(2, 0x0000ff); // Leftward
        //}
        

        // Right and Left Directional Selective Test :
        if(Results->L_sum < LPLC2.Params.T_cardinal)
        {
            Results->L_sum = 0 ;
            //wb_led_set(1, 0);
        }else{
            //wb_led_set(1, 0xff0000); // Leftward
        }

        //delay_ms(5); // to makesure LED_1 running 

        if(Results->R_sum < LPLC2.Params.T_cardinal)
        {
            Results->R_sum = 0 ;
            //wb_led_set(2, 0); 
        }else{
            //wb_led_set(2, 0xff0000);
        }

        // Down and Up Directional Selective Test : 
        // 
        // if(Results->D_sum < LPLC2.Params.T_cardinal)
        // {
        // Results->D_sum = 0 ;
        // wb_led_set(1, 0);
        // }else{
        // wb_led_set(1, 0xff0000); // Leftward
        // }
        // 
        // delay_ms(5); // to makesure LED_1 running 
        // 
        // if(Results->U_sum < LPLC2.Params.T_cardinal)
        // {
        // Results->U_sum = 0 ;
        // wb_led_set(2, 0); 
        // }else{
        // wb_led_set(2, 0xff0000);
        // }
        

        // test quadrant ourput :
        Results->Q1_b_r = Q1_b_r ;
        Results->Q1_u_l = Q1_u_l ;

        int stopdebug = 1 ;
        if (stopdebug == 0)
            stopdebug = 1 ;


        // Test '+' cross-line shaped EMD in 4 directions :

        // 
        // // Horizontal :
        // int i = 0 ;
        // for( x = (Width-Height)/2 + n*d ; x < Height - n*d ; x += step )
        // {
        // // Right: 
        // Layers->EMD[0][i] = HRC(Layers->ON [cur][AF_y][x], Layers->ON [cur][AF_y][x-step],
        // Layers->ON [pre][AF_y][x], Layers->ON [pre][AF_y][x-step], bias) // ON
        // + HRC(Layers->OFF[cur][AF_y][x], Layers->OFF[cur][AF_y][x-step],
        // Layers->OFF[pre][AF_y][x], Layers->OFF[pre][AF_y][x-step], bias) ; // OFF
        // 
        // // Left:
        // Layers->EMD[1][i] = HRC(Layers->ON [cur][AF_y][x], Layers->ON [cur][AF_y][x+step],
        // Layers->ON [pre][AF_y][x], Layers->ON [pre][AF_y][x+step], bias) // ON
        // + HRC(Layers->OFF[cur][AF_y][x], Layers->OFF[cur][AF_y][x+step],
        // Layers->OFF[pre][AF_y][x], Layers->OFF[pre][AF_y][x+step], bias) ; // OFF
        // 
        // R_sum+= Layers->EMD[0][i] ;
        // L_sum+= Layers->EMD[1][i] ;
        // Horizontal += (Layers->EMD[0][i] - Layers->EMD[1][i]) ;
        // 
        // i ++ ;
        // }
        // i = 0 ;
        // // Vertical :
        // for( y = 0 + n*d ; y < Height - n*d ; y += step )
        // {
        // // Down:
        // Layers->EMD[2][i] = HRC(Layers->ON [cur][y][AF_x], Layers->ON [cur][y+step][AF_x],
        // Layers->ON [pre][y][AF_x], Layers->ON [pre][y+step][AF_x], bias) // ON
        // + HRC(Layers->OFF[cur][y][AF_x], Layers->OFF[cur][y+step][AF_x],
        // Layers->OFF[pre][y][AF_x], Layers->OFF[pre][y+step][AF_x], bias) ; // OFF
        // // Up:
        // Layers->EMD[3][i] = HRC(Layers->ON [cur][y][AF_x], Layers->ON [cur][y-step][AF_x],
        // Layers->ON [pre][y][AF_x], Layers->ON [pre][y-step][AF_x], bias) // ON
        // + HRC(Layers->OFF[cur][y][AF_x], Layers->OFF[cur][y-step][AF_x],
        // Layers->OFF[pre][y][AF_x], Layers->OFF[pre][y-step][AF_x], bias) ; // OFF
        // 
        // D_sum+= Layers->EMD[2][i] ;
        // U_sum+= Layers->EMD[3][i] ;
        // Vertical += (Layers->EMD[2][i] - Layers->EMD[3][i]) ;
        // 
        // i ++ ;
        // }
        // 
        // Results->R_sum = R_sum;
        // Results->L_sum = L_sum;
        // Results->D_sum = D_sum;
        // Results->U_sum = U_sum;
        // 
        // Results->HS = Horizontal ;
        // Results->VS = Vertical ;
        

        return ;

    } // EMD Test Over ... 
*/    
    



    // T4 and T5 Directional Selective Neurons :

/*  // Single Attention Mechanism : 
    if(hLPLC2->Attention_Mechanism == 1 ) 
    {
        // Single Attention Machanism:
        uint8_t AF_y = Results->Centroid[0] ; // ordinate of AF center
        uint8_t AF_x = Results->Centroid[1] ; // abscissa of AF center

        // find LPLC2 Attention Field margin :
        int16_t AF_y_top    = AF_y - AF_Radius ;
        int16_t AF_y_bottom = AF_y + AF_Radius ;
        int16_t AF_x_left   = AF_x - AF_Radius ;
        int16_t AF_x_right  = AF_x + AF_Radius ; // AF is a square

        AF_y_top    = 36 - AF_Radius ;
        AF_y_bottom = 36 + AF_Radius ;
        AF_x_left   = 50 - AF_Radius ;
        AF_x_right  = 50 + AF_Radius ; // AF is a square
    
    
        int32_t Q1_R = 0 ;
        int32_t Q1_D = 0 ;
    
        int32_t Q2_L = 0 ;
        int32_t Q2_D = 0 ;
    
        int32_t Q3_L = 0 ;
        int32_t Q3_U = 0 ;
    
        int32_t Q4_R = 0 ;
        int32_t Q4_U = 0 ;
    
    
        // Quadrant 1 : | |1| upper right
        //              | | |
    
        // Colias view: | | | 
        //              | |1|

        int count = 0 ;
        for (y = AF_y_top + n * d ; y < AF_y ; y += step )
        {
            for (x = AF_x ; x < AF_x_right - n * d ; x += step )
            {
                Layers->T[y][x] = 0 ;
                for( i = 1; i <= n; i ++ )
                {
                    if(y < 0 || x >= Width )
                        continue ;
                    pre = (cur - delay*i ) >= 0 ? (cur - delay*i ) : ((cur - delay*i )+DELAY_WINDOW ) ;
        
                    Right = Layers->ON [cur][y][x + i*d] * Layers->ON [pre][y][x]
                          -(Layers->ON [pre][y][x + i*d] * Layers->ON [cur][y][x]) // >> bias; // PROBLEM CORE
                          + Layers->OFF[cur][y][x + i*d] * Layers->OFF[pre][y][x]
                          -(Layers->OFF[pre][y][x + i*d] * Layers->OFF[cur][y][x]) ;// >> bias;

                    // Down in Colias' view, up in original camera : 
                    Down = Layers->ON [cur][y - i*d][x] * Layers->ON [pre][y][x]
                         -(Layers->ON [pre][y - i*d][x] * Layers->ON [cur][y][x]) // >> bias
                         + Layers->OFF[cur][y - i*d][x] * Layers->OFF[pre][y][x]
                         -(Layers->OFF[pre][y - i*d][x] * Layers->OFF[cur][y][x]) ;// >> bias;
        
        
                    // Layers->T[y][x] = (uint32_t)sqrt(Right * Right + Up * Up) ;
                    // Results->Quadrant[0] += (float )Layers->T[y][x] ;
                    // Results->Quadrant[0] += (Right + Up) ;
                    Layers->T[y][x] += (Right + Down) ;
        
                    Q1_R += Right ;
                    Q1_D += Down ;
                }
            }
        }
    
        // Quadrant 2 : upper left |2| |
        //                         | | |
    
        // Colias view:            | | | 
        //                         |2| |
        for (y = AF_y_top + n * d ; y < AF_y ; y += step )
        {
            for (x = AF_x_left + n * d ; x < AF_x ; x += step )
            {
                Layers->T[y][x] = 0 ;
                for( i = 1; i <= n; i ++ )
                {
                    if(y < 0 || x >= Width )
                        continue ;
                    pre = (cur - delay*i ) >= 0 ? (cur - delay*i ) : ((cur - delay*i )+DELAY_WINDOW ) ;

                    Left = Layers->ON [cur][y][x - i*d] * Layers->ON [pre][y][x]
                         -(Layers->ON [pre][y][x - i*d] * Layers->ON [cur][y][x]) // >> bias
                         + Layers->OFF[cur][y][x - i*d] * Layers->OFF[pre][y][x]
                         -(Layers->OFF[pre][y][x - i*d] * Layers->OFF[cur][y][x]) ;// >> bias;

                    Down = Layers->ON [cur][y - i*d][x] * Layers->ON [pre][y][x]
                         -(Layers->ON [pre][y - i*d][x] * Layers->ON [cur][y][x]) // >> bias
                         + Layers->OFF[cur][y - i*d][x] * Layers->OFF[pre][y][x]
                         -(Layers->OFF[pre][y - i*d][x] * Layers->OFF[cur][y][x]) ;// >> bias;


                    // Layers->T[y][x] = (uint32_t)sqrt(Left * Left + Up * Up) ;
                    // Results->Quadrant[1] += Layers->T[y][x] ;
                    // Results->Quadrant[1] += (Left + Up) ;
                    Layers->T[y][x] += (Left + Down) ;

                    Q2_L += Left ;
                    Q2_D += Down ;
                }
            }
        }
    
        // Quadrant 3 :             | | |
        //              bottom left |3| |
    
        // Colias view:             |3| | 
        //                          | | |
        for (y = AF_y ; y < AF_y_bottom - n * d ; y += step )
        {
            for (x = AF_x_left + n * d ; x < AF_x ; x += step )
            {
                Layers->T[y][x] = 0 ;
                for( i = 1; i <= n; i ++ )
                {
                    if(y < 0 || x >= Width )
                        continue ;
                    pre = (cur - delay*i ) >= 0 ? (cur - delay*i ) : ((cur - delay*i )+DELAY_WINDOW ) ;

                    Left = Layers->ON [cur][y][x - i*d] * Layers->ON [pre][y][x]
                         -(Layers->ON [pre][y][x - i*d] * Layers->ON [cur][y][x]) // >> bias
                         + Layers->OFF[cur][y][x - i*d] * Layers->OFF[pre][y][x]
                         -(Layers->OFF[pre][y][x - i*d] * Layers->OFF[cur][y][x]) ;// >> bias;

                    Up   = Layers->ON [cur][y + i*d][x] * Layers->ON [pre][y][x]
                         -(Layers->ON [pre][y + i*d][x] * Layers->ON [cur][y][x]) // >> bias
                         + Layers->OFF[cur][y + i*d][x] * Layers->OFF[pre][y][x]
                         -(Layers->OFF[pre][y + i*d][x] * Layers->OFF[cur][y][x]) ;// >> bias;


                    // Layers->T[y][x] = (uint32_t)sqrt(Left * Left + Down * Down) ;
                    // Results->Quadrant[2] += Layers->T[y][x] ;
                    // Results->Quadrant[2] += (Left + Down) ;
                    Layers->T[y][x] += (Left + Up) ;

                    Q3_L += Left ;
                    Q3_U += Up ;
                }
            }
        }
    
        // Quadrant 4 : | | |
        //              | |4| bottom right
    
        // Colias view: | |4| 
        //              | | |
        for (y = AF_y ; y < AF_y_bottom - n * d ; y += step )
        {
            for (x = AF_x ; x < AF_x_right - n * d ; x += step )
            {
                Layers->T[y][x] = 0 ;
                for( i = 1; i <= n; i ++ )
                {
                    if(y < 0 || x >= Width )
                        continue ;
                    pre = (cur - delay*i ) >= 0 ? (cur - delay*i ) : ((cur - delay*i )+DELAY_WINDOW ) ;
    
                    Right = Layers->ON [cur][y][x + i*d] * Layers->ON [pre][y][x]
                          -(Layers->ON [pre][y][x + i*d] * Layers->ON [cur][y][x]) // >> bias
                          + Layers->OFF[cur][y][x + i*d] * Layers->OFF[pre][y][x]
                          -(Layers->OFF[pre][y][x + i*d] * Layers->OFF[cur][y][x]) ;// >> bias;
    
                    Up    = Layers->ON [cur][y + i*d][x] * Layers->ON [pre][y][x]
                          -(Layers->ON [pre][y + i*d][x] * Layers->ON [cur][y][x]) // >> bias
                          + Layers->OFF[cur][y + i*d][x] * Layers->OFF[pre][y][x]
                          -(Layers->OFF[pre][y + i*d][x] * Layers->OFF[cur][y][x]) ;// >> bias;
    
    
                    // Layers->T[y][x] = (uint32_t)sqrt(Right * Right + Down * Down);
                    // Results->Quadrant[3] += Layers->T[y][x];
                    // Results->Quadrant[3] += (Right + Down) ;
                    Layers->T[y][x] += (Right + Up) ;
    
                    Q4_R += Right ;
                    Q4_U += Up ;
                }
            }
        }
    
        // For each quadrant, we have 2 preferred directions.
        // Now we filt the essential one using 'min()' function.
        
        // Results->Quadrant[0] = min(R_sum, U_sum) ;
        // Results->Quadrant[1] = min(L_sum, U_sum) ;
        // Results->Quadrant[2] = min(L_sum, D_sum) ;
        // Results->Quadrant[3] = min(R_sum, D_sum) ;
        
        Results->Q1_R = Q1_R > 0 ? Q1_R : 0 ;
        Results->Q1_D = Q1_D > 0 ? Q1_D : 0 ;
    
        Results->Q2_L = Q2_L > 0 ? Q2_L : 0 ;
        Results->Q2_D = Q2_D > 0 ? Q2_D : 0 ;
    
        Results->Q3_L = Q3_L > 0 ? Q3_L : 0 ;
        Results->Q3_U = Q3_U > 0 ? Q3_U : 0 ;
    
        Results->Q4_R = Q4_R > 0 ? Q4_R : 0 ;
        Results->Q4_U = Q4_U > 0 ? Q4_U : 0 ;
    
        Results->Quadrant[0] = Q1_R > Q1_D ? Q1_R : Q1_D ;
        Results->Quadrant[1] = Q2_L > Q2_D ? Q2_L : Q2_D ;
        Results->Quadrant[2] = Q3_L > Q3_U ? Q3_L : Q3_U ;
        Results->Quadrant[3] = Q4_R > Q4_U ? Q4_R : Q4_U ;
    
        
        printf("\nAF Centroid : [ (y %d), (x %d) ] \n", Results->Centroid[0], Results->Centroid[1] );
        printf("   [%d] \n", AF_y_top) ;
        printf("[%d]  [%d] <-- AF \n", AF_x_left, AF_x_right) ;
        printf("   [%d] \n", AF_y_bottom) ;
    
    
        printf("\n[ Q2_L: %d, Q2_D: %d ] | [ Q1_R: %d, Q1_D: %d ] \n", Results->Q2_L, Results->Q2_D, Results->Q1_R, Results->Q1_D ) ;
        printf("---------------------------------------------------------------") ;
        printf("\n[ Q3_L: %d, Q3_U: %d ] | [ Q4_R: %d, Q4_U: %d ] \n", Results->Q3_L, Results->Q3_U, Results->Q4_R, Results->Q4_U ) ;
    
        printf("\nQ[0]: %d, Q[1]: %d, Q[2]: %d, Q[3]: %d \n", Results->Quadrant[0],Results->Quadrant[1],Results->Quadrant[2],Results->Quadrant[3] ) ;
        
    
    
    }else if(hLPLC2->Attention_Mechanism == 2 ){ 
*/

    // Multiple-Attention Mechanism : 

    // Traversal current existing AFs, and calculate local directional movement information for them respectively.
    for(N = 0; N < NUM_AF; N ++ ) 
    {
        // Reset : 
        AF[N].Quadrant[0] = 0 ;
        AF[N].Quadrant[1] = 0 ;
        AF[N].Quadrant[2] = 0 ;
        AF[N].Quadrant[3] = 0 ;

        // Multi-AF Center :
        uint8_t AF_y = AF[N].Centroid[0] ; // ordinate of AF center
        uint8_t AF_x = AF[N].Centroid[1] ; // abscissa of AF center

        // Find LPLC2 Attention Field's margin :
        int16_t AF_y_top    = AF[N].Centroid[0] - AF_Radius ;
        int16_t AF_y_bottom = AF[N].Centroid[0] + AF_Radius ;
        int16_t AF_x_left   = AF[N].Centroid[1] - AF_Radius ;
        int16_t AF_x_right  = AF[N].Centroid[1] + AF_Radius ; // AF is a square


        // 2025.04.07 Renyuan Removed for Memory Space: 
        // // Integration for 4 carinal directions : 
        // int32_t Q1_R = 0 ;
        // int32_t Q1_D = 0 ;
        // 
        // int32_t Q2_L = 0 ;
        // int32_t Q2_D = 0 ;
        // 
        // int32_t Q3_L = 0 ;
        // int32_t Q3_U = 0 ;
        // 
        // int32_t Q4_R = 0 ;
        // int32_t Q4_U = 0 ;


        // Quadrant 1 : | |1| upper right
        //              | | |

        // Colias view: | | | 
        //              | |1|
        int count = 0 ;
        //printf("\n%d~%d; %d~%d\n",AF_y_top, AF_y_bottom, AF_x_left, AF_x_right ) ;
        for (y = AF_y_top + n * d ; y < AF_y ; y += step )
        {
            for (x = AF_x ; x < AF_x_right - n * d ; x += step )
            {
                //Layers->T[y][x] = 0 ;
                for( i = 1; i <= n; i ++ )
                {
                    if(y < 0 || x >= Width )
                        continue ;
                    pre = (cur - delay*i ) >= 0 ? (cur - delay*i ) : ((cur - delay*i )+DELAY_WINDOW ) ;

                    Right = Layers->ON [cur][y][x + i*d] * Layers->ON [pre][y][x]
                          -(Layers->ON [pre][y][x + i*d] * Layers->ON [cur][y][x]) // >> bias; // PROBLEM CORE
                          + Layers->OFF[cur][y][x + i*d] * Layers->OFF[pre][y][x]
                          -(Layers->OFF[pre][y][x + i*d] * Layers->OFF[cur][y][x]) ;// >> bias;

                    // Down in Colias' view, up in original camera : 
                    Down = Layers->ON [cur][y - i*d][x] * Layers->ON [pre][y][x]
                         -(Layers->ON [pre][y - i*d][x] * Layers->ON [cur][y][x]) // >> bias
                         + Layers->OFF[cur][y - i*d][x] * Layers->OFF[pre][y][x]
                         -(Layers->OFF[pre][y - i*d][x] * Layers->OFF[cur][y][x]) ;// >> bias;
                    //printf("%d ", Down ) ;

                    // Layers->T[y][x] = (uint32_t)sqrt(Right * Right + Up * Up) ;
                    // Results->Quadrant[0] += (float )Layers->T[y][x] ;
                    AF[N].Quadrant[0] += (Right + Up) ;
                    
                    // Layers->T[y][x] += (Right + Down) ;
                    // Q1_R += Right ;
                    // Q1_D += Down ;
                }
            }
        }

        // Quadrant 2 : upper left |2| |
        //                         | | |

        // Colias view:            | | | 
        //                         |2| |
        for (y = AF_y_top + n * d ; y < AF_y ; y += step )
        {
            for (x = AF_x_left + n * d ; x < AF_x ; x += step )
            {
                //Layers->T[y][x] = 0 ;
                for( i = 1; i <= n; i ++ )
                {
                    if(y < 0 || x >= Width )
                        continue ;
                    pre = (cur - delay*i ) >= 0 ? (cur - delay*i ) : ((cur - delay*i )+DELAY_WINDOW ) ;

                    Left = Layers->ON [cur][y][x - i*d] * Layers->ON [pre][y][x]
                         -(Layers->ON [pre][y][x - i*d] * Layers->ON [cur][y][x]) // >> bias
                         + Layers->OFF[cur][y][x - i*d] * Layers->OFF[pre][y][x]
                         -(Layers->OFF[pre][y][x - i*d] * Layers->OFF[cur][y][x]) ;// >> bias;

                    Down = Layers->ON [cur][y - i*d][x] * Layers->ON [pre][y][x]
                         -(Layers->ON [pre][y - i*d][x] * Layers->ON [cur][y][x]) // >> bias
                         + Layers->OFF[cur][y - i*d][x] * Layers->OFF[pre][y][x]
                         -(Layers->OFF[pre][y - i*d][x] * Layers->OFF[cur][y][x]) ;// >> bias;


                    // Layers->T[y][x] = (uint32_t)sqrt(Left * Left + Up * Up) ;
                    // Results->Quadrant[1] += Layers->T[y][x] ;
                    AF[N].Quadrant[1] += (Left + Up) ;

                    // Layers->T[y][x] += (Left + Down) ;
                    // Q2_L += Left ;
                    // Q2_D += Down ;
                }
            }
        }

        // Quadrant 3 :             | | |
        //              bottom left |3| |

        // Colias view:             |3| | 
        //                          | | |
        for (y = AF_y ; y < AF_y_bottom - n * d ; y += step )
        {
            for (x = AF_x_left + n * d ; x < AF_x ; x += step )
            {
                //Layers->T[y][x] = 0 ;
                for( i = 1; i <= n; i ++ )
                {
                    if(y < 0 || x >= Width )
                        continue ;
                    pre = (cur - delay*i ) >= 0 ? (cur - delay*i ) : ((cur - delay*i )+DELAY_WINDOW ) ;

                    Left = Layers->ON [cur][y][x - i*d] * Layers->ON [pre][y][x]
                         -(Layers->ON [pre][y][x - i*d] * Layers->ON [cur][y][x]) // >> bias
                         + Layers->OFF[cur][y][x - i*d] * Layers->OFF[pre][y][x]
                         -(Layers->OFF[pre][y][x - i*d] * Layers->OFF[cur][y][x]) ;// >> bias;

                    Up   = Layers->ON [cur][y + i*d][x] * Layers->ON [pre][y][x]
                         -(Layers->ON [pre][y + i*d][x] * Layers->ON [cur][y][x]) // >> bias
                         + Layers->OFF[cur][y + i*d][x] * Layers->OFF[pre][y][x]
                         -(Layers->OFF[pre][y + i*d][x] * Layers->OFF[cur][y][x]) ;// >> bias;


                    // Layers->T[y][x] = (uint32_t)sqrt(Left * Left + Down * Down) ;
                    // Results->Quadrant[2] += Layers->T[y][x] ;
                    AF[N].Quadrant[2] += (Left + Down) ;

                    // Layers->T[y][x] += (Left + Up) ;
                    // Q3_L += Left ;
                    // Q3_U += Up ;
                }
            }
        }

        // Quadrant 4 : | | |
        //              | |4| bottom right

        // Colias view: | |4| 
        //              | | |
        for (y = AF_y ; y < AF_y_bottom - n * d ; y += step )
        {
            for (x = AF_x ; x < AF_x_right - n * d ; x += step )
            {
                //Layers->T[y][x] = 0 ;
                for( i = 1; i <= n; i ++ )
                {
                    if(y < 0 || x >= Width )
                        continue ;
                    pre = (cur - delay*i ) >= 0 ? (cur - delay*i ) : ((cur - delay*i )+DELAY_WINDOW ) ;

                    Right = Layers->ON [cur][y][x + i*d] * Layers->ON [pre][y][x]
                          -(Layers->ON [pre][y][x + i*d] * Layers->ON [cur][y][x]) // >> bias
                          + Layers->OFF[cur][y][x + i*d] * Layers->OFF[pre][y][x]
                          -(Layers->OFF[pre][y][x + i*d] * Layers->OFF[cur][y][x]) ;// >> bias;

                    Up    = Layers->ON [cur][y + i*d][x] * Layers->ON [pre][y][x]
                          -(Layers->ON [pre][y + i*d][x] * Layers->ON [cur][y][x]) // >> bias
                          + Layers->OFF[cur][y + i*d][x] * Layers->OFF[pre][y][x]
                          -(Layers->OFF[pre][y + i*d][x] * Layers->OFF[cur][y][x]) ;// >> bias;


                    // Layers->T[y][x] = (uint32_t)sqrt(Right * Right + Down * Down);
                    // Results->Quadrant[3] += Layers->T[y][x];
                    AF[N].Quadrant[3] += (Right + Down) ;

                    // Layers->T[y][x] += (Right + Up) ;
                    // Q4_R += Right ;
                    // Q4_U += Up ;
                }
            }
        }

        // For each quadrant, we have 2 preferred directions.
        // Now we filt the essential one using 'min()' function.
        /*
        Results->Quadrant[0] = min(R_sum, U_sum) ;
        Results->Quadrant[1] = min(L_sum, U_sum) ;
        Results->Quadrant[2] = min(L_sum, D_sum) ;
        Results->Quadrant[3] = min(R_sum, D_sum) ;
        */
        /*
        AF[N].Q1_R = Q1_R > 0 ? Q1_R : 0 ;
        AF[N].Q1_D = Q1_D > 0 ? Q1_D : 0 ;
           
        AF[N].Q2_L = Q2_L > 0 ? Q2_L : 0 ;
        AF[N].Q2_D = Q2_D > 0 ? Q2_D : 0 ;
           
        AF[N].Q3_L = Q3_L > 0 ? Q3_L : 0 ;
        AF[N].Q3_U = Q3_U > 0 ? Q3_U : 0 ;
           
        AF[N].Q4_R = Q4_R > 0 ? Q4_R : 0 ;
        AF[N].Q4_U = Q4_U > 0 ? Q4_U : 0 ;
        */


        // 2025.04.07 Renyuan Removed for Memory Space: 
        // AF[N].Q1_R = Q1_R ;
        // AF[N].Q1_D = Q1_D ;
        //                   
        // AF[N].Q2_L = Q2_L ;
        // AF[N].Q2_D = Q2_D ;
        //                   
        // AF[N].Q3_L = Q3_L ;
        // AF[N].Q3_U = Q3_U ;
        //                   
        // AF[N].Q4_R = Q4_R ;
        // AF[N].Q4_U = Q4_U ;
        // 
        // AF[N].Quadrant[0] = Q1_R > Q1_D ? Q1_R : Q1_D ;
        // AF[N].Quadrant[1] = Q2_L > Q2_D ? Q2_L : Q2_D ;
        // AF[N].Quadrant[2] = Q3_L > Q3_U ? Q3_L : Q3_U ;
        // AF[N].Quadrant[3] = Q4_R > Q4_U ? Q4_R : Q4_U ;



    
    } // Traversal AFs Over ... 


/*  
    return ; // Multi-Attention Mechanism AFs Calculation Over, return.
    
    }else{
        return ;
    } // Local Information Calculating Over ...
*/

} // T4/T5 Calculation Over ... 


void LPLC2_Projection(LPLC2_pControlTypedef* hLPLC2)
{
    // Pointer passing: 
    LPLC2struct_Params* Params = &(hLPLC2->Model->Params) ;
    LPLC2struct_Layers* Layers = &(hLPLC2->Model->Layers) ;

    // Parameters : 
    int64_t T_cardinal = Params->T_cardinal ;
    int64_t T_LPLC2 = Params->T_LPLC2 ;
    // LIF Parameters : 
    float tau_m = Params->tau_m ;
    int8_t V_rest = Params->V_rest ;
    int8_t V_threshold = Params->V_threshold ;
    int8_t V_reset = Params->V_reset ;
    int8_t V_spike = Params->V_spike ;
    float R = Params->R ;

    uint8_t n, i ; // n for NUM_AF, i for other iterations.



/*
    // Single Attention Mechanism : 
    if(hLPLC2->Attention_Mechanism == 1 )
    {
        LPLC2struct_Results* Results = &(hLPLC2->Model->Results) ;
        int32_t *Q = Results->Quadrant ;

        // Threshold for 4 direction represented by 4 quadrants :
        uint8_t expansion_edge_count = 0 ;
        for(i = 0 ; i < 4 ; i ++ )
        {
            if(Q[i] < Params->T_cardinal )
            {
                Q[i] = 0 ;
                continue ;
            }
            expansion_edge_count ++ ;
        }

        // LPLC2 Projection :
        if(expansion_edge_count < 2  ){ // Q[0] * Q[1] * Q[2] * Q[3] == 0 
            Results->Output[*(hLPLC2->hFrameCount)%4] = 0 ;
        }else{
            Results->Output[*(hLPLC2->hFrameCount)%4] = Q[0] + Q[1] + Q[2] + Q[3] ;
        }


        // LPLC2 average output in a period : 
        int32_t LPLC2_pre = Results->LPLC2 ;
        Results->LPLC2 = 0 ; // reset for '+=' operation 
        for(i = 0 ; i < 4 ; i ++ )
        {
            Results->LPLC2 += Results->Output[i] ;
        }
        Results->LPLC2 = Results->LPLC2 >> 2 ;

        double v = (Results->LPLC2 - LPLC2_pre ) / 0.033 ; // (ms)
        Results->I = Results->LPLC2 + v ; 
        Results->I = Results->I > 0 ? Results->I : 0 ;
        // printf("\nI: [%f]\n", I) ;

        // Leaky Integrate-and-Fire : 
        float* OUTPUT = &Results->OUTPUT[0] ;
        float k1, k2, k3, k4, dt = 0.033 / 30 ;

        for(i = 0; i < 30; i ++ )
        {
            int pre = i-1 < 0 ? i-1 + 30 : i-1 ;
            if(OUTPUT[pre] == V_spike ){
                OUTPUT[i] = V_reset ; // Hyperpolarization after a spike.
                continue ;
            }

            k1 = -(OUTPUT[i]           - V_rest ) / tau_m + (Results->I*R ) / tau_m ;
            k2 = -(OUTPUT[i] + dt*k1/2 - V_rest ) / tau_m + (Results->I*R ) / tau_m ;
            k3 = -(OUTPUT[i] + dt*k2/2 - V_rest ) / tau_m + (Results->I*R ) / tau_m ;
            k4 = -(OUTPUT[i] + dt*k3   - V_rest ) / tau_m + (Results->I*R ) / tau_m ;

            OUTPUT[i] = OUTPUT[pre] + (k1 + 2*k2 + 2*k3 + k4)*dt / 6.0;

            if(OUTPUT[i] > V_threshold )
                OUTPUT[i] = V_spike ; // Spiking !
        }
        // *OUTPUT += 0.033 * (-(*OUTPUT - V_rest ) ) / tau_m + I*R  ; // Eular method for LIF.

        printf("\nLPLC2 Current Output: %d; LIF Output: %f \n", Results->LPLC2, *OUTPUT );




        // Spiking : 
        if(Results->LPLC2 > T_LPLC2 ){
            Results->Spike ++ ;
        }else{
            Results->Spike = 0 ;
        }

        // Collision Detection :
        if(Results->Spike >= Params->n ){
            Results->Collision = 1 ;
            //if(!AC1.en_motion )
            //    wb_led_set(1, 0xff0000 ) ;
        }else{
            Results->Collision = 0 ;
            //if(!AC1.en_motion )
            //    wb_led_set(1, 0x000000 ) ;
        }



        // Use Results->Collision to Control Motion.
        printf("\nCollision : %d\n",  Results->Collision ) ;



    }else if(hLPLC2->Attention_Mechanism == 2 ){
*/

        // Multiple-Attention Fields Integration : 

        LPLC2struct_Results* AF = &(hLPLC2->Model->AF[0] ) ; // For Multiple-Attention Machanism
        
        // Traversal Existing AFs and integrate information to LPLC2 located there: 
        for(n = 0; n < NUM_AF; n ++ )
        {
            if(AF[n].Vacant_AF == 1 )
                continue ;
            
            int64_t *Q = &AF[n].Quadrant[0] ; 
            // Threshold for 4 direction represented by 4 quadrants :
            uint8_t expansion_edge_count = 0 ;
            for(i = 0 ; i < 4 ; i ++ )
            {
                //printf("\nQuadrant[%d] : %d ", i, Q[i] ) ; // Right Output.

                if(Q[i] < Params->T_cardinal )
                {
                    Q[i] = 0 ;
                    continue ;
                }
                expansion_edge_count ++ ;
            }

            // LPLC2 Projection :
            if(expansion_edge_count < 2 /*Q[0] * Q[1] * Q[2] * Q[3] == 0*/ ){
                AF[n].Output[*(hLPLC2->hFrameCount)%4] = 0 ;
            }else{
                AF[n].Output[*(hLPLC2->hFrameCount)%4] = Q[0] + Q[1] + Q[2] + Q[3] ;
            }
            //printf("\nAF[%d] : %d ", n, AF[n].Output[*(hLPLC2->hFrameCount)%4] ) ; // Right Output.


            // LPLC2 average output in a period : 
            int64_t LPLC2_pre = AF[n].LPLC2 ;
            AF[n].LPLC2 = 0 ; // reset for '+=' operation 
            for(i = 0 ; i < 4 ; i ++ )
            {
                AF[n].LPLC2 += AF[n].Output[i] ;
            }
            AF[n].LPLC2 = AF[n].LPLC2 >> 2 ;

            // Time Delay : 
            AF[n].LPLC2 = (AF[n].LPLC2 >> 1) + (LPLC2_pre >> 1) ; // decay parameter: 0.5 (>> 1 ) // Bracket outside the '>' is necessary !
            //printf("\nAF[%d](Recent 4 Average ) : %d ", n, AF[n].LPLC2 ) ; // Right Output.

            AF[n].LPLC2 = (AF[n].LPLC2 > LPLC2.Params.max_input) ? LPLC2.Params.max_input : AF[n].LPLC2 ;


            double v = (AF[n].LPLC2 - LPLC2_pre ) / 0.033 /*(ms)*/ ;
            //printf("\nv : %f ", v ) ; // Right Output.

            AF[n].I = AF[n].LPLC2 + v ; 
            //printf("\nI : %f ", AF[n].I ) ; // Right Output.

            AF[n].I = AF[n].I > 0 ? AF[n].I : 0 ;
            //printf("\nAF[%d].I, (%d, %d) : %f\n", n, AF[n].Centroid[0], AF[n].Centroid[1], AF[n].I ) ;

            // Leaky Integrate-and-Fire : 
            float* OUTPUT = &AF[n].OUTPUT[0] ;
            float k1, k2, k3, k4, dt = 0.033 / 30 ;

            for(i = 0; i < 30; i ++ )
            {
                int pre = i-1 < 0 ? i-1 + 30 : i-1 ;
                if(OUTPUT[pre] == V_spike ){
                    OUTPUT[i] = V_reset ; // Hyperpolarization after a spike.
                    continue ;
                }

                k1 = -(OUTPUT[i]           - V_rest ) / tau_m + (AF[n].I*R ) / tau_m ;
                k2 = -(OUTPUT[i] + dt*k1/2 - V_rest ) / tau_m + (AF[n].I*R ) / tau_m ;
                k3 = -(OUTPUT[i] + dt*k2/2 - V_rest ) / tau_m + (AF[n].I*R ) / tau_m ;
                k4 = -(OUTPUT[i] + dt*k3   - V_rest ) / tau_m + (AF[n].I*R ) / tau_m ;

                OUTPUT[i] = OUTPUT[pre] + (k1 + 2*k2 + 2*k3 + k4)*dt / 6.0;

                if(OUTPUT[i] > V_threshold )
                    OUTPUT[i] = V_spike ; // Spiking !
            }
            // *OUTPUT += 0.033 * (-(*OUTPUT - V_rest ) ) / tau_m + I*R  ; // Eular method for LIF.


        }// Traversal AFs Over ... 


        for(n = 0; n < NUM_AF; n ++ )
        {
            if(AF[n].Vacant_AF == 1 )
                continue ;
            // Spiking : 
            if(AF[n].LPLC2 > T_LPLC2 ){
                AF[n].Spike ++ ;
            }else{
                AF[n].Spike = 0 ;
            }
            
            // Collision Detection :
            if(AF[n].Spike >= Params->n ){
                AF[n].Collision = 1 ;
                //if(!AC1.en_motion )
                //    wb_led_set(1, 0xff0000 ) ;
            }else{
                AF[n].Collision = 0 ;
                //if(!AC1.en_motion )
                //    wb_led_set(1, 0x000000 ) ;
            }
            // Use Results->Collision to Control Motion.
            //printf("\nAF[%d] Collision : %d\n", n, AF[n].Collision ) ;




            // Monitor This AF: 
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            //printf("\n\nAFs after current frame calculation : \n") ;
            printf("\n\n______________AF %d: Active: %d Centroid: [%d, %d]______________ ",n , !AF[n].Vacant_AF, AF[n].Centroid[0], AF[n].Centroid[1] ) ;

            //printf("   [%d] \n", AF_y_top) ;
            //printf("[%d]  [%d] <-- AF \n", AF_x_left, AF_x_right) ;
            //printf("   [%d] \n", AF_y_bottom) ;
            //    printf("\n[ Q2_L: %d, Q2_D: %d ] | [ Q1_R: %d, Q1_D: %d ] \n", AF[N].Q2_L, AF[N].Q2_D, AF[N].Q1_R, AF[N].Q1_D ) ;
            //    printf("------------------[%d,%d]----------------------", AF[N].Centroid[0], AF[N].Centroid[1] ) ;
            //    printf("\n[ Q3_L: %d, Q3_U: %d ] | [ Q4_R: %d, Q4_U: %d ] \n", AF[N].Q3_L, AF[N].Q3_U, AF[N].Q4_R, AF[N].Q4_U ) ;

            //    printf("\nQ[0]: %d, Q[1]: %d, Q[2]: %d, Q[3]: %d ", AF[N].Quadrant[0], AF[N].Quadrant[1], AF[N].Quadrant[2], AF[N].Quadrant[3] ) ;
            // 
            // 
            printf("\n %d  Current LPLC2 Output, cur LPLC2: %d ", AF[n].Output[(*(hLPLC2->hFrameCount)-1)%4], (*(hLPLC2->hFrameCount)-1)%4 ) ;
            printf("\n %d  Recent LPLC2 Output ", AF[n].LPLC2 ) ;
            //    printf("\n %f  I ", AF[N].I ) ;
            //    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            printf("\n%d Collision ", AF[n].Collision );
            printf("\n______________________________________________________________");


        }

        // Monitor Visuomotor : 
        //printf("\n\n\n%f  Visuomotor Direction ", hLPLC2->Model->Visuomotor.Escape_Direction ) ;
        //printf("\n%f  Visuomotor Speed ", hLPLC2->Model->Visuomotor.Escape_Speed ) ;



/*
    }else{
        return ;
    }
*/




} // LPLC2 AFs Integration Over ... 



void VisionToMotor(LPLC2_pControlTypedef* hLPLC2 )
{
    // Pointer Passing :
    LPLC2struct_Params* Params = &(hLPLC2->Model->Params ) ;
    LPLC2struct_Layers* Layers = &(hLPLC2->Model->Layers ) ;
    LPLC2struct_Results* AF = &(hLPLC2->Model->AF[0] ) ; // For Multiple-Attention Machanism
    LPLC2struct_Visuomotor* Visuomotor = &(hLPLC2->Model->Visuomotor ) ;
    
    // Reset : 
    Visuomotor->Escape_Direction = 0 ;
    Visuomotor->Escape_Speed = 0 ;

    uint64_t numerator_x = 0 ;
    uint64_t denominator_x = 0 ;

    uint64_t GlobalStrength = 0 ; // Escape Speed by Global AFs' Strength

    // Search Colliding AFs : 
    for(int n = 0; n < NUM_AF; n ++ )
    {
        if(AF[n].Vacant_AF == 1 )
            continue ;

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
            printf("\n\nVISUOMOTOR OUTPUT : ");
            printf("\n%f Mass Centroid_x of AFs ", Centroid_x );

            // Threat Direction : 
            Visuomotor->Escape_Direction = (Centroid_x - Image_Width/2) * 70 / 99 ; // Centroid_x - 49, 0 ~ 98 - 49 = -49 ~ 49 (degree )
            printf("\n%f Threat Direction", Visuomotor->Escape_Direction );


            // Escape Direction : 
            if(Visuomotor->Escape_Direction > 0 ){
                Visuomotor->Escape_Direction -= 90 ; // Threat from right side, turn left.
            }else if(Visuomotor->Escape_Direction < 0 ){
                Visuomotor->Escape_Direction += 90 ; // Threat from left side, turn right.
            }
            printf("\n%f Escape Direction ", Visuomotor->Escape_Direction );


            // Escape Speed :
            Visuomotor->Escape_Speed = 1.0 / (1.0 + exp( - 1.0*((float )GlobalStrength / 1000000*NUM_AF) ) ) * 3.3 ; // MC1.Vmax = 1 * 3.3 = 3.3
            printf("\n%llu GlobalStrength input as speed", GlobalStrength );
            printf("\n%f Escape Speed ", Visuomotor->Escape_Speed );


            return ; // Find any collision, return after integrating global information.
        }
    }


}



