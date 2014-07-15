#ifndef GRAPHCUT_H
#define GRAPHCUT_H

#include "Constants.h"
#include "maxflow/graph.h"
#include <QPoint>
#include <QVector>
//#include "BPixelCollection.h"
#include "bimage.h"
#include "dimension.h"
#include "math.h"
#include <stdio.h>

class GraphCut
{
public:
    static int pixelsOfSeparation(int* invDistMap, int width, int height, BPixelCollection &img, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight=INT_POS_INFINITY, int split_method=SPLIT_HORZ, int vert_divide=-1);
    
    static int pixelsOfSeparation(int* invDistMap, int width, int height, const BPixelCollection &img, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight=INT_POS_INFINITY, int split_method=SPLIT_HORZ, int vert_divide=-1);
    
    static int pixelsOfSeparationWithSlope(int* invDistMap, int width, int height, BPixelCollection &img, const QVector<QVector<double> > &slopes, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight=INT_POS_INFINITY, int split_method=SPLIT_HORZ, int vert_divide=-1);
    static int pixelsOfSeparationNDimensions(int* invDistMap, int width, int height, const BPixelCollection &img, const NDimensions &dimensions, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight=INT_POS_INFINITY, int split_method=SPLIT_HORZ, int vert_divide=-1);
    
     static int pixelsOfSeparationNoDistMap(int* invDistMap, int width, int height, BPixelCollection &img, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight=INT_POS_INFINITY, int split_method=SPLIT_HORZ, int vert_divide=-1);
};

#endif // GRAPHCUT_H
