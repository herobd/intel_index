#include "enhancedbovw.h"
//#include "opencv2/gpu/gpu.hpp"

EnhancedBoVW::EnhancedBoVW()
{
    desc_thresh=.99667;
    LLC_numOfNN=3;
    codebook=NULL;
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
vector< tuple< vector<float>, Point2i > >* EnhancedBoVW::getDescriptors(Mat &img)
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
        if (descriptors1[i].size() > 0)
            descAndLoc->push_back(make_tuple(descriptors2[i],locations2[i]));
    }
    for (int i=0; i< descriptors3.size(); i++)
    {
        if (descriptors1[i].size() > 0)
            descAndLoc->push_back(make_tuple(descriptors3[i],locations3[i]));
    }
    return descAndLoc;
//    Mat heat;
//    cvtColor(img,heat,CV_GRAY2RGB);
//    int i=0;
//    for (vector<float> d : descriptors1)
//    {
//        double norm=0;
//        for (float n : d)
//        {
//            norm += n*n;
//        }
////        cout << norm << endl;
//        int x= (i%((img.cols-(blockSize1.width-blockStride.width))/blockStride.width))*blockStride.width + blockSize1.width/2-blockStride.width/2;
//        int y= (i/((img.cols-(blockSize1.width-blockStride.width))/blockStride.width))*blockStride.height + blockSize1.height/2-blockStride.height/2;
////        cout << x <<", "<<y<<endl;
//        assert(x+blockStride.width<img.cols && y+blockStride.height<img.rows);
        
//        for (int xoff=0; xoff<blockStride.width; xoff++)
//            for (int yoff=0; yoff<blockStride.height; yoff++)
//            {
////                heat.at<Vec3b>(y+yoff,x+xoff) = Vec3b(
////                        img.at<unsigned char>(y+yoff,x+xoff)*(((maxNorm-minNorm)-max(norm-minNorm,0.0))/(maxNorm-minNorm)),
////                        img.at<unsigned char>(y+yoff,x+xoff)*(((maxNorm-minNorm)-max(norm-minNorm,0.0))/(maxNorm-minNorm)),
////                        img.at<unsigned char>(y+yoff,x+xoff)*((max(norm-minNorm,0.0))/(maxNorm-minNorm)));
////                heat.at<Vec3b>(y+yoff,x+xoff) = Vec3b(
////                        img.at<unsigned char>(y+yoff,x+xoff)*(((maxNorm-minNorm)-max(norm-minNorm,0.0))/(maxNorm-minNorm)),
////                        img.at<unsigned char>(y+yoff,x+xoff)*(((maxNorm-minNorm)-max(norm-minNorm,0.0))/(maxNorm-minNorm)),
////                        img.at<unsigned char>(y+yoff,x+xoff));
//                if (norm!=0)
//                    heat.at<Vec3b>(y+yoff,x+xoff) = Vec3b(
//                                0,
//                                0,
//                                img.at<unsigned char>(y+yoff,x+xoff));
//            }
                
//        i++;
//    }
//    imshow("heat",heat);
//    waitKey();
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
            
            int x= (location%((imgSize.width-(blockSize1.width-blockStride.width))/blockStride.width))*blockStride.width + blockSize1.width/2-blockStride.width/2;
            int y= (location/((imgSize.height-(blockSize1.width-blockStride.width))/blockStride.width))*blockStride.height + blockSize1.height/2-blockStride.height/2;
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
          
          string fileName(ent->d_name);
          //cout << "examining " << fileName << endl;
          if (count>2000 || fileName[0] == '.' || (fileName[fileName.size()-1]!='G' && fileName[fileName.size()-1]!='g'))
              continue;
          count++;
          Mat img = imread(directory+fileName, CV_LOAD_IMAGE_GRAYSCALE);
          vector< tuple<vector<float>, Point2i > >* desc = getDescriptors(img);
          for (auto t : *desc)
          {
              accum.push_back(get<0>(t));
          }
          delete desc;
      }
      Mat centriods;
      TermCriteria crit(0,1200,.9);
      kmeans(accum,codebook_size,Mat(),crit,20,KMEANS_RANDOM_CENTERS,centriods);
      Codebook* cb = new Codebook();
      for (int r=0; r<centriods.rows; r++)
      {
          vector<double> toAdd;
          for (int c=0; c<centriods.cols; c++)
              toAdd.push_back(centriods.at<float>(r,c));
          cb->push_back(toAdd);
      }
      return cb;
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
            
            if (spatialPyramids.size()>1)
            {
                vector<float>* sub = getPooledDesc(samples,Rect(recTL.x,recTL.y,recBR.x-recTL.x,recBR.y-recTL.y),vector<Vec2i>(spatialPyramids.begin()+1,spatialPyramids.end()));
                ret->insert(ret->end(),sub->begin(),sub->end());
                delete sub;
            }
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
