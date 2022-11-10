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

#include <opencv2/opencv.hpp> 
#include <pthread.h>
#include <arm_neon.h>
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
    int start;
    int stop;
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


    // //char array to hold the file name
    // char file[20];
    // //put command line argument in variable
    // strcat(file,argv[1]);
    // //get file path for the image
    // char path[100];
    // getcwd(path, 100);
 
    // //add directory and file to get full path
    // char* final[100] = {strcat(path,"/")};
    // strcat(*final,file);
    
    //make vidocapture to get each frame form the video
    //VideoCapture video(*final);

    // VideoCapture video("/home/josh/Desktop/School/CPE442/lab5/vid");
    VideoCapture video("/home/CPE/Desktop/CPE442/lab5/vid1.mp4");
    //make Mat to hold each frame
    Mat vid_frame;
    //put the captured frame in the frame object
    video >> vid_frame;

    //make frame to hold the grayscale and filtered image
    Mat filtered_frame(vid_frame.rows,vid_frame.cols,CV_8UC1);
    Mat gray_frame(vid_frame.rows,vid_frame.cols,CV_8UC1);

    int data_chunk = ((vid_frame.rows*vid_frame.cols)/4);

    //initialize arguments for the threads
    argument[0] = {.start = 0,
                    .stop = data_chunk,
                    .frame = vid_frame,
                    .gray = gray_frame,
                    .sobel =  filtered_frame};

    argument[1] = {.start = data_chunk,
                    .stop = data_chunk * 2,
                    .frame = vid_frame,
                    .gray = gray_frame,
                    .sobel =  filtered_frame};

    argument[2] = {.start = data_chunk * 2,
                    .stop = data_chunk * 3,
                    .frame = vid_frame,
                    .gray = gray_frame,
                    .sobel =  filtered_frame};

    argument[3] = {.start = data_chunk * 3,
                    .stop = data_chunk * 4,
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
        imshow("vid_frame", gray_frame);

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
    int start = arguments->start;
    int stop = arguments->stop;
    Mat frame = arguments->frame;
    Mat graycale = arguments->gray;
    Mat filter_frame = arguments->sobel;


    //initialize values to hold pixel values
    uchar R = 0, B = 0, G = 0, gray = 0;
    //initialize values to hold calclated values
    short gx = 0, gy = 0, grad = 0;

    //create pointers to the picture data
    uchar *pixel = (uchar*) frame.data;
    uchar *gray_data = (uchar*) graycale.data;
    uchar *filter_data = (uchar*) filter_frame.data;

    //make vectors to hold red,blue,green pixels
    //make these 8x16 vectors due to floating point math at the end
    //or just make these 8x32 and make an intermidiate one to hold the constants?
    //make variable to hold all of the RGB values from the data.
    uint8x8x3_t colors; 
    uint8x8_t gray_chunk;
    //make vectors that hold the constants to multiply by
    //r_num
    //fills a vector with the value listed
    uint8x8_t r_num = vdup_n_u8(77);
    //b_num
    uint8x8_t b_num = vdup_n_u8(29);
    //g_num
    uint8x8_t g_num = vdup_n_u8(150); 

    //variable to hole the intermidate values (16 bit values)
    uint16x8_t holder_vect;

    //vector to hold the result of the gray pixel calculations
    uint8x8_t gray_vect;

    //get the number of total pixels (rows*cols) and divide it by 8 since it is pull 8 pixels at a time
    //TODO: check if there might need to be padding since it may not be divisable by 8?
    //it would be the last vector and would be the last couple of lanes
    //gets the number of pixels and devides it by 8 since were working in 8x8 chunks
    int pixel_num = stop/8;

    //for loop
    for(int i = start; i < pixel_num; i++, pixel += 8 * 3, gray_data += 8)
    {
        //takes the RGB data and breaks in into 3 8x8 vectors each having one color
        colors = vld3_u8(pixel); //TODO: might need to change this to include an offset pixel + start?
        //multiply each vector lane by one of the constants
        //multiples the red pixels by the red number then stores them in the holder vector
        holder_vect = vmull_u8(colors.val[0], r_num);
        //multiplies the green pixels by the number then adds it to the values in the  holder vector
        holder_vect = vmlal_u8(holder_vect, colors.val[1], g_num);
        //multiples the blue pixels by the blue number then adds it to the values in the holder vector
        holder_vect = vmlal_u8(holder_vect, colors.val[2], b_num);

        //bit shift the values from the holder vector to narrow them down from 16 to 8 bits
        gray_vect = vshrn_n_u16(holder_vect, 8);

        //store the values in the gray vector in the gray picture mat
        vst1_u8(gray_data, gray_vect);
    }
        

    //Sobel stuff
    //need to add padding for certain things?
    //load a vector with all of the constants for gx and gy (ignore the middle pixel value since it is always 0)
    //
    //gx_values = -1 0 1 -2 2 -1  0  1
    //gy_values =  1 2 1  0 0 -1 -2 -1
    //load gray pixels from gray address (16 8 bit pixels)
    //multiply each pixel by the gx and gy values (could probably optamize this)
    //put those values in vectors 
    //take the absolute value (not sure how yet with vectors)
    //add the vectors together (will probably have to make these 16bit vectors) (might need padding here)
    //truncate these to 255 if needed
    //store these values in the sobel filter

    uint8x8_t p1,p2,p3,p4,p6,p7,p8,p9;
    int16x8_t gx_holder_vect;
    int16x8_t gy_holder_vect;
    uint16x8_t min_comp_vect = vdupq_n_u16(255);
    uint16x8_t sobel_vect;


    uchar gray_vals[8];
    //TODO: check to see if I need to change this from pix num to something else
    for(int i = start; i < pixel_num; i++, gray_data += 8, filter_data += 8)
    {
        //load the first 3 elements for sobel calculations and put them in vectors
        p1 = vld1_u8(gray_data);
        p2 = vld1_u8(gray_data + 1);
        p3 = vld1_u8(gray_data + 2);
        p4 = vld1_u8(gray_data + graycale.cols);
        p6 = vld1_u8(gray_data + graycale.cols + 2);
        p7 = vld1_u8(gray_data + 2*graycale.cols);
        p8 = vld1_u8(gray_data + 2*graycale.cols + 1);
        p9 = vld1_u8(gray_data + 2*graycale.cols + 2);


        //multiply and add correct kernal values
        /*****************gx calculations********************/
        //X: -P1 -2P4 -P7 + P3 + 2P6 + P9
        //|-(p1 + p7) + (p3 + p9) + (2P6 - 2P4)|
        gx_holder_vect = vabsq_s16(
                  vaddq_u16(vsubq_u16(vshll_n_u8(p6,1), vshll_n_u8(p4,1))//2P6 - 2P4
                 ,vsubq_u16(vaddl_u8(p3, p9)//p3 + p9
                            ,vaddl_u8(p1, p7))));//p1 + p7
        /***************************************************/

        /*****************gy calculations********************/
        //Y: P1 - P7 + 2P2 - 2P8 + P3 -P9
        //|(P1 + P3) - (P7 + P9) + (2P2 - 2P8)|
        gy_holder_vect = vabsq_s16(
                         vaddq_u16(vsubq_u16(vaddl_s8(p1,p3), //P1 + P3
                                             vaddl_s8(p7,p9)), //P7 + P9
                                             vsubq_u16(vshll_n_u8(p2,1),vshll_n_u8(p8,1)))); //2P2 - 2P8
        
        /***************************************************/
        //add gx and gy
        sobel_vect = vaddq_s16(gx_holder_vect, gy_holder_vect);
        //get the min between G and the 255 vector
        sobel_vect = vminq_u16(sobel_vect, min_comp_vect);

        //narrow vector to 8 bits and store the values in memory
        vst1_u8(filter_data, vmovn_u16(sobel_vect));
    }



    while(!trim_threads)
    {

        
        //wait for all threads to finish processing the filtered image
        pthread_barrier_wait(&sobel_barrier);
        //wait until a new frame has been aquired
        pthread_barrier_wait(&display_barrier);
    }
    return NULL;   
}
