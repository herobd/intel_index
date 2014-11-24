#ifndef SLOPEIMAGE_H
#define SLOPEIMAGE_H

#include <QMap>
#include "Constants.h"
#include "bpartition.h"
#include <assert.h>
#include "bpartition.h"
#include "BPixelCollection.h"
#include "bimage.h"
#include <math.h>
#include <fstream>
#include <QRegExp>
#include "indexer3d.h"
#include "blobskeleton.h"


class AngleImage : public BPixelCollection
{
    
    struct tracePoint
    {
        int x;
        int y;
        QVector<int> connectedPoints;
        QVector<double> angleBetween;
        QVector<double> distanceBetween;
    };
    
public:
//    SlopeImage();
    AngleImage(const BPixelCollection* ofImage, int numOfBins, double minVal, double maxVal);
//    BImage(const BPartition* src1, const BPartition* src2);
    ~AngleImage();
//    AngleImage& operator=( const AngleImage& other );
    
    int width() const;
    int height() const;
    
    //use
    bool pixel(const QPoint &p) const;
    bool pixel(int x, int y) const;
    
    //extras
//    slopePixel pixelFull(const QPoint &p) const;
//    slopePixel pixelFull(int x, int y) const;
//    double pixelSlopePortion(const QPoint &p, double slope) const;
//    double pixelSlopePortion(int x, int y, double slope) const;
//    double pixelMajorityOwner(const QPoint &p) const;
//    double pixelMajorityOwner(int x, int y) const;
    
    //use
//    void setPixel(const QPoint &p, bool val);
//    void setPixel(int x, int y, bool val);
    
    //extras
//    void setPixelFull(const QPoint &p, const bPixel &strct);
//    void setPixelFull(int x, int y, const bPixel &strct);
    void setPixelSlope(const QPoint &p, double angle, double strength);
    void setPixelSlope(int x, int y, double angle, double strength);
    
//    void setPixelCombineOwner(int x, int y, BPartition* to, BPartition* with);
    
    
    BImage makeImage() const;
    bool pixelIsMine(int x, int y) const;
    
    void setNumOfBinsMinValMaxVal(int numOfBins, double minVal, double maxVal);
    int getNumOfBins() const;
    //returns the relative strength to the highest
    QMap<int,double> getBinsAndStrForPixel(int x, int y) const;
    int getBinForAngle(double angle) const;
//    int sizeOfBin(){return 2*PI/numOfBins;}
    bool noStrongerAngleForPixel(int x, int y, double angle, double strength) const;
    bool noAnglesForPixel(int x, int y) const;
    
    const BlobSkeleton& getSkeleton() const {return skeleton;}
    
private:
    void init();
//    QPoint findClosestPoint(QPoint &start);
    
//    QMap<double, double>** angles;
    QMap<int, double>** binStr;
    int myWidth;
    int myHeight;
    const BPixelCollection* src;
    int numOfBins;
    double minValue;
    double maxValue;
    BlobSkeleton skeleton;
};



#endif // SLOPEIMAGE_H
