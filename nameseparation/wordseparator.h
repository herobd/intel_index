#ifndef WORDSEPARATOR_H
#define WORDSEPARATOR_H
#include <QImage>
#include "wordprofile.h"
#include "Constants.h"
#include <QPoint>
#include "bimage.h"
#include "bpartition.h"
#include "BPixelCollection.h"
#include "dimension.h"
#include "graphcut.h"
#include "imageaverager.h"
#include "boxcleaner.h"
#include <fstream>
#include <QRegExp>

class WordSeparator
{
    struct tracePoint
    {
        int x;
        int y;
        QVector<int> connectedPoints;
        QVector<double> angleBetween;
    };
    
public:
    WordSeparator();
    static int minCut(BPixelCollection &toCut, QVector<BPartition*> &ret);
    
//    static QVector<QImage> cutNames(QImage &img);
//    static QVector<QImage> recursiveCutWordToFirstLetter(QImage &img);
    static QVector<BPartition*> horzCutEntries(BPixelCollection &img, int vert_divide);
    static void adjustHorzCutCrossOverAreas(BPartition* top, BPartition* bottom, QVector<QPoint> crossPoints, QVector<QVector<double> > descenderProbMap);
    static BPartition* chopOutTop(BPixelCollection &src);
    
    static QVector<BPartition*> testSlopeCut(BPixelCollection &img, const NDimensions &dimensions, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds /*const QVector<QVector<double> > &slopes*/);
    
    static QVector<BPartition*> segmentLinesOfWords(const BPixelCollection &column, int spacingEstimate);
    static QVector<BPartition*> recursiveHorizontalCutTwoWords(const BPixelCollection &img);
    static QVector<BPartition*> recursiveHorizontalCutTwoWordsTraining(const BPixelCollection &img);
    static QVector<BPartition*> recursiveHorizontalCutFirstLetter(const BPixelCollection &img);
    static QVector<BPartition*> recursiveHorizontalCutFirstLetterTraining(const BPixelCollection &img);
    static QVector<BPartition*> recursiveHorizontalCutFullTraining(const BPixelCollection &img);
    static QVector<BPartition*> recursiveHorizontalCutFull(const BPixelCollection &img);
    
    static QVector<BPartition*> cut3D(BPixelCollection &img, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds);
    static QVector<BPartition*> cutGivenSeeds(BPixelCollection &img, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds);
    
private:
    static void computeInverseDistanceMap(BPixelCollection &img, int* out);
    
    static int f(int x, int i, int y, int m, int* g);
    
    static int SepPlusOne(int i, int u, int y, int m, int* g);
    
    static QPoint findClosestPointOn(BPixelCollection &img, QPoint &start);
};

#endif // WORDSEPERATOR_H
