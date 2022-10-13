#include <opencv2/highgui.hpp> 
#include <unistd.h>
//number of channels for RGB
#define     RGB     3
//number of channels for grayscale
#define     GRAY    1

using namespace cv;
using namespace std;

Mat to442_grayscale(Mat &image)
{
    //initialize values to hold pixel values
    uchar R = 0, B = 0, G = 0, gray = 0;
    //initialize Mat to hold grayscale data. Same data size as original but with singal channel (one color)
    Mat grayscale(image.rows,image.cols,CV_8UC1);
    
    //create pointers to the picture data
    uchar *pixel = (uchar*) image.data;
    uchar *gray_pix = (uchar*) grayscale.data;

    //cycle through data and read each pixel
    for(int row = 0; row < image.rows; row++)
    {
        for(int col = 0; col < image.cols; col++)
        {
            //get the Blue, Red, and Green pixel values
            B = pixel[row*image.cols*RGB + col*RGB + 0];
            G = pixel[row*image.cols*RGB + col*RGB + 1];       
            R = pixel[row*image.cols*RGB + col*RGB + 2];

            //apply grayscale filter
            gray = 0.2126*R + 0.7152*G + 0.0722*B;

            //put filtered pixel into grayscale data
            gray_pix[row*image.cols + col] = gray;
        }
    }
    return grayscale;
}
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

    //pointers to grayscale data and sobel data
    uchar *pixel = (uchar*) gray.data;
    uchar *sob_pix = (uchar*) sobel.data;

    //initialize values to hold calclated values
    short gx = 0, gy = 0, G = 0;

    //get pixel
    for(int row = 1; row < (gray.rows - 1); row++)
    {
        for(int col = 1; col < (gray.cols - 1); col++)
        {
            
            //apply sobel filter to X direction
            // X: -1P1 -2P4 -P7 + P3 + 2P6 + P7 
            gx = - pixel[(row - 1)*gray.cols + (col-1)]
                 - 2*pixel[row*gray.cols + (col - 1)]
                 - pixel[(row + 1)*gray.cols + (col-1)]
                 + pixel[(row - 1)*gray.cols + (col+1)]
                 + 2*pixel[row*gray.cols + (col + 1)]
                 + pixel[(row + 1)*gray.cols + (col-1)];

            //apply sobel filter to Y direction
            // Y: P1 - P7 + 2P2 -2P8 + P3 -P9
            gy =   pixel[(row - 1)*gray.cols + (col-1)]
                 - pixel[(row + 1)*gray.cols + (col-1)]
                 + 2*pixel[(row - 1)*gray.cols + col]
                 - 2*pixel[(row + 1)*gray.cols + col]
                 + pixel[(row - 1)*gray.cols + (col+1)]
                 - pixel[(row + 1)*gray.cols + col];

            //calculate the gradient for new pixel value
            G = abs(gx) + abs(gy);

            //check if new value is above the largest 8bit value and replace if needed
            if(G > 255)
                G = 255;
            
            //put the calculated value in the sobel array
            sob_pix[row*gray.cols + col] = (uchar) G;
        }
    }
    return sobel;
}

int main()
{
    //make vidocapture to get each frame form the video
    VideoCapture video("/home/josh/Desktop/School/CPE442/lab3/vid3.mp4");

    while(1)
    {
        //make Mat object to hold each frame
        Mat vid_frame;
        //put the captured frame in the frame object
        video >> vid_frame;
        //convert frame to grayscale
        Mat grayscale = to442_grayscale(vid_frame);
        //apply sobel filter to the frame
        Mat sobel = to442_sobel(grayscale);
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

