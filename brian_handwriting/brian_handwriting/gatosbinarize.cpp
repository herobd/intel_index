#include "gatosbinarize.h"

GatosBinarize::GatosBinarize()
{
}

Mat GatosBinarize::binarize(const Mat &src, int dx, int dy, double dpi)
{
    Mat I = wienerFIlter(src);
    Mat S = sauvolaThresh(I,dpi,.2);
    Mat B = approxBackground(I,S,dx,dy);
    return finalThresh(I,S,B,.6,.5,.8);
}

Mat GatosBinarize::wienerFIlter(const Mat &src)
{
    Mat ret(src.size(),CV_8U);
    Mat mu(src.size(),CV_32F);
    Mat varSqr(src.size(),CV_32F);
    for (int x=0; x<src.cols; x++)
        for (int y=0; y<src.rows; y++)
        {
            double mean=0;
            for (int xoff=-1; xoff<=1; xoff++)
                for (int yoff=-1; yoff<=1; yoff++)
                {
                    mean+=src.at<unsigned char>(y+yoff,x+xoff);
                }
            mean /=9;
            mu.at<float>(y,x)=mean;
            double varSqrV=0;
            for (int xoff=-1; xoff<=1; xoff++)
                for (int yoff=-1; yoff<=1; yoff++)
                {
                    varSqrV+=pow(src.at<unsigned char>(y+yoff,x+xoff)-mean,2);
                }
            varSqrV /=9;
            varSqr.at<float>(y,x)=varSqrV;
        }
    
    
    
    for (int x=0; x<src.cols; x++)
        for (int y=0; y<src.rows; y++)
        {
            double avgSqr=0;
            for (int xoff=-1; xoff<=1; xoff++)
                for (int yoff=-1; yoff<=1; yoff++)
                {
                    avgSqr+=varSqr.at<unsigned char>(y+yoff,x+xoff);
                }
            avgSqr /=9;
            ret.at<unsigned char>(y,x) = (unsigned char) (mu.at<float>(y,x) + (varSqr.at<float>(y,x)-avgSqr)*((int)src.at<unsigned char>(y,x)-mu.at<float>(y,x))/(varSqr.at<float>(y,x)));
        }
}

Mat GatosBinarize::sauvolaThresh(const Mat &src, double dpi, double k)
{
    Mat ret(src.size(),CV_8U);
    
    
    
    int windowSize;
    if (dpi>=75 && dpi<=300)
    {
        windowSize=10+10*((dpi-75)/(300-75));
    }
    else if (dpi<75)
        windowSize=10;
    else
        windowSize=20;
    
    Mat averages(src.rows/windowSize,src.cols/windowSize,CV_32F);
    Mat transDiff(src.rows/windowSize,src.cols/windowSize,CV_32F);
    float tdMax=0;
    float tdMin=99999;
    
    for (int xc=0; xc<ret.cols; xc+=windowSize)
        for (int yc=0; yc<ret.rows; yc+=windowSize)
        {
            float avg=0;
            float td=0;
            for (int xoff=0; xoff<windowSize; xoff++)
                for (int yoff=0; yoff<windowSize; yoff++)
                {
                    int y = yc+yoff;
                    int x = xc+xoff;
                    avg += src.at<unsigned char>(y,x);
                    td += abs(2*src.at<unsigned char>(y,x)-(src.at<unsigned char>(y,x-1)+src.at<unsigned char>(y-1,x)));
                }
            avg /= windowSize*windowSize;
            averages.at<float>(yc/windowSize,xc/windowSize)=avg;
            td /= 255*255*windowSize*windowSize;
            transDiff.at<float>(yc/windowSize,xc/windowSize)=td;
            
            if (td>tdMax) tdMax=td;
            if (td<tdMin) tdMin=td;
        }
    
    for (int xb=0; xb<transDiff.cols; xb++)
        for (int yb=0; yb<transDiff.rows; yb++)
        {
            
        }
}

Mat GatosBinarize::approxBackground(const Mat &I, const Mat &S, int dx, int dy)
{
    Mat ret(I.size(),CV_8U);
    for (int x=0; x<ret.cols; x++)
        for (int y=0; y<ret.rows; y++)
        {
            if (S.at<unsigned char>(y,x)==0)
                ret.at<unsigned char>(y,x) = I.at<unsigned char>(y,x);
            else
            {
                int topSum=0;
                int bottomSum=0;
                for (int ix=x-dx; ix<=x+dx; ix++)
                    for (int iy=y-dy; iy<=y+dy; iy++)
                    {
                        topSum += I.at<unsigned char>(iy,ix)*(1-S.at<unsigned char>(iy,ix));
                        bottomSum += (1-S.at<unsigned char>(iy,ix));
                    }
                ret.at<unsigned char>(y,x) = (unsigned char) (topSum/bottomSum);
            }
        }
    return ret;
}

Mat GatosBinarize::finalThresh(const Mat &I, const Mat &S, const Mat &B, double q, double p1, double p2)
{
    Mat ret(I.size(),CV_8U);
    
    int bTop=0;
    int bBot=0;
    for (int x=0; x<ret.cols; x++)
        for (int y=0; y<ret.rows; y++)
        {
            bTop += B.at<unsigned char>(y,x)*(1-S.at<unsigned char>(y,x));
            bBot += (1-S.at<unsigned char>(y,x));
        }
    double b = (0.0+bTop)/bBot;
    //TODO
}
