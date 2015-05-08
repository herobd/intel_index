#include "codedimage.h"
#include <iostream>

CodedImage::CodedImage(FeaturizedImage &img, Codebook &codebook)
{
    fWidth = img.width();
    fHeight = img.height();
    imgRef=img.ref();
    data.resize(fWidth*fHeight);
    
    for (int j =0; j < img.height(); j++)
    {
        for (int i =0; i < img.width(); i++)
        {
            vector<double> fv = img.get(i,j);
            if(Codebook::twentythree && i==44 & j==2)
            {
                cout << "fv-dif: ";
                for (int iii=0; iii<fv.size(); iii++)
                    cout<<fv[iii]<<",";
                cout<<endl;
            }
            else if(Codebook::twentythree && i==44 & j==3)
            {
                cout << "fv-nex: ";
                for (int iii=0; iii<fv.size(); iii++)
                    cout<<fv[iii]<<",";
                cout<<endl;
            }
            
            data[i+j*fWidth] = codebook.quantize(fv);
        }
    }
    performOverIndexes=img.performOverIndexes;
}

CodedImage::CodedImage(int width, int height)
{
    fWidth = width;
    fHeight = height;
    data.resize(fWidth*fHeight);
}


int CodedImage::pixelWidth()
{
    return imgRef.size[1];
}

int CodedImage::pixelHeight()
{
    return imgRef.size[0];
}
int CodedImage::width()
{
    return fWidth;
}
int CodedImage::height()
{
    return fHeight;
}

cv::Mat CodedImage::ref()
{
    return imgRef;
}

int CodedImage::featureLength()
{
    return 1;
}

int CodedImage::get(int i, int j)
{
    return data[i+j*fWidth];
}

void CodedImage::setImg(cv::Mat &img)
{
    imgRef=img;
}

void CodedImage::set(int i, int j, int v)
{
    data[i+j*fWidth]=v;
}
