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

using namespace cv;
using namespace std;

struct timeval start,stop;

//struct to hold all of the variables for thread arguments
typedef struct thread_args
{
    int start_gray;
    int stop_gray;
    int start_sobel;
    int stop_sobel;
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
    //variables for calculating the fps
    float time_avg = 0.0;
    int frame_counter = 0;

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
    strcat(*final,argv[1]);
    
    // make vidocapture to get each frame form the video
    VideoCapture video(*final);

    //make Mat to hold each frame
    Mat vid_frame;
    //put the captured frame in the frame object
    video >> vid_frame;
    //get the time when the frame is captured
    gettimeofday(&start, NULL);
    frame_counter++;

    //make frame to hold the grayscale and filtered image
    Mat filtered_frame(vid_frame.rows,vid_frame.cols,CV_8UC1);
    Mat gray_frame(vid_frame.rows,vid_frame.cols,CV_8UC1);

    //get a quarter of all of the pixels
    int data_chunk = ((vid_frame.rows*vid_frame.cols)/4);
    // for(int arg_num = 0; arg_num < 4; arg_num++)
    // {
    //     argument[arg_num] = {.start_gray = arg_num * data_chunk,
    //                          .stop_gray = (arg_num + 1)*data_chunk,
    //                          .start_sobel = vid_frame.cols,
    //                          .stop_sobel = (arg_num + 1)*data_chunk,
    //                          .frame = vid_frame,
    //                          .gray = gray_frame,
    //                          .sobel =  filtered_frame};
    // }
    //initialize arguments for the threads
    argument[0] = {.start_gray = 0,
                    .stop_gray = data_chunk,
                    .start_sobel = vid_frame.cols,
                    .stop_sobel = data_chunk,
                    .frame = vid_frame,
                    .gray = gray_frame,
                    .sobel =  filtered_frame};

    argument[1] = {.start_gray = data_chunk - vid_frame.cols,
                    .stop_gray = data_chunk*2,
                    .start_sobel = data_chunk - vid_frame.cols,
                    .stop_sobel = data_chunk*2,
                    .frame = vid_frame,
                    .gray = gray_frame,
                    .sobel =  filtered_frame};

    argument[2] = {.start_gray = data_chunk*2 - vid_frame.cols,
                    .stop_gray = data_chunk*3,
                    .start_sobel = data_chunk*2 - vid_frame.cols,
                    .stop_sobel = data_chunk*3,
                    .frame = vid_frame,
                    .gray = gray_frame,
                    .sobel =  filtered_frame};

    argument[3] = {.start_gray = data_chunk*3 - vid_frame.cols,
                    .stop_gray = data_chunk*4,
                    .start_sobel = data_chunk*3- vid_frame.cols,
                    .stop_sobel = data_chunk*4 - vid_frame.cols,
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
        gettimeofday(&stop, NULL);
        
        //get the difference between the start and stop time for single frame in seconds
        time_avg += (stop.tv_sec - start.tv_sec) + (stop.tv_sec - start.tv_sec);
        //get new frame for processing
        video >> vid_frame;
        frame_counter++;
        //get the time when the frame is captured
        gettimeofday(&start, NULL);
        if(vid_frame.empty())
            break;
        
        
        //resize image to fit on 1920x1080 screen
        namedWindow("vid_frame", WINDOW_NORMAL);
        resizeWindow("vid_frame", 1920, 1080);
        
        //display the frame
        imshow("vid_frame", filtered_frame);

        //press the escape key to close the player
        char c = (char)waitKey(5);
        if(c==27)
            break;
        //wait for image to be shown and get a new frame
        pthread_barrier_wait(&display_barrier);

        //cout << "FPS: " << (float)frame_counter / time_avg << endl;
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
    cout << "final avg: " << frame_counter / time_avg << endl;
    return 0;
}

/*-----------------------------------------------------
* Function: thread_filter
* 
* Description: each thread will take in the frame of a video and processing 1/4 of it applying a sobel filter.
* Sobel calculations are done using vectors
* 
* param a: void pointer: start and stop values for image processing and the images to use
* 
* return: NULL
*--------------------------------------------------------*/
void *thread_filter(void *args)
{

    //get arguments and cast them
    thread_args *arguments = (thread_args*)(args);
    int start_gray = arguments->start_gray;
    int stop_gray = arguments->stop_gray;
    int start_sobel = arguments->start_sobel;
    int stop_sobel = arguments->stop_sobel;

    //create pointers to the picture data
    uchar *pixel = (uchar*) arguments->frame.data;
    uchar *gray_data = (uchar*) arguments->gray.data;
    uchar *sobel_data = (uchar*) arguments->sobel.data;

    //make variable to hold all of the RGB values from the data.
    //uint8x8x3_t colors;  
    uint8x8x3_t gray_row1;
    uint8x8x3_t gray_row2;
    uint8x8x3_t gray_row3;

    //make vectors that hold the constants to multiply by
    //r_num
    //fills a vector with the value listed
    uint8x8_t r_num = vdup_n_u8(77);
    //b_num
    uint8x8_t b_num = vdup_n_u8(29);
    //g_num
    uint8x8_t g_num = vdup_n_u8(150); 

    //variable to hold the intermidate values (16 bit values)
    uint16x8_t holder_vect;
    uint16x8_t holder2_vect;
    uint16x8_t holder3_vect;

    //vector to hold the result of the gray pixel calculations
    uint8x8_t gray_vect;

    //vectors to hold kernal pixels
    uint8x8_t pixels[9];
    //vectors for intermidiate calculations
    int16x8_t gx_holder_vect;
    int16x8_t gy_holder_vect;
    //final vector to be stored for sobel
    uint16x8_t sobel_vect;

    while(!trim_threads)
    {
        //move the pointer to the correct starting position
        pixel = arguments->frame.data + (start_gray *3); //multiply by 3 for RGB and 8 for the vectors
        //gray_data = arguments->gray.data + (start_gray); 
        sobel_data = arguments->sobel.data + (start_sobel);


        for(int i = start_sobel; i < stop_sobel; i+=8, sobel_data += 8, pixel += 8 * 3)
        {

            //calculate the gray values needed for sobel calculations
            for(int gray_ind = 0; gray_ind < 3; gray_ind++)
            {
                gray_row1 = vld3_u8(pixel + gray_ind*3); //get the first row
                gray_row2 = vld3_u8(pixel + arguments->frame.cols + gray_ind*3); //get the second row
                gray_row3 = vld3_u8(pixel + 2*arguments->frame.cols + gray_ind*3); //get the third row

                //calculate the first row values
                holder_vect = vmull_u8(gray_row1.val[2], r_num);
                holder_vect = vmlal_u8(holder_vect, gray_row1.val[1], g_num);
                holder_vect = vmlal_u8(holder_vect, gray_row1.val[0], b_num);
                pixels[gray_ind] = vshrn_n_u16(holder_vect, 8);

                //calculate the second row values
                holder2_vect = vmull_u8(gray_row2.val[2], r_num);
                holder2_vect = vmlal_u8(holder2_vect, gray_row2.val[1], g_num);
                holder2_vect = vmlal_u8(holder2_vect, gray_row2.val[0], b_num);
                pixels[gray_ind + 3] = vshrn_n_u16(holder2_vect, 8);
                    
                //calculate the third row values
                holder3_vect = vmull_u8(gray_row3.val[2], r_num);
                holder3_vect = vmlal_u8(holder3_vect, gray_row3.val[1], g_num);
                holder3_vect = vmlal_u8(holder3_vect, gray_row3.val[0], b_num);
                pixels[gray_ind + 6] = vshrn_n_u16(holder3_vect, 8);

            }
            // //load the kernal elements for sobel calculations and put them in vectors
            // //NOTE: the middle pixel (5) is not used so the index is pixel number - 2 after the fourth pixel
            // pixels[0] = vld1_u8(gray_data);
            // pixels[1] = vld1_u8(gray_data + 1);
            // pixels[2] = vld1_u8(gray_data + 2);
            // pixels[3] = vld1_u8(gray_data + arguments->gray.cols);
            // pixels[4] = vld1_u8(gray_data + arguments->gray.cols + 2);
            // pixels[5] = vld1_u8(gray_data + 2*arguments->gray.cols);
            // pixels[6] = vld1_u8(gray_data + 2*arguments->gray.cols + 1);
            // pixels[7] = vld1_u8(gray_data + 2*arguments->gray.cols + 2);


            //multiply and add correct kernal values
            /*****************gx calculations********************/
            //X: -P1 -2P4 -P7 + P3 + 2P6 + P9
            //|-(p1 + p7) + (p3 + p9) + (2P6 - 2P4)|
            gx_holder_vect = vabsq_s16(vaddq_s16( 
                                       vsubq_s16(vaddl_u8(pixels[2],pixels[8]),vaddl_u8(pixels[0],pixels[6])),//(p3 + p9) - (p1 + p7)
                                       vsubq_s16(vshll_n_u8(pixels[5],1),vshll_n_u8(pixels[3],1))));//(2P6 - 2P4)
            /***************************************************/

            /*****************gy calculations********************/
            //Y: P1 - P7 + 2P2 - 2P8 + P3 -P9
            //|(P1 + P3) - (P7 + P9) + (2P2 - 2P8)|
            gy_holder_vect = vabsq_s16(vaddq_s16(
                                       vsubq_s16(vaddl_u8(pixels[0],pixels[2]),vaddl_u8(pixels[6],pixels[8])), //(p1 + p3) - (p7 + p9)
                                       vsubq_s16(vshll_n_u8(pixels[1],1),vshll_n_u8(pixels[7],1)))); //(2p2 - 2p8)
            /***************************************************/
            //add gx and gy
            sobel_vect = vaddq_u16(gx_holder_vect, gy_holder_vect);

            //narrow vector to 8 bits and store the values in memory
            vst1_u8(sobel_data, vqmovn_u16(sobel_vect));
        }
        
        //wait for all threads to finish processing the filtered image
        pthread_barrier_wait(&sobel_barrier);
        //wait until a new frame has been aquired
        pthread_barrier_wait(&display_barrier);
    }
    return nullptr;   
}
