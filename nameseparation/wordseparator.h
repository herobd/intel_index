#ifndef WORDSEPARATOR_H
#define WORDSEPARATOR_H
#include <QImage>
#include "wordprofile.h"
#include "maxflow/graph.h"
#include "Constants.h"
#include <QPoint>
#include "bimage.h"
#include "bpartition.h"
#include "BPixelCollection.h"

#define SPLIT_HORZ 1
#define SPLIT_VERT 2

class WordSeparator
{
public:
    WordSeparator();
    static QVector<BPartition*> minCut(BPixelCollection &toCut, float claimPortion);
    
//    static QVector<QImage> cutNames(QImage &img);
//    static QVector<QImage> recursiveCutWordToFirstLetter(QImage &img);
    static QVector<BPartition*> horzCutEntries(BPixelCollection &img, int vert_divide);
    static void adjustHorzCutCrossOverAreas(BPartition* top, BPartition* bottom, QVector<QPoint> crossPoints);
    
private:
    static void computeInverseDistanceMap(BPixelCollection &img, int* out);
    static int pixelsOfSeparation(int* invDistMap, int width, int height, BPixelCollection &img, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight=INT_POS_INFINITY, int split_method=SPLIT_HORZ, int vert_divide=-1);
    static int f(int x, int i, int y, int m, int* g);
    static int SepPlusOne(int i, int u, int y, int m, int* g);
    
};

#endif // WORDSEPERATOR_H
