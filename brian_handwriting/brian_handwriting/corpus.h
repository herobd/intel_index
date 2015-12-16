#ifndef CORPUS_H
#define CORPUS_H

#include "FeaturizedImage.h"
#include "siftizedimage.h"
#include <stdlib.h>
#include "opencv2/core/core.hpp"
#include "codedimage.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <fstream>
#include "hmmquery.h"
#include "codebook_2.h"

#define CODEBOOK_SAMPLE_AMOUNT .05



using namespace std;
using namespace cv;

class Corpus
{
public:
    Corpus();
    Corpus(string savedPath);
    ~Corpus();
    void addImage(Mat img);
    void generateCodebook(int codebook_size=1024);
    
    void saveQuantizedImage(string filePath, int pageNum);
    
    void addPrevImage(Mat img, int i);
    void saveCodedCorpus(string filePath);
    
    void makeHeatMaps(Mat &query);
    void makeHeatMap(Mat &query, int page);
    
    void saveCodeBook(string filePath);
    void readCodeBook(string filePath);
    
private:
    vector<FeaturizedImage*> pages;
    vector<CodedImage*> codedPages;
    Codebook codebook;
    
    vector<Vec3b> colorTable;
};

#endif // CORPUS_H
