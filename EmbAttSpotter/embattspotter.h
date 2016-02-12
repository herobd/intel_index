#ifndef EMBATTSPOTTER_H
#define EMBATTSPOTTER_H

#include "embattspotter_global.h"
#include "opencv2/core/core.hpp"
#include <vector>

extern "C" {
  #include <vl/generic.h>
}

#define SIFT_DIM ?
#define DESC_DIM (SIFT_DIM+1)
#define FV_DIM (2*numGMMClusters*DESC_DIM)

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
    struct GMM
    {
        float* means;
        float* covariances;
        float* priors;
    };
    struct GMM _GMM;
    
    struct PCA
    {
        Mat mean;
        Mat eigvec;
    };
    struct PCA _PCA;
    
    struct AttributesModels
    {
        Mat W;
        //Mat B;
        Mat numPosSamples;
    }
    struct AttributesModels* _attModels;
    
    vector<Mat>* _batches_features;
    
    Mat* _phocs;
    
    string saveName;
    vector<int> SIFT_sizes;
    int stride;
    double magnif;
    double windowsize;   
    double contrastthreshold;
    
    int corpusSize;
    vector<string> corpusLabels;
    
    int numBatches; 
    vector<int> batches_index;//vector of size numBatches, containing the starting index of each batch
    vector<int> batches_indexEnd;
    
    int minH;//?
    int PCA_dim;
    int num_samples_PCA;
    int numGMMClusters;
    int numSpatialX;
    int numSpatialY;
    
    vector<int> phoc_levels;
    vector<char> unigrams;
    vector<string> bigrams;
    
public:
    EmbAttSpotter(string saveName="embAttSpotter");
    void loadCorpus(string dir);
    void train(string gtFile, string imageDir, string saveAs="embAtt");
    vector<float> spot(const Mat& exemplar) {return spot(exemplar,"",1);}
    vector<struct spotting_sw> spot_sw(const Mat& exemplar) {return spot_sw(exemplar,"",1);}
    vector<float> spot(const Mat& exemplar, string word, float alpha=0.5);
    vector<struct spotting_sw> spot_sw(const Mat& exemplar, string ngram, float alpha=0.5);
};

#endif // EMBATTSPOTTER_H
