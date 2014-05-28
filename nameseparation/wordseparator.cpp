#include "wordseparator.h"
 
#include <math.h>



#define ANCHOR_L 3.05
#define ANCHOR_R 2.4


WordSeparator::WordSeparator()
{
}


//This performs a horizontal separation of the image by creating a distance map and then doing a graph cut on it.
QVector<BPartition*> WordSeparator::minCut(BPartition &toCut)
{
    int num_pix = toCut.width()*toCut.height();
    //double pix_vals[num_pix];
    int invDistMap[num_pix];
    
    computeInverseDistanceMap(toCut,invDistMap);
    QVector<int> firstImgPixelIndexes;
    QVector<int> secondImgPixelIndexes;
    int maxflow = pixelsOfSeparation(invDistMap,toCut.width(),toCut.height(),toCut,firstImgPixelIndexes,secondImgPixelIndexes);
    
    BPartition* firstPart = new BPartition(toCut.getSrc());
    BPartition* secondPart = new BPartition(toCut.getSrc());
    
    foreach (int index, firstImgPixelIndexes)
    {
        int x = index%toCut.width();
        int y = index/toCut.width();
        toCut.getSrc()->setPixelOwner(x,y,firstPart,1);
    }
    
    foreach (int index, secondImgPixelIndexes)
    {
        int x = index%toCut.width();
        int y = index/toCut.width();
        toCut.getSrc()->setPixelOwner(x,y,secondPart,1);
    }
    
    
   
    QVector<BPartition*> ret;
    ret.append(firstPart);
    ret.append(secondPart);
    return ret;
}

//This performs a verticle separation of two words, it identifys descenders in an attempt to increase accuracy
//QVector<QImage> WordSeparator::horzCutEntries(QImage &img, int vert_divide, QVector<QPoint> crossPoints, QVector<QVector<double> > &probMap)
//{
//    int num_pix = img.width()*img.height();
//    //double pix_vals[num_pix];
//    int invDistMap[num_pix];
    
//    computeInverseDistanceMap(img,invDistMap);
//    QVector<int> firstImgBlackPixelIndexes;
    
//    //best anchor weight:300 - 425
//    int ANCHOR_WEIGHT = 1200;
//    int maxflow = pixelsOfSeparation(invDistMap,img.width(),img.height(),img,firstImgBlackPixelIndexes,ANCHOR_WEIGHT,SPLIT_VERT,vert_divide);
    
    
//    QImage twoColorImg = img.copy(0,0,img.width(),img.height());
//    foreach(int index, firstImgBlackPixelIndexes)
//    {
//        int x = index%img.width();
//        int y = index/img.width();
//        twoColorImg.setPixel(x,y,100);
//    }
    
//    //Find cross-over contected components
//    QImage mark = img.copy(0,0,img.width(),img.height());
//    QVector<QPoint> workingStack;
//    QVector<QPoint> ccccKeyPoints;
//    QVector<QVector<QPoint> > ccccLowerPoints;
    
//    foreach (QPoint keyPoint, crossPoints)
//    {
//        if (qGray(mark.pixel(keyPoint)) == BLACK)
//        {
//            workingStack.push_back(keyPoint);
//            int startingColor = qGray(twoColorImg.pixel(keyPoint));
//            QVector<QPoint> lowerPoints;
//            bool crossover=false;
//            while (!workingStack.isEmpty())
//            {   
//                QPoint cur = workingStack.back();
//                workingStack.pop_back();
                
//                if (cur.y() >= keyPoint.y() || qGray(twoColorImg.pixel(cur)) == BLACK)
//                {
//                    lowerPoints.append(cur);
//                }
                
//                //this needs changed
//                if (!crossover && qGray(twoColorImg.pixel(cur)) != startingColor)
//                {
//                    crossover=true;
//                }
                
//                if (cur.x()<mark.width()-1 && qGray(mark.pixel(cur.x()+1,cur.y())) == BLACK)
//                {
//                    QPoint pp(cur.x()+1,cur.y());
//                    workingStack.push_back(pp);
//                    mark.setPixel(pp,WHITE);
//                }
//                if (cur.y()<mark.height()-1 && qGray(mark.pixel(cur.x(),cur.y()+1)) == BLACK)
//                {
//                    QPoint pp(cur.x(),cur.y()+1);
//                    workingStack.push_back(pp);
//                    mark.setPixel(pp,WHITE);
//                }
//                if (cur.x()>0 && qGray(mark.pixel(cur.x()-1,cur.y())) == BLACK)
//                {
//                    QPoint pp(cur.x()-1,cur.y());
//                    workingStack.push_back(pp);
//                    mark.setPixel(pp,WHITE);
//                }
//                if (cur.y()>0 && qGray(mark.pixel(cur.x(),cur.y()-1)) == BLACK)
//                {
//                    QPoint pp(cur.x(),cur.y()-1);
//                    workingStack.push_back(pp);
//                    mark.setPixel(pp,WHITE);
//                }
//                //diagonals
//                if (cur.x()<mark.width()-1 && cur.y()<mark.height()-1 && qGray(mark.pixel(cur.x()+1,cur.y()+1)) == BLACK)
//                {
//                    QPoint pp(cur.x()+1,cur.y()+1);
//                    workingStack.push_back(pp);
//                    mark.setPixel(pp,WHITE);
//                }
//                if (cur.y()<mark.height()-1 && cur.x()>0 && qGray(mark.pixel(cur.x()-1,cur.y()+1)) == BLACK)
//                {
//                    QPoint pp(cur.x()-1,cur.y()+1);
//                    workingStack.push_back(pp);
//                    mark.setPixel(pp,WHITE);
//                }
//                if (cur.x()<mark.width()-1 && cur.y()>0 && qGray(mark.pixel(cur.x()+1,cur.y()-1)) == BLACK)
//                {
//                    QPoint pp(cur.x()+1,cur.y()-1);
//                    workingStack.push_back(pp);
//                    mark.setPixel(pp,WHITE);
//                }
//                if (cur.y()>0 && cur.x()>0 && qGray(mark.pixel(cur.x()-1,cur.y()-1)) == BLACK)
//                {
//                    QPoint pp(cur.x()-1,cur.y()-1);
//                    workingStack.push_back(pp);
//                    mark.setPixel(pp,WHITE);
//                }
//            }
            
//            if (crossover)
//            {
//                ccccKeyPoints.append(keyPoint);
//                ccccLowerPoints.append(lowerPoints);    
//            }
            
//        }
//    }
//    ////
    
//    //Perhaps we ought to gather the entire cccc into a collection, then iterate over all the points
//    //But we only need the lower points, and only the ones on the map
//    //What about only the points of the the regular black(lower)?
//    //we still need the keypoint for alignment, but that should sort of work. Most will get a zero map score
//    //started...
    
//    //apply prob map
//    double DESCENDER_SCORE_THRESH = 50;
    
//    for (int ci=0; ci<ccccKeyPoints.size(); ci++)
//    {
//        QPoint candidate = ccccKeyPoints[ci];
//        QVector<QPoint> candidateComponent = ccccLowerPoints[ci];
//        QVector<QVector<double> > scoreMap(probMap.size());
//        QVector<double> temp(probMap[0].size());
//        temp.fill(0);
//        scoreMap.fill(temp);
//        double scoreTotal=0;
//        int num_points_outside=0;
//        foreach (QPoint ccccP, candidateComponent)
//        {
//            int x = ccccP.x()-candidate.x() + probMap.size()/2;
//            int y = ccccP.y()-candidate.y();
            
//            if(x>=0 && x<probMap.size() && y>=0 && y<probMap[0].size())
//            {
//                scoreMap[x][y] = probMap[x][y]-.4;
//                scoreTotal+=scoreMap[x][y];
//            }
//            else
//            {
//                scoreTotal-=.4;
//                num_points_outside++;
//            }
            
            
//        }
        
//        printf("total score for point (%d,%d): %f\n",candidate.x(),candidate.y(),scoreTotal);
        
//        if (scoreTotal>DESCENDER_SCORE_THRESH)
//        {
//            //if this is disconnected (all pixels are important in map) we can add all
//            //but what if we are intersected with something else?
            
//            //expiriment
//            foreach (QPoint p, candidateComponent)
//            {
//                firstImgBlackPixelIndexes.append(p.x()+p.y()*img.width());
//            }
//        }
//    }
    
//    int firstLowestPixel = 0;
//    int secondHighestPixel = 0;
    
//    QImage firstImg = img.copy(0,0,img.width(),img.height());
//    firstImg.fill(255);
//    QImage secondImg = img.copy(0,0,img.width(),img.height());
    
//    foreach(int index, firstImgBlackPixelIndexes)
//    {
//        int x = index%img.width();
//        int y = index/img.width();
//        firstImg.setPixel(x,y,0);
//        secondImg.setPixel(x,y,255);
        
//        if (y>firstLowestPixel)
//            firstLowestPixel=y;
//    }
//    bool notFound = true;
//    for (int j=0; j<img.height() && notFound; j++)
//    {
//        for (int i=0; i<img.width() && notFound; i++)
//        {
//            if (qGray(secondImg.pixel(i,j))==0)
//            {
//                secondHighestPixel=j;
//                notFound = false;
//            }
//        }
//    }
    
//    printf("maxflow= %d\n",maxflow);
    
//    firstImg = firstImg.copy(0,0,firstImg.width(),firstLowestPixel+1);
//    secondImg = secondImg.copy(0, secondHighestPixel, secondImg.width(), secondImg.height()-secondHighestPixel);
    
    
   
//    QVector<QImage> ret;
//    ret.append(firstImg);
//    ret.append(secondImg);
//    return ret;
//}


//QVector<QImage> WordSeparator::cutNames(QImage &img)
//{
    
//    int maxFlow_firstCut;
//    int pixelWidth_firstCut_left;
//    int pixelWidth_firstCut_right;
//    int pixelCount_firstCut_left;
//    int pixelCount_firstCut_right;
    
//    int num_pix = img.width()*img.height();
//    int invDistMap[num_pix];
    
//    QVector<int> firstImgBlackPixelIndexes;
    
//    firstImgBlackPixelIndexes.clear();
    
//    computeInverseDistanceMap(img,invDistMap);//do we need a new distance map each cut?
    
//    maxFlow_firstCut = pixelsOfSeparation(invDistMap,img.width(),img.height(),img,firstImgBlackPixelIndexes);
    
//    int firstFarthestRightPixel = 0;
//    int secondFarthestLeftPixel = 0;
    
//    QImage firstImg = img.copy(0,0,img.width(),img.height());
//    firstImg.fill(255);
//    QImage secondImg = img.copy(0,0,img.width(),img.height());
    
//    foreach(int index, firstImgBlackPixelIndexes)
//    {
//        int x = index%img.width();
//        int y = index/img.width();
//        firstImg.setPixel(x,y,0);
//        secondImg.setPixel(x,y,255);
        
//        if (x>firstFarthestRightPixel)
//            firstFarthestRightPixel=x;
//    }
//    bool notFound = true;
//    for (int i=0; i<img.width() && notFound; i++)
//    {
//        for (int j=0; j<img.height() && notFound; j++)
//        {
//            if (qGray(secondImg.pixel(i,j))==0)
//            {
//                secondFarthestLeftPixel=i;
//                notFound = false;
//            }
//        }
//    }
    
//    QImage leftImg = firstImg.copy(0,0,firstFarthestRightPixel+1,firstImg.height());
//    QImage rightImg = secondImg.copy(secondFarthestLeftPixel, 0, secondImg.width()-secondFarthestLeftPixel, secondImg.height());
//    if (maxFlow_firstCut<0)
//        maxFlow_firstCut = INT_POS_INFINITY;
//    pixelWidth_firstCut_left = leftImg.width();
//    pixelWidth_firstCut_right = rightImg.width();
//    pixelCount_firstCut_left = firstImgBlackPixelIndexes.size();
//    pixelCount_firstCut_right = num_pix - pixelCount_firstCut_left;
    
//    //////
//    int maxFlow_secondCutL;
//    int pixelWidth_secondCutL_left;
//    int pixelWidth_secondCutL_right;
//    int pixelCount_secondCutL_left;
//    int pixelCount_secondCutL_right;
    
//    num_pix = leftImg.width()*leftImg.height();
    
//    firstImgBlackPixelIndexes.clear();
    
//    computeInverseDistanceMap(leftImg,invDistMap);//do we need a new distance map each cut?
    
//    maxFlow_secondCutL = pixelsOfSeparation(invDistMap,leftImg.width(),leftImg.height(),leftImg,firstImgBlackPixelIndexes);
    
//    firstFarthestRightPixel = 0;
//    secondFarthestLeftPixel = 0;
    
//    firstImg = leftImg.copy(0,0,leftImg.width(),leftImg.height());
//    firstImg.fill(255);
//    secondImg = leftImg.copy(0,0,leftImg.width(),leftImg.height());
    
//    foreach(int index, firstImgBlackPixelIndexes)
//    {
//        int x = index%leftImg.width();
//        int y = index/leftImg.width();
//        firstImg.setPixel(x,y,0);
//        secondImg.setPixel(x,y,255);
        
//        if (x>firstFarthestRightPixel)
//            firstFarthestRightPixel=x;
//    }
//    notFound = true;
//    for (int i=0; i<leftImg.width() && notFound; i++)
//    {
//        for (int j=0; j<leftImg.height() && notFound; j++)
//        {
//            if (qGray(secondImg.pixel(i,j))==0)
//            {
//                secondFarthestLeftPixel=i;
//                notFound = false;
//            }
//        }
//    }
    
//    QImage leftImg2L = firstImg.copy(0,0,firstFarthestRightPixel+1,firstImg.height());
//    QImage rightImg2L = secondImg.copy(secondFarthestLeftPixel, 0, secondImg.width()-secondFarthestLeftPixel, secondImg.height());
//    if (maxFlow_secondCutL<0)
//        maxFlow_secondCutL = INT_POS_INFINITY;
//    pixelWidth_secondCutL_right = rightImg2L.width();
//    pixelWidth_secondCutL_left = leftImg2L.width();
//    pixelCount_secondCutL_left = firstImgBlackPixelIndexes.size();
//    pixelCount_secondCutL_right = pixelCount_firstCut_left - pixelCount_secondCutL_left;
    
//    ////////b
    
//    int maxFlow_secondCutR;
//    int pixelWidth_secondCutR_left;
//    int pixelWidth_secondCutR_right;
//    int pixelCount_secondCutR_left;
//    int pixelCount_secondCutR_right;
    
//    num_pix = rightImg.width()*rightImg.height();
    
//    firstImgBlackPixelIndexes.clear();
    
//    computeInverseDistanceMap(rightImg,invDistMap);//do we need a new distance map each cut?
    
//    maxFlow_secondCutR = pixelsOfSeparation(invDistMap,rightImg.width(),rightImg.height(),rightImg,firstImgBlackPixelIndexes);
    
//    firstFarthestRightPixel = 0;
//    secondFarthestLeftPixel = 0;
    
//    firstImg = rightImg.copy(0,0,rightImg.width(),rightImg.height());
//    firstImg.fill(255);
//    secondImg = rightImg.copy(0,0,rightImg.width(),rightImg.height());
    
//    foreach(int index, firstImgBlackPixelIndexes)
//    {
//        int x = index%rightImg.width();
//        int y = index/rightImg.width();
//        firstImg.setPixel(x,y,0);
//        secondImg.setPixel(x,y,255);
        
//        if (x>firstFarthestRightPixel)
//            firstFarthestRightPixel=x;
//    }
//    notFound = true;
//    for (int i=0; i<rightImg.width() && notFound; i++)
//    {
//        for (int j=0; j<rightImg.height() && notFound; j++)
//        {
//            if (qGray(secondImg.pixel(i,j))==0)
//            {
//                secondFarthestLeftPixel=i;
//                notFound = false;
//            }
//        }
//    }
    
//    QImage leftImg2R = firstImg.copy(0,0,firstFarthestRightPixel+1,firstImg.height());
//    QImage rightImg2R = secondImg.copy(secondFarthestLeftPixel, 0, secondImg.width()-secondFarthestLeftPixel, secondImg.height());
//    if (maxFlow_secondCutR<0)
//        maxFlow_secondCutR = INT_POS_INFINITY;
//    pixelWidth_secondCutR_right = rightImg2R.width();
//    pixelWidth_secondCutR_left = leftImg2R.width();
//    pixelCount_secondCutR_left = firstImgBlackPixelIndexes.size();
//    pixelCount_secondCutR_right = pixelCount_firstCut_right - pixelCount_secondCutR_left;
//    ////////
    
    
    
//    //printf("Cut %d: maxflow=%d, size=%d\n",numOfCuts,cutFlows[numOfCuts],sizeOfCuts[numOfCuts]);
    
//    leftImg.save("first_left.pgm");
//    rightImg.save("first_right.pgm");
//    leftImg2L.save("secondL_left.pgm");
//    rightImg2L.save("secondL_right.pgm");
//    leftImg2R.save("secondR_left.pgm");
//    rightImg2R.save("secondR_right.pgm");
    
    
//    QVector<QImage> ret;
//    //ret.append(leftImg);
////    printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n",
////           maxFlow_firstCut,pixelWidth_firstCut_left,pixelWidth_firstCut_right,pixelCount_firstCut_left,pixelCount_firstCut_right,
////           maxFlow_secondCutL,pixelWidth_secondCutL_left,pixelWidth_secondCutL_right,pixelCount_secondCutL_left,pixelCount_secondCutL_right,
////           maxFlow_secondCutR,pixelWidth_secondCutR_left,pixelWidth_secondCutR_right,pixelCount_secondCutR_left,pixelCount_secondCutR_right,
////           maxFlow_firstCut-maxFlow_secondCutL,maxFlow_firstCut-maxFlow_secondCutR);
    
//    //Generated by J48 on data from res1,2,3
//    char sit = ' ';
//    if (pixelWidth_firstCut_right <= 167)
//    {
//        if (maxFlow_firstCut-maxFlow_secondCutR <= -31874)
//        {
//            if (pixelCount_secondCutL_right <= 786)
//                sit='b';
//            else
//                sit='l';
//        }
//        else
//        {
//            sit='b';
//        }
//    }
//    else
//    {
//        if (pixelWidth_secondCutR_right <= 149)
//        {
//            if (maxFlow_firstCut-maxFlow_secondCutR <= -78228)
//            {
//                if (pixelWidth_firstCut_left <= 79)
//                    sit='r';
//                else
//                    sit='o';
//            }
//            else
//            {
//                sit='r';
//            }
//        }
//        else
//        {
//            if (maxFlow_secondCutR <= 33942)
//            {
//                sit='b';
//            }
//            else
//            {
//                if (pixelWidth_secondCutR_left <= 116)
//                    sit='o';
//                else
//                    sit='b';
//            }
//        }
//    }
    
//    if (sit=='o' || sit=='r')
//    {
//        ret.append(leftImg);
//        ret.append(rightImg);
//    }
//    else if (sit=='l')
//    {
//        ret.append(leftImg2L);
//        ret.append(rightImg2L);
//        ret.append(rightImg);
//    }
//    else
//    {
//        QImage copy = img.copy(0,0,img.width(),img.height());
//        ret.append(copy);
//    }
    
//    return ret;
//}

///* There are two parameters to evaluate if we have the correct cut:
//   maxflow of the cut and surronding cuts and the number of pixels in the cut.
//   for a decision tree, these 
//   -num of pixels in cut
//   -difference between maxflow and maxflow of all other cuts
//   -width of cut, or relative width of cut
//   -
   
//  */
//QVector<QImage> WordSeparator::recursiveCutWordToFirstLetter(QImage &img)
//{
//    QVector<QImage> cuts;
//    QVector<int> cutFlows;
//    QVector<int> sizeOfCuts;
//    int numOfCuts = 0;
    
//    cuts.append(img);
//    cutFlows.append(0);
//    sizeOfCuts.append(0);
    
//    int num_pix = img.width()*img.height();
//    int invDistMap[num_pix];
    
//    QVector<int> firstImgBlackPixelIndexes;
//    while(true)
//    {
//        firstImgBlackPixelIndexes.clear();
//        num_pix = cuts[numOfCuts].width()*cuts[numOfCuts].height();
        
//        computeInverseDistanceMap(cuts[numOfCuts],invDistMap);//do we need a new distance map each cut?
        
//        int maxflow = pixelsOfSeparation(invDistMap,cuts[numOfCuts].width(),cuts[numOfCuts].height(),cuts[numOfCuts],firstImgBlackPixelIndexes);
        
//        int firstFarthestRightPixel = 0;
        
//        QImage firstImg = cuts[numOfCuts].copy(0,0,cuts[numOfCuts].width(),cuts[numOfCuts].height());
//        firstImg.fill(255);
        
//        foreach(int index, firstImgBlackPixelIndexes)
//        {
//            int x = index%cuts[numOfCuts].width();
//            int y = index/cuts[numOfCuts].width();
//            firstImg.setPixel(x,y,0);
            
//            if (x>firstFarthestRightPixel)
//                firstFarthestRightPixel=x;
//        }
        
//        cuts.push_back(firstImg.copy(0,0,firstFarthestRightPixel+1,firstImg.height()));
//        if (maxflow<0)
//            maxflow = INT_POS_INFINITY;
//        cutFlows.append(maxflow);
//        sizeOfCuts.append(firstImgBlackPixelIndexes.size());
//        numOfCuts++;
        
//        //printf("Cut %d: maxflow=%d, size=%d\n",numOfCuts,cutFlows[numOfCuts],sizeOfCuts[numOfCuts]);
//        QString numstr;
//        numstr.setNum(numOfCuts);
//        QString filename = "cut";
//        filename+=numstr;
//        filename+=".pgm";
//        cuts[numOfCuts].save(filename);
        
        
//        if (numOfCuts>4)
//        {
//            break;
//        }
//    }
    
    
////    printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n",
////           cuts[1].width(),cuts[2].width(),cuts[3].width(),cuts[4].width(),cuts[5].width(),
////           sizeOfCuts[1],sizeOfCuts[2],sizeOfCuts[3],sizeOfCuts[4],sizeOfCuts[5],
////           cutFlows[1],cutFlows[2],cutFlows[3],cutFlows[4],cutFlows[5],
////           cutFlows[2]-cutFlows[1],cutFlows[3]-cutFlows[2],cutFlows[4]-cutFlows[3],cutFlows[5]-cutFlows[4]);
//    char sit = ' ';
//    if (cuts[1].width() <= 44)
//    {
//        if (sizeOfCuts[1] <= 309)
//            sit='0';
//        else
//            sit='1';
//    }
//    else
//    {
//        if (cuts[3].width() <= 35)
//        {
//            if (sizeOfCuts[2] <= 299)
//            {
//                if (cuts[2].width() <= 38)
//                    sit='1';
//                else
//                    sit='2';
//            }
//            else
//            {
//                if (cutFlows[4] <= 132914)
//                {
//                    if (cuts[4].width() <= 12)
//                    {
//                        sit='2';
//                    }
//                    else
//                    {
//                        if (sizeOfCuts[3] <= 147)
//                        {
//                                sit='2';
//                        }
//                        else
//                        {
//                            if (sizeOfCuts[2] <= 599)
//                            {
//                                if (sizeOfCuts[1] <= 690)
//                                    sit='3';
//                                else
//                                    sit='2';
//                            }
//                            else
//                            {
//                                sit='3';
//                            }
                        
//                        }
//                    }
//                }
//                else
//                { 
//                            sit='2';
//                }
//            }
//        }
//        else
//        {
//            if (cuts[5].width() <= 25)
//            {
//                if (cuts[2].width() <= 54)
//                {
//                                sit='2';
//                }
//                else
//                {
//                    if (cutFlows[4]-cutFlows[3] <= 27599)
//                    {
//                        if (cutFlows[2] <= 55825)
//                        {
//                            if (sizeOfCuts[1] <= 1470)
//                                sit='2';
//                            else
//                                sit='4';
//                        }
//                        else
//                        {
//                            if (cutFlows[4] <= 103641)
//                            {
//                                if (cutFlows[3] <= 70113)
//                                    sit='4';
//                                else
//                                    sit='3';
//                            }
//                            else
//                            {
//                                sit='4';
//                            }
//                        }
//                    }
//                    else
//                    {
//                        sit='3';
//                    }
                    
//                }
//            }
//            else
//            {
//                if (cuts[4].width() <= 50)
//                    sit='4';
//                else
//                    sit='5';
//            }
//        }
//    }
    
//    //return top two guesse 
//    QVector<QImage> ret;
//    if (sit=='0')
//    {
//        ret.append(cuts[0]);
//        ret.append(cuts[1]);
//    }
//    else if (sit=='1')
//    {
//        ret.append(cuts[1]);
//        ret.append(cuts[2]);
//    }
//    else if (sit=='2')
//    {
//        ret.append(cuts[2]);
//        ret.append(cuts[3]);
//        //maybe 1
//    }
//    else if (sit=='3')
//    {
//        ret.append(cuts[3]);
//        ret.append(cuts[2]);
//    }
//    else if (sit=='4')
//    {
//        ret.append(cuts[4]);
//        ret.append(cuts[3]);
//    }
//    else if (sit=='5')
//    {
//        ret.append(cuts[5]);
//        ret.append(cuts[4]);
//    }
                                
//    return ret;
//}

//Meijster distance <http://fab.cba.mit.edu/classes/S62.12/docs/Meijster_distance.pdf>
//This can be parallelized. Should probably flip from column to row first
void WordSeparator::computeInverseDistanceMap(BPartition &src, int* out)
{
    int maxDist=0;
    int g[src.width()*src.height()];
    for (int x=0; x<src.width(); x++)
    {
        if (src.pixel(x,0))
        {
            g[x+0*src.width()]=0;
        }
        else
        {
            g[x+0*src.width()]=INT_POS_INFINITY;//src.width()*src.height();
        }
        
        for (int y=1; y<src.height(); y++)
        {
            if (src.pixel(x,y))
            {
                g[x+y*src.width()]=0;
            }
            else
            {
                if (g[x+(y-1)*src.width()] != INT_POS_INFINITY)
                    g[x+y*src.width()]=1+g[x+(y-1)*src.width()];
                else
                    g[x+y*src.width()] = INT_POS_INFINITY;
            }
        }
        
        for (int y=src.height()-2; y>=0; y--)
        {
            if (g[x+(y+1)*src.width()]<g[x+y*src.width()])
            {
                if (g[x+(y+1)*src.width()] != INT_POS_INFINITY)
                    g[x+y*src.width()]=1+g[x+(y+1)*src.width()];
                else
                    g[x+y*src.width()] = INT_POS_INFINITY;
            }
        }
        
        /*if(x==20)
        {
            for (int y=0; y<src.height(); y++)
            {
                printf("%d .. %d\n",qGray(src.pixel(x,y)),g[x+y*src.width()]);
            }
        }*/
    }
    
    int q;
    int s[src.width()];
    int t[src.width()];
    int w;
    for (int y=0; y<src.height(); y++)
    {
        q=0;
        s[0]=0;
        t[0]=0;
        for (int u=1; u<src.width();u++)
        {
            while (q>=0 && f(t[q],s[q],y,src.width(),g) > f(t[q],u,y,src.width(),g))
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
                w = SepPlusOne(s[q],u,y,src.width(),g);
                if (w<src.width())
                {
                    q++;
                    s[q]=u;
                    t[q]=w;
                }
            }
        }
        
        for (int u=src.width()-1; u>=0; u--)
        {
            out[u+y*src.width()]= f(u,s[q],y,src.width(),g);
            if (out[u+y*src.width()] > maxDist)
                maxDist = out[u+y*src.width()];
            if (u==t[q])
                q--;
        }
    }
    
    
    //    QImage debug(src.width(),src.height(),src.format());
    //    debug.setColorTable(src.colorTable());
    //    for (int i=0; i<debug.width(); i++)
    //    {
    //        for (int j=0; j<debug.height(); j++)
    //            debug.setPixel(i,j,(int)((pow(out[i+j*debug.width()],.2)/((double)pow(maxDist,.2)))*255));
            
    //    }
    //    debug.save("./reg_dist_map.pgm");
    //    printf("image format:%d\n",debug.format());
    
    //invert
//    printf("maxDist=%d\n",maxDist);
    maxDist++;
//    double normalizer = (24.0/maxDist);
    double normalizer = (20.0/2000);
    BImage mark = src.makeImage();
    QVector<QPoint> workingStack;
    QVector<QPoint> growingComponent;
    
    
    int newmax = 0;
    for (int q = 0; q < src.width()*src.height(); q++)
    {   
        //out[q] = pow(6,24-out[q]*normalizer)/pow(6,20);
        if (src.pixel(q%src.width(),q/src.width()) && mark.pixel(q%src.width(),q/src.width()))
        {
            //fill bias
            QPoint p(q%src.width(),q/src.width());
            workingStack.push_back(p);
            mark.setPixel(p,false);
            while (!workingStack.isEmpty())
            {   
                QPoint cur = workingStack.back();
                workingStack.pop_back();
                growingComponent.append(cur);
                
                
                
                
                if (cur.x()>0 && mark.pixel(cur.x()-1,cur.y()))
                {
                    QPoint pp(cur.x()-1,cur.y());
                    workingStack.push_back(pp);
                    mark.setPixel(pp,false);
                }
                
                
                if (cur.x()<mark.width()-1 && mark.pixel(cur.x()+1,cur.y()))
                {
                    QPoint pp(cur.x()+1,cur.y());
                    workingStack.push_back(pp);
                    mark.setPixel(pp,false);
                    
                }
                if (cur.y()<mark.height()-1 && mark.pixel(cur.x(),cur.y()+1))
                {
                    QPoint pp(cur.x(),cur.y()+1);
                    workingStack.push_back(pp);
                    mark.setPixel(pp,false);
                }
                if (cur.y()>0 && mark.pixel(cur.x(),cur.y()-1))
                {
                    QPoint pp(cur.x(),cur.y()-1);
                    workingStack.push_back(pp);
                    mark.setPixel(pp,false);
                }
            }
            int cc_size = growingComponent.size();
            while (!growingComponent.isEmpty())
            {
                QPoint cur = growingComponent.back();
                growingComponent.pop_back();
                int index = cur.x()+src.width()*cur.y();
                out[index] = pow(6,20-out[index]*normalizer)/pow(6,16) + 4*std::min(cc_size,500);
            }
            
        }
        else if (qGray(src.pixel(q%src.width(),q/src.width()))!=BLACK)
        {
            out[q] = pow(6,20-out[q]*normalizer)/pow(6,16);
        }

        if (out[q]>newmax)
            newmax=out[q];
    }
    
//    printf("newMax:%d\n",newmax);
    QImage debug=src.makeImage().getImage();
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
        return INT_POS_INFINITY;
    }
    return 1 + ((u*u)-(i*i)+g[u+y*m]*g[u+y*m]-(g[i+y*m]*g[i+y*m])) / (2*(u-i));
}


//Uses Boykov graph cut
int WordSeparator::pixelsOfSeparation(int* invDistMap, int width, int height, BPartition &img, QVector<int> &outSource, QVector<int> &outSink, int anchor_weight, int split_method, int vert_divide)
{
    typedef Graph<int,int,int> GraphType;
    GraphType *g = new GraphType(width*height, 2*(width-1)*(height-1)-(height+width)); 
    
    for (int i=0; i<width*height; i++)
    {
        g->add_node();
    }
    
//    QImage debug = img.copy(0,0,img.width(),img.height());
    
    if (split_method == SPLIT_HORZ)
    {
        //find source pixels
        int count_source = height*ANCHOR_L;
        for (int i=0; count_source>0 && i<width; i++)
        {
            for (int j=0; count_source>0 && j<height; j++)
            {
                if (img.pixel(i,j))
                {
                    int index = i+width*j;
                    g -> add_tweights(index, anchor_weight,0);//invDistMap[index], 0);
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
                if (img.pixel((width-1)-(o-i),(height-1)-i))
                {
                    int index = ((width-1)-(o-i))+width*((height-1)-i);
                    g -> add_tweights(index, 0, anchor_weight);
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
    }
    else if (split_method == SPLIT_VERT)
    {
        //all pixels are either source or sink
        for (int j=0; j<vert_divide; j++)
        {
            double anchor_weight_for_level = anchor_weight * ((vert_divide + vert_divide-j)/(double)(2*vert_divide));
            //printf("%f, ",((vert_divide-j)/(double)vert_divide));
            for (int i=0; i<width; i++)
            {
                if (img.pixel(i,j))
                {
                    int index = i+width*j;
                    g -> add_tweights(index, (int)anchor_weight_for_level,0);
    //                debug.setPixel(i,j,150);
                }
            }
        }
        for (int j=height-1; j>=vert_divide; j--)
        {
            double anchor_weight_for_level = anchor_weight * ((((height-1) -(double)vert_divide) + j-vert_divide)/(2.*((height-1) -(double)vert_divide)));
            //printf("%f- ",((j-vert_divide)/((height-1) -vert_divide)));
            for (int i=0; i<width; i++)
            {
                if (img.pixel(i,j))
                {
                    int index = i+width*j;
                    g -> add_tweights(index, 0, (int)anchor_weight_for_level);
    //                debug.setPixel(i,j,150);
                }
            }
        }
    }
    
    //printf("num source:%d, num sink:%d\n",count_source,count_sink);
    
    double BLACK_TO_BLACK_V_BIAS = 1;
    double BLACK_TO_BLACK_H_BIAS = 2;
    double BLACK_TO_BLACK_D_BIAS = 2.23;
    double WHITE_TO_BLACK_BIAS = .5;
    double BLACK_TO_WHITE_BIAS = .5;
    double WHITE_TO_WHITE_V_BIAS = 1;
    double WHITE_TO_WHITE_H_BIAS = .333;
    double WHITE_TO_WHITE_D_BIAS = .5;
    
    if (split_method==SPLIT_VERT)
    {
        BLACK_TO_BLACK_V_BIAS = 1.5;
        BLACK_TO_BLACK_H_BIAS = 1.5;
        BLACK_TO_BLACK_D_BIAS = sqrt(pow(BLACK_TO_BLACK_V_BIAS,2)+pow(BLACK_TO_BLACK_H_BIAS,2));
        WHITE_TO_BLACK_BIAS = .5;
        BLACK_TO_WHITE_BIAS = .5;
        WHITE_TO_WHITE_V_BIAS = .5;
        WHITE_TO_WHITE_H_BIAS = .5;
        WHITE_TO_WHITE_D_BIAS = sqrt(pow(WHITE_TO_WHITE_V_BIAS,2)+pow(WHITE_TO_WHITE_H_BIAS,2));
    }
    
    double reducer = 1;
    
    //connect all pixels
    for (int i=0; i<width; i++)
    {
        for (int j=0; j<height; j++)
        {   
            if (split_method==SPLIT_VERT)
            {
                if (j<vert_divide)
                {
                    reducer = ((vert_divide + vert_divide-j)/(double)(2*vert_divide));
                }
                else
                {
                    reducer = ((((height-1) -(double)vert_divide) + j-vert_divide)/(2.*((height-1) -(double)vert_divide)));
                }
            }
            
            
            if (i+1<width)
            {
                if (img.pixel(i,j) && img.pixel(i+1,j))
                    g -> add_edge(i+j*width, (i+1)+j*width,
                                  (invDistMap[(i+1)+j*width]+invDistMap[i+j*width])*(reducer*BLACK_TO_BLACK_H_BIAS),
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+j*width])*(reducer*BLACK_TO_BLACK_H_BIAS));
                else if (img.pixel(i,j) && !img.pixel(i+1,j))
                    g -> add_edge(i+j*width, (i+1)+j*width,
                                  (invDistMap[(i+1)+j*width]+invDistMap[i+j*width])*(BLACK_TO_WHITE_BIAS),
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+j*width])*(WHITE_TO_BLACK_BIAS));
                else if (!img.pixel(i,j) && img.pixel(i+1,j))
                    g -> add_edge(i+j*width, (i+1)+j*width,
                                  (invDistMap[(i+1)+j*width]+invDistMap[i+j*width])*(WHITE_TO_BLACK_BIAS),
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+j*width])*(BLACK_TO_WHITE_BIAS));
                else
                    g -> add_edge(i+j*width, (i+1)+j*width,
                                  (invDistMap[(i+1)+j*width]+invDistMap[i+j*width])*WHITE_TO_WHITE_V_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+j*width])*WHITE_TO_WHITE_V_BIAS);
            }
            
            if (j+1<height)
            {
                if (img.pixel(i,j) && img.pixel(i,j+1))
                    g -> add_edge(i+j*width, i+(j+1)*width,
                                  (invDistMap[i+(j+1)*width]+invDistMap[i+j*width])*reducer*BLACK_TO_BLACK_V_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[i+(j+1)*width])*reducer*BLACK_TO_BLACK_V_BIAS);
                else if (img.pixel(i,j) && !img.pixel(i,j+1))
                    g -> add_edge(i+j*width, i+(j+1)*width,
                                  (invDistMap[i+(j+1)*width]+invDistMap[i+j*width])*BLACK_TO_WHITE_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[i+(j+1)*width])*WHITE_TO_BLACK_BIAS);
                else if (!img.pixel(i,j) && img.pixel(i,j+1))
                    g -> add_edge(i+j*width, i+(j+1)*width,
                                  (invDistMap[i+(j+1)*width]+invDistMap[i+j*width])*WHITE_TO_BLACK_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[i+(j+1)*width])*BLACK_TO_WHITE_BIAS);
                else
                    g -> add_edge(i+j*width, i+(j+1)*width,
                                  (invDistMap[i+(j+1)*width]+invDistMap[i+j*width])*WHITE_TO_WHITE_H_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[i+(j+1)*width])*WHITE_TO_WHITE_H_BIAS);
            }
            
            if (j>0 && i<width-1)
            {
                if (img.pixel(i,j) && img.pixel(i+1,j-1))
                    g -> add_edge(i+j*width, (i+1)+(j-1)*width,
                                  (invDistMap[(i+1)+(j-1)*width]+invDistMap[i+j*width])*reducer*BLACK_TO_BLACK_D_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+(j-1)*width])*reducer*BLACK_TO_BLACK_D_BIAS);
                else if (img.pixel(i,j) && !img.pixel(i+1,j-1))
                    g -> add_edge(i+j*width, (i+1)+(j-1)*width,
                                  (invDistMap[(i+1)+(j-1)*width]+invDistMap[i+j*width])*BLACK_TO_WHITE_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+(j-1)*width])*WHITE_TO_BLACK_BIAS);
                else if (!img.pixel(i,j) && img.pixel(i+1,j-1))
                    g -> add_edge(i+j*width, (i+1)+(j-1)*width,
                                  (invDistMap[(i+1)+(j-1)*width]+invDistMap[i+j*width])*WHITE_TO_BLACK_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+(j-1)*width])*BLACK_TO_WHITE_BIAS);
                else
                    g -> add_edge(i+j*width, (i+1)+(j-1)*width,
                                  (invDistMap[(i+1)+(j-1)*width]+invDistMap[i+j*width])*WHITE_TO_WHITE_D_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+(j-1)*width])*WHITE_TO_WHITE_D_BIAS);
            }
            
            if (j<height-1 && i<width-1)
            {
                if (img.pixel(i,j) && img.pixel(i+1,j+1))
                    g -> add_edge(i+j*width, (i+1)+(j+1)*width,
                                  (invDistMap[(i+1)+(j+1)*width]+invDistMap[i+j*width])*reducer*BLACK_TO_BLACK_D_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+(j+1)*width])*reducer*BLACK_TO_BLACK_D_BIAS);
                else if (img.pixel(i,j) && !img.pixel(i+1,j+1))
                    g -> add_edge(i+j*width, (i+1)+(j+1)*width,
                                  (invDistMap[(i+1)+(j+1)*width]+invDistMap[i+j*width])*BLACK_TO_WHITE_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+(j+1)*width])*WHITE_TO_BLACK_BIAS);
                else if (!img.pixel(i,j) && img.pixel(i+1,j+1))
                    g -> add_edge(i+j*width, (i+1)+(j+1)*width,
                                  (invDistMap[(i+1)+(j+1)*width]+invDistMap[i+j*width])*WHITE_TO_BLACK_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+(j+1)*width])*BLACK_TO_WHITE_BIAS);
                else
                    g -> add_edge(i+j*width, (i+1)+(j+1)*width,
                                  (invDistMap[(i+1)+(j+1)*width]+invDistMap[i+j*width])*WHITE_TO_WHITE_D_BIAS,
                                  (invDistMap[i+j*width]+invDistMap[(i+1)+(j+1)*width])*WHITE_TO_WHITE_D_BIAS);
            }
        }
    }
    
    int ret = g -> maxflow();
    
//    QImage debug2 = debug.convertToFormat(QImage::Format_RGB16);
//    QRgb lw = qRgb(255, 100, 100);
//    QRgb lb = qRgb(155, 0, 0);
//    QRgb rw = qRgb(100,255, 100);
//    QRgb rb = qRgb(0, 155, 0);
//    QRgb a = qRgb(255, 255, 255);
    
    //add all black pixels which
    for (int index=0; index<width*height; index++)
    {
        /*if (qGray(debug.pixel(index%width,index/width))!=BLACK && qGray(debug.pixel(index%width,index/width))!=WHITE)
        {
            debug2.setPixel(index%width,index/width,a);
        }
        else*/ if (g->what_segment(index) == GraphType::SOURCE)
        {
            outSource.append(index);
//            debug2.setPixel(index%width,index/width,lb);
        }
        else
        {
            outSink.append(index);
        }
//        else if (g->what_segment(index) == GraphType::SOURCE)
//            debug2.setPixel(index%width,index/width,lw);
//        else if (qGray(img.pixel(index%width,index/width))==BLACK)
//            debug2.setPixel(index%width,index/width,rb);
//        else
//            debug2.setPixel(index%width,index/width,rw);
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

