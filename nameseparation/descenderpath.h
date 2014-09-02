#ifndef DESCENDERPATHPRIORITYQUEUE_H
#define DESCENDERPATHPRIORITYQUEUE_H

#include "blobskeleton.h"

#include <gsl/gsl_multifit.h>
#include "gsl/gsl_statistics.h"
#include <stdbool.h>


#define NEW_LOWER_MEAN_SLOPE -0.695677
#define NEW_UPPER_MEAN_SLOPE -0.667002
#define NEW_LOWER_STD_DEV_SLOPE 0.406538    
#define NEW_UPPER_STD_DEV_SLOPE 0.414845
#define NEW_LOWER_MEAN_CURVE -0.022714
#define NEW_UPPER_MEAN_CURVE 0.054275
#define NEW_LOWER_STD_DEV_CURVE 0.023222
#define NEW_UPPER_STD_DEV_CURVE 0.046421


class DescenderPath
{
public:
    DescenderPath(const BlobSkeleton* skeleton);
    DescenderPath(const DescenderPath &other);
    
    void append(unsigned int next);
    unsigned int at(unsigned int index) const;
    QPoint pointAt(unsigned int index) const;
    unsigned int size() const;
    unsigned int last() const;
    bool contains(unsigned int i) const;
    
    double score() const;
    bool hasTop() const;
    
    void printScore() const;
    
    int count(unsigned int i) const {return path.count(i);}
    
private:
    const BlobSkeleton* skeleton;
    double holdScore;
//    QVector<unsigned int> upperPath;
//    QVector<unsigned int> lowerPath;
    QVector<unsigned int> path;
    int divideIndex;
    
    double getRelAngle(int indexA, int indexB, int indexC) const;
    double polynomialfit(int obs, int degree, double *dx, double *dy, double *store, double *covarience) const;
    double computeHalfScore(bool upper, bool print=false) const;
};




#endif // DESCENDERPATHPRIORITYQUEUE_H
