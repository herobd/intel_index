#include "enhancedbovw.h"
//#include "opencv2/gpu/gpu.hpp"

EnhancedBoVW::EnhancedBoVW()
{
    desc_thresh=1;//.996;//.99667;
    LLC_numOfNN=3;
    codebook=NULL;
    spatialPyramids = {Vec2i(1,1),Vec2i(2,2)};
}
void EnhancedBoVW::scanImage(const Mat &img, const Mat &exemplar)
{
    auto samplesUncoded = getDescriptors(exemplar);
    auto samplesCoded = codeDescriptorsIntegralImage(samplesUncoded,exemplar.size);
    
    vector<float>* exe = getPooledDescFast(samplesCoded, Rect(0,0,exemplar.cols,exemplar.rows),spatialPyramids);
    normalizeDesc(exe);
    scanImage(img,*exe,exemplar.size());
    delete exe;
    
}


void EnhancedBoVW::scanImage(const Mat &img, const vector<float> &exemplar, Size exemplarSize)
{
    int hStride = 8;
    int vStride=8;
    int windowWidth=exemplarSize.width*1.05;
    int windowHeight=exemplarSize.height*1.05;
    int windowWidth2=exemplarSize.width;
    int windowHeight2=exemplarSize.height;
    int windowWidth3=exemplarSize.width*.95;
    int windowHeight3=exemplarSize.height*.95;
    
    Mat scores(img.rows/vStride, img.cols/hStride, CV_32FC3);
    float maxScore=0;
    float minScore=9999;
    
    auto samplesUncoded = getDescriptors(img);
//    auto samplesCoded = codeDescriptors(samplesUncoded);
    auto samplesCodedII = codeDescriptorsIntegralImage(samplesUncoded,img.size);
    
    
    for (int x=windowWidth3/3; x<img.cols-windowWidth3/3; x+=hStride)
        for (int y=windowHeight3/3; y<img.rows-windowHeight3/3; y+=vStride)
        {
            
            Rect r1(x-windowWidth/2, y-windowHeight/2, windowWidth, windowHeight);
            Rect r2(x-windowWidth2/2, y-windowHeight2/2, windowWidth2, windowHeight2);
            Rect r3(x-windowWidth3/2, y-windowHeight3/2, windowWidth3, windowHeight3);
            
            
            vector<float>* desc1 = getPooledDescFast(samplesCodedII, r1, spatialPyramids);
            normalizeDesc(desc1);
            vector<float>* desc2 = getPooledDescFast(samplesCodedII, r2, spatialPyramids);
            normalizeDesc(desc2);
            vector<float>* desc3 = getPooledDescFast(samplesCodedII, r3, spatialPyramids);
            normalizeDesc(desc3);
            
            float score1=0;
            float score2=0;
            float score3=0;
            
            for (int i=0; i<exemplar.size(); i++)
            {
                score1 += pow(exemplar[i]-desc1->at(i),2);
                score2 += pow(exemplar[i]-desc2->at(i),2);
                score3 += pow(exemplar[i]-desc3->at(i),2);
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
        }
    
    Mat heatmap;
    cvtColor(img,heatmap,CV_GRAY2RGB);
    for (int x=windowWidth3/3; x<img.cols-windowWidth3/3; x+=hStride)
        for (int y=windowHeight3/3; y<img.rows-windowHeight3/3; y+=vStride)
        {
            if (scores.at<Vec3f>(y/vStride, x/hStride)[0] > max(scores.at<Vec3f>(y/vStride, x/hStride)[1],scores.at<Vec3f>(y/vStride, x/hStride)[2]))
            {
                for (int xoff=-3; xoff<=4; xoff++)
                    for (int yoff=-3; yoff<=4; yoff++)
                    {
                        color(heatmap, (scores.at<Vec3f>(y/vStride,x/hStride))[0], maxScore, minScore,  x+xoff, y+yoff);
                    }
            }
            else if (scores.at<Vec3f>(y/vStride, x/hStride)[1] > scores.at<Vec3f>(y/vStride, x/hStride)[2])
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
    imwrite("./heatmap.jpg",heatmap);
    imshow("heatmap",heatmap);
    waitKey();
    
    delete samplesCodedII;
}

vector< tuple< vector< tuple<int,float> >, Point2i > >* EnhancedBoVW::codeDescriptors(vector< tuple< vector<float>, Point2i > >* desc)
{
    auto ret = new vector< tuple< vector< tuple<int,float> >, Point2i > >();
    for (int i=0; i< desc->size(); i++)
    {
        ret->push_back(make_tuple(codebook->quantizeSoft(get<0>(desc->at(i)),LLC_numOfNN),get<1>(desc->at(i))));
    }
    delete desc;
    return ret;
}

vector< vector< Mat/*< float >*/ > >* EnhancedBoVW::codeDescriptorsIntegralImage(vector< tuple< vector<float>, Point2i > >* desc, Mat::MSize imgsize)
{
    auto lam = [](const tuple< vector<float>, Point2i >& a, const tuple< vector<float>, Point2i > &b) -> bool
    {
         if (get<1>(a).x != get<1>(b).x) 
         return get<1>(a).x < get<1>(b).x;
         else
            return get<1>(a).y < get<1>(b).y;
    };




    sort(desc->begin(),desc->end(),lam);
    
    auto ret = new vector< vector< Mat/*< float >*/ > >(imgsize[1]);
    auto iter = desc->begin();
    //first col
    (*ret)[0].resize(imgsize[0]);                                                                                                                                                                                                                                                                                                                                                       
    for (int y=0; y<imgsize[0]; y++)
    {
        
        if (y==0)
            (*ret)[0][y] = Mat::zeros(codebook->size(),1,CV_32F);
        else
            (*ret)[0][y] = (*ret)[0][y-1].clone();
        while (iter!=desc->end() && get<1>(*iter).y==y && get<1>(*iter).x==0)
        {
            vector< tuple<int,float> > quan = codebook->quantizeSoft(get<0>(*iter),LLC_numOfNN);
            for (const auto &v : quan)
                (*ret)[0][y].at<float>(get<0>(v),0) += get<1>(v);
//            int it = codebook->quantize(get<0>(*iter));
//            (*ret)[0][y].at<float>(it,0) += 1;
            
//            cout << "feature at (0,"<<y<<")"<<endl;
            
            if (++iter==desc->end())
                break;
        }
    }
    for (int x=1; x<imgsize[1]; x++)
    {
        (*ret)[x].resize(imgsize[0]);
        for (int y=0; y<imgsize[0]; y++)
        {
            
            if (y==0)
                (*ret)[x][y] = (*ret)[x-1][y].clone();
            else
                (*ret)[x][y] = (*ret)[x][y-1]+(*ret)[x-1][y]-(*ret)[x-1][y-1];
            while (iter!=desc->end() && get<1>(*iter).y==y && get<1>(*iter).x==x)
            {
                vector< tuple<int,float> > quan = codebook->quantizeSoft(get<0>(*iter),LLC_numOfNN);
                for (const auto &v : quan)
                    (*ret)[x][y].at<float>(get<0>(v),0) += get<1>(v);
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

void EnhancedBoVW::color(Mat &heatMap, float score, float max, float min, int midI, int midJ)
{
    heatMap.at<Vec3b>(midJ,midI)[0] = heatMap.at<Vec3b>(midJ,midI)[0] * ((score-min)/(max-min));
    heatMap.at<Vec3b>(midJ,midI)[1] = heatMap.at<Vec3b>(midJ,midI)[2] * (1 - (score-min)/(max-min));
    heatMap.at<Vec3b>(midJ,midI)[2] = heatMap.at<Vec3b>(midJ,midI)[2] * (1 - (score-min)/(max-min));
}

vector< tuple< vector<float>, Point2i > >* EnhancedBoVW::getDescriptors(const Mat &img)
{
//    gpu::GpuMat gimg(img);
//    gpu::GpuMat desc1,
    
//    Size windowSize(20,20);
    
//    gpu::HOGDescriptor hog1(windowSize1,blockSize1,blockStride,cellSize)
//    gpu::HOGDescriptor::getdescriptors1(gimg,stride,desc1,)
    
    Size blockSize1(16,16);
    Size blockSize2(24,24);
    Size blockSize3(32,32);
    
    Size blockStride(4,4);
    
//    Size cellSize1(10,10);
    
//    Size windowSize1(img.size[1]-(img.size[1]%blockSize1.height),img.size[0]-(img.size[0]%blockSize1.width));
//    Size windowSize2(img.size[1]%24,img.size[1]%24);
//    Size windowSize3(img.size[1]%32,img.size[1]%32);
    
    HOGDescriptor hog1(blockSize1,blockSize1,blockStride,blockSize1,9,1,-1,HOGDescriptor::L2Hys,.2,true,1);
    HOGDescriptor hog2(blockSize2,blockSize2,blockStride,blockSize2,9,1,-1,HOGDescriptor::L2Hys,.2,true,1);
    HOGDescriptor hog3(blockSize3,blockSize3,blockStride,blockSize3,9,1,-1,HOGDescriptor::L2Hys,.2,true,1);
    
//    cout << "w:" << windowSize1 << " i:" << img.size() << endl;
    
//    Mat desc1,
    
    
    vector<float> desc1;
    hog1.compute(img,desc1,blockStride);
    vector<float> desc2;
    hog2.compute(img,desc2,blockStride);
    vector<float> desc3;
    hog3.compute(img,desc3,blockStride);

//    cout << hog1.getdescriptors1ize() << endl;
//    cout << desc.size() << endl;

    
    
    
    
    vector<vector<float> > descriptors1(1);
    vector<vector<float> > descriptors2(1);
    vector<vector<float> > descriptors3(1);
    vector< Point2i > locations1;
    vector< Point2i > locations2;
    vector< Point2i > locations3;
    
    filterDesc(desc1,descriptors1,locations1,hog1.getDescriptorSize(), blockSize1, blockStride, img.size());
    filterDesc(desc2,descriptors2,locations2,hog2.getDescriptorSize(), blockSize2, blockStride, img.size());
    filterDesc(desc3,descriptors3,locations3,hog3.getDescriptorSize(), blockSize3, blockStride, img.size());
    
    vector< tuple< vector< float >, Point2i > > *descAndLoc = new vector< tuple< vector< float >, Point2i > >();
    for (int i=0; i< descriptors1.size(); i++)
    {
        if (descriptors1[i].size() > 0)
            descAndLoc->push_back(make_tuple(descriptors1[i],locations1[i]));
    }
    for (int i=0; i< descriptors2.size(); i++)
    {
        if (descriptors2[i].size() > 0)
            descAndLoc->push_back(make_tuple(descriptors2[i],locations2[i]));
    }
    for (int i=0; i< descriptors3.size(); i++)
    {
        if (descriptors3[i].size() > 0)
            descAndLoc->push_back(make_tuple(descriptors3[i],locations3[i]));
    }
    

    Mat heat;
    cvtColor(img,heat,CV_GRAY2RGB);
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
    for (vector<float> d : descriptors1)
    {
        double norm=0;
        for (float n : d)
        {
            norm += n*n;
        }
        norm = sqrt(norm);
//        cout << norm << endl;
        int x= (i%((img.cols-(blockSize1.width-blockStride.width))/blockStride.width))*blockStride.width + blockSize1.width/2-blockStride.width/2;
        int y= (i/((img.cols-(blockSize1.width-blockStride.width))/blockStride.width))*blockStride.height + blockSize1.height/2-blockStride.height/2;
//        cout << x <<", "<<y<<endl;
        assert(x+blockStride.width<img.cols && y+blockStride.height<img.rows);
        
        for (int xoff=0; xoff<blockStride.width; xoff++)
            for (int yoff=0; yoff<blockStride.height; yoff++)
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
    imshow("heatfeatures",heat);
    waitKey();
    
    return descAndLoc;
}

void EnhancedBoVW::filterDesc(vector<float> &unparsedDescriptors, vector<vector<float> > &descriptors1, vector< Point2i > &locations, int descSize, Size blockSize1, Size blockStride, Size imgSize)
{
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

Codebook* EnhancedBoVW::makeCodebook(string directory, int codebook_size)
{
    vector< vector<float> > accum;
    
    
    int count=0;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (directory.c_str())) != NULL) {
      
      while ((ent = readdir (dir)) != NULL) {
          if (count++>200) break;
          
          string fileName(ent->d_name);
          //cout << "examining " << fileName << endl;
          if (fileName[0] == '.' || (fileName[fileName.size()-1]!='G' && fileName[fileName.size()-1]!='g' &&  fileName[fileName.size()-1]!='f'))
              continue;
          
          
          Mat img = imread(directory+fileName, CV_LOAD_IMAGE_GRAYSCALE);
          vector< tuple<vector<float>, Point2i > >* desc = getDescriptors(img);
          for (auto t : *desc)
          {
//              assert(get<0>(t).size() > 0);
              accum.push_back(get<0>(t));
          }
          delete desc;
      }
      Mat centriods;
      TermCriteria crit(0,500,.9);
//      Mat data(accum.size(),accum[0].size(),CV_32F);
//      for (int r=0; r< accum.size(); r++)
//          for (int c=0; c<accum[0].size(); c++)
//              data.at<float>(r,c) = accum[r][c];
      Mat data(codebook_size*100,accum[0].size(),CV_32F);
      for (int count=0; count< codebook_size*100; count++)
      {
          int r;
          do
          {
              r=rand()%accum.size();
          } while (accum[r].size()==0);
          
          
          for (int c=0; c<accum[0].size(); c++)
              data.at<float>(count,c) = accum[r][c];
          accum[r].resize(0);
      }
      Mat temp;
      kmeans(data,codebook_size,temp,crit,20,KMEANS_RANDOM_CENTERS,centriods);
      
      codebook = new Codebook();
      for (int r=0; r<centriods.rows; r++)
      {
          vector<double> toAdd;
          for (int c=0; c<centriods.cols; c++)
              toAdd.push_back(centriods.at<float>(r,c));
          codebook->push_back(toAdd);
      }
      
      return codebook;
    }
    else
        cout << "Error, could not load files for codebook." << endl;
    return NULL;
}


vector<float>* EnhancedBoVW::getPooledDesc(vector< tuple< vector< tuple<int,float> >, Point2i > >* samples, Rect window, vector<Vec2i> spatialPyramids)
{
    vector<float>* ret = new vector<float>();
    int binWidth = window.width/spatialPyramids.front()[0];
    int binHeight= window.height/spatialPyramids.front()[1];
    for (int binH=0; binH<spatialPyramids.front()[0]; binH++)
        for (int binV=0; binV<spatialPyramids.front()[1]; binV++)
        {
            Point2i recTL(window.x + binH*binWidth, window.y + binV*binHeight);
            Point2i recBR(window.x + (binH+1)*binWidth, window.y + (binV+1)*binHeight);
            
            vector<float> bins(codebook->size(),0);
            for (const auto &p : *samples)
            {
                if (get<1>(p).x>=recTL.x && get<1>(p).x<=recBR.x &&
                        get<1>(p).y>=recTL.y && get<1>(p).y<=recBR.y)
                {
                    for (const auto &binWeight : get<0>(p))
                    {
                        bins[get<0>(binWeight)] += get<1>(binWeight);
                    }
                }
            }
            ret->insert(ret->end(),bins.begin(),bins.end());
            
//            if (spatialPyramids.size()>1)
//            {
//                vector<float>* sub = getPooledDesc(samples,Rect(recTL.x,recTL.y,recBR.x-recTL.x,recBR.y-recTL.y),vector<Vec2i>(spatialPyramids.begin()+1,spatialPyramids.end()));
//                ret->insert(ret->end(),sub->begin(),sub->end());
//                delete sub;
//            }
        }
    if (spatialPyramids.size()>1)
    {
        vector<float>* sub = getPooledDesc(samples,window,vector<Vec2i>(spatialPyramids.begin()+1,spatialPyramids.end()));
        ret->insert(ret->end(),sub->begin(),sub->end());
        delete sub;
    }
    
    return ret;
}

vector<float>* EnhancedBoVW::getPooledDescFast(vector< vector< Mat/*< float >*/ > >* samplesIntegralImage, Rect window, vector<Vec2i> spatialPyramids)
{
    vector<float>* ret = new vector<float>();
    int binWidth = window.width/spatialPyramids.front()[0];
    int binHeight= window.height/spatialPyramids.front()[1];
    for (int binH=0; binH<spatialPyramids.front()[0]; binH++)
        for (int binV=0; binV<spatialPyramids.front()[1]; binV++)
        {
            Point2i recTL(max(0,window.x + binH*binWidth),
                          max(0,window.y + binV*binHeight));
            Point2i recBR(min((int)samplesIntegralImage->size()-1,window.x + (binH+1)*binWidth -1), 
                          min((int)samplesIntegralImage->at(0).size()-1,window.y + (binV+1)*binHeight -1));
            
//            vector<float> bins(codebook->size(),0);
//            for (int f=0; f<codebook->size(); f++)
//            {
//                bins[f] = samplesIntegralImage[recBR.x][recBR.y][f]
//            }
            Mat bins = (*samplesIntegralImage)[recBR.x][recBR.y] + 
                    (*samplesIntegralImage)[recTL.x][recTL.y] -
                    (*samplesIntegralImage)[recBR.x][recTL.y] - 
                    (*samplesIntegralImage)[recTL.x][recBR.y];
            
            for (int i=0; i<bins.size[0]; i++)
                ret->push_back(bins.at<float>(i,0));
//            ret->insert(ret->end(),bins.begin(),bins.end());
            
//            if (spatialPyramids.size()>1)
//            {
//                vector<float>* sub = getPooledDesc(samples,Rect(recTL.x,recTL.y,recBR.x-recTL.x,recBR.y-recTL.y),vector<Vec2i>(spatialPyramids.begin()+1,spatialPyramids.end()));
//                ret->insert(ret->end(),sub->begin(),sub->end());
//                delete sub;
//            }
        }
    if (spatialPyramids.size()>1)
    {
        vector<float>* sub = getPooledDescFast(samplesIntegralImage,window,vector<Vec2i>(spatialPyramids.begin()+1,spatialPyramids.end()));
        ret->insert(ret->end(),sub->begin(),sub->end());
        delete sub;
    }
    
    return ret;
}

void EnhancedBoVW::normalizeDesc(vector<float> *desc, float a)
{
    for (int i=0; i<desc->size(); i++)
    {
        desc->at(i) = copysign(desc->at(i),1) * pow(fabs(desc->at(i)),a);
    }
    double norm=0;
    for (int i=0; i<desc->size(); i++)
    {
        norm += pow(desc->at(i),2);
    }
    norm = sqrt(norm);
    for (int i=0; i<desc->size(); i++)
    {
        desc->at(i) /= norm;
    }
}
