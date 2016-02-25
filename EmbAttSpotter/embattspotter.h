#ifndef EMBATTSPOTTER_H
#define EMBATTSPOTTER_H

#define TEST_MODE 1

#include "embattspotter_global.h"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include <vector>
#include <fstream>

extern "C" {
  #include <vl/generic.h>
  #include <vl/svm.h>
  #include <vl/gmm.h>
}

#define SIFT_DIM 128
#define DESC_DIM (SIFT_DIM+2)
#define FV_DIM (2*numGMMClusters*DESC_DIM)

using namespace std;
using namespace cv;

struct spotting_sw
{
    float score;
    int posStart;
    int posEnd;
};

class EmbAttSpotter// : public SegmentationBasedSpotter
{
private:
    struct GMM_struct
    {
        float* means;
        float* covariances;
        float* priors;
    };
    struct GMM_struct _GMM;
    
    struct PCA_struct
    {
        double mean;
        Mat eigvec;
    };
    struct PCA_struct _PCA;
    
    struct AttributesModels
    {
        Mat W;
        //Mat B;
        Mat numPosSamples;
    };
    struct AttributesModels* _attModels;
    
    struct Embedding
    {
        Mat rndmatx;
        Mat rndmaty;
        int M;
        Mat matt;
        Mat mphoc;
        Mat Wx;
        Mat Wy;
    };
    struct Embedding* _embedding;
    
    vector<Mat>* _batches_cca_att;
    
    //vector<Mat>* _batches_features;
    vector<Mat>* _features_corpus;
    vector<Mat>* _feats_training;
    
    
    //Mat* _phocs;
    Mat* _phocsTr;
    
    string saveName;
    vector<int> SIFT_sizes;
    int stride;
    double magnif;
    double windowsize;   
    double contrastthreshold;
    
    int corpusSize;
    
    
    int numBatches; 
    vector<int> batches_index;//vector of size numBatches, containing the starting index of each batch
    vector<int> batches_indexEnd;
    int genericBatchSize;
    
    vector<string>* corpus_imgfiles;
    
    vector<string>* training_imgfiles;
    vector<string> training_labels;
    
    int minH;//?
    int PCA_dim;
    int num_samples_PCA;
    int numGMMClusters;
    int numSpatialX;
    int numSpatialY;
    
    vector<int> phoc_levels;
    vector<int> phoc_levels_bi;
    vector<char> unigrams;
    vector<string> bigrams;
    map<char,int> vocUni2pos;
    map<std::string,int> vocBi2pos; 
    int phocSize, phocSize_bi;
    
    
    #if TEST_MODE
        vector<string> testImages;
    #endif
    

    Mat extract_feats(const Mat& im);

    vector<Mat>* extract_FV_feats_fast_and_batch(const vector<string>& imageLocations,vector<int>* batches_index,vector<int>* batches_indexEnd, int batchSize);

    const vector<Mat>& features_corpus(bool retrain=false);
    const Mat& feats_training(bool retrain=false);

    Mat phow(const Mat& im, const struct PCA_struct* PCA_pt);
    Mat getImageDescriptorFV(const Mat& feats_m);

    const vector<Mat>& batches_cca_att();

    Mat batch_att(int batchNum);

    void learn_attributes_bagging();
    Mat cvSVM(const Mat& featsTrain, const float* labelsTrain, const Mat& featsVal, const float* labelsVal);
    double modelMap(VlSvm * svm, const Mat& featsVal, const float* labelsVal);
    
    Mat select_rows(const Mat& m, vector<int> idx);

    const struct AttributesModels& attModels(bool train=false);
    const struct Embedding& embedding(bool retrain=false);

    void learn_common_subspace();
    void cca2(Mat X, Mat Y, float reg, int d, Mat& Wx, Mat& Wy);

    const Mat& attReprTr(bool retrain=false);//correct orientation


    

    Mat scores(const Mat& query, const Mat& corpus_batch);

    Mat scores_sw(const Mat& query, const vector<Mat>& corpus_batch);

    void get_GMM_PCA(int numWordsTrain, string imageDir, string saveAs, bool retrain=false);

    void writeFloatMat(ofstream& dst, const Mat& m);
    Mat readFloatMat(ifstream& src);
    void writeFloatArray(ofstream& dst, const float* a, int size);
    float* readFloatArray(ifstream& src, int* sizeO);

    const struct PCA_struct & PCA_();
    const struct GMM_struct & GMM();

    void compute_PCA(const Mat& data, int PCA_dim);
    void compute_GMM(const Mat& bins, int numSpatialX, int numSpatialY, int numGMMClusters);



    Mat* embed_labels_PHOC(vector<string> labels);
    void computePhoc(string str, map<char,int> vocUni2pos, map<string,int> vocBi2pos, int Nvoc, vector<int> levels, int descSize, Mat *out, int instance);

    const Mat* phocsTr();//correct orientation
    
    void DoBB(const Mat& im, int* bb_x1, int* bb_x2, int* bb_y1, int* bb_y2);
    
    Mat sinMat(const Mat& x);
    Mat cosMat(const Mat& x);
    Mat& normalizeL2Columns(Mat& m);
    
    #if TEST_MODE
        void sinMat_cosMat_test();
        void normalizeL2Columns_test();
        void loadCorpus_test();
        void spot_test();
        void extract_feats_test();
        void extract_FV_feats_fast_and_batch_test();
        void features_corpus_test();
        void feats_training_test();
        void phow_test();
        void getImageDescriptorFV_test();
        void batches_cca_att_test();
    #endif
    
public:
    EmbAttSpotter(string saveName="embAttSpotter",bool useNumbers=false);
    ~EmbAttSpotter();
    void loadCorpus(string dir);
    void train(string gtFile, string imageDir, string saveAs="embAtt");
    vector<float> spot(const Mat& exemplar) {return spot(exemplar,"",1);}
    vector<struct spotting_sw> spot_sw(const Mat& exemplar) {return spot_sw(exemplar,"",1);}
    vector<float> spot(const Mat& exemplar, string word, float alpha=0.5);
    vector<struct spotting_sw> spot_sw(const Mat& exemplar, string ngram, float alpha=0.5);
    
    #if TEST_MODE
        void test();
    #endif
};

#endif // EMBATTSPOTTER_H
