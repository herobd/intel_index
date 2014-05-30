#ifndef BPARTITION_H
#define BPARTITION_H

#include "bimage.h"
#include "BPixelCollection.h"

class BImage;

class BPartition : public BPixelCollection
{
public:
    BPartition(const BPixelCollection* ofImage);
    ~BPartition();
    
    const BPixelCollection* getSrc() const;
    
    int width() const;
    int height() const;
    int getXOffset();
    int getYOffset();
//    bool pixel(const QPoint &p) const;
    bool pixel(int x, int y) const;
//    bool pixelIsMine(const QPoint &p) const;
    bool pixelIsMine(int x, int y) const;
//    void setPixelOwner(const QPoint &p, BPartition* owner, float portion);
//    void setPixelOwner(int x, int y, BPartition* owner, float portion);
//    void changedPortion(const QPoint &, float newPortion, float oldPortion);
//    void changedPortion(int x, int y, float newPortion, float oldPortion);
    BImage makeImage() const;
    
    void addPixelFromSrc(const QPoint &p);
    void addPixelFromSrc(int x, int y);
    void removePixel(int x, int y);
    void joinInto(BPartition* other);
//    void remove();
    void clear(BPartition* const pixelsToRemove);
    void shiftDownLayer();
   
private:
    int leftX;
    int rightX;
    int upperY;
    int lowerY;
    const BPixelCollection* src;
    unsigned int* myPixels; //flatten 2D array of bits for whole src img
    
};

#endif // BPARTITION_H
