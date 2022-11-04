/*********************************************************
* File: lab4.cpp
*
* Description: program takes a video and puts it through a sobel filter using threads
*
* Author: Joshua Alderson
* 
* Revisions: 0
*
**********************************************************/

#include <opencv2/highgui.hpp> 
#include <pthread.h>
#include <iostream>
#include <unistd.h>
//number of channels for RGB
#define     RGB     3
//number of channels for grayscale
#define     GRAY    1

using namespace cv;
using namespace std;

//struct to hold all of the variables for thread arguments
typedef struct thread_args
{
    int start_rows;
    int gray_start;
    int gray_stop;
    int stop_rows;
    Mat frame;
    Mat gray;
    Mat sobel;
}thread_args;

//initalize two barriers. One for synching image display and filtered
pthread_barrier_t display_barrier;
pthread_barrier_t sobel_barrier;

//global flag to determine when threads should stop
int trim_threads = 0;

//function prototype
void *thread_filter(void *args);



int main(int argc, char* argv[])
{
    
    //create thread variables
    pthread_t quadrant[4];

    //make array to hold argument structs
    thread_args argument[4];
    
    //set up barriers for use
    pthread_barrier_init(&sobel_barrier, NULL, 5);
    pthread_barrier_init(&display_barrier, NULL, 5);


    //char array to hold the file name
    char file[20];
    //put command line argument in variable
    strcat(file,argv[1]);
    //get file path for the image
    char path[100];
    getcwd(path, 100);
 
    //add directory and file to get full path
    char* final[100] = {strcat(path,"/")};
    strcat(*final,file);
    
    //make vidocapture to get each frame form the video
    VideoCapture video(*final);

    //make Mat to hold each frame
    Mat vid_frame;
    //put the captured frame in the frame object
    video >> vid_frame;

    //make frame to hold the grayscale and filtered image
    Mat filtered_frame(vid_frame.rows,vid_frame.cols,CV_8UC1);
    Mat gray_frame(vid_frame.rows,vid_frame.cols,CV_8UC1);

    //initialize arguments for the threads
    argument[0] = {.start_rows = 1,
                    .gray_start = 0,
                    .gray_stop = vid_frame.rows/4,
                    .stop_rows = vid_frame.rows/4,
                    .frame = vid_frame,
                    .gray = gray_frame,
                    .sobel =  filtered_frame};

    argument[1] = {.start_rows = vid_frame.rows/4 -1,
                    .gray_start = vid_frame.rows/4 -1,
                    .gray_stop = vid_frame.rows/2,
                    .stop_rows =  vid_frame.rows/2,
                    .frame = vid_frame,
                    .gray = gray_frame,
                    .sobel =  filtered_frame};

    argument[2] = {.start_rows = vid_frame.rows/2 - 1,
                    .gray_start = vid_frame.rows/2 -1,
                    .gray_stop = (vid_frame.rows *3) / 4,
                    .stop_rows = (vid_frame.rows *3) / 4,
                    .frame = vid_frame,
                    .gray = gray_frame,
                    .sobel =  filtered_frame};

    argument[3] = {.start_rows = (vid_frame.rows * 3) / 4 - 1,
                    .gray_start = (vid_frame.rows * 3) / 4 - 1,
                    .gray_stop =   vid_frame.rows,
                    .stop_rows =  (vid_frame.rows - 1),
                    .frame = vid_frame,
                    .gray = gray_frame,
                    .sobel =  filtered_frame};

    //create the threads for computation
    for(int i = 0; i < 4; i++)
    {
        //creates a thread at each address of the quadrands, calls the thread_filter,
        //and passes the argument struct to the function.
        pthread_create(&quadrant[i], NULL, thread_filter, (void*) &argument[i]);
    }

    //there will be 5 threads, 4 created and one for main
    while(1)
    {
        //wait for all threads and main to reach the loop
        pthread_barrier_wait(&sobel_barrier);

        //get new frame for processing
        video >> vid_frame;
        if(vid_frame.empty())
            break;
        
        
        //resize image to fit on 1920x1080 screen
        namedWindow("vid_frame", WINDOW_NORMAL);
        resizeWindow("vid_frame", 1920, 1080);
        
        //display the frame
        imshow("vid_frame", filtered_frame);

        //press the escape key to close the player
        char c = (char)waitKey(25);
        if(c==27)
            break;
        //wait for image to be shown and get a new frame
        pthread_barrier_wait(&display_barrier);
    }

    //turn on flag to stop thread loops
    trim_threads = 1;
    //call wait one last time for loops to finish
    pthread_barrier_wait(&display_barrier);

    //clear the memory used for the video capture
    video.release();
    //close video player
    cv::destroyAllWindows();
    //destroy barriers and resources they used
    pthread_barrier_destroy(&sobel_barrier);
    pthread_barrier_destroy(&display_barrier);

    return 0;
}

/*-----------------------------------------------------
* Function: thread_filter
* 
* Description: each thread will take in the frame of a video and processing 1/4 of it applying a sobel filter
* 
* param a: void pointer: start and stop values for image processing and the images to use
* 
* return: NULL
*--------------------------------------------------------*/
void *thread_filter(void *args)
{

    //get arguments and cast them
    thread_args *arguments = (thread_args*)(args);
    int start_row = arguments->start_rows;
    int stop_row = arguments->stop_rows;
    int start_gray = arguments->gray_start;
    int stop_gray = arguments->gray_stop;
    Mat frame = arguments->frame;
    Mat graycale = arguments->gray;
    Mat filter_frame = arguments->sobel;


    //initialize values to hold pixel values
    uchar R = 0, B = 0, G = 0, gray = 0;
    //initialize values to hold calclated values
    short gx = 0, gy = 0, grad = 0;

    //create pointers to the picture data
    uchar *pixel = (uchar*) frame.data;
    uchar *gray_pix = (uchar*) graycale.data;
    uchar *filter_pix = (uchar*) filter_frame.data;
    
    while(!trim_threads)
    {
        //apply gray scaling to pixels
        for(int row = start_gray; row < stop_gray; row++)
        {
            for(int col = 0; col < frame.cols; col++)
            {
                //get the Blue, Red, and Green pixel values
                B = pixel[row*frame.cols*RGB + col*RGB + 0];
                G = pixel[row*frame.cols*RGB + col*RGB + 1];       
                R = pixel[row*frame.cols*RGB + col*RGB + 2];

                //apply grayscale filter
                gray = 0.2126*R + 0.7152*G + 0.0722*B;

                //put filtered pixel into grayscale data
                gray_pix[row*frame.cols + col] = gray;
            }
        }

        //apply sobel filtering
        for(int sob_row = start_row; sob_row < stop_row; sob_row++)
        {
            for(int sob_col = 1; sob_col < (graycale.cols - 1); sob_col++)
            {
                
                //apply sobel filter to X direction
                // X: -1P1 -2P4 -P7 + P3 + 2P6 + P7 
                gx = - gray_pix[(sob_row - 1)*graycale.cols + (sob_col-1)]
                    - 2*gray_pix[sob_row*graycale.cols + (sob_col - 1)]
                    - gray_pix[(sob_row + 1)*graycale.cols + (sob_col-1)]
                    + gray_pix[(sob_row - 1)*graycale.cols + (sob_col+1)]
                    + 2*gray_pix[sob_row*graycale.cols + (sob_col + 1)]
                    + gray_pix[(sob_row + 1)*graycale.cols + (sob_col-1)];

                //apply sobel filter to Y direction
                // Y: P1 - P7 + 2P2 -2P8 + P3 -P9
                gy =   gray_pix[(sob_row - 1)*graycale.cols + (sob_col-1)]
                    - gray_pix[(sob_row + 1)*graycale.cols + (sob_col-1)]
                    + 2*gray_pix[(sob_row - 1)*graycale.cols + sob_col]
                    - 2*gray_pix[(sob_row + 1)*graycale.cols + sob_col]
                    + gray_pix[(sob_row - 1)*graycale.cols + (sob_col+1)]
                    - gray_pix[(sob_row + 1)*graycale.cols + sob_col];

                //calculate the gradient for new pixel value
                grad = abs(gx) + abs(gy);

                //check if new value is above the largest 8bit value and replace if needed
                if(grad > 255)
                    grad = 255;
                
                //put the calculated value in the sobel array
                filter_pix[sob_row*graycale.cols + sob_col] = (uchar) grad;
            }
        }
        
        //wait for all threads to finish processing the filtered image
        pthread_barrier_wait(&sobel_barrier);
        //wait until a new frame has been aquired
        pthread_barrier_wait(&display_barrier);
    }
    return NULL;   
}
