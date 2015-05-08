#include "siftizedimage.h"
#include <assert.h>
SIFTizedImage::SIFTizedImage(Mat &img)
{
    imgRef=img;
    fWidth = 1+ (img.size[1]-SIFT_EFFECTIVE_WINDOW_SIZE)/WINDOW_SIZE;
    fHeight = 1+ (img.size[0]-SIFT_EFFECTIVE_WINDOW_SIZE)/WINDOW_SIZE;
    
    int width = img.size[1];
    int height = img.size[0];
    performOverIndexes = [width, height](function<void(int,int,int,int)> func){
            int fI=0;
            int fJ=0;
            for (int j =SIFT_EFFECTIVE_WINDOW_SIZE/2; j < height-SIFT_EFFECTIVE_WINDOW_SIZE/2; j+=WINDOW_SIZE)
            {
                for (int i =SIFT_EFFECTIVE_WINDOW_SIZE/2; i < width-SIFT_EFFECTIVE_WINDOW_SIZE/2; i+=WINDOW_SIZE)
                {   
                    func(i,j,fI,fJ);
                    fI++;
                }
                fJ++;
            }
        };
    
    vector<KeyPoint> keyPoints;//(fWidth*fHeight);
//    for (int j =SIFT_EFFECTIVE_WINDOW_SIZE/2; j < img.size[0]-SIFT_EFFECTIVE_WINDOW_SIZE/2; j+=WINDOW_SIZE)
//    {
//        for (int i =SIFT_EFFECTIVE_WINDOW_SIZE/2; i < img.size[1]-SIFT_EFFECTIVE_WINDOW_SIZE/2; i+=WINDOW_SIZE)
//        {
//            //does angle need to be set? or is -1 proper?
//            KeyPoint p(i,j,SIFT_WINDOW_SIZE);
//            //keyPoints[(i/5)+(j/5)*fWidth] = (p);
//            //assert((i/5)+(j/5)*fWidth < fWidth*fHeight);
//            keyPoints.push_back(p);
//        }
//    }
    performOverIndexes([&keyPoints](int i, int j, int,int){KeyPoint p(i,j,SIFT_WINDOW_SIZE); keyPoints.push_back(p);});
    
#if USE_CUST_SIFT
    CustomSIFT::extract(img,keyPoints,descriptors);
#else
    SIFT detector;// = new SIFT();
    
    detector(img,Mat(),keyPoints,descriptors,true);
#endif
    
    assert(descriptors.size()>0);
}


int SIFTizedImage::pixelWidth()
{
    return imgRef.size[1];
}

int SIFTizedImage::pixelHeight()
{
    return imgRef.size[0];
}
int SIFTizedImage::width()
{
    return fWidth;
}
int SIFTizedImage::height()
{
    return fHeight;
}

Mat SIFTizedImage::ref()
{
    return imgRef;
}

int SIFTizedImage::featureLength()
{
#if USE_CUST_SIFT
    if (descriptors.size()>0)
        return descriptors[0].size();
    {
        cout << "error asked for feature length when empty"<<endl;
        return 128;
    }
#else
    return descriptors.size[1];
#endif
}

vector<double> SIFTizedImage::get(int i, int j)
{
#if USE_CUST_SIFT
    return descriptors[i+j*fWidth];
#else
    vector<double> ret;
    descriptors.row(i+j*fWidth).copyTo(ret);
    return ret;
#endif
}
