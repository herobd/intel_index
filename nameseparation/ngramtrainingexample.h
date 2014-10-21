#ifndef NGRAMTRAININGEXAMPLE_H
#define NGRAMTRAININGEXAMPLE_H

#include "dimage.h"
#include "lrmfeaturevector.h"
#include "BPixelCollection.h"

class NGramInstance
{
public:
    NGramInstance(const BPixelCollection& pixels, int n);
    NGramInstance(const BImage& pixels, int n);
    
    int getN() const {return n;}
//    bool isKnown();
    BPixelCollection* geBPixelCollection();
    DImage* getDImage();
    LRMFeatureVector getFeatureVector(){}
    
private:
    int n;
    BImage img;
};

class NGramTrainingInstance: public NGramInstance
{
public:
    NGramTrainingInstance(QString text);
    
//    bool isKnown();
    QString getText() {return text;}
    
private:
    QString text;
};

class NGramPossibleInstance: public NGramInstance
{
public:
    NGramPossibleInstance(int start, int end);
    
//    bool isKnown();
    std::pair<int,int> getChunkWindow() {return std::pair<int,int>(windowStart,windowEnd);}
    
private:
    int windowStart;
    int windowEnd;
};


#endif // NGRAMTRAININGEXAMPLE_H
