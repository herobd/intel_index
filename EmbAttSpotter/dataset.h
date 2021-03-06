#ifndef DATASET_H
#define DATASET_H


#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <vector>
#include <fstream>
#include <iostream>
#include <dirent.h>
#include <functional>



using namespace std;
using namespace cv;

class Dataset
{
//private:
//    vector<Mat> wordImages;

public:
    
    virtual const vector<string>& labels() const =0;
    virtual int size() const =0;
    virtual const Mat image(unsigned int) const =0;
    virtual ~Dataset() {}
    //virtual int size() const {return wordImages.size();}
    //virtual const Mat image(unsigned int i) const {return wordImages.at(i);}
    //virtual void preprocess(function< Mat(Mat) >& lambda)
    //{
    //    for (int i=0; i<size(); i++)
    //        wordImages[i] = lambda(wordImages[i]);
    //}
    virtual unsigned int wordId(unsigned int i) const {return i;}
};
#endif
