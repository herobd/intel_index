#ifndef ENHANCEDBOVW_H
#define ENHANCEDBOVW_H

//#include "opencv2/core/core.hpp"
#include "codebook_2.h"
//#include "opencv2/nonfree/features2d.hpp"
//#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/opencv.hpp"
#include <iostream>
#include <dirent.h>
#include "hog.h"
#include <tuple>
#include "preprocessor.h"
//#define NO_LLC 1
//#define NO_SP_PY 1
//#define NO_POW_NORM 1


#define P_PARAMS 1
#define CONCAT 1

using namespace std;
using namespace cv;

#define SMALL 0
#define MED 1
#define LARGE 2

#define DESC_THRESH 2700
typedef struct description
{
    vector<float> values;
    Point2i position;
    int scale;
    description(const vector<float>& values, const Point2i& position, int scale)
    {
        this->values = values;
        this->position = position;
        this->scale = scale;
    }
} description;

class EnhancedBoVW
{
public:
    Codebook* codebook;
    Codebook* codebook_small;
    Codebook* codebook_med;
    Codebook* codebook_large;
    
//#if !NO_LLC && !NO_SP_PY
    EnhancedBoVW(vector<Vec2i> spatialPyramids={Vec2i(3,2),Vec2i(9,2)}, int desc_thresh=DESC_THRESH, int LLC_numOfNN=10, int blockSize1=20, int blockSize2=30, int blockSize3=45, int blockStride=3, int hStride=3, int vStride=3, int skip=3, string savePrefix="./bovw_save/encode", string saveExt=".dat");
/*#elif !NO_SP_PY
    EnhancedBoVW(vector<Vec2i> spatialPyramids={Vec2i(3,2),Vec2i(9,2)}, int desc_thresh=DESC_THRESH, int LLC_numOfNN=1, int blockSize1=20, int blockSize2=30, int blockSize3=45, int blockStride=3, int hStride=3, int vStride=3, int skip=3);
#else
    EnhancedBoVW(vector<Vec2i> spatialPyramids={Vec2i(1,1)}, int desc_thresh=DESC_THRESH, int LLC_numOfNN=1, int blockSize1=20, int blockSize2=30, int blockSize3=45, int blockStride=3, int hStride=3, int vStride=3, int skip=3);
#endif*/
    ~EnhancedBoVW(){if(codebook!=NULL) delete codebook;}
    void readCodebooks(string loc)
    {
        codebook_small = new Codebook();
        codebook_small->readIn(loc+"_small");
        codebook_med = new Codebook();
        codebook_med->readIn(loc+"_med");
        codebook_large = new Codebook();
        codebook_large->readIn(loc+"_small");
    }
    void setPre(Preprocessor pre) {this->pre=pre;}
    void writeCodebooks(string loc)
    {
        codebook_small->save(loc+"_small");
        codebook_med->save(loc+"_med");
        codebook_large->save(loc+"_large");
    }

    vector<description> *getDescriptors(const Mat &img) const;
//    vector< tuple< vector< tuple<int,float> >, Point2i > >* codeDescriptors(vector< tuple< vector<float>, Point2i, int > >* desc);
    vector< vector< Mat/*< float >*/ > >* codeDescriptorsIntegralImageSkip(vector<description> *desc, Mat::MSize imgsize, int skip=1) const;
    
    vector<float>* getPooledDescFastSkip(const vector< vector< Mat/*< float >*/ > >* samplesIntegralImage, Rect window, vector<Vec2i> spatialPyramids = {Vec2i(3,2),Vec2i(9,2)}, int skip=1, int level=0) const;
    
    Codebook* makeCodebook(string dir, int codebook_size=4096);
    void make3Codebooks(string directory, int codebook_size=4096);
    vector< tuple<int,float> > quantizeSoft(const vector<float> &term, int t, int scale) const;
    float scanImage(const Mat &img, const Mat &exemplar) const;
    float scanImage(const Mat &img, const vector<float> &exemplar, Size exemplarSize) const;
    float scanImageHorz(const Mat &img, const Mat &exemplar) const;
    float scanImageHorz(const Mat &img, const vector<float> &exemplar, Size exemplarSize) const;
    float scanImageHorz(int imageIdx, const vector<float> &exemplar, Size exemplarSize) const;
    float scanImageHorz(const vector< vector< Mat > >* samplesCodedII, Size preprocessedSize, const vector<float> &exemplar, Size exemplarSize) const;
    float compareImage(const Mat &img, const vector<float> &exemplar) const;
    vector< vector< Mat/*< float >*/ > >* encodeImage(const Mat &img, Size* preprocessedSize=NULL) const;
    void encodeAndSaveImage(const Mat &img, int imageIdx) const;
    vector<float>* featurizeImage(const Mat &img, Size* preprocessedSize=NULL) const;
    void normalizeDesc(vector<float> *desc, float a=.35) const;
    
    int featureSize() const
    {
        int codebook_m;
        if (codebook != NULL)
            codebook_m = codebook->size()*3;
        else
            codebook_m = codebook_small->size() + codebook_med->size() + codebook_large->size();
        int py_m=0;
        for (Vec2i level : spatialPyramids)
        {
            py_m += level[0]*level[1];
        }
        return codebook_m * py_m;
    }
    
    void unittests();
    
    void showEncoding(const Mat &img) const;
    void printDescThreshContours(const Mat &img, int desc_thresh=3500) const;
    void setLLC(int llc) {LLC_numOfNN=llc;}
    
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
    
    Preprocessor pre;
    string savePrefix;
    string saveExt;
    //Codebook* computeCodebookFromExamples(int codebook_size,vector< vector<float> >& accum);
    
};

#endif // ENHANCEDBOVW_H
