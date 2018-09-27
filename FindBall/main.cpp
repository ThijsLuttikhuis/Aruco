#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <math.h>
#include <iostream>
#include <vector>

#define DEBUG true

using namespace cv;
class Image {
public:
    char name[30];  // name of the picture
    int x;          // amount of pixels in the x-direction
    int y;          // amount of pixels in the y-direction
};

Mat result;
const int SKIPPIXELS = 20;
const int NEWSKIPPIX = 3;

// using quickSort to sort the x- and y- values of orange pixels
void quickSort(std::vector<unsigned int> &arr, unsigned int left, unsigned int right) {
    unsigned int i = left, j = right;
    unsigned int tmp;
    unsigned int pivot = arr[(left + right) / 2];
    /* partition */
    while (i <= j) {
        while (arr[i] < pivot)
            i++;
        while (arr[j] > pivot)
            j--;
        if (i <= j) {
            tmp = arr[i];
            arr[i] = arr[j];
            arr[j] = tmp;
            i++;
            j--;
        }
    }
    /* recursion */
    if (left < j)
        quickSort(arr, left, j);
    if (i < right)
        quickSort(arr, i, right);
}



// (x position, y position, image, result, right/up/left/down
void findPixels(const int i, const int j, std::vector<unsigned int>&x, std::vector<unsigned int>&y, Mat image, bool direction[4], char maxIteration) {
    const Vec3b lowerBound = {50,122,230};         // set the boundaries of color 'orange'
    const Vec3b upperBound = {160,244,255};         // warning: cv::Mat uses BGR - blue, green, red
    Vec3b color = image.at<Vec3b>(i, j);            // the color of the pixel we are checking

    // assume the pixel is orange, and try to disprove it in the for loop
    bool orange = true;
    for (int k = 0; k <=2; k++) {
        if ( (color[k] > upperBound[k]) || (color[k] < lowerBound[k]) ) {
            orange = false;
        }
    }

    // if the pixel is orange, add the x and y values to the vector, and
    // check if the color 'NEWSKIPPIX' pixels away from it are orange as well

    // below the diagram of the recursive checking using newDirection
    //   ^ ^ ^ ^ ^
    //   ^ ^ ^ ^ ^
    //   ^ ^ ^ ^ ^
    //   < < < < o > >
    //           v v v
    //           v v v
    //           v v v

    if (orange) {
        x.push_back(i);
        y.push_back(j);

        if (DEBUG) {
            result.at<unsigned char>(i, j) = (unsigned char)(255 / ((SKIPPIXELS+2*NEWSKIPPIX) / NEWSKIPPIX) * maxIteration);
        }
        if (maxIteration>=1) {
            if (direction[0]) {
                bool newDirection[4] = {true, false, false, true};
                findPixels(i + NEWSKIPPIX, j, x, y, image, newDirection, (unsigned char)(maxIteration - 1));

            }
            if (direction[1]) {
                bool newDirection[4] = {false, true, false, false};
                findPixels(i, j + NEWSKIPPIX, x, y, image, newDirection, (unsigned char)(maxIteration - 1));

            }
            if (direction[2]) {
                bool newDirection[4] = {false, true, true, false};
                findPixels(i - NEWSKIPPIX, j, x, y, image, newDirection, (unsigned char)(maxIteration - 1));

            }
            if (direction[3]) {
                bool newDirection[4] = {false, false, false, true};
                findPixels(i, j - NEWSKIPPIX, x, y, image, newDirection, (unsigned char)(maxIteration - 1));

            }
        }

    }

    return;
}

int main() {
    Image test = {"whereisball3.jpg",4608,2592};

    Mat field = Mat(test.x, test.y, CV_8UC3);

    std::cout << test.name << ": " << test.x << " x " << test.y << std::endl;

    Mat image;
    image = imread(test.name);                     //load image to cv::Mat



    if(! image.data )                               // check for invalid input
    {
        std::cout <<  "Could not open or find the image" << std::endl ;
        return -1;
    }

    if (DEBUG) {                                    // in debug, show all orange pixels found
        result   = Mat(image.rows, image.cols, CV_8UC1);
        namedWindow( "Display frame",CV_WINDOW_NORMAL); // show the image
        imshow("Display frame", image);
        waitKey(1);
    }

    auto t = (double)getTickCount();                // check time taken

    std::vector<unsigned int> xResult;              // create vector of all orange x- and y-positions
    std::vector<unsigned int> yResult;

    // find orange pixels, skipping every ..th pixel
    // newDirection is the (recursive) directions the function has to look into when the found pixel is indeed orange
    for (int i = SKIPPIXELS; i < (test.y - SKIPPIXELS); i += SKIPPIXELS) {
        for (int j = SKIPPIXELS; j < (test.x - SKIPPIXELS); j += SKIPPIXELS) {
            bool newDirection[4] = {true,true,true,true};
            findPixels(i,j,xResult,yResult,image,newDirection,((SKIPPIXELS+2*NEWSKIPPIX) / NEWSKIPPIX) );
        }

    }

    unsigned int pixelsFound = xResult.size();
    quickSort(xResult,1,pixelsFound);
    quickSort(yResult,1,pixelsFound);
    std::cout << xResult.at( pixelsFound/2 ) << std::endl;
    std::cout << yResult.at( pixelsFound/2 ) << std::endl;

    t = ((double)getTickCount() - t)/getTickFrequency();
    std::cout << "Times passed in ms: " << t*1000 << std::endl;

    if (DEBUG) {                                    // in debug, show a plot of the found orange pixels
        namedWindow("Resulting frame", CV_WINDOW_NORMAL);
        imshow("Resulting frame", result);
        waitKey(0);
    }

    return 0;
}
