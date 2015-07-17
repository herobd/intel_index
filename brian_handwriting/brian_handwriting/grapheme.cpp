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
    upperBaseline=-1;
}

int Grapheme::minXBetween(int minY, int maxY)
{
    if (upperBaseline!=minY || lowerBaseline!=maxY)
    {
        findMinMaxBetweenBaselines(minY,maxY);
    }
    return minXBetweenBaselines;
    
}

int Grapheme::maxXBetween(int minY, int maxY)
{
    if (upperBaseline!=minY || lowerBaseline!=maxY)
    {
        findMinMaxBetweenBaselines(minY,maxY);
    }
    return maxXBetweenBaselines;
    
}

void Grapheme::findMinMaxBetweenBaselines(int minY, int maxY)
{
    upperBaseline=minY;
    lowerBaseline=maxY;
    for (int x=leftX; x<=rightX; x++)
        for (int y=upperBaseline; y<=lowerBaseline; y++)
        {
            if (graphemes.at<unsigned char>(y,x)==id)
                minXBetweenBaselines=x;
        }
    for (int x=rightX; x<=leftX; x--)
        for (int y=upperBaseline; y<=lowerBaseline; y++)
        {
            if (graphemes.at<unsigned char>(y,x)==id)
                maxXBetweenBaselines=x;
        }
}
