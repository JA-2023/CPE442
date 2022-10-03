#include <opencv2/highgui.hpp> 
#include <unistd.h>
//#include "lab3.hpp"

using namespace cv;
using namespace std;

// Mat to442_grayscale(void)
// {
//     Vec3b R = 0;
//     Vec3b B = 0;
//     Vec3b G = 0;
//     Vec3b pixel = 0;
//     Vec3b gray = 0;
//     //read image
//     Mat image = imread("/home/josh/Desktop/School/CPE442/lab3/sobel_test.PNG");
//     //check for padding
//     //cycle through data and read each pixel
//     for(int col = 0; col < image.cols; col++)
//         for(int row = 0; row < image.rows; row++)
//         {
//             //get the pixel at the row and column 
//             pixel = image.at<Vec3b>(row,col);

//             B = pixel[0];
//             G = pixel[1];
//             R = pixel[2];

//             gray = 0.2126*R + 0.7152*G + 0.0722*B;
//             pixel = gray;
//         }

//     //name the window
//     String window = "test";

//     //create the window
//     namedWindow(window);

//     //show the image in the window
//     imshow(window, image);
//     //wait for key press
//     waitKey(0);
//     //destroy window after key is pressed
//     destroyWindow(window);
//     //get each pixel
//     //apply each gray scale to each pixel
// }
// Mat to442_sobel(void)
// {

// }

int main()
{
    uint8_t R = 0;
    uint8_t B = 0;
    uint8_t G = 0;
    //Vec3b pixel = 0;
    uint8_t gray = 0;
    
    //read image
    Mat image = imread("/home/josh/Desktop/School/CPE442/lab3/sobel_test.PNG");
    uint8_t *pixel = (uint8_t*) image.data;
    int num = image.channels();

    
    //cycle through data and read each pixel
    for(int row = 0; row < image.rows; row++)
        for(int col = 0; col < image.cols; col++)
        {
            B = pixel[row*image.cols*3 + col*3 + 0];
            G = pixel[row*image.cols*3 + col*3 + 1];
            R = pixel[row*image.cols*3 + col*3 + 2];

            gray = 0.2126*R + 0.7152*G + 0.0722*B;

            pixel[row*image.cols*3 + col*3+ 0] = gray;
            pixel[row*image.cols*3 + col*3 + 1] = gray;
            pixel[row*image.cols*3 + col*3 + 2] = gray;

            //gray = 0.2126*R + 0.7152*G + 0.0722*B;
            //pixel = gray;
        }

    //name the window
    String window = "test";

    //create the window
    namedWindow(window);

    //show the image in the window
    imshow(window, image);
    //wait for key press
    waitKey(0);
    //destroy window after key is pressed
    destroyWindow(window);
    //get each pixel
    //apply each gray scale to each pixel
}

