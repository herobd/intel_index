#include <iostream>
#include "dimage.h"
#include "dwordfeatures.h"
#include "ddynamicprogramming.h"
#include "dthresholder.h"

using namespace std;



int main(int argc, char** argv)
{
    string similar = argv[1];
    
    DImage img1(argv[2]);
    string word1 = argv[3];
    
    DImage img2(argv[4]);
    string word2 = argv[5];
    
    DImage img3(argv[6]);
    string word3 = argv[7];
    
    DFeatureVector fv1 = DWordFeatures::extractWordFeatures(img1);
    DFeatureVector fv2 = DWordFeatures::extractWordFeatures(img2);
    DFeatureVector fv3 = DWordFeatures::extractWordFeatures(img3);
    
    int path12len;
    int path12[(1+fv1.vectLen)*(1+fv2.vectLen)];
    double cost12 = DDynamicProgramming::findDPAlignment(fv2,fv1,100,0,0,&path12len,path12);
    
    int path13len;
    int path13[(1+fv1.vectLen)*(1+fv3.vectLen)];
    double cost13 = DDynamicProgramming::findDPAlignment(fv3,fv1,100,0,0,&path13len,path13);
    
    DImage warped12 = DDynamicProgramming::piecewiseLinearWarpDImage(img2,img1.width(),path12len,path12,false);
    DImage warped13 = DDynamicProgramming::piecewiseLinearWarpDImage(img3,img1.width(),path13len,path13,false);
    
    img1.save("img1.png",DImage::DFileFormat_png);
    img2.save("img2.png",DImage::DFileFormat_png);
    img3.save("img3.png",DImage::DFileFormat_png);
    warped12.save("warped12.png",DImage::DFileFormat_png);
    warped13.save("warped13.png",DImage::DFileFormat_png);
    
    return 0;
}

struct point
{
    int x, y;
    point(int x, int y)
    {
        this->x=x;
        this->y=y;
    }
};

void trimImg(DImage &img, int noiseRemovalSize)
{
    DImage threshed;
    DThresholder::threshImage_(threshed, img, DThresholder::getOtsuThreshVal(img));
    
    //morphological open
    vector<struct point> pointsToDilate;
    DImage open_tmp(threshed);
    
    //errode
    for (int x=0; x<open_tmp.width(); x++)
        for (int y=0; y<open_tmp.height(); y++)
    {
        bool allBlack = true;
        for (int ri=-noiseRemovalSize; ri<=noiseRemovalSize && allBlack; ri++)
        {
            for (int rj=-noiseRemovalSize; rj<=noiseRemovalSize; rj++)
            {
                if (ri*ri+rj*rj<(noiseRemovalSize*noiseRemovalSize)) //somewhat circular
                {
                    if (ri+p.x()>=0 && ri+p.x()<open_tmp.width() &&
                            rj+p.y()>=0 && rj+p.y()<open_tmp.height())
                    {
                        if (open_tmp.getPixelGray(p.x()+ri,p.y()+rj)==255)//if off
                        {
                                allBlack=false;
                                break;
                        }
                    }
                }
            }
        }
        if (allBlack)
        {
            threshed.setPixel(x,y,0);//on
            pointsToDilate.push_back(point(x,y));
        }
        else
        {
            threshed.setPixel(x,y,255);//off
        }
        
    }
    
    //dialate
    for (struct point p : pointsToDilate)
    {
        
        for (int ri=-noiseRemovalSize; ri<=noiseRemovalSize; ri++)
        {
            for (int rj=-noiseRemovalSize; rj<=noiseRemovalSize; rj++)
            {
                if (ri*ri+rj*rj<(noiseRemovalSize*noiseRemovalSize))
                {
                    if (ri+p.x()>=0 && ri+p.x()<open_tmp.width() &&
                            rj+p.y()>=0 && rj+p.y()<open_tmp.height())
                    {
                        threshed.setPixel(x,y,0);//on
                    }
                }
            }
        }
    }
    
    int leftX, rightX, topY, botY;
    
    for (int x=0; x<threshed.width(); x++)
        for (int y=0; y<threshed.height(); y++)
        {
            if (threshed.getPixelGray(x,y)==0)//if on
            {
                leftX=x;
                x=threshed.width();
                break;
            }
        }
    for (int x=threshed.width()-1; x>=0; x--)
        for (int y=0; y<threshed.height(); y++)
        {
            if (threshed.getPixelGray(x,y)==0)//if on
            {
                rightX=x;
                x=-1;
                break;
            }
        }
    
    for (int y=0; y<threshed.height(); y++)
        for (int x=0; x<threshed.width(); x++)
        {
            if (threshed.getPixelGray(x,y)==0)//if on
            {
                topY=y;
                y=threshed.height();
                break;
            }
        }
    for (int y=threshed.height()-1; y>=0; y--)
        for (int x=0; x<threshed.width(); x++)
        {
            if (threshed.getPixelGray(x,y)==0)//if on
            {
                botY=y;
                y=-1;
                break;
            }
        }
    
    img = img.copy(leftX,topY,1+rightX-leftX,1+botY-topY);
}

void findMatchingPatches(string similar, DImage img1, string word1, DImage img2, string word2)
{
    double startLoc1 = word1.find(similar) / (0.0 + word1.length());
    double portion1 = similar.length() / (0.0 + word1.length());
    
    double startLoc2 = word2.find(similar) / (0.0 + word2.length());
    double portion2 = similar.length() / (0.0 + word2.length());
    
    trimImg(img1);
    trimImg(imt2);
}
