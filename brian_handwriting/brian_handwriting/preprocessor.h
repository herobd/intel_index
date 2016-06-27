#ifndef PREPRO_H
#define PREPRO_H

#include "defines.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <assert.h>

using namespace std;
using namespace cv;

#define PP_BASELINE_CENTER 1
#define PP_BASELINE_NORMALIZE 2

class Preprocessor
{
public:
    Preprocessor(unsigned int flags=PP_BASELINE_CENTER);
    Preprocessor(const Preprocessor &other)
    {
        doBaselineCenter=other.doBaselineCenter;
        doBaselineNormalization=other.doBaselineNormalization;
        baseLineNorm=other.baseLineNorm;
    }
    Mat process(const Mat& orig) const;

    void setBaseLineNorm(int v) {baseLineNorm=v;}
private:
    bool doBaselineCenter;
    bool doBaselineNormalization;

    int baseLineNorm;
};

#endif
