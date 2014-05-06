#ifndef WORDSEPARATOR_H
#define WORDSEPARATOR_H
#include <QImage>
#include "wordprofile.h"
#include "maxflow/graph.h"

class WordSeparator
{
public:
    WordSeparator();
    //WordSeperator(const QImage &img);
    static QImage removeFirstWord(QImage &from);
    static QImage removeFirstCapitalLetter(QImage &from);
    static int windowScanWidestMin(WordProfile &profile, int size);
    static QVector<QImage> minCut(QImage &img);
    static QImage trimBoundaries(QImage &img);
    static QImage removePixelNoise(QImage &img);
    
private:
    //static void computeInverseDistanceMap(double* in, double* out, int size);
    static void computeInverseDistanceMap(QImage &img, int* out);
    static int pixelsOfSeparation(int* invDistMap, int width, int height, QImage &img, QVector<int> &out);
    static int f(int x, int i, int y, int m, int* g);
    static int SepPlusOne(int i, int u, int y, int m, int* g);
};

#endif // WORDSEPERATOR_H
