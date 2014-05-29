#include "bpartition.h"
#include <stdio.h>

BPartition::BPartition(BImage* ofImage)
{
    src=ofImage;
    leftX=-1;
    rightX=-2;
    upperY=-1;
    lowerY=-2;
}

BImage* BPartition::getSrc()
{
    return src;
}

int BPartition::width() const
{
    return 1+rightX-leftX;
}

int BPartition::height() const
{
    return 1+lowerY-upperY;
}

int BPartition::getXOffset()
{
    return leftX;
}

int BPartition::getYOffset()
{
    return upperY;
}

bool BPartition::pixel(const QPoint &p)
{
    return pixel(p.x(),p.y());
}
bool BPartition::pixel(int x, int y)
{
    assert(x>=0 && x<width() && y>=0 && y<height());
    int realX = x+leftX;
    int realY = y+upperY;
    if (src->pixelOwnerPortion(realX,realY,this) > 0)
    {
        return src->pixel(realX,realY);
    }
    else
        return false;
}

bool BPartition::pixelIsMine(const QPoint &p)
{
    return pixelIsMine(p.x(),p.y());
}
bool BPartition::pixelIsMine(int x, int y)
{
    assert(x>=0 && x<width() && y>=0 && y<height());
    int realX = x+leftX;
    int realY = y+upperY;
    return (src->pixelOwnerPortion(realX,realY,this) > 0);
}

//void BPartition::(const QPoint &p, BPartition* owner, float portion)
//{
//    setPixelOwner(p.x(),p.y(),owner,portion);
//}

//void BPartition::setPixelOwner(int x, int y, BPartition* owner, float portion)
//{
//    src->setPixelOwner(x+leftX,y+upperY,owner,portion);
//}

void BPartition::changedPortion(const QPoint &p, float newPortion, float oldPortion)
{
    changedPortion(p.x(),p.y(),newPortion,oldPortion);
}

void BPartition::changedPortion(int x, int y, float newPortion, float oldPortion)
{
//    printf("changedPortion(%d,%d,%f,%f)  lx:%d rx:%d uy:%d ly:%d this:%x\n",x,y,newPortion,oldPortion,leftX,rightX,upperY,lowerY,this);
    if (newPortion==0 && oldPortion>0)
    {
        bool found = true;
        if (x==leftX)
        {
            found=false;
            for (int i=leftX; i<=rightX; i++)
            {
                for (int j=upperY; j<=lowerY; j++)
                {
                    if (src->pixelOwnerPortion(i,j,this)>0 && src->pixel(i,j))
                    {
                        leftX=i;
                        i=rightX+1;
                        j=lowerY+1;
                        found=true;
                    }
                }
            }
        }
        else if (x==rightX)
        {
            found = false;
            for (int i=rightX; i>=leftX; i--)
            {
                for (int j=upperY; j<=lowerY; j++)
                {
                    if (src->pixelOwnerPortion(i,j,this)>0 && src->pixel(i,j))
                    {
                        rightX=i;
                        i=-1;
                        j=lowerY+1;
                        found=true;
                    }
                }
            }
        }
        
        if (y==upperY)
        {
            found = false;
            for (int j=upperY; j<=lowerY; j++)
            {
                for (int i=leftX; i<=rightX; i++)
                {
                    if (src->pixelOwnerPortion(i,j,this)>0 && src->pixel(i,j))
                    {
                        upperY=j;
                        i=rightX+1;
                        j=lowerY+1;
                        found=true;
                    }
                }
            }
        }
        else if (y==lowerY)
        {
            found = false;
            for (int j=lowerY; j>=upperY; j--)
            {
                for (int i=leftX; i<=rightX; i++)
                {
                    if (src->pixelOwnerPortion(i,j,this)>0 && src->pixel(i,j))
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
    else if (newPortion>0 && oldPortion==0)
    {
        if (leftX==-1)
        {
            leftX=x;
            rightX=x;
            upperY=y;
            lowerY=y;
        }
        else
        {
            if (x<leftX)
                leftX=x;
            else if (x>rightX)
                rightX=x;
            
            if (y<upperY)
                upperY=y;
            else if (y>lowerY)
                lowerY=y;
        }
        
    }
}
BImage BPartition::makeImage()
{
    BImage ret(width(),height());
    for (int x=0; x<width(); x++)
    {
        for (int y=0; y<height(); y++)
        {
            if (pixel(x,y))
                ret.setPixel(x,y,true);
        }
    }
    return ret;
}

void BPartition::joinInto(BPartition* other)
{
    for (int x=leftX; x<=rightX; x++)
    {
        for (int y=upperY; y<= lowerY; y++)
        {
            src->setPixelCombineOwner(x,y,other,this);
            
        }
    }
}
