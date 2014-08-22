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

#define DESC_BIAS_LEN 16000
#define DESC_BIAS_Z 14000

typedef Graph<int,int,int> GraphType;



class GraphCut
{
public:
    static int pixelsOfSeparation(int* invDistMap, int width, int height, BPixelCollection &img, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight=INT_POS_INFINITY, int split_method=SPLIT_HORZ, int vert_divide=-1);
    
    static int pixelsOfSeparation(int* invDistMap, int width, int height, const BPixelCollection &img, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight=INT_POS_INFINITY, int split_method=SPLIT_HORZ, int vert_divide=-1);
    
    static int pixelsOfSeparation(int* invDistMap, int width, int height, const BPixelCollection &img, const AngleImage &angleImage, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight=INT_POS_INFINITY, int split_method=SPLIT_HORZ, int vert_divide=-1);
    
    static int pixelsOfSeparation(const long* invDistMap3D, int width, int height, int depth, const AngleImage &img, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds, QVector<int> &outSource, QVector<int> &outSink, const QPoint &crossOverPoint, int anchor_weight=INT_POS_INFINITY, int split_method=SPLIT_HORZ);
    
    static int pixelsOfSeparationWithSlope(int* invDistMap, int width, int height, BPixelCollection &img, const QVector<QVector<double> > &slopes, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight=INT_POS_INFINITY, int split_method=SPLIT_HORZ, int vert_divide=-1);
    static int pixelsOfSeparationNDimensions(int* invDistMap, int width, int height, const BPixelCollection &img, const NDimensions &dimensions, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight=INT_POS_INFINITY, int split_method=SPLIT_HORZ, int vert_divide=-1);
    
     static int pixelsOfSeparationNoDistMap(int* invDistMap, int width, int height, BPixelCollection &img, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight=INT_POS_INFINITY, int split_method=SPLIT_HORZ, int vert_divide=-1);
     
     static double polynomialfit(int obs, int degree, double *dx, double *dy, double *store, double *covarience);
     

     static void strengthenDescenderComponent(const AngleImage &img, const QPoint &crossOverPoint, GraphType *g, const Indexer3D &indexer);
     
private:
     
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
