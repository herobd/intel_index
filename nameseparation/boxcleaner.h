#ifndef BOXCLEANER_H
#define BOXCLEANER_H
#include "bimage.h"
#include "Constants.h"

class BoxCleaner
{
public:
    BoxCleaner();
    static BImage trimBoundaries(BImage &img);
    static BImage trimVerticleBoundaries(BImage &img);
    static BImage trimHorizontalBoundaries(BImage &img);
    static BImage trimHorizontalLines(BImage &img);
    static BImage removePixelNoise(BImage &img);
    static BImage removeVerticlePixelNoise(BImage &img);
    static BImage clearLineAndCloseLetters(BPixelCollection &src, int est_y, int* vert_divide=NULL, QVector<QPoint>* crossPoints=NULL);
    
private:
    static void lineFilterAtJ(int j, BImage &ret);
    static void cond_clear_line(int runLength, int i, int j, BImage &ret);
    static void blobFilter(BImage &on, int fromY, int toY, int blobThresh, int horzMarkThresh, double horzLongRatio, double stdDevLimit);
};

#endif // BOXCLEANER_H
