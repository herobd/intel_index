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
    bool pixel(const QPoint &p) const;
    bool pixel(int x, int y) const;
    void changePortion(const QPoint &, float newPortion, float oldPortion);
    void changePortion(int x, int y, float newPortion, float oldPortion);
    BImage makeImage();
   
private:
    int leftX;
    int rightX;
    int upperY;
    int lowerY;
    BImage* src;
    
};

#endif // BPARTITION_H
