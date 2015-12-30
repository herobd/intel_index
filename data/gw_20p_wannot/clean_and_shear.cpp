#include "mex.h"

#include "dimage.h"
#include "dslantangle.h"
#include "dtextlineseparator.h"
#include "dthresholder.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "binarization.h"

#include <list>
#include <vector>
#include <algorithm>
#include <iostream>

#define SCRUB_THRESH 220

using namespace std;
using namespace cv;

void convertMatToDImage(Mat& mat, DImage* dimg)
{
    dimg->setLogicalSize(mat.cols,mat.rows);
    unsigned char* dataD = dimg->dataPointer_u8();
    unsigned char* dataM = mat.data;
    for (int i=0; i< mat.cols * mat.rows; i++)
    {
        dataD[i]=dataM[i];
    }
}

void convertDImageToMat(DImage& dimg, Mat* mat)
{
    *mat = Mat_<unsigned char>(dimg.height(),dimg.width());
    unsigned char* dataD = dimg.dataPointer_u8();
    unsigned char* dataM = mat->data;
    for (int i=0; i< dimg.width() * dimg.height(); i++)
    {
        dataM[i]=dataD[i];
    }
}

int xDelta(int i)
{
    if (i==0 || i==2)
        return 0;
    if (i==4||i==1||i==5)
        return 1;
    if (i==3||i==6||i==7)
        return -1;
}
int yDelta(int i)
{
    if (i==3 || i==1)
        return 0;
    if (i==6||i==2||i==5)
        return 1;
    if (i==0||i==4||i==7)
        return -1;
}

void scrubCC(const Mat& bin, Mat& mask, int xStart, int yStart)
{
    vector<bool> visited(bin.cols*bin.rows);
    visited.assign(bin.cols*bin.rows,false);
    list<Point> toVisit;
    toVisit.push_back(Point(xStart,yStart));
    visited[xStart+yStart*bin.cols]=true;
    int count = 0;
    int minY=9999;
    vector<int> minY_x;
    int maxY=0;
    vector<int> maxY_x;
    int thresh = std::min(SCRUB_THRESH,0.05*(bin.rows*bin.cols));
    while (!toVisit.empty() && count<=thresh)
    {
        Point cur = toVisit.back();
        toVisit.pop_back();

        for (int direction=0; direction<8; direction++)
        {
            int x = cur.x+xDelta(direction);
            int y = cur.y+yDelta(direction);
            if (x<0 || y<0 || x>=bin.cols || y>=bin.rows)
                continue;
            if (bin.at<unsigned char>(y,x)>0 && !visited[x+y*bin.cols])
            {
                if (++count > SCRUB_THRESH) break;
                visited[x+y*bin.cols]=true;
                toVisit.push_back(Point(x,y));
                if (y<minY)
                {
                    minY=y;
                    minY_x.clear();
                    minY_x.push_back(x);
                }
                else if (y==minY)
                {
                    minY_x.push_back(x);
                }
                
                if (y>maxY)
                {
                    maxY=y;
                    maxY_x.clear();
                    maxY_x.push_back(x);
                }
                else if (y==maxY)
                {
                    maxY_x.push_back(x);
                }
            }
        }
    }

    if (count <= thresh)
    {   
        //cout <<"removing blob"<<endl;
        toVisit.clear();
        toVisit.push_back(Point(xStart,yStart));
        while (!toVisit.empty())
        {
            Point cur = toVisit.back();
            toVisit.pop_back();

            for (int direction=0; direction<8; direction++)
            {
                int x = cur.x+xDelta(direction);
                int y = cur.y+yDelta(direction);
                if (x<0 || y<0 || x>=bin.cols || y>=bin.rows)
                    continue;
                if (bin.at<unsigned char>(y,x)>0 && mask.at<unsigned char>(y,x)==0)
                {
                    mask.at<unsigned char>(y,x)=1;
                    toVisit.push_back(Point(x,y));
                }
            }
        }
    }
    else if (minY==0 && maxY==bin.rows-1)//take care of verticle lines on the sides
    {
        //cout <<"removing vert line"<<endl;
        sort(minY_x.begin(),minY_x.end());
        sort(maxY_x.begin(),maxY_x.end());
        vector<int> xMatches(minY_x.size()+maxY_x.size());
        auto xMatchesEnd = set_union(minY_x.begin(),minY_x.end(), 
                                     maxY_x.begin(),maxY_x.end(),
                                     xMatches.begin());
        xMatches.resize(xMatchesEnd-xMatches.begin());
        int iterMatches=0;
        int iterXs=0;
        while (iterXs<minY_x.size() && iterMatches<xMatches.size())
        {
            int x = minY_x[iterXs];
            if ( (x<xMatches[iterMatches] && x+2>=xMatches[iterMatches]) ||
                 (x=xMatches[iterMatches]) ||
                 (x>xMatches[iterMatches] && x-2<=xMatches[iterMatches])
               )
            {
                //scrub
                for (int y=0; y<bin.rows-1; y++)
                    if (bin.at<unsigned char>(y,x)>0)
                        mask.at<unsigned char>(y,x)=1;
            }
            if (x<xMatches[iterMatches])
                iterXs++;
            else if (x==xMatches[iterMatches])
            {
                iterXs++;
                iterMatches++;
            }
            else
                iterMatches++;
        }
        
        iterMatches=0;
        iterXs=0;
        while (iterXs<maxY_x.size() && iterMatches<xMatches.size())
        {
            int x = maxY_x[iterXs];
            if ( (x<xMatches[iterMatches] && x+2>=xMatches[iterMatches]) ||
                 (x>xMatches[iterMatches] && x-2<=xMatches[iterMatches])
               )
            {
                //scrub
                for (int y=0; y<bin.rows-1; y++)
                    if (bin.at<unsigned char>(y,x)>0)
                        mask.at<unsigned char>(y,x)=1;
            }
            if (x<xMatches[iterMatches])
                iterXs++;
            else if (x==xMatches[iterMatches])
            {
                iterXs++;
                iterMatches++;
            }
            else
                iterMatches++;
        }
    }
    
}

void cleanNoiseFromEdges(const Mat& bin, const Mat& bgEst, Mat& orig)
{
    Mat mask(bin.rows,bin.cols,CV_8U,Scalar(0));
    for (int x=0; x<5; x++)
        for (int y=0; y<bin.rows; y++)
            if (bin.at<unsigned char>(y,x)>0)
                scrubCC(bin,mask,x,y);

    for (int y=0; y<5; y++)
        for (int x=0; x<bin.cols; x++)
            if (bin.at<unsigned char>(y,x)>0)
                scrubCC(bin,mask,x,y);

    for (int x=bin.cols-5; x<bin.cols; x++)
        for (int y=0; y<bin.rows; y++)
            if (bin.at<unsigned char>(y,x)>0)
                scrubCC(bin,mask,x,y);

    for (int y=bin.rows-5; y<bin.rows; y++)
        for (int x=0; x<bin.cols; x++)
            if (bin.at<unsigned char>(y,x)>0)
                scrubCC(bin,mask,x,y);
    for (int y=0; y<bin.rows; y++)
        for (int x=0; x<bin.cols; x++)
            if (mask.at<unsigned char>(y,x)==1)
                orig.at<unsigned char>(y,x)=bgEst.at<unsigned char>(y,x);
}

void 
mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

	// check for proper number and size of arguments
	//errCheck(nrhs == 1,"Arguments:  filePath");
	//errCheck(nlhs == 1,"Outputs:  skew");

	std::string filePath = mxArrayToString(prhs[0]);
	std::string dirPath = mxArrayToString(prhs[1]);
	double slant = mxGetScalar(prhs[2]);
	
	bool VIZ=false;
	/*if (filePath.compare("wordimg_9")==0 || 
        filePath.compare("wordimg_78")==0 )
       VIZ=true;
	*/
	//DImage img(("tmp/"+filePath+".tif").c_str(),DImage::DFileFormat_tiff);
	Mat mat = imread("tmp/"+filePath+".tif", CV_LOAD_IMAGE_GRAYSCALE);
	//cout << "read image" <<endl;
	//TODO clean
	Mat initBin = Binarization::otsuBinarization(mat);
    if (VIZ)
        {Binarization::imshowB("otsu",initBin,255,0); waitKey();}
        
    Mat dilated = Binarization::dilate(initBin,3);//dilate the fg, shrinking the bg
    //cout << "dilated" <<endl;
    
    
    Mat dilatedFlip = dilated.clone();
    for (int r=0; r<dilatedFlip.rows; r++)
        for (int c=0; c<dilatedFlip.cols; c++)
        {
            if (dilatedFlip.at<unsigned char>(r,c)==1)
                dilatedFlip.at<unsigned char>(r,c)=0;
            else
                dilatedFlip.at<unsigned char>(r,c)=1;
        }
    
    Mat prime;
    Mat bg_estimation = Binarization::inpainting(mat,dilatedFlip,&prime);
    //cout << "inpainted" <<endl;
    if (VIZ)
        {imshow("bg_estimation",bg_estimation); waitKey();}
    double average, std;
    Binarization::extract_feat(prime, NULL, &average, &std);
    bg_estimation = Binarization::inpainting(mat,dilatedFlip,NULL,&average,&std);
    //cout << "found avg" <<endl;
    
    cleanNoiseFromEdges(dilated, bg_estimation, mat);
    //cout << "cleaned" <<endl;
	if (VIZ)
	    {imshow("cleaned",mat); waitKey();}
	
	DImage img;
	convertMatToDImage(mat,&img);
	//cout << "converted" <<endl;
	int pad=255;
	img = img.shearedH(slant,pad,true);//fill corners with black
	//cout << "sheared" <<endl;
	
	
	convertDImageToMat(img,&mat);
	//cout << "converted back" <<endl;
    if (VIZ)
	    {imshow("sheared",mat); waitKey();}
    
    //fill corners
	Mat fillMaskFlipped(mat.rows,mat.cols,CV_8U,Scalar(1));
	for (int r=fillMaskFlipped.rows-1; r>=0; r--)
        for (int c=0; c<fillMaskFlipped.cols; c++)
        {
            int flag = (abs(mat.at<unsigned char>(r,c)-pad)<3 || 
                        abs(mat.at<unsigned char>(r+1,c-1)-pad)<3 ||
                        abs(mat.at<unsigned char>(r-1,c+1)-pad)<3)?0:1;
            fillMaskFlipped.at<unsigned char>(r,c)=flag;
            
        }
	
	
	/*
	int space=1;
	//for (int r=fillMaskFlipped.rows-1; r>fillMaskFlipped.rows-1-space; r--)
    //    for (int c=0; c<space; c++)
    //    {
    //        int flag = (abs(mat.at<unsigned char>(r,c)-pad)<3)?0:1;
    //        fillMaskFlipped.at<unsigned char>(r,c)=flag;
    //    }
        
    for (int r=fillMaskFlipped.rows-1; r>=0; r--)
        for (int c=0; c<space; c++)
        {
            int flag = (abs(mat.at<unsigned char>(r,c)-pad)<3)?0:1;
            fillMaskFlipped.at<unsigned char>(r,c)=flag;
        }
	
	for (int c=0; c<fillMaskFlipped.cols; c++)
	    for (int r=fillMaskFlipped.rows-1; r>fillMaskFlipped.rows-1-space; r--)
        {
            int flag = (abs(mat.at<unsigned char>(r,c)-pad)<3)?0:1;
            fillMaskFlipped.at<unsigned char>(r,c)=flag;
        }
        
    for (int c=0; c<fillMaskFlipped.cols-space; c++)
    {
        bool cont=false;
        for (int r=fillMaskFlipped.rows-1; r>=space; r--)
        {
            int flag = (abs(mat.at<unsigned char>(r,c)-pad)<3)?0:1;
            if (flag==0 && !cont)
                cont=true;
            fillMaskFlipped.at<unsigned char>(r-space,c+space)=flag;
            //if (r==fillMaskFlipped.rows-1 && flag==0)
            //{
            //    for (int rf=r-space-1; rf<=fillMaskFlipped.rows-1; rf++)
            //        fillMaskFlipped.at<unsigned char>(rf,c+space)=flag;
            //}
            //if (c==0 && flag==0)
            //{
            //    for (int cf=c+space-1; cf>=0; cf--)
            //        fillMaskFlipped.at<unsigned char>(r-space,cf)=flag;
            //}
        
        }
        if (!cont)
            break;
    }
    
    //for (int c=fillMaskFlipped.cols-1; c>fillMaskFlipped.cols-1-space; c--)
    //    for (int r=0; r<space; r++)
    //    {
    //        int flag = (abs(mat.at<unsigned char>(r,c)-pad)<3)?0:1;
    //        fillMaskFlipped.at<unsigned char>(r,c)=flag;
    //    }
    for (int c=fillMaskFlipped.cols-1; c>=0; c--)
        for (int r=0; r<space; r++)
        {
            int flag = (abs(mat.at<unsigned char>(r,c)-pad)<3)?0:1;
            fillMaskFlipped.at<unsigned char>(r,c)=flag;
        }
    for (int r=0; r<fillMaskFlipped.rows; r++)
        for (int c=fillMaskFlipped.cols-1; c>fillMaskFlipped.cols-1-space; c--)
        {
            int flag = (abs(mat.at<unsigned char>(r,c)-pad)<3)?0:1;
            fillMaskFlipped.at<unsigned char>(r,c)=flag;
        }
    
    for (int c=fillMaskFlipped.cols-1; c>=space; c--)
    {
        bool cont=false;
        for (int r=0; r<fillMaskFlipped.rows-space; r++)
        {
            int flag = (abs(mat.at<unsigned char>(r,c)-pad)<3)?0:1;
            if (flag==0 && !cont)
                cont=true;
            fillMaskFlipped.at<unsigned char>(r+space,c-space)=flag;
            //if (c==fillMaskFlipped.cols-1 && flag==0)
            //{
           //   for (int cf=c-space-1; cf<=fillMaskFlipped.cols-1; cf++)
           //         fillMaskFlipped.at<unsigned char>(r+space,cf)=flag;
            //}
            //if (r==0 && flag==0)
            //{
            //    for (int rf=r+space-1; rf>=0; rf--)
            //        fillMaskFlipped.at<unsigned char>(rf,c-space)=flag;
            //}
        
        }
        if (!cont)
            break;
    }*/
    if (VIZ)
    {   Binarization::imshowB("fillamsk",fillMaskFlipped,255,0); waitKey();
        Mat showMask = mat.clone();
        Mat showUnmask = mat.clone();
        for (int r=fillMaskFlipped.rows-1; r>=0; r--)
            for (int c=0; c<fillMaskFlipped.cols; c++)
            {
                if (fillMaskFlipped.at<unsigned char>(r,c) == 0)
                    showUnmask.at<unsigned char>(r,c)=255;
                else if (fillMaskFlipped.at<unsigned char>(r,c) == 1)
                    showMask.at<unsigned char>(r,c)=255;
            }
        imshow("masked",showMask); waitKey();
        imshow("unmasked",showUnmask); waitKey();
    }
	mat = Binarization::inpainting(mat,fillMaskFlipped,NULL,&average,&std,VIZ);
	//cout << "filled" <<endl;
	/*for (int r=0; r<mat.rows; r++)
	    for (int c=0; c<mat.cols; c++)
	    {
	        if (fillMaskFlipped.at<unsigned char>(r,c)==0 && fabs(mat.at<unsigned char>(r,c)-average)>std)
	            mat.at<unsigned char>(r,c)=average;
	    }*/
	if (VIZ)
	    {imshow("filled",mat); waitKey();}
	//img.save((dirPath+"/"+filePath+".png").c_str(), DImage::DFileFormat_png);
	imwrite(dirPath+"/"+filePath+".png",mat);
	//cout << "write "<<dirPath<<"/"<<filePath<<".png"<<endl;
}


