#include "angleimage.h"

AngleImage::AngleImage(const BPixelCollection* ofImage, int numOfBins, double minVal, double maxVal)
{
    src=ofImage;
    this->numOfBins = numOfBins;
    minValue=minVal;
    maxValue=maxVal;
    
    angles = new QMap<double, double>*[src->width()];
    for (int x=0; x<src->width(); x++)
    {
        angles[x] = new QMap<double, double>[src->height()];
//        for (int y=0; y<src->height(); y++)
//        {
//            angles[x][y][-1]=1;
//        }
    }
}

AngleImage::~AngleImage()
{
    for (int i=0; i<src->width(); i++)
    {
        delete[] angles[i];
    }
    delete[] angles;
}


bool AngleImage::pixel(int x, int y) const
{
    assert(x>=0 && x<width() && y>=0 && y<height());

    return src->pixel(x,y);
}
int AngleImage::width()
{
    return src->width();
}
int AngleImage::height()
{
    return src->height();
}

void AngleImage::setPixelSlope(const QPoint &p, double angle, double strength)
{
    setPixelSlope(p.x(),p.y(),slope,strength);
}

void AngleImage::setPixelSlope(int x, int y, double angle, double strength)
{
    assert(x>=0 && x<width() && y>=0 && y<height());
    if (!angles[x][y].contains(angle))
        angles[x][y][angle]=strength;
    else
        angles[x][y][angle]=max(strength,angles[x][y][angle]);
    
//    double portionLeft=portion;
//    if (angles[x][y].contains(-1))
//    {
//        portionLeft = max(portionLeft-angles[x][y][-1],0);
//        angles[x][y][-1]-=portion-portionLeft;
//        if (angles[x][y][-1]==0)
//            angles[x][y].remove(-1);
//    }
//    if (portionLeft>0)
//    {
//        if (angles[x][y].size() > 0)
//        {
//            double old = 0;
//            if (angles[x][y].contains(angle))
//            {
//                old = pixels[x][y][angle];
//            }
//            double converter = (1-portionLeft)/(1-old);
//            QMap<double,double>::iterator i = angles[x][y].begin();
//            while(i != angles[x][y].end())
//            {
//                if (i.key() != angle)
//                {
//                    double oldother = i.value();
//                    if (oldother!=0)
//                    {
//                        i.value()=oldother*converter;
//                    }
//                }
//                ++i;
//            }
//        }
        
//    }
//    pixels[x][y][angle]=portion;
}



BImage AngleImage::makeImage() const
{
    return src->makeImage();
}

bool AngleImage::pixelIsMine(int x, int y) const{return true;}

void AngleImage::setNumOfBinsMinValMaxVal(int numOfBins, double minVal, double maxVal)
{
    this->numOfBins = numOfBins;
    minValue=minVal;
    maxValue=maxVal;
}

int AngleImage::getNumOfBins()
{
    return numOfBins;
}

QMap<int,double> AngleImage::getBinsAndStrForPixel(int x, int y)
{
    QMap<int,double> ret;
    double totalStr=0;
    foreach (double str, angles[x][y].values())
    {
        totalStr+=str;
    }

    foreach (double angle, angles[x][y].keys())
    {
        int bin = (int)((angle-minValue)*((numOfBins-1)/(maxValue-minValue)));
        double strength = angles[x][y][angle]/totalStr;
        ret[bin]=strength;
    }

    return ret;
}

bool AngleImage::noAngleForPixel(int x, int y, double angle)
{
    return !angles[x][y].contains(angle);
}

//////////////////

AngleIndexer::AngleIndexer(int width, int height)
{
    this->width=width;
    this->height=height;
}


int AngleIndexer::getIndex(int x, int y, int angleBin) const
{
    return angleBin*width*height + y*width + x;
}


