#ifndef WORDSEPARATOR_H
#define WORDSEPARATOR_H
#include <QImage>
#include "wordprofile.h"
#include "Constants.h"
#include <QPoint>
#include "BPixelCollection.h"
#include "bimage.h"
#include "bpartition.h"

#include "dimension.h"
#include "graphcut.h"
#include "imageaverager.h"
#include "boxcleaner.h"
#include <fstream>
#include "distancetransform.h"



class WordSeparator
{
    
    
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
    
    static QVector<BPartition*> recut3D(const BPixelCollection &img, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds, const QPoint &crossOverPoint);
    static QVector<BPartition*> recut2D(const BPixelCollection &img, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds, const QPoint &crossOverPoint);
    static QVector<BPartition*> cutGivenSeeds(const BPixelCollection &img, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds);
    
private:
    
    
    
//    static QPoint findClosestPointOn(const BPixelCollection &img, QPoint &start);
};

#endif // WORDSEPERATOR_H
