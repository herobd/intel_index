#include "grapheme.h"

Grapheme::Grapheme(Mat graphemes, int id)
{
    this->id = id;
    this->graphemes=graphemes;
    
    leftX=9999;
    rightX=-1;
    topY=9999;
    bottomY=-1;
    
    for (int x=0; x<graphemes.cols; x++)
        for (int y=0; y<graphemes.rows; y++)
        {
            if (graphemes.at<unsigned char>(y,x)==id)
            {
                if (x<leftX) leftX=x;
                if (x>rightX) rightX=x;
                if (y<topY) topY=y;
                if (y>bottomY) bottomY=y;
            }
        }
}

int Grapheme::maxXBetween(int minY, int maxY)const
{
    
    for (int x=rightX; x<=leftX; x--)
        for (int y=minY; y<=maxY; y++)
        {
            if (graphemes.at<unsigned char>(y,x)==id)
            {
                return x;
            }
        }
    return -1;
}
int Grapheme::minXBetween(int minY, int maxY)const
{
    for (int x=leftX; x<=rightX; x++)
        for (int y=minY; y<=maxY; y++)
        {
            if (graphemes.at<unsigned char>(y,x)==id)
            {
                return x;
            }
        }
    return -1;
}


Point2f Grapheme::centriod()
{
    int xS=0;
    int yS=0;
    int count;
    for (int x=leftX; x<=rightX; x++)
        for (int y=topY; y<=bottomY; y++)
        {
            if (graphemes.at<unsigned char>(y,x)==id)
            {
                xS+=x;
                yS+=y;
                count++;
            }
        }
    return Point2f(xS/(double)count,yS/(double)count);
}
