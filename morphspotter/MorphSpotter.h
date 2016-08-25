#ifndef MORPHSPOTTER_H
#define MORPHSPOTTER_H

#include <vector>
#include <list>
#include "dmorphink.h"
#include "dthresholder.h"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "dataset.h"

#define SCRUB_THRESH 19

using namespace std;
using namespace cv;

class MorphSpotter
{
    public:
    MorphSpotter();

    double score(Mat im1, Mat im2);
    double scoreFast(Mat im1, Mat im2);
    double score_preThreshed(Mat im1, Mat im2);
    double scoreFast_preThreshed(Mat im1, Mat im2);


    void eval(const Dataset* data);

    private:
    Mat binarize(const Mat& orig);
    void scrubCCs(Mat& im);
    void cvToD(const Mat& cvI, DImage& dI);
    
    DMorphInk mobj;
};

#endif
