#include "hog.h"

#include <omp.h>
#include <iostream>
#include "opencv2/highgui/highgui.hpp"

#define BIN_SIZE (180.0/num_bins)
#define HOG_PAR 0
#define INTERPOLATE_LOC 1

HOG::HOG(float thresh, int blockSize, int stepSize, int num_bins)
{
    this->thresh=thresh;
    this->blockSize=blockSize;
    this->cellSize=blockSize/2;
    this->stepSize=stepSize;
    this->num_bins=num_bins;
#if USE_CV_HOG
    assert(blockSize%2==0);
    hog = HOGDescriptor(Size(cellSize,cellSize), Size(cellSize,cellSize), Size(cellSize/2,cellSize/2), Size(cellSize/2,cellSize/2), num_bins);
#endif
}

void HOG::show(const Mat &img)
{
    vector<vector<float> > descriptors;
    vector< Point2i > locations;
    compute(img,descriptors,locations);
    Mat out = Mat::zeros(img.rows,img.cols,img.type());
    Mat color;
    cvtColor(out,color,CV_GRAY2BGR);
    cvtColor(color,color,CV_BGR2HSV);
    Mat color2 = color.clone();
    assert(descriptors.size()==locations.size());
    /*for (int i=0; i<locations.size(); i++)
    {
        for (int j=0; j<descriptors[i].size(); j++)
            cout <<descriptors.at(i).at(j)<<", ";
        cout <<endl;
    }*/
    for (int i=0; i<locations.size(); i++)
    {
        int top = max(0.0,locations[i].y-ceil(stepSize/2.0));
        int bot = min(out.rows-1.0,locations[i].y+floor(stepSize/2.0));
        int left = max(0.0,locations[i].x-ceil(stepSize/2.0));
        int right = min(out.cols-1.0,locations[i].x+floor(stepSize/2.0));
        map<float,float> slopes;
        float min=999999;
        float max=-999999;
        float maxAngle;
        float max2=-999999;
        float maxAngle2;
        for (int b=0; b<num_bins; b++)
        {
            float score = descriptors.at(i).at(b);
            slopes[score] = tan(b*BIN_SIZE+BIN_SIZE/2);
            if (descriptors[i][b] > max)
            {
                maxAngle = b*BIN_SIZE+BIN_SIZE/2;
                max = descriptors[i][b];
            }
            else if (descriptors[i][b] > max2)
            {
                maxAngle2 = b*BIN_SIZE+BIN_SIZE/2;
                max2 = descriptors[i][b];
            }
            if (descriptors[i][b] < min)
                min = descriptors[i][b];
        }
        for (auto p : slopes)
        {
            float slope = p.second;
            float y=locations[i].y;
            float x=locations[i].x;
            while (x>=left && x<=right && y>=top && y<=bot)
            {
                out.at<unsigned char>(y,x)=255*(p.first-min)/max;
                if (fabs(slope)>1.0)
                {
                    y+=1;
                    x+=1/slope;
                }
                else
                {
                    y+=slope;
                    x+=1;
                }
            }
            y=locations[i].y;
            x=locations[i].x;
            while (x>=left && x<=right && y>=top && y<=bot)
            {
                out.at<unsigned char>(y,x)=255*(p.first-min)/max;
                if (fabs(slope)>1.0)
                {
                    y-=1;
                    x-=1/slope;
                }
                else
                {
                    y-=slope;
                    x-=1;
                }
            }
        }

        //rectangle(color,Point(left,top),Point(right,bot),Scalar(255,255,maxAngle),CV_FILLED);
        for (int r=top; r<=bot; r++)
            for (int c=left; c<=right; c++)
            {
                color.at<Vec3b>(r,c) = Vec3b(maxAngle,255,255);
                color2.at<Vec3b>(r,c) = Vec3b(maxAngle2,255,255);
            }
    }
    cvtColor(color,color,CV_HSV2BGR);
    cvtColor(color2,color2,CV_HSV2BGR);
    imshow("orig",img);
    imshow("hog",out);
    imshow("color2",color2);
    imshow("color",color);
    waitKey();
}


void HOG::compute(const Mat &img, vector<vector<float> > &descriptors, vector< Point2i > &locations)
{
#if USE_CV_HOG
    vector<float> descs;
    vector<Point> locs;
    hog.compute (img, descs, Size(stepSize,stepSize), Size(), locs) ;
    int dsize = hog.getDescriptorSize();
    cout <<"dsize: "<<dsize<<endl;
    cout <<"descs: "<<descs.size()<<endl;
    cout <<"locs : "<<locs.size()<<endl;
    cout <<"rows : "<<img.rows<<endl;
    cout <<"cols : "<<img.cols<<endl;
    cout <<"expct: "<<(img.rows*img.cols*num_bins)/(cellSize*cellSize)<<endl;
    int cells_in_x_dir = img.cols / (cellSize/2);
    int cells_in_y_dir = img.rows / (cellSize/2);
    // nr of blocks = nr of cells - 1
    // since there is a new block on each cell (overlapping blocks!) but the last one
    int blocks_in_x_dir = cells_in_x_dir - 1;
    int blocks_in_y_dir = cells_in_y_dir - 1;
 
    // compute gradient strengths per cell
    int descriptorDataIdx = 0;
    //int cellx = 0;
    //int celly = 0;
 
    for (int blockx=0; blockx<blocks_in_x_dir; blockx++)
    {
        for (int blocky=0; blocky<blocks_in_y_dir; blocky++)
        {
            // 4 cells per block ...
            vector <float> descriptor(dsize);
            int dIndex=0;
            for (int cellNr=0; cellNr<4; cellNr++)
            {
                // compute corresponding cell nr
                /*cellx = blockx;
                celly = blocky;
                if (cellNr==1) celly++;
                if (cellNr==2) cellx++;
                if (cellNr==3)
                {
                    cellx++;
                    celly++;
                }*/
 
                for (int bin=0; bin<num_bins; bin++)
                {
                    float gradientStrength = descs[ descriptorDataIdx ];
                    descriptorDataIdx++;
 
                    //gradientStrengths[celly][cellx][bin] += gradientStrength;
                    descriptor.at(dIndex++) = gradientStrength;
                } // for (all bins)
 
                // note: overlapping blocks lead to multiple updates of this sum!
                // we therefore keep track how often a cell was updated,
                // to compute average gradient strengths
                //cellUpdateCounter[celly][cellx]++;
 
            } // for (all cells)
            assert(descriptor.size() == dsize);
            float mag=0;
            for (float v : descriptor)
            {
                mag += v*v;
            }
            mag = sqrt(mag);
            cout <<"mag ("<<blockx*stepSize+cellSize/2<<", "<<blocky*stepSize+cellSize/2<<") : "<<mag<<endl;
            if (mag>thresh)
            {
                descriptors.push_back(descriptor);
                //locations.push_back(Point2i(blockx*blockSize+blockSize/2, blocky*blockSize+blockSize/2));
                locations.push_back(Point2i(blockx*stepSize+cellSize/2, blocky*stepSize+cellSize/2));
            } 
 
        } // for (all block x pos)
    } // for (all block y pos)
#else
    Mat grad = computeGradient(img);
    
    //init bins
    int binsHorz=(img.cols-cellSize)/stepSize;
    int binsVert=(img.rows-cellSize)/stepSize;
    
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
    //private(binsHorz,binsVert,cellSize,stepSize)
    
    //we will iterate over each cell
#if HOG_PAR
#pragma omp parallel for if (HOG_PAR)
#endif
    for (int i=0; i<binsHorz; i++)
    {
        //top-left X of the cell
        int tlX = i*stepSize;// - (cellSize/*-stepSize*/)/2;// the minus part is just an arbitrary choosing of where we start cells from
        for (int j=0; j<binsVert; j++)
        {
            //top-left Y of the cell
            int tlY = j*stepSize;// - (cellSize/*-stepSize*/)/2;
            
            
            //iterate over each pixel in the cell
            for (int x=tlX; x<tlX+cellSize; x++)
                for (int y=tlY; y<tlY+cellSize; y++)
                {
                    //trilinear interpolation?
#if INTERPOLATE_LOC
                    float horzDist = (x-tlX)-(-.5+cellSize/2.0);
                    int other_i;
                    float other_iW;
                    float my_iW;
                    if (horzDist <0)
                    {
                        other_i=(tlX-cellSize)/stepSize;
                        other_iW=(horzDist*-1.0)/(cellSize);
                        my_iW=((cellSize)+horzDist+0.0)/(cellSize);
                    }
                    else if (horzDist >0)
                    {
                        other_i=(tlX+cellSize)/stepSize;
                        other_iW=(horzDist*1.0)/(cellSize);
                        my_iW=((cellSize)-horzDist+0.0)/(cellSize);
                    }
                    else
                    {
                        other_i=i;
                        my_iW=1;
                        other_iW=0;
                    }
                    
                    if (other_i<=0 || other_i>=(binsHorz)-1)
                    {
                        other_i=i;
                        other_iW=0;
                    }
                    
                    float vertDist = (y-tlY)-(-.5+cellSize/2.0);
                    int other_j;
                    float other_jW;
                    float my_jW;
                    if (vertDist <0)
                    {
                        other_j=(tlY-cellSize)/stepSize;
                        other_jW=(vertDist*-1.0)/(cellSize);
                        my_jW=((cellSize)+vertDist+0.0)/(cellSize);
                    }
                    else if (vertDist >0)
                    {
                        
                        
                        my_jW=((cellSize)-vertDist+0.0)/(cellSize);
                    }
                    else
                    {
                        other_j=j;
                        my_jW=1;
                        other_jW=0;
                    }
                    
                    if(other_j<=0 || other_j>=(binsVert)-1)
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
                    float angleDist = angle - (bin*BIN_SIZE +(BIN_SIZE/2));// the +(BIN_SIZE/2) is to center
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
                    bins[i][j].at(bin) += mag*my_iW*my_jW*my_binW;
                    bins[other_i][j].at(bin) += mag*other_iW*my_jW*my_binW;
                    bins[i][other_j].at(bin) += mag*my_iW*other_jW*my_binW;
                    bins[other_i][other_j].at(bin) += mag*other_iW*other_jW*my_binW;
                    
                    bins[i][j].at(other_bin) += mag*my_iW*my_jW*other_binW;
                    bins[other_i][j].at(other_bin) += mag*other_iW*my_jW*other_binW;
                    bins[i][other_j].at(other_bin) += mag*my_iW*other_jW*other_binW;
                    bins[other_i][other_j].at(other_bin) += mag*other_iW*other_jW*other_binW;
#endif
                    
#else
                    bins[i][j].at(bin) += mag*my_binW;
                    bins[i][j].at(other_bin) += mag*other_binW;
#endif
                }
        }
    }
    //vector<float> blank;
    
    //filter and feature points to return objects
    //for (int i=0; i<binsHorz; i++)
    for (int tlX=0; tlX<img.cols-blockSize; tlX+=stepSize)
    {
        //int tlX = i*stepSize - (cellSize/*-stepSize*/)/2;
        //for (int j=0; j<binsVert; j++)
        for (int tlY=0; tlY<img.rows-blockSize; tlY+=stepSize)
        {
            //int tlY = j*stepSize - (cellSize/*-stepSize*/)/2;
            
            float mag=0;
            vector<float> mm(num_bins);
            vector<float> desc(4*num_bins);
            int dIndex=0;
            for (int cellNr=0; cellNr<4; cellNr++)
            {
                // compute corresponding cell nr
                int i = tlX/stepSize;
                int j = tlY/stepSize;
                if (cellNr==1) j=(tlY+cellSize)/stepSize;
                if (cellNr==2) i=(tlX+cellSize)/stepSize;
                if (cellNr==3)
                {
                    j=(tlY+cellSize)/stepSize;
                    i=(tlX+cellSize)/stepSize;
                } 
                for (int b=0; b<num_bins; b++)
                {
                    assert(bins[i][j][b]>=0);
                    //mag += pow(bins[i][j][b],2);
                    mm[b] += bins[i][j][b];
                    desc.at(dIndex++)=bins[i][j][b];
                }
            }

            for (int b=0; b<num_bins; b++)
                mag += mm[b]*mm[b];
            mag = sqrt(mag);
            if (mag>thresh)
            {
                //vector<float> cc(bins[i][j]);
                descriptors.push_back(desc);
                int locX=tlX+cellSize;
                int locY=tlY+cellSize;
                assert(locX>=0 && locX<img.cols);
                assert(locY>=0 && locY<img.rows);
                locations.push_back(Point2i(locX,locY));
            }
//            else
//            {
//                descriptors.push_back(blank);
//                locations.push_back(Point2i(tlX+cellSize/2,tlY+cellSize/2));
//            }
        }
    }
    
    
    for (int i=0; i<binsHorz; i++)
    {
        
        delete[] bins[i];
    }
    delete[] bins;
#endif //USE_CV_HOG
}


Mat HOG::computeGradient(const Mat &img)
{
    Mat h = (Mat_<float>(1,3) << -1, 0, 1);
    Mat v = (Mat_<float>(3,1) << -1, 0, 1);
    
    Mat h_grad;
    filter2D(img,h_grad,CV_32F,h);
    
    Mat v_grad;
    filter2D(img,v_grad,CV_32F,v);
//    h_grad=cv::abs(h_grad);
//    v_grad=cv::abs(v_graad);
    /*Mat sv_grad = (v_grad/510)+.5;
    Mat sh_grad = (h_grad/510)+.5;
    imshow("h",sh_grad);
    imshow("v",sv_grad);
    waitKey();*/
    
    Mat chan[2] = {h_grad, v_grad};
    Mat ret;
    merge(chan,2,ret);
    return ret;
}

void HOG::unittest()
{
    Mat img1 = (Mat_<unsigned char>(5,5) << 0, 0, 0, 1, 1,
                                            0, 0, 0, 1, 1,
                                            0, 0, 0, 0, 0,
                                            1, 1, 0, 0, 0,
                                            1, 1, 0, 0, 0);
    Mat grad = computeGradient(img1);
    assert(grad.at<Vec2f>(0,1)[0]==0);
    assert(grad.at<Vec2f>(0,2)[0]==1);
    assert(grad.at<Vec2f>(0,3)[0]==1);
    assert(grad.at<Vec2f>(1,1)[0]==0);
    assert(grad.at<Vec2f>(1,2)[0]==1);
    assert(grad.at<Vec2f>(1,3)[0]==1);
    assert(grad.at<Vec2f>(2,1)[0]==0);
    assert(grad.at<Vec2f>(2,2)[0]==0);
    assert(grad.at<Vec2f>(2,3)[0]==0);
    assert(grad.at<Vec2f>(3,1)[0]==-1);
    assert(grad.at<Vec2f>(3,2)[0]==-1);
    assert(grad.at<Vec2f>(3,3)[0]==0);
    assert(grad.at<Vec2f>(4,1)[0]==-1);
    assert(grad.at<Vec2f>(4,2)[0]==-1);
    assert(grad.at<Vec2f>(4,3)[0]==0);
    
    assert(grad.at<Vec2f>(1,0)[1]==0);
    assert(grad.at<Vec2f>(2,0)[1]==1);
    assert(grad.at<Vec2f>(3,0)[1]==1);
    assert(grad.at<Vec2f>(1,1)[1]==0);
    assert(grad.at<Vec2f>(2,1)[1]==1);
    assert(grad.at<Vec2f>(3,1)[1]==1);
    assert(grad.at<Vec2f>(1,2)[1]==0);
    assert(grad.at<Vec2f>(2,2)[1]==0);
    assert(grad.at<Vec2f>(3,2)[1]==0);
    assert(grad.at<Vec2f>(1,3)[1]==-1);
    assert(grad.at<Vec2f>(2,3)[1]==-1);
    assert(grad.at<Vec2f>(3,3)[1]==0);
    assert(grad.at<Vec2f>(1,4)[1]==-1);
    assert(grad.at<Vec2f>(2,4)[1]==-1);
    assert(grad.at<Vec2f>(3,4)[1]==0);
    
    thresh=0;
    cellSize=5;
    stepSize=1;
    num_bins=9;
    
    Mat img2 = (Mat_<unsigned char>(20,20)<< 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0,
                                             0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0,
                                             0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0,
                                             0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                             1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
                                             1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0,
                                             1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0,
                                             1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
                                             1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    
    
    for (int x=0; x<img2.cols; x++)
        for (int y=0; y<img2.rows; y++)
        {
            if (img2.at<unsigned char>(y,x)==0)
                img2.at<unsigned char>(y,x)=255;
            else
                img2.at<unsigned char>(y,x)=0;
        }
    
    
    vector<vector<float> > desc;
    vector<Point2i> loc;
    compute(img2,desc,loc);
    for (int i=0; i<desc.size(); i++)
    {
        if ((loc[i].x == 2 && loc[i].y == 2) || (loc[i].x == 8 && loc[i].y == 8) || (loc[i].x == 2 && loc[i].y == 14))
        {
            assert(desc[i][0] > desc[i][1]);
            assert(desc[i][0] > desc[i][2]);
            assert(desc[i][0] > desc[i][3]);
            assert(desc[i][0] > desc[i][4]);
            assert(desc[i][0] > desc[i][5]);
            assert(desc[i][0] > desc[i][6]);
            assert(desc[i][0] > desc[i][7]);
        }
        else if ((loc[i].x == 8 && loc[i].y == 2) || (loc[i].x == 14 && loc[i].y == 8) || (loc[i].x == 8 && loc[i].y == 14))
        {
            assert(desc[i][6] > desc[i][0]);
            assert(desc[i][6] > desc[i][1]);
            assert(desc[i][6] > desc[i][2]);
            assert(desc[i][6] > desc[i][3]);
            assert(desc[i][6] > desc[i][4]);
            assert(desc[i][6] > desc[i][5]);
            assert(desc[i][6] > desc[i][7]);
            assert(desc[i][6] > desc[i][8]);
        }
        else if (loc[i].x == 14 && loc[i].y == 2)
        {
            assert(desc[i][4] > desc[i][0]);
            assert(desc[i][4] > desc[i][1]);
            assert(desc[i][4] > desc[i][2]);
            assert(desc[i][4] > desc[i][3]);
            assert(desc[i][4] > desc[i][5]);
            assert(desc[i][4] > desc[i][6]);
            assert(desc[i][4] > desc[i][7]);
            assert(desc[i][4] > desc[i][8]);
        }
        else if (loc[i].x == 2 && loc[i].y == 8)
        {
            assert(desc[i][2] > desc[i][0]);
            assert(desc[i][2] > desc[i][1]);
            assert(desc[i][2] > desc[i][3]);
            assert(desc[i][2] > desc[i][4]);
            assert(desc[i][2] > desc[i][5]);
            assert(desc[i][2] > desc[i][6]);
            assert(desc[i][2] > desc[i][7]);
            assert(desc[i][2] > desc[i][8]);
        }
    }
    
    /////////////////////////////////////////////////
    
    thresh=0;
    cellSize=5;
    stepSize=2;
    num_bins=9;
    
    desc.clear();
    loc.clear();
    compute(img2,desc,loc);
    for (int i=0; i<desc.size(); i++)
    {
        if ((loc[i].x == 2 && loc[i].y == 2) || (loc[i].x == 8 && loc[i].y == 8))
        {
            assert(desc[i][0] > desc[i][1]);
            assert(desc[i][0] > desc[i][2]);
            assert(desc[i][0] > desc[i][3]);
            assert(desc[i][0] > desc[i][4]);
            assert(desc[i][0] > desc[i][5]);
            assert(desc[i][0] > desc[i][6]);
            assert(desc[i][0] > desc[i][7]);
        }
        else if ((loc[i].x == 8 && loc[i].y == 2) || (loc[i].x == 14 && loc[i].y == 8))
        {
            assert(desc[i][6] > desc[i][0]);
            assert(desc[i][6] > desc[i][1]);
            assert(desc[i][6] > desc[i][2]);
            assert(desc[i][6] > desc[i][3]);
            assert(desc[i][6] > desc[i][4]);
            assert(desc[i][6] > desc[i][5]);
            assert(desc[i][6] > desc[i][7]);
            assert(desc[i][6] > desc[i][8]);
        }
        else if (loc[i].x == 14 && loc[i].y == 2)
        {
            assert(desc[i][4] > desc[i][0]);
            assert(desc[i][4] > desc[i][1]);
            assert(desc[i][4] > desc[i][2]);
            assert(desc[i][4] > desc[i][3]);
            assert(desc[i][4] > desc[i][5]);
            assert(desc[i][4] > desc[i][6]);
            assert(desc[i][4] > desc[i][7]);
            assert(desc[i][4] > desc[i][8]);
        }
        else if (loc[i].x == 2 && loc[i].y == 8)
        {
            assert(desc[i][2] > desc[i][0]);
            assert(desc[i][2] > desc[i][1]);
            assert(desc[i][2] > desc[i][3]);
            assert(desc[i][2] > desc[i][4]);
            assert(desc[i][2] > desc[i][5]);
            assert(desc[i][2] > desc[i][6]);
            assert(desc[i][2] > desc[i][7]);
            assert(desc[i][2] > desc[i][8]);
        }
    }
    
    
    thresh=0;
    cellSize=10;
    stepSize=2;
    num_bins=9;
    
    desc.clear();
    loc.clear();
    Mat el3 = imread("../../data/simple_corpus2/elements/3.png",CV_LOAD_IMAGE_GRAYSCALE);
    
//    Mat el3 = (Mat_<unsigned char>(10,10)<<  0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 
//                                             0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 
//                                             0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 
//                                             0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 
//                                             0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 
//                                             0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 
//                                             0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 
//                                             0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 
//                                             0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 
//                                             0, 0, 0, 0, 1, 1, 0, 0, 0, 0);
//    for (int x=0; x<el3.cols; x++)
//        for (int y=0; y<el3.rows; y++)
//        {
//            if (el3.at<unsigned char>(y,x)==0)
//                el3.at<unsigned char>(y,x)=255;
//            else
//                el3.at<unsigned char>(y,x)=0;
//        }
    imwrite("debug.png",el3);
    compute(el3,desc,loc);
    for (int i=0; i<desc.size(); i++)
    {
//        if ((loc[i].x == 2 && loc[i].y == 2) || (loc[i].x == 8 && loc[i].y == 8))
        if (desc[i].size()>0 && (loc[i].x > 2 && loc[i].y > 2) && (loc[i].x < 8 && loc[i].y < 8))
        {
            assert(desc[i][0] > desc[i][1]);
            assert(desc[i][0] > desc[i][2]);
            assert(desc[i][0] > desc[i][3]);
            assert(desc[i][0] > desc[i][4]);
            assert(desc[i][0] > desc[i][5]);
            assert(desc[i][0] > desc[i][6]);
            assert(desc[i][0] > desc[i][7]);
        }
    }
    
    
    cout << "HOG passed its tests!" << endl;
}
