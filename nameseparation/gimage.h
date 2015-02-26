#ifndef GIMAGE_H
#define GIMAGE_H

#include <QImage>
#include "gpixelcollection.h"

class GImage : public GPixelCollection
{
public:
    GImage(QImage &other);
    GImage(int w, int h);
    int pixel(int x, int y) const;
    bool pixelIsMine(int x, int y) const;
    int width() const;
    int height() const;
    QImage makeImage() const;
    
    void setPixel(int x, int y, int intensity);
    
private:
    QImage image;
};

#endif // GIMAGE_H
