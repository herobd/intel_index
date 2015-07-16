#include "hog.h"

#include <omp.h>
#include <iostream>

#define BIN_SIZE (180/num_bins)
#define HOG_PAR 0
#define INTERPOLATE_LOC 1

HOG::HOG(float thresh, int cellSize, int stepSize, int num_bins)
{
    this->thresh=thresh;
    this->cellSize=cellSize;
    this->stepSize=stepSize;
    this->num_bins=num_bins;
}

int mod(int n, int m)
{
    while(1)
    {
        if (n>=0)
            return n%m;
        n+=m;
    }
    
}

void HOG::compute(const Mat &img, vector<vector<float> > &descriptors, vector< Point2i > &locations)
{
    Mat grad = computeGradient(img);
    
    //init bins
    int binsHorz=(img.cols-stepSize)/stepSize;
    int binsVert=(img.rows-stepSize)/stepSize;
    
    vector<float>** bins = new vector<float>*[binsHorz];
    for (int i=0; i<binsHorz; i++)
    {
        bins[i] = new vector<float>[binsVert];
        for (int j=0; j<binsVert; j++)
        {
            bins[i][j].resize(num_bins);
        }
    }
    
    //compute the bins  ,tlX,tlY,x,y,hGrad,vGrad,angle,bin,other_bin,my_binW,other_binW,mag
    int stepSize_par=stepSize;
    int cellSize_par=cellSize;
    //private(binsHorz,binsVert,cellSize_par,stepSize_par)
#pragma omp parallel for if (HOG_PAR)
    for (int i=0; i<binsHorz; i++)
    {
        int tlX = i*stepSize_par - (cellSize_par-stepSize_par)/2;
        for (int j=0; j<binsVert; j++)
        {
            int tlY = j*stepSize_par - (cellSize_par-stepSize_par)/2;
            //each cell
            
            for (int x=tlX; x<tlX+cellSize_par; x++)
                for (int y=tlY; y<tlY+cellSize_par; y++)
                {
                    //trilinear interpolation?
#if INTERPOLATE_LOC
                    float horzDist = (x-tlX)-(-.5+cellSize_par/2.0);
                    int other_i;
                    float other_iW;
                    float my_iW;
                    if (horzDist <0)
                    {
                        other_i=i-1;
                        other_iW=(horzDist*-1.0)/(cellSize_par/2.0);
                        my_iW=((cellSize_par/2.0)+horzDist+0.0)/(cellSize_par/2.0);
                    }
                    else if (horzDist >0)
                    {
                        other_i=i+1;
                        other_iW=(horzDist*1.0)/(cellSize_par/2.0);
                        my_iW=((cellSize_par/2.0)-horzDist+0.0)/(cellSize_par/2.0);
                    }
                    else
                    {
                        other_i=i;
                        my_iW=1;
                        other_iW=0;
                    }
                    
                    if (i<=0 || i>=(binsHorz)-1)
                    {
                        other_i=i;
                        other_iW=0;
                    }
                    
                    float vertDist = (y-tlY)-(-.5+cellSize_par/2.0);
                    int other_j;
                    float other_jW;
                    float my_jW;
                    if (vertDist <0)
                    {
                        other_j=j-1;
                        other_jW=(vertDist*-1.0)/(cellSize_par/2.0);
                        my_jW=((cellSize_par/2.0)+vertDist+0.0)/(cellSize_par/2.0);
                    }
                    else if (vertDist >0)
                    {
                        
                        other_j=j+1;
                        other_jW=(vertDist*1.0)/(cellSize_par/2.0);
                        
                        my_jW=((cellSize_par/2.0)-vertDist+0.0)/(cellSize_par/2.0);
                    }
                    else
                    {
                        other_j=j;
                        my_jW=1;
                        other_jW=0;
                    }
                    
                    if(j<=0 || j>=(binsVert)-1)
                    {
                        other_j=j;
                        other_jW=0;
                    }
#endif
                    
                    float hGrad=0;
                    float vGrad=0;
                    if (y>=0 && y<grad.rows && x>=0 && x<grad.cols)
                    {
                        hGrad=grad.at<Vec2f>(y,x)[0];
                        vGrad=grad.at<Vec2f>(y,x)[1];
                    }
                    
                    if (hGrad==0 && vGrad==0) continue;
                    
                    float angle = 180*atan2(vGrad,hGrad)/(CV_PI);
                    if (angle>=180) angle-=180;
                    if (angle<0) angle += 180;
                    int bin = angle/BIN_SIZE;
//                    cout << "angle: "<<angle<<", bin: "<<bin<<endl;
                    
                    int other_bin;
                    float my_binW;
                    float other_binW;
                    float angleDist = angle - (bin*BIN_SIZE+BIN_SIZE/ 2);
                    if (angleDist <0)
                    {
                        other_bin=mod(bin-1,num_bins);
                        other_binW=(angleDist*-1.0)/BIN_SIZE;
                        my_binW=(BIN_SIZE+angleDist+0.0)/BIN_SIZE;
                    }
                    else if (angleDist >0)
                    {
                        other_bin=(bin+1)%num_bins;
                        other_binW=(angleDist*1.0)/BIN_SIZE;
                        my_binW=(BIN_SIZE-angleDist+0.0)/BIN_SIZE;
                    }
                    else
                    {
                        other_bin=bin;
                        my_binW=1;
                        other_binW=0;
                    }
                    
                    float mag = sqrt(hGrad*hGrad + vGrad*vGrad);
                    
//                    //add pixel to bins[i][j]
#if INTERPOLATE_LOC
                    
#if HOG_PAR
                    float a= mag*my_iW*my_jW*my_binW;
                    float b= mag*other_iW*my_jW*my_binW;
                    float c= mag*my_iW*other_jW*my_binW;
                    float d= mag*other_iW*other_jW*my_binW;
                    
                    float e= mag*my_iW*my_jW*other_binW;
                    float f= mag*other_iW*my_jW*other_binW;
                    float g= mag*my_iW*other_jW*other_binW;
                    float h= mag*other_iW*other_jW*other_binW;
                    
#pragma omp critical
                    {
                    bins[i][j][bin] += a;
                    bins[other_i][j][bin] += b;
                    bins[i][other_j][bin] += c;
                    bins[other_i][other_j][bin] += d;
                    
                    bins[i][j][other_bin] += e;
                    bins[other_i][j][other_bin] += f;
                    bins[i][other_j][other_bin] += g;
                    bins[other_i][other_j][other_bin] += h;
                    }
#else
                    bins[i][j][bin] += mag*my_iW*my_jW*my_binW;
                    bins[other_i][j][bin] += mag*other_iW*my_jW*my_binW;
                    bins[i][other_j][bin] += mag*my_iW*other_jW*my_binW;
                    bins[other_i][other_j][bin] += mag*other_iW*other_jW*my_binW;
                    
                    bins[i][j][other_bin] += mag*my_iW*my_jW*other_binW;
                    bins[other_i][j][other_bin] += mag*other_iW*my_jW*other_binW;
                    bins[i][other_j][other_bin] += mag*my_iW*other_jW*other_binW;
                    bins[other_i][other_j][other_bin] += mag*other_iW*other_jW*other_binW;
#endif
                    
#else
                    bins[i][j][bin] += mag*my_binW;
                    bins[i][j][other_bin] += mag*other_binW;
#endif
                }
        }
    }
    
    vector<float> blank;
    
    //filter and feature points to return objects
    for (int i=0; i<binsHorz; i++)
    {
        int tlX = i*stepSize - (cellSize-stepSize)/2;
        for (int j=0; j<binsVert; j++)
        {
            int tlY = j*stepSize - (cellSize-stepSize)/2;
            
            float mag=0;
            for (int b=0; b<num_bins; b++)
                mag += pow(bins[i][j][b],2);
            mag = sqrt(mag);
            if (mag>thresh)
            {
                descriptors.push_back(bins[i][j]);
                locations.push_back(Point2i(tlX+cellSize/2,tlY+cellSize/2));
            }
            else
            {
                descriptors.push_back(blank);
                locations.push_back(Point2i(tlX+cellSize/2,tlY+cellSize/2));
            }
        }
    }
    
    
    for (int i=0; i<binsHorz; i++)
    {
        
        delete[] bins[i];
    }
    delete[] bins;
}


Mat HOG::computeGradient(const Mat &img)
{
    Mat h = (Mat_<double>(1,3) << -1, 0, 1);
    Mat v = (Mat_<double>(3,1) << -1, 0, 1);
    
    Mat h_grad;
    filter2D(img,h_grad,CV_32F,h);
    
    Mat v_grad;
    filter2D(img,v_grad,CV_32F,v);
//    h_grad=cv::abs(h_grad);
//    v_grad=cv::abs(v_grad);
    
    Mat chan[2] = {h_grad, v_grad};
    Mat ret;
    merge(chan,2,ret);
    return ret;
}

