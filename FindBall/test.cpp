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

int aaa = 0;
const int SKIPPIXELS = 10;
const int MAXNOTBLOB = 3;
const int NEWSKIPPIX = 10;                          //half of skippixels, rounded up

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

void findPixels(const unsigned int i, const unsigned int j, std::vector< std::vector<unsigned int> >&x,
                std::vector< std::vector<unsigned int> >&y, Mat image, Mat result, int iteration) {

    const Vec3b lowerBound = {50,122,230};          // set the boundaries of color 'orange'
    const Vec3b upperBound = {160,244,255};         // warning: cv::Mat uses BGR - blue, green, red
    Vec3b color = image.at<Vec3b>(i, j);            // the color of the pixel we are checking

    // assume the pixel is orange, and try to disprove it in the for loop
    bool orange = true;
    for (int k = 0; k <=2; k++) {
        if ( (color[k] > upperBound[k]) || (color[k] < lowerBound[k]) ) {
            orange = false;
        }
    }
    std::cout << color[0] << std::endl;
    // iteration == 1 means iterating in current blob
    // iteration >  1 means no orange pixel found, but still looking for orange for MAXNOTBLOB pixels
    if (iteration >= 1 && iteration <= MAXNOTBLOB) {
        if (false) {
            result.at<unsigned int>(i, j) = 255;
            x.push_back({i});
            y.push_back({j});

            std::cout << result.at<unsigned int>(i,j) << " x " << aaa++ << std::endl;

            if (result.at<unsigned int>(i,j) == 0) {
                findPixels(i-NEWSKIPPIX,j,x,y,image,result,1);
                std::cout << (double)aaa*100 << " z " << i << " z " << j << std::endl;
            }
            if (result.at<unsigned int>(i,j) == 0) {
                findPixels(i+NEWSKIPPIX,j,x,y,image,result,1);
                std::cout << (double)aaa/100 << " z " << i << " z " << j << std::endl;
            }
            if (result.at<unsigned int>(i,j-NEWSKIPPIX) == 0) {
                findPixels(i,j-NEWSKIPPIX,x,y,image,result,1);
            }
            if (result.at<unsigned int>(i,j+NEWSKIPPIX) == 0) {
                findPixels(i,j+NEWSKIPPIX,x,y,image,result,1);
            }
            return;
        }
        if (false) {
            result.at<unsigned int>(i, j) = 1;
            iteration++;
            if (result.at<unsigned int>(i-NEWSKIPPIX,j)) {
                findPixels(i-NEWSKIPPIX,j,x,y,image,result,iteration);
            }
            if (result.at<unsigned int>(i+NEWSKIPPIX,j)) {
                findPixels(i+NEWSKIPPIX,j,x,y,image,result,iteration);
            }
            if (result.at<unsigned int>(i,j-NEWSKIPPIX)) {
                findPixels(i,j-NEWSKIPPIX,x,y,image,result,iteration);
            }
            if (result.at<unsigned int>(i,j+NEWSKIPPIX)) {
                findPixels(i,j+NEWSKIPPIX,x,y,image,result,iteration);
            }
            return;
        }

    }                                               // iteration == 0 and orange pixel means start a new blob
    if (orange) {
        result.at<unsigned int>(i, j) = 255;

        std::vector< std::vector <unsigned int> >xNewBlob = {{i}};                  //store values of x and y inside this blob
        std::vector< std::vector <unsigned int> >yNewBlob = {{j}};

        //findPixels(i-NEWSKIPPIX,j,xNewBlob,yNewBlob,image,result,1);
        //findPixels(i+NEWSKIPPIX,j,xNewBlob,yNewBlob,image,result,1);
        //findPixels(i,j-NEWSKIPPIX,xNewBlob,yNewBlob,image,result,1);
        //findPixels(i,j+NEWSKIPPIX,xNewBlob,yNewBlob,image,result,1);

        x.push_back(xNewBlob[0]);                      //push the new blob-values to the x and y-arrays
        y.push_back(yNewBlob[0]);

        return;
    }


}







int main() {
    Image test = {"whereisball3.jpg",4608,2592};



    std::cout << test.name << ": " << test.x << " x " << test.y << std::endl;

    Mat image;
    image = imread(test.name);                     //load image to cv::Mat
    Mat result;
    result = Mat(image.rows, image.cols, CV_8UC1);


    if(! image.data )                               // check for invalid input
    {
        std::cout <<  "Could not open or find the image" << std::endl ;
        return -1;
    }

    if (DEBUG) {                                    // in debug, show all orange pixels found
        namedWindow( "Display frame",CV_WINDOW_NORMAL); // show the image
        imshow("Display frame", image);
        waitKey(1);
    }

    auto t = (double)getTickCount();                // check time taken

    // create vector of all orange x- and y-positions
    std::vector< std::vector<unsigned int> > xBlobs;
    std::vector< std::vector<unsigned int> > yBlobs;

    // find orange pixels, skipping every ..th pixel
    for (unsigned int i = SKIPPIXELS; i < (test.y - SKIPPIXELS); i += SKIPPIXELS) {
        for (unsigned int j = SKIPPIXELS; j < (test.x - SKIPPIXELS); j += SKIPPIXELS) {
            if (true) {//( result.at<unsigned int>(i,j) == 0 ) {
                findPixels(i, j, xBlobs, yBlobs, image, result, 0);
            }
        }

    }

    //unsigned int blobsFound = xBlobs.size();
    //quickSort(xResult,1,pixelsFound);
    //quickSort(yResult,1,pixelsFound);
    //std::cout << xResult.at( pixelsFound/2 ) << std::endl;
    //std::cout << yResult.at( pixelsFound/2 ) << std::endl;

    t = ((double)getTickCount() - t)/getTickFrequency();
    std::cout << "Times passed in ms: " << t*1000 << std::endl;

    if (DEBUG) {                                    // in debug, show a plot of the found orange pixels
        namedWindow("Resulting frame", CV_WINDOW_NORMAL);
        imshow("Resulting frame", result);
        waitKey(0);
    }

    return 0;
}
