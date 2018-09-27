//
// Created by thijs on 25-9-18.
//

#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <math.h>
#include <iostream>
#include <vector>


using namespace cv;

void findPixels(const unsigned int i, const unsigned int j, std::vector<int>&x, std::vector<int>&y, Mat image,
                const bool direction[4], char maxIteration);

int findBlobs(std::vector<int> &x, std::vector<int> &y, std::vector<int> &xBlob, std::vector<int> &yBlob);

int main();
