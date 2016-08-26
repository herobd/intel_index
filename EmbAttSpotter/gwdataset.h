#ifndef GWDATASET_H
#define GWDATASET_H


#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <vector>
#include <fstream>
#include <iostream>
#include <dirent.h>
#include <assert.h>
#include <regex>

#include "dataset.h"


using namespace std;
using namespace cv;

class GWDataset : public Dataset
{
private:
    vector<Mat> wordImages;
    vector<string> pathIms, _labels;
    vector<Rect> locs;
    
/*public:
    GWDataset(const string& queries, const string& imDir, int margin=16);
    virtual const vector<string>& labels() const;
    virtual int size() const;
    //virtual const Mat image(unsigned int i) const;
};*/
public:
    GWDataset(const string& queries, const string& imDir, int margin=16);
    virtual const vector<string>& labels() const;
    virtual int size() const;
    virtual const Mat image(unsigned int i) const;
    static Mat preprocess(Mat im);
};
#endif
