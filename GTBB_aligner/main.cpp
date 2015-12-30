/*Program: --
 *Author: Brian Davis (briandavis@byu.net)
 *Purpose: 
 *Usage: 
 *
 */


#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "dimage.h"
#include "dglobalskew.h"
#include "dtextlineseparator.h"
#include "dslantangle.h"
#include "dhough.h"

#include <vector>
#include <assert.h>
#include <iostream>
#include <string>
#include <limits>

#include "binarization.h"

#define DESKEW 1
#define S 1.0

using namespace std;
using namespace cv;

double compute_skew(Mat src);
Mat deskew(Mat img, double angle, Mat &orig);
void fillBorder(Mat &img);
Mat DImageToMat(const DImage& src);
DImage MatToDImage(const Mat& src);
Mat translateImg(Mat &img, int offsetx, int offsety){
    Mat trans_mat = (Mat_<double>(2,3) << 1, 0, offsetx, 0, 1, offsety);
    warpAffine(img,img,trans_mat,img.size());
    return trans_mat;
}

int main (int argc, char** argv)
{

    Mat orig = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
    Mat orig2 = imread(argv[2], CV_LOAD_IMAGE_GRAYSCALE);
    //assert(orig.cols==orig2.cols && orig.rows==orig2.rows);
    double aspectRatio = orig.rows/(double)orig.cols;
    double aspectRatio2 = orig2.rows/(double)orig2.cols;
    if ((aspectRatio < 1 /*2-pages*/ && aspectRatio2 > 1 /*1-page*/) ||
        (aspectRatio2 < 1 /*2-pages*/ && aspectRatio > 1 /*1-page*/) )
    {
        cout << "Mismatched pages" << endl;
        return 1;
    }
    
    if (orig.rows > orig2.rows)
        resize(orig2, orig2, Size(round(orig2.cols*(orig.rows/(double)orig2.rows)), orig.rows), 0, 0, CV_INTER_CUBIC );
    else if (orig.rows < orig2.rows)
        resize(orig, orig, Size(round(orig.cols*(orig2.rows/(double)orig.rows)), orig2.rows), 0, 0, CV_INTER_CUBIC );
    
    Mat shrunk;
    resize(orig,shrunk,Size(0,0),1/S,1/S, CV_INTER_AREA);
    
    #if DESKEW
    Mat src = Binarization::ntirogiannisBinarization(shrunk,255,0);
    threshold( shrunk, src, 150, 255,0);
    
    //fillBorder(src);
    
    bitwise_not(src, src);
    double skew = compute_skew(src);
    Mat deskewed = deskew(src,skew,shrunk);
    #endif
    ////////////
    Mat shrunk2;
    resize(orig2,shrunk2,Size(0,0),1/S,1/S, CV_INTER_AREA);
    
    #if DESKEW
    Mat src2=Binarization::ntirogiannisBinarization(shrunk2,255,0);
    //threshold( shrunk2, src2, 150, 255,0);
    
    //fillBorder(src2);
    
    bitwise_not(src2, src2);
    double skew2 = compute_skew(src2);
    Mat deskewed2 = deskew(src2,skew2,shrunk2);
    #endif
    
    
    
    /////////////////
    Mat fImage = shrunk;
    Mat fImage2 = shrunk2;
    //Make the same size for Fourier stuff
    if(fImage.cols!=fImage2.cols || fImage.rows!=fImage2.rows)
    {
        if (fImage.cols<fImage2.cols)
        {
            int difx =  fImage2.cols-fImage.cols;
            if (fImage.rows<fImage2.rows)
            {
                int dify = fImage2.rows-fImage.rows;
                fImage2=fImage2(Rect(difx/2, dify/2, fImage2.cols-difx, fImage2.rows-dify));
            }
            else
            {
                fImage2=fImage2(Rect(difx/2, 0, fImage2.cols-difx, fImage2.rows));
                
                int dify = fImage.rows-fImage2.rows;
                fImage=fImage(Rect(0, dify/2, fImage.cols, fImage.rows-dify));
            }
            
            
        }
        else 
        {
            int difx =  fImage.cols-fImage2.cols;
            if (fImage.rows>fImage2.rows)
            {
                int dify = fImage.rows-fImage2.rows;
                fImage=fImage(Rect(difx/2, dify/2, fImage.cols-difx, fImage.rows-dify));
            }
            else
            {
                fImage=fImage(Rect(difx/2, 0, fImage.cols-difx, fImage.rows));
                
                int dify = fImage2.rows-fImage.rows;
                fImage2=fImage2(Rect(0, dify/2, fImage2.cols, fImage2.rows-dify));
            }
        }
    }
    
    
    fImage.convertTo(fImage, CV_32F);
    
    fImage2.convertTo(fImage2, CV_32F);
    
    Mat padded;                            //expand input image to optimal size
    int m = getOptimalDFTSize( fImage.rows );
    int n = getOptimalDFTSize( fImage.cols ); // on the border add zero values
    copyMakeBorder(fImage, padded, 0, m - fImage.rows, 0, n - fImage.cols, BORDER_CONSTANT, Scalar::all(0));
    Mat complexI;

    dft(fImage, complexI, cv::DFT_SCALE|cv::DFT_COMPLEX_OUTPUT);
    
    
    Mat padded2;                            //expand input image to optimal size
    copyMakeBorder(fImage2, padded2, 0, m - fImage2.rows, 0, n - fImage2.cols, BORDER_CONSTANT, Scalar::all(0));
    Mat complexI2;
    dft(fImage2, complexI2, cv::DFT_SCALE|cv::DFT_COMPLEX_OUTPUT);
    
    Mat resultF(complexI.size(),complexI.type());
    Mat result;
    mulSpectrums(complexI,complexI2,resultF,0,true);
    /*for (int i=0; i< resultF.cols; i++)
        for (int j=0; j< resultF.rows; j++)
        {
            float real = complexI.at<Vec2f>(j,i)[0]*complexI2.at<Vec2f>(j,i)[0] + complexI.at<Vec2f>(j,i)[1]*complexI2.at<Vec2f>(j,i)[1];
            float imag = complexI.at<Vec2f>(j,i)[1]*complexI2.at<Vec2f>(j,i)[0] + complexI.at<Vec2f>(j,i)[0]*complexI2.at<Vec2f>(j,i)[1];
            resultF.at<Vec2f>(j,i) = Vec2f(real,imag);
            cout << real <<  " + " << imag << "i" << endl;
        }*/
        
    ///////////////////////////////////////////////
    dft(resultF,result,cv::DFT_INVERSE|cv::DFT_REAL_OUTPUT);
    //dft(complexI2,result,cv::DFT_INVERSE|cv::DFT_REAL_OUTPUT);
    //split(result, planes); 
    
    float maxVal = 0;
    Point2i maxPoint;
    for (int i=0; i< result.cols; i++)
        for (int j=0; j< result.rows; j++)
        {
            if (result.at<float>(j,i) > maxVal)
            {
                maxVal = result.at<float>(j,i);
                maxPoint = Point2i(i,j);
            }
            //cout << result.at<float>(j,i) << endl;
        }
    
    /*DImage imgSrc = MatToDImage(orig);
    DImage imgAccum;
    DHough::houghImageCalcEdges_(imgAccum, imgSrc);
    
    unsigned int* data = imgAccum.dataPointer_u32();
    int max =0;
    int min = numeric_limits<int>::max();
    for (int i=0; i< imgAccum.height() * imgAccum.width(); i++)
    {
        int v =data[i];
        if (v<min)min=v;
        if(v>max)max=v;
    }
    Mat accumOut(imgAccum.height(),imgAccum.width(),CV_8U);
    unsigned char* data0 = accumOut.data;
    for (int i=0; i< imgAccum.height() * imgAccum.width(); i++)
    {
        data0[i]=255.0*(data[i]-min*1.0)/max;
    }*/
    
    //normalize(planes[0], planes[0], 0, 1, CV_MINMAX);
    //normalize(result, result, 0, 1, CV_MINMAX);
    Mat out;
    result.convertTo(out, CV_8U);
    cvtColor(out,out,CV_GRAY2RGB);
    out.at<Vec3b>(maxPoint) = Vec3b(0,0,255);
    cout << "max("<<maxPoint.x<<","<<maxPoint.y<<"): " << maxVal << endl;
    
    int shiftX = (maxPoint.x<=result.cols/2?maxPoint.x:maxPoint.x-result.cols);//*S
    int shiftY = (maxPoint.y<=result.rows/2?maxPoint.y:maxPoint.y-result.rows);//*S
    cout << "shift: " << shiftX <<", "<<shiftY<<endl;
    translateImg(shrunk2,shiftX,shiftY);
    
    
    imwrite("img1.png",orig);
    imwrite("img2.png",orig2);
    imwrite("conv.png",out);
    //imwrite("conv1.png",planes[1]);
    //imgSrc.save(argv[2],DImage::DFileFormat_png);
    //imgAccum.save(argv[3],DImage::DFileFormat_png);
    //imwrite(argv[3],accumOut);
    
    ////////////////////

    ///////////////////////////
    
    return 0;
}

void fillBorder(Mat &img)
{
    vector<Point2i> stack;
    Point2i start(1,1);
    img.at<unsigned char>(start) = 255;
    
    
    stack.push_back(start);
    
    
    while (!stack.empty())
    {
        Point2i p = stack.back();
        
//        int test = stack.size();
        stack.pop_back();
//        assert(test-1 == stack.size());
        
        assert (!(img.at<unsigned char>(p) <1));
        
        if (p.x>0 && img.at<unsigned char>(p.y,p.x-1) <1)
        {
            
            Point2i next(p.x-1,p.y);
            img.at<unsigned char>(next) = 255;
            assert (! (img.at<unsigned char>(next) <1));
            stack.push_back(next);
            
            
            
        }
        if (p.x<img.cols-1 && img.at<unsigned char>(p.y,p.x+1) <1)
        {
            Point2i next(p.x+1,p.y);
            img.at<unsigned char>(next) = 255;
            stack.push_back(next);
            
            assert (! (img.at<unsigned char>(next) <1));
        }
        if (p.y>0 && img.at<unsigned char>(p.y-1,p.x) <1)
        {
            Point2i next(p.x,p.y-1);
            img.at<unsigned char>(next) = 255;
            stack.push_back(next);
            
            assert (! (img.at<unsigned char>(next) <1));
        }
        if (p.y<img.rows-1 && img.at<unsigned char>(p.y+1,p.x) <1)
        {
            Point2i next(p.x,p.y+1);
            img.at<unsigned char>(next) = 255;
            stack.push_back(next);
            
            assert (! (img.at<unsigned char>(next) <1));
        }
        
        
//        if (stack.size()%100==0)
//        {
//            cout << "back:"<<stack.back()<< "  img[back]:" << (unsigned int)img.at<unsigned char>(stack.back())<<endl << "p:" << p  << " img[p]:" << (unsigned int)img.at<unsigned char>(p)<<endl;
//            cout << "size: "<<stack.size()<<endl;
            
//            imshow("threshed/filled...part", img);
//            waitKey(5);
//        }
    }
}

Mat DImageToMat(const DImage& src)
{
    Mat img(src.height(),src.width(),CV_8U);
    unsigned char* data1 = src.dataPointer_u8();
    unsigned char* data0 = img.data;
    for (int i=0; i< src.height() * src.width(); i++)
    {
        data0[i]=data1[i];
    }
    return img;
}
DImage MatToDImage(const Mat& src)
{
    DImage img;
    img.setLogicalSize(src.cols,src.rows);
    unsigned char* data1 = img.dataPointer_u8();
    unsigned char* data0 = src.data;
    for (int i=0; i< src.cols * src.rows; i++)
    {
        data1[i]=data0[i];
    }
    return img;
}

//http://felix.abecassis.me/2011/09/opencv-detect-skew-angle/
double compute_skew(Mat src)
{
    DImage img = MatToDImage(src);
    return DGlobalSkew::getSkewAng_fast(img,-30,30,.2);
    
    ////////////////////////////////////////////////
    /*
    Size size = src.size();
    
    vector<Vec4i> lines;
    HoughLinesP(src, lines, 1, CV_PI/180, 100, size.width / 7.f, 20);
    Mat disp_lines(size, CV_8UC1, Scalar(0, 0, 0));
    double angle = 0.;
    unsigned nb_lines = lines.size();
    for (unsigned i = 0; i < nb_lines; ++i)
    {
       line(disp_lines, Point(lines[i][0], lines[i][1]),
                Point(lines[i][2], lines[i][3]), Scalar(255, 0 ,0));
       angle += atan2((double)lines[i][3] - lines[i][1],
                      (double)lines[i][2] - lines[i][0]);
    }
    angle /= nb_lines; // mean angle, in radians.
    
//    cout << "File " << filename << ": " << angle * 180 / CV_PI << endl;
    
//    imshow("lines", disp_lines);
//    waitKey(0);
//    destroyWindow("lines");
    return angle*180/CV_PI;
    */
}

//http://felix.abecassis.me/2011/10/opencv-rotation-deskewing/
Mat deskew(Mat img, double angle, Mat &orig)
{
    
    vector<Point> points;
    Mat_<uchar>::iterator it = img.begin<uchar>();
    Mat_<uchar>::iterator end = img.end<uchar>();
    for (; it != end; ++it)
        if (*it)
            points.push_back(it.pos());
    
    RotatedRect box = minAreaRect(Mat(points));
    Mat rot_mat = getRotationMatrix2D(box.center, angle, 1);
    Mat rotated;
    warpAffine(img, rotated, rot_mat, img.size(), INTER_CUBIC);
    warpAffine(orig, orig, rot_mat, img.size(), INTER_CUBIC);
    Size box_size = box.size;
    if (box.angle < -45.)
        swap(box_size.width, box_size.height);
    Mat cropped;
    getRectSubPix(rotated, box_size, box.center, cropped);
    
    
    //getRectSubPix(orig, box_size, box.center, orig);
//    imshow("Original", img);
//    imshow("Rotated", rotated);
//    imshow("Cropped", cropped);
//    waitKey(0);
    
    return cropped;
    
}
