#include "wordseparator.h"
 
#include <math.h>



#define ANCHOR_L 3.05
#define ANCHOR_R 2.4

WordSeparator::WordSeparator()
{
}

QImage WordSeparator::removeFirstWord(QImage &from)
{
    WordProfile profile(from,true,0);
    QVector<int> mins = profile.getLocalMins();
    /*
      Approaches:
      1. It about in the middle, so sut at middle local min
      2. It's where whitespace is so find the min thats 0.
      3. The mins for a single word will cluster tighter together

      */

    //int selectedMin = 0;
    int selectedCut;

    QVector<int> zeros;
    QVector<int> values;
    int sum=0;
    for (int i=0; i<mins.size(); i++)
    {
        values.append(profile.getValue(mins[i]));
        sum += profile.getValue(mins[i]);
        //printf("val: %d\n",profile.getValue(mins[i]));

        if (profile.getValue(mins[i])==0)
                zeros.append(mins[i]);
    }
    if (zeros.size() > 0)
    {
        int min = abs((from.width()/2)-zeros[0]);
        selectedCut = zeros[0];
        for (int i=1; i<zeros.size(); i++)
        {
            if (abs((from.width()/2)-zeros[i]) < min)
            {
                min = abs((from.width()/2)-zeros[i]);
                selectedCut = zeros[i];
            }
        }
    }
    else
    {
        selectedCut=windowScanWidestMin(profile,5);
    }

    printf("Cut at %d\n",selectedCut);
    QImage toReturn = from.copy(0,0,selectedCut,from.height());
    from = from.copy(selectedCut,0,from.width()-selectedCut,from.height());
    return toReturn;
}

int WordSeparator::windowScanWidestMin(WordProfile &profile, int size)
{
    QVector<int> mins = profile.getLocalMins();
    int minSum = std::numeric_limits<int>::max();
    int index = 0;
    foreach (int min, mins)
    {
        int lastLocalSum = std::numeric_limits<int>::max();
        for (int i=min; i>=0; i--)
        {
            int sum = 0;
            for (int j=0; j<size; j++)
                sum +=profile.getValue(i+j);
            if (sum < minSum)
            {
                minSum = sum;
                index = i;
            }
            if (sum > lastLocalSum)
                break;
            lastLocalSum=sum;
        }
    }
    return index + size/2;
}






QVector<QImage> WordSeparator::minCut(QImage &img)
{
    int num_pix = img.width()*img.height();
    //double pix_vals[num_pix];
    int invDistMap[num_pix];
    
    computeInverseDistanceMap(img,invDistMap);
    QVector<int> firstImgBlackPixelIndexes;
    int maxflow = pixelsOfSeparation(invDistMap,img.width(),img.height(),img,firstImgBlackPixelIndexes);
    
    int firstFarthestRightPixel = 0;
    int secondFarthestLeftPixel = 0;
    
    QImage firstImg = img.copy(0,0,img.width(),img.height());
    firstImg.fill(255);
    QImage secondImg = img.copy(0,0,img.width(),img.height());
    
    foreach(int index, firstImgBlackPixelIndexes)
    {
        int x = index%img.width();
        int y = index/img.width();
        firstImg.setPixel(x,y,0);
        secondImg.setPixel(x,y,255);
        
        if (x>firstFarthestRightPixel)
            firstFarthestRightPixel=x;
    }
    bool notFound = true;
    for (int i=0; i<img.width() && notFound; i++)
    {
        for (int j=0; j<img.height() && notFound; j++)
        {
            if (qGray(secondImg.pixel(i,j))==0)
            {
                secondFarthestLeftPixel=i;
                notFound = false;
            }
        }
    }
    
    //printf("maxflow= %d, first right border:%d, second left border:%d\n",maxflow,firstFarthestRightPixel,secondFarthestLeftPixel);
    
    firstImg = firstImg.copy(0,0,firstFarthestRightPixel+1,firstImg.height());
    secondImg = secondImg.copy(secondFarthestLeftPixel, 0, secondImg.width()-secondFarthestLeftPixel, secondImg.height());
    
    
   
    QVector<QImage> ret;
    ret.append(firstImg);
    ret.append(secondImg);
    return ret;
}


QVector<QImage> WordSeparator::cutNames(QImage &img)
{
    
    int maxFlow_firstCut;
    int pixelWidth_firstCut_left;
    int pixelWidth_firstCut_right;
    int pixelCount_firstCut_left;
    int pixelCount_firstCut_right;
    
    int num_pix = img.width()*img.height();
    int invDistMap[num_pix];
    
    QVector<int> firstImgBlackPixelIndexes;
    
    firstImgBlackPixelIndexes.clear();
    
    computeInverseDistanceMap(img,invDistMap);//do we need a new distance map each cut?
    
    maxFlow_firstCut = pixelsOfSeparation(invDistMap,img.width(),img.height(),img,firstImgBlackPixelIndexes);
    
    int firstFarthestRightPixel = 0;
    int secondFarthestLeftPixel = 0;
    
    QImage firstImg = img.copy(0,0,img.width(),img.height());
    firstImg.fill(255);
    QImage secondImg = img.copy(0,0,img.width(),img.height());
    
    foreach(int index, firstImgBlackPixelIndexes)
    {
        int x = index%img.width();
        int y = index/img.width();
        firstImg.setPixel(x,y,0);
        secondImg.setPixel(x,y,255);
        
        if (x>firstFarthestRightPixel)
            firstFarthestRightPixel=x;
    }
    bool notFound = true;
    for (int i=0; i<img.width() && notFound; i++)
    {
        for (int j=0; j<img.height() && notFound; j++)
        {
            if (qGray(secondImg.pixel(i,j))==0)
            {
                secondFarthestLeftPixel=i;
                notFound = false;
            }
        }
    }
    
    QImage leftImg = firstImg.copy(0,0,firstFarthestRightPixel+1,firstImg.height());
    QImage rightImg = secondImg.copy(secondFarthestLeftPixel, 0, secondImg.width()-secondFarthestLeftPixel, secondImg.height());
    if (maxFlow_firstCut<0)
        maxFlow_firstCut = INT_POS_INFINITY;
    pixelWidth_firstCut_left = leftImg.width();
    pixelWidth_firstCut_right = rightImg.width();
    pixelCount_firstCut_left = firstImgBlackPixelIndexes.size();
    pixelCount_firstCut_right = num_pix - pixelCount_firstCut_left;
    
    //////
    int maxFlow_secondCutL;
    int pixelWidth_secondCutL_left;
    int pixelWidth_secondCutL_right;
    int pixelCount_secondCutL_left;
    int pixelCount_secondCutL_right;
    
    num_pix = leftImg.width()*leftImg.height();
    
    firstImgBlackPixelIndexes.clear();
    
    computeInverseDistanceMap(leftImg,invDistMap);//do we need a new distance map each cut?
    
    maxFlow_secondCutL = pixelsOfSeparation(invDistMap,leftImg.width(),leftImg.height(),leftImg,firstImgBlackPixelIndexes);
    
    firstFarthestRightPixel = 0;
    secondFarthestLeftPixel = 0;
    
    firstImg = leftImg.copy(0,0,leftImg.width(),leftImg.height());
    firstImg.fill(255);
    secondImg = leftImg.copy(0,0,leftImg.width(),leftImg.height());
    
    foreach(int index, firstImgBlackPixelIndexes)
    {
        int x = index%leftImg.width();
        int y = index/leftImg.width();
        firstImg.setPixel(x,y,0);
        secondImg.setPixel(x,y,255);
        
        if (x>firstFarthestRightPixel)
            firstFarthestRightPixel=x;
    }
    notFound = true;
    for (int i=0; i<leftImg.width() && notFound; i++)
    {
        for (int j=0; j<leftImg.height() && notFound; j++)
        {
            if (qGray(secondImg.pixel(i,j))==0)
            {
                secondFarthestLeftPixel=i;
                notFound = false;
            }
        }
    }
    
    QImage leftImg2L = firstImg.copy(0,0,firstFarthestRightPixel+1,firstImg.height());
    QImage rightImg2L = secondImg.copy(secondFarthestLeftPixel, 0, secondImg.width()-secondFarthestLeftPixel, secondImg.height());
    if (maxFlow_secondCutL<0)
        maxFlow_secondCutL = INT_POS_INFINITY;
    pixelWidth_secondCutL_right = rightImg2L.width();
    pixelWidth_secondCutL_left = leftImg2L.width();
    pixelCount_secondCutL_left = firstImgBlackPixelIndexes.size();
    pixelCount_secondCutL_right = pixelCount_firstCut_left - pixelCount_secondCutL_left;
    
    ////////b
    
    int maxFlow_secondCutR;
    int pixelWidth_secondCutR_left;
    int pixelWidth_secondCutR_right;
    int pixelCount_secondCutR_left;
    int pixelCount_secondCutR_right;
    
    num_pix = rightImg.width()*rightImg.height();
    
    firstImgBlackPixelIndexes.clear();
    
    computeInverseDistanceMap(rightImg,invDistMap);//do we need a new distance map each cut?
    
    maxFlow_secondCutR = pixelsOfSeparation(invDistMap,rightImg.width(),rightImg.height(),rightImg,firstImgBlackPixelIndexes);
    
    firstFarthestRightPixel = 0;
    secondFarthestLeftPixel = 0;
    
    firstImg = rightImg.copy(0,0,rightImg.width(),rightImg.height());
    firstImg.fill(255);
    secondImg = rightImg.copy(0,0,rightImg.width(),rightImg.height());
    
    foreach(int index, firstImgBlackPixelIndexes)
    {
        int x = index%rightImg.width();
        int y = index/rightImg.width();
        firstImg.setPixel(x,y,0);
        secondImg.setPixel(x,y,255);
        
        if (x>firstFarthestRightPixel)
            firstFarthestRightPixel=x;
    }
    notFound = true;
    for (int i=0; i<rightImg.width() && notFound; i++)
    {
        for (int j=0; j<rightImg.height() && notFound; j++)
        {
            if (qGray(secondImg.pixel(i,j))==0)
            {
                secondFarthestLeftPixel=i;
                notFound = false;
            }
        }
    }
    
    QImage leftImg2R = firstImg.copy(0,0,firstFarthestRightPixel+1,firstImg.height());
    QImage rightImg2R = secondImg.copy(secondFarthestLeftPixel, 0, secondImg.width()-secondFarthestLeftPixel, secondImg.height());
    if (maxFlow_secondCutR<0)
        maxFlow_secondCutR = INT_POS_INFINITY;
    pixelWidth_secondCutR_right = rightImg2R.width();
    pixelWidth_secondCutR_left = leftImg2R.width();
    pixelCount_secondCutR_left = firstImgBlackPixelIndexes.size();
    pixelCount_secondCutR_right = pixelCount_firstCut_right - pixelCount_secondCutR_left;
    ////////
    
    
    
    //printf("Cut %d: maxflow=%d, size=%d\n",numOfCuts,cutFlows[numOfCuts],sizeOfCuts[numOfCuts]);
    
    leftImg.save("first_left.pgm");
    rightImg.save("first_right.pgm");
    leftImg2L.save("secondL_left.pgm");
    rightImg2L.save("secondL_right.pgm");
    leftImg2R.save("secondR_left.pgm");
    rightImg2R.save("secondR_right.pgm");
    
    
    QVector<QImage> ret;
    //ret.append(leftImg);
//    printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n",
//           maxFlow_firstCut,pixelWidth_firstCut_left,pixelWidth_firstCut_right,pixelCount_firstCut_left,pixelCount_firstCut_right,
//           maxFlow_secondCutL,pixelWidth_secondCutL_left,pixelWidth_secondCutL_right,pixelCount_secondCutL_left,pixelCount_secondCutL_right,
//           maxFlow_secondCutR,pixelWidth_secondCutR_left,pixelWidth_secondCutR_right,pixelCount_secondCutR_left,pixelCount_secondCutR_right,
//           maxFlow_firstCut-maxFlow_secondCutL,maxFlow_firstCut-maxFlow_secondCutR);
    
    //Generated by J48 on data from res1,2,3
    char sit = ' ';
    if (pixelWidth_firstCut_right <= 167)
    {
        if (maxFlow_firstCut-maxFlow_secondCutR <= -31874)
        {
            if (pixelCount_secondCutL_right <= 786)
                sit='b';
            else
                sit='l';
        }
        else
        {
            sit='b';
        }
    }
    else
    {
        if (pixelWidth_secondCutR_right <= 149)
        {
            if (maxFlow_firstCut-maxFlow_secondCutR <= -78228)
            {
                if (pixelWidth_firstCut_left <= 79)
                    sit='r';
                else
                    sit='o';
            }
            else
            {
                sit='r';
            }
        }
        else
        {
            if (maxFlow_secondCutR <= 33942)
            {
                sit='b';
            }
            else
            {
                if (pixelWidth_secondCutR_left <= 116)
                    sit='o';
                else
                    sit='b';
            }
        }
    }
    
    if (sit=='o' || sit=='r')
    {
        ret.append(leftImg);
        ret.append(rightImg);
    }
    else if (sit=='l')
    {
        ret.append(leftImg2L);
        ret.append(rightImg2L);
        ret.append(rightImg);
    }
    else
    {
        QImage copy = img.copy(0,0,img.width(),img.height());
        ret.append(copy);
    }
    
    return ret;
}

/* There are two parameters to evaluate if we have the correct cut:
   maxflow of the cut and surronding cuts and the number of pixels in the cut.
   for a decision tree, these 
   -num of pixels in cut
   -difference between maxflow and maxflow of all other cuts
   -width of cut, or relative width of cut
   -
   
  */
QVector<QImage> WordSeparator::recursiveCutWordToFirstLetter(QImage &img)
{
    QVector<QImage> cuts;
    QVector<int> cutFlows;
    QVector<int> sizeOfCuts;
    int numOfCuts = 0;
    
    cuts.append(img);
    cutFlows.append(0);
    sizeOfCuts.append(0);
    
    int num_pix = img.width()*img.height();
    int invDistMap[num_pix];
    
    QVector<int> firstImgBlackPixelIndexes;
    while(true)
    {
        firstImgBlackPixelIndexes.clear();
        num_pix = cuts[numOfCuts].width()*cuts[numOfCuts].height();
        
        computeInverseDistanceMap(cuts[numOfCuts],invDistMap);//do we need a new distance map each cut?
        
        int maxflow = pixelsOfSeparation(invDistMap,cuts[numOfCuts].width(),cuts[numOfCuts].height(),cuts[numOfCuts],firstImgBlackPixelIndexes);
        
        int firstFarthestRightPixel = 0;
        
        QImage firstImg = cuts[numOfCuts].copy(0,0,cuts[numOfCuts].width(),cuts[numOfCuts].height());
        firstImg.fill(255);
        
        foreach(int index, firstImgBlackPixelIndexes)
        {
            int x = index%cuts[numOfCuts].width();
            int y = index/cuts[numOfCuts].width();
            firstImg.setPixel(x,y,0);
            
            if (x>firstFarthestRightPixel)
                firstFarthestRightPixel=x;
        }
        
        cuts.push_back(firstImg.copy(0,0,firstFarthestRightPixel+1,firstImg.height()));
        if (maxflow<0)
            maxflow = INT_POS_INFINITY;
        cutFlows.append(maxflow);
        sizeOfCuts.append(firstImgBlackPixelIndexes.size());
        numOfCuts++;
        
        //printf("Cut %d: maxflow=%d, size=%d\n",numOfCuts,cutFlows[numOfCuts],sizeOfCuts[numOfCuts]);
        QString numstr;
        numstr.setNum(numOfCuts);
        QString filename = "cut";
        filename+=numstr;
        filename+=".pgm";
        cuts[numOfCuts].save(filename);
        
        
        if (numOfCuts>4)
        {
            break;
        }
    }
    
    
//    printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n",
//           cuts[1].width(),cuts[2].width(),cuts[3].width(),cuts[4].width(),cuts[5].width(),
//           sizeOfCuts[1],sizeOfCuts[2],sizeOfCuts[3],sizeOfCuts[4],sizeOfCuts[5],
//           cutFlows[1],cutFlows[2],cutFlows[3],cutFlows[4],cutFlows[5],
//           cutFlows[2]-cutFlows[1],cutFlows[3]-cutFlows[2],cutFlows[4]-cutFlows[3],cutFlows[5]-cutFlows[4]);
    char sit = ' ';
    if (cuts[1].width() <= 44)
    {
        if (sizeOfCuts[1] <= 309)
            sit='0';
        else
            sit='1';
    }
    else
    {
        if (cuts[3].width() <= 35)
        {
            if (sizeOfCuts[2] <= 299)
            {
                if (cuts[2].width() <= 38)
                    sit='1';
                else
                    sit='2';
            }
            else
            {
                if (cutFlows[4] <= 132914)
                {
                    if (cuts[4].width() <= 12)
                    {
                        sit='2';
                    }
                    else
                    {
                        if (sizeOfCuts[3] <= 147)
                        {
                                sit='2';
                        }
                        else
                        {
                            if (sizeOfCuts[2] <= 599)
                            {
                                if (sizeOfCuts[1] <= 690)
                                    sit='3';
                                else
                                    sit='2';
                            }
                            else
                            {
                                sit='3';
                            }
                        
                        }
                    }
                }
                else
                { 
                            sit='2';
                }
            }
        }
        else
        {
            if (cuts[5].width() <= 25)
            {
                if (cuts[2].width() <= 54)
                {
                                sit='2';
                }
                else
                {
                    if (cutFlows[4]-cutFlows[3] <= 27599)
                    {
                        if (cutFlows[2] <= 55825)
                        {
                            if (sizeOfCuts[1] <= 1470)
                                sit='2';
                            else
                                sit='4';
                        }
                        else
                        {
                            if (cutFlows[4] <= 103641)
                            {
                                if (cutFlows[3] <= 70113)
                                    sit='4';
                                else
                                    sit='3';
                            }
                            else
                            {
                                sit='4';
                            }
                        }
                    }
                    else
                    {
                        sit='3';
                    }
                    
                }
            }
            else
            {
                if (cuts[4].width() <= 50)
                    sit='4';
                else
                    sit='5';
            }
        }
    }
    
    //return top two guesse 
    QVector<QImage> ret;
    if (sit=='0')
    {
        ret.append(cuts[0]);
        ret.append(cuts[1]);
    }
    else if (sit=='1')
    {
        ret.append(cuts[1]);
        ret.append(cuts[2]);
    }
    else if (sit=='2')
    {
        ret.append(cuts[2]);
        ret.append(cuts[3]);
        //maybe 1
    }
    else if (sit=='3')
    {
        ret.append(cuts[3]);
        ret.append(cuts[2]);
    }
    else if (sit=='4')
    {
        ret.append(cuts[4]);
        ret.append(cuts[3]);
    }
    else if (sit=='5')
    {
        ret.append(cuts[5]);
        ret.append(cuts[4]);
    }
                                
    return ret;
}

//Meijster distance <http://fab.cba.mit.edu/classes/S62.12/docs/Meijster_distance.pdf>
//This can be parallelized. Should probably flip from column to row first
void WordSeparator::computeInverseDistanceMap(QImage &img, int* out)
{
    int maxDist=0;
    int g[img.width()*img.height()];
    for (int x=0; x<img.width(); x++)
    {
        if (qGray(img.pixel(x,0))==BLACK)
        {
            g[x+0*img.width()]=0;
        }
        else
        {
            g[x+0*img.width()]=INT_POS_INFINITY;//img.width()*img.height();
        }
        
        for (int y=1; y<img.height(); y++)
        {
            if (qGray(img.pixel(x,y))==BLACK)
            {
                g[x+y*img.width()]=0;
            }
            else
            {
                if (g[x+(y-1)*img.width()] != INT_POS_INFINITY)
                    g[x+y*img.width()]=1+g[x+(y-1)*img.width()];
                else
                    g[x+y*img.width()] = INT_POS_INFINITY;
            }
        }
        
        for (int y=img.height()-2; y>=0; y--)
        {
            if (g[x+(y+1)*img.width()]<g[x+y*img.width()])
            {
                if (g[x+(y+1)*img.width()] != INT_POS_INFINITY)
                    g[x+y*img.width()]=1+g[x+(y+1)*img.width()];
                else
                    g[x+y*img.width()] = INT_POS_INFINITY;
            }
        }
        
        /*if(x==20)
        {
            for (int y=0; y<img.height(); y++)
            {
                printf("%d .. %d\n",qGray(img.pixel(x,y)),g[x+y*img.width()]);
            }
        }*/
    }
    
    int q;
    int s[img.width()];
    int t[img.width()];
    int w;
    for (int y=0; y<img.height(); y++)
    {
        q=0;
        s[0]=0;
        t[0]=0;
        for (int u=1; u<img.width();u++)
        {
            while (q>=0 && f(t[q],s[q],y,img.width(),g) > f(t[q],u,y,img.width(),g))
            {
                q--;
            }
            
            if (q<0)
            {
                q=0;
                s[0]=u;
            }
            else
            {
                w = SepPlusOne(s[q],u,y,img.width(),g);
                if (w<img.width())
                {
                    q++;
                    s[q]=u;
                    t[q]=w;
                }
            }
        }
        
        for (int u=img.width()-1; u>=0; u--)
        {
            out[u+y*img.width()]= f(u,s[q],y,img.width(),g);
            if (out[u+y*img.width()] > maxDist)
                maxDist = out[u+y*img.width()];
            if (u==t[q])
                q--;
        }
    }
    
    //invert
//    printf("maxDist=%d\n",maxDist);
    maxDist++;
//    double normalizer = (24.0/maxDist);
    double normalizer = (20.0/2000);
    QImage mark = img.copy(0,0,img.width(),img.height());
    QVector<QPoint> workingStack;
    QVector<QPoint> growingComponent;
    
    
    int newmax = 0;
    for (int q = 0; q < img.width()*img.height(); q++)
    {   
        //out[q] = pow(6,24-out[q]*normalizer)/pow(6,20);
        if (qGray(img.pixel(q%img.width(),q/img.width()))==BLACK && qGray(mark.pixel(q%img.width(),q/img.width()))==BLACK)
        {
            //fill bias
            QPoint p(q%img.width(),q/img.width());
            workingStack.push_back(p);
            mark.setPixel(p,WHITE);
            while (!workingStack.isEmpty())
            {   
                QPoint cur = workingStack.back();
                workingStack.pop_back();
                growingComponent.append(cur);
                
                
                
                
                if (cur.x()>0 && qGray(mark.pixel(cur.x()-1,cur.y())) == BLACK)
                {
                    QPoint pp(cur.x()-1,cur.y());
                    workingStack.push_back(pp);
                    mark.setPixel(pp,WHITE);
                }
                
                
                if (cur.x()<mark.width()-1 && qGray(mark.pixel(cur.x()+1,cur.y())) == BLACK)
                {
                    QPoint pp(cur.x()+1,cur.y());
                    workingStack.push_back(pp);
                    mark.setPixel(pp,WHITE);
                    
                }
                if (cur.y()<mark.height()-1 && qGray(mark.pixel(cur.x(),cur.y()+1)) == BLACK)
                {
                    QPoint pp(cur.x(),cur.y()+1);
                    workingStack.push_back(pp);
                    mark.setPixel(pp,WHITE);
                }
                if (cur.y()>0 && qGray(mark.pixel(cur.x(),cur.y()-1)) == BLACK)
                {
                    QPoint pp(cur.x(),cur.y()-1);
                    workingStack.push_back(pp);
                    mark.setPixel(pp,WHITE);
                }
            }
            int cc_size = growingComponent.size();
            while (!growingComponent.isEmpty())
            {
                QPoint cur = growingComponent.back();
                growingComponent.pop_back();
                int index = cur.x()+img.width()*cur.y();
                out[index] = pow(6,20-out[index]*normalizer)/pow(6,16) + 4*std::min(cc_size,500);
            }
            
        }
        else if (qGray(img.pixel(q%img.width(),q/img.width()))!=BLACK)
        {
            out[q] = pow(6,20-out[q]*normalizer)/pow(6,16);
        }

        if (out[q]>newmax)
            newmax=out[q];
    }
    
//    printf("newMax:%d\n",newmax);
    QImage debug=img.copy(0,0,img.width(),img.height());//(img.width(),img.height(),img.format());
    for (int i=0; i<debug.width(); i++)
    {
        for (int j=0; j<debug.height(); j++)
            debug.setPixel(i,j,(int)((out[i+j*debug.width()]/((double)newmax))*255));
        
    }
    debug.save("./inv_dist_map.pgm");
}

int WordSeparator::f(int x, int i, int y, int m, int* g)
{
    if (g[i+y*m]==INT_POS_INFINITY || x==INT_POS_INFINITY)
        return INT_POS_INFINITY;
    return (x-i)*(x-i) + g[i+y*m]*g[i+y*m];
}

int WordSeparator::SepPlusOne(int i, int u, int y, int m, int* g)
{
    if (g[u+y*m] == INT_POS_INFINITY)// && g[i+y*m] != INT_POS_INFINITY)
    {
        //printf("Sep is inginite\n");
        return INT_POS_INFINITY;
    }
    return 1 + ((u*u)-(i*i)+g[u+y*m]*g[u+y*m]-(g[i+y*m]*g[i+y*m])) / (2*(u-i));
}


//Uses Boykov graph cut
int WordSeparator::pixelsOfSeparation(int* invDistMap, int width, int height, QImage &img, QVector<int> &out)
{
    typedef Graph<int,int,int> GraphType;
    GraphType *g = new GraphType(width*height, 2*(width-1)*(height-1)-(height+width)); 
    
    for (int i=0; i<width*height; i++)
    {
        g->add_node();
    }
    
    QImage debug = img.copy(0,0,img.width(),img.height());
    
    //find source pixels
    int count_source = height*ANCHOR_L;
    for (int i=0; count_source>0 && i<width; i++)
    {
        for (int j=0; count_source>0 && j<height; j++)
        {
            if (qGray(img.pixel(i,j))==BLACK)
            {
                int index = i+width*j;
                g -> add_tweights(index, INT_POS_INFINITY,0);//invDistMap[index], 0);
//                debug.setPixel(i,j,150);
                count_source--;
            }
        }
    }
    
    
    
    
    //diag method
//    QImage mark = img.copy(0,0,img.width(),img.height());
//    QVector<QPoint> workingStack;
//    for (int o=0; o<width && count_source>0; o++)
//    {
//        for (int i=0; i<=o && i<height && count_source>0; i++)
//        {
//            if (qGray(img.pixel(o-i,i))==BLACK)
//            {
//                int index = (o-i)+width*i;
//                g -> add_tweights(index, INT_POS_INFINITY,0);
//                count_source--;
//                debug.setPixel((o-i),i,150);
                
                //fill
//                QPoint p(o-i,i);
//                workingStack.push_back(p);
//                while (!workingStack.isEmpty() && count_source>0)
//                {   
//                    QPoint cur = workingStack.back();
//                    workingStack.pop_back();
//                    int index = cur.x()+width*cur.y();
//                    g -> add_tweights(index, INT_POS_INFINITY,0);
//                    //debug.setPixel(cur,150);
//                    count_source--;
                    
//                    mark.setPixel(cur,WHITE);
                    
//                    if (cur.x()>0 && qGray(mark.pixel(cur.x()-1,cur.y())) == BLACK)
//                    {
//                        QPoint pp(cur.x()-1,cur.y());
//                        workingStack.push_back(pp);
//                    }
                    
                    
//                    if (cur.x()<mark.width()-1 && qGray(mark.pixel(cur.x()+1,cur.y())) == BLACK)
//                    {
//                        QPoint pp(cur.x()+1,cur.y());
//                        workingStack.push_back(pp);
                        
//                    }
//                    if (cur.y()<mark.height()-1 && qGray(mark.pixel(cur.x(),cur.y()+1)) == BLACK)
//                    {
//                        QPoint pp(cur.x(),cur.y()+1);
//                        workingStack.push_back(pp);
//                    }
//                    if (cur.y()>0 && qGray(mark.pixel(cur.x(),cur.y()-1)) == BLACK)
//                    {
//                        QPoint pp(cur.x(),cur.y()-1);
//                        workingStack.push_back(pp);
//                    }
//                }
//            }
//        }
//    }
//    workingStack.clear();
    
    int count_sink=height*ANCHOR_R;
    
    //find sink pixels
//    for (int i=width-1; count_sink>0 && i>=0; i--)
//    {
//        for (int j=height-1; count_sink>0 && j>=0; j--)
//        {
//            if (qGray(img.pixel(i,j))==BLACK)
//            {
//                int index = i+width*j;
//                g -> add_tweights(index, 0, INT_POS_INFINITY);//invDistMap[index]);
////                debug.setPixel(i,j,150);
//                count_sink--;
//            }
//        }
//    }
    
    //diag method
    for (int o=0; o<width && count_sink>0; o++)
    {
        for (int i=0; i<=o && i<height && count_sink>0; i++)
        {
            //debug.setPixel((width-1)-(o-i),(height-1)-i,220);
            if (qGray(img.pixel((width-1)-(o-i),(height-1)-i))==BLACK)
            {
                int index = ((width-1)-(o-i))+width*((height-1)-i);
                g -> add_tweights(index, 0, INT_POS_INFINITY);
                count_sink--;
//                debug.setPixel((width-1)-(o-i),(height-1)-i,150);
                
                //fill
//                QPoint p((width-1)-(o-i),(height-1)-i);
//                workingStack.push_back(p);
//                while (!workingStack.isEmpty() && count_sink>0)
//                {   
//                    QPoint cur = workingStack.back();
//                    workingStack.pop_back();
//                    int index = cur.x()+width*cur.y();
//                    g -> add_tweights(index, 0, INT_POS_INFINITY);
//                    //debug.setPixel(cur,150);
//                    count_sink--;
                    
//                    mark.setPixel(cur,WHITE);
                    
//                    if (cur.x()<mark.width()-1 && qGray(mark.pixel(cur.x()+1,cur.y())) == BLACK)
//                    {
//                        QPoint pp(cur.x()+1,cur.y());
//                        workingStack.push_back(pp);
                        
//                    }
                    
                    
//                    if (cur.x()>0 && qGray(mark.pixel(cur.x()-1,cur.y())) == BLACK)
//                    {
//                        QPoint pp(cur.x()-1,cur.y());
//                        workingStack.push_back(pp);
//                    }
//                    if (cur.y()>0 && qGray(mark.pixel(cur.x(),cur.y()-1)) == BLACK)
//                    {
//                        QPoint pp(cur.x(),cur.y()-1);
//                        workingStack.push_back(pp);
//                    }
//                    if (cur.y()<mark.height()-1 && qGray(mark.pixel(cur.x(),cur.y()+1)) == BLACK)
//                    {
//                        QPoint pp(cur.x(),cur.y()+1);
//                        workingStack.push_back(pp);
//                    }
//                }
            }
        }
    }
    
    //printf("num source:%d, num sink:%d\n",count_source,count_sink);
    
    double BLACK_TO_BLACK_V_BIAS = 1;
    double BLACK_TO_BLACK_H_BIAS = 2;
    double BLACK_TO_BLACK_D_BIAS = 2.23;
    double WHITE_TO_BLACK_BIAS = .5;
    double BLACK_TO_WHITE_BIAS = .5;
    
    //connect all pixels
    for (int i=0; i<width; i++)
    {
        for (int j=0; j<height; j++)
        {   
            //double wieght between black pixels?
            
            
            if (i+1<width)
            {
                if (qGray(img.pixel(i,j))==BLACK && qGray(img.pixel(i+1,j))==BLACK)
                    g -> add_edge(i+j*width, (i+1)+j*width,
                                  (invDistMap[(i+1)+j*width]+invDistMap[i+j*width])*(BLACK_TO_BLACK_H_BIAS),
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+j*width])*(BLACK_TO_BLACK_H_BIAS));
                else if (qGray(img.pixel(i,j))==BLACK && qGray(img.pixel(i+1,j))==WHITE)
                    g -> add_edge(i+j*width, (i+1)+j*width,
                                  (invDistMap[(i+1)+j*width]+invDistMap[i+j*width])*(BLACK_TO_WHITE_BIAS),
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+j*width])*(WHITE_TO_BLACK_BIAS));
                else if (qGray(img.pixel(i,j))==WHITE && qGray(img.pixel(i+1,j))==BLACK)
                    g -> add_edge(i+j*width, (i+1)+j*width,
                                  (invDistMap[(i+1)+j*width]+invDistMap[i+j*width])*(WHITE_TO_BLACK_BIAS),
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+j*width])*(BLACK_TO_WHITE_BIAS));
                else
                    g -> add_edge(i+j*width, (i+1)+j*width,
                                  (invDistMap[(i+1)+j*width]+invDistMap[i+j*width])/1,
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+j*width])/1);
            }
            
            if (j+1<height)
            {
                if (qGray(img.pixel(i,j))==BLACK && qGray(img.pixel(i,j+1))==BLACK)
                    g -> add_edge(i+j*width, i+(j+1)*width,
                                  (invDistMap[i+(j+1)*width]+invDistMap[i+j*width])*BLACK_TO_BLACK_V_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[i+(j+1)*width])*BLACK_TO_BLACK_V_BIAS);
                else if (qGray(img.pixel(i,j))==BLACK && qGray(img.pixel(i,j+1))==WHITE)
                    g -> add_edge(i+j*width, i+(j+1)*width,
                                  (invDistMap[i+(j+1)*width]+invDistMap[i+j*width])*BLACK_TO_WHITE_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[i+(j+1)*width])*WHITE_TO_BLACK_BIAS);
                else if (qGray(img.pixel(i,j))==WHITE && qGray(img.pixel(i,j+1))==BLACK)
                    g -> add_edge(i+j*width, i+(j+1)*width,
                                  (invDistMap[i+(j+1)*width]+invDistMap[i+j*width])*WHITE_TO_BLACK_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[i+(j+1)*width])*BLACK_TO_WHITE_BIAS);
                else
                    g -> add_edge(i+j*width, i+(j+1)*width,
                                  (invDistMap[i+(j+1)*width]+invDistMap[i+j*width])/3,
                                  (invDistMap[i+j*width]+invDistMap[i+(j+1)*width])/3);
            }
            
            if (j>0 && i<width-1)
            {
                if (qGray(img.pixel(i,j))==BLACK && qGray(img.pixel(i+1,j-1))==BLACK)
                    g -> add_edge(i+j*width, (i+1)+(j-1)*width,
                                  (invDistMap[(i+1)+(j-1)*width]+invDistMap[i+j*width])*BLACK_TO_BLACK_D_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+(j-1)*width])*BLACK_TO_BLACK_D_BIAS);
                else if (qGray(img.pixel(i,j))==BLACK && qGray(img.pixel(i+1,j-1))==WHITE)
                    g -> add_edge(i+j*width, (i+1)+(j-1)*width,
                                  (invDistMap[(i+1)+(j-1)*width]+invDistMap[i+j*width])*BLACK_TO_WHITE_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+(j-1)*width])*WHITE_TO_BLACK_BIAS);
                else if (qGray(img.pixel(i,j))==WHITE && qGray(img.pixel(i+1,j-1))==BLACK)
                    g -> add_edge(i+j*width, (i+1)+(j-1)*width,
                                  (invDistMap[(i+1)+(j-1)*width]+invDistMap[i+j*width])*WHITE_TO_BLACK_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+(j-1)*width])*BLACK_TO_WHITE_BIAS);
                else
                    g -> add_edge(i+j*width, (i+1)+(j-1)*width,
                                  (invDistMap[(i+1)+(j-1)*width]+invDistMap[i+j*width])/2,
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+(j-1)*width])/2);
            }
            
            if (j<height-1 && i<width-1)
            {
                if (qGray(img.pixel(i,j))==BLACK && qGray(img.pixel(i+1,j+1))==BLACK)
                    g -> add_edge(i+j*width, (i+1)+(j+1)*width,
                                  (invDistMap[(i+1)+(j+1)*width]+invDistMap[i+j*width])*BLACK_TO_BLACK_D_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+(j+1)*width])*BLACK_TO_BLACK_D_BIAS);
                else if (qGray(img.pixel(i,j))==BLACK && qGray(img.pixel(i+1,j+1))==WHITE)
                    g -> add_edge(i+j*width, (i+1)+(j+1)*width,
                                  (invDistMap[(i+1)+(j+1)*width]+invDistMap[i+j*width])*BLACK_TO_WHITE_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+(j+1)*width])*WHITE_TO_BLACK_BIAS);
                else if (qGray(img.pixel(i,j))==WHITE && qGray(img.pixel(i+1,j+1))==BLACK)
                    g -> add_edge(i+j*width, (i+1)+(j+1)*width,
                                  (invDistMap[(i+1)+(j+1)*width]+invDistMap[i+j*width])*WHITE_TO_BLACK_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+(j+1)*width])*BLACK_TO_WHITE_BIAS);
                else
                    g -> add_edge(i+j*width, (i+1)+(j+1)*width,
                                  (invDistMap[(i+1)+(j+1)*width]+invDistMap[i+j*width])/2,
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+(j+1)*width])/2);
            }
        }
    }
    
    int ret = g -> maxflow();
    
    QImage debug2 = debug.convertToFormat(QImage::Format_RGB16);
    QRgb lw = qRgb(255, 100, 100);
    QRgb lb = qRgb(155, 0, 0);
    QRgb rw = qRgb(100,255, 100);
    QRgb rb = qRgb(0, 155, 0);
    QRgb a = qRgb(255, 255, 255);
    
    //add all black pixels which
    for (int index=0; index<width*height; index++)
    {
        /*if (qGray(debug.pixel(index%width,index/width))!=BLACK && qGray(debug.pixel(index%width,index/width))!=WHITE)
        {
            debug2.setPixel(index%width,index/width,a);
        }
        else*/ if (g->what_segment(index) == GraphType::SOURCE && qGray(img.pixel(index%width,index/width))==BLACK)
        {
            out.append(index);
            debug2.setPixel(index%width,index/width,lb);
        }
        else if (g->what_segment(index) == GraphType::SOURCE)
            debug2.setPixel(index%width,index/width,lw);
        else if (qGray(img.pixel(index%width,index/width))==BLACK)
            debug2.setPixel(index%width,index/width,rb);
        else
            debug2.setPixel(index%width,index/width,rw);
    }
    
//    QString debugfile = "./cut_";
//    QString num;
//    num.setNum(width);
//    debugfile.append(num);
//    debugfile.append(".ppm");
//    debug2.save(debugfile);
    
    delete g;
    return ret;
}

/*
  New seperator algorithm
  Compute distance map, track highest distance for pixel
  create graph
    invert distance map
    add highest distance (to all pixels)
    node for each pixel, weight for arc is average of two (new) distances
  use min cut
  cut is seperation of images
  */

QImage WordSeparator::removeFirstCapitalLetter(QImage &from)
{
    WordProfile profile(from,true,0);
    QVector<int> mins = profile.getLocalMins();
    /*
      Approaches:
      1. It about in the middle, so sut at middle local min
      2. It's where whitespace is so find the min thats 0.
      3. The mins for a single word will cluster tighter together

      */

    //int selectedMin = 0;
    int selectedCut;

    QVector<int> values;
    int sum=0;
    for (int i=0; i<mins.size(); i++)
    {
        values.append(profile.getValue(mins[i]));
        sum += profile.getValue(mins[i]);
    }
    double avg = sum/(double)mins.size();
    //I don't know if this is really the best way to go about this
    int min = abs((from.width()/2)-mins[0]);
    selectedCut = mins[0];
    for (int i=1; i<mins.size(); i++)
    {
        if (values[i] <= avg && abs((from.width()/2)-mins[i]) < min)
        {
            min = abs((from.width()/2)-mins[i]);
            selectedCut = mins[i];
        }
    }
    

    printf("Cut at %d\n",selectedCut);
    QImage toReturn = from.copy(0,0,selectedCut,from.height());
    from = from.copy(selectedCut,0,from.width()-selectedCut,from.height());
    return toReturn;
}
