#include "preprocessor.h"


Preprocessor::Preprocessor(unsigned int flags)
{
    doBaselineCenter = (flags & PP_BASELINE_CENTER)!=0;
    doBaselineNormalization = (flags & PP_BASELINE_NORMALIZE)!=0;

    baseLineNorm=20;
}
Mat Preprocessor::process(const Mat& orig) const
{
    Mat ret;
    if (orig.type()==CV_8UC3)
       cvtColor(orig,ret,CV_RGB2GRAY);
    else
       ret = orig.clone();
    if (doBaselineCenter || doBaselineNormalization)
    {
        int avgWhite=0;
        int countWhite=0;
        Mat bin;
        int blockSize = min(ret.rows/1.5,ret.cols/1.5);
        if (blockSize%2==0)
            blockSize++;
        adaptiveThreshold(ret, bin, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, blockSize, 10);//255 is 'on'
        Mat hist = Mat::zeros(ret.rows,1,CV_32F);
        map<int,int> topPixCounts, botPixCounts;
        for (int c=0; c<ret.cols; c++)
        {
            int topPix=-1;
            int lastSeen=-1;
            for (int r=0; r<ret.rows; r++)
            {
                if (bin.at<unsigned char>(r,c))
                {
                    if (topPix==-1)
                    {
                        topPix=r;
                    }
                    lastSeen=r;
                }
                else
                {
                    avgWhite+=ret.at<unsigned char>(r,c);
                    countWhite++;
                }
                hist.at<float>(r,0)+=ret.at<unsigned char>(r,c);
            }
            if (topPix!=-1)
                topPixCounts[topPix]++;
            if (lastSeen!=-1)
                botPixCounts[lastSeen]++;
        }
        avgWhite /= countWhite;

        int maxTop=-1;
        int maxTopCount=-1;
        for (auto c : topPixCounts)
        {
            if (c.second > maxTopCount)
            {
                maxTopCount=c.second;
                maxTop=c.first;
            }
        }
        int maxBot=-1;
        int maxBotCount=-1;
        for (auto c : botPixCounts)
        {
            if (c.second > maxBotCount)
            {
                maxBotCount=c.second;
                maxBot=c.first;
            }
        }
        
        //Mat kernel = Mat::ones( 5, 1, CV_32F )/ (float)(5);
        //filter2D(hist, hist, -1 , kernel );
        Mat edges;
        float kernelData[11] = {1,1,1,1,0.5,0,-0.5,-1,-1,-1,-1};
        Mat kernel = Mat(11,1,CV_32F,kernelData);
        filter2D(hist, edges, -1 , kernel );
        int topBaseline=-1;
        float maxEdge=-9999999;
        int botBaseline=-1;
        float minEdge=9999999;
        for (int r=0; r<ret.rows; r++)
        {
            float v = edges.at<float>(r,0);
            if (v>maxEdge)
            {
                maxEdge=v;
                topBaseline=r;
            }
            if (v<minEdge)
            {
                minEdge=v;
                botBaseline=r;
            }
        }

        //cout << "Top: "<<maxTop<<" "<<topBaseline<<endl;
        //cout << "Bot: "<<maxBot<<" "<<botBaseline<<endl;
        if (botBaseline < topBaseline)//If they fail this drastically, the others won't be much better.
        {
            topBaseline=maxTop;
            botBaseline=maxBot;
        }

        if (doBaselineCenter)
        {
            int center = topBaseline + (botBaseline-topBaseline)/2;
            int adjust = (ret.rows/2)-center;
            //+ add to top, - add to bottom
            Mat newRet = Mat::zeros(ret.rows+abs(adjust),ret.cols,ret.type());
            //newRet(Rect(0,max(0,adjust),ret.cols,ret.rows)) = ret;
            ret.copyTo(newRet(Rect(0,max(0,adjust),ret.cols,ret.rows)));
            ret=newRet;
            if (adjust>0)
            {
                for (int r=adjust-1; r>=0; r--)
                {
                    int lastWhite = avgWhite;
                    for (int c=0; c<ret.cols; c++)
                    {
                        int fromR=2*adjust-r-1;
                        int whiteValue = lastWhite;
                        while (bin.at<unsigned char>(fromR,c)&&fromR<ret.rows)
                            fromR++;
                        if (fromR<ret.rows) {
                        //if (!bin.at<unsigned char>(fromR,c)) {

                            whiteValue += ret.at<unsigned char>(fromR,c);
                            whiteValue/=2;
                        }
                        double toTop = (adjust-r)/(1.0*adjust);
                        whiteValue = whiteValue*(1-toTop) + avgWhite*toTop;
                        ret.at<unsigned char>(r,c)=whiteValue;
                        lastWhite=whiteValue;
                    }
                }
            }
            if (adjust<0)
            {
                for (int r=ret.rows+adjust; r<ret.rows; r++)
                {
                    int lastWhite=avgWhite;
                    for (int c=0; c<ret.cols; c++)
                    {
                        int fromR=2*(ret.rows+adjust)-r-1;
                        double toBot = (0.0+r-(ret.rows+adjust))/(-1.0*adjust);
                        int whiteValue = lastWhite;
                        //while (bin.at<unsigned char>(fromR,c)&&fromR>=0)
                        //    fro;
                        //if (fromR>=0) {
                        if (!bin.at<unsigned char>(fromR,c)) {
                            //whiteValue = ret.at<unsigned char>(fromR,c);
                            whiteValue += ret.at<unsigned char>(fromR,c);
                            whiteValue/=2;
                        }
                        whiteValue = whiteValue*(1-toBot) + avgWhite*toBot;
                        ret.at<unsigned char>(r,c)=whiteValue;
                        assert(abs(whiteValue-lastWhite)<100);
                        lastWhite=whiteValue;
                    }
                }
            }

        }

        if (doBaselineNormalization)
        {
            double scale = baseLineNorm/(botBaseline-topBaseline);
            resize(ret,ret,Size(),1,scale);
        }
    }
    return ret;
}
