#include "gpartition.h"

#include <stdio.h>
#include <math.h>
#include <assert.h>

GPartition::GPartition(const GPixelCollection* ofImage)
{
    src=ofImage;
    rootSrc=NULL;
    leftX=-1;
    rightX=-2;
    upperY=-1;
    lowerY=-2;
//    myPixels = new unsigned int[(int)ceil(src->width()*src->height() / ((8*sizeof(unsigned int))) * 1.0)];
//    for (int i=0; i<(int)ceil(src->width()*src->height() / ((8*sizeof(unsigned int))) * 1.0); i++)
//        myPixels[i]=0;
    initMyPixels();
}
GPartition::GPartition(const GImage* ofImage)
{
    src=ofImage;
    rootSrc=ofImage;
    leftX=-1;
    rightX=-2;
    upperY=-1;
    lowerY=-2;
//    myPixels = new unsigned int[(int)ceil(src->width()*src->height() / ((8*sizeof(unsigned int))) * 1.0)];
//    for (int i=0; i<(int)ceil(src->width()*src->height() / ((8*sizeof(unsigned int))) * 1.0); i++)
//        myPixels[i]=0;
    initMyPixels();
}
GPartition::GPartition(const GPartition* ofImage)
{
    src=ofImage;
    rootSrc=ofImage->rootSrc;
    leftX=-1;
    rightX=-2;
    upperY=-1;
    lowerY=-2;
//    myPixels = new unsigned int[(int)ceil(src->width()*src->height() / ((8*sizeof(unsigned int))) * 1.0)];
//    for (int i=0; i<(int)ceil(src->width()*src->height() / ((8*sizeof(unsigned int))) * 1.0); i++)
//        myPixels[i]=0;
    initMyPixels();
}

GPartition::~GPartition()
{
    for (int i=0; i<src->width(); i++)
    {
        delete[] myPixels[i];
    }
    delete[] myPixels;
}

GPartition& GPartition::operator=( const GPartition& other )
{
    src = other.getSrc();
    rootSrc = other.rootSrc;
    leftX=other.leftX;
    rightX=other.rightX;
    upperY=other.upperY;
    lowerY=other.lowerY;
    
//    myPixels = new unsigned int[(int)ceil(src->width()*src->height() / ((8*sizeof(unsigned int))) * 1.0)];
//    for (int i=0; i<(int)ceil(src->width()*src->height() / ((8*sizeof(unsigned int))) * 1.0); i++)
//        myPixels[i]=0;
    initMyPixels();
    for (int x=0; x<width(); x++)
    {
        for(int y=0; y<height(); y++)
        {
            if (other.pixelIsMine(x,y))
                setMyPixelTrue(x+leftX,y+upperY);
        }
    }
        
        return *this;
}


void GPartition::initMyPixels()
{
    myPixels = new bool*[src->width()];
    for (int i=0; i<src->width(); i++)
    {
        myPixels[i] = new bool[src->height()];
        for (int j=0; j<src->height(); j++)
        {
            myPixels[i][j] = false;
        }
    }
}

const GPixelCollection* GPartition::getSrc() const
{
    return src;
}

int GPartition::width() const
{
    return 1+rightX-leftX;
}

int GPartition::height() const
{
    return 1+lowerY-upperY;
}

int GPartition::getXOffset()
{
    return leftX;
}

int GPartition::getYOffset()
{
    return upperY;
}

//bool BPartition::pixel(const QPoint &p) const
//{
//    return pixel(p.x(),p.y());
//}
int GPartition::pixel(int x, int y) const
{
    assert(x>=0 && x<width() && y>=0 && y<height());
    
    if (pixelIsMine(x,y))
    {
        int realX = x+leftX;
        int realY = y+upperY;
        return src->pixel(realX,realY);
    }
    else
        return 0;
}

bool GPartition::pixelIsMineSrc(const QPoint &src_p) const
{
    assert(src_p.x()>=0 && src_p.x()<src->width() && src_p.y()>=0 && src_p.y()<src->height());
//    int index = src_p.x() + src_p.y()*src->width();
//    int arrayIndex = index/(8*sizeof(unsigned int));
//    int intIndex = index%(8*sizeof(unsigned int));
//    return myPixels[arrayIndex] & (1 << intIndex);
    return myPixels[src_p.x()][src_p.y()];
}
bool GPartition::pixelIsMine(int x, int y) const
{
    assert(x>=0 && x<width() && y>=0 && y<height());
    int realX = x+leftX;
    int realY = y+upperY;
//    int index = realX + realY*src->width();
//    int arrayIndex = index/(8*sizeof(unsigned int));
//    int intIndex = index%(8*sizeof(unsigned int));
//    return myPixels[arrayIndex] & (1 << intIndex);
    return myPixels[realX][realY];
}

//void BPartition::(const QPoint &p, BPartition* owner, float portion)
//{
//    setPixelOwner(p.x(),p.y(),owner,portion);
//}

//void BPartition::setPixelOwner(int x, int y, BPartition* owner, float portion)
//{
//    src->setPixelOwner(x+leftX,y+upperY,owner,portion);
//}

//void BPartition::changedPortion(const QPoint &p, float newPortion, float oldPortion)
//{
//    changedPortion(p.x(),p.y(),newPortion,oldPortion);
//}

//void BPartition::changedPortion(int x, int y, float newPortion, float oldPortion)
//{
////    printf("changedPortion(%d,%d,%f,%f)  lx:%d rx:%d uy:%d ly:%d this:%x\n",x,y,newPortion,oldPortion,leftX,rightX,upperY,lowerY,this);
//    if (newPortion==0 && oldPortion>0)
//    {
//        bool found = true;
//        if (x==leftX)
//        {
//            found=false;
//            for (int i=leftX; i<=rightX; i++)
//            {
//                for (int j=upperY; j<=lowerY; j++)
//                {
//                    if (src->pixelOwnerPortion(i,j,this)>0 && src->pixel(i,j))
//                    {
//                        leftX=i;
//                        i=rightX+1;
//                        j=lowerY+1;
//                        found=true;
//                    }
//                }
//            }
//        }
//        else if (x==rightX)
//        {
//            found = false;
//            for (int i=rightX; i>=leftX; i--)
//            {
//                for (int j=upperY; j<=lowerY; j++)
//                {
//                    if (src->pixelOwnerPortion(i,j,this)>0 && src->pixel(i,j))
//                    {
//                        rightX=i;
//                        i=-1;
//                        j=lowerY+1;
//                        found=true;
//                    }
//                }
//            }
//        }
        
//        if (y==upperY)
//        {
//            found = false;
//            for (int j=upperY; j<=lowerY; j++)
//            {
//                for (int i=leftX; i<=rightX; i++)
//                {
//                    if (src->pixelOwnerPortion(i,j,this)>0 && src->pixel(i,j))
//                    {
//                        upperY=j;
//                        i=rightX+1;
//                        j=lowerY+1;
//                        found=true;
//                    }
//                }
//            }
//        }
//        else if (y==lowerY)
//        {
//            found = false;
//            for (int j=lowerY; j>=upperY; j--)
//            {
//                for (int i=leftX; i<=rightX; i++)
//                {
//                    if (src->pixelOwnerPortion(i,j,this)>0 && src->pixel(i,j))
//                    {
//                        lowerY=j;
//                        i=rightX+1;
//                        j=-1;
//                        found=true;
//                    }
//                }
//            }
//        }
//        if (!found)
//        {
//            leftX=-1;
//            rightX=-2;
//            upperY=-1;
//            lowerY=-2;
//        }
//    }
//    else if (newPortion>0 && oldPortion==0)
//    {
//        if (leftX==-1)
//        {
//            leftX=x;
//            rightX=x;
//            upperY=y;
//            lowerY=y;
//        }
//        else
//        {
//            if (x<leftX)
//                leftX=x;
//            else if (x>rightX)
//                rightX=x;
            
//            if (y<upperY)
//                upperY=y;
//            else if (y>lowerY)
//                lowerY=y;
//        }
        
//    }
//}
QImage GPartition::makeImage() const
{
    QImage ret(width(),height(),QImage::Format_Indexed8);
    QVector<QRgb> default_color_table;
    for (int i=0; i<256; i++)
    {
        default_color_table.append(qRgb(i,i,i));
    }
    ret.setColorTable(default_color_table);
    for (int x=0; x<width(); x++)
    {
        for (int y=0; y<height(); y++)
        {
            ret.setPixel(x,y,pixel(x,y));
        }
    }
    return ret;
}

void GPartition::addPixelFromSrc(const QPoint &src_p)
{
    addPixelFromSrc(src_p.x(),src_p.y());
}

void GPartition::addPixelFromSrc(int src_x, int src_y)
{
    assert(src_x>=0 && src_x<src->width() && src_y>=0 && src_y<src->height());
    setMyPixelTrue(src_x,src_y);
    if (leftX==-1)
    {
        leftX=src_x;
        rightX=src_x;
        upperY=src_y;
        lowerY=src_y;
    }
    else
    {
        if (src_x<leftX)
            leftX=src_x;
        else if (src_x>rightX)
            rightX=src_x;
        
        if (src_y<upperY)
            upperY=src_y;
        else if (src_y>lowerY)
            lowerY=src_y;
    }
}

void GPartition::removePixel(const QPoint &src_p)
{
    removePixel(src_p.x(),src_p.y());
}

void GPartition::removePixel(int src_x, int src_y)
{
    setMyPixelFalse(src_x,src_y);
    
    bool found = true;
    if (src_x==leftX)
    {
        found=false;
        for (int i=leftX; i<=rightX; i++)
        {
            for (int j=upperY; j<=lowerY; j++)
            {
                if (pixelIsMine(i-leftX,j-upperY) && src->pixel(i,j))
                {
                    leftX=i;
                    i=rightX+1;
                    j=lowerY+1;
                    found=true;
                }
            }
        }
    }
    else if (src_x==rightX)
    {
        found = false;
        for (int i=rightX; i>=leftX; i--)
        {
            for (int j=upperY; j<=lowerY; j++)
            {
                if (pixelIsMine(i-leftX,j-upperY) && src->pixel(i,j))
                {
                    rightX=i;
                    i=-1;
                    j=lowerY+1;
                    found=true;
                }
            }
        }
    }
    
    if (src_y==upperY)
    {
        found = false;
        for (int j=upperY; j<=lowerY; j++)
        {
            for (int i=leftX; i<=rightX; i++)
            {
                if (pixelIsMine(i-leftX,j-upperY) && src->pixel(i,j))
                {
                    upperY=j;
                    i=rightX+1;
                    j=lowerY+1;
                    found=true;
                }
            }
        }
    }
    else if (src_y==lowerY)
    {
        found = false;
        for (int j=lowerY; j>=upperY; j--)
        {
            for (int i=leftX; i<=rightX; i++)
            {
                if (pixelIsMine(i-leftX,j-upperY) && src->pixel(i,j))
                {
                    lowerY=j;
                    i=rightX+1;
                    j=-1;
                    found=true;
                }
            }
        }
    }
    if (!found)
    {
        leftX=-1;
        rightX=-2;
        upperY=-1;
        lowerY=-2;
    }
}

//void GPartition::joinInto(BPartition* other)
//{
//    assert(false);//imcomplete
//    for (int x=leftX; x<=rightX; x++)
//    {
//        for (int y=upperY; y<= lowerY; y++)
//        {
//            //src->setPixelCombineOwner(x,y,other,this);
//            addPixelFromSrc(x,y);
//        }
//    }
//}

//void BPartition::remove()
//{
//    for (int x=leftX; x<=rightX; x++)
//    {
//        for (int y=upperY; y<= lowerY; y++)
//        {
//            src->setPixelOwner(x,y,this,0);
//        }
//    }
//}

void GPartition::clear(GPartition* const pixelsToRemove)
{
    assert(src == pixelsToRemove->getSrc());
    bool checkLeftBound = false;
    bool checkRightBound = false;
    bool checkUpperBound = false;
    bool checkLowerBound = false;
    for (int x=0; x<pixelsToRemove->width(); x++)
    {
        for (int y=0; y<pixelsToRemove->height(); y++)
        {
            if (pixelsToRemove->pixelIsMine(x,y))
            {
                int realX = x+pixelsToRemove->getXOffset();
                int realY = y+pixelsToRemove->getYOffset();
                setMyPixelFalse(realX,realY);
                if (x==leftX) checkLeftBound = true;
                if (x==rightX) checkRightBound = true;
                if (y==upperY) checkUpperBound = true;
                if (y==lowerY) checkLowerBound = true;
                
            }
            
        }
    }
    bool found = true;
    if (checkLeftBound)
    {
        found=false;
        for (int i=leftX; i<=rightX; i++)
        {
            for (int j=upperY; j<=lowerY; j++)
            {
                if (pixelIsMine(i-leftX,j-upperY) && src->pixel(i,j))
                {
                    leftX=i;
                    i=rightX+1;
                    j=lowerY+1;
                    found=true;
                }
            }
        }
    }
    if (checkRightBound)
    {
        found = false;
        for (int i=rightX; i>=leftX; i--)
        {
            for (int j=upperY; j<=lowerY; j++)
            {
                if (pixelIsMine(i-leftX,j-upperY) && src->pixel(i,j))
                {
                    rightX=i;
                    i=-1;
                    j=lowerY+1;
                    found=true;
                }
            }
        }
    }
    
    if (checkUpperBound)
    {
        found = false;
        for (int j=upperY; j<=lowerY; j++)
        {
            for (int i=leftX; i<=rightX; i++)
            {
                if (pixelIsMine(i-leftX,j-upperY) && src->pixel(i,j))
                {
                    upperY=j;
                    i=rightX+1;
                    j=lowerY+1;
                    found=true;
                }
            }
        }
    }
    if (checkLowerBound)
    {
        found = false;
        for (int j=lowerY; j>=upperY; j--)
        {
            for (int i=leftX; i<=rightX; i++)
            {
                if (pixelIsMine(i-leftX,j-upperY) && src->pixel(i,j))
                {
                    lowerY=j;
                    i=rightX+1;
                    j=-1;
                    found=true;
                }
            }
        }
    }
    if (!found)
    {
        leftX=-1;
        rightX=-2;
        upperY=-1;
        lowerY=-2;
    }
}

//void BPartition::shiftDownLayer(BPartition* mySrc)
//{
//    assert(src == mySrc);
//    src = mySrc->getSrc();
//    leftX += mySrc->getXOffset();
//    rightX += mySrc->getXOffset();
//    upperY + mySrc->getYOffset();
//    lowerY + mySrc->getYOffset();
    
//    int tempPixels[] = new unsigned int[(int)ceil(src->width()*src->height() / ((8*sizeof(unsigned int))) * 1.0)];
//}

void GPartition::trim(int offLeftSide, int offRightSide, int offTopSide, int offBottomSide)
{
    assert(offLeftSide>=0 && offRightSide>=0 && offTopSide>=0 && offBottomSide>=0);
    for (int i = 0; i<offLeftSide; i++)
    {
        for (int y=upperY; y<=lowerY; y++)
        {
            setMyPixelFalse(i+leftX,y);
        }
    }
    leftX += offLeftSide;
    
    for (int i = 0; i<offRightSide; i++)
    {
        for (int y=upperY; y<=lowerY; y++)
        {
            setMyPixelFalse(rightX-i,y);
        }
    }
    rightX -= offRightSide;
    
    for (int j = 0; j<offTopSide; j++)
    {
        for (int x=leftX; x<=rightX; x++)
        {
            setMyPixelFalse(x,j+upperY);
        }
    }
    upperY += offTopSide;
    
    for (int j = 0; j<offBottomSide; j++)
    {
        for (int x=leftX; x<=rightX; x++)
        {
            setMyPixelFalse(x,lowerY-j);
        }
    }
    lowerY -= offBottomSide;
}

inline void GPartition::setMyPixelTrue(int src_x, int src_y)
{
//    int index = src_x + src_y*src->width();
//    int arrayIndex = index/(8*sizeof(unsigned int));
//    int intIndex = index%(8*sizeof(unsigned int));
//    myPixels[arrayIndex] |= (1 << intIndex);
    myPixels[src_x][src_y] = true;
}

inline void GPartition::setMyPixelFalse(int src_x, int src_y)
{
//    int index = src_x + src_y*src->width();
//    int arrayIndex = index/(8*sizeof(unsigned int));
//    int intIndex = index%(8*sizeof(unsigned int));
//    myPixels[arrayIndex] &= ~(1 << intIndex);
    myPixels[src_x][src_y] = false;
}

//bool BPartition::rebase()
//{
//    if (rootSrc==src || rootSrc == NULL)
//        return false;
    
//    while(rootSrc!=src)
//    {
        
//    }
//    myPixels = new bool*[src->width()];
//    for (int i=0; i<src->width(); i++)
//    {
//        myPixels[i] = new bool[src->height()];
//        for (int j=0; j<src->height(); j++)
//        {
//            myPixels[i][j] = false;
//        }
//    }
    
//    return true;
//}

void GPartition::changeSrc(const GPixelCollection* newSrc, int srcXOffset, int srcYOffset)
{
//    assert(newSrc->width()==src->width() && newSrc->height()==src->height());
    bool** newMyPixels = new bool*[newSrc->width()];
    for (int i=0; i<newSrc->width(); i++)
    {
        newMyPixels[i] = new bool[newSrc->height()];
        for (int j=0; j<newSrc->height(); j++)
        {
            newMyPixels[i][j] = false;
        }
    }
    for (int x=leftX; x<=rightX; x++)
    {
        for (int y=upperY; y<= lowerY; y++)
        {
            newMyPixels[x+srcXOffset][y+srcYOffset]=pixel(x-leftX,y-upperY);
        }
    }
    leftX += srcXOffset;
    rightX += srcXOffset;
    upperY += srcYOffset;
    lowerY += srcYOffset;
    
    for (int i=0; i<src->width(); i++)
        delete[] myPixels[i];
//    delete[] myPixels;
    myPixels=newMyPixels;
    src=newSrc;
}

//void BPartition::makeFull()
//{
//    leftX=0;
//    upperY=0;
//    rightX=src->width()-1;
//    lowerY=src->height()-1;
//}
