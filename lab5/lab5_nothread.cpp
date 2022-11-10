/*********************************************************
* File: lab3.cpp
*
* Description: brief description of file purpose
*
* Author: Joshua Alderson
* 
* Revisions: 0
*
**********************************************************/

#include <opencv2/highgui.hpp> 
#include <arm_neon.h>
#include <unistd.h>
//number of channels for RGB
#define     RGB     3
//number of channels for grayscale
#define     GRAY    1

using namespace cv;
using namespace std;

/*-----------------------------------------------------
* Function: to442_grayscale
* 
* Description: Takes in a image, reads the data, converts colors to gray, and returns another image
* 
* param a: Mat: original image frame from video
* 
* return: Mat
*--------------------------------------------------------*/
Mat to442_grayscale(Mat &image, Mat &grayscale)
{
    
    //create pointers to the picture data
    uchar *pixel = (uchar*) image.data;
    uchar *gray_data = (uchar*) grayscale.data;

    uint8x8x3_t colors; 
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
    int pixel_num = (image.rows * image.cols)/8;

    //for loop
    for(int i = 0; i < pixel_num; i++, pixel += 8 * 3, gray_data += 8)
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
    return grayscale;
}

/*-----------------------------------------------------
* Function: to442_sobel
* 
* Description: Takes a grayscaled pixel and applys a sobel filter to it
* 
* param a: Mat: the gray scaled image made previously
* 
* return: Mat
*--------------------------------------------------------*/
Mat to442_sobel(Mat &gray)
{
    /*
             |-1 0 1|        | 1  2  1|      |P1 P2 P3|
        Gx = |-2 0 2|   Gy = | 0  0  0|  P = |P4 P5 P6|
             |-1 0 1|        |-1 -2 -1|      |P7 P8 P9|
        for each pixel convlove the 3x3 pixel grid with Gx and Gy
        X: -1P1 -2P4 -P7 + P3 + 2P6 + P7 
        Y: P1 - P7 + 2P2 -2P8 + P3 -P9
    */

    //Mat object to hold sobel data
    Mat sobel(gray.rows,gray.cols,CV_8UC1);
    uchar *gray_data = (uchar*) gray.data;
    uchar *filter_data = (uchar*) sobel.data;

    //make vectors for kernal values that are not 1 or 0
    // uint8x8x3_t sobel_row1; 
    // uint8x8x3_t sobel_row2; 
    // uint8x8x3_t sobel_row3; 
    uint8x8_t p1,p2,p3,p4,p6,p7,p8,p9
    int16x8_t gx_holder_vect;
    int16x8_t gy_holder_vect;
    uint16x8_t min_comp_vect = vdupq_n_u16(255);
    uint16x8_t sobel_vect;

    int pixel_num = ((gray.rows * gray.cols) - gray.cols)/8;
    //TODO: check to see if I need to change this from pix num to something else
    for(int i = gray.cols; i < pixel_num; i++, gray_data += 8, filter_data += 8)
    {
        //load the first 3 elements for sobel calculations and put them in vectors
        //TODO: need to get row offsets?
        p1 = vld1_u8(gray_data);
        p2 = vld1_u8(gray_data + 1);
        p3 = vld1_u8(gray_data + 2);
        p4 = vld1_u8(gray_data + gray.cols);
        p6 = vld1_u8(gray_data + gray.cols + 2);
        p7 = vld1_u8(gray_data + 2*gray.cols);
        p8 = vld1_u8(gray_data + 2*gray.cols);
        p9 = vld1_u8(gray_data + 2*gray.cols);
        // sobel_row1 = vld3_u8(gray_data); // p1,p2,p3 but as vectors
        // sobel_row2 = vld3_u8(gray_data + gray.cols); // p4,p5,p6 in vectors. don't use the p5 vector so it is a bit wasteful but more convienent
        // sobel_row3 = vld3_u8(gray_data + gray.cols * 2); // p7,p8,p9


        // //multiply and add correct kernal values
        // /*****************gx calculations********************/
        // //X: -P1 -2P4 -P7 + P3 + 2P6 + P9
        // //|-(p1 + p7) + (p3 + p9) + (2P6 - 2P4)|
        // gx_holder_vect = vabsq_s16(
        //           vaddq_u16(vsubq_u16(vshll_n_u8(sobel_row2.val[2],1), vshll_n_u8(sobel_row2.val[0],1))//2P6 - 2P4
        //          ,vsubq_u16(vaddl_u8(sobel_row1.val[2], sobel_row3.val[2])//p3 + p9
        //                     ,vaddl_u8(sobel_row1.val[2], sobel_row3.val[2]))));//p1 + p7
        // /***************************************************/

        // /*****************gy calculations********************/
        // //Y: P1 - P7 + 2P2 - 2P8 + P3 -P9
        // //|(P1 + P3) - (P7 + P9) + (2P2 - 2P8)|
        // gy_holder_vect = vabsq_s16(
        //                  vaddq_u16(vsubq_u16(vaddl_s8(sobel_row1.val[0],sobel_row1.val[2]), //P1 + P3
        //                                      vaddl_s8(sobel_row3.val[0],sobel_row3.val[2])), //P7 + P9
        //                                      vsubq_u16(vshll_n_u8(sobel_row1.val[1],1),vshll_n_u8(sobel_row3.val[2],1)))); //2P2 - 2P8
        
        // /***************************************************/

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
    return sobel;
}

int main(int argc, char* argv[])
{
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

    while(1)
    {
        //make Mat object to hold each frame
        Mat vid_frame;
        
        //put the captured frame in the frame object
        video >> vid_frame;
        Mat grayscale(vid_frame.rows,vid_frame.cols,CV_8UC1);
        //convert frame to grayscale
        to442_grayscale(vid_frame, grayscale);
        //apply sobel filter to the frame
        Mat sobel = to442_sobel(grayscale);
        
        // //resize image to fit on 1920x1080 screen
        // namedWindow("vid_frame", WINDOW_NORMAL);
        // resizeWindow("vid_frame", 1920, 1080);

        //display the frame
        imshow("Frame", sobel);
        
        //press the escape key to close the player
        char c = (char)waitKey(25);
        if(c==27)
            break;
    }
    
    //clear the memory used for the video capture
    video.release();

    //close the video player
    destroyAllWindows();
    
    return 0;
}

