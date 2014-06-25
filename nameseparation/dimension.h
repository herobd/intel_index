#ifndef DIMENSION_H
#define DIMENSION_H

#include <QVector>
#include <QPoint>
#include <assert.h>

class Dimension
{
public:
    Dimension();
    Dimension(int width, int height);
    int binForPixel(int x, int y) const;
    int secondBinForPixel(int x, int y) const;
    QVector<int> binsForPixel(int x, int y) const;
    int getNumBins() const;
    
    void setValueForPixel(int x, int y, double value);
    void setValueForPixel(const QPoint &p, double value);
    void setSecondValueForPixel(int x, int y, double value);
    void setSecondValueForPixel(const QPoint &p, double value);
    void setValuesForPixel(int x, int y, QVector<double> value);
    void setValuesForPixel(const QPoint &p, QVector<double> value);
    void setNumOfBins(int num);
    void setMinMax(int min, int max);
private:
    
    QVector<QVector<QVector<double> > > values;
//    QVector<QVector<QVector<double> > > values2;
    double minValue;
    double maxValue;
    int numOfBins;
    
};

class NDimensions
{
public:
    NDimensions();
    QVector<int> getBinsForPixel(int x, int y) const;
    QVector<int> getSecondBinsForPixel(int x, int y) const;
    QVector<QVector<int> > getBinsForDimensionsOfPixel(int x, int y) const;
    QVector<int> getBinNums() const;
    int numOfDim() const;
    
    void addDimension(const Dimension dim);
    const Dimension* getDimension(int index) const;
private:
    QVector<Dimension> dimensions;
};
//describes formula for creating n-dimensional mesh http://download.springer.com/static/pdf/41/art%253A10.1007%252Fs40436-014-0065-2.pdf?auth66=1402248354_79fc4617422cd93938d0634575aa5034&ext=.pdf

class Indexer
{
public:
    Indexer(int width, int height, const NDimensions *dimensions);
    int getIndex(int x, int y) const;
    int getIndex(int x, int y, int slopeBin) const;
    
private:
    const NDimensions *dimensions;
    int width;
    int height;
};
#endif // DIMENSION_H
