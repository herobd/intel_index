#ifndef HOG_H
#define HOG_H

#define USE_CV_HOG 0

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#if USE_CV_HOG
#include "opencv2/objdetect/objdetect.hpp"
#endif
#include "defines.h"

using namespace cv;
using namespace std;

class HOG
{
public:
    HOG(float thresh, int cellSize, int stepSize=5, int num_bins=9);
    void compute(const Mat &img, vector<vector<float> > &descriptors, vector< Point2i > &locations);
    void unittest();
    void show(const Mat &img);
    
private:
    float thresh;
    int blockSize;
    int cellSize;
    int stepSize;
    int num_bins;
    
    Mat computeGradient(const Mat &img);
#if USE_CV_HOG
    HOGDescriptor hog;
#endif
};

#endif // HOG_H
