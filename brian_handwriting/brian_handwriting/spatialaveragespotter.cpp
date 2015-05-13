#include "spatialaveragespotter.h"

#include <dirent.h>
#include <stdlib.h>
//../../data/centriods.dat ../../data/bigram/ee/ ../../data/name_segs/
SpatialAverageSpotter::SpatialAverageSpotter(const Codebook *codebook)
{
    this->codebook=codebook;
    learned.assign(codebook->size(),0);
    learned_tfidf.assign(codebook->size(),0);
    windowWidth=0;
    windowHeight=0;
    stepSize=10;//20 DD 8 TT
    
    
    vector<Vec3b> tempTable;
    for (int c=0; c<codebook->size(); c++)
    {
        Vec3b color(180*(c%(1+(codebook->size()/2)))/(1.0+(codebook->size()/2)),255,(c>codebook->size()/2)?255:150);
        
        tempTable.push_back(color);
    }
    cvtColor(tempTable,colorTable,CV_HSV2RGB); 
    
    
//    detector = new SIFT(0,3,.04,5);
    detector = new SURF(20000,2,4,true,true);//TT 20000
}

#define MIN_FEATURE_SIZE 4
#define MAX_FEATURE_SIZE 10
void SpatialAverageSpotter::detectKeypoints(Mat &img, vector<KeyPoint> &keypoints, Mat &desc)
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

Mat translateImg(Mat &img, int offsetx, int offsety){
    Mat trans_mat = (Mat_<double>(2,3) << 1, 0, offsetx, 0, 1, offsety);
    warpAffine(img,img,trans_mat,img.size());
    return trans_mat;
}

Mat strechImg(Mat &img, double scalex, double scaley){
    Mat trans_mat = (Mat_<double>(2,3) << scalex, 0, 0, 0, scaley, 0);
    warpAffine(img,img,trans_mat,img.size());
    return trans_mat;
}


#define BORDER_SIZE 10
#define PRE_BASE_SIZE 140//150 DD 70 TT
#define BASE_SIZE PRE_BASE_SIZE+2*BORDER_SIZE 
bool SpatialAverageSpotter::train(string dirPath)
{
    
    int count=0;
    vector<vector<tuple<int,Point2f> > > features;
    featureAverages.resize(codebook->size());
    for (int i =0; i<codebook->size(); i++)
        featureAverages[i]=Mat(BASE_SIZE,BASE_SIZE,CV_32F,Scalar(0));
    
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (dirPath.c_str())) != NULL) {
      /* print all the files and directories within directory */
//      Mat img;
      while ((ent = readdir (dir)) != NULL) {
          
          string fileName(ent->d_name);
//          cout << "examining " << fileName << endl;
          if (fileName[0] == '.' || fileName[fileName.size()-1]!='G')
              continue;
          
          Mat img = imread(dirPath+fileName, CV_LOAD_IMAGE_GRAYSCALE);
          resize(img,img,Size(0,0),2,2);
          threshold(img,img,120.0,255,THRESH_BINARY);
          windowWidth += img.cols;
          windowHeight += img.rows;
//          int avg=0;
//          for (int x=0; x<img.cols; x++)
//              for (int  y=0; y<img.rows; y++)
//                  avg += (int)img.at<unsigned char>(y,x);
////          cout << "avg="<<avg<<"/"<<img.cols*img.rows<<" = "<<avg/(img.cols*img.rows)<<endl;
//          avg /= img.cols*img.rows;
          
          resize(img,img,Size(PRE_BASE_SIZE,PRE_BASE_SIZE*((0.0+img.rows)/img.cols)));
          
          
          copyMakeBorder( img, img, BORDER_SIZE, BORDER_SIZE, BORDER_SIZE, BORDER_SIZE, BORDER_CONSTANT, 255 );
          assert(img.cols > 1 && img.rows > 1);
          adjustedTrainingImages.push_back(img.clone());
          
          Point2f centerOfMass = findCenterOfMass(img);
          int offsetx=(img.cols/2)-centerOfMass.x;
          int offsety=(img.rows/2)-centerOfMass.y;
          translateImg(img,offsetx,offsety);
          
        
          vector<KeyPoint> keypoints;
          Mat desc;
          detectKeypoints( img,keypoints, desc);
          Mat out;
          cvtColor(img,out,CV_GRAY2RGB);
          circle(out,centerOfMass,1,Scalar(0,0,255));
          
          features.resize(count+1);
          //double scaling = BASE_SIZE/img
          for (int r=0; r<desc.rows; r++)
          {
              int f = codebook->quantize(desc.row(r));
              Point2f offsetPoint(keypoints[r].pt.x - centerOfMass.x, keypoints[r].pt.y - centerOfMass.y);
              features[count].push_back(make_tuple(f,offsetPoint));//we're ignoring the keypoint scale.. 
              
              
//              circle(out,keypoints[r].pt,keypoints[r].size,Scalar(colorTable[f]));
              Rect rec(keypoints[r].pt.x-(keypoints[r].size/2),keypoints[r].pt.y-(keypoints[r].size/2),keypoints[r].size,keypoints[r].size);
              rectangle(out,rec,Scalar(colorTable[f]));
              
          }
          guassColorIn(features[count]);
          
          
          imshow("learning keypoints",out);
          cout << "image "<<count<<endl;
          waitKey();
          
          count++;
//          img.release();
      }
      closedir (dir);
    } else {
      /* could not open directory */
      perror ("");
      return false;
    }
    
    //We now step through adjusting the scales of the various images so the guass coloring is maximized
    //But we may still want to look at tfidf to see which ones should be weighted more, etc.
    maximizeAlignment(features);
    
    float max=0;
    float min =99999;
    float avg_max=0;
    for (int f=0; f<codebook->size(); f++)
    {
        float local_max=0;
        for (int x=0; x<featureAverages[f].cols; x++)
            for (int y=0; y<featureAverages[f].rows; y++)
            {
                float val = featureAverages[f].at<float>(y,x);
                if (val > 300 || val < -300)
                    cout << "val (" << x <<","<<y<<") " << val << endl;
                if (val>max) max=val;
                if (val<min) min=val;
                if (val>local_max) local_max=val;
            }
        avg_max+=local_max;
    }
//    penalty=min+(max-min)*.2;
    avg_max /= codebook->size();
    penalty=avg_max*.15;//.2
    
    windowWidth/=count;
    windowHeight/=count;
    
    //show averages
    showAverages();
    
    return true;
}

    void color(Mat &heatMap, double score, double max, int midI, int midJ)
    {
        heatMap.at<Vec3b>(midJ,midI)[0] = heatMap.at<Vec3b>(midJ,midI)[0] * (score/max);
        heatMap.at<Vec3b>(midJ,midI)[1] = 0;
        heatMap.at<Vec3b>(midJ,midI)[2] = heatMap.at<Vec3b>(midJ,midI)[2] * (1 - score/max);
    }

    
    
Point2f SpatialAverageSpotter::findCenterOfMass(Mat &img)
{
//    int avg=0;
//    for (int x=0; x<img.cols; x++)
//        for (int y=0; y<img.rows; y++)
//        {
//            avg += img.at<char>(y,x);
//        }
//    avg/=img.cols+img.rows;
    
    int sum =0;
    int xSum=0;
    int ySum=0;
    for (int x=0; x<img.cols; x++)
        for (int y=0; y<img.rows; y++)
        {
            int val = img.at<unsigned char>(y,x);
//            val = max(0,val-avg);
            sum += val;
            xSum += val*x;
            ySum += val*y;
        }
    
    return Point2f((0.0+xSum)/sum,(0.0+ySum)/sum);
}

void SpatialAverageSpotter::showAverages()
{
    float max=0;
    float min =99999;
    for (int f=0; f<codebook->size(); f++)
    {
        
        for (int x=0; x<featureAverages[f].cols; x++)
            for (int y=0; y<featureAverages[f].rows; y++)
            {
                float val = featureAverages[f].at<float>(y,x);
                if (val > 300 || val < -300)
                    cout << "val (" << x <<","<<y<<") " << val << endl;
                if (val>max) max=val;
                if (val<min) min=val;
            }
    }
    Mat out((featureAverages[0].rows+1)*(codebook->size()),featureAverages[0].cols,CV_8U);
    for (int f=0; f<codebook->size(); f++)
    {
        
        for (int x=0; x<featureAverages[f].cols; x++)
        {
            for (int y=0; y<featureAverages[f].rows; y++)
            {
                float val = featureAverages[f].at<float>(y,x);
                out.at<char>(y+f*(featureAverages[0].rows+1),x) = 255*(val-min)/max;
            }
            out.at<char>((f+1)*(featureAverages[0].rows+1) -1,x) = 255;
        }
        
        
        
    }
    
    cvtColor(out,out,CV_GRAY2RGB);
    for (int f=0; f<codebook->size(); f++)
    {
        for (int i=0; i<5; i++)
            for (int j=0; j<5; j++)
                out.at<Vec3b>(j+f*(featureAverages[0].rows+1),i)=colorTable[f];
    }
    
    
//    cout << "max  is " << max << " min is " << min << endl;
    imshow("features",out);
    waitKey(10);
}
 

#define STAY_THRESH 1
#define MAX_REF_ITER 50
void SpatialAverageSpotter::maximizeAlignment(vector<vector<tuple<int,Point2f> > > &features)
{
    int stayCount=0;
    int iters=0;
    vector<bool> adjusted(features.size(),false);
    while (stayCount<STAY_THRESH*features.size() && iters<MAX_REF_ITER*features.size())//20
    {
        iters++;
        int imageNum= rand()%features.size();
        int imageNumOld=imageNum;
        while(1)
        {
            if (!adjusted[imageNum])
            {
                adjusted[imageNum]=true;
                break;
            }
            imageNum = (imageNum+1)%features.size();
            if ((imageNum) == imageNumOld)
            {
                adjusted.assign(features.size(),false);
                adjusted[imageNum]=true;
//                stayCount /= 2;
                stayCount=0;
                break;
            }
        }
        
        
        double shiftStep = 2*(1.0-iters/(0.0+MAX_REF_ITER*features.size()));//3* DD 2* TT
        if (shiftStep<1) shiftStep=1.0; //1.0 DD
        double scaleStep = .03*(1.0-iters/(0.0+MAX_REF_ITER*features.size()));//.01 DD
        double scaleStep2 = .06*(1.0-iters/(0.0+MAX_REF_ITER*features.size()));// .015 DD
        if (scaleStep<1) scaleStep=.003; //DD remove
        if (scaleStep2<1) scaleStep2=.006; //DD remove
        //examine gradient at each f point (discluding self)
        //if gradients agrre, make scale step
        guassColorOut(features[imageNum]);
        
        double bestScore=0;
        ExemplarTransform bestMove=STAY;
        
        double leftScore=0;
        double rightScore=0;
        double upScore=0;
        double downScore=0;
        double smallerScore=0;
        double biggerScore=0;
        double shortenScore=0;
        double hightenScore=0;
        double slimScore=0;
        double fattenScore=0;
//        double leftSmallerScore=0;
//        double rightSmallerScore=0;
//        double upSmallerScore=0;
//        double downSmallerScore=0;
//        double leftBiggerScore=0;
//        double rightBiggerScore=0;
//        double upBiggerScore=0;
//        double downBiggerScore=0;
        
              
        for (tuple<int,Point2f> f : features[imageNum])
        {
            bestScore += getUsingOffset(featureAverages[get<0>(f)],get<1>(f).x,get<1>(f).y);
            leftScore += getUsingOffset(featureAverages[get<0>(f)], get<1>(f).x-shiftStep, get<1>(f).y);
            rightScore += getUsingOffset(featureAverages[get<0>(f)], get<1>(f).x+shiftStep, get<1>(f).y);
            upScore += getUsingOffset(featureAverages[get<0>(f)], get<1>(f).x, get<1>(f).y-shiftStep);
            downScore += getUsingOffset(featureAverages[get<0>(f)], get<1>(f).x, get<1>(f).y+shiftStep);
            smallerScore += getUsingOffset(featureAverages[get<0>(f)], get<1>(f).x*(1-scaleStep), get<1>(f).y*(1-scaleStep));
            biggerScore += getUsingOffset(featureAverages[get<0>(f)], get<1>(f).x*(1+scaleStep), get<1>(f).y*(1+scaleStep));
            shortenScore += getUsingOffset(featureAverages[get<0>(f)], get<1>(f).x,get<1>(f).y*(1-scaleStep2));
            hightenScore += getUsingOffset(featureAverages[get<0>(f)], get<1>(f).x, get<1>(f).y*(1+scaleStep2));
            slimScore += getUsingOffset(featureAverages[get<0>(f)], get<1>(f).x*(1-scaleStep2), get<1>(f).y);
            fattenScore += getUsingOffset(featureAverages[get<0>(f)], get<1>(f).x*(1+scaleStep2), get<1>(f).y);
//            leftSmallerScore += getUsingOffset(featureAverages[get<0>(f)], (get<1>(f).x-shiftStep)*(1-scaleStep), get<1>(f).y*(1-scaleStep));
//            rightSmallerScore += getUsingOffset(featureAverages[get<0>(f)], (get<1>(f).x+shiftStep)*(1-scaleStep), get<1>(f).y*(1-scaleStep));
//            upSmallerScore += getUsingOffset(featureAverages[get<0>(f)], (get<1>(f).x)*(1-scaleStep), (get<1>(f).y-shiftStep)*(1-scaleStep));
//            downSmallerScore += getUsingOffset(featureAverages[get<0>(f)], (get<1>(f).x)*(1-scaleStep), (get<1>(f).y+shiftStep)*(1-scaleStep));
//            leftBiggerScore += getUsingOffset(featureAverages[get<0>(f)], (get<1>(f).x-shiftStep)*(1+scaleStep), get<1>(f).y*(1+scaleStep));
//            rightBiggerScore += getUsingOffset(featureAverages[get<0>(f)], (get<1>(f).x+shiftStep)*(1+scaleStep), get<1>(f).y*(1+scaleStep));
//            upBiggerScore += getUsingOffset(featureAverages[get<0>(f)], get<1>(f).x*(1+scaleStep), (get<1>(f).y-shiftStep)*(1+scaleStep));
//            downBiggerScore += getUsingOffset(featureAverages[get<0>(f)], get<1>(f).x*(1+scaleStep), (get<1>(f).y+shiftStep)*(1+scaleStep));
        }
        
        double test=bestScore;
        
        if (leftScore > bestScore)
        {
            bestScore=leftScore; bestMove = LEFT;
        }
        if (rightScore > bestScore)
        {
            bestScore=rightScore; bestMove = RIGHT;
        }
        if (upScore > bestScore)
        {
            bestScore=upScore; bestMove = UP;
        }
        if (downScore > bestScore)
        {
            bestScore=downScore; bestMove = DOWN;
        }
        if (smallerScore > bestScore)
        {
            bestScore=smallerScore; bestMove = SHRINK;
        }
        if (biggerScore > bestScore)
        {
            bestScore=biggerScore; bestMove = GROW;
        }
        if (shortenScore > bestScore)
        {
            bestScore=shortenScore; bestMove = SHORTEN;
        }
        if (hightenScore > bestScore)
        {
            bestScore=hightenScore; bestMove = HIGHTEN;
        }
        if (slimScore > bestScore)
        {
            bestScore=slimScore; bestMove = SLIM;
        }
        if (fattenScore > bestScore)
        {
            bestScore=fattenScore; bestMove = FATTEN;
        }
//        if (leftSmallerScore > bestScore)
//        {
//            bestScore=leftSmallerScore; bestMove = LEFT_SHRINK;
//        }
//        if (rightSmallerScore > bestScore)
//        {
//            bestScore=rightSmallerScore; bestMove = RIGHT_SHRINK;
//        }
//        if (upSmallerScore > bestScore)
//        {
//            bestScore=upSmallerScore; bestMove = UP_SHRINK;
//        }
//        if (downSmallerScore > bestScore)
//        {
//            bestScore=downSmallerScore; bestMove = DOWN_SHRINK;
//        }
//        if (leftBiggerScore > bestScore)
//        {
//            bestScore=leftBiggerScore; bestMove = LEFT_GROW;
//        }
//        if (rightBiggerScore > bestScore)
//        {
//            bestScore=rightBiggerScore; bestMove = RIGHT_GROW;
//        }
//        if (upBiggerScore > bestScore)
//        {
//            bestScore=upBiggerScore; bestMove = UP_GROW;
//        }
//        if (downBiggerScore > bestScore)
//        {
//            bestScore=downBiggerScore; bestMove = DOWN_GROW;
//        }
        
        auto fI = features[imageNum].begin();
        for(; fI!=features[imageNum].end(); fI++)
        {
            switch(bestMove)
            {
            case LEFT:
                get<1>(*fI) = Point2f(get<1>(*fI).x-shiftStep, get<1>(*fI).y);
                
                break;
            case RIGHT:
                get<1>(*fI) = Point2f(get<1>(*fI).x+shiftStep, get<1>(*fI).y);
                
                break;
            case UP:
                get<1>(*fI) = Point2f(get<1>(*fI).x, get<1>(*fI).y-shiftStep);
//                cout << "upscore:"<<upScore<<" stayscore:"<<test<<endl;
                break;
            case DOWN:
                get<1>(*fI) = Point2f(get<1>(*fI).x, get<1>(*fI).y+shiftStep);
                
                break;
            case SHRINK:
                get<1>(*fI) = Point2f(get<1>(*fI).x*(1-scaleStep), get<1>(*fI).y*(1-scaleStep));
                
                break;
            case GROW:
                get<1>(*fI) = Point2f(get<1>(*fI).x*(1+scaleStep), get<1>(*fI).y*(1+scaleStep));
                
                break;
            case SHORTEN:
                get<1>(*fI) = Point2f(get<1>(*fI).x, get<1>(*fI).y*(1-scaleStep2));
                
                break;
            case HIGHTEN:
                get<1>(*fI) = Point2f(get<1>(*fI).x, get<1>(*fI).y*(1+scaleStep2));
                
                break;
            case SLIM:
                get<1>(*fI) = Point2f(get<1>(*fI).x*(1-scaleStep2), get<1>(*fI).y);
                
                break;
            case FATTEN:
                get<1>(*fI) = Point2f(get<1>(*fI).x*(1+scaleStep2), get<1>(*fI).y);
                
                break;
//            case LEFT_SHRINK:
//                get<1>(*fI) = Point2f((get<1>(*fI).x-shiftStep)*(1-scaleStep), get<1>(*fI).y*(1-scaleStep));
                
//                break;
//            case RIGHT_SHRINK:
//                get<1>(*fI) = Point2f((get<1>(*fI).x+shiftStep)*(1-scaleStep), get<1>(*fI).y*(1-scaleStep));
                
//                break;
//            case UP_SHRINK:
//                get<1>(*fI) = Point2f((get<1>(*fI).x)*(1-scaleStep), (get<1>(*fI).y-shiftStep)*(1-scaleStep));
                
//                break;
//            case DOWN_SHRINK:
//                get<1>(*fI) = Point2f((get<1>(*fI).x)*(1-scaleStep), (get<1>(*fI).y+shiftStep)*(1-scaleStep));
                
//                break;
//            case LEFT_GROW:
//                get<1>(*fI) = Point2f((get<1>(*fI).x-shiftStep)*(1+scaleStep), get<1>(*fI).y*(1+scaleStep));
                
//                break;
//            case RIGHT_GROW:
//                get<1>(*fI) = Point2f((get<1>(*fI).x+shiftStep)*(1+scaleStep), get<1>(*fI).y*(1+scaleStep));
                
//                break;
//            case UP_GROW:
//                get<1>(*fI) = Point2f(get<1>(*fI).x*(1+scaleStep), (get<1>(*fI).y-shiftStep)*(1+scaleStep));
                
//                break;
//            case DOWN_GROW:
//                get<1>(*fI) = Point2f(get<1>(*fI).x*(1+scaleStep), (get<1>(*fI).y+shiftStep)*(1+scaleStep));
                
//                break;
            case STAY:
                break;
            }
        }
        if(bestMove == STAY)
        {
            stayCount+=1;
        }
        else if (stayCount>0)
        {
           stayCount--;
        }
        
        
        //////
        switch(bestMove)
        {
        case LEFT:
            translateImg(adjustedTrainingImages[imageNum],-shiftStep,0);
            
            break;
        case RIGHT:
            translateImg(adjustedTrainingImages[imageNum],shiftStep,0);
            
            break;
        case UP:
            translateImg(adjustedTrainingImages[imageNum],0,-shiftStep);
            break;
        case DOWN:
            translateImg(adjustedTrainingImages[imageNum],0,shiftStep);
            
            break;
        case SHRINK:
            strechImg(adjustedTrainingImages[imageNum],(1-scaleStep),(1-scaleStep));
            break;
        case GROW:
            strechImg(adjustedTrainingImages[imageNum],(1+scaleStep),(1+scaleStep));
            
            break;
        case SHORTEN:
            strechImg(adjustedTrainingImages[imageNum],1,(1-scaleStep2));
            
            break;
        case HIGHTEN:
            strechImg(adjustedTrainingImages[imageNum],1,(1+scaleStep2));
            
            break;
        case SLIM:
            strechImg(adjustedTrainingImages[imageNum],(1-scaleStep2),1);
            
            break;
        case FATTEN:
            strechImg(adjustedTrainingImages[imageNum],(1+scaleStep2),1);
            
            break;
        case STAY:
            break;
        }
        
        //////
        
        guassColorIn(features[imageNum]);
        
        cout <<"image "<<imageNum<<" move "<<bestMove<<", staycount="<<stayCount<<endl;
        showAverages();
//        imshow("adjusted",adjustedTrainingImages[imageNum]);
//        waitKey();
    }
    
    Mat averageImage(adjustedTrainingImages[0].rows,adjustedTrainingImages[0].cols,CV_8U);
    for (int x=0; x< averageImage.cols; x++)
        for (int y=0; y<averageImage.rows; y++)
        {
            int sum=0;
            for (Mat image : adjustedTrainingImages)
            {
                sum += image.at<unsigned char>(y,x);
            }
            sum /= adjustedTrainingImages.size();
            assert(sum>=0 && sum<256);
            averageImage.at<unsigned char>(y,x) = sum;
        }
    int i=0;
    for (Mat image : adjustedTrainingImages)
    {
        imshow("adjusted",image);
        cout << "image "<<i++<<endl;
        waitKey();
    }
    
    imshow("average",averageImage);
    waitKey();
}

#define STD_DEV 10//8 DD 5 TT
#define STD_DEV_RANGE 3
void SpatialAverageSpotter::guassColorIn(const vector<tuple<int,Point2f> > &feature)
{
    for (tuple<int,Point2f> f : feature)
    {
//        cout << "coloring on feature " << get<0>(f) << " at("<<get<1>(f).x<<","<<get<1>(f).y<<")"<<endl;
        for (int xOff=-STD_DEV_RANGE*STD_DEV; xOff<=STD_DEV_RANGE*STD_DEV; xOff++)
            for (int yOff=-STD_DEV_RANGE*STD_DEV; yOff<=STD_DEV_RANGE*STD_DEV; yOff++)
            {
                
                addUsingOffset(featureAverages[get<0>(f)],floor(get<1>(f).x+xOff), floor(get<1>(f).y+yOff) ,
                        codebook->getInverseDocFreq(get<0>(f))/feature.size()*
                        exp(-((pow(xOff,2)/(2*pow(STD_DEV,2))) + ((pow(yOff,2)/(2*pow(STD_DEV,2)))))));
//                cout << "sum "<<  getUsingOffset(featureAverages[get<0>(f)],get<1>(f).x+xOff,get<1>(f).y+yOff) << endl;
//                if ((int)(get<1>(f).x+xOff) == 0) cout <<"x="<<get<1>(f).x<<" off="<<xOff<<endl;
            }
    }
}

void SpatialAverageSpotter::guassColorOut(const vector<tuple<int,Point2f> > &feature)
{
    for (tuple<int,Point2f> f : feature)
    {
        for (int xOff=-STD_DEV_RANGE*STD_DEV; xOff<=STD_DEV_RANGE*STD_DEV; xOff++)
            for (int yOff=-STD_DEV_RANGE*STD_DEV; yOff<=STD_DEV_RANGE*STD_DEV; yOff++)
            {
                addUsingOffset(featureAverages[get<0>(f)],floor(get<1>(f).x+xOff), floor(get<1>(f).y+yOff) , 
                        -codebook->getInverseDocFreq(get<0>(f))/feature.size()*
                        exp(-((pow(xOff,2)/(2*pow(STD_DEV,2))) + ((pow(yOff,2)/(2*pow(STD_DEV,2)))))));
            }
    }
}

float SpatialAverageSpotter::getUsingOffset(Mat &featureAverage, double xOff, double yOff)
{
    double x = featureAverage.cols/2 +xOff;
    double y = featureAverage.rows/2 +yOff;
    if (x >0 && y>0 && x<featureAverage.cols && y<featureAverage.rows)
        return featureAverage.at<float>(y,x);
    return 0;
}

void SpatialAverageSpotter::addUsingOffset(Mat &featureAverage, int xOff, int yOff, float add)
{

    int x = featureAverage.cols/2 +xOff;
    int y = featureAverage.rows/2 +yOff;
    if (x >0 && y>0 && x<featureAverage.cols && y<featureAverage.rows)
        featureAverage.at<float>(y,x) += add;
    
//    if (xOff==0)
//        cout <<"("<<xOff<<","<<yOff<<") on line " << add << endl;
//    if (xOff==1)
//        cout <<"("<<xOff<<","<<yOff<<") off+ line " << add << endl;
//    if (xOff==-1)
//        cout <<"("<<xOff<<","<<yOff<<") off- line " << add << endl;
}

bool SpatialAverageSpotter::testSpotting(string dirPath)
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

void SpatialAverageSpotter::produceHeatMap(string fileName)
{
    Mat img = imread(fileName, CV_LOAD_IMAGE_GRAYSCALE);
    resize(img,img,Size(0,0),2,2);
    threshold(img,img,120.0,255,THRESH_BINARY);
    //resize(img,img,Size(img.cols/2,img.rows/2));
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

//double SpatialAverageSpotter::detect(Mat img)
//{
//    vector<int> here(codebook->size());
    
    
//    vector<KeyPoint> keypoints;
//    Mat desc;
//    (*detector)( img, Mat(), keypoints, desc);
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

map< int, map< int, vector<int> > >* SpatialAverageSpotter::buildFeatureMap(Mat img)
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
    waitKey(5);
    
    return fm;
}

#define SCORE_SCALE_RANGE .6
double SpatialAverageSpotter::detect(map< int, map< int, vector<int> > >* fm, Point2i corner)
{
    double startScaling = (0.0+BASE_SIZE)/windowHeight;
    float score1 = score(fm,corner,startScaling);
    float scoreShrink = score(fm,corner,startScaling-.1);
    float scoreGrow = score(fm,corner,startScaling+.1);
    
    if (score1>=scoreShrink && score1>=scoreGrow)
    {
//        if (scoreShrink>60)
//        cout << "score1= "<<score1<<", " << startScaling << endl;
        return max((float)0.0,score1);
    }
    
    if (scoreShrink>scoreGrow)
    {
        double scaling = startScaling-.1;
        while(scaling > startScaling-SCORE_SCALE_RANGE)
        {
            scaling -=.1;
            float newScore = score(fm,corner,scaling);
            if (newScore>scoreShrink)
                scoreShrink = newScore;
            else
                break;
        }
//        if (scoreShrink>60)
//        cout << "scoreShrink= "<<scoreShrink<<", " << scaling << endl;
//        assert(scoreShrink<14);
        
        return max((float)0.0,scoreShrink);
    }
    else
    {
        double scaling = startScaling+.1;
        while(scaling < startScaling+SCORE_SCALE_RANGE)
        {
            scaling +=.1;
            float newScore = score(fm,corner,scaling);
            if (newScore>scoreGrow)
                scoreGrow = newScore;
            else
                break;
        }
//        if (scoreShrink>60)
//        cout << "scoreGrow= "<<scoreGrow<<", " << scaling << endl;
        return max((float)0.0,scoreGrow);
    }
    
}

float SpatialAverageSpotter::score(map< int, map< int, vector<int> > >* fm, Point2i corner, double scaling)
{
    
    auto it=fm->begin();
    for (int x=corner.x; x<corner.x+windowWidth; x++)
    {
        it=fm->find(x);
        if (it != fm->end())
            break;
    }
    float score=0;
    for (; it != fm->end(); ++it)
    {
        int x = (*it).first;
        if (x>corner.x+windowWidth)
            break;
        auto it2 = (*it).second.begin();
        for (int y=corner.y; y<corner.y+windowHeight; y++)
        {
            it2=(*it).second.find(y);
            if (it2 != (*it).second.end())
                break;
        }
        for (; it2 != (*it).second.end(); ++it2)
        {
            int y = (*it2).first;
            if (y>corner.y+windowHeight)
                break;
            
            int centeredX=x-(corner.x+windowWidth/2);
            int centeredY=y-(corner.y+windowHeight/2);
            for (int f : (*it2).second)
            { 
                score-=penalty;
                score+=getUsingOffset(featureAverages[f],centeredX*scaling, centeredY*scaling);
//                cout << "scored("<<score<<") f"<<f<<" at ["<<scaling<<"]rel("<< centeredX*scaling<<","<<centeredY*scaling <<")="<<getUsingOffset(featureAverages[f],centeredX*scaling, centeredY*scaling)<<endl;
            }
        }
           
    }
    
    return score;
}

void SpatialAverageSpotter::putPointsOn(Mat &img, map< int, map< int, vector<int> > >* fm, Point2i corner)
{
    auto it=fm->begin();
    for (int x=corner.x; x<corner.x+windowWidth; x++)
    {
        it=fm->find(x);
        if (it != fm->end())
            break;
    }
    
    for (; it != fm->end(); ++it)
    {
        int x = (*it).first;
        if (x>corner.x+windowWidth)
            break;
        auto it2 = (*it).second.begin();
        for (int y=corner.y; y<corner.y+windowHeight; y++)
        {
            it2=(*it).second.find(y);
            if (it2 != (*it).second.end())
                break;
        }
        for (; it2 != (*it).second.end(); ++it2)
        {
            int y = (*it2).first;
            if (y>corner.y+windowHeight)
                break;
            for (int f : (*it2).second)
            {
                circle(img,Point2i(x,y),(int)learned[f],Scalar(colorTable[f]));
                
            } 
        }
           
    }
}
