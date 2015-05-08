#ifndef CUSTOMSIFT_H
#define CUSTOMSIFT_H

#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <math.h>
#include <iostream>
using namespace cv;
using namespace std;

class CustomSIFT
{
public:
    CustomSIFT();
    static void extract(const Mat &img, const vector<KeyPoint> &keyPoints, vector< vector<double> >& descriptors);
};

#endif // CUSTOMSIFT_H
