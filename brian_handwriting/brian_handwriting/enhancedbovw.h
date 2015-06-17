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

using namespace std;
using namespace cv;


class EnhancedBoVW
{
public:
    Codebook* codebook;
    
    EnhancedBoVW();
    ~EnhancedBoVW(){if(codebook!=NULL) delete codebook;}
    vector< tuple< vector<float>, Point2i > >* getDescriptors(const Mat &img);
    vector< tuple< vector< tuple<int,float> >, Point2i > >* codeDescriptors(vector< tuple< vector<float>, Point2i > >* desc);
    vector< vector< Mat/*< float >*/ > >* codeDescriptorsIntegralImage(vector< tuple< vector<float>, Point2i > >* desc, Mat::MSize imgsize);
    Codebook* makeCodebook(string dir, int codebook_size=4096);
    void scanImage(const Mat &img, const Mat &exemplar);
    void scanImage(const Mat &img, const vector<float> &exemplar, Size exemplarSize);
    
private:
    double desc_thresh;
    
    int LLC_numOfNN;
    vector<Vec2i> spatialPyramids;
    
    void color(Mat &heatMap, float score, float max, float min, int midI, int midJ);
    
    void filterDesc(vector<float> &desc1, vector<vector<float> > &descriptors1, vector< Point2i > &locations, int descSize, Size blockSize1, Size blockStride, Size imgSize);
    vector<float>* getPooledDesc(vector< tuple< vector< tuple<int,float> >, Point2i > >* samples, Rect window, vector<Vec2i> spatialPyramids = {Vec2i(3,2),Vec2i(9,2)});
vector<float>* getPooledDescFast(vector< vector< Mat/*< float >*/ > >* samplesIntegralImage, Rect window, vector<Vec2i> spatialPyramids = {Vec2i(3,2),Vec2i(9,2)});
    void normalizeDesc(vector<float> *desc, float a=.35);
};

#endif // ENHANCEDBOVW_H
