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
#include <stdbool.h>
//#include <math.h>

#define LOWER_MEAN_SLOPE -0.695677
#define UPPER_MEAN_SLOPE -0.667002
#define LOWER_MEAN_CURVE -0.022714
#define UPPER_MEAN_CURVE 0.054275
#define CLOCKWISE_THRESH -5
#define COUNTER_CLOCKWISE_THRESH -5

typedef Graph<int,int,int> GraphType;



class GraphCut
{
public:
    static int pixelsOfSeparation(int* invDistMap, int width, int height, BPixelCollection &img, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight=INT_POS_INFINITY, int split_method=SPLIT_HORZ, int vert_divide=-1);
    
    static int pixelsOfSeparation(int* invDistMap, int width, int height, const BPixelCollection &img, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight=INT_POS_INFINITY, int split_method=SPLIT_HORZ, int vert_divide=-1);
    
    static int pixelsOfSeparation(int* invDistMap, int width, int height, const BPixelCollection &img, const AngleImage &angleImage, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight=INT_POS_INFINITY, int split_method=SPLIT_HORZ, int vert_divide=-1);
    
    static int pixelsOfSeparation(const long* invDistMap3D, int width, int height, int depth, const BPixelCollection &img, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds, QVector<int> &outSource, QVector<int> &outSink, const QPoint &crossOverPoint, int anchor_weight=INT_POS_INFINITY, int split_method=SPLIT_HORZ);
    
    static int pixelsOfSeparationWithSlope(int* invDistMap, int width, int height, BPixelCollection &img, const QVector<QVector<double> > &slopes, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight=INT_POS_INFINITY, int split_method=SPLIT_HORZ, int vert_divide=-1);
    static int pixelsOfSeparationNDimensions(int* invDistMap, int width, int height, const BPixelCollection &img, const NDimensions &dimensions, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight=INT_POS_INFINITY, int split_method=SPLIT_HORZ, int vert_divide=-1);
    
     static int pixelsOfSeparationNoDistMap(int* invDistMap, int width, int height, BPixelCollection &img, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight=INT_POS_INFINITY, int split_method=SPLIT_HORZ, int vert_divide=-1);
     
private:
     static void strengthenDescenderComponent(const BPixelCollection &img, const QPoint &crossOverPoint, GraphType *g, const Indexer3D &indexer, int numAngleValues);
     
     static void lowerDescenderTraverser(const BlobSkeleton &skeleton, QVector<unsigned int>* bestLowerPath, double* bestLowerScore, QVector<unsigned int>* bestUpperPath, double* bestUpperScore, const QVector<unsigned int>* currentPath, double clockwiseScore, PathStackMap* upperPaths);
     static void upperDescenderTraverser(const BlobSkeleton &skeleton, const QVector<unsigned int>* currentPath, double counterClockwiseScore, PathStackMap* upperPaths);
     static double polynomialfit(int obs, int degree, double *dx, double *dy, double *store);
     static int extractSampleFromPath(const BlobSkeleton &skeleton, const QVector<unsigned int>* currentPath, QVector<double>* xOut, QVector<double>* yOut);
     
     static double computeScore(int sampleSize, double* x, double* y, double meanSlope, double meanCurve, bool print=false);
     
};

class SkeletonTraversePriorityQueue
{
public:
    SkeletonTraversePriorityQueue(const BlobSkeleton* skeleton);
    
private:
    
};

#endif // GRAPHCUT_H
