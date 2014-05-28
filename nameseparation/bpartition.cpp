#include "bpartition.h"

BPartition::BPartition(BImage* ofImage)
{
    src=ofImage;
    leftX=-1;
    rightX=-1;
    upperY=-1;
    lowerY=-1;
}

BImage* BPartition::getSrc()
{
    return src;
}

int BPartition::width() const
{
    return rightX-leftX;
}

int BPartition::height() const
{
    return lowerY-upperY;
}

bool BPartition::pixel(const QPoint &p) const
{
    return pixel(p.x(),p.y());
}
bool BPartition::pixel(int x, int y) const
{
    assert(x>=0 && x<width() && y>=0 && y<height());
    int realX = x-leftX;
    int realY = y-upperY;
    if (src->pixelOwnerPortion(x,y,this) > 0)
    {
        return src->pixel(x,y);
    }
    else
        return false;
}

void BPartition::changePortion(const QPoint &, float newPortion, float oldPortion)
{
    changePortion(p.x(),p.y(),newPoition,oldPortition);
}

void BPartition::changePortion(int x, int y, float newPortion, float oldPortion)
{
    if (newPortion==0 && oldPortion>0)
    {
        if (x==leftX)
        {
            bool found = false;
            for (int i=leftX; i<=rightX; i++)
            {
                for (int j=upperY; j<=lowerY; j++)
                {
                    if (src->pixelOwnerPortion(x,y,this)>0 && src->pixel(x,y))
                    {
                        leftX=i;
                        i=rightX+1;
                        j=lowerY+1;
                        found=true;
                    }
                }
            }
            if (!found)
            {
                leftX=-1;
                rightX=-1;
                upperY=-1;
                lowerY=-1;
            }
        }
        else if (x==rightX)
        {
            bool found = false;
            for (int i=rightX; i>=leftX; i-)
            {
                for (int j=upperY; j<=lowerY; j++)
                {
                    if (src->pixelOwnerPortion(x,y,this)>0 && src->pixel(x,y))
                    {
                        rightX=i;
                        i=-1;
                        j=lowerY+1;
                        found=true;
                    }
                }
            }
            if (!found)
            {
                leftX=-1;
                rightX=-1;
                upperY=-1;
                lowerY=-1;
            }
        }
        
        if (y==upperY)
        {
            bool found = false;
            for (int j=upperY; j<=lowerY; j++)
            {
                for (int i=leftX; i<=rightX; i++)
                {
                    if (src->pixelOwnerPortion(x,y,this)>0 && src->pixel(x,y))
                    {
                        upperY=i;
                        i=rightX+1;
                        j=lowerY+1;
                        found=true;
                    }
                }
            }
            if (!found)
            {
                leftX=-1;
                rightX=-1;
                upperY=-1;
                lowerY=-1;
            }
        }
        else if (y==lowerY)
        {
            bool found = false;
            for (int j=lowerY; j>=upperY; j--)
            {
                for (int i=leftX; i<=rightX; i++)
                {
                    if (src->pixelOwnerPortion(x,y,this)>0 && src->pixel(x,y))
                    {
                        lowerY=i;
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
            rightX=-1;
            upperY=-1;
            lowerY=-1;
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
