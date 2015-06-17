#include "hog.h"

#define BIN_SIZE (180/num_bins)

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
//    Mat binned = bin(grad);
    
    //init bins
    int binsH=(img.cols-stepSize)/stepSize;
    int binsV=(img.rows-stepSize)/stepSize;
    
    vector< vector< vector<float> > > bins;
    bins.resize(binsH);
    for (int i=0; i<binsH; i++)
    {
//        int tlX = i*stepSize;
        
        bins.at(i).resize(binsV);
        for (int j=0; j<binsV; j++)
        {
//            int tlY = j*stepSize;
            bins.at(i).at(j).resize(num_bins);
        }
    }
    
    //compute the bins
    for (int i=0; i<binsH; i++)
    {
        int tlX = i*stepSize - (cellSize-stepSize)/2;
        for (int j=0; j<binsV; j++)
        {
            int tlY = j*stepSize - (cellSize-stepSize)/2;
            //each cell
            
            for (int x=tlX; x<tlX+cellSize; x++)
                for (int y=tlY; y<tlY+cellSize; y++)
                {
                    //trilinear interpolation?
//                    float horzDist = (x-tlX)-(.5+stepSize/2.0);
//                    int other_i;
//                    float other_iW;
//                    float my_iW;
//                    if (horzDist <0 && i>0)
//                    {
//                        other_i=i-1;
//                        my_iW=(horzDist*-1.0)/stepSize;
//                        other_iW=(stepSize+horzDist+0.0)/stepSize;
//                    }
//                    else if (horzDist >0 && i<(binsH)-1)
//                    {
//                        other_i=i+1;
//                        my_iW=(horzDist*1.0)/stepSize;
//                        other_iW=(stepSize-horzDist+0.0)/stepSize;
//                    }
//                    else
//                    {
//                        other_i=i;
//                        my_iW=1;
//                        other_iW=0;
//                    }
                    
//                    float vertDist = (y-tlY)-(.5+stepSize/2.0);
//                    int other_j;
//                    float other_jW;
//                    float my_jW;
//                    if (vertDist <0 && j>0)
//                    {
//                        other_j=j-1;
//                        my_jW=(vertDist*-1.0)/stepSize;
//                        other_jW=(stepSize+vertDist+0.0)/stepSize;
//                    }
//                    else if (vertDist >0 && j<(binsV)-1)
//                    {
//                        other_j=j+1;
//                        my_jW=(vertDist*1.0)/stepSize;
//                        other_jW=(stepSize-vertDist+0.0)/stepSize;
//                    }
//                    else
//                    {
//                        other_j=j;
//                        my_jW=1;
//                        other_jW=0;
//                    }
                    
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
                    int other_bin;
                    float my_binW;
                    float other_binW;
                    float angleDist = angle - (bin*BIN_SIZE+BIN_SIZE/ 2);
                    if (angleDist <0)
                    {
                        other_bin=mod(bin-1,num_bins);
                        my_binW=(angleDist*-1.0)/BIN_SIZE;
                        other_binW=(BIN_SIZE+angleDist+0.0)/BIN_SIZE;
                    }
                    else if (angleDist >0)
                    {
                        other_bin=(bin+1)%num_bins;
                        my_binW=(angleDist*1.0)/BIN_SIZE;
                        other_binW=(BIN_SIZE-angleDist+0.0)/BIN_SIZE;
                    }
                    else
                    {
                        other_bin=bin;
                        my_binW=1;
                        other_binW=0;
                    }
                    
                    float mag = sqrt(pow(grad.at<Vec2f>(y,x)[0],2) + pow(grad.at<Vec2f>(y,x)[1],2));
                    
//                    //add pixel to bins[i][j]
//                    bins[i][j][bin] += mag*my_iW*my_jW*my_binW;
//                    bins[other_i][j][bin] += mag*other_iW*my_jW*my_binW;
//                    bins[i][other_j][bin] += mag*my_iW*other_jW*my_binW;
//                    bins.at(other_i).at(other_j).at(bin) += mag*other_iW*other_jW*my_binW;
                    
//                    bins[i][j][other_bin] += mag*my_iW*my_jW*other_binW;
//                    bins[other_i][j][other_bin] += mag*other_iW*my_jW*other_binW;
//                    bins[i][other_j][other_bin] += mag*my_iW*other_jW*other_binW;
//                    bins.at(other_i).at(other_j).at(other_bin) += mag*other_iW*other_jW*other_binW;
                    
                    bins[i][j][bin] += mag*my_binW;
                    bins[i][j][other_bin] += mag*other_binW;
                }
        }
    }
    
    //filter and feature points to return objects
    for (int i=0; i<binsH; i++)
    {
        int tlX = i*stepSize - (cellSize-stepSize)/2;
        for (int j=0; j<binsV; j++)
        {
            int tlY = j*stepSize - (cellSize-stepSize)/2;
            
            float mag=0;
            for (int f=0; f<num_bins; f++)
                mag += pow(bins[i][j][f],2);
            mag = sqrt(mag);
            if (mag>thresh)
            {
                descriptors.push_back(bins[i][j]);
                locations.push_back(Point2i(tlX+cellSize/2,tlY+cellSize/2));
            }
        }
    }
            
}


Mat HOG::computeGradient(const Mat &img)
{
    Mat h = (Mat_<double>(1,3) << -1, 0, 1);
    Mat v = (Mat_<double>(3,1) << -1, 0, 1);
    
    Mat h_grad;
    filter2D(img,h_grad,CV_32F,h);
    
    Mat v_grad;
    filter2D(img,v_grad,CV_32F,v);
    h_grad=cv::abs(h_grad);
    v_grad=cv::abs(v_grad);
    
    Mat chan[2] = {h_grad, v_grad};
    Mat ret;
    merge(chan,2,ret);
    return ret;
}

