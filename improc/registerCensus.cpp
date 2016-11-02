//g++ -std=c++11 registerCensus.cpp -o registerCensus -lopencv_core -lopencv_highgui -lopencv_imgproc -L/home/brian/robert_stuff/documentproject/lib -ldocumentproject -lpng -ltiff -ljpeg -lpthread -I/home/brian/robert_stuff/documentproject/src

//Used to reg the mexico census 
#include <fstream>
#include <iostream>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <vector>
#include <set>
#include <assert.h>
#include <string>
#include "dimage.h"
#include "dglobalskew.h"

using namespace std;
using namespace cv;

#define CUT_OFF_TOP 350
#define CUT_OFF_BOT 210
#define CUT_OFF_SIDE 90
#define BLOCK_SIZE 80

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}
Mat transform(Mat source, int offsetX, int offsetY, double rotation, int width, int height, double filler) {
    //assert(source.type() == CV_8U);
    int srcWidth = source.cols;
    int srcHeight = source.rows;
    assert(source.cols>0);
    //assert(offsetX >= -srcWidth && offsetX < srcWidth);
    //assert(offsetY >= -srcHeight && offsetY < srcHeight);
    //assert(width > 0 && width < srcWidth);
    //assert(height > 0 && height < srcHeight);
    Mat M = (Mat_<double>(2,3) << cos(rotation), -sin(rotation), offsetX, 
                                  sin(rotation),  cos(rotation), offsetY);
    Mat res;
    warpAffine(source,res,M,Size(width,height),INTER_LINEAR,BORDER_CONSTANT,filler);
    return res;
}

void convertToD(Mat& orig, DImage& dimg)
{
    dimg.setLogicalSize(orig.cols,orig.rows);
    unsigned char* data1 = dimg.dataPointer_u8();
    unsigned char* dataO = orig.data;
    for (int i=0; i< orig.cols * orig.rows; i++)
    {
        data1[i]=dataO[i];
    }
}

Mat grayInvert(const Mat& orig, int avg)
{
    //Mat ret(orig.rows,orig.cols, orig.type());
    //for (int r=0; r<orig.rows; r++)
    //    for(int c=0; c<orig.cols; c++)
    //        ret.at<unsigned

    //return (255-orig);
    return cv::max(((unsigned char)avg)-orig,0.0);
}

void increaseBot(Mat& horzLines, double thresh)
{
    bool on=false;
    for (int r=horzLines.rows-2; r>horzLines.rows/2; r--)
    {
        cout<<" "<<horzLines.at<double>(r,0);
        if (on || horzLines.at<double>(r,0)>0.15)//thresh)
        {
            on=true;
            if (horzLines.at<double>(r,0)<horzLines.at<double>(r+1,0))
            {
                horzLines.at<double>(r+1,0)*=2;
                return;
            }
        }
    }
}

double otsu(vector<int> histogram)
{
    double sum =0;
    int total=0;
    for (int i = 1; i < histogram.size(); ++i)
    {
        total+=histogram[i];
        sum += i * histogram[i];
    }
    double sumB = 0;
    double wB = 0;
    double wF = 0;
    double mB;
    double mF;
    double max = 0.0;
    double between = 0.0;
    double threshold1 = 0.0;
    double threshold2 = 0.0;
    for (int i = 0; i < histogram.size(); ++i)
    {
        wB += histogram[i];
        if (wB == 0)
            continue;
        wF = total - wB;
        if (wF == 0)
            break;
        sumB += i * histogram[i];
        mB = sumB / (wB*1.0);
        mF = (sum - sumB) / (wF*1.0);
        between = wB * wF * pow(mB - mF, 2);
        if ( between >= max )
        {
            threshold1 = i;
            if ( between > max )
            {
                threshold2 = i;
            }
            max = between;
        }
    }

    return ( threshold1 + threshold2 ) / 2.0;
}

int main (int argc, char** argv)
{
    if (argc<4)
    {
        cout<<"usage: "<<argv[0]<<" pp_old.csv imdir average.image [outDir]"<<endl;
        exit(1);
    }
    string fileN=argv[1];
    string imdir=argv[2];
    string out=argv[3];
    string outDir="";
    if (argc>4)
        outDir=argv[4];
    int resW=-1;
    int resH=-1;
    //vector< vector< multiset<unsigned char> > > res;
    vector< vector< long int > > res;
    //read reg csv
    ifstream filePP(fileN.c_str());
    assert(filePP.good());
    string line;
    getline(filePP,line);//burn header
    int count=0;
    int done=0;


    vector<string> images;
    while (getline(filePP,line))
    {
        stringstream ss(line);
        string tmp;
        getline(ss,tmp,',');
        trim(tmp);
        string xml=tmp;
        getline(ss,tmp,',');
        trim(tmp);
        string jpg=tmp;
        images.push_back(jpg);
    }
    filePP.close();

    Mat first = imread(images[0],CV_LOAD_IMAGE_GRAYSCALE);
    first=first(Rect(CUT_OFF_SIDE,CUT_OFF_TOP,first.cols-(2*CUT_OFF_SIDE),first.rows-(CUT_OFF_TOP+CUT_OFF_BOT)));
    //imshow("first",first);
    //waitKey();
    double subVal = sum(first)[0]/(first.cols*first.rows+0.0);
    double toss;
    //minMaxLoc(first,&toss,&subVal);
    //Mat firstB;
    //adaptiveThreshold(first, firstB, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, BLOCK_SIZE, 10);
    DImage dimg;
    convertToD(first,dimg);
    double skew = DGlobalSkew::getSkewAng_var(dimg,-1.1,1.1,0.1)/180.0 * CV_PI;
    first = transform(first,0,0,skew,first.cols,first.rows,subVal);
    first = grayInvert(first,subVal);


    cout<<images[0]<<": skew: "<<skew<<endl;
    imshow("first-skewed",first);
    waitKey();

    Mat vertLines; 
    reduce(first,vertLines,0,CV_REDUCE_SUM,CV_64F);
    double divVert;
    minMaxLoc(vertLines,&toss,&divVert);
    vertLines/=divVert;
    Mat horzLines; 
    reduce(first,horzLines,1,CV_REDUCE_SUM,CV_64F);
    double divHorz;
    minMaxLoc(horzLines,&toss,&divHorz);
    horzLines/=divHorz;


    for (int i=1; i<images.size(); i++)
    {
        Mat doc=imread(images[i],CV_LOAD_IMAGE_GRAYSCALE);
        doc=doc(Rect(CUT_OFF_SIDE,CUT_OFF_TOP,doc.cols-(2*CUT_OFF_SIDE),doc.rows-(CUT_OFF_TOP+CUT_OFF_BOT)));
        convertToD(doc,dimg);
        skew = DGlobalSkew::getSkewAng_var(dimg,-1.1,1.1,0.1)/180.0 * CV_PI;
        doc = transform(doc,0,0,skew,doc.cols,doc.rows,subVal);
        Mat inv = grayInvert(doc,subVal);
        //Check other angles?
        //Verify deskew
        Mat left = inv(Rect(0,.25*inv.rows,.2*inv.cols,.5*inv.rows));
        Mat horzLinesLeft; 
        reduce(left,horzLinesLeft,1,CV_REDUCE_SUM,CV_64F);
        horzLinesLeft/=divHorz;
        double minC, maxC;
        minMaxLoc(horzLinesLeft,&minC,&maxC);
        vector<int> hist(50);
        for (int r=0; r<horzLinesLeft.rows; r++)
        {
            int bin = 50*(horzLinesLeft.at<double>(r,0)-minC)/maxC;
            hist[bin]++;
        }
        double peakThresh = 0.7* ((otsu(hist)/50)*maxC + minC);
        vector<int> peaksLeft;
        for (int r=0; r<horzLinesLeft.rows; r++)
        {
            if (horzLinesLeft.at<double>(r,0)>peakThresh)
            {
                bool bad=false;
                for (int n=-20; n<=20; n++)
                {
                    if (r+n>=0 && r+n<horzLinesLeft.rows &&
                            horzLinesLeft.at<double>(r,0)<horzLinesLeft.at<double>(r+n,0))
                    {
                        bad=true;
                        break;
                    }
                }
                if (!bad)
                    peaksLeft.push_back(r);
            }
        }
        ///
        Mat show;
        cvtColor(left,show,CV_GRAY2RGB);
        for (int peak : peaksLeft)
        {
            int r = peak;
            for (int c=0; c<left.cols; c++)
            {
                show.at<Vec3b>(r,c)[0]=255;
                show.at<Vec3b>(r,c)[1]=0;
            }
        }
        imshow("left",show);
        waitKey(1000);
        ///
        Mat right = inv(Rect(.8*inv.cols,.25*inv.rows,.2*inv.cols,.5*inv.rows));
        Mat horzLinesRight; 
        reduce(right,horzLinesRight,1,CV_REDUCE_SUM,CV_64F);
        horzLinesRight/=divHorz;


        Mat vertLinesDoc; 
        reduce(inv,vertLinesDoc,0,CV_REDUCE_SUM,CV_64F);
        vertLinesDoc/=divVert;
        Mat horzLinesDoc; 
        reduce(inv,horzLinesDoc,1,CV_REDUCE_SUM,CV_64F);
        horzLinesDoc/=divHorz;
        //cout<<"thresh: "<<(100*doc.cols)/divHorz<<endl;
        //increaseBot(horzLinesDoc,2.0);
        //cout<<endl;

        double bestScoreX=-1;
        double bestScaleX=0;
        int bestX=0;
        double bestScoreY=-1;
        double bestScaleY=0;
        int bestY=0;
        for (double scale=0.96; scale<=1.04; scale+=0.02)
        {
            Mat scaledVertLinesDoc;
            resize(vertLinesDoc,scaledVertLinesDoc,Size(),scale,1);
            for (int offsetx=-240; offsetx<=240; offsetx+=2)
            {
                double score = sum(vertLines.mul(transform(scaledVertLinesDoc,offsetx,0,0,vertLines.cols,vertLines.rows,0)))[0];
                //cout<<score<<"=x= s:"<<scale<<" x:"<<offsetx<<endl;
                if (score>bestScoreX)
                {
                    bestScaleX=scale;
                    bestX=offsetx;
                    bestScoreX=score;
                }
            }
            Mat scaledHorzLinesDoc;
            resize(horzLinesDoc,scaledHorzLinesDoc,Size(),1,scale);
            for (int offsety=-240; offsety<=240; offsety+=2)
            {
                double score = sum(horzLines.mul(transform(scaledHorzLinesDoc,0,offsety,0,horzLines.cols,horzLines.rows,0)))[0];
                //cout<<score<<"=y= s:"<<scale<<" y:"<<offsety<<endl;
                if (score>bestScoreY)
                {
                    bestScaleY=scale;
                    bestY=offsety;
                    bestScoreY=score;
                }
            }
        }
        if (bestScaleY != bestScaleX)
        {
            double scale = (bestScaleY+bestScaleX)/2.0;
            bestScaleX=scale;
            bestScaleY=scale;
            bestScoreX=-1;
            bestX=0;
            bestScoreY=-1;
            bestY=0;

            Mat scaledVertLinesDoc;
            resize(vertLinesDoc,scaledVertLinesDoc,Size(),scale,1);
            for (int offsetx=-240; offsetx<=240; offsetx+=2)
            {
                double score = sum(vertLines.mul(transform(scaledVertLinesDoc,offsetx,0,0,vertLines.cols,vertLines.rows,0)))[0];
                if (score>bestScoreX)
                {
                    bestX=offsetx;
                    bestScoreX=score;
                }
            }
            Mat scaledHorzLinesDoc;
            resize(horzLinesDoc,scaledHorzLinesDoc,Size(),1,scale);
            for (int offsety=-240; offsety<=240; offsety+=2)
            {
                double score = sum(horzLines.mul(transform(scaledHorzLinesDoc,0,offsety,0,horzLines.cols,horzLines.rows,0)))[0];
                if (score>bestScoreY)
                {
                    bestY=offsety;
                    bestScoreY=score;
                }
            }
        }
        resize(doc,doc,Size(),bestScaleX,bestScaleX);
        Mat reg = transform(doc,bestX,bestY,0/*rotation*/,doc.cols+std::min(bestX,0),doc.rows+std::min(bestY,0),subVal);
        if (outDir.length()>0)
        {
            int index = images[i].find_last_of('/');
            string file = images[i].substr(index+1);
            imwrite(outDir+file,reg);
            cout<<file<<"= "<<bestScoreX<<" + "<<bestScoreY<<" = "<<(bestScoreX+bestScoreY)<<" skew:"<<skew<<" scale:"<<bestScaleX<<" x:"<<bestX<<" y:"<<bestY<<endl;
            imshow("reg",reg);
            waitKey();
        }
        //if (conf<60)
        //    continue;
        count++;
        if (count%9!=0)
            continue;
        if (resW==-1)
        {
            resW=reg.cols;
            resH=reg.rows;
            res.resize(resH);
            for (int r=0; r<resH; r++)
                res[r].resize(resW);
        }
        /*else
        {
            resize(reg,reg,Size(resW,resH));
        }*/
        //assert(reg.rows==resH && reg.cols==resW);
        for (int r=0; r<min(resH,reg.rows); r++)
            for (int c=0; c<min(resW,reg.cols); c++)
                //res[r][c].insert(reg.at<unsigned char>(r,c));
                res[r][c]+=reg.at<unsigned char>(r,c);
        //cout <<"did "<<file<<endl;
        done++;
        if (done>1000)
            break;

    }
    //fileReg.close();
    Mat med(resH,resW,CV_8U);
    for (int r=0; r<resH; r++)
        for (int c=0; c<resW; c++)
        {
            //auto iter = res[r][c].begin();
            //for (int i=0; i<res[r][c].size()/2; i++)
            //    iter++;
            //med.at<unsigned char>(r,c)=*iter;
            med.at<unsigned char>(r,c)=(0.0+res[r][c])/done;
        }
    imwrite(out,med);
}
