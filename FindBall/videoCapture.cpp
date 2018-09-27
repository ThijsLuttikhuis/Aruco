//
// Created by thijs on 25-9-18.
//


#include "videoCapture.h"
#define DEBUG



using namespace cv;

Mat result;
const Vec3b g_INITIALMINCOLOR = {50,122,230};
const Vec3b g_INITIALMAXCOLOR = {160,244,255};
auto g_minColor = g_INITIALMINCOLOR;               // color orange
auto g_maxColor = g_INITIALMAXCOLOR;

int g_skipPixels = 8;                               // skip every so many pixels
int g_newSkipPix = 3;                               // in an orange area, skip this many pixels
int g_minBlobPixels = 100;                          // minimum amount of orange pixels in a blob
int g_maxBlobRadius = 200;                          // maximum radius per blob



/// find all orange pixels
void findPixels(const unsigned int i, const unsigned int j, std::vector<int>&x, std::vector<int>&y, Mat image,
                const bool direction[4], char maxIteration) {

    Vec3b color = image.at<Vec3b>(i, j);            // the color of the pixel we are checking

    // assume the pixel is orange, and try to disprove it in the for loop
    bool orange = true;

    for (int k = 0; k <=2; k++) {
        if ((g_maxColor[k] < color[k]) || (color[k] < g_minColor[k]) ) {
            orange = false;
        }
    }

    // if the pixel is orange, add the x and y values to the vector, and
    // check if the color 'g_newSkipPix' pixels away from it is orange as well

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

#ifdef DEBUG
        result.at<Vec3b>(i, j) = {0, static_cast<uchar>(192 / ((g_skipPixels + g_newSkipPix) / g_newSkipPix) * maxIteration), 0};
#endif
        if (maxIteration>1) {                       // if we can go one step further into recursion,
            if (direction[0]) {                     // go (t,f,f,t) = (right and down) and find more orange pixels
                bool newDirection[4] = {true, false, false, true};
                findPixels(i + g_newSkipPix, j, x, y, image, newDirection, (unsigned char)(maxIteration - 1));
            }
            if (direction[1]) {
                bool newDirection[4] = {false, true, false, false};
                findPixels(i, j + g_newSkipPix, x, y, image, newDirection, (unsigned char)(maxIteration - 1));
            }
            if (direction[2]) {
                bool newDirection[4] = {false, true, true, false};
                findPixels(i - g_newSkipPix, j, x, y, image, newDirection, (unsigned char)(maxIteration - 1));
            }
            if (direction[3]) {
                bool newDirection[4] = {false, false, false, true};
                findPixels(i, j - g_newSkipPix, x, y, image, newDirection, (unsigned char)(maxIteration - 1));
            }
        }

    }
}

/// group pixels of certain size together into a single blob
int findBlobs(std::vector<int> &x, std::vector<int> &y, std::vector<int> &xBlob, std::vector<int> &yBlob) {
    int pixelsFound = (int)x.size();
    std::cout << "pixels found: " << pixelsFound << std::endl;

    std::vector<int> xBlobCandidate;                // candidate blobs with max size g_maxBlobRadius
    std::vector<int> yBlobCandidate;

    std::vector<int> pixelsInBlob;                  // variables for calculating the (mass)center of an orange blob
    std::vector<int> xTotalSumBlobs;
    std::vector<int> yTotalSumBlobs;

    for (int i = 0; i < (int)x.size(); i++) {       // for every orange pixel

        bool newBlob = true;                        // assume we make a new blob ..
        for (int j = 0; j < (int)xBlobCandidate.size(); j++) {
            if ( ( abs(x[i] - xBlobCandidate[j]) <= g_maxBlobRadius/2 ) && ( abs(y[i] - yBlobCandidate[j]) <= g_maxBlobRadius/2 ) ) {
                newBlob = false;                    // unless there is another blob already nearby:
                pixelsInBlob[j]++;                  // then add this pixel to that nearby blow
                xTotalSumBlobs[j] += x[i];
                yTotalSumBlobs[j] += y[i];
                break;
            }
        }
        if (newBlob) {                              // if there are no blobs nearby, create one

#ifdef DEBUG
            std::cout << std::endl;
            result.at<Vec3b>(x[i],y[i]) = {0,0,255};
            std::cout << "new blob at: x = " << x[i] << ", y = " << y[i] << std::endl;
#endif

            xBlobCandidate.push_back( x[i] );       // new blob candidate
            yBlobCandidate.push_back( y[i] );
            pixelsInBlob.push_back(0);              // new calculation for center of mass
            xTotalSumBlobs.push_back(0);
            yTotalSumBlobs.push_back(0);
        }
    }

    int printBlobPos = 0;
    bool printBlob = false;

    // for all blobs, see if there are at least n amount of pixels in that blob, if true -> the blob is an orange ball
    for (int j = 0; j < (int)xBlobCandidate.size(); j++) {

        if (pixelsInBlob[j] >= (g_minBlobPixels)) {
            xBlob.push_back(xTotalSumBlobs[j] / pixelsInBlob[j]);
            yBlob.push_back(yTotalSumBlobs[j] / pixelsInBlob[j]);
            printBlobPos++;
            printBlob = true;
        }
#ifdef DEBUG
        std::cout << std::endl;
        if (printBlob) {
            result.at<Vec3b>(xBlob[printBlobPos-1],yBlob[printBlobPos-1]) = {255,255,255};
            std::cout << "( " << pixelsInBlob[j] << " px) - final blob position at: x = " << xBlob[printBlobPos] <<
                      ", y = " << yBlob[printBlobPos] << std::endl;
        } else {
            std::cout << "( " << pixelsInBlob[j] << " px) - blob number: " << j << " too small!" << std::endl;
        }
#endif
    }
    return printBlobPos;
}

int main() {
    namedWindow("Resulting output", CV_WINDOW_NORMAL);

    // create sliders for colors
    int sliderBlueLower = g_minColor[0];
    int sliderGreenLower = g_minColor[1];
    int sliderRedLower = g_minColor[2];

    int sliderBlueUpper = g_maxColor[0];
    int sliderGreenUpper = g_maxColor[1];
    int sliderRedUpper = g_maxColor[2];

    int newSkip = g_newSkipPix;
    int skip = g_skipPixels;
    int minPix = g_minBlobPixels;
    int maxRadius = g_maxBlobRadius;

    createTrackbar("(lower) BLUE", "Resulting output", &sliderBlueLower, 255);
    createTrackbar("(lower) GREEN", "Resulting output", &sliderGreenLower, 255);
    createTrackbar("(lower) RED", "Resulting output", &sliderRedLower, 255);

    createTrackbar("(upper) BLUE", "Resulting output", &sliderBlueUpper, 255);
    createTrackbar("(upper) GREEN", "Resulting output", &sliderGreenUpper, 255);
    createTrackbar("(upper) RED", "Resulting output", &sliderRedUpper, 255);

    createTrackbar("g_skipPixels", "Resulting output", &skip, 36);
    createTrackbar("g_NEWSKIPPIXELS", "Resulting output", &newSkip, 12);
    createTrackbar("g_minBlobPixels", "Resulting output", &minPix, 1000);
    createTrackbar("g_maxBlobRadius", "Resulting output", &maxRadius, 500);

    VideoCapture cap(0);                            // capture video..
    if(!cap.isOpened())
    {
        std::cout << "Error opening video stream" << std::endl;
        return 1;
    }


    while(sliderBlueLower <= 511) {             // calculate for evah!

        auto tickTime = (double) 1 / getTickFrequency();
        auto tZero = (double) getTickCount();    // time at t=0

        Mat image;                              // save video data to Mat-rix
        cap >> image;
        if (image.empty()) {
            std::cout << "image empty??" << std::endl;
            continue; // end of video stream
        }
#ifdef DEBUG                          // show video feed
        imshow("this is you, smile! :)", image);
        result = Mat::zeros(image.rows, image.cols, CV_8UC3);
#endif

        auto tShowImage = (double) getTickCount();

        std::vector<int> xResult;                       // create vector of all orange x- and y-positions
        std::vector<int> yResult;

        // find orange pixels, skipping SKIPPIXEL pixels
        // newDirection is the (recursive) directions the function has to look into when the found pixel is indeed orange
        for (int i = g_skipPixels; i < (image.rows - g_skipPixels); i += g_skipPixels) {
            for (int j = g_skipPixels; j < (image.cols - g_skipPixels); j += g_skipPixels) {
                bool newDirection[4] = {true, true, true, true};
                findPixels(static_cast<const unsigned int>(i), static_cast<const unsigned int>(j), xResult, yResult,
                           image, newDirection,
                           static_cast<char>((g_skipPixels + g_newSkipPix) / g_newSkipPix));
            }

        }

        auto tFindPixels = (double) getTickCount();

        std::vector<int> xBlob;                         // create vector for all blobs found
        std::vector<int> yBlob;

        // find the blobs!
        int blobCount = findBlobs(xResult, yResult, xBlob, yBlob);
        std::cout << "total blobs: " << blobCount << std::endl << "_____________" << std::endl;

        auto tFindBlobs = (double) getTickCount();

#ifdef DEBUG                                    // in debug, show a plot of the found orange pixels
        // set the lower and upper bounds according to the sliders
        g_minColor = {(unsigned char)sliderBlueLower,
                      (unsigned char)sliderGreenLower,
                      (unsigned char)sliderRedLower};

        g_maxColor = {(unsigned char)sliderBlueUpper,
                      (unsigned char)sliderGreenUpper,
                      (unsigned char)sliderRedUpper};

        g_newSkipPix = newSkip+1;
        g_skipPixels = skip+1;
        g_minBlobPixels = minPix+10;
        g_maxBlobRadius = maxRadius+20;

        imshow("Resulting output", result);

        if ( (char)27==waitKey(1) ) break;



        auto tTotal = (double) getTickCount();

        // check time taken..
        std::cout << std::endl;
        std::cout << "image loading time (ms): " << (tShowImage - tZero) * (1000 * tickTime) << std::endl;
        std::cout << "calculation time findPixels (ms): " << (tFindPixels - tShowImage) * (1000 * tickTime)
                  << std::endl;
        std::cout << "calculation time findBlobs (ms): " << (tFindBlobs - tFindPixels) * (1000 * tickTime) << std::endl;
        std::cout << "check sliders/debug stuff (ms): " << (tTotal - tFindBlobs) * (1000 * tickTime) << std::endl;
        std::cout << "total time taken (ms): " << (tTotal - tZero) * (1000 * tickTime) << std::endl;
        std::cout << std::endl << "___________________" << std::endl;

#endif
    }
    // When everything done, release the video capture and write object
    cap.release();

    // Closes all the windows
    destroyAllWindows();
    return 0;
}
