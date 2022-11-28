/*********************************************************
* File: final.cpp
*
* Description: program takes a video and puts it through a sobel filter using threads and vectors
*
* Author: Joshua Alderson
* 
* Revisions: 0
*
**********************************************************/

#include <opencv2/opencv.hpp> 
#include <pthread.h>
#include <arm_neon.h>
#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <V3DLib.h>

using namespace cv;
using namespace std;
using namespace V3DLib;

struct timeval start,stop;

void gpu_gray(Int::Ptr R, Int::Ptr B, Int::Ptr G, Int::Ptr R_num, Int::Ptr G_num, Int::Ptr B_num, Int::Ptr gray);
void gpu_sobel(Int::Ptr p1, Int::Ptr p2, Int::Ptr p3, Int::Ptr p4,
               Int::Ptr p6, Int::Ptr p7, Int::Ptr p8, Int::Ptr p9, Int::Ptr holder, Int::Ptr sobel);

int main(int argc, char* argv[])
{
    //set up GPU threading

    //construct the computation kernal
    auto gray_k = compile(gpu_gray);
    auto sobel_k = compile(gpu_sobel);

    //allocate memory for the gpu calculations
    Int::Array R(16), B(16), G(16), R_num(16), G_num(16), B_num(16), gray(16);
    Int::Array p1(16), p2(16), p3(16), p4(16), p6(16), p7(16), p8(16), p9(16), holder(16), sobel(16);

    //initialize constants for the grayscale math
    for(int i = 0; i < 16; i++)
    {
        R_num[i] = 77;
        G_num[i] = 150;
        B_num[i] = 29;
    }

    //variables for calculating the fps
    float time_avg = 0.0;
    int frame_counter = 0;

    //char array to hold the file name
    char file[20];
    //put command line argument in variable
    strcat(file,argv[1]);
    //get file path for the image
    char path[100];
    getcwd(path, 100);
 
    //add directory and file to get full path
    char* final[100] = {strcat(path,"/")};
    strcat(*final,argv[1]);
    
    // make vidocapture to get each frame form the video
    VideoCapture video(*final);

    //make Mat to hold each frame
    Mat vid_frame;

    //get frame/chunk size for threading
    int frame_size = vid_frame.rows*vid_frame.cols;
    int data_chunk = frame_size/4;
    //pointer to pixel data of video frame
    uchar *gpu_pixel = (uchar*) vid_frame.data;
    

    //make frame to hold the grayscale and filtered image
    Mat filtered_frame(vid_frame.rows,vid_frame.cols,CV_8UC1);
    Mat gray_frame(vid_frame.rows,vid_frame.cols,CV_8UC1);

    uchar *gpu_gdata = (uchar*) gray_frame.data;
    uchar *gpu_sobel = (uchar*) filtered_frame.data;

    int gray_num = frame_size / 16;
    int sobel_num = frame_size / 16 - gray_frame.cols;

    while(1)
    {

        
        //get the difference between the start and stop time for single frame in seconds
        time_avg += (stop.tv_sec - start.tv_sec) + (stop.tv_sec - start.tv_sec);
        //get new frame for processing
        video >> vid_frame;
        frame_counter++;
        //get the time when the frame is captured
        gettimeofday(&start, NULL);
        if(vid_frame.empty())
            break;
        
        
        //TODO: reset the pixel pointer and gray pointer
        //calculate grayscale data using the gpu
        for(int count = 0; count < gray_num; count++, gpu_pixel += 16*3, gpu_gdata += 16)
        {
            //get new data and put it in arrays for gpu calculations
            for(int i = 0, i < 16; i++)
            {
                //get red pixel
                R[i] = (int)(gpu_pixel + i*3);
                //get blue pixel
                B[i] = (int)(gpu_pixel + i*3 + 1);
                //get green pixel
                G[i] = (int)(gpu_pixel + i*3 + 2);
            }

            gray_k.load(&G, &B, &G, &gray);
            gpu_gdata = (uchar)gray;
        }

        //TODO: reset the gray data pointer
        gpu_gdata = gray_frame.data;
        //calculate sobel data using the GPU
        for(int count = gray_frame.cols; count < sobel_num; count++ , gpu_gdata += 16, gpu_sobel += 16)
        {
            //get new data and put it in arrays for gpu calculations
            for(int i = 0, i < 16; i++)
            {
                //get gray pixel values and put them in vectors
                p1[i] = (int)(gpu_gdata + i);
                p2[i] = (int)(gpu_gdata + i + 1);
                p3[i] = (int)(gpu_gdata + i + 2);
                p4[i] = (int)(gpu_gdata + i + gray_frame.cols);
                p6[i] = (int)(gpu_gdata + i + gray_frame.cols + 2);
                p7[i] = (int)(gpu_gdata + i + 2*gray_frame.cols);
                p8[i] = (int)(gpu_gdata + i + 2*gray_frame.cols + 1);
                p9[i] = (int)(gpu_gdata + i + 2*gray_frame.cols + 2);

            }


            sobel_k.load(&p1, &p2, &p3, &p4, &p6, &p7, &p8, &p9, &holder, &sobel)
            gpu_sobel = (uchar)sobel;
        }
        

        //invoke the kernal
        //resize image to fit on 1920x1080 screen
        namedWindow("vid_frame", WINDOW_NORMAL);
        resizeWindow("vid_frame", 1920, 1080);
        
        //display the frame
        imshow("vid_frame", filtered_frame);

        //press the escape key to close the player
        char c = (char)waitKey(5);
        if(c==27)
            break;


        //cout << "FPS: " << (float)frame_counter / time_avg << endl;
    }


    //clear the memory used for the video capture
    video.release();
    //close video player
    cv::destroyAllWindows();

    cout << "final avg: " << frame_counter / time_avg << endl;
    return 0;
}

void gpu_gray(Int::Ptr R, Int::Ptr B, Int::Ptr G, Int::Ptr R_num, Int::Ptr G_num, Int::Ptr B_num, Int::Ptr gray)
{
    //make variables for understanding
    int r = *R;
    int b = *B;
    int g = *G;
    int r_num = *R_num;
    int b_num = *B_num;
    int g_num = *G_num;
    int gray_data = *gray;

    //multiply pixels by their constants and add together
    gray_data = r * r_num;
    gray_data = gray_data + b * b_num;
    gray_data = gray_data + b * b_num;

    //truncate values to a byte
    for(int i = 0; i < 16; i++)
    {
        if(gray_data[i] > 255)
        {
            gray_data[i] = 255;
        }
    }
}

void gpu_sobel(Int::Ptr p1, Int::Ptr p2, Int::Ptr p3, Int::Ptr p4,
               Int::Ptr p6, Int::Ptr p7, Int::Ptr p8, Int::Ptr p9, Int::Ptr holder, Int::Ptr sobel)
{
    //X: -P1 -2P4 -P7 + P3 + 2P6 + P9
    //|-(p1 + p7) + (p3 + p9) + (2P6 - 2P4)|
    *holder = *p3 + 2*(*p6) + *p9 - *p1 -2*(*p4) - *p7;

    //Y: P1 - P7 + 2P2 - 2P8 + P3 -P9
    //|(P1 + P3) - (P7 + P9) + (2P2 - 2P8)|
    *holder = *holder + *p1 - *p7 + 2*(*p2) + 2*(*p8) + *p3 - *p9

    for(int i = 0; i < 16; i++)
    {
        if(*holder[i] > 255)
        {
            *sobel[i] = 255;
        }
    }
}
