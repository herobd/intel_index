#ifndef SEGMENTATIONBASEDPOTTER_H
#define SEGMENTATIONBASEDPOTTER_H

#include "opencv2/core/core.hpp"

using namespace std;
using namespace cv;

struct spotting_sw
{
    float score;
    int posStart;
    int posEnd;
};

class SegmentationBasedSpotter
{
    
public:
    SegmentationBasedSpotter();
    void loadCorpus(string dir)=0;
    vector<float> spot(const Mat& exemplar)=0;
    vector<struct spotting_sw> spot_sw(const Mat& exemplar)=0;
};

#endif // SEGMENTATIONBASEDPOTTER_H
