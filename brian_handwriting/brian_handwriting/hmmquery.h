#ifndef HMMQUERY_H
#define HMMQUERY_H
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "corpus.h"
#include <iostream>
#include "siftizedimage.h"
#include "codebook.h"
#include "customsift.h"
#include <random>
//#include <HMMlib/hmm_table.hpp>
//#include <HMMlib/hmm_vector.hpp>
//#include <HMMlib/hmm.hpp>
//#include "hmm.h"
//#include "trellis.h"
//#include "seqTracks.h"

using namespace std;
using namespace cv;
//using namespace hmmlib;

#define SCALE .17
#define SPACING 5
#define NUM_MIXTURES 10//ODEBOOK_SIZE

class HMMQuery
{
public:
    HMMQuery(Mat &img, const Codebook* codebook, /**/vector<Vec3b> &colorTable);
    
    double eval(const vector< vector<int> > &observation);
    double logEval(const vector< vector<int> > &observation);
    int height(){return fHeight;}
    int width(){return fWidth;}
    
private:
    int numStates;
    int timeSpan;
    const Codebook* codebook;
    int fHeight;
    int fWidth;
    vector< vector<int> > termVectors;
    
    vector< vector<double> > c;//output coefficent [state][feature]
    vector < map<int,double> > a;//transition probabilities [from_state][to_state]
    vector<double> initProb;
    
    vector< vector< Mat > > mu;
    vector< vector< Mat > > U;
    
    
    double prob(const vector<int> &obs, int state);
    void SIFTize(Mat &img);
    void baumwelch();
    Mat makeO(const vector<int> &oVec);
    double gauss(const vector<int> &oVec, const Mat &mu, const Mat &U);
//    int quantize(vector<double> term);
    
//    boost::shared_ptr<HMMVector<double> > initProb_ptr;
//    boost::shared_ptr<HMMMatrix<double> > transProb_ptr;
//    boost::shared_ptr<HMMMatrix<double> > emmProb_ptr;
    
};

#endif // HMMQUERY_H
