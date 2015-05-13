#ifndef MATCHKEYPOINTS_H
#define MATCHKEYPOINTS_H

#include <stdio.h>
#include <iostream>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <assert.h>
#include "codebook.h"

using namespace cv;
int matchKeypoints( int argc, char** argv );

#endif // MATCHKEYPOINTS_H
