/**
Algorithm originally created by Scott Swindle, modified by Brian Davis  
**/

#ifndef BLOBSKELETON_H
#define BLOBSKELETON_H

#include <QVector>
#include <QPoint>
#include <BPixelCollection.h>
#include <QImage>
#include <math.h>
#include <QSet>
#include <QColor>

#define ECCENTRICITY_LIMIT 1.5
#define MIN_REGION_SIZE 6

#define NO_ASSIGMENT -2

struct tracePoint
{
    int x;
    int y;
    QVector<unsigned int> connectedPoints;
    QVector<double> angleBetween;
    QVector<double> distanceBetween;
    tracePoint(int xx, int yy) {x=xx; y=yy;}
    tracePoint() {x=-1; y=-1;}
};

class BlobSkeleton
{
public:
    BlobSkeleton(const BPixelCollection* src);
    ~BlobSkeleton();
    unsigned int numberOfVertices()const {return centersOfMass.size();}
    const tracePoint& operator[] (unsigned int index) const {return centersOfMass[index];}
    void draw(QString name);
    int regionIdForPoint(const QPoint &p);
    const QVector<QPoint>& getRegion(unsigned int index) const {return regions[index];}
    
private:
    QVector<QVector<QPoint> > regions;
    QVector<tracePoint> centersOfMass;
    const BPixelCollection* src;
    int** assignments;
    
    QPoint findStartPoint();
    void blobFill(const QPoint &begin);
};

#endif // BLOBSKELETON_H
