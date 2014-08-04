#include "bimage.h"

#include <stdio.h>
#include <fstream>

BImage::BImage()
{
    myHeight=0;
    myWidth=0;
}

BImage::BImage(const QImage &src)
{
    myWidth = src.width();
    myHeight = src.height();
    pixels = new bPixel*[myWidth];
    for (int x=0; x<myWidth; x++)
    {
        pixels[x] = new bPixel[myHeight];
        for (int y=0; y<myHeight; y++)
        {
            pixels[x][y].val= qGray(src.pixel(x,y)) == BLACK;
        }
    }
}

BImage::BImage(int width, int height)
{
    myWidth=width;
    myHeight=height;
    pixels = new bPixel*[myWidth];
    for (int x=0; x<myWidth; x++)
    {
        pixels[x] = new bPixel[myHeight];
        for (int y=0; y<myHeight; y++)
        {
            pixels[x][y].val= false;
        }
    }
}

BImage::BImage(const BImage &other)
{
    myWidth = other.width();
    myHeight = other.height();
    pixels = new bPixel*[myWidth];
    for (int x=0; x<myWidth; x++)
    {
        pixels[x] = new bPixel[myHeight];
        for (int y=0; y<myHeight; y++)
        {
            pixels[x][y].val= other.pixel(x,y);
        }
    }
}

BImage::BImage(const BPixelCollection &other)
{
    myWidth = other.width();
    myHeight = other.height();
    pixels = new bPixel*[myWidth];
    for (int x=0; x<myWidth; x++)
    {
        pixels[x] = new bPixel[myHeight];
        for (int y=0; y<myHeight; y++)
        {
            pixels[x][y].val= other.pixel(x,y);
        }
    }
}

//BImage::BImage(const BPartition* src1, const BPartition* src2)
//{
//    assert(src1->getSrc()==src2->getSrc());
//    myWidth = src1->getSrc()->width();
//    myHeight = src1->getSrc()->height();
//    for (int x=0; x<myWidth; x++)
//    {
//        for (int y=0; y<myHeight; y++)
//        {
//            if (src1->pixel(x,y) || src2->pixel(x,y))
//                ret.setPixel(x,y,true);
//        }
//    }
//    return ret;
//}

BImage::~BImage()
{
    for (int x=0; x<myWidth; x++)
    {
        delete[] pixels[x];
        pixels[x]=NULL;
    }
    delete[] pixels;
    pixels=NULL;
}

BImage& BImage::operator=( const BImage& other )
{
    if (myWidth != 0)
    {
        for (int x=0; x<myWidth; x++)
        {
            delete[] pixels[x];
            pixels[x]=NULL;
        }
        delete[] pixels;
        pixels=NULL;
    }
    
    myHeight = other.myHeight;
    myWidth = other.myWidth;
    pixels = new bPixel*[myWidth];
    for (int x=0; x<myWidth; x++)
    {
        pixels[x] = new bPixel[myHeight];
        for (int y=0; y<myHeight; y++)
        {
            pixels[x][y].val= other.pixel(x,y);
        }
    }
    return *this;
  }

BImage BImage::copy()//this doesn't handle paritions
{
    BImage ret(myWidth,myHeight);
    for (int x=0; x<myWidth; x++)
        for (int y=0; y<myHeight; y++)
        {
            ret.setPixelFull(x,y,pixels[x][y]);
        }
    
    return ret;
}


void BImage::save(const QString& filepath)
{
    QImage temp = getImage();
    temp.save(filepath);
}

void BImage::saveOwners(const QString& filepath)
{
    QImage temp = getOwnersImage();
    bool test = temp.save(filepath);
    if (!test)
        printf("ERROR: failed to saveOwners\n");
}

bool BImage::pixel(const QPoint &p) const
{
    assert(p.x()>=0 && p.x()<myWidth && p.y()>=0 && p.y()<myHeight);
    return pixels[p.x()][p.y()].val;
}
bool BImage::pixel(int x, int y) const
{
    assert(x>=0 && x<myWidth && y>=0 && y<myHeight);
    return pixels[x][y].val;
}
bPixel BImage::pixelFull(const QPoint &p) const
{
    assert(p.x()>=0 && p.x()<myWidth && p.y()>=0 && p.y()<myHeight);
    return pixels[p.x()][p.y()];
}
bPixel BImage::pixelFull(int x, int y) const
{
    assert(x>=0 && x<myWidth && y>=0 && y<myHeight);
    return pixels[x][y];
}

float BImage::pixelOwnerPortion(const QPoint &p, BPartition* owner) const
{
    assert(p.x()>=0 && p.x()<myWidth && p.y()>=0 && p.y()<myHeight);
    if (pixels[p.x()][p.y()].ownership.contains(owner->id()))
        return pixels[p.x()][p.y()].ownership[owner->id()];
    else 
        return 0;
}
float BImage::pixelOwnerPortion(int x, int y, BPartition* owner) const
{
    assert(x>=0 && x<myWidth && y>=0 && y<myHeight);
    if (pixels[x][y].ownership.contains(owner->id()))
        return pixels[x][y].ownership[owner->id()];
    else 
        return 0;
}

int BImage::pixelMajorityOwner(const QPoint &p) const
{
    return pixelMajorityOwner(p.x(),p.y());
}

int BImage::pixelMajorityOwner(int x, int y) const
{
    assert(x>=0 && x<myWidth && y>=0 && y<myHeight);
    int mostId=-1;
    float most=0;
    QMap<int,float>::const_iterator i = pixels[x][y].ownership.constBegin();
    while (i != pixels[x][y].ownership.constEnd())
    {
        if (i.value() > most)
        {
            most = i.value();
            mostId = i.key();
        }
        ++i;
    }
    return mostId;
}

void BImage::setPixel(const QPoint &p, bool val)
{
    assert(p.x()>=0 && p.x()<myWidth && p.y()>=0 && p.y()<myHeight);
    pixels[p.x()][p.y()].val=val;
}

void BImage::setPixel(int x, int y, bool val)
{
    assert(x>=0 && x<myWidth && y>=0 && y<myHeight);
    pixels[x][y].val=val;
}

void BImage::setPixelFull(const QPoint &p, const bPixel &strct)
{
    assert(p.x()<myWidth && p.y()<myHeight);
    pixels[p.x()][p.y()].val=strct.val;
    pixels[p.x()][p.y()].ownership.unite(strct.ownership);
}

void BImage::setPixelFull(int x, int y, const bPixel &strct)
{
    assert(x<myWidth && y<myHeight);
    pixels[x][y].val=strct.val;
    pixels[x][y].ownership.unite(strct.ownership);
}

void BImage::setPixelOwner(const QPoint &p, BPartition* owner, float portion)
{
    setPixelOwner(p.x(),p.y(),owner,portion);
}

void BImage::setPixelOwner(int x, int y, BPartition* owner, float portion)
{
    assert(x>=0 && x<myWidth && y>=0 && y<myHeight);
    
    if (pixels[x][y].ownership.size() > 0)
    {
        float old = 0;
        if (pixels[x][y].ownership.contains(owner->id()))
        {
            old = pixels[x][y].ownership[owner->id()];
        }
        float converter = (1-portion)/(1-old);
        QMap<int,float>::iterator i = pixels[x][y].ownership.begin();
        while(i != pixels[x][y].ownership.end())
        {
            if (i.key() != owner->id())
            {
                float oldother = i.value();
                if (oldother!=0)
                {
                    i.value()=oldother*converter;
    //                (i.key())->changedPortion(x,y,i.value(),oldother);
                }
            }
            ++i;
        }
    }
    pixels[x][y].ownership[owner->id()]=portion;
//    owner->changedPortion(x,y,portion,old);
}

//void BImage::setPixelCombineOwner(int x, int y, BPartition* to, BPartition* with)
//{
//    assert(x>=0 && x<myWidth && y>=0 && y<myHeight);
    
//    pixels[x][y].ownership[to] += pixels[x][y].ownership[with];
//    pixels[x][y].ownership[with] = 0;
//}

QImage BImage::getImage()
{
    QVector<QRgb> default_color_table;
    default_color_table.append(qRgb(0,0,0));
    default_color_table.append(qRgb(255,255,255));
    
    QImage ret(myWidth,myHeight,QImage::Format_Indexed8);
    ret.setColorTable(default_color_table);
    for (int x=0; x<myWidth; x++)
        for (int y=0; y<myHeight; y++)
        {
            if (pixels[x][y].val)
                ret.setPixel(x,y,0);
            else
                ret.setPixel(x,y,1);
        }
    
    return ret;
}

QImage BImage::getOwnersImage()
{
    QVector<QRgb> color_table;
    color_table.append(qRgb(0,0,0));
    color_table.append(qRgb(255,255,255));
    
    color_table.append(qRgb(3,21,255));
    color_table.append(qRgb(123,136,255));
    
    color_table.append(qRgb(255,0,0));
    color_table.append(qRgb(255,120,120));
    
    color_table.append(qRgb(0,184,179));
    color_table.append(qRgb(149,255,252));
    
    
    color_table.append(qRgb(235,0,135));
    color_table.append(qRgb(235,132,192));
    
    color_table.append(qRgb(179,212,0));
    color_table.append(qRgb(221,242,133));
    
    color_table.append(qRgb(225,94,13));
    color_table.append(qRgb(224,147,125));
    
    color_table.append(qRgb(21,212,0));
    color_table.append(qRgb(193,255,149));
    
    color_table.append(qRgb(207,178,3));
    color_table.append(qRgb(255,241,159));
    
    color_table.append(qRgb(122,0,122));
    color_table.append(qRgb(224,0,223));
    
    QMap<int,int> partitionIndex;
    int currentIndex=0;
    
    QImage ret(myWidth,myHeight,QImage::Format_Indexed8);
    ret.setColorTable(color_table);
    for (int y=0; y<myHeight; y++)
        for (int x=0; x<myWidth; x++)
        {
            if (pixels[x][y].ownership.size()>0)
            {
            
                int mostId = pixelMajorityOwner(x,y);
                
                if (mostId!=-1)
                {
                    if (!partitionIndex.contains(mostId))
                    {
                        currentIndex = (1+currentIndex)%((color_table.size()-2)/2);
                        partitionIndex[mostId]=currentIndex;
                    }
                    
                    if (pixels[x][y].val)
                        ret.setPixel(x,y,2+partitionIndex[mostId]*2);
                    else
                        ret.setPixel(x,y,2+partitionIndex[mostId]*2 + 1);
                    
                    continue;
                }
            }
            
            if (pixels[x][y].val)
                ret.setPixel(x,y,0);
            else
                ret.setPixel(x,y,1);
            
        }
    
    return ret;
}

void BImage::saveICDAR(QString name)
{
    int* out = new int[myHeight*myWidth];
    int currentIndex=0;
    QMap<int,int> partitionIndex;
    for (int y=0; y<myHeight; y++)
        for (int x=0; x<myWidth; x++)
        {
            if (pixels[x][y].val)
            {
                if (pixels[x][y].ownership.size()>0)
                {
                
                    int mostId = pixelMajorityOwner(x,y);
                    
                    if (mostId!=-1)
                    {
                        if (!partitionIndex.contains(mostId))
                        {
//                            currentIndex++;
                            partitionIndex[mostId]=++currentIndex;
//                            printf("new index %d\n",currentIndex);
                        }
                        
                        out[x+y*myWidth]=2+partitionIndex[mostId];
                        
                        continue;
                    }
                }
                out[x+y*myWidth]=0;
//                printf("error, unclaimed pixel (%d,%d)\n",x,y);
            }
            else
                out[x+y*myWidth]=0;
            
        }
    
    std::ofstream outfile(name.toLocal8Bit().data(), std::ios::out | std::ios::binary);
    outfile.write(reinterpret_cast<const char *>(out),sizeof(int)*myWidth*myHeight);
    outfile.close();
    delete[] out;
}

void BImage::claimOwnership(BPartition* claimer, float amount)
{
    assert(claimer->getSrc() == this);
    for (int x=0; x<claimer->width(); x++)
    {
         for (int y=0; y<claimer->height(); y++)
         {
             if (claimer->pixelIsMine(x,y))
                setPixelOwner(x+claimer->getXOffset(),y+claimer->getYOffset(),claimer,amount);
         }
    }
}

void BImage::claimOwnershipVia(BPartition* intermediate, BPartition* claimer, float amount)
{
    assert(intermediate->getSrc() == this);
    assert(claimer->getSrc() == intermediate);
    for (int x=0; x<claimer->width(); x++)
    {
         for (int y=0; y<claimer->height(); y++)
         {
             if (claimer->pixelIsMine(x,y))
                setPixelOwner(x+claimer->getXOffset()+intermediate->getXOffset(),y+claimer->getYOffset()+intermediate->getYOffset(),claimer,amount);
         }
    }
}

int BImage::width() const
{
    return myWidth;
}
int BImage::height() const
{
    return myHeight;
}

BPartition* BImage::getFullPartition()
{
    BPartition* ret = new BPartition(this);
    for (int x=0; x<myWidth; x++)
    {
        for (int y=0; y<myHeight; y++)
        {
//            setPixelOwner(x,y,ret,1);
            ret->addPixelFromSrc(x,y);
        }
    }
    return ret;
}

bool BImage::pixelIsMine(int x, int y) const
{
    assert(x>=0 && x<myWidth && y>=0 && y<myHeight);
    return true;
}

BImage BImage::makeImage() const
{
    BImage ret(*this);
    return ret;
}
