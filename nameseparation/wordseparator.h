#ifndef WORDSEPARATOR_H
#define WORDSEPARATOR_H
#include <QImage>
#include "wordprofile.h"
#include "maxflow/graph.h"
#include "Constants.h"

#define SPLIT_HORZ 1
#define SPLIT_VERT 2

class WordSeparator
{
public:
    WordSeparator();
    //WordSeperator(const QImage &img);
    static QImage removeFirstWord(QImage &from);
    static QImage removeFirstCapitalLetter(QImage &from);
    static int windowScanWidestMin(WordProfile &profile, int size);
    static QVector<QImage> minCut(QImage &img);
    
    static QVector<QImage> cutNames(QImage &img);
    static QVector<QImage> recursiveCutWordToFirstLetter(QImage &img);
    static QVector<QImage> horzCutEntries(QImage &img, int vert_divide, QVector<QPoints> aboveBoundaryPoints, QVector<QPoints> belowBoundaryPoints);
    
private:
    //static void computeInverseDistanceMap(double* in, double* out, int size);
    static void computeInverseDistanceMap(QImage &img, int* out);
    static int pixelsOfSeparation(int* invDistMap, int width, int height, QImage &img, QVector<int> &out, int anchor_weight=INT_POS_INFINITY, int split_method=SPLIT_HORZ, int vert_divide=-1);
    static int f(int x, int i, int y, int m, int* g);
    static int SepPlusOne(int i, int u, int y, int m, int* g);
    
};

#endif // WORDSEPERATOR_H
