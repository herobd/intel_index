#include "enhancedbovw.h"

#include <omp.h>
//#include "opencv2/gpu/gpu.hpp"
#include "kmeans_tbb.cpp"

#define SHOW_HEATMAP 0

EnhancedBoVW::EnhancedBoVW(vector<Vec2i> spatialPyramids, int desc_thresh, int LLC_numOfNN, int blockSize1, int blockSize2, int blockSize3, int blockStride, int hStride, int vStride, int skip)
{
    this->desc_thresh=desc_thresh;
    this->LLC_numOfNN=LLC_numOfNN;
    codebook=NULL;
    codebook_small=NULL;
    codebook_med=NULL;
    codebook_large=NULL;
    this->spatialPyramids = spatialPyramids;
//    spatialPyramids = {Vec2i(2,2),Vec2i(4,2)};


//    cout << "testing pyramid (2,2) (4,2) bigrams horz" << endl;
//    spatialPyramids = {Vec2i(1,1),Vec2i(2,2)};
//    spatialPyramids = {Vec2i(1,1)};
    
    this->hStride = hStride;
    this->vStride=vStride;
    this->skip=skip;
    
    this->blockSize1=blockSize1;
    this->blockSize2=blockSize2;
    this->blockSize3=blockSize3;
    this->blockStride=blockStride;
}

vector<float>* EnhancedBoVW::featurizeImage(const Mat &img) const
{
    auto samplesUncoded = getDescriptors(img);
    auto samplesCoded = codeDescriptorsIntegralImageSkip(samplesUncoded,img.size,skip);
    
    vector<float>* exe = getPooledDescFastSkip(samplesCoded, Rect(0,0,img.cols,img.rows),spatialPyramids,skip);
    normalizeDesc(exe);
    
    delete samplesCoded;
    return exe;
    
}

float EnhancedBoVW::scanImage(const Mat &img, const Mat &exemplar) const
{
    auto samplesUncoded = getDescriptors(exemplar);
    auto samplesCoded = codeDescriptorsIntegralImageSkip(samplesUncoded,exemplar.size,skip);
    
    vector<float>* exe = getPooledDescFastSkip(samplesCoded, Rect(0,0,exemplar.cols,exemplar.rows),spatialPyramids,skip);
    normalizeDesc(exe);
    float ret=scanImage(img,*exe,exemplar.size());
    delete exe;
    delete samplesCoded;
    return ret;
}


float EnhancedBoVW::scanImage(const Mat &img, const vector<float> &exemplar, Size exemplarSize) const
{
    
    int windowWidth=exemplarSize.width*1.2;
    int windowHeight=exemplarSize.height*1.2;
    int windowWidth2=exemplarSize.width;
    int windowHeight2=exemplarSize.height;
    int windowWidth3=exemplarSize.width*.8;
    int windowHeight3=exemplarSize.height*.8;
    
    Mat scores(img.rows/vStride, img.cols/hStride, CV_32FC3);
    float maxScore=0;
    float minScore=9999;
    
    auto samplesUncoded = getDescriptors(img);
    auto samplesCodedII = codeDescriptorsIntegralImageSkip(samplesUncoded,img.size,skip);
    
    
    
    for (int x=windowWidth3/3; x<img.cols-windowWidth3/3; x+=hStride)
        for (int y=windowHeight3/3; y<img.rows-windowHeight3/3; y+=vStride)
        {
            
            Rect r1(x-windowWidth/2, y-windowHeight/2, windowWidth, windowHeight);
            Rect r2(x-windowWidth2/2, y-windowHeight2/2, windowWidth2, windowHeight2);
            Rect r3(x-windowWidth3/2, y-windowHeight3/2, windowWidth3, windowHeight3);
            
            
            vector<float>* desc1 = getPooledDescFastSkip(samplesCodedII, r1, spatialPyramids,skip);
            normalizeDesc(desc1);
            vector<float>* desc2 = getPooledDescFastSkip(samplesCodedII, r2, spatialPyramids,skip);
            normalizeDesc(desc2);
            vector<float>* desc3 = getPooledDescFastSkip(samplesCodedII, r3, spatialPyramids,skip);
            normalizeDesc(desc3);
            
            
            
            float score1=0;
            float score2=0;
            float score3=0;
            
            for (int i=0; i<exemplar.size(); i++)
            {
                score1 += pow(exemplar[i]-(*desc1)[i],2);
                score2 += pow(exemplar[i]-(*desc2)[i],2);
                score3 += pow(exemplar[i]-(*desc3)[i],2);
                
            }
            scores.at<Vec3f>(y/vStride, x/hStride) = Vec3f(score1,score2,score3);
            
            
            if (score1<minScore) minScore=score1;
            if (score2<minScore) minScore=score2;
            if (score3<minScore) minScore=score3;
            
            if (score1>maxScore) maxScore=score1;
            if (score2>maxScore) maxScore=score2;
            if (score3>maxScore) maxScore=score3;
            
            delete desc1;
            delete desc2;
            delete desc3;
            
//            cout << "scores("<<x<<","<<y<<"): " << score1 << ", " << score2 << ", " << score3 << endl;
//            Mat tmp;
//            cvtColor(img,tmp,CV_GRAY2RGB);
//            rectangle(tmp,r2,Scalar(0,0,255));
//            rectangle(tmp,r1,Scalar(255,0,0));
//            rectangle(tmp,r3,Scalar(0,255,0));
//            imshow("sliding window",tmp);
//            waitKey(60);
        }
    
//    cout << "min score " << minScore << endl;
    
#if SHOW_HEATMAP
    ///Display heatmap of scores
    Mat heatmap;
    cvtColor(img,heatmap,CV_GRAY2BGR);
    cvtColor(heatmap,heatmap,CV_BGR2HSV);
    
    maxScore=min(maxScore,1.f);
    maxScore*=.25;
    
    for (int x=windowWidth3/3; x<img.cols-windowWidth3/3; x+=hStride)
        for (int y=windowHeight3/3; y<img.rows-windowHeight3/3; y+=vStride)
        {
            if (scores.at<Vec3f>(y/vStride, x/hStride)[0] < 
                    min(scores.at<Vec3f>(y/vStride, x/hStride)[1],scores.at<Vec3f>(y/vStride, x/hStride)[2]))
            {
                for (int xoff=-3; xoff<=4; xoff++)
                    for (int yoff=-3; yoff<=4; yoff++)
                    {
                        color(heatmap, (scores.at<Vec3f>(y/vStride,x/hStride))[0], maxScore, minScore,  x+xoff, y+yoff);
                    }
            }
            else if (scores.at<Vec3f>(y/vStride, x/hStride)[1] < scores.at<Vec3f>(y/vStride, x/hStride)[2])
            {
                for (int xoff=-2; xoff<=3; xoff++)
                    for (int yoff=-2; yoff<=3; yoff++)
                    {
                        color(heatmap,scores.at<Vec3f>(y/vStride, x/hStride)[1],maxScore, minScore,x+xoff,y+yoff);
                    }
            }
            else
            {
//                color(heatmap,scores.at<Vec3f>(y/vStride, x/hStride)[2],maxScore,x,y);
//                color(heatmap,scores.at<Vec3f>(y/vStride, x/hStride)[2],maxScore,x+1,y);
//                color(heatmap,scores.at<Vec3f>(y/vStride, x/hStride)[2],maxScore,x-1,y);
//                color(heatmap,scores.at<Vec3f>(y/vStride, x/hStride)[2],maxScore,x,y+1);
//                color(heatmap,scores.at<Vec3f>(y/vStride, x/hStride)[2],maxScore,x,y-1);
                for (int xoff=-1; xoff<=2; xoff++)
                    for (int yoff=-1; yoff<=2; yoff++)
                    {
                        color(heatmap,scores.at<Vec3f>(y/vStride, x/hStride)[1],maxScore, minScore,x+xoff,y+yoff);
                    }
            }
        }
    
    cvtColor(heatmap,heatmap,CV_HSV2BGR);
    imwrite("./heatmap.jpg",heatmap);
    imshow("heatmap",heatmap);
    
    waitKey();
#endif
    
    delete samplesCodedII;
    return minScore;
}



float EnhancedBoVW::scanImageHorz(const Mat &img, const Mat &exemplar) const
{
    auto samplesUncoded = getDescriptors(exemplar);
    auto samplesCoded = codeDescriptorsIntegralImageSkip(samplesUncoded,exemplar.size,skip);
    
    vector<float>* exe = getPooledDescFastSkip(samplesCoded, Rect(0,0,exemplar.cols,exemplar.rows),spatialPyramids,skip);
    normalizeDesc(exe);
    float ret=scanImageHorz(img,*exe,exemplar.size());
    delete exe;
    
    delete samplesCoded;
    return ret;
}


float EnhancedBoVW::scanImageHorz(const Mat &img, const vector<float> &exemplar, Size exemplarSize) const
{
    
    int windowWidth=exemplarSize.width*1.2;
//    int windowHeight=exemplarSize.height*1.2;
    int windowWidth2=exemplarSize.width;
//    int windowHeight2=exemplarSize.height;
    int windowWidth3=exemplarSize.width*.8;
//    int windowHeight3=exemplarSize.height*.8;
    
#if SHOW_HEATMAP
    Mat scores(img.rows/vStride, img.cols/hStride, CV_32FC3);
    float maxScore=0;
#endif
    
    float minScore=9999;
    
    auto samplesUncoded = getDescriptors(img);
    auto samplesCodedII = codeDescriptorsIntegralImageSkip(samplesUncoded,img.size,skip);
    
    
    
    for (int x=windowWidth3/3; x<img.cols-windowWidth3/3; x+=hStride)
    {
        
        Rect r1(x-windowWidth/2, 0, windowWidth, img.rows);
        Rect r2(x-windowWidth2/2, 0, windowWidth2, img.rows);
        Rect r3(x-windowWidth3/2, 0, windowWidth3, img.rows);
        
        
        vector<float>* desc1 = getPooledDescFastSkip(samplesCodedII, r1, spatialPyramids,skip);
        normalizeDesc(desc1);
        vector<float>* desc2 = getPooledDescFastSkip(samplesCodedII, r2, spatialPyramids,skip);
        normalizeDesc(desc2);
        vector<float>* desc3 = getPooledDescFastSkip(samplesCodedII, r3, spatialPyramids,skip);
        normalizeDesc(desc3);
        
        
        
        float score1=0;
        float score2=0;
        float score3=0;
        
        for (int i=0; i<exemplar.size(); i++)
        {
            score1 += pow(exemplar[i]-(*desc1)[i],2);
            score2 += pow(exemplar[i]-(*desc2)[i],2);
            score3 += pow(exemplar[i]-(*desc3)[i],2);
            
        }
#if SHOW_HEATMAP
        scores.at<Vec3f>(0, x/hStride) = Vec3f(score1,score2,score3);
        if (score1>maxScore) maxScore=score1;
        if (score2>maxScore) maxScore=score2;
        if (score3>maxScore) maxScore=score3;
#endif
        
        if (score1<minScore) minScore=score1;
        if (score2<minScore) minScore=score2;
        if (score3<minScore) minScore=score3;
        
        
        
        delete desc1;
        delete desc2;
        delete desc3;
        
//            cout << "scores("<<x<<","<<y<<"): " << score1 << ", " << score2 << ", " << score3 << endl;
//            Mat tmp;
//            cvtColor(img,tmp,CV_GRAY2RGB);
//            rectangle(tmp,r2,Scalar(0,0,255));
//            rectangle(tmp,r1,Scalar(255,0,0));
//            rectangle(tmp,r3,Scalar(0,255,0));
//            imshow("sliding window",tmp);
//            waitKey(60);
    }
    
//    cout << "min score " << minScore << endl;
    
#if SHOW_HEATMAP
    ///Display heatmap of scores
    Mat heatmap;
    cvtColor(img,heatmap,CV_GRAY2BGR);
    cvtColor(heatmap,heatmap,CV_BGR2HSV);
    
    maxScore=min(maxScore,1.f);
    maxScore*=.25;
    
    for (int x=windowWidth3/3; x<img.cols-windowWidth3/3; x+=hStride)
    {
        if (scores.at<Vec3f>(0, x/hStride)[0] < 
                min(scores.at<Vec3f>(0, x/hStride)[1],scores.at<Vec3f>(0, x/hStride)[2]))
        {
            for (int xoff=-3; xoff<=4; xoff++)
                for (int yoff=-3; yoff<=4; yoff++)
                {
                    color(heatmap, (scores.at<Vec3f>(0,x/hStride))[0], maxScore, minScore,  x+xoff, heatmap.rows/2+yoff);
                }
        }
        else if (scores.at<Vec3f>(0, x/hStride)[1] < scores.at<Vec3f>(0, x/hStride)[2])
        {
            for (int xoff=-2; xoff<=3; xoff++)
                for (int yoff=-2; yoff<=3; yoff++)
                {
                    color(heatmap,scores.at<Vec3f>(0, x/hStride)[1],maxScore, minScore,x+xoff,heatmap.rows/2+yoff);
                }
        }
        else
        {
//                color(heatmap,scores.at<Vec3f>(y/vStride, x/hStride)[2],maxScore,x,y);
//                color(heatmap,scores.at<Vec3f>(y/vStride, x/hStride)[2],maxScore,x+1,y);
//                color(heatmap,scores.at<Vec3f>(y/vStride, x/hStride)[2],maxScore,x-1,y);
//                color(heatmap,scores.at<Vec3f>(y/vStride, x/hStride)[2],maxScore,x,y+1);
//                color(heatmap,scores.at<Vec3f>(y/vStride, x/hStride)[2],maxScore,x,y-1);
            for (int xoff=-1; xoff<=2; xoff++)
                for (int yoff=-1; yoff<=2; yoff++)
                {
                    color(heatmap,scores.at<Vec3f>(0, x/hStride)[1],maxScore, minScore,x+xoff,heatmap.rows/2+yoff);
                }
        }
    }
    
    cvtColor(heatmap,heatmap,CV_HSV2BGR);
    imwrite("./heatmap.jpg",heatmap);
    imshow("heatmap",heatmap);
    
    waitKey();
#endif
    
    delete samplesCodedII;
    return minScore;
}

float EnhancedBoVW::compareImage(const Mat &img, const vector<float> &exemplar) const
{
    auto samplesUncoded = getDescriptors(img);
    auto samplesCodedII = codeDescriptorsIntegralImageSkip(samplesUncoded,img.size,skip);
    Rect r1(0, 0, img.cols, img.rows);
    vector<float>* desc1 = getPooledDescFastSkip(samplesCodedII, r1, spatialPyramids,skip);
    normalizeDesc(desc1);
    float score1=0;
    
    for (int i=0; i<exemplar.size(); i++)
    {
        score1 += pow(exemplar[i]-(*desc1)[i],2);
        //cout << exemplar[i]-(*desc1)[i] << " ";
    }
    //cout << endl;
    
    delete samplesCodedII;
    delete desc1;
    return score1;
}

void EnhancedBoVW::showEncoding(const Mat &img) const
{
    auto samplesUncoded = getDescriptors(img);
    for (auto desc : *samplesUncoded)
    {
        if (desc.scale==0 && desc.values.size()>0)
        {
            
            vector< tuple<int,float> > quan = codebook->quantizeSoft(desc.values,LLC_numOfNN);
            cout << "at (" << desc.position.x << "," << desc.position.y << "): " << get<0>(quan.front()) << endl;
//            for (int i=0; i<codebook->size(); i++)
//            {
//                bool f=false;
//                for (auto q : quan)
//                    if (get<0>(q)==i)
//                    {
//                        f=true;
//                        cout << get<1>(q) << ",\t";
//                    }
//                if (!f)
//                    cout << 0.0 << ",\t";
//            }
//            cout << endl;
            for (float f : desc.values)
                cout << f << ",\t";
            cout << endl;
        }
            
    }
}



vector< vector< Mat/*< float >*/ > >* EnhancedBoVW::codeDescriptorsIntegralImageSkip(vector< description >* desc, Mat::MSize imgsize, int skip) const
{
    auto lam = [&skip](const description& a, const description &b) -> bool
    {
         if (a.position.x/skip != b.position.x/skip) 
             return a.position.x/skip < b.position.x/skip;
         else
            return a.position.y/skip < b.position.y/skip;
    };



    sort(desc->begin(),desc->end(),lam);
    
    auto ret = new vector< vector< Mat/*< float >*/ > >(imgsize[1]/skip);
    auto iter = desc->begin();
    //first col
    (*ret)[0].resize(imgsize[0]/skip);                                                                                                                                                                                                                                                                                                                                                       
    for (int y=0; y<imgsize[0]/skip; y++)
    {
        
        if (y==0)
        {
            if (codebook != NULL)
                (*ret)[0][y] = Mat::zeros(codebook->size()*3,1,CV_32F);
            else
                (*ret)[0][y] = Mat::zeros(codebook_small->size() + 
                                          codebook_med->size() +
                                          codebook_large->size(),1,CV_32F);
        }
        else
            (*ret)[0][y] = (*ret)[0][y-1].clone();
        while (iter!=desc->end() && iter->position.y/skip==y && iter->position.x/skip==0)
        {
            vector< tuple<int,float> > quan = quantizeSoft(iter->values,LLC_numOfNN,iter->scale);
            for (const auto &v : quan)
            {
                if (codebook != NULL)
                    (*ret)[0][y].at<float>(get<0>(v)+codebook->size()*(iter->scale),0) += get<1>(v);
                else
                    (*ret)[0][y].at<float>(get<0>(v)+
                                           (iter->scale>0?codebook_small->size():0)+
                                           (iter->scale>1?codebook_med->size():0),0) += get<1>(v);
            }
//            int it = codebook->quantize(get<0>(*iter));
//            (*ret)[0][y].at<float>(it,0) += 1;
            
//            cout << "feature at (0,"<<y<<")"<<endl;
            
            if (++iter==desc->end())
                break;
        }
    }
    for (int x=1; x<imgsize[1]/skip; x++)
    {
        (*ret)[x].resize(imgsize[0]/skip);
        for (int y=0; y<imgsize[0]/skip; y++)
        {
            
            if (y==0)
                (*ret)[x][y] = (*ret)[x-1][y].clone();
            else
                (*ret)[x][y] = (*ret)[x][y-1]+(*ret)[x-1][y]-(*ret)[x-1][y-1];
            while (iter!=desc->end() && iter->position.y/skip==y && iter->position.x/skip==x)
            {
                vector< tuple<int,float> > quan = quantizeSoft(iter->values,LLC_numOfNN,iter->scale);
                for (const auto &v : quan)
                {
                    if (codebook != NULL)
                        (*ret)[x][y].at<float>(get<0>(v)+codebook->size()*iter->scale,0) += get<1>(v);
                    else
                        (*ret)[x][y].at<float>(get<0>(v)+
                                               (iter->scale>0?codebook_small->size():0)+
                                               (iter->scale>1?codebook_med->size():0),0) += get<1>(v);
                }
//                int it = codebook->quantize(get<0>(*iter));
//                (*ret)[x][y].at<float>(it,0) += 1;
                
//                cout << "feature at ("<<x<<","<<y<<")"<<endl;
                
                if (++iter==desc->end())
                    break;
            }
        }
    }
    
    
    delete desc;
    return ret;
}

void EnhancedBoVW::color(Mat &heatMap, float score, float maxV, float minV, int midI, int midJ) const
{
//    heatMap.at<Vec3b>(midJ,midI)[0] = heatMap.at<Vec3b>(midJ,midI)[0]*(165.0/255) + 90;
//    heatMap.at<Vec3b>(midJ,midI)[1] = heatMap.at<Vec3b>(midJ,midI)[1]*(165.0/255) + 90;
//    heatMap.at<Vec3b>(midJ,midI)[2] = heatMap.at<Vec3b>(midJ,midI)[2]*(165.0/255) + 90;
    
//    heatMap.at<Vec3b>(midJ,midI)[0] = heatMap.at<Vec3b>(midJ,midI)[0] * (1 - (score-min)/(max-min));
//    heatMap.at<Vec3b>(midJ,midI)[1] = heatMap.at<Vec3b>(midJ,midI)[1] * ((score-min)/(max-min));
//    heatMap.at<Vec3b>(midJ,midI)[2] = heatMap.at<Vec3b>(midJ,midI)[2] * (1 - (score-min)/(max-min));
    
    //HSV color space
    heatMap.at<Vec3b>(midJ,midI)[0] = (char) (150 * min((score-minV)/(maxV-minV),1.f));
    heatMap.at<Vec3b>(midJ,midI)[1] = 255;
    heatMap.at<Vec3b>(midJ,midI)[2] = heatMap.at<Vec3b>(midJ,midI)[2]*(165.0/255) + 90;
}


vector< description >* EnhancedBoVW::getDescriptors(const Mat &img) const
{   
//    Size blockSize1(16,16);
//    Size blockSize2(24,24);
//    Size blockSize3(32,32);
    
//    Size blockStride(4,4);
    
   
//    HOGDescriptor hog1(blockSize1,blockSize1,blockStride,blockSize1,9,1,-1,HOGDescriptor::L2Hys,.2,true,1);
//    HOGDescriptor hog2(blockSize2,blockSize2,blockStride,blockSize2,9,1,-1,HOGDescriptor::L2Hys,.2,true,1);
//    HOGDescriptor hog3(blockSize3,blockSize3,blockStride,blockSize3,9,1,-1,HOGDescriptor::L2Hys,.2,true,1);

    
    
//    vector<float> desc1;
//    hog1.compute(img,desc1,blockStride);
//    vector<float> desc2;
//    hog2.compute(img,desc2,blockStride);
//    vector<float> desc3;
//    hog3.compute(img,desc3,blockStride);
 
//    vector<vector<float> > descriptors1(1);
//    vector<vector<float> > descriptors2(1);
//    vector<vector<float> > descriptors3(1);
//    vector< Point2i > locations1;
//    vector< Point2i > locations2;
//    vector< Point2i > locations3;
    
//    filterDesc(desc1,descriptors1,locations1,hog1.getDescriptorSize(), blockSize1, blockStride, img.size());
//    filterDesc(desc2,descriptors2,locations2,hog2.getDescriptorSize(), blockSize2, blockStride, img.size());
//    filterDesc(desc3,descriptors3,locations3,hog3.getDescriptorSize(), blockSize3, blockStride, img.size());
    
//    vector< tuple< vector< float >, Point2i > > *descAndLoc = new vector< tuple< vector< float >, Point2i > >();
//    for (int i=0; i< descriptors1.size(); i++)
//    {
//        if (descriptors1[i].size() > 0)
//            descAndLoc->push_back(make_tuple(descriptors1[i],locations1[i]));
//    }
//    for (int i=0; i< descriptors2.size(); i++)
//    {
//        if (descriptors2[i].size() > 0)
//            descAndLoc->push_back(make_tuple(descriptors2[i],locations2[i]));
//    }
//    for (int i=0; i< descriptors3.size(); i++)
//    {
//        if (descriptors3[i].size() > 0)
//            descAndLoc->push_back(make_tuple(descriptors3[i],locations3[i]));
//    }
    
//    int blockSize1=16;
//    int blockSize2=24;
//    int blockSize3=32;
//    int blockStride=3;
    

    
    
    HOG hog1(desc_thresh,blockSize1,blockStride);
    HOG hog2(desc_thresh,blockSize2,blockStride);
    HOG hog3(desc_thresh,blockSize3,blockStride);
    vector<vector<float> > descriptors1;
    vector<vector<float> > descriptors2;
    vector<vector<float> > descriptors3;
    vector< Point2i > locations1;
    vector< Point2i > locations2;
    vector< Point2i > locations3;
    hog1.compute(img,descriptors1,locations1);
    hog2.compute(img,descriptors2,locations2);
    hog3.compute(img,descriptors3,locations3);
    
    vector< description > *descAndLoc = new vector< description >();
    

    
    for (int i=0; i< descriptors1.size(); i++)
    {
        descAndLoc->push_back(description(descriptors1[i],locations1[i],SMALL));
    }
    for (int i=0; i< descriptors2.size(); i++)
    {
        descAndLoc->push_back(description(descriptors2[i],locations2[i],MED));
    }
    for (int i=0; i< descriptors3.size(); i++)
    {
        descAndLoc->push_back(description(descriptors3[i],locations3[i],LARGE));
    }

    
    return descAndLoc;
}

void EnhancedBoVW::printDescThreshContours(const Mat &img, int desc_thresh) const
{   
    //based on Aldavert et al paper parameters
    //I'm trying to recreate their figure 1
    
    int blockSize1=16;
    int blockSize2=24;
    int blockSize3=32;
    int blockStride=1;
    

    
    
    HOG hog1(desc_thresh,blockSize1,blockStride);
    HOG hog2(desc_thresh,blockSize2,blockStride);
    HOG hog3(desc_thresh,blockSize3,blockStride);
    vector<vector<float> > descriptors1;
    vector<vector<float> > descriptors2;
    vector<vector<float> > descriptors3;
    vector< Point2i > locations1;
    vector< Point2i > locations2;
    vector< Point2i > locations3;
    hog1.compute(img,descriptors1,locations1);
    hog2.compute(img,descriptors2,locations2);
    hog3.compute(img,descriptors3,locations3);
    
    vector< description > *descAndLoc = new vector< description >();
    
    

    
    for (int i=0; i< descriptors1.size(); i++)
    {
        descAndLoc->push_back(description(descriptors1[i],locations1[i],SMALL));
    }
    for (int i=0; i< descriptors2.size(); i++)
    {
        descAndLoc->push_back(description(descriptors2[i],locations2[i],MED));
    }
    for (int i=0; i< descriptors3.size(); i++)
    {
        descAndLoc->push_back(description(descriptors3[i],locations3[i],LARGE));
    }

    Mat heat;
    cvtColor(img,heat,CV_GRAY2RGB);
    heat.setTo(Vec3b(0,0,0));
    int i=0;
    double minNorm=999999;
    double maxNorm=0;
    for (vector<float> d : descriptors1)
    {
        double norm=0;
        for (float n : d)
        {
            norm += n*n;
        }
        norm = sqrt(norm);
        if (norm > maxNorm) maxNorm=norm;
        if (norm < minNorm) minNorm=norm;
    }
    for (int idx=0; idx<descriptors1.size(); idx++)
    {
        vector<float> d = descriptors1[idx];
        double norm=0;
        for (float n : d)
        {
            norm += n*n;
        }
        norm = sqrt(norm);
//        cout << norm << endl;
//        int x= (i%((img.cols-(blockSize1-blockStride))/blockStride))*blockStride + blockSize1/2-blockStride/2;
//        int y= (i/((img.cols-(blockSize1-blockStride))/blockStride))*blockStride + blockSize1/2-blockStride/2;
//        cout << x <<", "<<y<<endl;
        int x = locations1[idx].x;
        int y = locations1[idx].y;
        
        
        
        for (int xoff=-blockStride/2; xoff<=blockStride/2; xoff++)
            for (int yoff=-blockStride/2; yoff<=blockStride/2; yoff++)
            {
//                heat.at<Vec3b>(y+yoff,x+xoff) = Vec3b(
//                        img.at<unsigned char>(y+yoff,x+xoff)*(((maxNorm-minNorm)-max(norm-minNorm,0.0))/(maxNorm-minNorm)),
//                        img.at<unsigned char>(y+yoff,x+xoff)*(((maxNorm-minNorm)-max(norm-minNorm,0.0))/(maxNorm-minNorm)),
//                        img.at<unsigned char>(y+yoff,x+xoff)*((max(norm-minNorm,0.0))/(maxNorm-minNorm)));
                heat.at<Vec3b>(y+yoff,x+xoff) = Vec3b(
                        img.at<unsigned char>(y+yoff,x+xoff)*(((maxNorm-minNorm)-max(norm-minNorm,0.0))/(maxNorm-minNorm)),
                        img.at<unsigned char>(y+yoff,x+xoff)*(((maxNorm-minNorm)-max(norm-minNorm,0.0))/(maxNorm-minNorm)),
                        img.at<unsigned char>(y+yoff,x+xoff));
//                if (norm!=0)
//                    heat.at<Vec3b>(y+yoff,x+xoff) = Vec3b(
//                                0,
//                                0,
//                                img.at<unsigned char>(y+yoff,x+xoff));
            }
                
        i++;
    }
    cout << "there are " << descriptors1.size() << " small descriptors" << endl;
    imshow("heatfeatures1",heat);
    waitKey();
    
    cvtColor(img,heat,CV_GRAY2RGB);
    heat.setTo(Vec3b(0,0,0));
    
    i=0;
    minNorm=999999;
    maxNorm=0;
    for (vector<float> d : descriptors2)
    {
        double norm=0;
        for (float n : d)
        {
            norm += n*n;
        }
        norm = sqrt(norm);
        if (norm > maxNorm) maxNorm=norm;
        if (norm < minNorm) minNorm=norm;
    }
    for (int idx=0; idx<descriptors2.size(); idx++)
    {
        vector<float> d = descriptors2[idx];
        double norm=0;
        for (float n : d)
        {
            norm += n*n;
        }
        norm = sqrt(norm);
        int x = locations2[idx].x;
        int y = locations2[idx].y;
        
        for (int xoff=-blockStride/2; xoff<=blockStride/2; xoff++)
            for (int yoff=-blockStride/2; yoff<=blockStride/2; yoff++)
            {
                heat.at<Vec3b>(y+yoff,x+xoff) = Vec3b(
                        img.at<unsigned char>(y+yoff,x+xoff)*(((maxNorm-minNorm)-max(norm-minNorm,0.0))/(maxNorm-minNorm)),
                        img.at<unsigned char>(y+yoff,x+xoff)*(((maxNorm-minNorm)-max(norm-minNorm,0.0))/(maxNorm-minNorm)),
                        img.at<unsigned char>(y+yoff,x+xoff));
            }
                
        i++;
    }
    
    cout << "there are " << descriptors2.size() << " med descriptors" << endl;
    imshow("heatfeatures2",heat);
    waitKey();
    
    cvtColor(img,heat,CV_GRAY2RGB);
    heat.setTo(Vec3b(0,0,0));
    i=0;
    minNorm=999999;
    maxNorm=0;
    for (vector<float> d : descriptors3)
    {
        double norm=0;
        for (float n : d)
        {
            norm += n*n;
        }
        norm = sqrt(norm);
        if (norm > maxNorm) maxNorm=norm;
        if (norm < minNorm) minNorm=norm;
    }
    for (int idx=0; idx<descriptors3.size(); idx++)
    {
        vector<float> d = descriptors3[idx];
        double norm=0;
        for (float n : d)
        {
            norm += n*n;
        }
        norm = sqrt(norm);
        int x = locations3[idx].x;
        int y = locations3[idx].y;
        
        for (int xoff=-blockStride/2; xoff<=blockStride/2; xoff++)
            for (int yoff=-blockStride/2; yoff<=blockStride/2; yoff++)
            {
                heat.at<Vec3b>(y+yoff,x+xoff) = Vec3b(
                        img.at<unsigned char>(y+yoff,x+xoff)*(((maxNorm-minNorm)-max(norm-minNorm,0.0))/(maxNorm-minNorm)),
                        img.at<unsigned char>(y+yoff,x+xoff)*(((maxNorm-minNorm)-max(norm-minNorm,0.0))/(maxNorm-minNorm)),
                        img.at<unsigned char>(y+yoff,x+xoff));
            }
                
        i++;
    }
    
    cout << "there are " << descriptors3.size() << " large descriptors" << endl;
    imshow("heatfeatures3",heat);
    waitKey();
}

void EnhancedBoVW::filterDesc(vector<float> &unparsedDescriptors, vector<vector<float> > &descriptors1, vector< Point2i > &locations, int descSize, Size blockSize1, Size blockStride, Size imgSize) const
{
//    cout << "desc size " << descSize<<endl;
    int i=0;
    int location=-1;
    double norm=0;
    double maxNorm = 0;
    double minNorm = 999999;
    for (float n : unparsedDescriptors)
    {
        
        
        
        if ((i++)%descSize==0) {
            location++;
            descriptors1.resize(location+1);
            
//            int x= (location/((imgSize.width-(blockSize1.width-blockStride.width))/blockStride.width))*blockStride.width + blockSize1.width/2-blockStride.width/2;
//            int y= (location%((imgSize.height-(blockSize1.width-blockStride.width))/blockStride.width))*blockStride.height + blockSize1.height/2-blockStride.height/2;
            
            int x= (location/(1+(imgSize.height-blockSize1.height)/blockStride.height)) * blockStride.width+blockSize1.width/2;
            int y= (location%(1+(imgSize.height-blockSize1.height)/blockStride.height)) * blockStride.height+blockSize1.height/2;
            
//            int x= (location%(1+(imgSize.width-blockSize1.width)/blockStride.width)) * blockStride.width+blockSize1.width/2;
//            int y= (location/(1+(imgSize.width-blockSize1.width)/blockStride.width)) * blockStride.height+blockSize1.height/2;
            
            assert(x<imgSize.width && y<imgSize.height);
            locations.push_back(Point2i(x,y));
            
            if (location!=0)
            {
//                cout << "norm: "<<sqrt(norm)<<endl;
                if (norm > desc_thresh)
                {
                    descriptors1[location-1].resize(0);
                }
                if (norm > maxNorm) maxNorm = norm;
                if (norm < minNorm/* && norm!=0*/) minNorm = norm;
                
                
            }
            
            norm=0;
        }
        descriptors1[location].push_back(n);
        norm += n*n;
    }
}

vector< tuple<int,float> > EnhancedBoVW::quantizeSoft(const vector<float> &term, int t, int scale) const
{
    if (codebook != NULL)
    {
        return codebook->quantizeSoft(term,t);
    }
    else if(scale == SMALL && codebook_small!=NULL)
    {
        return codebook_small->quantizeSoft(term,t);
    }
    else if(scale == MED && codebook_med!=NULL)
    {
        return codebook_med->quantizeSoft(term,t);
    }
    else if(scale == LARGE && codebook_large!=NULL)
    {
        return codebook_large->quantizeSoft(term,t);
    }
    return vector< tuple<int,float> >();
}

Codebook* EnhancedBoVW::makeCodebook(string directory, int codebook_size)
{
    vector< vector<float> > accum;
    
    
    
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (directory.c_str())) != NULL)
    {
      cout << "reading images and obtaining descriptors" << endl;
      
      vector<string> fileNames;
      while ((ent = readdir (dir)) != NULL) {
          string fileName(ent->d_name);
          if (fileName[0] == '.' || (fileName[fileName.size()-1]!='G' && fileName[fileName.size()-1]!='g' &&  fileName[fileName.size()-1]!='f'))
              continue;
          fileNames.push_back(fileName);
      }
      //private(fileName,img,desc,t)
      int loopCrit = min((int)5000,(int)fileNames.size());
#pragma omp parallel for 
      for (int nameIdx=0; nameIdx<loopCrit; nameIdx++)
      {
          
          
          string fileName=fileNames[nameIdx];
          //cout << "examining " << fileName << endl;
          
          
          
          Mat img = imread(directory+fileName, CV_LOAD_IMAGE_GRAYSCALE);
          
          auto desc = getDescriptors(img);
          
#pragma omp critical
          {
              for (auto t : *desc)
              {
                  //              assert(get<0>(t).size() > 0);
                  if (t.values.size() > 0)
                      accum.push_back(t.values);
              }
          }
          
          delete desc;
      }
      
      codebook = computeCodebookFromExamples(codebook_size,accum);
      
      return codebook;
    }
    else
        cout << "Error, could not load files for codebook." << endl;
    return NULL;
}

void EnhancedBoVW::make3Codebooks(string directory, int codebook_size)
{
    vector< vector<float> > accum_small;
    vector< vector<float> > accum_med;
    vector< vector<float> > accum_large;
    
    
    
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (directory.c_str())) != NULL)
    {
      cout << "training codebooks" << endl;
      cout << "reading images and obtaining descriptors" << endl;
      
      vector<string> fileNames;
      while ((ent = readdir (dir)) != NULL) {
          string fileName(ent->d_name);
          if (fileName[0] == '.' || (fileName[fileName.size()-1]!='G' && fileName[fileName.size()-1]!='g' &&  fileName[fileName.size()-1]!='f'))
              continue;
          fileNames.push_back(fileName);
      }
      //private(fileName,img,desc,t)
      int loopCrit = min((int)5000,(int)fileNames.size());
#pragma omp parallel for 
      for (int nameIdx=0; nameIdx<loopCrit; nameIdx++)
      {
          
          
          string fileName=fileNames[nameIdx];
          //cout << "examining " << fileName << endl;
          
          
          
          Mat img = imread(directory+fileName, CV_LOAD_IMAGE_GRAYSCALE);
          
          auto desc = getDescriptors(img);
          
#pragma omp critical
          {
              for (auto t : *desc)
              {
                  //              assert(get<0>(t).size() > 0);
                  if (t.values.size() > 0)
                  {
                      if (t.scale==SMALL)
                          accum_small.push_back(t.values);
                      else if (t.scale==MED)
                          accum_med.push_back(t.values);
                      else if (t.scale==LARGE)
                          accum_large.push_back(t.values);
                      
                  }
              }
          }
          
          delete desc;
      }

#pragma omp parallel num_threads(3)
{      
      int id = omp_get_thread_num();
      if (id==0)
          codebook_small = computeCodebookFromExamples(codebook_size,accum_small);
      else if (id==1)
          codebook_med = computeCodebookFromExamples(codebook_size,accum_med);
      else if (id==2)
          codebook_large = computeCodebookFromExamples(codebook_size,accum_large);
}      
    }
    else
        cout << "Error, could not load files for codebooks." << endl;
}

Codebook* EnhancedBoVW::computeCodebookFromExamples(int codebook_size,vector< vector<float> >& accum)
{
    Mat centriods;
    TermCriteria crit(0,500,.9);
    //      Mat data(accum.size(),accum[0].size(),CV_32F);
    //      for (int r=0; r< accum.size(); r++)
    //          for (int c=0; c<accum[0].size(); c++)
    //              data.at<float>(r,c) = accum[r][c];
    Mat data(codebook_size*300,accum[0].size(),CV_32F);
    Codebook* codebook;
    
    cout << "selecting random set" << endl;
    cout << "really. accum is " << accum.size() << endl;
    for (int count=0; count< codebook_size*300; count++)
    {
        int r=rand()%accum.size();
        int orig=r;
        while (accum[r].size()==0)
        {
            r = (1+r)%accum.size();
            if (r==orig)
            {
                cout << "ERROR: not enough descriptors" << endl;
                return NULL;
            }
        }
        
        
        for (int c=0; c<accum[0].size(); c++)
        {
            assert(accum[r][c] >= 0);
            data.at<float>(count,c) = accum[r][c];
        }
        accum[r].resize(0);
    }
    cout << "computing kmeans" << endl;
    
    Mat temp;
    //      Kmeans(data,codebook_size,temp,crit,10,KMEANS_RANDOM_CENTERS,&centriods);
    kmeans(data,codebook_size,temp,crit,10,KMEANS_RANDOM_CENTERS,centriods);
    
    cout << "compiling codebook" << endl;
    
    codebook = new Codebook();
    for (int r=0; r<centriods.rows; r++)
    {
        vector<double> toAdd;
        for (int c=0; c<centriods.cols; c++)
        {
            assert(centriods.at<float>(r,c) >= 0);
            toAdd.push_back(centriods.at<float>(r,c));
        }
        codebook->push_back(toAdd);
    }
    
    return codebook;
  
  
}

vector<float>* EnhancedBoVW::getPooledDescFastSkip(vector< vector< Mat/*< float >*/ > >* samplesIntegralImage, Rect window, vector<Vec2i> spatialPyramids, int skip, int level) const
{
    float compensation = spatialPyramids.at(0)[0]*spatialPyramids.at(0)[1];
    for (int l=1; l <= level; l++)
    {
        compensation *= (spatialPyramids.at(l)[0]/spatialPyramids.at(l-1)[0])*(spatialPyramids.at(l)[1]/spatialPyramids.at(l-1)[1]);
    }
    
    vector<float>* ret = new vector<float>();
    int binWidth = window.width/spatialPyramids.at(level)[0];
    int binHeight= window.height/spatialPyramids.at(level)[1];
    for (int binH=0; binH<spatialPyramids.at(level)[0]; binH++)
        for (int binV=0; binV<spatialPyramids.at(level)[1]; binV++)
        {
            Point2i recTL(max(0,(window.x + binH*binWidth)/skip),
                          max(0,(window.y + binV*binHeight)/skip));
            Point2i recBR(min((int)samplesIntegralImage->size()-1,(window.x + (binH+1)*binWidth )/skip -1), 
                          min((int)samplesIntegralImage->front().size()-1,(window.y + (binV+1)*binHeight )/skip -1));
            
//            vector<float> bins(codebook->size(),0);
//            for (int f=0; f<codebook->size(); f++)
//            {
//                bins[f] = samplesIntegralImage[recBR.x][recBR.y][f]
//            }
            Mat bins;
            
            //check window is not totally outside image
            if (recTL.x<samplesIntegralImage->size() && recTL.y < samplesIntegralImage->front().size() &&
                    recBR.x>=0 && recBR.y>=0)
            {
                if (recTL.x == 0 && recTL.y == 0)
                {
                    bins = (*samplesIntegralImage)[recBR.x][recBR.y];
                }
                else if (recTL.x == 0)
                {
                    bins = (*samplesIntegralImage)[recBR.x][recBR.y] -
                            (*samplesIntegralImage)[recBR.x][recTL.y-1];
                }
                else if (recTL.y == 0)
                {
                    bins = (*samplesIntegralImage)[recBR.x][recBR.y] - 
                            (*samplesIntegralImage)[recTL.x-1][recBR.y];
                }
                else
                {
                    bins = (*samplesIntegralImage)[recBR.x][recBR.y] + 
                            (*samplesIntegralImage)[recTL.x-1][recTL.y-1] -
                            (*samplesIntegralImage)[recBR.x][recTL.y-1] - 
                            (*samplesIntegralImage)[recTL.x-1][recBR.y];
                }
            }
            else
            {
                bins = Mat::zeros((*samplesIntegralImage)[0][0].size(),(*samplesIntegralImage)[0][0].type());
            }
            
            int oldSize=ret->size();
            ret->resize(oldSize+bins.size[0]);
            for (int i=0; i<bins.size[0]; i++)
            {
                (*ret)[oldSize+i] = compensation * (bins.at<float>(i,0));
            }
        }
    if (spatialPyramids.size()-1>level)
    {
        vector<float>* sub = getPooledDescFastSkip(samplesIntegralImage,window,spatialPyramids,skip,level+1);
        ret->insert(ret->end(),sub->begin(),sub->end());
        delete sub;
    }
    
    return ret;
}

void EnhancedBoVW::normalizeDesc(vector<float> *desc, float a) const
{
    double norm=0;
    for (int i=0; i<desc->size(); i++)
    {
        float val = (*desc)[i];
        val = copysign(val,1) * pow(fabs(val),a);
        (*desc)[i] = val;
        norm += val*val;
    }
    norm = sqrt(norm);
    if (norm!=0)
        for (int i=0; i<desc->size(); i++)
        {
            (*desc)[i] /= norm;
        }
//    for (int spatialBin=0; spatialBin < desc->size()/codebook->depth(); spatialBin++)
//    {
//        double norm=0;
//        for (int i=spatialBin*codebook->depth(); i<(1+spatialBin)*codebook->depth(); i++)
//        {
//            float val = (*desc)[i];
//            val = copysign(val,1) * pow(fabs(val),a);
//            (*desc)[i] = val;
//            norm += val*val;
//        }
//        norm = sqrt(norm);
//        if (norm!=0)
//            for (int i=spatialBin*codebook->depth(); i<(1+spatialBin)*codebook->depth(); i++)
//            {
//                (*desc)[i] /= norm;
//            }
        
//    }
}

void EnhancedBoVW::unittests()
{
    HOG hog(0,0);
    hog.unittest();
    
    Codebook cbTest;
    cbTest.unittest();
    
    codebook = new Codebook();
    ofstream file;
    file.open ("tmp.dat435", ios::out);
    
    file << 3 << endl;
    file << 3 << endl;
    
    file << 1 << endl;
    file << 0 << endl;
    file << 0 << endl;
    
    file << 0 << endl;
    file << 1 << endl;
    file << 0 << endl;
    
    file << 0 << endl;
    file << 0 << endl;
    file << 1 << endl;
    
    file.close();
    codebook->readIn("tmp.dat435");
    codebook->save("tmp.dat435");
    delete codebook;
    codebook = new Codebook();
    codebook->readIn("tmp.dat435");
    
    vector<float> term = {.5,0,.1};
    vector< tuple<int,float> > quan = codebook->quantizeSoft(term,1);
    assert(get<0>(quan[0])==0 && get<1>(quan[0])==1);
    
    vector<float> term2 = {.3,0,.3};
    quan = codebook->quantizeSoft(term2,2);
    assert(get<1>(quan[0])==.5 && get<1>(quan[1])==.5);
    
    LLC_numOfNN=1;
    /* 0 0 0
     * 1 1 1
     * 2 2 2
     * 
     * ans
     * (1,0,0) (2,0,0) (3,0,0)
     * (1,1,0) (2,2,0) (3,3,0)
     * (1,1,1) (2,2,2) (3,3,3)
    */
    vector<float> class0 = {1,0,0};
    vector<float> class1 = {0,1,0};
    vector<float> class2 = {0,0,1};
    Mat img(3,3,CV_8U);
    
    vector< description >* desc = new vector< description >();
    desc->push_back(description(class0,Point2i(0,0),0));
    desc->push_back(description(class0,Point2i(1,0),0));
    desc->push_back(description(class0,Point2i(2,0),0));
    desc->push_back(description(class1,Point2i(0,1),0));
    desc->push_back(description(class1,Point2i(1,1),0));
    desc->push_back(description(class1,Point2i(2,1),0));
    desc->push_back(description(class2,Point2i(0,2),0));
    desc->push_back(description(class2,Point2i(1,2),0));
    desc->push_back(description(class2,Point2i(2,2),0));
    
    
    vector< vector< Mat/*< float >*/ > >* descII = codeDescriptorsIntegralImageSkip( desc, img.size, 1);
    assert((*descII)[0][0].at<float>(0,0)==1 && (*descII)[0][0].at<float>(1,0)==0 && (*descII)[0][0].at<float>(2,0)==0);
    assert((*descII)[1][0].at<float>(0,0)==2 && (*descII)[1][0].at<float>(1,0)==0 && (*descII)[1][0].at<float>(2,0)==0);
    assert((*descII)[2][0].at<float>(0,0)==3 && (*descII)[2][0].at<float>(1,0)==0 && (*descII)[2][0].at<float>(2,0)==0);
    
    assert((*descII)[0][1].at<float>(0,0)==1 && (*descII)[0][1].at<float>(1,0)==1 && (*descII)[0][1].at<float>(2,0)==0);
    assert((*descII)[1][1].at<float>(0,0)==2 && (*descII)[1][1].at<float>(1,0)==2 && (*descII)[1][1].at<float>(2,0)==0);
    assert((*descII)[2][1].at<float>(0,0)==3 && (*descII)[2][1].at<float>(1,0)==3 && (*descII)[2][1].at<float>(2,0)==0);
    
    assert((*descII)[0][2].at<float>(0,0)==1 && (*descII)[0][2].at<float>(1,0)==1 && (*descII)[0][2].at<float>(2,0)==1);
    assert((*descII)[1][2].at<float>(0,0)==2 && (*descII)[1][2].at<float>(1,0)==2 && (*descII)[1][2].at<float>(2,0)==2);
    assert((*descII)[2][2].at<float>(0,0)==3 && (*descII)[2][2].at<float>(1,0)==3 && (*descII)[2][2].at<float>(2,0)==3);
    
    vector<Vec2i> spatialPyramids={Vec2i(1,1)};
    
    vector<float>* pooled = getPooledDescFastSkip(descII, Rect(0,0,3,3), spatialPyramids, 1);
    assert((*pooled)[0]==3 && (*pooled)[1]==3 && (*pooled)[2]==3);
    pooled = getPooledDescFastSkip(descII, Rect(1,1,2,2), spatialPyramids, 1);
    assert((*pooled)[0]==0 && (*pooled)[1]==2 && (*pooled)[2]==2);
    pooled = getPooledDescFastSkip(descII, Rect(1,0,2,2), spatialPyramids, 1);
    assert((*pooled)[0]==2 && (*pooled)[1]==2 && (*pooled)[2]==0);
    pooled = getPooledDescFastSkip(descII, Rect(0,1,2,2), spatialPyramids, 1);
    assert((*pooled)[0]==0 && (*pooled)[1]==2 && (*pooled)[2]==2);
    
    desc = new vector< description >();
    desc->push_back(description(class0,Point2i(0,0),0));
    desc->push_back(description(class0,Point2i(1,0),0));
    desc->push_back(description(class0,Point2i(2,0),0));
    desc->push_back(description(class1,Point2i(0,1),0));
    desc->push_back(description(class1,Point2i(1,1),0));
    desc->push_back(description(class1,Point2i(2,1),0));
    desc->push_back(description(class2,Point2i(0,2),0));
    desc->push_back(description(class2,Point2i(1,2),0));
    desc->push_back(description(class2,Point2i(2,2),0));
    
    desc->push_back(description(class0,Point2i(0,0),1));
    desc->push_back(description(class0,Point2i(1,0),1));
    desc->push_back(description(class0,Point2i(2,0),1));
    desc->push_back(description(class1,Point2i(0,1),1));
    desc->push_back(description(class1,Point2i(1,1),1));
    desc->push_back(description(class1,Point2i(2,1),1));
    desc->push_back(description(class2,Point2i(0,2),1));
    desc->push_back(description(class2,Point2i(1,2),1));
    desc->push_back(description(class2,Point2i(2,2),1));
    
    
    descII = codeDescriptorsIntegralImageSkip( desc, img.size, 1);
    assert((*descII)[0][0].at<float>(0,0)==1 && (*descII)[0][0].at<float>(1,0)==0 && (*descII)[0][0].at<float>(2,0)==0);
    assert((*descII)[1][0].at<float>(0,0)==2 && (*descII)[1][0].at<float>(1,0)==0 && (*descII)[1][0].at<float>(2,0)==0);
    assert((*descII)[2][0].at<float>(0,0)==3 && (*descII)[2][0].at<float>(1,0)==0 && (*descII)[2][0].at<float>(2,0)==0);
    
    assert((*descII)[0][1].at<float>(0,0)==1 && (*descII)[0][1].at<float>(1,0)==1 && (*descII)[0][1].at<float>(2,0)==0);
    assert((*descII)[1][1].at<float>(0,0)==2 && (*descII)[1][1].at<float>(1,0)==2 && (*descII)[1][1].at<float>(2,0)==0);
    assert((*descII)[2][1].at<float>(0,0)==3 && (*descII)[2][1].at<float>(1,0)==3 && (*descII)[2][1].at<float>(2,0)==0);
    
    assert((*descII)[0][2].at<float>(0,0)==1 && (*descII)[0][2].at<float>(1,0)==1 && (*descII)[0][2].at<float>(2,0)==1);
    assert((*descII)[1][2].at<float>(0,0)==2 && (*descII)[1][2].at<float>(1,0)==2 && (*descII)[1][2].at<float>(2,0)==2);
    assert((*descII)[2][2].at<float>(0,0)==3 && (*descII)[2][2].at<float>(1,0)==3 && (*descII)[2][2].at<float>(2,0)==3);
    
    
    assert((*descII)[0][0].at<float>(3,0)==1 && (*descII)[0][0].at<float>(4,0)==0 && (*descII)[0][0].at<float>(5,0)==0);
    assert((*descII)[1][0].at<float>(3,0)==2 && (*descII)[1][0].at<float>(4,0)==0 && (*descII)[1][0].at<float>(5,0)==0);
    assert((*descII)[2][0].at<float>(3,0)==3 && (*descII)[2][0].at<float>(4,0)==0 && (*descII)[2][0].at<float>(5,0)==0);
   
    assert((*descII)[0][1].at<float>(3,0)==1 && (*descII)[0][1].at<float>(4,0)==1 && (*descII)[0][1].at<float>(5,0)==0);
    assert((*descII)[1][1].at<float>(3,0)==2 && (*descII)[1][1].at<float>(4,0)==2 && (*descII)[1][1].at<float>(5,0)==0);
    assert((*descII)[2][1].at<float>(3,0)==3 && (*descII)[2][1].at<float>(4,0)==3 && (*descII)[2][1].at<float>(5,0)==0);
    
    assert((*descII)[0][2].at<float>(3,0)==1 && (*descII)[0][2].at<float>(4,0)==1 && (*descII)[0][2].at<float>(5,0)==1);
    assert((*descII)[1][2].at<float>(3,0)==2 && (*descII)[1][2].at<float>(4,0)==2 && (*descII)[1][2].at<float>(5,0)==2);
    assert((*descII)[2][2].at<float>(3,0)==3 && (*descII)[2][2].at<float>(4,0)==3 && (*descII)[2][2].at<float>(5,0)==3);
    
    
    
    /* 0 0 0 0
     * 1 1 1 1
     * 2 2 2 2
     * 0 1 2 2
     * 
     * ans
     * (1,0,0) (2,0,0) (3,0,0) (4,0,0)
     * (1,1,0) (2,2,0) (3,3,0) (4,4,0)
     * (1,1,1) (2,2,2) (3,3,3) (4,4,4)
     * (2,1,1) (3,3,2) (4,4,4) (5,5,6)
     * 
     * ans (skipped)
     *  (2,2,0)  (4,4,0)
     *  (3,3,2)  (5,5,6)
    */
    Mat img2(4,4,CV_8U);
    
    desc = new vector< description >();
    desc->push_back(description(class0,Point2i(0,0),0));
    desc->push_back(description(class0,Point2i(1,0),0));
    desc->push_back(description(class0,Point2i(2,0),0));
    desc->push_back(description(class0,Point2i(3,0),0));
    desc->push_back(description(class1,Point2i(0,1),0));
    desc->push_back(description(class1,Point2i(1,1),0));
    desc->push_back(description(class1,Point2i(2,1),0));
    desc->push_back(description(class1,Point2i(3,1),0));
    desc->push_back(description(class2,Point2i(0,2),0));
    desc->push_back(description(class2,Point2i(1,2),0));
    desc->push_back(description(class2,Point2i(2,2),0));
    desc->push_back(description(class2,Point2i(3,2),0));
    
    desc->push_back(description(class0,Point2i(0,3),0));
    desc->push_back(description(class1,Point2i(1,3),0));
    desc->push_back(description(class2,Point2i(2,3),0));
    desc->push_back(description(class2,Point2i(3,3),0));
    descII = codeDescriptorsIntegralImageSkip( desc, img2.size, 2);
    assert(descII->size()==2);
    assert((*descII)[0][0].at<float>(0,0)==2 && (*descII)[0][0].at<float>(1,0)==2 && (*descII)[0][0].at<float>(2,0)==0);
    assert((*descII)[1][0].at<float>(0,0)==4 && (*descII)[1][0].at<float>(1,0)==4 && (*descII)[1][0].at<float>(2,0)==0);
    
    assert((*descII)[0][1].at<float>(0,0)==3 && (*descII)[0][1].at<float>(1,0)==3 && (*descII)[0][1].at<float>(2,0)==2);
    assert((*descII)[1][1].at<float>(0,0)==5 && (*descII)[1][1].at<float>(1,0)==5 && (*descII)[1][1].at<float>(2,0)==6);
    
    
    pooled = getPooledDescFastSkip(descII, Rect(0,0,4,4), spatialPyramids, 2);
    assert((*pooled)[0]==5 && (*pooled)[1]==5 && (*pooled)[2]==6);
    pooled = getPooledDescFastSkip(descII, Rect(2,2,2,2), spatialPyramids, 2);
    assert((*pooled)[0]==0 && (*pooled)[1]==0 && (*pooled)[2]==4);
    pooled = getPooledDescFastSkip(descII, Rect(0,2,2,2), spatialPyramids, 2);
    assert((*pooled)[0]==1 && (*pooled)[1]==1 && (*pooled)[2]==2);
    
    vector<Vec2i> spatialPyramids2={Vec2i(1,1),Vec2i(2,1)};
    pooled = getPooledDescFastSkip(descII, Rect(0,0,4,4), spatialPyramids2, 2);
    assert((*pooled)[0]==5 && (*pooled)[1]==5 && (*pooled)[2]==6);
    assert((*pooled)[9]==2*3 && (*pooled)[10]==2*3 && (*pooled)[11]==2*2);
    assert((*pooled)[18]==2*2 && (*pooled)[19]==2*2 && (*pooled)[20]==2*4);
    
    
    
    
    Mat testimg = (Mat_<unsigned char>(20,20)<< 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0,
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
                                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0,
                                                0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0,
                                                0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1,
                                                0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1,
                                                0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0,
                                                0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1);
    
    
    for (int x=0; x<img2.cols; x++)
        for (int y=0; y<img2.rows; y++)
        {
            if (img2.at<unsigned char>(y,x)==0)
                img2.at<unsigned char>(y,x)=255;
            else
                img2.at<unsigned char>(y,x)=0;
        }
    
    desc_thresh=0;
    blockSize1=5;
    blockSize2=11;
    blockSize3=17;
    skip=1;
    blockStride=1;
    auto samplesUncoded = getDescriptors(testimg);
    
    int testsDone=0;
    for (int i=0; i<samplesUncoded->size(); i++)
    {
        if ((samplesUncoded->at(i).scale)==SMALL)
        {
            Point loc = (samplesUncoded->at(i).position);
            vector<float> desc = (samplesUncoded->at(i).values);
            if ((loc.x == 2 && loc.y == 2) || (loc.x == 8 && loc.y == 8) || (loc.x == 2 && loc.y == 14))
            {
                assert(desc[0] > desc[1]);
                assert(desc[0] > desc[2]);
                assert(desc[0] > desc[3]);
                assert(desc[0] > desc[4]);
                assert(desc[0] > desc[5]);
                assert(desc[0] > desc[6]);
                assert(desc[0] > desc[7]);
                testsDone++;
            }
            else if ((loc.x == 8 && loc.y == 2) || (loc.x == 14 && loc.y == 8) || (loc.x == 8 && loc.y == 14))
            {
                assert(desc[6] > desc[0]);
                assert(desc[6] > desc[1]);
                assert(desc[6] > desc[2]);
                assert(desc[6] > desc[3]);
                assert(desc[6] > desc[4]);
                assert(desc[6] > desc[5]);
                assert(desc[6] > desc[7]);
                assert(desc[6] > desc[8]);
                testsDone++;
            }
            else if (loc.x == 14 && loc.y == 2)
            {
                assert(desc[4] > desc[0]);
                assert(desc[4] > desc[1]);
                assert(desc[4] > desc[2]);
                assert(desc[4] > desc[3]);
                assert(desc[4] > desc[5]);
                assert(desc[4] > desc[6]);
                assert(desc[4] > desc[7]);
                assert(desc[4] > desc[8]);
                testsDone++;
            }
            else if (loc.x == 2 && loc.y == 8)
            {
                assert(desc[2] > desc[0]);
                assert(desc[2] > desc[1]);
                assert(desc[2] > desc[3]);
                assert(desc[2] > desc[4]);
                assert(desc[2] > desc[5]);
                assert(desc[2] > desc[6]);
                assert(desc[2] > desc[7]);
                assert(desc[2] > desc[8]);
                testsDone++;
            }
        }
        else if ((samplesUncoded->at(i).scale)==MED)
        {
            Point loc = (samplesUncoded->at(i).position);
            vector<float> desc = (samplesUncoded->at(i).values);
            if ((loc.x == 6 && loc.y == 6))
            {
                assert(desc[0] > desc[1]);
                assert(desc[0] > desc[2]);
                assert(desc[0] > desc[3]);
                assert(desc[0] > desc[4]);
                assert(desc[0] > desc[5]);
                assert(desc[0] > desc[6]);
                assert(desc[0] > desc[7]);
                testsDone++;
            }
            else if ((loc.x == 17 && loc.y == 17))
            {
                assert(desc[6] > desc[0]);
                assert(desc[6] > desc[1]);
                assert(desc[6] > desc[2]);
                assert(desc[6] > desc[3]);
                assert(desc[6] > desc[4]);
                assert(desc[6] > desc[5]);
                assert(desc[6] > desc[7]);
                assert(desc[6] > desc[8]);
                testsDone++;
            }
            
        }
    }
    assert(testsDone==10);
    
    cout << "EnhancedBoVW passed its tests!" << endl;
}
