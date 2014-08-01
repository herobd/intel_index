#ifndef INDEXER3D_H
#define INDEXER3D_H

#include <QVector>


struct point3D
{
    int x;
    int y;
    int z;
    point3D(int x_, int y_, int z_)
    {
        x=x_;
        y=y_;
        z=z_;
    }
    point3D()
    {
        x=0; y=0; z=0;
    }
};

class Indexer3D
{
public:
    Indexer3D(int width, int height)
    {
        this->width=width;
        this->height=height;
    }
//    int getIndex(int x, int y) const;
    int getIndex(int x, int y, int z) const
    {
        return z*width*height + y*width + x;
    }
    
    int getIndex(point3D p) const
    {
        return p.z*width*height + p.y*width + p.x;
    }
    
private:
//    const AngleImage* angleImage;
    int width;
    int height;
};


class IndexerKD
{
public:
    IndexerKD(int k, const int* dim)
    {
        for (int i=0; i<k; i++)
        {
            size.append(dim[i]);
        }
    }
//    int getIndex(int x, int y) const;
    int getIndex(const int* x) const
    {
        int ret=0;
        for (int i=0; i<size.size(); i++)
        {
            int mult = 1;
            for (int j=0; j<i; j++)
                mult *= size[j];
            
            ret += x[i]*mult;
        }
        return ret;
    }
    
private:
//    const AngleImage* angleImage;
    QVector<int> size;
};

#endif // INDEXER3D_H
