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

bool AngleImage::pixel(const QPoint &p) const
{
    assert(p.x()>=0 && p.x()<width() && p.y()>=0 && p.y()<height());

    return src->pixel(p.x(),p.y()   );
}
bool AngleImage::pixel(int x, int y) const
{
    assert(x>=0 && x<width() && y>=0 && y<height());

    return src->pixel(x,y);
}
int AngleImage::width() const
{
    return src->width();
}
int AngleImage::height() const
{
    return src->height();
}

void AngleImage::setPixelSlope(const QPoint &p, double angle, double strength)
{
    setPixelSlope(p.x(),p.y(),angle,strength);
}

void AngleImage::setPixelSlope(int x, int y, double angle, double strength)
{
    assert(x>=0 && x<width() && y>=0 && y<height());
    if (!angles[x][y].contains(angle))
        angles[x][y][angle]=strength;
    else
        angles[x][y][angle]=std::max(strength,angles[x][y][angle]);
    
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

int AngleImage::getNumOfBins() const
{
    return numOfBins;
}

QMap<int,double> AngleImage::getBinsAndStrForPixel(int x, int y) const
{
    QMap<int,double> ret;
    double totalStr=0;
    double maxStr=0;
    foreach (double str, angles[x][y].values())
    {
        totalStr+=str;
        if (str>maxStr)
            maxStr=str;
    }

    foreach (double angle, angles[x][y].keys())
    {
        int bin = (int)((angle-minValue)*((numOfBins-1)/(maxValue-minValue)));
        double strength = angles[x][y][angle]/maxStr;
        ret[bin]=strength;
    }

    return ret;
}

bool AngleImage::noStrongerAngleForPixel(int x, int y, double angle, double strength) const
{
    if (pixel(x,y))
    {
        if(!angles[x][y].contains(angle))
            return true;
        else
            return angles[x][y][angle]<strength;
    }
    return false;
}

bool AngleImage::noAnglesForPixel(int x, int y) const
{
    return pixel(x,y) && angles[x][y].empty();
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


