#include "dimension.h"
#include <stdio.h>

Dimension::Dimension()
{
    assert(false);
}

Dimension::Dimension(int width, int height)
{
    values.resize(width);
    for (int i=0; i<width; i++)
    {
        values[i].resize(height);
        for (int j=0; j<height; j++)
            values[i][j]=-1;
    }
    
    values2.resize(width);
    for (int i=0; i<width; i++)
    {
        values2[i].resize(height);
        for (int j=0; j<height; j++)
            values2[i][j]=-1;
    }
    
    numOfBins = 0;
    maxValue=INT_MIN;
    minValue=INT_MAX;
}

int Dimension::binForPixel(int x, int y) const
{
    if (numOfBins==0)
        return -1;
    
//    if (values2[x][y]<0)
        return (int)((values[x][y]-minValue)*((numOfBins-1)/(maxValue-minValue)));
//    else
//        return (int)((values[x][y]-minValue)*((numOfBins-1)/(maxValue-minValue))) + 
//                numOfBins*(int)((values2[x][y]-minValue)*((numOfBins-1)/(maxValue-minValue)));
}
int Dimension::secondBinForPixel(int x, int y) const
{
    if (numOfBins==0)
        return -1;
    
    return (int)((values2[x][y]-minValue)*((numOfBins-1)/(maxValue-minValue)));
}
int Dimension::getNumBins() const
{
    return numOfBins;
}

void Dimension::setValueForPixel(int x, int y, double value)
{
    values[x][y]=value;
//    if (value < minValue)
//        minValue=value;
    
//    if (value>maxValue)
//        maxValue=value;
}

void Dimension::setValueForPixel(const QPoint &p, double value)
{
    values[p.x()][p.y()]=value;
//    if (value < minValue)
//        minValue=value;
    
//    if (value>maxValue)
//        maxValue=value;
}

void Dimension::setSecondValueForPixel(int x, int y, double value)
{
    values2[x][y]=value;
//    if (value < minValue)
//        minValue=value;
    
//    if (value>maxValue)
//        maxValue=value;
}

void Dimension::setSecondValueForPixel(const QPoint &p, double value)
{
    values2[p.x()][p.y()]=value;
//    if (value < minValue)
//        minValue=value;
    
//    if (value>maxValue)
//        maxValue=value;
}

void Dimension::setNumOfBins(int num)
{
    numOfBins = num;
}

void Dimension::setMinMax(int min, int max)
{
    minValue=min;
    maxValue=max;
    if (minValue < 0)
        printf("WARNING Dimension.minValue was set <0 (%d).\n",min);
}

NDimensions::NDimensions()
{
    
}

QVector<int> NDimensions::getBinsForPixel(int x, int y) const
{
    QVector<int> ret;
    foreach (Dimension dim, dimensions)
    {
        ret.append(dim.binForPixel(x,y));
    }
    return ret;
}
QVector<int> NDimensions::getSecondBinsForPixel(int x, int y) const
{
    QVector<int> ret;
    foreach (Dimension dim, dimensions)
    {
        ret.append(dim.secondBinForPixel(x,y));
    }
    return ret;
}
QVector<int> NDimensions::getBinNums() const
{
    QVector<int> ret;
    foreach (Dimension dim, dimensions)
    {
        ret.append(dim.getNumBins());
    }
    return ret;
}
int NDimensions::numOfDim() const
{
    return dimensions.size();
}
void NDimensions::addDimension(const Dimension dim)
{
    dimensions.append(dim);
}

const Dimension* NDimensions::getDimension(int index) const
{
    return &dimensions[index];
}

Indexer::Indexer(int width, int height, const NDimensions *dimensions)
{
    this->width=width;
    this->height=height;
    this->dimensions=dimensions;
}

int Indexer::getIndex(int x, int y) const
{
    assert(dimensions->numOfDim()==1);
    int slopeBin = dimensions->getBinsForPixel(x,y)[0];
//    printf("prob anchor at (%d,%d,%d)\n",x,y,slopeBin);
    return slopeBin*width*height + y*width + x;
}
int Indexer::getIndex(int x, int y, int slopeBin) const
{
    assert(dimensions->numOfDim()==1);
    return slopeBin*width*height + y*width + x;
}
