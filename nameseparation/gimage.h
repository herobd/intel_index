#ifndef GIMAGE_H
#define GIMAGE_H

#include <QImage>
#include "gpixelcollection.h"

class GImage : public GPixelCollection
{
public:
    GImage(QImage &other);
    int pixel(int x, int y) const;
    bool pixelIsMine(int x, int y) const;
    int width() const;
    int height() const;
    QImage makeImage() const;
    
private:
    QImage image;
};

#endif // GIMAGE_H
