#ifndef SEARCHER_H
#define SEARCHER_H
#include "corpus.h"

using namespace std;
using namespace cv;

class Searcher
{
public:
    Searcher();
    HMMQuery createModel(Mat &img);
    void setCorpus(const Corpus &corpus);
    vector<Mat> topMatches(Mat &query);
    
};

#endif // SEARCHER_H
