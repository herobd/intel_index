#ifndef GRAPHEME_H
#define GRAPHEME_H

#include "opencv2/core/core.hpp"

using namespace cv;

class Grapheme
{
public:
    const Mat* img(){return &graphemes;}
    int imgId(){return id;}
    
private:
    int id;
    const Mat graphemes;//same referece as all graphemes
    int MOG_class;
};

#endif // GRAPHEME_H
