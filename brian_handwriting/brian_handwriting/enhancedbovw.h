#ifndef ENHANCEDBOVW_H
#define ENHANCEDBOVW_H

//#include "opencv2/core/core.hpp"
#include "codebook.h"
//#include "opencv2/nonfree/features2d.hpp"
//#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/opencv.hpp"
#include <iostream>
#include <dirent.h>
#include "hog.h"
#include <tuple>

#define P_PARAMS 1
#define CONCAT 1

using namespace std;
using namespace cv;


class EnhancedBoVW
{
public:
    Codebook* codebook;
    
    EnhancedBoVW(vector<Vec2i> spatialPyramids={Vec2i(3,2),Vec2i(9,2)}, int desc_thresh=3500, int LLC_numOfNN=3, int blockSize1=20, int blockSize2=30, int blockSize3=45, int blockStride=5, int hStride=8, int vStride=8, int skip=4);
    ~EnhancedBoVW(){if(codebook!=NULL) delete codebook;}
    

    vector< tuple< vector<float>, Point2i, int > >* getDescriptors(const Mat &img) const;
//    vector< tuple< vector< tuple<int,float> >, Point2i > >* codeDescriptors(vector< tuple< vector<float>, Point2i, int > >* desc);
    vector< vector< Mat/*< float >*/ > >* codeDescriptorsIntegralImageSkip(vector< tuple< vector<float>, Point2i, int > >* desc, Mat::MSize imgsize, int skip=1) const;
    
    vector<float>* getPooledDescFastSkip(vector< vector< Mat/*< float >*/ > >* samplesIntegralImage, Rect window, vector<Vec2i> spatialPyramids = {Vec2i(3,2),Vec2i(9,2)}, int skip=1, int level=0) const;
    
    Codebook* makeCodebook(string dir, int codebook_size=4096);
    float scanImage(const Mat &img, const Mat &exemplar) const;
    float scanImage(const Mat &img, const vector<float> &exemplar, Size exemplarSize) const;
    float scanImageHorz(const Mat &img, const Mat &exemplar) const;
    float scanImageHorz(const Mat &img, const vector<float> &exemplar, Size exemplarSize) const;
    float compareImage(const Mat &img, const vector<float> &exemplar) const;
    vector<float>* featurizeImage(const Mat &img) const;
    void normalizeDesc(vector<float> *desc, float a=.35) const;
    
    void unittests();
    
    void showEncoding(const Mat &img) const;
    void printDescThreshContours(const Mat &img, int desc_thresh=3500) const;
    
private:
    double desc_thresh;
    
    int LLC_numOfNN;
    vector<Vec2i> spatialPyramids;
    
    int hStride;
    int vStride;
    int skip;
    
    int blockSize1;
    int blockSize2;
    int blockSize3;
    int blockStride;
    
    void color(Mat &heatMap, float score, float max, float min, int midI, int midJ) const;
    
    void filterDesc(vector<float> &desc1, vector<vector<float> > &descriptors1, vector< Point2i > &locations, int descSize, Size blockSize1, Size blockStride, Size imgSize) const;

    
};

#endif // ENHANCEDBOVW_H
