#ifndef EMBATTSPOTTER_H
#define EMBATTSPOTTER_H

#define TEST_MODE 1
#define DRAW 0

#define USE_VL 1

#include "embattspotter_global.h"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include <vector>
#include <deque>
#include <fstream>
#include <iostream>
#include <dirent.h>
#include <iomanip>
#include <functional>
#include "dataset.h"



#if USE_VL
extern "C" {
  #include "vlfeat-0.9.20/vl/generic.h"
  #include "vlfeat-0.9.20/vl/svm.h"
  #include "vlfeat-0.9.20/vl/gmm.h"
  #include "vlfeat-0.9.20/vl/fisher.h"
  #include "vlfeat-0.9.20/vl/dsift.h"
  #include "vlfeat-0.9.20/vl/imopv.h"
}
#endif

#include <math.h>

#define SIFT_DIM 128
#define DESC_DIM (SIFT_DIM+2)
#define FV_DIM (2*numGMMClusters*numSpatialX*numSpatialY*AUG_PCA_DIM)
#define AUG_PCA_DIM (PCA_dim+2)

using namespace std;
using namespace cv;

struct spotting_sw
{
    float score;
    int posStart;
    int posEnd;
};
struct SubwordSpottingResult {
    int imIdx;
    float score;
    int startX;
    int endX;
    SubwordSpottingResult(int imIdx, float score, int startX, int endX) : 
        imIdx(imIdx), score(score), startX(startX), endX(endX)
    {
    }
    SubwordSpottingResult() : 
        imIdx(-1), score(0), startX(-1), endX(-1)
    {
    }
};

class EmbAttSpotter// : public SegmentationBasedSpotter
{
private:
    
    
    ////
    ////Lazy elements
    ////
    struct GMM_struct
    {
        float* means;
        float* covariances;
        float* priors;
    };
    GMM_struct _GMM;
    
    struct PCA_struct
    {
        Mat mean;
        Mat eigvec;
    };
    PCA_struct _PCA;
    
    struct AttributesModels
    {
        Mat W;
        //Mat B;
        Mat numPosSamples;
    };
    AttributesModels* _attModels;
    
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
    Embedding* _embedding;
    
    Mat _attReprTr;
    
    vector<Mat>* _batches_cca_att;
    vector<Mat>* _subwordWindows_cca_att_saved;
    
    vector<Mat>* _features_corpus;
    Mat _feats_training;
    
    Mat _phocsTr;
    
    vector<Mat> _corpus_phows;    
    vector<vector<int> > _corpus_phows_xs;

    double _averageCharWidth;
    ////
    ////End lazy elements
    ////
    
    
    
    
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
    int genericBatchSize;//This gets set to detirmine how many iamges go into a batch
    
    const Dataset* training_dataset;
    const Dataset* corpus_dataset;
    //Or, these are paths to the images.
    vector<string>* corpus_imgfiles;
    vector<string>* training_imgfiles;//when training files are "loaded" this becomes poulated
    vector<string>* training_labels;//when training files are "loaded" this becomes poulated
    
    int numWordsTrainGMM;
    int minH;//?
    int PCA_dim;
    int num_samples_PCA;
    int G;
    int numGMMClusters;
    int numSpatialX;
    int numSpatialY;
    
    vector<float> sgdparams_lbds;
    
    vector<int> phoc_levels;
    vector<int> phoc_levels_bi;
    vector<char> unigrams;
    vector<string> bigrams;
    map<char,int> vocUni2pos;
    map<std::string,int> vocBi2pos; 
    int phocSize, phocSize_bi;
    
    function< Mat(Mat) > pp;    
    #if TEST_MODE
        vector<string> testImages;
    #endif
    

    Mat extract_feats(const Mat& im);
    Mat extract_feats(const Mat& im) const;

    vector<Mat>* extract_FV_feats_fast_and_batch(const vector<string>& imageLocations,vector<int>* batches_index,vector<int>* batches_indexEnd, int batchSize);
    vector<Mat>* extract_FV_feats_fast_and_batch(const Dataset* dataset,vector<int>* batches_index,vector<int>* batches_indexEnd, int batchSize);
    Mat get_FV_feats(const Dataset* dataset);

    const vector<Mat>& features_corpus(bool retrain=false);
    const Mat& feats_training(bool retrain=false);

    Mat phow(const Mat& im, const struct PCA_struct* PCA_pt=NULL, vector<int>* xs=NULL) const;
    Mat getImageDescriptorFV(const Mat& feats_m);
    Mat getImageDescriptorFV(const Mat& feats_m) const;

    const vector<Mat>& batches_cca_att();
    Mat subwordWindows_cca_att_saved(int imIdx, int windWidth, int stride);
    const Mat subwordWindows_cca_att_saved(int imIdx, int windWidth, int stride) const;

    Mat batch_att(int batchNum);

    void learn_attributes_bagging();
    Mat cvSVM(const Mat& featsTrain, const double* labelsTrain, const Mat& featsVal, const float* labelsVal, VlSvm ** bestsvm);
    double modelMap(int model_size, double const* svm, const Mat& featsVal, const float* labelsVal);
    
    Mat select_rows(const Mat& m, vector<int> idx) const;

    const AttributesModels& attModels(bool retrain=false);
    

    void learn_common_subspace();
    void cca2(Mat X, Mat Y, float reg, int d, Mat& Wx, Mat& Wy);

    


    

    /*Mat scores(const Mat& query, const Mat& corpus_batch);

    Mat scores_sw(const Mat& query, const vector<Mat>& corpus_batch);*/

    void get_GMM_PCA(int numWordsTrain, string saveAs, bool retrain=false);

    //This are my homemade functions to write data to a file and read it back
    void writeFloatMat(ofstream& dst, const Mat& m);
    Mat readFloatMat(ifstream& src);
    void writeFloatArray(ofstream& dst, const float* a, int size);
    float* readFloatArray(ifstream& src, int* sizeO=NULL);

    //PCA, but this is only used to compute GMM. Lazy
    const PCA_struct & PCA_(bool retrain=false);
    
    //The GMM parameters for the Fisher Vector. Lazy
    const GMM_struct & GMM(bool retrain=false);

    void compute_PCA(const Mat& data, int PCA_dim);
    void compute_GMM(const vector<Mat>& bins, int numSpatialX, int numSpatialY, int numGMMClusters);


    //creates the PHOC for each given string, stores in the columns of the returned Mat 
    Mat embed_labels_PHOC(const vector<string>& labels);
    
    //This does EITHER unigrams or bigrams, but fills them into the right spot (you call it twice).
    void computePhoc(string str, map<char,int> vocUni2pos, map<string,int> vocBi2pos, int Nvoc, vector<int> levels, int descSize, Mat &out, int instance) const;
    
    //The PHOCs of the training set. Lazy
    const Mat& phocsTr(bool retrain=false);//correct orientation
    
    //Returns an estimated average char width from corpus dataset and a fixed ratio. Lazy
    double averageCharWidth();

    //Computes a bounding box roughly capturing the word lengthwise and the baselines
    void DoBB(const Mat& im, int* bb_x1, int* bb_x2, int* bb_y1, int* bb_y2) const;
    
    //Compute element-wise sine or cosine
    Mat sinMat(const Mat& x) const;
    Mat cosMat(const Mat& x) const;
    
    //L2-Normalize the columns of the matrix
    Mat& normalizeL2Columns(Mat& m) const;
    
    Mat otsuBinarization(const Mat& src) const;
    
    SubwordSpottingResult refine(float score, int imIdx, int windIdx, int windWidth, int stride, const Mat& query_cca) const;
    Mat subwordWindows_cca_att(int imIdx, int windWidth, int stride);
    Mat subwordWindows_cca_att(int imIdx, int windWidth, int stride) const;
    Mat subword_cca_att(int imIdx, int windS, int windE);
    Mat subword_cca_att(int imIdx, int windS, int windE) const;
    Mat phowsByX(int i, int xS, int xE);
    Mat phowsByX(int i, int xS, int xE) const;
    int test_mode;
    
    #if TEST_MODE
        void sinMat_cosMat_test();
        void normalizeL2Columns_test();
        void otsuBinarization_test();
        void DoBB_test();
        void loadCorpus_test();
        void spot_test();
        void extract_feats_test();
        void extract_FV_feats_fast_and_batch_test();
        void features_corpus_test();
        void feats_training_test();
        void phow_test();
        void getImageDescriptorFV_test();
        void batches_cca_att_test();
        void embed_labels_PHOC_test();
        void phocsTr_testM();
        void get_GMM_PCA_testM();
        void feats_training_testM();
        void learn_attributes_bagging_test();
        
        void readCSV(string fileName, vector< vector<float> >& out);
        void readCSV(string fileName, Mat& out, bool trans=false);
        void compareToCSV(Mat mine, string csvloc, bool transpose=false, float thresh=0.001);
        void compareToCSVAbs(Mat mine, string csvloc, bool transpose=false, float thresh=0.001);
        void compareToMat(Mat mine, Mat other, float thresh=0.001);

        void cvSVM_isotest();
        void compute_PCA_isotest();
        void compute_GMM_isotest();
        void modelMap_isotest();
        void subwordSpot_test();
    #endif
    
public:
    const Mat& attReprTr(bool retrain=false);//correct orientation
    const Embedding& embedding(bool retrain=false);

    EmbAttSpotter(string saveName="embAttSpotter", bool load=false ,bool useNumbers=true, int test_mode=0);
    ~EmbAttSpotter();
    void loadCorpus(string dir);
    void setTrainData(string gtFile, string imageDir, string saveAs="");
    vector<float> spot(const Mat& exemplar) {return spot(exemplar,"",1);}
    vector< SubwordSpottingResult > subwordSpot(const Mat& exemplar, string word, float alpha, float refinePortion=0.25);
    vector< SubwordSpottingResult > subwordSpot(const Mat& exemplar, string word, float alpha, float refinePortion=0.25) const;
    float compare(const Mat& im1, const Mat& im2);
    float compare(string text, const Mat& im);
    float compare(string text, const Mat& im) const;
    vector<float> spot(const Mat& exemplar, string word, float alpha=0.5);
    
    void setTraining_dataset(const Dataset* d);
    void setCorpus_dataset(const Dataset* d, bool load=false);
    
    void eval(const Dataset* data);
    void evalSpotting(const Dataset* exemplars, const Dataset* data, double hyV=-1);
    
    #if TEST_MODE
        void test();
    #else
        void test() {}
    #endif
    
    void retrain()
    {
        system(("rm "+saveName+"*.dat").c_str());
    }
    void primeSubwordSpotting(int len);
};

#endif // EMBATTSPOTTER_H
