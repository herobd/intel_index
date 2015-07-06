#ifndef FEATURIZEDIMAGE_H
#define FEATURIZEDIMAGE_H

#include <vector>
#include "opencv2/core/core.hpp"
#include <functional>
using namespace std;

class FeaturizedImage
{
public:
    virtual int pixelWidth() =0;
    virtual int pixelHeight() =0;
    virtual int width() =0;
    virtual int height() =0;
    virtual cv::Mat ref() =0;
    
    virtual int featureLength() =0;
    virtual vector<double> get(int, int) =0;
    function<void(function<void(int,int,int,int)>)> performOverIndexes;
};

#endif // FEATURIZEDIMAGE_H
