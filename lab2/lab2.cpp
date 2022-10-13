#include <opencv2/highgui.hpp> 
#include <unistd.h>


using namespace cv;


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

    //read the image
    Mat swan = imread(*final);

    //name the window
    String window = "swan";

    //create the window
    namedWindow(window);

    //show the image in the window
    imshow(window, swan);
    //wait for key press
    waitKey(0);
    //destroy window after key is pressed
    destroyWindow(window);

    return(0);
}
       