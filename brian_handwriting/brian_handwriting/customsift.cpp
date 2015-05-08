#include "customsift.h"

CustomSIFT::CustomSIFT()
{
}

double getSubpix(const Mat& img, Point2f pt)
{
    Mat patch;
    getRectSubPix(img, cv::Size(1,1), pt, patch);
    return patch.at<int>(0,0);
}

double getSubpix(const Mat& img, float x, float y, KeyPoint p)
{
    Mat patch;
    Point2f pt (x+p.pt.x,y+p.pt.y);
    getRectSubPix(img, cv::Size(1,1), pt, patch);
    return patch.at<int>(0,0);
}

void CustomSIFT::extract(const Mat &img, const vector<KeyPoint> &keyPoints, vector< vector<double> >& descriptors)
{
    
    descriptors.resize(keyPoints.size());
    for (int kpi=0; kpi<keyPoints.size(); kpi++)
    {
        KeyPoint p = keyPoints[kpi];
        //skipping blur, we'll just to interpolation
        double scaling = p.size/16;
        //vector<double> bins(128);
        descriptors[kpi].resize(128);
        descriptors[kpi].assign(128,0);
        int boxNum=0;
        for (int xCorner=0; xCorner<=12; xCorner+=4)
        {
            for (int yCorner=0; yCorner<=12; yCorner+=4)
            {
                //vector<double> bins(8);
                //bins.assign(8,0);
                
                for (int xOffset=0; xOffset<4; xOffset++)
                {
                    for (int yOffset=0; yOffset<4; yOffset++)
                    {  
                        double xPointOffset = ((xCorner+xOffset)-7.5) * scaling;
                        double yPointOffset = ((yCorner+yOffset)-7.5) * scaling;
                        double xPointOffset_p1 = ((xCorner+xOffset+1)-7.5) * scaling;
                        double yPointOffset_p1 = ((yCorner+yOffset+1)-7.5) * scaling;
                        double xPointOffset_m1 = ((xCorner+xOffset-1)-7.5) * scaling;
                        double yPointOffset_m1 = ((yCorner+yOffset-1)-7.5) * scaling;
                        double mag = sqrt(pow(getSubpix(img,xPointOffset_p1,yPointOffset,p)-
                                              getSubpix(img,xPointOffset_m1,yPointOffset,p),2) +
                                          pow(getSubpix(img,xPointOffset,yPointOffset_p1,p)-
                                              getSubpix(img,xPointOffset,yPointOffset_m1,p),2));
                        double theta = atan2((getSubpix(img,xPointOffset,yPointOffset_p1,p)-
                                            getSubpix(img,xPointOffset,yPointOffset_m1,p)),
                                           (getSubpix(img,xPointOffset_p1,yPointOffset,p)-
                                            getSubpix(img,xPointOffset_m1,yPointOffset,p)));
                        
                        //Guassian weighting
                        mag *=exp(-1*(pow((xCorner+xOffset)-7.5,2)/(2*16) + 
                                      pow((yCorner+yOffset)-7.5,2)/(2*16)));
                       // cout << "mag: "<<mag<<" theta: "<<theta<<endl;
                        
                        int bin = (theta+CV_PI)/(2*CV_PI) * 8;
                        //cout << "["<<kpi<<"][8*("<<boxNum<<")+"<<bin<<"]"<<endl;
                        descriptors[kpi][8*(boxNum)+bin] += mag;
                    }
                }
                
                boxNum++;
            }
        }
        
        //normalize
        double max=0;
        double min=99999;
        for (double v : descriptors[kpi])
        {
            if (v>max)
                max=v;
            if (v<min)
                min=v;
        }
        if (max!= 0)
            for (int i=0; i<descriptors[kpi].size(); i++)
            {
                descriptors[kpi][i] = (descriptors[kpi][i]-min)/max;
            }
        
        //descriptors.push_back(bins);
        
//        cout << "des: " << descriptors[kpi][0];
//        for (int i=1; i<descriptors[kpi].size(); i++)
//        {
//            cout  << ", "<<descriptors[kpi][i];
//        }
//        cout <<endl;
    }
    
    assert(descriptors.size() == keyPoints.size());
    //cout << "extracted " << descriptors.size() <<endl;
}
