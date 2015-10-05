/*Brian Davis
 *Jitter
 *
 *This project is simply to add some noise to a directory of images.
 *We expect the images to be indexed by some sort of number.
 */

#include <iostream>
#include <random>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;
using namespace cv;


int main(int argc, char** argv)
{
    string dir = argv[1];
    int startIndex = atoi(argv[2]);
    int endIndex = atoi(argv[3]);
    int numberOfRep = atoi(argv[4]);
    
    string imageNamePre = argv[5];
    string imageNamePost = argv[6];
    
    default_random_engine generator;
    normal_distribution<double> shiftH_distribution(0,4);
    normal_distribution<double> shiftV_distribution(0,2);
    normal_distribution<double> noise_distribution(0,40);
    
    for (int i=0; i<numberOfRep; i++)
    {
        for (int index = startIndex; index<=endIndex; index++)
        {
            int newIndex=1+(i+1)*(1+endIndex-startIndex) + (index-startIndex);
            Mat img = imread(dir + imageNamePre + to_string(index) + imageNamePost,CV_LOAD_IMAGE_GRAYSCALE);
            
            int shiftH = shiftH_distribution(generator);
            if (shiftH > 8 || shiftH < -8) shiftH=0;
            int shiftV = shiftV_distribution(generator);
            if (shiftV > 4 || shiftV < -4) shiftV=0;
            
            Mat newImg = img.clone();
            for (int x=0; x<img.cols; x++)
                for (int y=0; y<img.rows; y++)
                {
                    int noise = noise_distribution(generator);
                    int newVal = 255;
                    if (x+shiftH >= 0 && x+shiftH < img.cols && y+shiftV >= 0 && y+shiftV < img.rows)
                        newVal = img.at<unsigned char>(y+shiftV,x+shiftH) + noise;
                    if (newVal<0) newVal=0;
                    else if (newVal>255) newVal=255;
                    newImg.at<unsigned char>(y,x) = newVal;
                }
            
            imwrite(dir + imageNamePre + to_string(newIndex) + imageNamePost, newImg);
        }
    }
}


