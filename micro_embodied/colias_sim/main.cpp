//#include "main.h"
//#include "coliasapi.h"

#include <opencv2/opencv.hpp>

#include <stdio.h>
#include <string.h>
#include <math.h>
//#include <arm_math.h>


#include <LPLC2_model.h>
#include <coliasSense_LPLC2.h>
#include <stdint.h>


uint64_t MAX_LPLC2 = 0 ;



#define HEIGHT 72
#define WIDTH 99
#define MAX_FRAMES 500  // Video MAX frame
int FRAMES = 0 ; // Video real frame


// Video 
uint16_t frames[MAX_FRAMES][HEIGHT][WIDTH];


void aLPLC2_VisualModel(void) ;
// Founctions for LPLC2 Calculating : 
void LPLC2_Init(LPLC2_pControlTypedef* hLPLC2);
void LPLC2_PreSynaptic(LPLC2_pControlTypedef* hLPLC2);
void LPLC2_T4_T5(LPLC2_pControlTypedef* hLPLC2);
void LPLC2_Projection(LPLC2_pControlTypedef* hLPLC2);

// Visualizasion : 
void visualize_layer_ON(int time, int frame );
void visualize_layer_OFF(int time, int frame );
void visualize_layer_T(int frame);

void write_txt_multiple_AFs();
void write_txt();
void process_video(const char* filename);


void write_txt_multiple_AFs(void) 
{
    const char *file_path = "C:\\A_RESEARCH\\Off_Line\\C\\LPLC2_Colias\\OUTPUT\\OUTPUT2.txt";

    FILE *fp;
    if (fopen_s(&fp, file_path, "a") != 0) 
    {
        printf("cannot open file: %s ! \n", file_path);
        return;
    }

    //fprintf(fp, "%.6f\n", LPLC2.Results.OUTPUT);

    for(int i = 0; i < 30; i ++ )
    {
        fprintf(fp, "%.6f\n", LPLC2.AF[0].OUTPUT[i] ) ;
        printf("\nData %.6f written to OUTPUT2.txt\n", LPLC2.AF[0].OUTPUT[i] );
    }

    fclose(fp);

}

/*
void write_txt(void) 
{
    const char *file_path = "C:\\A_RESEARCH\\Off_Line\\C\\LPLC2_Colias\\OUTPUT\\OUTPUT.txt";

    FILE *fp;
    if (fopen_s(&fp, file_path, "a") != 0) 
    {
        printf("cannot open file: %s ! \n", file_path);
        return;
    }

    //fprintf(fp, "%.6f\n", LPLC2.Results.OUTPUT);

    for(int i = 0; i < 30; i ++ )
    {
        fprintf(fp, "%.6f\n", LPLC2.Results.OUTPUT[i] ) ;
        printf("\nData %.6f written to OUTPUT.txt\n", LPLC2.Results.OUTPUT[i] );
    }

    fclose(fp);

}
*/

void process_video(const char* filename) 
{
    // Open video file
    cv::VideoCapture video(filename);
    if (!video.isOpened()) {
        printf("Error: Could not open video.\n");
        return;
    }

    // Get video properties (width, height, and frame count)
    int original_width = video.get(cv::CAP_PROP_FRAME_WIDTH);  // Get width of the original video
    int original_height = video.get(cv::CAP_PROP_FRAME_HEIGHT);  // Get height of the original video
    int frame_count = video.get(cv::CAP_PROP_FRAME_COUNT);  // Get total number of frames

    FRAMES = frame_count ; // 

    // Print video information
    printf("\n\n\n\n\nOriginal Video Size: %d x %d\n", original_width, original_height);
    printf("Total Frames: %d\n\n", frame_count);

    cv::Mat frame;
    int frame_index = 0;
    while (video.read(frame) && frame_index < MAX_FRAMES) 
    {
        // Resize the frame to 72x99
        cv::Mat resized_frame;
        cv::resize(frame, resized_frame, cv::Size(WIDTH, HEIGHT));

        // Convert the image to grayscale
        cv::Mat gray_frame;
        cv::cvtColor(resized_frame, gray_frame, cv::COLOR_BGR2GRAY);

        // Store the grayscale value in the uint16_t's high 8 bits to simulate Colias' camera.
        //if(frame_index % 2 == 0 )
        {
            for (int row = 0; row < HEIGHT; row++) 
            {
                for (int col = 0; col < WIDTH; col++) 
                {
                    // core
                    uint8_t gray_value = gray_frame.at<uchar>(row, col);  // Get grayscale value
                    frames[frame_index][row][col] = (gray_value << 8);  // Store in the high 8 bits
                }
            }
        }
        

        frame_index++;
    }

    video.release();
    printf("Video processed and stored in frames array.\n\n\n");
}


void visualize_layer_ON(int time, int frame )
{
    cv::Mat image(HEIGHT, WIDTH, CV_8UC1);


    for (int row = 0; row < HEIGHT; row++) 
    {
        for (int col = 0; col < WIDTH; col++) 
        {
            int value = LPLC2.Layers.ON[time][row][col] * 2 ;
            // value = std::max(0, std::min(255, value));
            image.at<uchar>(row, col) = static_cast<uchar>(value);
        }
    }


    cv::Mat enlarged_image;
    cv::resize(image, enlarged_image, cv::Size(WIDTH * 10, HEIGHT * 10), 0, 0, cv::INTER_NEAREST);

    char text[50];
    sprintf_s(text, sizeof(text), "Frame [%d]  - cur %d - ON Layer", frame, hLPLC2.cur);
    cv::putText(enlarged_image, text, cv::Point(20, 40),  // 文字位置
        cv::FONT_HERSHEY_SIMPLEX, 0.5,            // 字体 & 大小
        cv::Scalar(255), 1);                      // 颜色（白色）& 粗细

    cv::imshow("ON Layer", enlarged_image);
    std::cout << "Press any key to close the window ..." << std::endl;
    //cv::waitKey(0);
    //cv::destroyAllWindows();

    int key = cv::waitKey(30);
    if (key != -1)
    {
        cv::waitKey(0);
    }
}

void visualize_layer_OFF(int time, int frame ) 
{
    cv::Mat image(HEIGHT, WIDTH, CV_8UC1);


    for (int row = 0; row < HEIGHT; row++) 
    {
        for (int col = 0; col < WIDTH; col++) 
        {
            int value = LPLC2.Layers.OFF[time][row][col] * 2 ;
            // value = std::max(0, std::min(255, value));
            image.at<uchar>(row, col) = static_cast<uchar>(value);
        }
    }


    cv::Mat enlarged_image;
    cv::resize(image, enlarged_image, cv::Size(WIDTH * 10, HEIGHT * 10), 0, 0, cv::INTER_NEAREST);

    char text[500];
    if(hLPLC2.Attention_Mechanism == 1 || hLPLC2.Attention_Mechanism == 0 )
        //sprintf_s(text, sizeof(text), "Frame [%d] - cur %d - OFF Layer - AFC [%d, %d]", frame, hLPLC2.cur, LPLC2.Results.Centroid[0], LPLC2.Results.Centroid[1]);
        ;
    if(hLPLC2.Attention_Mechanism == 2 ) // Multiple AFs
        sprintf_s(text, sizeof(text), "Frame [%d] - cur %d - OFF Layer - AFC [%d, %d]; [%d, %d]; [%d, %d]", frame, hLPLC2.cur, LPLC2.AF[0].Centroid[0], LPLC2.AF[0].Centroid[1], LPLC2.AF[1].Centroid[0], LPLC2.AF[1].Centroid[1], LPLC2.AF[2].Centroid[0], LPLC2.AF[2].Centroid[1]);

    cv::putText(enlarged_image, text, cv::Point(20, 40),  // 文字位置
        cv::FONT_HERSHEY_SIMPLEX, 0.5 ,            // 字体 & 大小
        cv::Scalar(255), 1);                      // 颜色（白色）& 粗细


    cv::imshow("OFF Layer", enlarged_image);
    //cv::waitKey(0);
    //cv::destroyAllWindows();

    int key = cv::waitKey(100);
    if (key != -1)
    {
        cv::waitKey(0);
    }
}

// 2025.04.07 Renyuan Removed for Memory Space: 
// void visualize_layer_T(int frame)
// {
//     cv::Mat image(HEIGHT, WIDTH, CV_8UC1);
// 
//     for (int row = 0; row < HEIGHT; row++)
//     {
//         for (int col = 0; col < WIDTH; col++)
//         {
//             double value = (LPLC2.Layers.T[row][col] > 0) ? (LPLC2.Layers.T[row][col] * 255 / 32258) : 0;
//             if(value > 255 )
//                 value = 255 ;
//             image.at<uchar>(row, col) = static_cast<uchar>(value);
//         }
//     }
// 
//     cv::Mat enlarged_gray;
//     cv::resize(image, enlarged_gray, cv::Size(WIDTH * 10, HEIGHT * 10), 0, 0, cv::INTER_NEAREST);
// 
// 
//     // Mark AFC : 
//     cv::Mat enlarged_image;
//     cv::cvtColor(enlarged_gray, enlarged_image, cv::COLOR_GRAY2BGR);
//     if(hLPLC2.Attention_Mechanism == 1 ){
//     /*
//         for (int n = 0; n < NUM_AF; n++)
//         {
//             int x1 = LPLC2.Results.Centroid[1] * 10;
//             int y1 = LPLC2.Results.Centroid[0] * 10;
// 
//             // Mark AFC:
//             cv::circle(enlarged_image, cv::Point(x1, y1), 5, cv::Scalar(0, 255, 0), -1);
// 
//             int8_t AF_Radius_Exclusive = LPLC2.Params.AF_Radius_Exclusive;
//             int8_t AF_Radius = LPLC2.Params.AF_Radius;
// 
//             // Mark AF:
//             int16_t d = (LPLC2.Results.Centroid[0] + AF_Radius) * 10;
//             int16_t u = (LPLC2.Results.Centroid[0] - AF_Radius) * 10;
//             int16_t r = (LPLC2.Results.Centroid[1] + AF_Radius) * 10;
//             int16_t l = (LPLC2.Results.Centroid[1] - AF_Radius) * 10;
//             cv::rectangle(enlarged_image, cv::Point(l, u), cv::Point(r, d), cv::Scalar(0, 100, 0), 2);
// 
//             // Mark AF Exclusive Field: 
//             d = (LPLC2.Results.Centroid[0] + AF_Radius_Exclusive) * 10;
//             u = (LPLC2.Results.Centroid[0] - AF_Radius_Exclusive) * 10;
//             r = (LPLC2.Results.Centroid[1] + AF_Radius_Exclusive) * 10;
//             l = (LPLC2.Results.Centroid[1] - AF_Radius_Exclusive) * 10;
//             cv::rectangle(enlarged_image, cv::Point(l, u), cv::Point(r, d), cv::Scalar(0, 50, 0), 2);
//         }
//     */
//     }else if(hLPLC2.Attention_Mechanism == 2 ){
//         for (int n = 0; n < NUM_AF; n++)
//         {
//             if(LPLC2.AF[n].Vacant_AF == 1 )
//                 continue ;
//             int x1 = LPLC2.AF[n].Centroid[1] * 10;
//             int y1 = LPLC2.AF[n].Centroid[0] * 10;
// 
//             // Mark AFC:
//             cv::circle(enlarged_image, cv::Point(x1, y1), 5, cv::Scalar(0, 255, 0), -1);
// 
//             int8_t AF_Radius_Exclusive = LPLC2.Params.AF_Radius_Exclusive;
//             int8_t AF_Radius = LPLC2.Params.AF_Radius;
// 
//             // Mark AF:
//             int16_t d = (LPLC2.AF[n].Centroid[0] + AF_Radius) * 10;
//             int16_t u = (LPLC2.AF[n].Centroid[0] - AF_Radius) * 10;
//             int16_t r = (LPLC2.AF[n].Centroid[1] + AF_Radius) * 10;
//             int16_t l = (LPLC2.AF[n].Centroid[1] - AF_Radius) * 10;
//             cv::rectangle(enlarged_image, cv::Point(l, u), cv::Point(r, d), cv::Scalar(0, 100, 0), 2);
// 
//             // Mark AF Exclusive Field: 
//             d = (LPLC2.AF[n].Centroid[0] + AF_Radius_Exclusive) * 10;
//             u = (LPLC2.AF[n].Centroid[0] - AF_Radius_Exclusive) * 10;
//             r = (LPLC2.AF[n].Centroid[1] + AF_Radius_Exclusive) * 10;
//             l = (LPLC2.AF[n].Centroid[1] - AF_Radius_Exclusive) * 10;
//             cv::rectangle(enlarged_image, cv::Point(l, u), cv::Point(r, d), cv::Scalar(0, 50, 0), 2);
//         }
// 
//     }else{
//         // Meditation when no AF ... 
//     }
//     
// 
// 
//     char text[500];
//     if (hLPLC2.Attention_Mechanism == 1 || hLPLC2.Attention_Mechanism == 0)
//         // sprintf_s(text, sizeof(text), "Frame [%d] - cur %d - T Layer - AFC [%d, %d]", frame, hLPLC2.cur, LPLC2.Results.Centroid[0], LPLC2.Results.Centroid[1]);
//         ;
//     if (hLPLC2.Attention_Mechanism == 2) // Multiple AFs
//         if (NUM_AF == 1){
//             sprintf_s(text, sizeof(text), "Frame [%d] - cur %d - T Layer - AFC %d[%d, %d] ", frame, hLPLC2.cur, !LPLC2.AF[0].Vacant_AF, LPLC2.AF[0].Centroid[0], LPLC2.AF[0].Centroid[1]);
//         }else if (NUM_AF == 2){
//             sprintf_s(text, sizeof(text), "Frame [%d] - cur %d - T Layer - AFC %d[%d, %d]; %d[%d, %d] ", frame, hLPLC2.cur, !LPLC2.AF[0].Vacant_AF, LPLC2.AF[0].Centroid[0], LPLC2.AF[0].Centroid[1], !LPLC2.AF[1].Vacant_AF, LPLC2.AF[1].Centroid[0], LPLC2.AF[1].Centroid[1]);
//         }else{
//             sprintf_s(text, sizeof(text), "Frame [%d] - cur %d - T Layer - AFC %d[%d, %d]; %d[%d, %d]; %d[%d, %d] ", frame, hLPLC2.cur, !LPLC2.AF[0].Vacant_AF, LPLC2.AF[0].Centroid[0], LPLC2.AF[0].Centroid[1], !LPLC2.AF[1].Vacant_AF, LPLC2.AF[1].Centroid[0], LPLC2.AF[1].Centroid[1], !LPLC2.AF[2].Vacant_AF, LPLC2.AF[2].Centroid[0], LPLC2.AF[2].Centroid[1]);
//         }
// 
//     cv::putText(enlarged_image, text, cv::Point(20, 40), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1); // White Text.
// 
// 
// 
//     cv::imshow("T Layer", enlarged_image);
// 
//     int key = cv::waitKey(100);
//     if (key != -1)
//     {
//         cv::waitKey(0);
//     }
// }


void aLPLC2_VisualModel(void)
{

	//static uint32_t temp = 0, interval_begin = 0 ;

	//hLPLC2.Interval_aLPLC2 = TOCin5 - interval_begin ; // timing ends. aLPLC2 interval: 32 ms, < 33 ms.
	//interval_begin = TOCin5 ; // timing starts

	//temp = TOCin5 ;
	LPLC2_PreSynaptic(&hLPLC2); // 1.9 ms
	//hLPLC2.RunTime_aLPLC2_PreSynaptic = TOCin5 - temp ;

	//temp = TOCin5 ;
	LPLC2_T4_T5(&hLPLC2); // 2.1 ms
	//hLPLC2.RunTime_aLPLC2_T4_T5 = TOCin5 - temp ;

	//temp = TOCin5 ;
	LPLC2_Projection(&hLPLC2); // 0.0017 ms
	//hLPLC2.RunTime_aLPLC2_Projection = TOCin5 - temp ;


    VisionToMotor(&hLPLC2);
}


int main(void)
{
    // Put gray-scale information into 3-demention array : 
    //process_video("C:\\A_RESEARCH\\Data\\Video_Dataset\\uav\\black_balloon_looming1.mp4");
    //process_video("C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\synthetic_vedios\\Vedio_basic_test\\test_black_approach.avi");
    //process_video("C:\\A_RESEARCH\\Off_Line\\C\\LPLC2_Colias\\square.avi");
    //process_video("C:\\A_RESEARCH\\Off_Line\\C\\LPLC2_Colias\\square_right.avi");
    //process_video("C:\\A_RESEARCH\\Off_Line\\C\\LPLC2_Colias\\square_right_to_centre.avi");
    //process_video("C:\\A_RESEARCH\\Off_Line\\C\\LPLC2_Colias\\black_center.avi");
    //process_video("C:\\A_RESEARCH\\Off_Line\\C\\LPLC2_Colias\\square_right_left.avi");
    process_video("C:\\A_RESEARCH\\Off_Line\\C\\LPLC2_Colias\\square_double.avi");
    //process_video("C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\synthetic_vedios\\Vedio_dynamic_complex_bg\\standared_UAV_outdoor_1.avi");
    

    remove("C:\\A_RESEARCH\\Off_Line\\C\\LPLC2_Colias\\OUTPUT\\OUTPUT.txt") ;


	//Colias_Init();
	LPLC2_Init(&hLPLC2) ; // Colias_Init_aLPLC2();
	//CAPI_MotionEnable();

	// On-line version : 
	// Colias_loopbody_aLPLC2();
	



	// Off-line version loop body : 
    uint32_t* frame = (uint32_t*)malloc(sizeof(uint32_t) ) ;
	for(*frame = 0 ; *frame < FRAMES; (*frame) ++ )
	{

        // Camera Simulation : Put video into Colias' camera.
        for(int i = 0; i < HEIGHT; i ++ )
        {
            for(int j = 0; j < WIDTH; j ++ )
            {
                Image[(*frame)%3][i][j] = frames[*frame][i][j] ;
            }
        }
        

        // Frame count : 
		hLPLC2.hFrameCount = frame ; // In on-line version, this is uodated by camera automatically.
        printf("\n\n\n\n[ Frame %d ] \n", *frame ) ;


		// See : 
		aLPLC2_VisualModel() ;

		// And Move : 
		//LCF_StateType STATE ;
		//STATE = FSM_aLPLC2_motion(&LCFModel1 ) ;


        
        if (LPLC2.AF[0].Output[*(hLPLC2.hFrameCount)%4] > MAX_LPLC2 )
            MAX_LPLC2 = LPLC2.AF[0].Output[*(hLPLC2.hFrameCount)%4] ;
        if (LPLC2.AF[1].Output[*(hLPLC2.hFrameCount)%4] > MAX_LPLC2 )
            MAX_LPLC2 = LPLC2.AF[1].Output[*(hLPLC2.hFrameCount)%4] ;
        if (LPLC2.AF[2].Output[*(hLPLC2.hFrameCount)%4] > MAX_LPLC2 )
            MAX_LPLC2 = LPLC2.AF[2].Output[*(hLPLC2.hFrameCount)%4] ;
      

        //write_txt() ; 
        //write_txt_multiple_AFs() ;







        // Layers Visualizasion : 
        // uint8_t cur = hLPLC2.currentDiffImage, pre = !cur ;
        uint8_t cur = hLPLC2.cur ; // belongs to 0~9.
        uint8_t pre = (cur-LPLC2.Params.Delay) >= 0 ? (cur-LPLC2.Params.Delay) : ((cur-LPLC2.Params.Delay)+DELAY_WINDOW) ;
        //visualize_layer_ON(cur, *frame) ;
        
        //visualize_layer_OFF(cur, *frame) ;
        
        // visualize_layer_T(*frame) ;



        /*
        // AF Status Map : 
        printf("\n\ncur :") ;
        for (int row = 0; row < HEIGHT; row++) 
        {
            printf("\n") ;
            for (int col = 0; col < WIDTH; col++) 
            {

                //uint8_t value = Image[1][row][col] >> 9 ;
                //int8_t value = Diff_Image[cur][row][col] ;
                //uint8_t value = LPLC2.Layers.ON[cur][row][col] ;
                //uint8_t value = LPLC2.Layers.OFF[cur][row][col] ;
                uint8_t value = LPLC2.Layers.IS_AF[row][col] ;



                if(value > 100 || value <= -10 ){
                    printf("%d", value);
                }else if(value > 10 || (value < 0 && value > -10 ) ){
                    printf("%d ", value);
                }else{
                    printf("%d  ", value);
                }

            }
        } // end print
        */


	} // end for
    printf("\n\n\nMAX LPLC2 output: %d \n\n\n", MAX_LPLC2 ) ;



    return 0;
}


/*

// Print Layers (Plain printf ... ) : 
uint8_t cur = hLPLC2.currentDiffImage, pre = !cur ;
// Pre : 
printf("\n\npre :") ;
for (int row = 0; row < HEIGHT; row++) 
{
    printf("\n") ;
    for (int col = 0; col < WIDTH; col++) 
    {

        //uint8_t value = Image[1][row][col] >> 9 ;
        //int8_t value = Diff_Image[pre][row][col] ;
        //uint8_t value = LPLC2.Layers.ON[pre][row][col] ;
        uint8_t value = LPLC2.Layers.OFF[pre][row][col] ;



        if(value > 100 || value <= -10 ){
            printf("%d", value);
        }else if(value > 10 || (value < 0 && value > -10 ) ){
            printf("%d ", value);
        }else{
            printf("%d  ", value);
        }

    }
} // end print

// Cur : 
printf("\n\ncur :") ;
for (int row = 0; row < HEIGHT; row++) 
{
    printf("\n") ;
    for (int col = 0; col < WIDTH; col++) 
    {

        //uint8_t value = Image[1][row][col] >> 9 ;
        //int8_t value = Diff_Image[cur][row][col] ;
        //uint8_t value = LPLC2.Layers.ON[cur][row][col] ;
        uint8_t value = LPLC2.Layers.OFF[cur][row][col] ;
        uint8_t value = LPLC2.Layers.IS_AF[row][col] ;



        if(value > 100 || value <= -10 ){
            printf("%d", value);
        }else if(value > 10 || (value < 0 && value > -10 ) ){
            printf("%d ", value);
        }else{
            printf("%d  ", value);
        }

    }
} // end print


printf("\n\nEMD :") ;
for (int row = 0; row < HEIGHT; row++) 
{
    printf("\n\n") ;
    for (int col = 0; col < WIDTH; col++) 
    {

        int16_t value = LPLC2.Layers.T[row][col] ;



        if(value > 10000 ){
            printf("%d ", value );
        }else if(value > 1000 ){
            printf("%d  ", value);
        }else if(value > 100 ){
            printf("%d   ", value);
        }else if(value > 10 ){
            printf("%d    ", value);
        }else{
            printf("%d     ", value);
        }

    }
} // end print


*/
