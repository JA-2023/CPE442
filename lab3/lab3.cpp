#include <opencv2/highgui.hpp> 
#include <unistd.h>
//#include "lab3.hpp"

using namespace cv;
using namespace std;

Mat to442_grayscale(Mat &image)
{
    uchar R = 0;
    uchar B = 0;
    uchar G = 0;
    uchar gray = 0;
    Mat grayscale(image.rows,image.cols,CV_8UC1);
    
    
    uchar *pixel = (uchar*) image.data;
    uchar *gray_pix = (uchar*) grayscale.data;
    int num = image.channels();
    int g = grayscale.channels();

    
    //cycle through data and read each pixel
    for(int row = 0; row < image.rows; row++)
    {
        for(int col = 0; col < image.cols; col++)
        {
            B = pixel[row*image.cols*3 + col*3 + 0];
            G = pixel[row*image.cols*3 + col*3 + 1];       
            R = pixel[row*image.cols*3 + col*3 + 2];

            gray = 0.2126*R + 0.7152*G + 0.0722*B;

            gray_pix[row*image.cols + col] = gray;
        }
    }

    return grayscale;
}
Mat to442_sobel(Mat &gray)
{
    //start at (1,1) can't use edge pixels
    /*
             |-1 0 1|        | 1  2  1|      |P1 P2 P3|
        Gx = |-2 0 2|   Gy = | 0  0  0|  P = |P4 P5 P6|
             |-1 0 1|        |-1 -2 -1|      |P7 P8 P9|
        for each pixel convlove the 3x3 pixel grid with Gx and Gy
        X: -1P1 -2P4 -P7 + P3 + 2P6 + P7 
        Y: P1 - P7 + 2P2 -2P8 + P3 -P9
    */
    Mat sobel(gray.rows,gray.cols,CV_8UC1);
    uchar *pixel = (uchar*) gray.data;
    uchar *g_pix = (uchar*) sobel.data;

    

    
    uchar p1 = 0;
    uchar p2 = 0;
    uchar p3 = 0;
    uchar p4 = 0;
    uchar p5 = 0;
    uchar p6 = 0;
    uchar p7 = 0;
    uchar p8 = 0;
    uchar p9 = 0;
    short gx = 0;
    short gy = 0;
    short G = 0;
    //get pixel
    for(int row = 1; row < (gray.rows - 1); row++)
    {
        for(int col = 1; col < (gray.cols - 1); col++)
        {
            
            //top left
            p1 = pixel[(row - 1)*gray.cols + (col-1)];

            //top mid
            p2 = pixel[(row - 1)*gray.cols + col];

            //top right
            p3 = pixel[(row - 1)*gray.cols + (col+1)];

            //mid left
            p4 = pixel[row*gray.cols + (col - 1)];
            
            //center
            p5 = pixel[row*gray.cols + col];

            //mid right
            p6 = pixel[row*gray.cols + (col + 1)];

            //bottom left
            p7 = pixel[(row + 1)*gray.cols + (col-1)];

            //bottom mid
            p8 = pixel[(row + 1)*gray.cols + col];

            //bottom right
            p9 = pixel[(row + 1)*gray.cols + col];

            // X: -1P1 -2P4 -P7 + P3 + 2P6 + P7 
            // Y: P1 - P7 + 2P2 -2P8 + P3 -P9
            gx = -p1 - 2*p4 -p7 + p3 + 2*p6 + p7;
            gy = p1 - p7 + 2*p2 - 2*p8 + p3 - p9;
            G = abs(gx) + abs(gy);

            if(G > 255)
                G = 255;
            
            g_pix[row*gray.cols + col] = (uchar) G;
        }
        
        //TODO: replace the pixel this calculate value
    }
    
    return sobel;

}

int main()
{
    VideoCapture vid("/home/josh/Desktop/School/CPE442/lab3/vid2.mp4");

    while(1)
    {
        Mat frame;
        vid >> frame;
        Mat grayscale = to442_grayscale(frame);
        Mat sobel = to442_sobel(grayscale);
        imshow("Frame", sobel);

        char c = (char)waitKey(25);
        if(c==27)
            break;
    }
    
    vid.release();

    destroyAllWindows();
    
    return 0;
}

