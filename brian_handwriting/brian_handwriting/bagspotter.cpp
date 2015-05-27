#include "bagspotter.h"
#include <dirent.h>

#define SIFT_THRESH .04
#define MIN_FEATURE_SIZE 5

//../../data/centriods.dat ../../data/bigram/ee/ ../../data/name_segs/
BagSpotter::BagSpotter(const Codebook *codebook)
{
    this->codebook=codebook;
    learned.assign(codebook->size(),0);
    learned_tfidf.assign(codebook->size(),0);
    windowWidth=0;
    windowHeight=0;
    stepSize=8;//20
    
    for (int c=0; c<codebook->size(); c++)
    {
        Vec3b color(rand()%256,rand()%256,rand()%256);
        colorTable.push_back(color);
    }
//    detector = new SIFT(0,3,SIFT_THRESH,5);
    detector = new SURF(10000,2,4,true,true);//TT 20000, 3000 for grayscale hw, 10000 for binary hw
}

void BagSpotter::detectKeypoints(Mat &img, vector<KeyPoint> &keypoints, Mat &desc)
{
    Mat temp;
    (*detector)( img, Mat(), keypoints, temp);
    auto kpI=keypoints.begin();
//    auto dsI=desc.begin();
    int r=0;
    while (kpI != keypoints.end() )
    {
//        if (kpI->size < MIN_FEATURE_SIZE) DD
//        if (kpI->size > MAX_FEATURE_SIZE)
//        {
//            kpI = keypoints.erase(kpI);
            
//        }
//        else
        {
            
            desc.push_back(temp.row(r));
            kpI++; r++;
        }
    }
}

bool BagSpotter::train(string dirPath)
{
    
    int count=0;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (dirPath.c_str())) != NULL) {
      /* print all the files and directories within directory */
      while ((ent = readdir (dir)) != NULL) {
          
          string fileName(ent->d_name);
          //cout << "examining " << fileName << endl;
          if (fileName[0] == '.' || fileName[fileName.size()-1]!='G')
              continue;
          count++;
          Mat img = imread(dirPath+fileName, CV_LOAD_IMAGE_GRAYSCALE);
          //resize(img,img,Size(img.cols/2,img.rows/2));
          threshold(img,img,120.0,255,THRESH_BINARY);
          
          assert(img.cols > 1 && img.rows > 1);
          windowWidth += img.cols;
          windowHeight += img.rows;
        
          vector<KeyPoint> keypoints;
//          vector< vector<float> > desc;
          Mat desc;
          detectKeypoints(img, keypoints, desc);
//          for (vector<float> &d : desc)
//          {
//              int f = codebook->quantize(d);
//              learned[f] += 1;
//          }
          Mat out;
          cvtColor(img,out,CV_GRAY2RGB);
          
          for (int r=0; r<desc.rows; r++)
          {
              int f = codebook->quantize(desc.row(r));
              learned[f] += 1;    
              circle(out,keypoints[r].pt,keypoints[r].size,Scalar(colorTable[f]));
              
          }
          
          imshow("learning keypoints",out);
          waitKey(5);
      }
      closedir (dir);
    } else {
      /* could not open directory */
      perror ("");
      return false;
    }
    
    double sum=0;
    for_each(learned.begin(), learned.end(), [&](double &i){
            i/=count;
            sum+=i;
    });
    
double norm=0;
for (int i=0; i<codebook->size(); i++)
    {
learned_tfidf[i]=(learned[i]/sum)*codebook->getInverseDocFreq(i);
norm+=learned_tfidf[i]*learned_tfidf[i];

    }
norm = pow(norm,.5);
for (int i=0; i<codebook->size(); i++)
    {
learned_tfidf[i]/=norm;

    }
    windowWidth/=count;
    windowHeight/=count;
    return true;
}

    void BagSpotter::color(Mat &heatMap, double score, double max, int midI, int midJ)
    {
        heatMap.at<Vec3b>(midJ,midI)[0] = heatMap.at<Vec3b>(midJ,midI)[0] * (score/max);
        heatMap.at<Vec3b>(midJ,midI)[1] = 0;
        heatMap.at<Vec3b>(midJ,midI)[2] = heatMap.at<Vec3b>(midJ,midI)[2] * (1 - score/max);
    }
    
bool BagSpotter::testSpotting(string dirPath)
{
    
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (dirPath.c_str())) != NULL) {
      /* print all the files and directories within directory */
      while ((ent = readdir (dir)) != NULL) {
          
          string fileName(ent->d_name);
          
          if (fileName[0] == '.' || fileName[fileName.size()-1]!='G')
              continue;
          cout << "examining " << fileName << endl;
          produceHeatMap(dirPath+fileName);
          
      }
      closedir (dir);
    } else {
      /* could not open directory */
      perror ("");
      return false;
    }
    return true;
}

void BagSpotter::produceHeatMap(string fileName)
{
    Mat img = imread(fileName, CV_LOAD_IMAGE_GRAYSCALE);
    //resize(img,img,Size(img.cols/2,img.rows/2));
    threshold(img,img,120.0,255,THRESH_BINARY);
    
    Mat heatMap;
    cvtColor(img,heatMap,CV_GRAY2RGB);
    vector< vector< double > > scores(img.cols-windowWidth);
    double max =0;
    Point2i maxCorner;
    map< int, map< int, vector<int> > >* fm = buildFeatureMap(img);
    
    
    for (int i=0; i<img.cols-windowWidth; i+=stepSize)
    {
        scores[i].resize(img.rows-windowHeight);
        for (int j=0; j<img.rows-windowHeight; j+=stepSize)
        {
            scores[i][j] = detect(fm,Point2i(i,j));
            if (scores[i][j] > max)
            {
                max = scores[i][j];
                maxCorner = Point2i(i,j);
            }
        }
    }
    for (int i=0; i<img.cols-windowWidth; i+=stepSize)
    {
        for (int j=0; j<img.rows-windowHeight; j+=stepSize)
        {
            int midI = i+windowWidth/2;
            int midJ = j+ windowHeight/2;
            for (int oi=-stepSize/2; oi<stepSize/2; oi++)
                for (int oj=-stepSize/2; oj<stepSize/2; oj++)
                {
                    color(heatMap,scores[i][j],max,midI+oi,midJ+oj);
                }
            
//                  color(heatMap,scores[i][j],max,midI,midJ);
//                  color(heatMap,scores[i][j],max,midI-1,midJ);
//                  color(heatMap,scores[i][j],max,midI,midJ-1);
//                  color(heatMap,scores[i][j],max,midI+1,midJ);
//                  color(heatMap,scores[i][j],max,midI,midJ+1);
        }
    }
    putPointsOn(heatMap,fm,maxCorner);
    cout << "max score: "<<max<<endl;
    imshow("heatmap",heatMap);
    waitKey();
    delete fm;
}

//double BagSpotter::detect(Mat img)
//{
//    vector<int> here(codebook->size());
    
    
//    vector<KeyPoint> keypoints;
//    Mat desc;
//    detectKeypoints(img, keypoints, desc);
//    for (int r=0; r<desc.rows; r++)
//    {
//        int f = codebook->quantize(desc.row(r));
//        here[f] += 1;     
//    }
    
//    double score=0;
//    for (int i=0; i<learned.size(); i++)
//    {
//        score += pow(learned[i]-here[i],2);
//    }
//    return score;
//}

map< int, map< int, vector<int> > >* BagSpotter::buildFeatureMap(Mat img)
{
    map< int, map< int, vector<int> > >* fm = new map< int, map< int, vector<int> > >();
    
    vector<KeyPoint> keypoints;
    Mat desc;
    detectKeypoints( img, keypoints, desc);
    
    Mat pnts;
    cvtColor(img,pnts,CV_GRAY2RGB);
    
    for (int r=0; r<desc.rows; r++)
    {
        int f = codebook->quantize(desc.row(r));
        //assert((*fm)[keypoints[r].pt.x][keypoints[r].pt.y]==0);
        (*fm)[keypoints[r].pt.x][keypoints[r].pt.y].push_back(f); 
        
        circle(pnts,keypoints[r].pt,keypoints[r].size,Scalar(colorTable[f]));
    }
    
    imshow("points",pnts);
    waitKey();
    
    return fm;
}

void BagSpotter::forAllFeaturesInWindow(int windowWidth, int windowHeight, map< int, map< int, vector<int> > >* fm, Point2i corner, function<void(int,int,int)> doThis)
{
    auto it=fm->lower_bound(corner.x);
    auto itEnd=fm->end();
    
    for (; it != itEnd; ++it)
    {
        int x = (*it).first;
        if (x>corner.x+windowWidth)
            break;
        auto it2=(*it).second.lower_bound(corner.y);
        auto it2End=(*it).second.end();
        for (; it2 != it2End; ++it2)
        {
            int y = (*it2).first;
            if (y>corner.y+windowHeight)
                break;
            
            
            for (int f : (*it2).second)
            { 
                doThis(f,x,y);
            }
        }
           
    }
}

double BagSpotter::detect(map< int, map< int, vector<int> > >* fm, Point2i corner)
{
    vector<int> here(codebook->size());
    vector<double> tfidf(codebook->size());
    
    int sum=0;
    auto doSum = [&sum, &here](int f, int x, int y){ 
        here[f] += 1;  
        sum+=1;
    };
    forAllFeaturesInWindow(windowWidth,windowHeight,fm,corner, doSum);
    
    
    if (sum>0)
    {
        double norm=0;
        for (int i=0; i<codebook->size(); i++)
        {
            tfidf[i]=((here[i]*1.0)/sum)*codebook->getInverseDocFreq(i);
            norm+=tfidf[i]*tfidf[i];
            //        assert(norm!=0 || i!=2 || corner.x!=0 || corner.y!=60);
        }
        
        norm = pow(norm,.5);
        for (int i=0; i<codebook->size(); i++)
        {
            if (norm==0)
            {
                assert(tfidf[i]==0);
                
            }
            else
                tfidf[i]/=norm;
            
        }
    }
    
    double score=0;
//    cout << "start scoring" << endl;
    for (int i=0; i<codebook->size(); i++)
    {
//        cout << learned_tfidf[i] <<  " - " << tfidf[i] << endl;
        score += pow(learned_tfidf[i]-tfidf[i],2);
        if(score>100)
        {
//            cout << "score is " << score << endl;
            assert(false);
        }
    }
    
    return score;
    
//    double score=0;
//    for (int i=0; i<learned.size(); i++)
//    {
//        score += pow(learned[i]-here[i],2);
//    }
//    return score;
}

void BagSpotter::putPointsOn(Mat &img, map< int, map< int, vector<int> > >* fm, Point2i corner)
{
    auto doDraw = [&img, this](int f, int x, int y){circle(img,Point2i(x,y),(int)learned[f],Scalar(colorTable[f]));};
forAllFeaturesInWindow(windowWidth,windowHeight,fm,corner,doDraw);
}
