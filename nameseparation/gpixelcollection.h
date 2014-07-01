#ifndef GPIXELCOLLECTION_H
#define GPIXELCOLLECTION_H

#include <QImage>
//class GImage;

class GPixelCollection
{
public:
    virtual int pixel(const QPoint &p) const {return pixel(p.x(),p.y());}
    virtual int pixel(int x, int y) const =0;
    virtual bool pixelIsMine(const QPoint &p) const {return pixelIsMine(p.x(),p.y());}
    virtual bool pixelIsMine(int x, int y) const =0;
    virtual int width() const =0;
    virtual int height() const =0;
    virtual QImage makeImage() const =0;
};

#endif // GPIXELCOLLECTION_H
