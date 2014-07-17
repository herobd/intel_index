#ifndef SLOPEIMAGE_H
#define SLOPEIMAGE_H

#include <QMap>
#include "Constants.h"
#include "bpartition.h"
#include <assert.h>
#include "bpartition.h"
#include "BPixelCollection.h"
#include "bimage.h"



class AngleImage : public BPixelCollection
{
public:
//    SlopeImage();
    AngleImage(const BPixelCollection &other, int numOfBins, double minVal, double maxVal);
//    BImage(const BPartition* src1, const BPartition* src2);
    ~AngleImage();
    AngleImage& operator=( const AngleImage& other );
    
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
    int getNumOfBins();
    QMap<int,double> getBinsAndStrForPixel(int x, int y);
    bool noAngleForPixel(int x, int y, double angle);
    
private:
    QMap<double, double>** angles;
    int myWidth;
    int myHeight;
    BPixelCollection* src;
    int numOfBins;
    double minValue;
    double maxValue;
};

class AngleIndexer
{
public:
    AngleIndexer(int width, int height);
//    int getIndex(int x, int y) const;
    int getIndex(int x, int y, int slopeBin) const;
    
private:
//    const AngleImage* angleImage;
    int width;
    int height;
};

#endif // SLOPEIMAGE_H
