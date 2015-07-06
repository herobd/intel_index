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
    
    EnhancedBoVW();
    ~EnhancedBoVW(){if(codebook!=NULL) delete codebook;}
    
#if CONCAT
    vector< tuple< vector<float>, Point2i, int > >* getDescriptors(const Mat &img);
//    vector< tuple< vector< tuple<int,float> >, Point2i > >* codeDescriptors(vector< tuple< vector<float>, Point2i, int > >* desc);
    vector< vector< Mat/*< float >*/ > >* codeDescriptorsIntegralImage(vector< tuple< vector<float>, Point2i, int > >* desc, Mat::MSize imgsize);
    vector< vector< Mat/*< float >*/ > >* codeDescriptorsIntegralImageSkip(vector< tuple< vector<float>, Point2i, int > >* desc, Mat::MSize imgsize, int skip=1);
#else
    vector< tuple< vector<float>, Point2i > >* getDescriptors(const Mat &img);
//    vector< tuple< vector< tuple<int,float> >, Point2i > >* codeDescriptors(vector< tuple< vector<float>, Point2i > >* desc);
    vector< vector< Mat/*< float >*/ > >* codeDescriptorsIntegralImage(vector< tuple< vector<float>, Point2i > >* desc, Mat::MSize imgsize);
#endif
    
    Codebook* makeCodebook(string dir, int codebook_size=4096);
    float scanImage(const Mat &img, const Mat &exemplar);
    float scanImage(const Mat &img, const vector<float> &exemplar, Size exemplarSize);
    vector<float>* featurizeImage(const Mat &img);
    
private:
    double desc_thresh;
    
    int LLC_numOfNN;
    vector<Vec2i> spatialPyramids;
    
    int hStride;
    int vStride;
    int skip;
    
    void color(Mat &heatMap, float score, float max, float min, int midI, int midJ);
    
    void filterDesc(vector<float> &desc1, vector<vector<float> > &descriptors1, vector< Point2i > &locations, int descSize, Size blockSize1, Size blockStride, Size imgSize);
    vector<float>* getPooledDesc(vector< tuple< vector< tuple<int,float> >, Point2i > >* samples, Rect window, vector<Vec2i> spatialPyramids = {Vec2i(3,2),Vec2i(9,2)});
vector<float>* getPooledDescFast(vector< vector< Mat/*< float >*/ > >* samplesIntegralImage, Rect window, vector<Vec2i> spatialPyramids = {Vec2i(3,2),Vec2i(9,2)});
vector<float>* getPooledDescFastSkip(vector< vector< Mat/*< float >*/ > >* samplesIntegralImage, Rect window, vector<Vec2i> spatialPyramids = {Vec2i(3,2),Vec2i(9,2)}, int skip=1);
    void normalizeDesc(vector<float> *desc, float a=.35);
};

#endif // ENHANCEDBOVW_H
