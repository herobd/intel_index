#ifndef BPIXELCOLLECTION_H
#define BPIXELCOLLECTION_H

#include <QPoint>

class BImage;

class BPixelCollection
{
public:
    virtual bool pixel(const QPoint &p) const {return pixel(p.x(),p.y());}
    virtual bool pixel(int x, int y) const =0;
    virtual bool pixelIsMine(const QPoint &p) const {return pixelIsMine(p.x(),p.y());}
    virtual bool pixelIsMine(int x, int y) const =0;
    virtual int width() const =0;
    virtual int height() const =0;
    virtual BImage makeImage() const =0;
};

#include "bimage.h"

#endif // BPIXELCOLLECTION_H
