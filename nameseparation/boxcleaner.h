#ifndef BOXCLEANER_H
#define BOXCLEANER_H
#include <QImage>
#include "Constants.h"

class BoxCleaner
{
public:
    BoxCleaner();
    static QImage trimBoundaries(QImage &img);
    static QImage removePixelNoise(QImage &img);
    static QImage clearLineAndCloseLetters(QImage &src, int est_y);
    
private:
    static void lineFilterAtJ(int j, QImage &ret);
    static void cond_clear_line(int runLength, int i, int j, QImage &ret);
};

#endif // BOXCLEANER_H
