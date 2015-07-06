
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "dimage.h"
#include "dglobalskew.h"
#include "dtextlineseparator.h"
#include "dslantangle.h"

#include <vector>
#include <assert.h>
#include <iostream>
#include <string>

//#include "nameseparation/bimage.h"
//#include "nameseparation/wordseparator.h"
//#include "nameseparation/boxcleaner.h"
//#include <QVector>

double compute_skew(cv::Mat src);
cv::Mat deskew(cv::Mat img, double angle, cv::Mat &orig);
void fillBorder(cv::Mat &img);

int main (int argc, char** argv)
{
    cv::Mat orig = cv::imread(argv[1], 0);
    cv::Mat src;
    cv::threshold( orig, src, 150, 255,0);
    
    fillBorder(src);
    
    cv::bitwise_not(src, src);
    double skew = compute_skew(src);
    std::cout << "skew:"<<skew<<std::endl;
    cv::Mat deskewed = deskew(src,skew,orig);
    cv::imwrite("intermediate.pgm",deskewed);
    
//    cv::bitwise_not(deskewed,deskewed);
    
    
    
    
////    DImage img2("intermediate.pgm",DImage::DFileFormat_pgm);
    DImage img1;
    img1.setLogicalSize(orig.cols,orig.rows);
    unsigned char* data1 = img1.dataPointer_u8();
    unsigned char* dataO = orig.data;
    for (int i=0; i< orig.cols * orig.rows; i++)
    {
        data1[i]=dataO[i];
    }
    
    DImage img2;
    img2.setLogicalSize(deskewed.cols,deskewed.rows);
    unsigned char* data2 = img2.dataPointer_u8();
    unsigned char* data = deskewed.data;
    for (int i=0; i< deskewed.cols * deskewed.rows; i++)
    {
        data2[i]=data[i]>150?0:255;
    }
    
    int numTextLines;
    DRect* lines;
    DTextlineSeparator::getTextlineRects(img2,&numTextLines,&lines);
//    int avgHeight = DTextlineSeparator::estimateAvgHeight(img2);
    
    
//    BImage bimg(deskewed.cols,deskewed.rows);
//    for (int x=0; x<bimg.width(); x++)
//    {
//        for (int y=0; y<bimg.height(); y++)
//        {
//            bimg.setPixel(x,y,deskewed.at<unsigned char>(y,x)>0);
//        }
//    }
//    BImage cleared = BoxCleaner::trimVerticleBoundaries(bimg);
//    cleared = BoxCleaner::trimHorizontalBoundaries(cleared);
//    cleared = BoxCleaner::removeVerticlePixelNoise(cleared);
    
//    QVector<BPartition*> lines= WordSeparator::segmentLinesOfWords(cleared,avgHeight,false);
    
    cv::Mat out(deskewed.rows+1*numTextLines,deskewed.cols+30,0);
    out.setTo(0);
    int cur_y=0;
    for (int i=0; i<numTextLines; i++)
    {
        DImage line;
        line.setLogicalSize(lines[i].w,lines[i].h);
        line.pasteFromImage(0,0,img2,lines[i].x,lines[i].y,lines[i].w,lines[i].h);
        
        DImage rline;
        rline.setLogicalSize(lines[i].w,lines[i].h);
        rline.pasteFromImage(0,0,img1,lines[i].x,lines[i].y,lines[i].w,lines[i].h);
        
//        line.setLogicalSize(lines[i]->width(),lines[i]->height());
//        unsigned char* lineData = line.dataPointer_u8();
//        for (int x=0; x<lines[i]->width(); x++)
//        {
//            for (int y=0; y<lines[i]->height(); y++)
//            {
//                lineData[x+y*x] = lines[i]->pixel(x,y)?0:255;
//            }
//        }
        
        double slant = DSlantAngle::getTextlineSlantAngleDeg(line,5);
//        double slant = DSlantAngle::getTextlineSlantAngleDeg(img2,5,lines[i].x,lines[i].y,lines[i].w,lines[i].h);
//        double slant = DGlobalSkew::getSkewAng_var(img2);
        std::cout << "slant: " << slant << std::endl;
        rline = rline.shearedH(slant,0,true);
        unsigned char* data3 = rline.dataPointer_u8();
        
        
        //    img2.save(argv[2],DImage::DFileFormat_png,false,false);
//        assert(lines[i].w == deskewed.cols);
        for (int ii=0; ii< rline.width(); ii++)
            for (int jj=0; jj< rline.height(); jj++)
                out.data[ii+(jj+cur_y)*out.cols]=data3[ii+jj*rline.width()];
        cur_y+=rline.height();
    }
    cv::imwrite(argv[2],out);
    return 0;
}

void fillBorder(cv::Mat &img)
{
    std::vector<cv::Point2i> stack;
    cv::Point2i start(1,1);
    img.at<unsigned char>(start) = 255;
    
    
    stack.push_back(start);
    
    
    while (!stack.empty())
    {
        cv::Point2i p = stack.back();
        
//        int test = stack.size();
        stack.pop_back();
//        assert(test-1 == stack.size());
        
        assert (!(img.at<unsigned char>(p) <1));
        
        if (p.x>0 && img.at<unsigned char>(p.y,p.x-1) <1)
        {
            
            cv::Point2i next(p.x-1,p.y);
            img.at<unsigned char>(next) = 255;
            assert (! (img.at<unsigned char>(next) <1));
            stack.push_back(next);
            
            
            
        }
        if (p.x<img.cols-1 && img.at<unsigned char>(p.y,p.x+1) <1)
        {
            cv::Point2i next(p.x+1,p.y);
            img.at<unsigned char>(next) = 255;
            stack.push_back(next);
            
            assert (! (img.at<unsigned char>(next) <1));
        }
        if (p.y>0 && img.at<unsigned char>(p.y-1,p.x) <1)
        {
            cv::Point2i next(p.x,p.y-1);
            img.at<unsigned char>(next) = 255;
            stack.push_back(next);
            
            assert (! (img.at<unsigned char>(next) <1));
        }
        if (p.y<img.rows-1 && img.at<unsigned char>(p.y+1,p.x) <1)
        {
            cv::Point2i next(p.x,p.y+1);
            img.at<unsigned char>(next) = 255;
            stack.push_back(next);
            
            assert (! (img.at<unsigned char>(next) <1));
        }
        
        
//        if (stack.size()%100==0)
//        {
//            std::cout << "back:"<<stack.back()<< "  img[back]:" << (unsigned int)img.at<unsigned char>(stack.back())<<std::endl << "p:" << p  << " img[p]:" << (unsigned int)img.at<unsigned char>(p)<<std::endl;
//            std::cout << "size: "<<stack.size()<<std::endl;
            
//            cv::imshow("threshed/filled...part", img);
//            cv::waitKey(5);
//        }
    }
}

//http://felix.abecassis.me/2011/09/opencv-detect-skew-angle/
double compute_skew(cv::Mat src)
{
    
    
    cv::Size size = src.size();
    
    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(src, lines, 1, CV_PI/180, 100, size.width / 7.f, 20);
    cv::Mat disp_lines(size, CV_8UC1, cv::Scalar(0, 0, 0));
    double angle = 0.;
    unsigned nb_lines = lines.size();
    for (unsigned i = 0; i < nb_lines; ++i)
    {
       cv::line(disp_lines, cv::Point(lines[i][0], lines[i][1]),
                cv::Point(lines[i][2], lines[i][3]), cv::Scalar(255, 0 ,0));
       angle += atan2((double)lines[i][3] - lines[i][1],
                      (double)lines[i][2] - lines[i][0]);
    }
    angle /= nb_lines; // mean angle, in radians.
    
//    std::cout << "File " << filename << ": " << angle * 180 / CV_PI << std::endl;
    
//    cv::imshow("lines", disp_lines);
//    cv::waitKey(0);
//    cv::destroyWindow("lines");
    return angle*180/CV_PI;
}

//http://felix.abecassis.me/2011/10/opencv-rotation-deskewing/
cv::Mat deskew(cv::Mat img, double angle, cv::Mat &orig)
{
    
    std::vector<cv::Point> points;
    cv::Mat_<uchar>::iterator it = img.begin<uchar>();
    cv::Mat_<uchar>::iterator end = img.end<uchar>();
    for (; it != end; ++it)
        if (*it)
            points.push_back(it.pos());
    
    cv::RotatedRect box = cv::minAreaRect(cv::Mat(points));
    cv::Mat rot_mat = cv::getRotationMatrix2D(box.center, angle, 1);
    cv::Mat rotated;
    cv::warpAffine(img, rotated, rot_mat, img.size(), cv::INTER_CUBIC);
    cv::warpAffine(orig, orig, rot_mat, img.size(), cv::INTER_CUBIC);
    cv::Size box_size = box.size;
    if (box.angle < -45.)
        std::swap(box_size.width, box_size.height);
    cv::Mat cropped;
    cv::getRectSubPix(rotated, box_size, box.center, cropped);
    cv::getRectSubPix(orig, box_size, box.center, orig);
//    cv::imshow("Original", img);
//    cv::imshow("Rotated", rotated);
//    cv::imshow("Cropped", cropped);
//    cv::waitKey(0);
    
    return cropped;
}
