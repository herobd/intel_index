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

using namespace std;
using namespace cv;


class EnhancedBoVW
{
public:
    EnhancedBoVW();
    ~EnhancedBoVW(){if(codebook!=NULL) delete codebook;}
    vector< tuple< vector<float>, Point2i > >* getDescriptors(Mat &img);
    vector< tuple< vector< tuple<int,float> >, Point2i > >* codeDescriptors(vector< tuple< vector<float>, Point2i > >* desc);
    Codebook* makeCodebook(string dir, int codebook_size=4096);
    
private:
    double desc_thresh;
    Codebook* codebook;
    int LLC_numOfNN;
    
    void filterDesc(vector<float> &desc1, vector<vector<float> > &descriptors1, vector< Point2i > &locations, int descSize, Size blockSize1, Size blockStride, Size imgSize);
    vector<float>* getPooledDesc(vector< tuple< vector< tuple<int,float> >, Point2i > >* samples, Rect window, vector<Vec2i> spatialPyramids = {Vec2i(3,2),Vec2i(9,2)});
    void normalizeDesc(vector<float> *desc, float a=.35);
};

#endif // ENHANCEDBOVW_H
