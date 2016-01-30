#ifndef EMBATTSPOTTER_H
#define EMBATTSPOTTER_H

#include "embattspotter_global.h"
#include "opencv2/core/core.hpp"
#include <vector>

extern "C" {
  #include <vl/generic.h>
}

using namespace std;
using namespace cv;

struct spotting_sw
{
    float score;
    int posStart;
    int posEnd;
};

class EmbAttSpotter : public SegmentationBasedSpotter
{
    
public:
    EmbAttSpotter();
    void loadCorpus(string dir);
    void train(string gtFile, string imageDir, string saveAs="embAtt");
    vector<float> spot(const Mat& exemplar) {return spot(exemplar,"",1);}
    vector<struct spotting_sw> spot_sw(const Mat& exemplar) {return spot_sw(exemplar,"",1);}
    vector<float> spot(const Mat& exemplar, string word, float alpha=0.5);
    vector<struct spotting_sw> spot_sw(const Mat& exemplar, string ngram, float alpha=0.5);
};

#endif // EMBATTSPOTTER_H
