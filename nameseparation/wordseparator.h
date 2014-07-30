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
#include "indexer3d.h"


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
    
    static QVector<BPartition*> cut3D(const BPixelCollection &img, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds);
    static QVector<BPartition*> cutGivenSeeds(const BPixelCollection &img, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds);
    
private:
    static void computeInverseDistanceMap(const BPixelCollection &img, int* out);
    static void compute3DInverseDistanceMap(const bool* src, int* out, int width, int height, int depth);
    
    static int f(int x, int i, int y, int m, int* g);
    static int SepPlusOne(int i, int u, int y, int m, int* g);
    
    static int f2D(int x, int i, int y, int z, Indexer3D &ind, int* g);
    static int SepPlusOne2D(int i, int u, int y, int z, Indexer3D &ind, int* g);
    
    static int f3D(int x, int y, int z, int i, Indexer3D &ind, int* g);
    static int SepPlusOne3D(int x, int y, int i, int u, Indexer3D &ind, int* g);
    
    static void compute3DInverseDistanceMapTest(const bool* src, int* out, int width, int height, int depth);
    static int f3DTest(int x, int y, int z, int i, Indexer3D &ind, int* g);
    static int SepPlusOne3DTest(int x, int y, int i, int u, Indexer3D &ind, int* g);
    
    static void computeKDInverseDistanceMap(const bool* in, int* out, int k, const int* dim);
    
    static int ComputeEDT(const bool* I, int* D, int k, const int* n, const IndexerKD &ind, int d, int* i);
    static int recursiveFor(const bool* I, int* D, int k, const int* n, const IndexerKD &ind, int d, int* i, int level);
    static int VoronoiEDT(const bool* I, int* D, int k, const int* n, const IndexerKD &ind, int d, int* j);
    static bool RemoveEDT(int dis_sqr_u_Rd, int dis_sqr_v_Rd, int dis_sqr_w_Rd, int u_d, int v_d, int w_d);
    static void copyArray(int* from, int* to, int c);
    
//    static QPoint findClosestPointOn(const BPixelCollection &img, QPoint &start);
};

#endif // WORDSEPERATOR_H
