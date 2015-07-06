#ifndef BPIXELCOLLECTION_H
#define BPIXELCOLLECTION_H

#include <QPoint>
#include <QVector>

class BImage;

class BPixelCollection
{
public:
    virtual ~BPixelCollection(){}
    virtual bool pixel(const QPoint &p) const {return pixel(p.x(),p.y());}
    virtual bool pixel(int x, int y) const =0;
    virtual bool pixelIsMine(const QPoint &p) const {return pixelIsMine(p.x(),p.y());}
    virtual bool pixelIsMine(int x, int y) const =0;
    virtual int width() const =0;
    virtual int height() const =0;
    virtual BImage makeImage() const =0;
    QPoint findClosestPoint(QPoint &start) const
    {
        QVector<QPoint> searchQueue;
        searchQueue.append(start);
//        BImage mark(width(),height());
        bool mark[width()*height()];
        for (int i=0; i<width()*height(); i++)
        {
            mark[i]=true;
        }
        setPixel(mark,start,false);
        while (!searchQueue.empty())
        {
            QPoint cur = searchQueue.front();
            searchQueue.pop_front();
            if (pixel(cur))
                return cur;
            
            QPoint up(cur.x(),cur.y()-1);
            QPoint down(cur.x(),cur.y()+1);
            QPoint left(cur.x()-1,cur.y());
            QPoint right(cur.x()+1,cur.y());
            QPoint lu(cur.x()-1,cur.y()-1);
            QPoint ld(cur.x()-1,cur.y()+1);
            QPoint ru(cur.x()+1,cur.y()-1);
            QPoint rd(cur.x()+1,cur.y()+1);
            if (cur.y()>0 && getPixel(mark,up))
            {
                searchQueue.append(up);
                setPixel(mark,up,false);
            }
            if (cur.y()+1<height() && getPixel(mark,down))
            {
                searchQueue.append(down);
                setPixel(mark,down,false);
            }
            if (cur.x()>0 && getPixel(mark,left))
            {
                searchQueue.append(left);
                setPixel(mark,left,false);
            }
            if (cur.x()+1<width() && getPixel(mark,right))
            {
                searchQueue.append(right);
                setPixel(mark,right,false);
            }
            if (cur.x()>0 && cur.y()>0 &&getPixel(mark,lu))
            {
                searchQueue.append(lu);
                setPixel(mark,lu,false);
            }
            if (cur.x()>0 && cur.y()+1<height() && getPixel(mark,ld))
            {
                searchQueue.append(ld);
                setPixel(mark,ld,false);
            }
            if (cur.x()+1<width() && cur.y()>0 && getPixel(mark,ru))
            {
                searchQueue.append(ru);
                setPixel(mark,ru,false);
            }
            if (cur.x()+1<width() && cur.y()+1<height() && getPixel(mark,rd))
            {
                searchQueue.append(rd);
                setPixel(mark,rd,false);
            }
            
        }
//        printf("findClosestPointOn failed to find point\n");
        QPoint x(-1,-1);
        return x;
    }
    
    QVector<int> horizontalProfile() {
        QVector<int> ret(width());
        ret.fill(0);
        for (int x=0; x<width(); x++)
        {
            int sum=0;
            for (int y=0; y<height(); y++)
            {
                if (pixel(x,y))
                    sum++;
            }
            ret.append(sum);
        }
        return ret;
    }
    
private:
    void setPixel(bool* mark, const QPoint &p, bool set) const
    {
        mark[p.x() + p.y()*width()]=set;
    }
    bool getPixel(const bool* mark, const QPoint &p) const
    {
        return mark[p.x() + p.y()*width()];
    }
};

#include "bimage.h"

#endif // BPIXELCOLLECTION_H
