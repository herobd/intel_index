#ifndef LIANG_H
#define LIANG_H

#include <vector>
#include <list>
#include <map>
#include <fstream>
#include "opencv2/core/core.hpp"

#include "grapheme.h"

using namespace std;
using namespace cv;

class Liang
{
public:
    Liang();
    
    
private:
    map<char, vector<float> > graphemeSpectrums;
    map<char, float> charWidthAvgs;
    map<char, float> charWidthStdDevs;
    
    bool trained;
};

#endif // LIANG_H
