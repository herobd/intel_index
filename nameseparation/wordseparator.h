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
    
    static QVector<BPartition*> testSlopeCut(BPixelCollection &img, const NDimensions &dimensions /*const QVector<QVector<double> > &slopes*/);
    
    static QVector<BPartition*> segmentLinesOfWords(const BPixelCollection &column, int spacingEstimate);
    static QVector<BPartition*> recursiveHorizontalCutWords(const BPixelCollection &img);
    
private:
    static void computeInverseDistanceMap(BPixelCollection &img, int* out);
    
    static int f(int x, int i, int y, int m, int* g);
    
    static int SepPlusOne(int i, int u, int y, int m, int* g);
    
};

#endif // WORDSEPERATOR_H
