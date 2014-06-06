#include "dimension.h"

Dimension::Dimension(int width, int height)
{
    values.resize(width);
    for (int i=0; i<width; i++)
    {
        values[i].resize(height);
    }
}

int Dimension::binForPixel(int x, int y) const
{
    return (int)((values[x][y]-minValue)*(numOfBins/(maxValue-minValue)));
}
int Dimension::getNumBins() const
{
    return numOfBins;
}

void Dimension::setValueForPixel(int x, int y, double value)
{
    values[x][y]=value;
}

void Dimension::setNumOfBins(int num)
{
    numOfBins = num;
}

NDimensions::NDimensions()
{
    
}

QVector<int> NDimensions::getBinsForPixel(int x, int y) const
{
    QVector<int> ret();
    foreach (Dimension dim, dimensions)
    {
        ret.append(dim.binForPixel(x,y));
    }
    return ret;
}
QVector<int> NDimensions::getBinSizes() const
{
    QVector<int> ret();
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

Indexer::Indexer(int width, int height, const NDimensions *dimensions)
{
    this->width=width;
    this->height=height;
    this->dimensions=dimensions;
}

//int Indexer::getIndex(int x, int y) const;
int Indexer::getIndex(int x, int y, int slopeBin) const
{
    assert(dimensions->numOfDim()==1);
    return slopeBin*width*height + y*width + x;
}
