#include "gimage.h"

GImage::GImage(QImage &other)
{
    image=other;
}

int GImage::pixel(int x, int y) const
{
    return qGray(image.pixel(x,y));
}
bool GImage::pixelIsMine(int x, int y) const {return true;}
int GImage::width() const {return image.width();}
int GImage::height() const {return image.height();}
QImage GImage::makeImage() const {return image;}
