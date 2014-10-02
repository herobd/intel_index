#ifndef GRAPHCUT_H
#define GRAPHCUT_H

#include "Constants.h"
#include "maxflow/graph.h"
#include <QPoint>
#include <QVector>
#include "bimage.h"
#include "dimension.h"
#include "math.h"
#include <stdio.h>
#include "angleimage.h"
#include "blobskeleton.h"
#include "pathstackmap.h"
#include <QVector3D>
#include "descenderpath.h"
#include <queue>

#include <gsl/gsl_multifit.h>
#include "gsl/gsl_statistics.h"
#include <stdbool.h>
//#include <math.h>

#define LOWER_MEAN_SLOPE -0.695677
#define UPPER_MEAN_SLOPE -0.667002
#define LOWER_STD_DEV_SLOPE 0.406538    
#define UPPER_STD_DEV_SLOPE 0.414845
#define LOWER_MEAN_CURVE -0.022714
#define UPPER_MEAN_CURVE 0.054275
#define LOWER_STD_DEV_CURVE 0.023222
#define UPPER_STD_DEV_CURVE 0.046421
#define CLOCKWISE_THRESH -10
#define COUNTER_CLOCKWISE_THRESH -5
#define SCORE_THRESH 17//15
#define COMBINE_SCORE_THRESH 30//27

#define CENTER_BIAS_ANCHOR_WIEGHT 10.0

#define DESC_BIAS_LEN_2D 16000
#define DESC_BIAS_T_2D 600
#define DESC_BIAS_LEN_3D 16000
#define DESC_BIAS_T_3D 600
#define DESC_BIAS_Z 14000

#define NEW_SCORE_THRESH 60//25//20
#define DESCENDER_LIKELIHOOD_THRESH 11

typedef Graph<int,int,int> GraphType;



class GraphCut
{
public:
    static int pixelsOfSeparation(int* invDistMap, int width, int height, BPixelCollection &img, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight=INT_POS_INFINITY/2, int split_method=SPLIT_HORZ, int vert_divide=-1);
    
    static int pixelsOfSeparationMicro(int* invDistMap, int width, int height, BPixelCollection &img, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight=INT_POS_INFINITY/2, int split_method=SPLIT_HORZ, int vert_divide=-1);
    
    static int pixelsOfSeparation(int* invDistMap, int width, int height, const BPixelCollection &img, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight=INT_POS_INFINITY/2, int split_method=SPLIT_HORZ, int vert_divide=-1);
    
    static int pixelsOfSeparation(int* invDistMap, int width, int height, const BPixelCollection &img, const AngleImage &angleImage, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight=INT_POS_INFINITY/2, int split_method=SPLIT_HORZ, int vert_divide=-1);
    
    static int pixelsOfSeparationRecut3D(const long* invDistMap3D, int width, int height, int depth, const AngleImage &img, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds, QVector<int> &outSource, QVector<int> &outSink, const QPoint &crossOverPoint, int anchor_weight=INT_POS_INFINITY/2, int split_method=SPLIT_HORZ);
    
    static int pixelsOfSeparationWithSlope(int* invDistMap, int width, int height, BPixelCollection &img, const QVector<QVector<double> > &slopes, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight=INT_POS_INFINITY/2, int split_method=SPLIT_HORZ, int vert_divide=-1);
    static int pixelsOfSeparationNDimensions(int* invDistMap, int width, int height, const BPixelCollection &img, const NDimensions &dimensions, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight=INT_POS_INFINITY/2, int split_method=SPLIT_HORZ, int vert_divide=-1);
    
     static int pixelsOfSeparationNoDistMap(int* invDistMap, int width, int height, BPixelCollection &img, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight=INT_POS_INFINITY/2, int split_method=SPLIT_HORZ, int vert_divide=-1);
     
     static int pixelsOfSeparationRecut2D(const BPixelCollection &img, const int* invDistMap, int width, int height, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds, QVector<int> &outSource, QVector<int> &outSink, const QPoint &crossOverPoint, int anchor_weight=270, int split_method=SPLIT_VERT);
     
     static double polynomialfit(int obs, int degree, double *dx, double *dy, double *store, double *covarience);
     
     static void strengthenDescenderComponentAccum(const AngleImage &img, const QPoint &crossOverPoint, GraphType *g, const Indexer3D &indexer);
     static void strengthenDescenderComponent(const AngleImage &img, const QPoint &crossOverPoint, GraphType *g, const Indexer3D &indexer);
     static void strengthenDescenderComponent2D(const BPixelCollection &img, const QPoint &crossOverPoint, GraphType *g);
     
     static QVector<double> correctScores;
     static QVector<double> incorrectScores;
     
private:
     
     static DescenderPath* findDescenderAccumulatively(const BlobSkeleton &skeleton, const QPoint &startPoint);
     
     static void strengthenConnection3D(int curX, int curY, int curZ, int nextX, int nextY, int nextZ, GraphType *g, const BPixelCollection &img, unsigned int depth, QImage *test);
     static void strengthenConnection2D(int curX, int curY, int nextX, int nextY, GraphType *g, const BPixelCollection &img, QImage *test);
     
     static void lowerDescenderTraverser(const BlobSkeleton &skeleton, QVector<QVector<unsigned int> >* bestLowerPaths, QVector<double>* bestLowerScores, QVector<QVector<unsigned int> >* bestUpperPaths, QVector<double>* bestUpperScores, const QVector<unsigned int>* currentPath, double clockwiseScore, PathStackMap* upperPaths);
     static void upperDescenderTraverser(const BlobSkeleton &skeleton, const QVector<unsigned int>* currentPath, double counterClockwiseScore, PathStackMap* upperPaths);
     
     static int extractSampleFromPath(const BlobSkeleton &skeleton, const QVector<unsigned int>* currentPath, QVector<double>* xOut, QVector<double>* yOut);
     
     static double computeScore(int sampleSize, double* x, double* y, double meanSlope, double stdDevSlope, double meanCurve, double stdDevCurve, bool print=false);
     
};

class SkeletonTraversePriorityQueue
{
public:
    SkeletonTraversePriorityQueue(const BlobSkeleton* skeleton);
    
private:
    
};

#endif // GRAPHCUT_H
