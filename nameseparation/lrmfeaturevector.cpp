#include "lrmfeaturevector.h"

#include "fft/fft8g.c"

#define CENTRAL_THRESH .1

LRMFeatureVector::LRMFeatureVector(const BImage &img)
{
    //trim whitespace?
    
    v[WIDTH] = img.width();
    v[HEIGHT] = img.height();
    v[ASPECT_RATIO] = v[WIDTH]/v[HEIGHT];
    v[AREA] = v[WIDTH]*v[HEIGHT];
    v[NUM_ASC] = 0;
    v[NUM_DESC] = 0;
    
    QVector<int> verticleProfile(img.height());
    QVector<int> horzProfile(img.width());
    QVector<int> upperProfile(img.width());
    QVector<int> lowerProfile(img.width());
    int total=0;
    for (int x=0; x<img.width(); x++)
    {
        upperProfile[x]=img.height()-1;
        for (int y=0; y<img.height(); y++)
        {
            if (img.pixel(x,y))
            {
                total++;
                verticleProfile[y]++;
                horzProfile[x]++;
                lowerProfile[x]=y;
                if (upperProfile[x]==img.height()-1)
                    upperProfile[x]=y;
            }
        }
    }
    
    int upperCentralBound;
    int lowerCentralBound;
    for (int y=0; y>img.height(); y++)
    {
        if (verticleProfile[y]*1.0/total > CENTRAL_THRESH)
        {
            upperCentralBound=y;
            break;
        }
    }
    for (int y=img.height()-1; y>=0; y--)
    {
        if (verticleProfile[y]*1.0/total > CENTRAL_THRESH)
        {
            lowerCentralBound=y;
            break;
        }
    }
    
    for (int x=0; x>img.width(); x++)
    {
        if (horzProfile[x] > lowerCentralBound && (x==0 || horzProfile[x-1] <= lowerCentralBound))
        {
            v[NUM_DESC]++;
        }
        
        if (horzProfile[x] < upperCentralBound && (x==0 || horzProfile[x-1] >= lowerCentralBound))
        {
            v[NUM_ASC]++;
        }
    }
    
    //debug, display profiles
//    for (int y=img.height(); y>0; y--)
//    {
//        for (int x=0; x<img.width(); x++)
//        {
//            if (horzProfile[x]>=y)
//                printf("#");
//            else
//                printf(" ");
//        }
//        printf("\n");
//    }
    
//    for (int y=0; y<img.height(); y++)
//    {
//        for (int x=0; x<img.width(); x++)
//        {
//            if (upperProfile[x]==y && lowerProfile[x]==y)
//                printf("B");
//            else if (upperProfile[x]==y)
//                printf("U");
//            else if (lowerProfile[x]==y)
//                printf("L");
//            else
//                printf(" ");
//        }
//        printf("\n");
//    }
    
    
    
    //TODO: DFT on profiles
    int m = ceil(log(img.width())/log(2));
    int n = pow(2,m);
    double *profileData = new double[2*n];
    int *ip = new int[2+(int)ceil(pow(n,.5))];
    double *w = new double[n/2];
    
    for (int i=0; i<img.width(); i++)
    {
        profileData[i*2] = (double) horzProfile[i];
        profileData[i*2+1]= 0;
    }
    for (int i=img.width(); i<n; i++)
    {
        profileData[i*2]=0;
        profileData[i*2+1]=0;
    }
    
    ip[0]=0;
    cdft(2*n, 1, profileData, ip, w);
    
    for (int i=0; i<4; i++)
    {
        v[PROJ_PROFILE_REAL(i)]=profileData[2*i];
    }
    for (int i=1; i<4; i++)
    {
        v[PROJ_PROFILE_IMAG(i-1)]=profileData[2*i + 1];
    }
    
    //upper
    for (int i=0; i<img.width(); i++)
    {
        profileData[i*2] = (double) upperProfile[i];
        profileData[i*2+1]= 0;
    }
    for (int i=img.width(); i<n; i++)
    {
        profileData[i*2]=0;
        profileData[i*2+1]=0;
    }
    
    ip[0]=0;
    cdft(2*n, 1, profileData, ip, w);
    
    for (int i=0; i<4; i++)
    {
        v[UPPER_PROFILE_REAL(i)]=profileData[2*i];
    }
    for (int i=1; i<4; i++)
    {
        v[UPPER_PROFILE_IMAG(i-1)]=profileData[2*i + 1];
    }
    
    //lower
    for (int i=0; i<img.width(); i++)
    {
        profileData[i*2] = (double) lowerProfile[i];
        profileData[i*2+1]= 0;
    }
    for (int i=img.width(); i<n; i++)
    {
        profileData[i*2]=0;
        profileData[i*2+1]=0;
    }
    
    ip[0]=0;
    cdft(2*n, 1, profileData, ip, w);
    
    for (int i=0; i<4; i++)
    {
        v[LOWER_PROFILE_REAL(i)]=profileData[2*i];
    }
    for (int i=1; i<4; i++)
    {
        v[LOWER_PROFILE_IMAG(i-1)]=profileData[2*i + 1];
    }
    delete[] profileData;
    delete[] ip;
    delete[] w;
    
}
