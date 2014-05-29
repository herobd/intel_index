#ifndef BPARTITION_H
#define BPARTITION_H

#include "bimage.h"

class BImage;

class BPartition
{
public:
    BPartition(BImage* ofImage);
    
    BImage* getSrc();
    
    int width() const;
    int height() const;
    int getXOffset();
    int getYOffset();
    bool pixel(const QPoint &p);
    bool pixel(int x, int y);
    bool pixelIsMine(const QPoint &p);
    bool pixelIsMine(int x, int y);
//    void setPixelOwner(const QPoint &p, BPartition* owner, float portion);
//    void setPixelOwner(int x, int y, BPartition* owner, float portion);
    void changedPortion(const QPoint &, float newPortion, float oldPortion);
    void changedPortion(int x, int y, float newPortion, float oldPortion);
    BImage makeImage();
    
    void joinInto(BPartition* other);
    void remove();
   
private:
    int leftX;
    int rightX;
    int upperY;
    int lowerY;
    BImage* src;
    
};

#endif // BPARTITION_H
