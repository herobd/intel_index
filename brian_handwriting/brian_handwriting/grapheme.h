#ifndef GRAPHEME_H
#define GRAPHEME_H

#include "opencv2/core/core.hpp"

using namespace cv;

class Grapheme
{
public:
    Grapheme(Mat graphemes, int id);
    const Mat* img(){return &graphemes;}
    int imgId(){return id;}
    int maxX(){return rightX;}
    int minX(){return leftX;}
    int maxY(){return bottomY;}
    int minY(){return topY;}
    int maxXBetween(int minY, int maxY);
    int minXBetween(int minY, int maxY);
    
private:
    int id;
    const Mat graphemes;//same referece as all graphemes
    int MOG_class;
    int rightX, leftX, topY, bottomY;
    int upperBaseline, lowerBaseline;
    int minXBetweenBaselines, maxXBetweenBaselines;
    
    void findMinMaxBetweenBaselines(int minY, int maxY);
};

#endif // GRAPHEME_H
