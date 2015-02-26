#include "gimage.h"

GImage::GImage(QImage &other)
{
    image=other;
}

GImage::GImage(int w, int h)
{
    QImage n(w,h,QImage::Format_RGB32);
    image=n;
    image.fill(qRgb(0,0,0));
}

int GImage::pixel(int x, int y) const
{
    return qGray(image.pixel(x,y));
}
bool GImage::pixelIsMine(int x, int y) const {return true;}
int GImage::width() const {return image.width();}
int GImage::height() const {return image.height();}
QImage GImage::makeImage() const {return image;}
void GImage::setPixel(int x, int y, int intensity)
{
    image.setPixel(x,y,qRgb(intensity,intensity,intensity));
}


//GImage GImage::convolveWith(const GImage &kernel)
//{
//    GImage ret(image.width(),image.height());
    
//    for (int x=0; x<ret.width(); x++)
//    {
//        for (int y=0; y<ret.height(); y++)
//        {
            
//        }
//    }
//}
