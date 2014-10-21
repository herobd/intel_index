/**
Algorithm originally created by Scott Swindle, modified by Brian Davis  

A brief explination:
Upon initailization, the object finds all of the regions using the blob spreading method.
The center points are stored as skeletonVertices with information about their neighboring regions.
Each region has a unique int ID. This is how they are accessed and identified in all things (pretty much anywhere you see "unsigned int").

You'll notice I set the eccentricity and min region size just as constants.

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


class skeletonVertex
{
public:
    int x;
    int y;
    skeletonVertex(int xx, int yy) {x=xx; y=yy;}
    skeletonVertex() {x=-1; y=-1;}
    QList<unsigned int> connectedPoints() const {return anglesBetween.keys();}
    double angleBetween(unsigned int index) const {return anglesBetween[index];}
    double distanceBetween(unsigned int index) const {return distancesBetween[index];}
    void addNeighbor(unsigned int index, double angle, double distance)
    {
        anglesBetween[index]=angle;
        distancesBetween[index]=distance;
    }

private:
    
//    QVector<unsigned int> connectedPoints;
//    QVector<double> angleBetween;
//    QVector<double> distanceBetween;
    QMap<unsigned int, double> anglesBetween;
    QMap<unsigned int, double> distancesBetween;
};

class BlobSkeleton : BPixelCollection
{
public:
    BlobSkeleton();
    BlobSkeleton(const BPixelCollection* src);
    ~BlobSkeleton();
    void init(const BPixelCollection* src);
    
    void initHand(const BPixelCollection* src, const QImage &handMarkedRegions);
    
    //This returns the number of regions identified
    unsigned int numberOfVertices()const {return centersOfMass.size();}
    
    //This is for convience of accessing the internal structs.
    const skeletonVertex& operator[] (unsigned int index) const {return centersOfMass[index];}
    
    //This saves an image file
    void draw(QString name) const;
    
    //Returns the id of the region this point falls into. Negative if it belongs to no region.
    int regionIdForPoint(const QPoint &p) const;
    int regionIdForPoint(int x, int y) const;
    
    //Same as above, but finds the closest if doesn't fall on region.
    int closestRegionIdForPoint(const QPoint &point) const;
    
    //Returns a vector of the points which comprimise the region.
    const QVector<QPoint>& getRegion(unsigned int index) const {return regions[index];}
    
    //superclass methods
    BImage makeImage() const;
    bool pixelIsMine(int x, int y) const;
    int width() const;
    int height() const;
    
    //use
    bool pixel(int x, int y) const;
    
private:
    QVector<QVector<QPoint> > regions;
    QVector<skeletonVertex> centersOfMass;
    const BPixelCollection* src;
    int** assignments;
    
    QPoint findStartPoint();
    void blobFill(const QPoint &begin);//the actual blob algorithm
    void evalHand(const QImage &markedRegions);
};

#endif // BLOBSKELETON_H
