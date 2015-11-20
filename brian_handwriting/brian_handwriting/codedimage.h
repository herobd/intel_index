#ifndef CODEDIMAGE_H
#define CODEDIMAGE_H

#include "FeaturizedImage.h"
#include "opencv2/core/core.hpp"
#include "codebook_2.h"

class CodedImage
{
public:
    CodedImage(FeaturizedImage &img, Codebook &codebook);
    CodedImage(int width, int height);
    virtual int pixelWidth();
    virtual int pixelHeight();
    virtual int width();
    virtual int height();
    virtual cv::Mat ref();
    
    virtual int featureLength();
    virtual int get(int i, int j);
    void setImg(cv::Mat &img);
    void set(int i, int j, int v);
    function<void(function<void(int,int,int,int)>)> performOverIndexes;
    
private:
    vector<int> data;
    
    int fWidth;
    int fHeight;
    cv::Mat imgRef;
    
};



#endif // CODEDIMAGE_H

