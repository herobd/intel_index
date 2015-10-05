#include <iostream>
#include "dimage.h"
#include "dwordfeatures.h"
#include "ddynamicprogramming.h"
#include "dthresholder.h"

using namespace std;

void trimImg(DImage &img, int noiseRemovalSize=2);
void findMatchingPatches(string similar, DImage img1, string word1, DImage img2, string word2, DImage img3, string word3);
void findMatchingPatches_grow(string similar, DImage img1, string word1, DImage img2, string word2, DImage img3, string word3);
void findMatching_exemplar_grow(string similar, DImage img1, string word1, DImage examplar);

int main(int argc, char** argv)
{
    
    string similar = argv[1];
    DImage exemplar(argv[2]);
    
    DImage img1(argv[3]);
    string word1 = argv[4];
    
    findMatching_exemplar_grow(similar,img1,word1, exemplar);
    
//    string similar = argv[1];
    
//    DImage img1(argv[2]);
//    string word1 = argv[3];
    
//    DImage img2(argv[4]);
//    string word2 = argv[5];
    
//    DImage img3(argv[6]);
//    string word3 = argv[7];
    
//    findMatchingPatches_grow(similar,img1,word1,img2,word2, img3, word3);
    
    
//    DFeatureVector fv1 = DWordFeatures::extractWordFeatures(img1);
//    DFeatureVector fv2 = DWordFeatures::extractWordFeatures(img2);
//    DFeatureVector fv3 = DWordFeatures::extractWordFeatures(img3);
    
//    int path12len;
//    int path12[(1+fv1.vectLen)*(1+fv2.vectLen)];
//    double cost12 = DDynamicProgramming::findDPAlignment(fv2,fv1,100,0,0,&path12len,path12);
    
//    int path13len;
//    int path13[(1+fv1.vectLen)*(1+fv3.vectLen)];
//    double cost13 = DDynamicProgramming::findDPAlignment(fv3,fv1,100,0,0,&path13len,path13);
    
//    DImage warped12 = DDynamicProgramming::piecewiseLinearWarpDImage(img2,img1.width(),path12len,path12,false);
//    DImage warped13 = DDynamicProgramming::piecewiseLinearWarpDImage(img3,img1.width(),path13len,path13,false);
    
//    warped12.save("warped12.png",DImage::DFileFormat_png);
//    warped13.save("warped13.png",DImage::DFileFormat_png);
    
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
    
    //first we perform a morphological open to get rid of noise which might interfere with trimming
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
                    if (ri+x>=0 && ri+x<open_tmp.width() &&
                            rj+y>=0 && rj+y<open_tmp.height())
                    {
                        if (open_tmp.getPixelGray(x+ri,y+rj)==255)//if off
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
                    if (ri+p.x>=0 && ri+p.x<open_tmp.width() &&
                            rj+p.y>=0 && rj+p.y<open_tmp.height())
                    {
                        threshed.setPixel(p.x,p.y,0);//on
                    }
                }
            }
        }
    }
    
    //Trim the borders of whitespace
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

//This returns a subimage centered around the desired n-gram
//testPortionScale refers to how many times larger the portion should be to the expected size of the n-gram
DImage getTestPortion(const string& similar, const DImage& img, const string& word, double testPortionScale)
{
    double startLoc = word.find(similar) / (0.5 + word.length());
    double portion = similar.length() / (0.5 + word.length());
    
    int testPortionStart=max(0.0,(startLoc-(portion*(testPortionScale-1)/2))*img.width());
    
    int testPortionEnd=min((double)img.width(), ((startLoc-(portion*(testPortionScale-1)/2)) + (testPortionScale*portion))*img.width());
    
    int testPortionWidth=testPortionEnd-testPortionStart;
    
    return img.copy(testPortionStart,0,testPortionWidth,img.height());
}

DImage getTestPortion(const string& similar, const DImage& img, const string& word, double testPortionScale, double portionOffset)
{
    double startLoc = word.find(similar) / (0.5 + word.length());
    double portion = similar.length() / (0.5 + word.length());
    
    int testPortionStart=max(0.0,(startLoc-(portion*(testPortionScale-1)/2) + portion*portionOffset)*img.width());
    
    int testPortionEnd=min((double)img.width(), ((startLoc-(portion*(testPortionScale-1)/2)) + (testPortionScale*portion) + portion*portionOffset)*img.width());
    
    int testPortionWidth=testPortionEnd-testPortionStart;
    
    return img.copy(testPortionStart,0,testPortionWidth,img.height());
}

void makeWindows(vector<DImage>& windows, int& portionWindowSize, int& portionWindowStide, const DImage& testPortion, double portionWindowScale, double portionWindowStrideScale)
{
    portionWindowSize = testPortion.width()*portionWindowScale;
    portionWindowStide = portionWindowSize*portionWindowStrideScale;
    
    ;
    for (int i=0; i< (testPortion.width()-portionWindowSize)/portionWindowStide; i++)
    {
        windows.push_back(testPortion.copy(i*portionWindowStide,
                                       0,
                                       portionWindowSize,
                                       testPortion.height()));
    }
}

void findMatchingPatches(string similar, DImage img1, string word1, DImage img2, string word2, DImage img3, string word3)
{
    
    trimImg(img1);
    trimImg(img2);
    trimImg(img3);
    
    DImage testPortion1 = getTestPortion(similar,img1,word1,3);
    DImage testPortion2 = getTestPortion(similar,img2,word2,3);
    DImage testPortion3 = getTestPortion(similar,img3,word3,3);
    
    
    double portionWindowScale=.6;
    double portionWindowStrideScale=.1;
    
    int portionWindowSize1, portionWindowSize2, portionWindowSize3;
    int portionWindowStide1, portionWindowStide2, portionWindowStide3;
    
    vector<DImage> windows1;
    makeWindows(windows1,portionWindowSize1,portionWindowStide1,testPortion1,portionWindowScale,portionWindowStrideScale);
    
    vector<DImage> windows2;
    makeWindows(windows2,portionWindowSize2,portionWindowStide2,testPortion2,portionWindowScale,portionWindowStrideScale);
    
    vector<DImage> windows3;
    makeWindows(windows3,portionWindowSize3,portionWindowStide3,testPortion3,portionWindowScale,portionWindowStrideScale);
    
    tuple<int,int, int> bestMatch;
    double bestCost=99999;
    
    
    for (int i1=0; i1< (testPortion1.width()-portionWindowSize1)/portionWindowStide1; i1++)
    {
        for (int i2=0; i2< (testPortion2.width()-portionWindowSize2)/portionWindowStide2; i2++)
        {
            for (int i3=0; i3< (testPortion3.width()-portionWindowSize3)/portionWindowStide3; i3++)
            {
                DFeatureVector fv1 = DWordFeatures::extractWordFeatures(windows1[i1]);
                DFeatureVector fv2 = DWordFeatures::extractWordFeatures(windows2[i2]);
                DFeatureVector fv3 = DWordFeatures::extractWordFeatures(windows3[i3]);
                
                double cost = DDynamicProgramming::findDPAlignment(fv2,fv1,100,100,0);
                cost += DDynamicProgramming::findDPAlignment(fv3,fv1,100,100,0);
                cost += DDynamicProgramming::findDPAlignment(fv3,fv3,100,100,0);
                if (cost < bestCost)
                {
                    bestCost = cost;
                    bestMatch = make_tuple(i1,i2,i3);
                }
            }
        }
    }
    
    DFeatureVector fv1 = DWordFeatures::extractWordFeatures(windows1[get<0>(bestMatch)]);
    DFeatureVector fv2 = DWordFeatures::extractWordFeatures(windows2[get<1>(bestMatch)]);
    
    int path12len;
    int path12[(1+fv1.vectLen)*(1+fv2.vectLen)];
    double cost12 = DDynamicProgramming::findDPAlignment(fv2,fv1,100,100,0,&path12len,path12);
    
    
    DImage warped12 = DDynamicProgramming::piecewiseLinearWarpDImage(windows2[get<1>(bestMatch)],windows1[get<0>(bestMatch)].width(),path12len,path12,false);
    
    img1.save("img1Trimmed.png",DImage::DFileFormat_png);
    img2.save("img2Trimmed.png",DImage::DFileFormat_png);
    testPortion1.save("testPortion1.png",DImage::DFileFormat_png);
    testPortion2.save("testPortion2.png",DImage::DFileFormat_png);
    windows1[get<0>(bestMatch)].save("window1.png",DImage::DFileFormat_png);
    windows2[get<1>(bestMatch)].save("window2.png",DImage::DFileFormat_png);
    windows3[get<2>(bestMatch)].save("window3.png",DImage::DFileFormat_png);
    warped12.save("warped12.png",DImage::DFileFormat_png);
    
}


//void findMatchingPatches_grow(string similar, DImage img1, string word1, DImage img2, string word2, DImage img3, string word3)
//{
    
//    trimImg(img1);
//    trimImg(img2);
//    trimImg(img3);
    
//    double scan = 0;
//    for (;;)
//    {
    
//        DImage testPortion1 = getTestPortion(similar,img1,word1,1);
//        DImage testPortion2 = getTestPortion(similar,img2,word2,1);
//        DImage testPortion3 = getTestPortion(similar,img3,word3,1);
        
        
        
//        DFeatureVector fv1 = DWordFeatures::extractWordFeatures(testPortion1);
//        DFeatureVector fv2 = DWordFeatures::extractWordFeatures(testPortion2);
        
//        int path12len;
//        int path12[(1+fv1.vectLen)*(1+fv2.vectLen)];
//        double cost12 = DDynamicProgramming::findDPAlignment(fv2,fv1,100,100,0,&path12len,path12);
        
//    }
    
    
//    DImage warped12 = DDynamicProgramming::piecewiseLinearWarpDImage(testPortion2,testPortion1.width(),path12len,path12,false);
    
//    img1.save("img1Trimmed.png",DImage::DFileFormat_png);
//    img2.save("img2Trimmed.png",DImage::DFileFormat_png);
//    testPortion1.save("testPortion1.png",DImage::DFileFormat_png);
//    testPortion2.save("testPortion2.png",DImage::DFileFormat_png);
//    warped12.save("warped12.png",DImage::DFileFormat_png);
//}

void findMatching_exemplar_grow(string similar, DImage img1, string word1, DImage examplar)
{
    
    trimImg(img1);
    trimImg(examplar);
    
    double bestScore=9999;
    double bestScan;
    
    for (double scan = 0; scan<1.5; scan +=0.1)
    {
    
        DImage testPortion1 = getTestPortion(similar,img1,word1,1,scan);
        testPortion1.save(("tmp/testPortion_"+to_string(scan)+".png").c_str(),DImage::DFileFormat_png);
        
        
        DFeatureVector fv1 = DWordFeatures::extractWordFeatures(testPortion1);
        DFeatureVector fvE = DWordFeatures::extractWordFeatures(examplar);
        
        int path12len;
        int path12[(1+fv1.vectLen)*(1+fvE.vectLen)];
        double cost12 = DDynamicProgramming::findDPAlignment(fv1,fvE,100,1000,1000,&path12len,path12);
        if (cost12<bestScore)
        {
            bestScore=cost12;
            bestScan=scan;
        }
        
        if (scan != 0.0)
        {
            DImage testPortion1 = getTestPortion(similar,img1,word1,1,-1*scan);
            testPortion1.save(("tmp/testPortion_-"+to_string(scan)+".png").c_str(),DImage::DFileFormat_png);
            
            
            DFeatureVector fv1 = DWordFeatures::extractWordFeatures(testPortion1);
            DFeatureVector fvE = DWordFeatures::extractWordFeatures(examplar);
            
            int path12len;
            int path12[(1+fv1.vectLen)*(1+fvE.vectLen)];
            double cost12 = DDynamicProgramming::findDPAlignment(fv1,fvE,100,1000,1000,&path12len,path12);
            if (cost12<bestScore)
            {
                bestScore=cost12;
                bestScan=scan;
            }
        }
        
    }
    
    DImage testPortion1 = getTestPortion(similar,img1,word1,1,bestScan);
    DFeatureVector fv1 = DWordFeatures::extractWordFeatures(testPortion1);
    DFeatureVector fvE = DWordFeatures::extractWordFeatures(examplar);
    int path12len;
    int path12[(1+fv1.vectLen)*(1+fvE.vectLen)];
    double cost12 = DDynamicProgramming::findDPAlignment(fv1,fvE,100,100,0,&path12len,path12);
    DImage warped12 = DDynamicProgramming::piecewiseLinearWarpDImage(testPortion1,examplar.width(),path12len,path12,false);
    
    testPortion1.save("testPortion1.png",DImage::DFileFormat_png);
    warped12.save("warped12.png",DImage::DFileFormat_png);
}
