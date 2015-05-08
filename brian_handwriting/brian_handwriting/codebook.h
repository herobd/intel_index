#ifndef CODEBOOK_H
#define CODEBOOK_H

#include <fstream>
#include <vector>
#include <math.h>
#include "opencv2/core/core.hpp"
using namespace std;

#define CODEBOOK_SIZE /*4096*/100

class Codebook
{
public:
    Codebook();
    int quantize(const vector<double> &term) const;
    int quantize(const vector<float> &term) const;
    int quantize(const cv::Mat &term) const;
    void push_back(vector<double> & exe);
    unsigned int size() const {return codebook.size();}
    double getInverseDocFreq(int i) const {assert(i<codebook.size()); return inverseDocFreq[i];}
    void save(string filePath);
    void readIn(string filePath);
    void readInCSV(string filePath);
    
    static bool twentythree;
    
private:
    vector< vector<double> > codebook;
    vector<double> inverseDocFreq;
};

#endif // CODEBOOK_H
