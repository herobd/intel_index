#ifndef SIFTIZEDIMAGE_H
#define SIFTIZEDIMAGE_H

#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "FeaturizedImage.h"
#include "customsift.h"
#include <iostream>
using namespace cv;
using namespace std;

#define USE_CUST_SIFT 1

#define SIFT_WINDOW_SIZE 15//.2
#define SIFT_EFFECTIVE_WINDOW_SIZE 9
#define WINDOW_SIZE 5

class SIFTizedImage : public FeaturizedImage
{
public:
    SIFTizedImage(Mat &img);
    
    virtual int pixelWidth();
    virtual int pixelHeight();
    virtual int width();
    virtual int height();
    virtual Mat ref();
    
    virtual int featureLength();
    virtual vector<double> get(int i, int j);
    
private:
    Mat imgRef;
#if USE_CUST_SIFT
    vector< vector<double> > descriptors;
#else
    Mat descriptors;
#endif
    
    int fWidth;
    int fHeight;
    
//    function<void(function<void(int,int,int,int)>)> performOverIndexes;
};

#endif // SIFTIZEDIMAGE_H
