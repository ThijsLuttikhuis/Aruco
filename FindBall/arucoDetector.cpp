//
// Created by thijs on 25-9-18.
//

#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <math.h>
#include <iostream>
#include <vector>

#define DEBUG

using namespace cv;
int g_skipPixels = 10;
int g_whiteMargin = 210;
const int ARUCOSIZE = 3;

void findWhitePixels(int i, int j, std::vector<int> &x, std::vector<int> &y, Vec3b color, Mat result) {
    bool pixelIsWhite = true;
    for (int c = 0; c <= 2; c++) {
        if (color[c] < g_whiteMargin) {
            pixelIsWhite = false;
        }
    }
    if (pixelIsWhite) {
        result.at<Vec3b>(i/g_skipPixels,j/g_skipPixels) = {0,255,0};
        x.push_back(i);
        y.push_back(j);
    }

}

void findWhiteEdges(int i, int j, std::vector<int> &xEdge, std::vector<int> &yEdge, std::vector<int> &idEdge,
                    Mat whitePixels, Mat result) {
    // nWhite contains data for 9 pixels around the current pixel being searched for.
    Vec3b green = {0,255,0};
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            int nGreen = 0;
            if (whitePixels.at<Vec3b>(x + i, y + j) == green) {
                nGreen++;
            }
            if (whitePixels.at<Vec3b>(x + i + 1, y + j) == green) {
                nGreen++;
            }
            if (whitePixels.at<Vec3b>(x + i, y + j + 1) == green) {
                nGreen++;
            }
            if (whitePixels.at<Vec3b>(x + i + 1, y + j + 1) == green) {
                nGreen++;
            }
            if (nGreen > 0) {
                xEdge.push_back(x + i);
                yEdge.push_back(y + j);
                idEdge.push_back(nGreen);
                switch (nGreen) {
                    case 1:
                        result.at<Vec3b>(x + i, y + j) = {255, 0, 0};
                        break;
                    case 2:
                        result.at<Vec3b>(x + i, y + j) = {0, 0, 255};
                        break;
                    case 3:
                        result.at<Vec3b>(x + i, y + j) = {0, 255, 0};
                        break;
                    case 4:
                        result.at<Vec3b>(x + i, y + j) = {127, 127, 127};
                        break;
                }
            }
        }
    }

}
void findSecondRow(int i, int j, std::vector<int> &x, std::vector<int> &y, Mat whitePixels, std::vector<int> ) {


}



void identifyMarker() {
    return;
}




int main() {

    auto tickTime = (double) 1 / getTickFrequency();
    auto tZero = (double) getTickCount();                // check time taken

    Mat image;
    image = imread("test.jpg");                         //load image to cv::Mat

    Mat resultWhitePixels;
    resultWhitePixels = Mat::zeros(image.rows / g_skipPixels, image.cols / g_skipPixels, CV_8UC3);

    Mat resultWhiteEdges;
    resultWhiteEdges = Mat::zeros(image.rows / g_skipPixels - 1, image.cols / g_skipPixels - 1, CV_8UC3);


#ifdef DEBUG                          // show video feed
    namedWindow("this is you, smile! :)", CV_WINDOW_NORMAL);     // show the image
    imshow("this is you, smile! :)", image);
    waitKey(1);
#endif

    if (!image.data)                                   // check for invalid input
    {
        std::cout << "Could not open or find the image" << std::endl;
        return 1;
    }

    auto tLoadImage = (double) getTickCount();           // check time taken


    // create vector of all x- and y-positions of the white pixels
    std::vector<int> xWhitePixels;
    std::vector<int> yWhitePixels;

    for (int i = g_skipPixels; i < image.rows; i += g_skipPixels) {
        for (int j = g_skipPixels; j < image.cols; j += g_skipPixels) {
            auto &color = image.at<Vec3b>(i, j);
            findWhitePixels(i, j, xWhitePixels, yWhitePixels, color, resultWhitePixels);

        }
    }

    auto tFindWhitePixels = (double) getTickCount();    // check time taken

    // create vector of all x y positions and id of the edge-pixels
    std::vector<int> xEdgePixels;
    std::vector<int> yEdgePixels;
    std::vector<int> idEdgePixels;

    for (unsigned int pix = 0; pix < xWhitePixels.size(); pix++) {
        int x = xWhitePixels.at(pix) / g_skipPixels;
        int y = yWhitePixels.at(pix) / g_skipPixels;
        findWhiteEdges(x, y, xEdgePixels, yEdgePixels, idEdgePixels, resultWhitePixels, resultWhiteEdges);
    }

    auto tFindWhiteEdges = (double) getTickCount();    // check time taken

    std::vector<int> xLineResult;
    std::vector<int> yLineResult;

    for (unsigned int pix = 0; pix < xEdgePixels.size(); pix++) {
        if (true) {
           //findSecondRow(pix, xEdgePixels, yEdgePixels, resultWhiteEdges, xLineResult, yLineResult);
        }
    }

    auto tTraceWhiteEdges = (double)getTickCount();    // check time taken




#ifdef DEBUG                                            // in debug, show a plot of the found orange pixels
    namedWindow("Resulting white pixels", CV_WINDOW_NORMAL);
    imshow("Resulting white pixels", resultWhitePixels);
    namedWindow("Resulting white edges", CV_WINDOW_NORMAL);
    imshow("Resulting white edges", resultWhiteEdges);



    auto tTotal = (double) getTickCount();

// check time taken..
    std::cout << std::endl;
    std::cout << "image loading time (ms): " << (tLoadImage - tZero) * (1000 * tickTime) << std::endl;
    std::cout << "calculation time findWhitePixels(ms): " << (tFindWhitePixels - tLoadImage) * (1000 * tickTime)
              << std::endl;
    std::cout << "calculation time findWhiteEdges (ms): " << (tFindWhiteEdges - tFindWhitePixels) * (1000 * tickTime) << std::endl;
    std::cout << "calculation time combined function (ms): " << (tTraceWhiteEdges - tFindWhiteEdges) * (1000 * tickTime) << std::endl;

    std::cout << "debug/finishing up (ms): " << (tTotal - tFindWhiteEdges) * (1000 * tickTime) << std::endl;
    std::cout << "total time taken (ms): " << (tTotal - tZero) * (1000 * tickTime) << std::endl;
    std::cout << std::endl << "___________________" << std::endl;

#endif
    waitKey(0);

    return 0;
}
