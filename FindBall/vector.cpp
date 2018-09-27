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
    char name[30];                                  // name of the picture
    int x;                                          // amount of pixels in the x-direction
    int y;                                          // amount of pixels in the y-direction
};

Mat result;
const int SKIPPIXELS = 20;
const int NEWSKIPPIX = 8;
const int MINBLOBSIDELENGTH = 150;
const int MAXBLOBRADIUS = 240;


// (looking at x-position, looking at y-position, found x positions, found y positions, image, directions: right/up/left/down, maximum iterations)
void findPixels(const unsigned int i, const unsigned int j, std::vector<int>&x, std::vector<int>&y, Mat image,
                const bool direction[4], char maxIteration) {

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

    // if the pixel is orange, add the x and y values to the vector, and
    // check if the color 'NEWSKIPPIX' pixels away from it is orange as well

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
            result.at<unsigned char>(i, j) = (unsigned char)(63 / ((SKIPPIXELS+NEWSKIPPIX) / NEWSKIPPIX) * maxIteration);
        }
        if (maxIteration>1) {                       // if we can go one step further into recursion,
            if (direction[0]) {                     // go (t,f,f,t) = (right and down) and find more orange pixels
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
}

int findBlobs(std::vector<int> &x, std::vector<int> &y, std::vector<int> &xBlob, std::vector<int> &yBlob) {
    int pixelsFound = (int)x.size();
    std::cout << "pixels found: " << pixelsFound << std::endl;

    std::vector<int> xBlobCandidate;                // candidate blobs with max size MAXBLOBRADIUS
    std::vector<int> yBlobCandidate;

    std::vector<int> pixelsInBlob;                  // variables for calculating the (mass)center of an orange blob
    std::vector<int> xTotalSumBlobs;
    std::vector<int> yTotalSumBlobs;

    for (int i = 0; i < (int)x.size(); i++) {       // for every orange pixel

        bool newBlob = true;                        // assume we make a new blob ..
        for (int j = 0; j < (int)xBlobCandidate.size(); j++) {
            if ( ( abs(x[i] - xBlobCandidate[j]) <= MAXBLOBRADIUS/2 ) && ( abs(y[i] - yBlobCandidate[j]) <= MAXBLOBRADIUS/2 ) ) {
                newBlob = false;                    // unless there is another blob already nearby:
                pixelsInBlob[j]++;                  // then add this pixel to that nearby blow
                xTotalSumBlobs[j] += x[i];
                yTotalSumBlobs[j] += y[i];
                break;
            }
        }
        if (newBlob) {                              // if there are no blobs nearby, create one

            if (DEBUG) {
                result.at<unsigned char>(x[i],y[i]) = (unsigned char)(127-xBlobCandidate.size());
                std::cout << "new blob at: x = " << x[i] << ", y = " << y[i] << std::endl;
            }

            xBlobCandidate.push_back( x[i] );       // new blob candidate
            yBlobCandidate.push_back( y[i] );
            pixelsInBlob.push_back(0);              // new calculation for center of mass
            xTotalSumBlobs.push_back(0);
            yTotalSumBlobs.push_back(0);
        }
    }

    int printBlobPos = -1;
    bool printBlob = false;

    // for all blobs, see if there are at least n amount of pixels in that blob, if true -> the blob is an orange ball
    for (int j = 0; j < (int)xBlobCandidate.size(); j++) {

        if (pixelsInBlob[j] >= (MINBLOBSIDELENGTH * MINBLOBSIDELENGTH) / (SKIPPIXELS * SKIPPIXELS) ) {
            xBlob.push_back(xTotalSumBlobs[j] / pixelsInBlob[j]);
            yBlob.push_back(yTotalSumBlobs[j] / pixelsInBlob[j]);
            printBlobPos++;
            printBlob = true;
        } else printBlob = false;
        if (DEBUG) {
            if (printBlob) {
                result.at<unsigned char>(xBlob[printBlobPos],yBlob[printBlobPos]) = 255;
                std::cout << "( " << pixelsInBlob[j] << " px) - final blob position at: x = " << xBlob[printBlobPos] <<
                          ", y = " << yBlob[printBlobPos] << std::endl;
            } else {
                std::cout << "( " << pixelsInBlob[j] << " px) - blob number: " << j << " too small!" << std::endl;
            }
        }
    }
    return printBlobPos;
}

int main() {
    auto tZero = (double) getTickCount();
    Image test = {"whereisball4.jpg", 4608, 2592};

    std::cout << test.name << ": " << test.x << " x " << test.y << std::endl;

    auto tLoadImage = (double) getTickCount();
    Mat image;
    image = imread(test.name);                     //load image to cv::Mat

    if (DEBUG) {                                    // in debug, show all orange pixels found
        result = Mat(image.rows, image.cols, CV_8UC1);
        namedWindow("Display frame", CV_WINDOW_NORMAL);
        imshow("Display frame", image);             // show the image
        waitKey(1);
    }

    auto tShowImage = (double) getTickCount();

    std::vector<int> xResult;                       // create vector of all orange x- and y-positions
    std::vector<int> yResult;

    // find orange pixels, skipping SKIPPIXEL pixels
    // newDirection is the (recursive) directions the function has to look into when the found pixel is indeed orange
    for (int i = SKIPPIXELS; i < (test.y - SKIPPIXELS); i += SKIPPIXELS) {
        for (int j = SKIPPIXELS; j < (test.x - SKIPPIXELS); j += SKIPPIXELS) {
            bool newDirection[4] = {true, true, true, true};
            findPixels(i, j, xResult, yResult, image, newDirection, ((SKIPPIXELS + NEWSKIPPIX) / NEWSKIPPIX));
        }

    }

    auto tFindPixels = (double) getTickCount();

    std::vector<int> xBlob;                         // create vector for all blobs found
    std::vector<int> yBlob;

    // find the blobs!
    int blobCount = findBlobs(xResult, yResult, xBlob, yBlob);
    std::cout << "total blobs: " << blobCount << std::endl;

    auto tFindBlobs = (double) getTickCount();
    auto tTotal = (double) getTickCount();

    // check time taken..
    auto tickTime = (double) 1 / getTickFrequency();

    std::cout << "image loading time (ms): " << (tLoadImage - tZero) * (1000 * tickTime) << std::endl;
    std::cout << "calculation time findPixels (ms): " << (tFindPixels - tShowImage) * (1000 * tickTime)
              << std::endl;
    std::cout << "calculation time findBlobs (ms): " << (tFindBlobs - tFindPixels) * (1000 * tickTime) << std::endl;
    std::cout << "total time taken (ms): " << (tTotal - tZero) * (1000 * tickTime) << std::endl;


    if (DEBUG) {                                    // in debug, show a plot of the found orange pixels
        std::cout << "(debug) - display image time (ms): " << (tShowImage - tLoadImage) * (1000 * tickTime)
                  << std::endl;
        namedWindow("Resulting frame", CV_WINDOW_NORMAL);
        imshow("Resulting frame", result);
        waitKey(0);
    }




    return 0;
}
