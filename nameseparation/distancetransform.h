#ifndef DISTANCETRANSFORM_H
#define DISTANCETRANSFORM_H

#include "indexer3d.h"
#include <QVector3D>
#include <QPoint>
#include "BPixelCollection.h"
#include "operators.h"
#include <math.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>


class DistanceTransform
{
public:
    static void computeInverseDistanceMap(const BPixelCollection &img, int* out);
    
    static void compute3DInverseDistanceMapNaive(const bool* src, int* out, int width, int height, int depth);
    static void compute3DInverseDistanceMap(const bool* src, int* out, int width, int height, int depth);
    static void compute3DInverseDistanceMapNew(const double* src, long* out, int width, int height, int depth);
    static void compute3DInverseDistanceMapTest(const bool* src, int* out, int width, int height, int depth);
    static void computeKDInverseDistanceMap(const bool* in, int* out, int k, const int* dim);
    
private:
    
    static int f(int x, int i, int y, int m, int* g);
    static int SepPlusOne(int i, int u, int y, int m, int* g);
    
    static int f2D(int x, int i, int y, int z, Indexer3D &ind, int* g);
    static int SepPlusOne2D(int i, int u, int y, int z, Indexer3D &ind, int* g);
    
    static int f3D(int x, int y, int z, int i, Indexer3D &ind, int* g);
    static int SepPlusOne3D(int x, int y, int i, int u, Indexer3D &ind, int* g);
    
    
    static int f3DTest(int x, int y, int z, int i, Indexer3D &ind, int* g);
    static int SepPlusOne3DTest(int x, int y, int i, int u, Indexer3D &ind, int* g);
    
    
    
    static int ComputeEDT(const bool* I, int* D, int k, const int* n, const IndexerKD &ind, int d, int* i);
    static int recursiveFor(const bool* I, int* D, int k, const int* n, const IndexerKD &ind, int d, int* i, int level);
    static int VoronoiEDT(const bool* I, int* D, int k, const int* n, const IndexerKD &ind, int d, int* j);
    static bool RemoveEDT(int dis_sqr_u_Rd, int dis_sqr_v_Rd, int dis_sqr_w_Rd, int u_d, int v_d, int w_d);
    static void copyArray(int* from, int* to, int c);
    
    static void ComputeEDT3D(const bool* I, int* D, const int* n, const IndexerKD &ind, int d, int* i);
    static void VoronoiEDT3D(const bool* I, int* D, const int* n, const IndexerKD &ind, int d, int* j);
};

#endif // DISTANCETRANSFORM_H
