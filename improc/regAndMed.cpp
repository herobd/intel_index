//g++ -std=c++11 regAndMed.cpp -o regAndMed -lopencv_core -lopencv_highgui -lopencv_imgproc

//Used to reg the us1930 with the given "params_1930_Census.csv" file.
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

using namespace std;
using namespace cv;

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
Mat transform(Mat source, int offsetX, int offsetY, double rotation, int width, int height) {
    assert(source.type() == CV_8U);
    int srcWidth = source.cols;
    int srcHeight = source.rows;
    assert(offsetX >= -srcWidth && offsetX < srcWidth);
    assert(offsetY >= -srcHeight && offsetY < srcHeight);
    assert(width > 0 && width < srcWidth);
    assert(height > 0 && height < srcHeight);
    Mat M = (Mat_<double>(2,3) << cos(rotation), -sin(rotation), offsetX, 
                                  sin(rotation),  cos(rotation), offsetY);
    Mat res;
    warpAffine(source,res,M,Size(width,height));
    return res;
}

int main (int argc, char** argv)
{
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
    ifstream fileReg(fileN.c_str());
    assert(fileReg.good());
    string line;
    getline(fileReg,line);//burn header
    int count=0;
    int done=0;
    while (getline(fileReg,line))
    {
        stringstream ss(line);
        string tmp;
        getline(ss,tmp,',');
        trim(tmp);
        string state=tmp;
        getline(ss,tmp,',');
        trim(tmp);
        string file=tmp;
        getline(ss,tmp,',');
        trim(tmp);
        int offsetx=stoi(tmp);
        getline(ss,tmp,',');
        trim(tmp);
        int offsety=stoi(tmp);
        getline(ss,tmp,',');
        trim(tmp);
        double rotation=stof(tmp);
        getline(ss,tmp,',');
        trim(tmp);
        int width=stoi(tmp);
        getline(ss,tmp,',');
        trim(tmp);
        int height=stoi(tmp);
        getline(ss,tmp,',');
        trim(tmp);
        int conf=stoi(tmp);

        Mat orig=imread(imdir+file,CV_LOAD_IMAGE_GRAYSCALE);
        Mat reg = transform(orig,offsetx,offsety,rotation,width,height);
        if (outDir.length()>0)
            imwrite(outDir+file,reg);
            //imshow("reg",reg);
        //waitKey();
        if (conf<60)
            continue;
        count++;
        if (count>100 && count%2==0)
            continue;
        if (count>300 && count%3!=1)
            continue;
        if (count>600 && count%13!=2)
            continue;
        if (count>1200 && count%43!=3)
            continue;
        if (count>5200 && count%203!=4)
            continue;
        if (resW==-1)
        {
            resW=reg.cols;
            resH=reg.rows;
            res.resize(resH);
            for (int r=0; r<resH; r++)
                res[r].resize(resW);
        }
        else
        {
            resize(reg,reg,Size(resW,resH));
        }
        assert(reg.rows==resH && reg.cols==resW);
        for (int r=0; r<resH; r++)
            for (int c=0; c<resW; c++)
                //res[r][c].insert(reg.at<unsigned char>(r,c));
                res[r][c]+=reg.at<unsigned char>(r,c);
        cout <<"did "<<file<<endl;
        done++;
        //if (done>1000)
        //    break;

    }
    fileReg.close();
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
