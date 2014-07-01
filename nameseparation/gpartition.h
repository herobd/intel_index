#ifndef GPARTITION_H
#define GPARTITION_H

#include <QImage>
#include "gimage.h"
#include "gpixelcollection.h"
//class GPartition;

class GPartition : public GPixelCollection
{
public:
    GPartition(const GPixelCollection* ofImage);
    GPartition(const GImage* ofImage);
    GPartition(const GPartition* ofImage);
    ~GPartition();
    GPartition& operator=( const GPartition& other );
    const GPixelCollection* getSrc() const;
    
    int width() const;
    int height() const;
    int getXOffset();
    int getYOffset();
//    bool pixel(const QPoint &p) const;
    int pixel(int x, int y) const;
//    bool pixelIsMine(const QPoint &p) const;
    bool pixelIsMineSrc(const QPoint &src_p) const;
    bool pixelIsMine(int x, int y) const;
//    void setPixelOwner(const QPoint &p, BPartition* owner, float portion);
//    void setPixelOwner(int x, int y, BPartition* owner, float portion);
//    void changedPortion(const QPoint &, float newPortion, float oldPortion);
//    void changedPortion(int x, int y, float newPortion, float oldPortion);
    QImage makeImage() const;
    
    void addPixelFromSrc(const QPoint &src_p);
    void addPixelFromSrc(int src_x, int src_y);
    void removePixel(const QPoint &src_p);
    void removePixel(int src_x, int src_y);
//    void joinInto(GPartition* other);
//    void remove();
    void clear(GPartition* const pixelsToRemove);
    void shiftDownLayer();
   
    void trim(int offLeftSide, int offRightSide, int offTopSide, int offBottomSide);
    
//    bool rebase();
    void changeSrc(const GPixelCollection* newSrc, int srcXOffset, int srcYOffset);
//    void makeFull();
    
private:
    int leftX;
    int rightX;
    int upperY;
    int lowerY;
    const GPixelCollection* src;
    const GImage* rootSrc;
    //unsigned int* myPixels; //flatten 2D array of bits for whole src img
    bool** myPixels;
    
    void initMyPixels();
    inline void setMyPixelTrue(int src_x, int src_y);
    inline void setMyPixelFalse(int src_x, int src_y);
    
};


#endif // GPARTITION_H
