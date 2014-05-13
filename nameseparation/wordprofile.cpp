//Brian Davis

#include "wordprofile.h"
WordProfile::WordProfile()
{

}

WordProfile::WordProfile(const QImage &from, bool bianized, int on)
{
    QImage toProfile = from;
    if (!bianized)
    {
        //TODO something
    }
    profile = (int*) malloc(sizeof(int)*toProfile.width());

    width = toProfile.width();
    for (int i=0; i<width; i++)
        profile[i]=0;

    for (int i=0; i < width; i++)
    {
        for (int j=0; j<toProfile.height(); j++)
        {
            if (on==qGray(toProfile.pixel(i,j)))
                profile[i]++;
        }
    }
}

WordProfile::~WordProfile()
{
   free(profile);
}

QVector<int> WordProfile::getLocalMins()
{
    if (minima.size()>0)
        return minima;

    QVector<int> ret;
    //int lastDescent = 0;
    double lastDif = 0;
    bool past_begining = false;
    for (int i=4; i<width-3; i++)
    {
        //This is designed to be equilivalent to a first derivitive, though filtering noise
        double lowpass_dif = ((profile[i-4]+profile[i-3]+profile[i-2]+2*profile[i-1])-(2*profile[i]+profile[i+1]+profile[i+2]+profile[i+3]))/11.0;
        //printf("dif=%f\n",lowpass_dif);
        if (lowpass_dif==0 && past_begining)
        {
            ret.append(i);
            printf("min(0):%d\n",i);
        }
        else if (lastDif>0 && lowpass_dif<0)
        {
            if (lastDif < -1*lowpass_dif)
            {
                ret.append(i-1);
                printf("min:%d\n",i-1);
            }
            else
            {
                ret.append(i);
                printf("min:%d\n",i);
            }
        }
        past_begining = past_begining || lowpass_dif<0;
        lastDif = lowpass_dif;
    }
    minima = ret;
   return ret;
}

double WordProfile::similarity(const WordProfile &compareTo)
{
   return 0.0;
}

int WordProfile::getWidth()
{
    return width;
}

int WordProfile::getValue(int i)
{
    if (i<width)
        return profile[i];
    else
        return -1;
}

void WordProfile::print()
{
   printf("Profile:%2d",0);
   for (int i=1; i<width; i++)
       printf(", %2d",i);
   printf("\n");
   printf("        %2d",profile[0]);
   for (int i=1; i<width; i++)
       printf(", %2d",profile[i]);
   printf("\n");
}
