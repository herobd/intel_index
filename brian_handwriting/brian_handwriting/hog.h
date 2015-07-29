#ifndef HOG_H
#define HOG_H

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "defines.h"

using namespace cv;
using namespace std;

class HOG
{
public:
    HOG(float thresh, int cellSize, int stepSize=5, int num_bins=9);
    void compute(const Mat &img, vector<vector<float> > &descriptors, vector< Point2i > &locations);
    
private:
    float thresh;
    int cellSize;
    int stepSize;
    int num_bins;
    
    Mat computeGradient(const Mat &img);
    
};

#endif // HOG_H
