#ifndef GRAPHEME_H
#define GRAPHEME_H

#include "opencv2/core/core.hpp"

using namespace cv;

class Grapheme
{
public:
    Grapheme(Mat graphemes, int id);
    const Mat* img()const {return &graphemes;}
    int imgId()const {return id;}
    int maxX()const {return rightX;}
    int minX()const {return leftX;}
    int maxY()const {return bottomY;}
    int minY()const {return topY;}
    int maxXBetween(int minY, int maxY)const;
    int minXBetween(int minY, int maxY)const;
    Point2f centriod();
    
private:
    int id;
    Mat graphemes;//same referece as all graphemes
    int MOG_class;
    int rightX, leftX, topY, bottomY;
};

#endif // GRAPHEME_H
