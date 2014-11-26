#include "wordseparator.h"
 
#include <math.h>

#include <stdlib.h>




int WordSeparator::recursiveCountTwoWords;            
int WordSeparator::recursiveCountFirstLetter;         
                                       
int WordSeparator::recursiveCountFull;                
int WordSeparator::recursiveSumTwoWords;              
int WordSeparator::recursiveSumFirstLetter;           
int WordSeparator::recursiveSumFull;                  
QMap<int,int> WordSeparator::recursiveModeTwoWords;   
QMap<int,int> WordSeparator::recursiveModeFirstLetter;
QMap<int,int> WordSeparator::recursiveModeFull; 


//This performs a horizontal separation of the image by creating a distance map and then doing a graph cut on it.
int WordSeparator::minCut(BPixelCollection &toCut, QVector<BPartition*> &ret)
{
    int toCut_width = toCut.width();
    int toCut_height = toCut.height();
    int num_pix = toCut_width*toCut_height;
    int invDistMap[num_pix];
    
    DistanceTransform::computeInverseDistanceMap(toCut,invDistMap);
    QVector<int> firstImgPixelIndexes;
    QVector<int> secondImgPixelIndexes;
    int maxflow = GraphCut::pixelsOfSeparation(invDistMap,toCut_width,toCut_height,toCut,firstImgPixelIndexes,secondImgPixelIndexes);
    
    BPartition* firstPart = new BPartition(&toCut);
    BPartition* secondPart = new BPartition(&toCut);
    
    
    foreach (int index, firstImgPixelIndexes)
    {
        int x = index%toCut_width;
        int y = index/toCut_width;
        firstPart->addPixelFromSrc(x,y);
//        toCut.getSrc()->setPixelOwner(x+xOffset,y+yOffset,firstPart,claimPortion);
    }
    
    foreach (int index, secondImgPixelIndexes)
    {
        int x = index%toCut_width;
        int y = index/toCut_width;
        secondPart->addPixelFromSrc(x,y);
//        toCut.getSrc()->setPixelOwner(x+xOffset,y+yOffset,secondPart,claimPortion);
    }
    
    
   
    ret.append(firstPart);
    ret.append(secondPart);
    return maxflow;
}

int WordSeparator::microMinCut(BPixelCollection &toCut, QVector<BPartition*> &ret)
{
    int toCut_width = toCut.width();
    int toCut_height = toCut.height();
    int num_pix = toCut_width*toCut_height;
    int invDistMap[num_pix];
    
    DistanceTransform::computeInverseDistanceMap(toCut,invDistMap);
    QVector<int> firstImgPixelIndexes;
    QVector<int> secondImgPixelIndexes;
    int maxflow = GraphCut::pixelsOfSeparationMicro(invDistMap,toCut_width,toCut_height,toCut,firstImgPixelIndexes,secondImgPixelIndexes);
    
    BPartition* firstPart = new BPartition(&toCut);
    BPartition* secondPart = new BPartition(&toCut);
    
    
    foreach (int index, firstImgPixelIndexes)
    {
        int x = index%toCut_width;
        int y = index/toCut_width;
        firstPart->addPixelFromSrc(x,y);
//        toCut.getSrc()->setPixelOwner(x+xOffset,y+yOffset,firstPart,claimPortion);
    }
    
    foreach (int index, secondImgPixelIndexes)
    {
        int x = index%toCut_width;
        int y = index/toCut_width;
        secondPart->addPixelFromSrc(x,y);
//        toCut.getSrc()->setPixelOwner(x+xOffset,y+yOffset,secondPart,claimPortion);
    }
    
    
   
    ret.append(firstPart);
    ret.append(secondPart);
    return maxflow;
}

int WordSeparator::dumbMinCut(BPixelCollection &toCut, QVector<BPartition*> &ret)
{
    int toCut_width = toCut.width();
    int toCut_height = toCut.height();
    int num_pix = toCut_width*toCut_height;
    int dumbInvDistMap[num_pix];
    
    for (int x=0; x<toCut_width; x++)
    {
        for (int y=0; y<toCut_height; y++)
        {
            if (toCut.pixel(x,y))
                dumbInvDistMap[x+toCut_width*y]=100;
            else
                dumbInvDistMap[x+toCut_width*y]=1;
        }
    }
    
    QVector<int> firstImgPixelIndexes;
    QVector<int> secondImgPixelIndexes;
    int maxflow = GraphCut::pixelsOfSeparation(dumbInvDistMap,toCut_width,toCut_height,toCut,firstImgPixelIndexes,secondImgPixelIndexes);
    
    BPartition* firstPart = new BPartition(&toCut);
    BPartition* secondPart = new BPartition(&toCut);
    
    
    foreach (int index, firstImgPixelIndexes)
    {
        int x = index%toCut_width;
        int y = index/toCut_width;
        firstPart->addPixelFromSrc(x,y);
//        toCut.getSrc()->setPixelOwner(x+xOffset,y+yOffset,firstPart,claimPortion);
    }
    
    foreach (int index, secondImgPixelIndexes)
    {
        int x = index%toCut_width;
        int y = index/toCut_width;
        secondPart->addPixelFromSrc(x,y);
//        toCut.getSrc()->setPixelOwner(x+xOffset,y+yOffset,secondPart,claimPortion);
    }
    
    
   
    ret.append(firstPart);
    ret.append(secondPart);
    return maxflow;
}

//This performs a verticle separation of two words, it identifys descenders in an attempt to increase accuracy
QVector<BPartition*> WordSeparator::horzCutEntries(BPixelCollection &img, int vert_divide)
{
    int num_pix = img.width()*img.height();
    //double pix_vals[num_pix];
    int invDistMap[num_pix];
    
    DistanceTransform::computeInverseDistanceMap(img,invDistMap);
    QVector<int> firstImgIndexes;
    QVector<int> secondImgIndexes;
    
    //best anchor weight:300 - 425
    int ANCHOR_WEIGHT = 270;
    GraphCut::pixelsOfSeparation(invDistMap,img.width(),img.height(),img,firstImgIndexes,secondImgIndexes,ANCHOR_WEIGHT,SPLIT_VERT,vert_divide);
    
    BPartition* firstPart = new BPartition(&img);
    BPartition* secondPart = new BPartition(&img);
    
    int img_width = img.width();
//    int img_height = img.height();
//    int h=0;
//    int w=0;
    foreach (int index, firstImgIndexes)
    {
        int x = index%img_width;
        int y = index/img_width;
//        if (y>h)
//        {
//            h=y;
//            w=x;
//        }
        firstPart->addPixelFromSrc(x,y);
//        img.getSrc()->setPixelOwner(x+xOffset,y+yOffset,firstPart,claimPortion);
    }
    
    foreach (int index, secondImgIndexes)
    {
        int x = index%img_width;
        int y = index/img_width;
        secondPart->addPixelFromSrc(x,y);
//        img.getSrc()->setPixelOwner(x+xOffset,y+yOffset,secondPart,claimPortion);
    }
    
    
//    printf("first height=%d, h=(%d,%d)\n",firstPart->height(),w,h);
   
    QVector<BPartition*> ret;
    ret.append(firstPart);
    ret.append(secondPart);
    return ret;
}

//This performs a verticle separation of two words based purly on the given divide
QVector<BPartition*> WordSeparator::dumbHorzCutEntries(BPixelCollection &img, int vert_divide)
{
    
    
    BPartition* firstPart = new BPartition(&img);
    BPartition* secondPart = new BPartition(&img);
    
    for (int y=0; y<vert_divide; y++)
    {
        for (int x=0; x<img.width(); x++)
        {
            firstPart->addPixelFromSrc(x,y);
        }
    }
    
    for (int y=vert_divide; y<img.height(); y++)
    {
        for (int x=0; x<img.width(); x++)
        {
            secondPart->addPixelFromSrc(x,y);
        }
    }
   
    QVector<BPartition*> ret;
    ret.append(firstPart);
    ret.append(secondPart);
    return ret;
}

QVector<BPartition*> WordSeparator::testSlopeCut(BPixelCollection &img, const NDimensions &dimensions, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds /*const QVector<QVector<double> > &slopes*/)
{
    int num_pix = img.width()*img.height();
    //double pix_vals[num_pix];
    int invDistMap[num_pix];
    
    DistanceTransform::computeInverseDistanceMap(img,invDistMap);
    QVector<int> firstImgIndexes;
    QVector<int> secondImgIndexes;
    
//    int maxflow = GraphCut::pixelsOfSeparationWithSlope(invDistMap,img.width(),img.height(),img, slopes,firstImgIndexes,secondImgIndexes);
    GraphCut::pixelsOfSeparationNDimensions(invDistMap,img.width(),img.height(),img, dimensions,sourceSeeds,sinkSeeds,firstImgIndexes,secondImgIndexes);
    
    BPartition* firstPart = new BPartition(&img);
    BPartition* secondPart = new BPartition(&img);
    
    int img_width = img.width();
//    int img_height = img.height();
    
    foreach (int index, firstImgIndexes)
    {
        int x = index%img_width;
        int y = index/img_width;
        firstPart->addPixelFromSrc(x,y);
//        img.getSrc()->setPixelOwner(x+xOffset,y+yOffset,firstPart,claimPortion);
    }
    
    foreach (int index, secondImgIndexes)
    {
        int x = index%img_width;
        int y = index/img_width;
        secondPart->addPixelFromSrc(x,y);
//        img.getSrc()->setPixelOwner(x+xOffset,y+yOffset,secondPart,claimPortion);
    }
    
    
    
   
    QVector<BPartition*> ret;
    ret.append(firstPart);
    ret.append(secondPart);
    return ret;
}

void WordSeparator::adjustHorzCutCrossOverAreas(BPartition* top, BPartition* bottom, QVector<QPoint> crossPoints, QVector<QVector<double> > descenderProbMap)
{
    if (crossPoints.empty())
        return;
    int SUBSECTION_WIDTH_FROM_KEYPOINT = 80;
    int SUBSECTION_HEIGHT_ABOVE_KEYPOINT = 40;//10 + crossPoints[0].y();
    int SUBSECTION_HEIGHT_BELOW_KEYPOINT = 60;
    int SUBSECTION_PIXEL_COUNT_MAX = 950;
    int NEW_SUBSECTION_WIDTH_FROM_KEYPOINT = 60;
    
    int SOURCE_SINK_SEED_BUFFER = 5;
    //Find cross-over contected components
    assert(top->getSrc() == bottom->getSrc());
    
    
    QVector<QPoint> workingStack;
    QVector<QPoint> ccccKeyPoints;
    QVector<QVector<QPoint> > ccccLowerPoints;
    
    //clear dvide line
//    int vert_divide = crossPoints[0].y();
    
    
//    mark.save("./mark.ppm");
    QMap<int,bool> skip;
    for (int cpIndex=0; cpIndex<crossPoints.size(); cpIndex++)
    {
        if (skip[cpIndex])
            continue;
        
        QPoint keyPoint = crossPoints[cpIndex];
        printf("Evaluating keyPoint(%d,%d)\n",keyPoint.x(),keyPoint.y());
        QVector<QPoint> keyPoints;
        keyPoints.append(keyPoint);
        int vert_divide = keyPoint.y();
        int leftmost_key = keyPoint.x();
        int rightmost_key = keyPoint.x();
        BImage mark = (top->getSrc())->makeImage();
//        for (int x=0; x<top->getSrc()->width(); x++)
//        {
//            mark.setPixel(x,vert_divide,false);
//        }
//        if (mark.pixel(keyPoint))
        {
            workingStack.push_back(keyPoint);
            mark.setPixel(keyPoint,false);
            bool topStarted = top->pixelIsMineSrc(keyPoint);
            QVector<QPoint> lowerPoints;
            BPartition subsection(top->getSrc());
            bool crossover=false;
            int subsectionTopPixelCount=0;
            int subsectionBottomPixelCount=0;
            QVector<QPoint> sourceStart;
            QVector<QPoint> sinkStart;
            QVector<QPoint> rightBorder;
            
            while (!workingStack.isEmpty())
            {   
                QPoint cur = workingStack.front();
                workingStack.pop_front();
                
                int bottomX = cur.x()-bottom->getXOffset();
                int bottomY = cur.y()-bottom->getYOffset();
                if ((bottomX>=0 && bottomX<bottom->width() && bottomY>=0 && bottomY<bottom->height() && bottom->pixelIsMine(bottomX, bottomY))/* && cur.y()>vert_divide*/)
                    lowerPoints.append(cur);
                
                
                if (   (cur.y()>vert_divide || 
                           (cpIndex==0 || 
                            (leftmost_key-cur.x()) < (leftmost_key-crossPoints[cpIndex-1 ].x())/2) &&
                           (cpIndex+keyPoints.size()>=crossPoints.size() || 
                            (cur.x()-rightmost_key) < (crossPoints[cpIndex+keyPoints.size()].x()-rightmost_key)/2)
                        ) &&
                        std::min(abs(cur.x()-leftmost_key),abs(cur.x()-rightmost_key)) < SUBSECTION_WIDTH_FROM_KEYPOINT && 
                        cur.y()-vert_divide < SUBSECTION_HEIGHT_BELOW_KEYPOINT &&
                        vert_divide-cur.y() < SUBSECTION_HEIGHT_ABOVE_KEYPOINT)
                {
                    
                    if (cur.y() <= vert_divide && 
                            (subsectionTopPixelCount < SUBSECTION_PIXEL_COUNT_MAX*keyPoints.size() /*|| abs(cur.x()-keyPoint.x())<10*/))
                    {
                        if (!crossover && top->pixelIsMineSrc(cur) != topStarted)
                        {
                            crossover=true;
                        }
                        
                        subsection.addPixelFromSrc(cur);
                        subsectionTopPixelCount++;
                    }
                    else if ((keyPoints.size()+cpIndex<crossPoints.size() && abs(crossPoints[keyPoints.size()+cpIndex].x()-rightmost_key) < SUBSECTION_WIDTH_FROM_KEYPOINT) || 
                            cur.y() > vert_divide && 
                            (subsectionBottomPixelCount < SUBSECTION_PIXEL_COUNT_MAX*keyPoints.size() /*|| abs(cur.x()-keyPoint.x())<10*/))
                    {
                        if (!crossover && top->pixelIsMineSrc(cur) != topStarted)
                        {
                            crossover=true;
                        }
                        
                        subsection.addPixelFromSrc(cur);
                        subsectionBottomPixelCount++;
                    }
                    else
                    {
                        //
                        
                        if (cur.y() <= vert_divide - SOURCE_SINK_SEED_BUFFER)
                            sourceStart.append(cur);
                        else if (cur.y() > vert_divide + SOURCE_SINK_SEED_BUFFER)
                            sinkStart.append(cur);
                        continue;
                    }
                }
                else if (    (cur.y()>vert_divide || cpIndex==0 || (leftmost_key-cur.x()) < (leftmost_key-crossPoints[cpIndex-1 ].x())/2) &&
                             std::min(abs(cur.x()-leftmost_key),abs(cur.x()-rightmost_key)) < SUBSECTION_WIDTH_FROM_KEYPOINT && 
                             cur.y()-vert_divide < SUBSECTION_HEIGHT_BELOW_KEYPOINT &&
                             vert_divide-cur.y() < SUBSECTION_HEIGHT_ABOVE_KEYPOINT)
                {
                    rightBorder.append(cur);
                    continue;
                }
                else
                {
                    //
                    
                    if (cur.y() <= vert_divide - SOURCE_SINK_SEED_BUFFER)
                        sourceStart.append(cur);
                    else if (cur.y() > vert_divide + SOURCE_SINK_SEED_BUFFER)
                        sinkStart.append(cur);
                    continue;
                }
                
              
                int tableIndex=8;
                int pIndex;
                for (int cc=0; cc<9; cc++)
                {
                    tableIndex=(tableIndex+2)%9;
                    if (tableIndex==4)
                        continue;
                    
                    int xDelta=(tableIndex%3)-1;
                    int yDelta=(tableIndex/3)-1;
                    int x = cur.x()+xDelta;
                    int y = cur.y()+yDelta;
                    if (x>=0 && x<mark.width() && y>=0 && y<mark.height() && 
                            mark.pixel(x,y) && 
                            (top->pixelSrc(x,y) || bottom->pixelSrc(x,y)))
                    {
                        QPoint pp(x,y);
                        workingStack.push_back(pp);
                        mark.setPixel(pp,false);
                        
                        if (yDelta<0 && (pIndex = crossPoints.indexOf(pp))!=-1)
                        {
                            keyPoints.append(pp);
                            if (pp.x()<leftmost_key)
                                leftmost_key=pp.x();
                            if(pp.x()>rightmost_key)
                            {
                                rightmost_key=pp.x();
                                workingStack += rightBorder;
                            }
                            
                            skip[pIndex]=true;
                        }
                    }
                }
                
            }
            
            foreach (QPoint p, rightBorder)
            {
                if (p.y() <= vert_divide - SOURCE_SINK_SEED_BUFFER)
                    sourceStart.append(p);
//                else if (p.y() > vert_divide + SOURCE_SINK_SEED_BUFFER)
//                    sinkStart.append(p);
            }
            
            if (crossover)
            {
                bool foundDesc=false;
                if (keyPoints.size()==1)
                {
                    ccccKeyPoints.append(keyPoint);
                    ccccLowerPoints.append(lowerPoints);
                    
                    ///test///
    //                BPartition test(top->getSrc());
    //                foreach (QPoint p, lowerPoints)
    //                {
    //                    test.addPixelFromSrc(p.x(),p.y());
    //                }
    //                QString xs;
    //               QString ys;
    //               xs.setNum(keyPoint.x());
    //               ys.setNum(keyPoint.y());
    //               QString loc = "./lowerPoints/lowerPoints";
    //               loc+=xs;
    //               loc+="_";
    //               loc+=ys;
    //               loc+=".ppm";
    //                test.makeImage().save(loc);
                    ///test///
                    
                    QVector<QVector<double> > scoreMap(descenderProbMap.size());
                    QVector<double> temp(descenderProbMap[0].size());
                    temp.fill(0);
                    scoreMap.fill(temp);
                    double scoreTotal=0;
    //                int num_points_outside=0;
                    foreach (QPoint ccccP, lowerPoints)
                    {
                        int x = ccccP.x()-keyPoint.x() + descenderProbMap.size()/2;
                        int y = ccccP.y()-keyPoint.y();
                        
                        if(x>=0 && x<descenderProbMap.size() && y>=0 && y<descenderProbMap[0].size())
                        {
                            scoreMap[x][y] = descenderProbMap[x][y];//-.4;
                            scoreTotal+=scoreMap[x][y];
                        }
    //                    else
    //                    {
    //                        scoreTotal-=.4;
    //                        num_points_outside++;
    //                    }
                        
                        
                    }
                    
    //                printf("total score for point (%d,%d): %f\n",keyPoint.x(),keyPoint.y(),scoreTotal);
    //                double DESCENDER_SCORE_THRESH = 45;
                    double scoreRatio = scoreTotal/lowerPoints.size();
    //                printf("scoreRario for point (%d,%d): %f\n",keyPoint.x(),keyPoint.y(),scoreRatio);
                    if (scoreRatio>.5)//if (scoreTotal>DESCENDER_SCORE_THRESH)
                    {
                        //if this is disconnected (all pixels are important in map) we can add all
                        //but what if we are intersected with something else?
                        
                        //expiriment
                        foreach (QPoint p, lowerPoints)
                        {
    //                        firstImgBlackPixelIndexes.append(p.x()+p.y()*img.width());
                            top->addPixelFromSrc(p);
                            bottom->removePixel(p);
                        }
                        foundDesc=true;
                    }
                }
                        
#if USE_3D_CUT       
                if (!foundDesc) /*if(false)//skipping*/
                {//do something fancy, like a 3D cut
                    
                    
                    
                    
                      //what is going on here? why do I make a second subsection?
                    
                    //Only do connected component
                    BPartition newSubsection(subsection.getSrc());
                    mark = subsection.makeImage();
                    
                    workingStack.clear();
                    QPoint rel_keyPoint(keyPoint.x()-subsection.getXOffset(), keyPoint.y()-subsection.getYOffset());
                    workingStack.push_back(rel_keyPoint);
                    while (!workingStack.isEmpty())
                    {   
                        QPoint cur = workingStack.front();
                        workingStack.pop_front();
                        
                        QPoint src_cur(cur.x()+subsection.getXOffset(), cur.y()+subsection.getYOffset());
                        
                        if (abs(src_cur.x()-keyPoint.x()) > NEW_SUBSECTION_WIDTH_FROM_KEYPOINT)
                        {
                            //add new anchors
                            if (src_cur.y() <= vert_divide - SOURCE_SINK_SEED_BUFFER)
                                sourceStart.append(src_cur);
                            else if (src_cur.y() > vert_divide + SOURCE_SINK_SEED_BUFFER)
                                sinkStart.append(src_cur);
                            continue;
                        }
                        
                        newSubsection.addPixelFromSrc(src_cur);
                        
                        int tableIndex=8;
                        for (int cc=0; cc<9; cc++)
                        {
                            tableIndex=(tableIndex+2)%9;
                            if (tableIndex==4)
                                continue;
                            
                            int xDelta=(tableIndex%3)-1;
                            int yDelta=(tableIndex/3)-1;
                            int x = cur.x()+xDelta;
                            int y = cur.y()+yDelta;
                            
                            if (x>0 && x<mark.width() && y>0 && y<mark.height() && mark.pixel(x,y) &&
                                    (yDelta<=0 || x==rel_keyPoint.x() || y!=rel_keyPoint.y()))
                            {
                                QPoint pp(x,y);
                                workingStack.push_back(pp);
                                mark.setPixel(pp,false);
                            }
                        }
                        
                    }

                    
                    QVector<QPoint> sourceSeeds;
                    foreach (QPoint p, sourceStart)
                    {
                        QPoint local(p.x()-newSubsection.getXOffset(),p.y()-newSubsection.getYOffset());
                        if (newSubsection.pixelSrc(p.x(),p.y()))
                            sourceSeeds.append(local);
                        else if (p.x()+1 < newSubsection.getSrc()->width() && newSubsection.pixelSrc(p.x()+1, p.y()))
                        {
                            QPoint newlocal(local.x()+1, local.y());
                            sourceSeeds.append(newlocal);
                        }
                        else if (p.x()-1 >= 0 && newSubsection.pixelSrc(p.x()-1, p.y()))
                        {
                            QPoint newlocal(local.x()-1, local.y());
                            sourceSeeds.append(newlocal);
                        }
                        else if (p.y()+1 < newSubsection.getSrc()->width() && newSubsection.pixelSrc(p.x(), p.y()+1))
                        {
                            QPoint newlocal(local.x(), local.y()+1);
                            sourceSeeds.append(newlocal);
                        }
                        else if (p.y()-1 >=0 && newSubsection.pixelSrc(p.x(), p.y()-1))
                        {
                            QPoint newlocal(local.x(), local.y()-1);
                            sourceSeeds.append(newlocal);
                        }
                    }
                    QVector<QPoint> sinkSeeds;
                    foreach (QPoint p, sinkStart)
                    {
                        QPoint local(p.x()-newSubsection.getXOffset(),p.y()-newSubsection.getYOffset());
                        if (newSubsection.pixelSrc(p.x(),p.y()))
                            sinkSeeds.append(local);
                        else if (p.x()+1 < newSubsection.getSrc()->width() && newSubsection.pixelSrc(p.x()+1, p.y()))
                        {
                            QPoint newlocal(local.x()+1, local.y());
                            sinkSeeds.append(newlocal);
                        }
                        else if (p.x()-1 >= 0 && newSubsection.pixelSrc(p.x()-1, p.y()))
                        {
                            QPoint newlocal(local.x()-1, local.y());
                            sinkSeeds.append(newlocal);
                        }
                        else if (p.y()+1 < newSubsection.getSrc()->width() && newSubsection.pixelSrc(p.x(), p.y()+1))
                        {
                            QPoint newlocal(local.x(), local.y()+1);
                            sinkSeeds.append(newlocal);
                        }
                        else if (p.y()-1 >=0 && newSubsection.pixelSrc(p.x(), p.y()-1))
                        {
                            QPoint newlocal(local.x(), local.y()-1);
                            sinkSeeds.append(newlocal);
                        }
                    }
                    
                    if (sourceSeeds.empty())
                    {
                        bool cont = true;
                        for (int y=0; y<newSubsection.height() && cont; y++)
                            for (int x=0; x<newSubsection.width() && cont; x++)
                                if (newSubsection.pixel(x,y))
                                {
                                    cont=false;
                                    QPoint toAdd(x,y);
                                    sourceSeeds.append(toAdd);
                                }
                    }
                    
                    if (sinkSeeds.empty())
                    {
                        bool cont = true;
                        for (int y=newSubsection.height()-1; y>=0 && cont; y--)
                            for (int x=0; x<newSubsection.width() && cont; x++)
                                if (newSubsection.pixel(x,y))
                                {
                                    cont=false;
                                    QPoint toAdd(x,y);
                                    sinkSeeds.append(toAdd);
                                }
                    }
                    QPoint newCrossOverPoint(keyPoint.x()-newSubsection.getXOffset(),keyPoint.y()-newSubsection.getYOffset());
                    QVector<QPoint> newKeyPoints;
                    foreach (QPoint kp, keyPoints)
                    {
                        QPoint nkp(kp.x()-newSubsection.getXOffset(),kp.y()-newSubsection.getYOffset());
                        newKeyPoints.append(nkp);
                    }

                    QVector<BPartition*> result3DCut = recut3D(newSubsection, sourceSeeds, sinkSeeds, newCrossOverPoint.y(),newKeyPoints);
//                    QVector<BPartition*> result3DCut = recut2D(newSubsection, sourceSeeds, sinkSeeds, newCrossOverPoint);
                
#if SAVE_SUBSECTION
                    ///test///start
                    QString xs;
                    QString ys;
                    xs.setNum(keyPoint.x());
                    ys.setNum(keyPoint.y());
                    QString loc = "./subsection/subsection";
                    loc+=xs;
                    loc+="_";
                    loc+=ys;
                    loc+=".ppm";
                    
                    newSubsection.makeImage().save(loc);
                    ///test///end
#endif
                    
                    
                    result3DCut[0]->changeSrc(newSubsection.getSrc(), newSubsection.getXOffset(), newSubsection.getYOffset());
                    result3DCut[1]->changeSrc(newSubsection.getSrc(), newSubsection.getXOffset(), newSubsection.getYOffset());
                    
                    
                    
                    for (int x=0; x<newSubsection.width(); x++)
                    {
                        for (int y=0; y<newSubsection.height(); y++)
                        {
                            int x_src = x+newSubsection.getXOffset();
                            int y_src = y+newSubsection.getYOffset();
                            if (top->pixelIsMineSrc(x_src,y_src) && newSubsection.pixelSrc(x_src,y_src))
                                top->removePixel(x_src,y_src);
                            
                            if (bottom->pixelIsMineSrc(x_src,y_src) && newSubsection.pixelSrc(x_src,y_src))
                                bottom->removePixel(x_src,y_src);
                        }
                    }
                    
                    for (int x=0; x<result3DCut[0]->width(); x++)
                    {
                        for (int y=0; y<result3DCut[0]->height(); y++)
                        {
                            int x_src = x+result3DCut[0]->getXOffset();
                            int y_src = y+result3DCut[0]->getYOffset();
                            if (result3DCut[0]->pixel(x,y))
                                top->addPixelFromSrc(x_src,y_src);
                        }
                    }
                    
                    for (int x=0; x<result3DCut[1]->width(); x++)
                    {
                        for (int y=0; y<result3DCut[1]->height(); y++)
                        {
                            int x_src = x+result3DCut[1]->getXOffset();
                            int y_src = y+result3DCut[1]->getYOffset();
                            if (result3DCut[1]->pixel(x,y))
                                bottom->addPixelFromSrc(x_src,y_src);
                        }
                    }
                    
//                    /test///
//                    BImage yep = result3DCut[0]->getSrc()->makeImage();
//                    result3DCut[0]->changeSrc(&yep, 0, 0);
//                    result3DCut[1]->changeSrc(&yep, 0, 0);
//                    yep.claimOwnership(result3DCut[0],.5);
//                    yep.claimOwnership(result3DCut[1],.5);
//                    yep.saveOwners("slope_separation.ppm");
//                    char read;
//                    printf("cont? ");
//                    scanf("%c",&read);
//                    /test///
                    
                    delete result3DCut[0];
                    delete result3DCut[1];
                    
                }
#endif
            }
            
        }
//        else
//        {
//            //TODO this case still needs handled
//        }
        
        
        
    }
}



QVector<BPartition*> WordSeparator::segmentLinesOfWords(const BPixelCollection &column, int spacingEstimate, bool dumb)
{
    QVector<BPartition*> ret;
//    BImage base(column);
//    base.save("./starting_image.ppm");
    
    
    int cutEstimate = spacingEstimate;
    
    
    
    QImage probMap("./average_desc.pgm");
    QVector<QVector<double> > descenderProbMap=ImageAverager::produceProbabilityMap(probMap);
    
    int test_count=0;
    
    int accumulativeXOffset=0;
    int accumulativeYOffset=0;
    
    QVector<int> dividingLines;
    BImage linesRemoved(column);
    QVector<QVector<QPoint> > crossPointsForLine;
    //remove/find lines
    while (cutEstimate < linesRemoved.height() - spacingEstimate/2)
    {
        int vert_divide;
        QVector<QPoint> crossPoints;
        linesRemoved = BoxCleaner::clearLineAndCloseLetters(linesRemoved,cutEstimate,&vert_divide,&crossPoints);
        cutEstimate = vert_divide + spacingEstimate;
        crossPointsForLine.append(crossPoints);
        dividingLines.append(vert_divide);
        
//        printf("cutline:%d\n",vert_divide);
    }
    
    BPartition* unfinishedCol = new BPartition(&linesRemoved);
    for (int x=0; x<linesRemoved.width(); x++)
    {
        for (int y=0; y<linesRemoved.height(); y++)
        {
            unfinishedCol->addPixelFromSrc(x,y);
        }
    }
    //segment
    for (int i=0; i<dividingLines.size(); i++)
    {
        printf("Cut %d:\t",i);
        
        int cutY = dividingLines[i] - accumulativeYOffset;
        QVector<BPartition*> cuts;
        if (dumb)
            cuts = dumbHorzCutEntries(*unfinishedCol,cutY);
        else
            cuts= horzCutEntries(*unfinishedCol,cutY);
        
        assert(cuts[0]->width()>5 && cuts[1]->width()>5);
        
//        cuts[0]->makeImage().save("./test0.ppm");
//        cuts[1]->makeImage().save("./test1.ppm");
//        lineremoved.claimOwnership(cuts[0],1);
//        lineremoved.saveOwners("./test.ppm");
//        printf("vert:%d, cuts[0].height=%d\n",dividingLines[i],cuts[0]->height());
        
//        int tempXOffset = accumulativeXOffset + cuts[1]->getXOffset();
//        int tempYOffset = accumulativeYOffset + cuts[1]->getYOffset();
        cuts[0]->changeSrc(&linesRemoved,accumulativeXOffset,accumulativeYOffset);
        cuts[1]->changeSrc(&linesRemoved,accumulativeXOffset,accumulativeYOffset);
        
        if(!dumb)
            adjustHorzCutCrossOverAreas(cuts[0],cuts[1],crossPointsForLine[i],descenderProbMap);
        accumulativeXOffset = cuts[1]->getXOffset();
        accumulativeYOffset = cuts[1]->getYOffset();
        cuts[0]->changeSrc(&column,0,0);
//        cuts[1]->changeSrc(&column,tempXOffset,tempYOffset);
        
//        printf("cuts[0].height after change=%d\n",cuts[0]->height());
        
        
        ret.append(cuts[0]);
        delete unfinishedCol;
        unfinishedCol = cuts[1];
        
        
//        cutEstimate = (unfinishedCol->height() - cut_dist_from_bottom) + spacingEstimate;
        
        //test//
        QString debugfile = "./segmentation/cut_";
        QString num;
        num.setNum(test_count++);
        debugfile.append(num);
        debugfile.append(".ppm");;
        cuts[0]->makeImage().save(debugfile);
        cuts[1]->makeImage().save("./segmentation/left.ppm");
        
//        if (i==39)
//        {
//            char read;
//            printf("cont? ");
//            scanf("%c",&read);
//        }
        //test//
    }
    unfinishedCol->changeSrc(&column,0,0);
    ret.append(unfinishedCol);
    
    return ret;
}




QVector<BPartition*> WordSeparator::recursiveHorizontalCutTwoWords(const BPixelCollection &img)
{
    QVector<BPartition*> ret;
//    BImage base(column);
//    base.save("./starting_image.ppm");
    BPartition* unfinished = new BPartition(&img);
    for (int x=0; x<img.width(); x++)
    {
        for (int y=0; y<img.height(); y++)
        {
            unfinished->addPixelFromSrc(x,y);
        }
    }

    int test_count=0;
    
    int accumulativeXOffset=0;
    int accumulativeYOffset=0;
    
    bool cont = true;
    int index=0;
    
//    QVector<char> cutStates;
//    QVector<int> maxFlows;
//    QVector<int> imgWidthsLeft;
//    QVector<int> imgWidthsRight;
//    QVector<int> pixCountsLeft;
//    QVector<int> pixCountsRight;
    QVector<BPartition*> cuts;
    int lastMaxflow = minCut(*unfinished,cuts);
    BPartition* leftUnfinished = cuts[0];
    BPartition* rightUnfinished = cuts[1];
    

    int leftWidth = leftUnfinished->width();
    int rightWidth = rightUnfinished->width();
    int leftCount=0;
    int rightCount=0;
    for (int x=0; x<cuts[0]->width(); x++)
    {
        for (int y=0; y<cuts[0]->height(); y++)
        {
            if (cuts[0]->pixel(x,y))
                leftCount++;
        }
    }
    for (int x=0; x<cuts[1]->width(); x++)
    {
        for (int y=0; y<cuts[1]->height(); y++)
        {
            if (cuts[1]->pixel(x,y))
                rightCount++;
        }
    }
    
//    char lastread='%';
    bool recurLeft;
    WordSeparator::recursiveCountTwoWords++;
    while (cont)
    {
        recursiveSumTwoWords++;
//        leftUnfinished->makeImage().save("./test0.ppm");
//        rightUnfinished->makeImage().save("./test1.ppm");
//        char dump;
//        printf("cont");
//        scanf("%c",&dump);
        
        
        int tempLeftXOffset = accumulativeXOffset + leftUnfinished->getXOffset();
        int tempLeftYOffset = accumulativeYOffset + leftUnfinished->getYOffset();
        int tempRightXOffset = accumulativeXOffset + rightUnfinished->getXOffset();
        int tempRightYOffset = accumulativeYOffset + rightUnfinished->getYOffset();
        
        leftUnfinished->changeSrc(&img,accumulativeXOffset,accumulativeYOffset);
        rightUnfinished->changeSrc(&img,accumulativeXOffset,accumulativeYOffset);
        
        QVector<BPartition*> leftCuts;
        int leftMaxflow = minCut(*leftUnfinished,leftCuts);
        QVector<BPartition*> rightCuts;
        int rightMaxflow = minCut(*rightUnfinished,rightCuts);
        
        int leftCutLeftCount=0;
        int leftCutRightCount=0;
        for (int x=0; x<leftCuts[0]->width(); x++)
        {
            for (int y=0; y<leftCuts[0]->height(); y++)
            {
                if (leftCuts[0]->pixel(x,y))
                    leftCutLeftCount++;
            }
        }
        for (int x=0; x<leftCuts[1]->width(); x++)
        {
            for (int y=0; y<leftCuts[1]->height(); y++)
            {
                if (leftCuts[1]->pixel(x,y))
                    leftCutRightCount++;
            }
        }
        int rightCutLeftCount=0;
        int rightCutRightCount=0;
        for (int x=0; x<rightCuts[0]->width(); x++)
        {
            for (int y=0; y<rightCuts[0]->height(); y++)
            {
                if (rightCuts[0]->pixel(x,y))
                    rightCutLeftCount++;
            }
        }
        for (int x=0; x<rightCuts[1]->width(); x++)
        {
            for (int y=0; y<rightCuts[1]->height(); y++)
            {
                if (rightCuts[1]->pixel(x,y))
                    rightCutRightCount++;
            }
        }
        
        //state check//
        
        char read;
        
        //tree Newest, tree uses min 20: 83.65%
        if ((leftWidth<=15 && rightWidth<=15) || (leftWidth<=2 || rightWidth<=2))//added for sanity
        {
            read='e';
            printf("Failsafe activated (twowords)\n");
        }
        else if (leftWidth <= 235)
        {
            if(rightCount <= 1318)
                if (rightCutLeftCount <= 257)
                    read='l';
                else
                    if (lastMaxflow-rightMaxflow <= -2023)
                        read = 'e';
                    else
                        read = 'l';
            else
            {
                if (leftWidth <= 119)
                    read='r';
                else
                    read='e';
            }
                
        }
        else
        {
            read='l';
        }
        //end tree
//        //tree
//        if ((leftWidth<=15 && rightWidth<=15) || (leftWidth<=2 || rightWidth<=2))//added for sanity
//        {
//            read='e';
//            printf("Failsafe activated (twowords)\n");
//        }
//        else if (rightCount <= 853)
//        {
//            if(leftCount <= 866)
//                read='e';
//            else
//                read='l';
//        }
//        else
//        {
//            if (leftWidth <= 90)
//            {
//                if (rightCutLeftCount <= 310)
//                {
//                    read='r';
//                }
//                else
//                {
//                    if (rightCuts[0]->width() <= 113)
//                        read='e';
//                    else
//                        read='r';
//                }
//            }
//            else
//            {
//                read='e';
//            }
//        }
//        //end tree
        
        if (read == 'e')
        {
            cont = false;
        }
        else if (read == 'l')
        {
            recurLeft = true;
            accumulativeXOffset = tempLeftXOffset;
            accumulativeYOffset = tempLeftYOffset;
        }
        else if (read == 'r')
        {
            recurLeft = false;
            accumulativeXOffset = tempRightXOffset;
            accumulativeYOffset = tempRightYOffset;
        }
        test_count++;
        


        
//        lastread=read;
        
        if (cont && recurLeft)
        {
            
            ret.insert(index,rightUnfinished);
            delete unfinished;
            delete rightCuts[0];
            delete rightCuts[1];
            
            unfinished = leftUnfinished;
            leftUnfinished=leftCuts[0];
            leftWidth = leftCuts[0]->width();
            leftCount = leftCutLeftCount;
            rightUnfinished=leftCuts[1];
            rightWidth = leftCuts[1]->width();
            rightCount = leftCutRightCount;
            lastMaxflow = leftMaxflow;
        }
        else if (cont)
        {
            ret.insert(index,leftUnfinished);
            index++;
            delete leftCuts[0];
            delete leftCuts[1];
            
            delete unfinished;
            unfinished = rightUnfinished;
            leftUnfinished=rightCuts[0];
            leftWidth = rightCuts[0]->width();
            leftCount = rightCutLeftCount;
            rightUnfinished=rightCuts[1];
            rightWidth = rightCuts[1]->width();
            rightCount = rightCutRightCount;
            lastMaxflow = rightMaxflow;
        }
        else
        {
//            ret.insert(index,unfinished);
            delete leftCuts[0];
            delete leftCuts[1];
            delete rightCuts[0];
            delete rightCuts[1];
//            delete leftUnfinished;
//            delete rightUnfinished;
            ret.insert(index++,leftUnfinished);
            ret.insert(index,rightUnfinished);
            delete unfinished;
            
        }
        
        
        
    }

    
    BPartition* left = new BPartition(&img);
    for (int i=0; i<index; i++)
    {
        left->join(ret[i]);
        delete ret[i];
    }
    BPartition* right = new BPartition(&img);
    for (int i=index; i<ret.size(); i++)
    {
        right->join(ret[i]);
        delete ret[i];
    }
    
    ret.clear();
    ret.append(left);
    ret.append(right);
    
    recursiveModeTwoWords[test_count] += 1;
    
    return ret;
}

QVector<BPartition*> WordSeparator::recursiveHorizontalCutTwoWordsTraining(const BPixelCollection &img)
{
    std::ofstream results ("./twowords_training_results.dat",std::ofstream::app);
    QVector<BPartition*> ret;
//    BImage base(column);
//    base.save("./starting_image.ppm");
    BPartition* unfinished = new BPartition(&img);
    for (int x=0; x<img.width(); x++)
    {
        for (int y=0; y<img.height(); y++)
        {
            unfinished->addPixelFromSrc(x,y);
        }
    }

    int test_count=0;
    
    int accumulativeXOffset=0;
    int accumulativeYOffset=0;
    
    bool cont = true;
    int index=0;
    
//    QVector<char> cutStates;
//    QVector<int> maxFlows;
//    QVector<int> imgWidthsLeft;
//    QVector<int> imgWidthsRight;
//    QVector<int> pixCountsLeft;
//    QVector<int> pixCountsRight;
    QVector<BPartition*> cuts;
    int lastMaxflow = minCut(*unfinished,cuts);
    BPartition* leftUnfinished = cuts[0];
    BPartition* rightUnfinished = cuts[1];
    

    int leftWidth = leftUnfinished->width();
    int rightWidth = rightUnfinished->width();
    int leftCount=0;
    int rightCount=0;
    for (int x=0; x<cuts[0]->width(); x++)
    {
        for (int y=0; y<cuts[0]->height(); y++)
        {
            if (cuts[0]->pixel(x,y))
                leftCount++;
        }
    }
    for (int x=0; x<cuts[1]->width(); x++)
    {
        for (int y=0; y<cuts[1]->height(); y++)
        {
            if (cuts[1]->pixel(x,y))
                rightCount++;
        }
    }
    
//    char lastread='%';
    bool recurLeft;
    
    while (cont)
    {
//        QVector<BPartition*> cuts;
//        int maxflow = minCut(*unfinished,cuts);
//        maxFlows.append(maxflow);
        
//        BImage test = unfinished->makeImage();
        leftUnfinished->makeImage().save("./test0.ppm");
        rightUnfinished->makeImage().save("./test1.ppm");
//        test.claimOwnership(cuts[0],1);
//        test.saveOwners("./test.ppm");
        
        
        
        int tempLeftXOffset = accumulativeXOffset + leftUnfinished->getXOffset();
        int tempLeftYOffset = accumulativeYOffset + leftUnfinished->getYOffset();
        int tempRightXOffset = accumulativeXOffset + rightUnfinished->getXOffset();
        int tempRightYOffset = accumulativeYOffset + rightUnfinished->getYOffset();
        
        leftUnfinished->changeSrc(&img,accumulativeXOffset,accumulativeYOffset);
        rightUnfinished->changeSrc(&img,accumulativeXOffset,accumulativeYOffset);
        
        QVector<BPartition*> leftCuts;
        int leftMaxflow = minCut(*leftUnfinished,leftCuts);
        QVector<BPartition*> rightCuts;
        int rightMaxflow = minCut(*rightUnfinished,rightCuts);
        
        int leftCutLeftCount=0;
        int leftCutRightCount=0;
        for (int x=0; x<leftCuts[0]->width(); x++)
        {
            for (int y=0; y<leftCuts[0]->height(); y++)
            {
                if (leftCuts[0]->pixel(x,y))
                    leftCutLeftCount++;
            }
        }
        for (int x=0; x<leftCuts[1]->width(); x++)
        {
            for (int y=0; y<leftCuts[1]->height(); y++)
            {
                if (leftCuts[1]->pixel(x,y))
                    leftCutRightCount++;
            }
        }
        int rightCutLeftCount=0;
        int rightCutRightCount=0;
        for (int x=0; x<rightCuts[0]->width(); x++)
        {
            for (int y=0; y<rightCuts[0]->height(); y++)
            {
                if (rightCuts[0]->pixel(x,y))
                    rightCutLeftCount++;
            }
        }
        for (int x=0; x<rightCuts[1]->width(); x++)
        {
            for (int y=0; y<rightCuts[1]->height(); y++)
            {
                if (rightCuts[1]->pixel(x,y))
                    rightCutRightCount++;
            }
        }
        
        //state check//
        
        char read;
        char dump;
        while(true)
        {
            printf("Recur%d:",test_count);
            scanf("%c%c",&read,&dump);
            if (read == 'l')
            {
                read='e';
                cont = false;
                break;
            }
            else if (read == ',')
            {
                read='l';
                recurLeft = true;
//                tempXOffset += cuts[0]->getXOffset();
//                tempYOffset += cuts[0]->getYOffset();
                accumulativeXOffset = tempLeftXOffset;
                accumulativeYOffset = tempLeftYOffset;
                break;
            }
            else if (read == '.')
            {
                read='r';
                recurLeft = false;
//                tempXOffset += cuts[1]->getXOffset();
//                tempYOffset += cuts[1]->getYOffset();
                accumulativeXOffset = tempRightXOffset;
                accumulativeYOffset = tempRightYOffset;
                break;
            }
        }
        test_count++;
        

        char buffer[150];
        sprintf(buffer,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%c",
                lastMaxflow,
                leftWidth,
                rightWidth,
                leftCount,
                rightCount,
                leftMaxflow,
                leftCuts[0]->width(),
                leftCuts[1]->width(),
                leftCutLeftCount,
                leftCutRightCount,
                rightMaxflow,
                rightCuts[0]->width(),
                rightCuts[1]->width(),
                rightCutLeftCount,
                rightCutRightCount,
                lastMaxflow-leftMaxflow,
                lastMaxflow-rightMaxflow,
                read);
        
        results << buffer << std::endl;

        
//        lastread=read;
        
        if (cont && recurLeft)
        {
            
            ret.insert(index,rightUnfinished);
            delete unfinished;
            delete rightCuts[0];
            delete rightCuts[1];
            
            unfinished = leftUnfinished;
            leftUnfinished=leftCuts[0];
            leftWidth = leftCuts[0]->width();
            leftCount = leftCutLeftCount;
            rightUnfinished=leftCuts[1];
            rightWidth = leftCuts[1]->width();
            rightCount = leftCutRightCount;
            lastMaxflow = leftMaxflow;
        }
        else if (cont)
        {
            ret.insert(index,leftUnfinished);
            index++;
            delete leftCuts[0];
            delete leftCuts[1];
            
            delete unfinished;
            unfinished = rightUnfinished;
            leftUnfinished=rightCuts[0];
            leftWidth = rightCuts[0]->width();
            leftCount = rightCutLeftCount;
            rightUnfinished=rightCuts[1];
            rightWidth = rightCuts[1]->width();
            rightCount = rightCutRightCount;
            lastMaxflow = rightMaxflow;
        }
        else
        {
//            ret.insert(index,unfinished);
            delete leftCuts[0];
            delete leftCuts[1];
            delete rightCuts[0];
            delete rightCuts[1];
//            delete leftUnfinished;
//            delete rightUnfinished;
            ret.insert(index++,leftUnfinished);
            ret.insert(index,rightUnfinished);
            delete unfinished;
            
        }
        
        
        
    }
    results.close();
    
//    if (recurLeft)
//    {
//        index++;
//    }
    
    BPartition* left = new BPartition(&img);
    for (int i=0; i<index; i++)
    {
        left->join(ret[i]);
        delete ret[i];
    }
    BPartition* right = new BPartition(&img);
    for (int i=index; i<ret.size(); i++)
    {
        right->join(ret[i]);
        delete ret[i];
    }
    
    ret.clear();
    ret.append(left);
    ret.append(right);
    
    return ret;
}


QVector<BPartition*> WordSeparator::recursiveHorizontalCutFirstLetter(const BPixelCollection &img)
{
    QVector<BPartition*> ret;
//    BImage base(column);
//    base.save("./starting_image.ppm");
    BPartition* unfinished = new BPartition(&img);
    for (int x=0; x<img.width(); x++)
    {
        for (int y=0; y<img.height(); y++)
        {
            unfinished->addPixelFromSrc(x,y);
        }
    }

    int test_count=0;
    
    int accumulativeXOffset=0;
    int accumulativeYOffset=0;
    
    bool cont = true;
    int index=0;
    
//    QVector<char> cutStates;
//    QVector<int> maxFlows;
//    QVector<int> imgWidthsLeft;
//    QVector<int> imgWidthsRight;
//    QVector<int> pixCountsLeft;
//    QVector<int> pixCountsRight;
    QVector<BPartition*> cuts;
    int lastMaxflow = minCut(*unfinished,cuts);
    BPartition* leftUnfinished = cuts[0];
    BPartition* rightUnfinished = cuts[1];
    

    int leftWidth = leftUnfinished->width();
    int rightWidth = rightUnfinished->width();
    int leftCount=0;
    int rightCount=0;
    for (int x=0; x<cuts[0]->width(); x++)
    {
        for (int y=0; y<cuts[0]->height(); y++)
        {
            if (cuts[0]->pixel(x,y))
                leftCount++;
        }
    }
    for (int x=0; x<cuts[1]->width(); x++)
    {
        for (int y=0; y<cuts[1]->height(); y++)
        {
            if (cuts[1]->pixel(x,y))
                rightCount++;
        }
    }
    
    recursiveCountFirstLetter++;
    while (cont)
    {
        recursiveSumFirstLetter++;
//        leftUnfinished->makeImage().save("./test0.ppm");
//        rightUnfinished->makeImage().save("./test1.ppm");
        
        
        
        int tempLeftXOffset = accumulativeXOffset + leftUnfinished->getXOffset();
        int tempLeftYOffset = accumulativeYOffset + leftUnfinished->getYOffset();
        int tempRightXOffset = accumulativeXOffset + rightUnfinished->getXOffset();
        int tempRightYOffset = accumulativeYOffset + rightUnfinished->getYOffset();
        
        leftUnfinished->changeSrc(&img,accumulativeXOffset,accumulativeYOffset);
        rightUnfinished->changeSrc(&img,accumulativeXOffset,accumulativeYOffset);
        
        QVector<BPartition*> leftCuts;
        int leftMaxflow = minCut(*leftUnfinished,leftCuts);
        QVector<BPartition*> rightCuts;
        int rightMaxflow = minCut(*rightUnfinished,rightCuts);
        
        int leftCutLeftCount=0;
        int leftCutRightCount=0;
        for (int x=0; x<leftCuts[0]->width(); x++)
        {
            for (int y=0; y<leftCuts[0]->height(); y++)
            {
                if (leftCuts[0]->pixel(x,y))
                    leftCutLeftCount++;
            }
        }
        for (int x=0; x<leftCuts[1]->width(); x++)
        {
            for (int y=0; y<leftCuts[1]->height(); y++)
            {
                if (leftCuts[1]->pixel(x,y))
                    leftCutRightCount++;
            }
        }
        int rightCutLeftCount=0;
        int rightCutRightCount=0;
        for (int x=0; x<rightCuts[0]->width(); x++)
        {
            for (int y=0; y<rightCuts[0]->height(); y++)
            {
                if (rightCuts[0]->pixel(x,y))
                    rightCutLeftCount++;
            }
        }
        for (int x=0; x<rightCuts[1]->width(); x++)
        {
            for (int y=0; y<rightCuts[1]->height(); y++)
            {
                if (rightCuts[1]->pixel(x,y))
                    rightCutRightCount++;
            }
        }
        
        //state check//
        bool recurLeft;
        char read;
        //tree  Newest 20 min:93.28%
        if (lastMaxflow <= 15095)
        {
            if (leftCount <= 476)
                read='e';
            else
                read='l';
        }
        else
        {
            if (leftCount <= 973)
                read='e';
            else
                read='l';
                
        }
        //end tree
//        //tree  15bin
//        if (leftCutLeftCount <= 316)
//        {
//            read='e';
//        }
//        else
//        {
//            if (leftMaxflow <= 15754)
//            {
//                if (leftCount <= 1003)
//                {
//                    if (leftMaxflow <= 12613)
//                    {
//                        read='l';
//                    }
//                    else
//                    {
//                        if (leftCutLeftCount <= 523)
//                            read='e';
//                        else
//                            read='l';
//                    }
//                }
//                else
//                {
//                    read='l';
//                }
//            }
//            else
//            {
//                if (leftCutLeftCount <= 605)
//                {
//                    read='e';
//                }
//                else
//                {
//                    if (leftCount <= 954)
//                        read='e';
//                    else
//                        read='l';
//                }
//            }
//        }
//        //end tree

        if (read == 'e')
        {
            cont = false;
        }
        else if (read == 'l')
        {
            recurLeft = true;
            accumulativeXOffset = tempLeftXOffset;
            accumulativeYOffset = tempLeftYOffset;
        }
        else if (read == 'r')
        {
            recurLeft = false;
            accumulativeXOffset = tempRightXOffset;
            accumulativeYOffset = tempRightYOffset;
        }
        test_count++;
        
        
        if (cont && recurLeft)
        {
            
            ret.insert(index,rightUnfinished);
            delete unfinished;
            delete rightCuts[0];
            delete rightCuts[1];
            
            unfinished = leftUnfinished;
            leftUnfinished=leftCuts[0];
            leftWidth = leftCuts[0]->width();
            leftCount = leftCutLeftCount;
            rightUnfinished=leftCuts[1];
            rightWidth = leftCuts[1]->width();
            rightCount = leftCutRightCount;
            lastMaxflow = leftMaxflow;
        }
        else if (cont)
        {
            ret.insert(index,leftUnfinished);
            index++;
            delete leftCuts[0];
            delete leftCuts[1];
            
            delete unfinished;
            unfinished = rightUnfinished;
            leftUnfinished=rightCuts[0];
            leftWidth = rightCuts[0]->width();
            leftCount = rightCutLeftCount;
            rightUnfinished=rightCuts[1];
            rightWidth = rightCuts[1]->width();
            rightCount = rightCutRightCount;
            lastMaxflow = rightMaxflow;
        }
        else//no overshoot
        {
//            ret.insert(index,unfinished);
            delete leftCuts[0];
            delete leftCuts[1];
            delete rightCuts[0];
            delete rightCuts[1];
//            delete leftUnfinished;
//            delete rightUnfinished;
            ret.insert(index++,leftUnfinished);
            ret.insert(index,rightUnfinished);
            delete unfinished;
            
        }
        
        
        
    }
    recursiveModeFirstLetter[test_count] += 1;
    return ret;
}

bool WordSeparator::recursiveHorizontalCutFirstLetterTraining(const BPixelCollection &img, QVector<BPartition*> &ret)
{
    std::ofstream results ("./firstletter_training_results.dat",std::ofstream::app);
    
//    BImage base(column);
//    base.save("./starting_image.ppm");
    BPartition* unfinished = new BPartition(&img);
    for (int x=0; x<img.width(); x++)
    {
        for (int y=0; y<img.height(); y++)
        {
            unfinished->addPixelFromSrc(x,y);
        }
    }

    int test_count=0;
    
    int accumulativeXOffset=0;
    int accumulativeYOffset=0;
    
    bool cont = true;
    int index=0;
    
//    QVector<char> cutStates;
//    QVector<int> maxFlows;
//    QVector<int> imgWidthsLeft;
//    QVector<int> imgWidthsRight;
//    QVector<int> pixCountsLeft;
//    QVector<int> pixCountsRight;
    QVector<BPartition*> cuts;
    int lastMaxflow = minCut(*unfinished,cuts);
    BPartition* leftUnfinished = cuts[0];
    BPartition* rightUnfinished = cuts[1];
    

    int leftWidth = leftUnfinished->width();
    int rightWidth = rightUnfinished->width();
    int leftCount=0;
    int rightCount=0;
    for (int x=0; x<cuts[0]->width(); x++)
    {
        for (int y=0; y<cuts[0]->height(); y++)
        {
            if (cuts[0]->pixel(x,y))
                leftCount++;
        }
    }
    for (int x=0; x<cuts[1]->width(); x++)
    {
        for (int y=0; y<cuts[1]->height(); y++)
        {
            if (cuts[1]->pixel(x,y))
                rightCount++;
        }
    }
    
    char lastread='%';
    char buffer[150];
    while (cont)
    {
//        QVector<BPartition*> cuts;
//        int maxflow = minCut(*unfinished,cuts);
//        maxFlows.append(maxflow);
        
//        BImage test = unfinished->makeImage();
        leftUnfinished->makeImage().save("./test0.ppm");
        rightUnfinished->makeImage().save("./test1.ppm");
//        test.claimOwnership(cuts[0],1);
//        test.saveOwners("./test.ppm");
        
        
        
        int tempLeftXOffset = accumulativeXOffset + leftUnfinished->getXOffset();
        int tempLeftYOffset = accumulativeYOffset + leftUnfinished->getYOffset();
        int tempRightXOffset = accumulativeXOffset + rightUnfinished->getXOffset();
        int tempRightYOffset = accumulativeYOffset + rightUnfinished->getYOffset();
        
        leftUnfinished->changeSrc(&img,accumulativeXOffset,accumulativeYOffset);
        rightUnfinished->changeSrc(&img,accumulativeXOffset,accumulativeYOffset);
        
        QVector<BPartition*> leftCuts;
        int leftMaxflow = minCut(*leftUnfinished,leftCuts);
        QVector<BPartition*> rightCuts;
        int rightMaxflow = minCut(*rightUnfinished,rightCuts);
        
        int leftCutLeftCount=0;
        int leftCutRightCount=0;
        for (int x=0; x<leftCuts[0]->width(); x++)
        {
            for (int y=0; y<leftCuts[0]->height(); y++)
            {
                if (leftCuts[0]->pixel(x,y))
                    leftCutLeftCount++;
            }
        }
        for (int x=0; x<leftCuts[1]->width(); x++)
        {
            for (int y=0; y<leftCuts[1]->height(); y++)
            {
                if (leftCuts[1]->pixel(x,y))
                    leftCutRightCount++;
            }
        }
        int rightCutLeftCount=0;
        int rightCutRightCount=0;
        for (int x=0; x<rightCuts[0]->width(); x++)
        {
            for (int y=0; y<rightCuts[0]->height(); y++)
            {
                if (rightCuts[0]->pixel(x,y))
                    rightCutLeftCount++;
            }
        }
        for (int x=0; x<rightCuts[1]->width(); x++)
        {
            for (int y=0; y<rightCuts[1]->height(); y++)
            {
                if (rightCuts[1]->pixel(x,y))
                    rightCutRightCount++;
            }
        }
        
        //state check//
        bool recurLeft;
        char read;
        char dump;
        while(true)
        {
            printf("Recur%d:",test_count);
            scanf("%c%c",&read,&dump);
            if (read == 'l')
            {
                read='e';
                cont = false;
                break;
            }
            else if (read == ',')
            {
                read='l';
                recurLeft = true;
//                tempXOffset += cuts[0]->getXOffset();
//                tempYOffset += cuts[0]->getYOffset();
                accumulativeXOffset = tempLeftXOffset;
                accumulativeYOffset = tempLeftYOffset;
                break;
            }
            else if (read == '.')
            {
                read='r';
                recurLeft = false;
//                tempXOffset += cuts[1]->getXOffset();
//                tempYOffset += cuts[1]->getYOffset();
                accumulativeXOffset = tempRightXOffset;
                accumulativeYOffset = tempRightYOffset;
                break;
            }
            else if (read == 's')
            {
                
                results << lastread << std::endl;
                results.close();
                printf("lastread=%c\n",lastread);
                return test_count>1;
            }
        }
        test_count++;
        
        
        {
            if (test_count>1)
                results << buffer;
            
            sprintf(buffer,"%c\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,",
                    lastread,
                    lastMaxflow,
                    leftWidth,
                    rightWidth,
                    leftCount,
                    rightCount,
                    leftMaxflow,
                    leftCuts[0]->width(),
                    leftCuts[1]->width(),
                    leftCutLeftCount,
                    leftCutRightCount,
                    rightMaxflow,
                    rightCuts[0]->width(),
                    rightCuts[1]->width(),
                    rightCutLeftCount,
                    rightCutRightCount,
                    lastMaxflow-leftMaxflow,
                    lastMaxflow-rightMaxflow);
            
            
        }
        if (!cont)
        {
//            char buffer[5];
//            sprintf(buffer,"%c",lastread);
            
            results << buffer << 'e' << std::endl;
        }
        
        lastread=read;
        
        if (cont && recurLeft)
        {
            
            ret.insert(index,rightUnfinished);
            delete unfinished;
            delete rightCuts[0];
            delete rightCuts[1];
            
            unfinished = leftUnfinished;
            leftUnfinished=leftCuts[0];
            leftWidth = leftCuts[0]->width();
            leftCount = leftCutLeftCount;
            rightUnfinished=leftCuts[1];
            rightWidth = leftCuts[1]->width();
            rightCount = leftCutRightCount;
            lastMaxflow = leftMaxflow;
        }
        else if (cont)
        {
            ret.insert(index,leftUnfinished);
            index++;
            delete leftCuts[0];
            delete leftCuts[1];
            
            delete unfinished;
            unfinished = rightUnfinished;
            leftUnfinished=rightCuts[0];
            leftWidth = rightCuts[0]->width();
            leftCount = rightCutLeftCount;
            rightUnfinished=rightCuts[1];
            rightWidth = rightCuts[1]->width();
            rightCount = rightCutRightCount;
            lastMaxflow = rightMaxflow;
        }
        else//overshoot
        {
            ret.insert(index,unfinished);
            delete leftCuts[0];
            delete leftCuts[1];
            delete rightCuts[0];
            delete rightCuts[1];
            delete leftUnfinished;
            delete rightUnfinished;
//            ret.insert(cur.index++,cur.leftUnfinished);
//            ret.insert(cur.index,cur.rightUnfinished);
//            delete cur.unfinished;
            
        }
        
        
        
    }
    results.close();
    return true;
}

struct side
{
    int index;
    int accumulativeXOffset;
    int accumulativeYOffset;
    BPartition* unfinished;
    BPartition* leftUnfinished;
    int leftWidth;
    int leftCount;
    BPartition* rightUnfinished;
    int rightWidth;
    int rightCount;
    int costOfCut;
    
};

QVector<BPartition*> WordSeparator::recursiveHorizontalCutFullTraining(const BPixelCollection &img)
{
    std::ofstream results ("./training_results.dat",std::ofstream::app);
    
    QVector<BPartition*> ret;
//    BImage base(column);
//    base.save("./starting_image.ppm");
    BPartition* unfinished = new BPartition(&img);
    for (int x=0; x<img.width(); x++)
    {
        for (int y=0; y<img.height(); y++)
        {
            unfinished->addPixelFromSrc(x,y);
        }
    }

    int test_count=0;
    
    int accumulativeXOffset=0;
    int accumulativeYOffset=0;
    
    
    int insertIndex=0;
    
//    QVector<char> cutStates;
//    QVector<int> maxFlows;
//    QVector<int> futureLeftMaxFlows;
//    QVector<int> futureRightMaxFlows;
//    QVector<int> imgWidthsLeft;
//    QVector<int> imgWidthsRight;
//    QVector<int> pixCountsLeft;
//    QVector<int> pixCountsRight;
    
    QVector<BPartition*> cuts;
    int maxflow = minCut(*unfinished,cuts);
    
//    imgWidthsLeft.append(cuts[0]->width());
//    imgWidthsRight.append(cuts[1]->width());
    int leftCount=0;
    int rightCount=0;
    for (int x=0; x<cuts[0]->width(); x++)
    {
        for (int y=0; y<cuts[0]->height(); y++)
        {
            if (cuts[0]->pixel(x,y))
                leftCount++;
        }
    }
    for (int x=0; x<cuts[1]->width(); x++)
    {
        for (int y=0; y<cuts[1]->height(); y++)
        {
            if (cuts[1]->pixel(x,y))
                rightCount++;
        }
    }
//    pixCountsLeft.append(leftCount);
//    pixCountsRight.append(rightCount);
    
    side first;
    first.index=insertIndex;
    first.accumulativeXOffset=accumulativeXOffset;
    first.accumulativeYOffset=accumulativeYOffset;
    first.unfinished = unfinished;
    first.leftUnfinished = cuts[0];
    first.leftWidth=cuts[0]->width();
    first.leftCount=leftCount;
    first.rightUnfinished = cuts[1];
    first.rightWidth=cuts[1]->width();
    first.rightCount=rightCount;
    first.costOfCut = maxflow;
    QVector<side> stack;
    stack.append(first);
    
    while (!stack.empty())
    {
        bool cont = true;
        side cur = stack.back();
        stack.pop_back();
        while (cont)
        {
            
            
//            maxFlows.append(cur.costOfCut);
            
    //        BImage test = unfinished->makeImage();
            cur.leftUnfinished->makeImage().save("./test0.ppm");
            cur.rightUnfinished->makeImage().save("./test1.ppm");
    //        test.claimOwnership(cuts[0],1);
    //        test.saveOwners("./test.ppm");
            
            
            
            int tempXOffset = cur.accumulativeXOffset;
            int tempYOffset = cur.accumulativeYOffset;
            
            //state check//
            side newSide;
            bool recurLeft;
            char read;
            char dump;
            while(true)
            {
                printf("Recur%d:",test_count);
                scanf("%c%c",&read,&dump);
                if (read == 'l')
                {
//                    cutStates.append('e');
                    read = 'e';
                    cont = false;
                    break;
                }
                else if (read == ',')
                {
//                    cutStates.append('l');
                    read='l';
                    recurLeft = true;
                    tempXOffset += cur.leftUnfinished->getXOffset();
                    tempYOffset += cur.leftUnfinished->getYOffset();
                    break;
                }
                else if (read == '.')
                {
//                    cutStates.append('r');
                    read='r';
                    recurLeft = false;
                    tempXOffset += cur.rightUnfinished->getXOffset();
                    tempYOffset += cur.rightUnfinished->getYOffset();
                    break;
                }
                else if (read == ' ')
                {
                    //both
                    
                    newSide.accumulativeXOffset = tempXOffset + cur.leftUnfinished->getXOffset();
                    newSide.accumulativeYOffset = tempYOffset + cur.leftUnfinished->getYOffset();
                    newSide.index = cur.index;
                    
//                    cutStates.append('b');
                    read='b';
                    recurLeft = false;
                    tempXOffset += cur.rightUnfinished->getXOffset();
                    tempYOffset += cur.rightUnfinished->getYOffset();
                    
                    //split
                    
                    break;
                }
            }
            test_count++;
            
            cur.leftUnfinished->changeSrc(&img,cur.accumulativeXOffset,cur.accumulativeYOffset);
            cur.rightUnfinished->changeSrc(&img,cur.accumulativeXOffset,cur.accumulativeYOffset);
            
            
            cur.accumulativeXOffset = tempXOffset;
            cur.accumulativeYOffset = tempYOffset;
            
            QVector<BPartition*> leftCuts;
            int leftMaxflow = minCut(*cur.leftUnfinished,leftCuts);
            QVector<BPartition*> rightCuts;
            int rightMaxflow = minCut(*cur.rightUnfinished,rightCuts);
            
            int leftCutLeftCount=0;
            int leftCutRightCount=0;
            for (int x=0; x<leftCuts[0]->width(); x++)
            {
                for (int y=0; y<leftCuts[0]->height(); y++)
                {
                    if (leftCuts[0]->pixel(x,y))
                        leftCutLeftCount++;
                }
            }
            for (int x=0; x<leftCuts[1]->width(); x++)
            {
                for (int y=0; y<leftCuts[1]->height(); y++)
                {
                    if (leftCuts[1]->pixel(x,y))
                        leftCutRightCount++;
                }
            }
            int rightCutLeftCount=0;
            int rightCutRightCount=0;
            for (int x=0; x<rightCuts[0]->width(); x++)
            {
                for (int y=0; y<rightCuts[0]->height(); y++)
                {
                    if (rightCuts[0]->pixel(x,y))
                        rightCutLeftCount++;
                }
            }
            for (int x=0; x<rightCuts[1]->width(); x++)
            {
                for (int y=0; y<rightCuts[1]->height(); y++)
                {
                    if (rightCuts[1]->pixel(x,y))
                        rightCutRightCount++;
                }
            }
            char buffer[150];
            sprintf(buffer,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%c",
                   cur.costOfCut,
                   cur.leftWidth,
                   cur.rightWidth,
                   cur.leftCount,
                   cur.rightCount,
                   leftMaxflow,
                   leftCuts[0]->width(),
                   leftCuts[1]->width(),
                   leftCutLeftCount,
                   leftCutRightCount,
                   rightMaxflow,
                   rightCuts[0]->width(),
                   rightCuts[1]->width(),
                   rightCutLeftCount,
                   rightCutRightCount,
                   cur.costOfCut-leftMaxflow,
                   cur.costOfCut-rightMaxflow,
                   read);
            
            results << buffer << std::endl;
            
            
            if (cont && recurLeft)
            {
                
                ret.insert(cur.index,cur.rightUnfinished);
                delete cur.unfinished;
                delete rightCuts[0];
                delete rightCuts[1];
                
                cur.unfinished = cur.leftUnfinished;
                cur.leftUnfinished=leftCuts[0];
                cur.leftWidth = leftCuts[0]->width();
                cur.leftCount = leftCutLeftCount;
                cur.rightUnfinished=leftCuts[1];
                cur.rightWidth = leftCuts[1]->width();
                cur.rightCount = leftCutRightCount;
                cur.costOfCut = leftMaxflow;
            }
            else if (cont)
            {
                if (read!='b')
                {
                    ret.insert(cur.index,cur.leftUnfinished);
                    cur.index++;
                    delete leftCuts[0];
                    delete leftCuts[1];
                }
                else
                {
                    newSide.unfinished = cur.leftUnfinished;
                    newSide.leftUnfinished=leftCuts[0];
                    newSide.leftWidth = leftCuts[0]->width();
                    newSide.leftCount = leftCutLeftCount;
                    newSide.rightUnfinished=leftCuts[1];
                    newSide.rightWidth = leftCuts[1]->width();
                    newSide.rightCount = leftCutRightCount;
                    newSide.costOfCut = leftMaxflow;
                    stack.append(newSide);
                }
                
                delete cur.unfinished;
                cur.unfinished = cur.rightUnfinished;
                cur.leftUnfinished=rightCuts[0];
                cur.leftWidth = rightCuts[0]->width();
                cur.leftCount = rightCutLeftCount;
                cur.rightUnfinished=rightCuts[1];
                cur.rightWidth = rightCuts[1]->width();
                cur.rightCount = rightCutRightCount;
                cur.costOfCut = rightMaxflow;
            }
            else
            {
                ret.insert(cur.index++,cur.leftUnfinished);
                ret.insert(cur.index,cur.rightUnfinished);
                delete cur.unfinished;
                delete leftCuts[0];
                delete leftCuts[1];
                delete rightCuts[0];
                delete rightCuts[1];
//                overshoot
//                delete cur.leftUnfinished;
//                delete cur.rightUnfinished;
//                ret.insert(cur.index,cur.unfinished);
            }
            
            
            
            
            
        }
    }
    results.close();
//    int magic = 10;
//    QString p;
//    for (int i=0; i<magic; i++)
//    {
//        if (cutStates.size()>i)
//        {
//            p += QString(cutStates[i]);
//            p += QString(",");
//            p += QString::number(maxFlows[i]);
//            p += QString(",");
//            p += QString::number(imgWidthsLeft[i]);
//            p += QString(",");
//            p += QString::number(imgWidthsRight[i]);
//            p += QString(",");
//            p += QString::number(pixCountsLeft[i]);
//            p += QString(",");
//            p+= QString::number(pixCountsRight[i]);
//            p += QString(",");
//        }
//        else
//        {
//            p += QString(cutStates.last());
//            p += QString(",");
//            p += QString::number(maxFlows.last());
//            p += QString(",");
//            p += QString::number(imgWidthsLeft.last());
//            p += QString(",");
//            p += QString::number(imgWidthsRight.last());
//            p += QString(",");
//            p += QString::number(pixCountsLeft.last());
//            p += QString(",");
//            p+= QString::number(pixCountsRight.last());
//            p += QString(",");
            
//        }
//    }
//    p[p.size()-1]='\n';
//    printf("%s",qPrintable(p) );
    return ret;
}


QVector<BPartition*> WordSeparator::recursiveHorizontalCutFull(const BPixelCollection &img)
{
    
    QVector<BPartition*> ret;
//    BImage base(column);
//    base.save("./starting_image.ppm");
    BPartition* unfinished = new BPartition(&img);
    for (int x=0; x<img.width(); x++)
    {
        for (int y=0; y<img.height(); y++)
        {
            unfinished->addPixelFromSrc(x,y);
        }
    }

    int test_count=0;
    
    int accumulativeXOffset=0;
    int accumulativeYOffset=0;
    
    
    int insertIndex=0;
    
//    QVector<char> cutStates;
//    QVector<int> maxFlows;
//    QVector<int> futureLeftMaxFlows;
//    QVector<int> futureRightMaxFlows;
//    QVector<int> imgWidthsLeft;
//    QVector<int> imgWidthsRight;
//    QVector<int> pixCountsLeft;
//    QVector<int> pixCountsRight;
    
    QVector<BPartition*> cuts;
    int maxflow = minCut(*unfinished,cuts);
    
//    imgWidthsLeft.append(cuts[0]->width());
//    imgWidthsRight.append(cuts[1]->width());
    int leftCount=0;
    int rightCount=0;
    for (int x=0; x<cuts[0]->width(); x++)
    {
        for (int y=0; y<cuts[0]->height(); y++)
        {
            if (cuts[0]->pixel(x,y))
                leftCount++;
        }
    }
    for (int x=0; x<cuts[1]->width(); x++)
    {
        for (int y=0; y<cuts[1]->height(); y++)
        {
            if (cuts[1]->pixel(x,y))
                rightCount++;
        }
    }
//    pixCountsLeft.append(leftCount);
//    pixCountsRight.append(rightCount);
    
    side first;
    first.index=insertIndex;
    first.accumulativeXOffset=accumulativeXOffset;
    first.accumulativeYOffset=accumulativeYOffset;
    first.unfinished = unfinished;
    first.leftUnfinished = cuts[0];
    first.leftWidth=cuts[0]->width();
    first.leftCount=leftCount;
    first.rightUnfinished = cuts[1];
    first.rightWidth=cuts[1]->width();
    first.rightCount=rightCount;
    first.costOfCut = maxflow;
    QVector<side> stack;
    stack.append(first);
    
    recursiveCountFull++;
    while (!stack.empty())
    {
        recursiveSumFull++;
        bool cont = true;
        side cur = stack.back();
        stack.pop_back();
        while (cont)
        {
//            cur.leftUnfinished->makeImage().save("./test0.ppm");
//            cur.rightUnfinished->makeImage().save("./test1.ppm");
//            char dump;
//            printf("cont\n");
//            scanf("%c",&dump);
            
            int tempLeftXOffset = cur.accumulativeXOffset + cur.leftUnfinished->getXOffset();
            int tempLeftYOffset = cur.accumulativeYOffset + cur.leftUnfinished->getYOffset();
            int tempRightXOffset = cur.accumulativeXOffset + cur.rightUnfinished->getXOffset();
            int tempRightYOffset = cur.accumulativeYOffset + cur.rightUnfinished->getYOffset();
            
            cur.leftUnfinished->changeSrc(&img,cur.accumulativeXOffset,cur.accumulativeYOffset);
            cur.rightUnfinished->changeSrc(&img,cur.accumulativeXOffset,cur.accumulativeYOffset);
            
            QVector<BPartition*> leftCuts;
            int leftMaxflow = minCut(*cur.leftUnfinished,leftCuts);
            QVector<BPartition*> rightCuts;
            int rightMaxflow = minCut(*cur.rightUnfinished,rightCuts);
            
            int leftCutLeftCount=0;
            int leftCutRightCount=0;
            for (int x=0; x<leftCuts[0]->width(); x++)
            {
                for (int y=0; y<leftCuts[0]->height(); y++)
                {
                    if (leftCuts[0]->pixel(x,y))
                        leftCutLeftCount++;
                }
            }
            for (int x=0; x<leftCuts[1]->width(); x++)
            {
                for (int y=0; y<leftCuts[1]->height(); y++)
                {
                    if (leftCuts[1]->pixel(x,y))
                        leftCutRightCount++;
                }
            }
            int rightCutLeftCount=0;
            int rightCutRightCount=0;
            for (int x=0; x<rightCuts[0]->width(); x++)
            {
                for (int y=0; y<rightCuts[0]->height(); y++)
                {
                    if (rightCuts[0]->pixel(x,y))
                        rightCutLeftCount++;
                }
            }
            for (int x=0; x<rightCuts[1]->width(); x++)
            {
                for (int y=0; y<rightCuts[1]->height(); y++)
                {
                    if (rightCuts[1]->pixel(x,y))
                        rightCutRightCount++;
                }
            }
            //print
            
            //state check//
            side newSide;
            bool recurLeft;
            char read;
            
            //tree goes here
            if (leftMaxflow <= 4194)
            {
                if (cur.leftWidth <= 104)
                    read='e';
                else if (cur.costOfCut-rightMaxflow <= -3128)
                {
                    read='l';
                }
                else
                {
                    read='b';
                }
            }
            else
            {
                if (rightCuts[1]->width() <= 29)
                    read = 'e';
                else if (rightMaxflow <= 4347)
                {
                    read = 'r';
                }
                else
                {
                    read ='e';
                }
            }
            //end tree
            
            if (read == 'e')
            {
                cont = false;
            }
            else if (read == 'l')
            {
                recurLeft = true;
                cur.accumulativeXOffset = tempLeftXOffset;
                cur.accumulativeYOffset = tempLeftYOffset;
            }
            else if (read == 'r')
            {
                recurLeft = false;
                cur.accumulativeXOffset = tempRightXOffset;
                cur.accumulativeYOffset = tempRightYOffset;
            }
            else if (read == 'b')
            {
                
                newSide.accumulativeXOffset = tempLeftXOffset;
                newSide.accumulativeYOffset = tempLeftYOffset;
                newSide.index = cur.index;
                
                recurLeft = false;
                cur.accumulativeXOffset = tempRightXOffset;
                cur.accumulativeYOffset = tempRightYOffset;
                
                //split
                
            }
            
            test_count++;
           
            
            
            
            
            if (cont && recurLeft)
            {
                
                ret.insert(cur.index,cur.rightUnfinished);
                delete cur.unfinished;
                delete rightCuts[0];
                delete rightCuts[1];
                
                cur.unfinished = cur.leftUnfinished;
                cur.leftUnfinished=leftCuts[0];
                cur.leftWidth = leftCuts[0]->width();
                cur.leftCount = leftCutLeftCount;
                cur.rightUnfinished=leftCuts[1];
                cur.rightWidth = leftCuts[1]->width();
                cur.rightCount = leftCutRightCount;
                cur.costOfCut = leftMaxflow;
            }
            else if (cont)
            {
                if (read!='b')
                {
                    ret.insert(cur.index,cur.leftUnfinished);
                    cur.index++;
                    delete leftCuts[0];
                    delete leftCuts[1];
                }
                else
                {
                    newSide.unfinished = cur.leftUnfinished;
                    newSide.leftUnfinished=leftCuts[0];
                    newSide.leftWidth = leftCuts[0]->width();
                    newSide.leftCount = leftCutLeftCount;
                    newSide.rightUnfinished=leftCuts[1];
                    newSide.rightWidth = leftCuts[1]->width();
                    newSide.rightCount = leftCutRightCount;
                    newSide.costOfCut = leftMaxflow;
                    stack.append(newSide);
                }
                
                delete cur.unfinished;
                cur.unfinished = cur.rightUnfinished;
                cur.leftUnfinished=rightCuts[0];
                cur.leftWidth = rightCuts[0]->width();
                cur.leftCount = rightCutLeftCount;
                cur.rightUnfinished=rightCuts[1];
                cur.rightWidth = rightCuts[1]->width();
                cur.rightCount = rightCutRightCount;
                cur.costOfCut = rightMaxflow;
            }
            else
            {
                ret.insert(cur.index++,cur.leftUnfinished);
                ret.insert(cur.index,cur.rightUnfinished);
                delete cur.unfinished;
                delete leftCuts[0];
                delete leftCuts[1];
                delete rightCuts[0];
                delete rightCuts[1];
            }
            
            
            
            
            
        }
    }
    recursiveModeFull[test_count] += 1;
    return ret;
}



QVector<BPartition*> WordSeparator::recut3D(const BPixelCollection &img, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds, int crossOverY, const QVector<QPoint> &crossOverPoints)
{
    
    int numOfBins = std::min((img.width()+img.height())/2, 80);//std::min(img.width(),img.height());
    
    AngleImage angleImage(&img,numOfBins,0.0,PI);
    printf("%d x %d x %d\n",img.width(),img.height(),numOfBins);
    
    /////begin intensity
    
    //we first must read in a parallel, greyscale file
//    QImage testg(argv[2]);
//    GImage gimg(testg);
    
//    BPartition pbCells(&bimg);
    
//    GPartition pgCells(&gimg);
    
    
//    int x1=473;
//    int y1=2056;
//    int x2=900;
//    int y2=2137;
//    for (int x=x1; x<=x2; x++)
//    {
//        for (int y=y1; y<=y2; y++)
//        {
//            if (bimg.pixel(x,y))
//            {
//                pbCells.addPixelFromSrc(x,y);
//            }
//        }
//    }
//    BImage bCells= pbCells.makeImage();
//    bCells = BoxCleaner::clearLineAndCloseLetters(bCells,40);
//    bCells.save("./test.ppm");
    
//    for (int x=x1; x<=x2; x++)
//    {
//        for (int y=y1; y<=y2; y++)
//        {
//            if (bCells.pixel(x-x1,y-y1))
//            {
//                pgCells.addPixelFromSrc(x,y);
//            }
//        }
//    }
//    QImage temp = pgCells.makeImage();
//    GImage gCells(temp);
    
//    Dimension intensities(bCells.width(),bCells.height());
    
//    //I'll first try a naive scale approach
//    for (int x=0; x<bCells.width(); x++)
//    {
//        for (int y=0; y<bCells.height(); y++)
//        {
////            QVector<double> intensity(1);
//            if (bCells.pixel(x,y))
//            {
//                int count = 0;
//                double average = 0;//gCells.pixel(x,y);
//                for (int i=-2; i<=2; i++)
//                {
//                    for (int j=-2; j<=2; j++)
//                    {
//                        if ((i!=-2||j!=-2) && (i!=2||j!=-2) && (i!=-2||j!=2) && (i!=2||j!=2) && x+i>=0 && y+j>=0 && x+i<bCells.width() && y+j<bCells.height() && bCells.pixel(x+i,y+j))
//                        {
//                            count++;
//                            average += gCells.pixel(x+i,y+j);
//                        }
//                    }
//                }
//                average /=count;
//                intensities.setValueForPixel(x,y,average);
//            }
//            else
//            {
////                intensity[0]=0;
//                intensities.setValueForPixel(x,y,0);
//            }
            
//        }
//    }
//    intensities.setNumOfBins(50);
//    intensities.setMinMax(0,255);
//    NDimensions dimensions;
//    dimensions.addDimension(intensities);
            
    /////end intensity
    
    
    
//    int invDistMap[num_pix];
    
//    DistanceTransform::computeInverseDistanceMap(img,invDistMap);
    QVector<int> firstImgIndexes;
    QVector<int> secondImgIndexes;
    
//    int maxflow = GraphCut::pixelsOfSeparationWithSlope(invDistMap,img.width(),img.height(),img, slopes,firstImgIndexes,secondImgIndexes);
//    int maxflow = GraphCut::pixelsOfSeparationNDimensions(invDistMap,img.width(),img.height(),img, dimensions,sourceSeeds,sinkSeeds,firstImgIndexes,secondImgIndexes);
//    int maxflow = GraphCut::pixelsOfSeparation(invDistMap,img.width(),img.height(),img, angleImage,sourceSeeds,sinkSeeds,firstImgIndexes,secondImgIndexes);
    
    ///test 3ddist
//    bool* img3d = new bool[angleImage.width()*angleImage.height()*numOfBins];
//    int dim[3];
//    dim[0]=angleImage.width();
//    dim[1]=angleImage.height();
//    dim[2]=numOfBins;
//    IndexerKD ind(3,dim);
//    float max_dist=1000;
//    for (int x=0; x<angleImage.width(); x++)
//    {
//        for (int y=0; y<angleImage.height(); y++)
//        {
////            int pass[3];
////            pass[0]=x; pass[1]=y;
////            img3d[ind.getIndex(pass)]=angleImage.pixel(x,y);
//            for (int z=0; z<numOfBins; z++)
//            {
//                int pass[3];
//                pass[0]=x; pass[1]=y; pass[2]=z;
//                img3d[ind.getIndex(pass)]=false;
////                image3d(x,y,z,0)=max_dist;
//            }
            
//            QMap<int,double> bins = angleImage.getBinsAndStrForPixel(x,y);
//            foreach(int bin, bins.keys())
//            {
//                if (bins[bin]>.2)
//                {
                    
//                    for (int e=1; e<10*bins[bin]; e++)
//                    {
//                        int pass[3];
//                        pass[0]=x; pass[1]=y; pass[2]=mod(bin+e,numOfBins);
//                        img3d[ind.getIndex(pass)]=true;
//                        pass[0]=x; pass[1]=y; pass[2]=mod(bin-e,numOfBins);
//                        img3d[ind.getIndex(pass)]=true;
////                        image3d(x,y,mod(bin+e,numOfBins),0)=0;
////                        image3d(x,y,mod(bin+e,numOfBins),0)=0;
//                    }
//                }
//            }
//        }
//    }
    
    double* img3d = new double[angleImage.width()*angleImage.height()*numOfBins];
    Indexer3D ind(angleImage.width(),angleImage.height());
    for (int x=0; x<angleImage.width(); x++)
    {
        for (int y=0; y<angleImage.height(); y++)
        {
            for (int z=0; z<numOfBins; z++)
                img3d[ind.getIndex(x,y,z)]=0.0;
            
            QMap<int,double> bins = angleImage.getBinsAndStrForPixel(x,y);
            foreach(int bin, bins.keys())
            {
//                for ()
                img3d[ind.getIndex(x,y,bin)]=bins[bin];
            }
        }
    }
    long* distmap3d = new long[angleImage.width()*angleImage.height()*numOfBins];
//    DistanceTransform::computeKDInverseDistanceMap(img3d,distmap,3,dim);old
//    DistanceTransform::compute3DInverseDistanceMapTest(img3d,distmap,angleImage.width(),angleImage.height(),numOfBins);old
    DistanceTransform::compute3DInverseDistanceMapNew(img3d,distmap3d,angleImage.width(),angleImage.height(),numOfBins,img);
    
    
    GraphCut::pixelsOfSeparationRecut3D(distmap3d,img.width(),img.height(),numOfBins,angleImage,sourceSeeds,sinkSeeds,firstImgIndexes,secondImgIndexes, crossOverY, crossOverPoints, INT_POS_INFINITY/2);
    
    delete[] distmap3d;
    delete[] img3d;
    
    BPartition* firstPart = new BPartition(&img);
    BPartition* secondPart = new BPartition(&img);
    
    int img_width = img.width();
    
    
    
    
    foreach (int index, firstImgIndexes)
    {
        int x = index%img_width;
        int y = index/img_width;
        firstPart->addPixelFromSrc(x,y);
        
        
        
    }
    
    foreach (int index, secondImgIndexes)
    {
        int x = index%img_width;
        int y = index/img_width;
        secondPart->addPixelFromSrc(x,y);
    }
    
    ///test//
//    firstPart->makeImage().save("./3d1.ppm");
//    secondPart->makeImage().save("./3d2.ppm");
//    char dump;
//    printf("3D cut done.");
//    scanf("%c",&dump);
    ///test///
   
    QVector<BPartition*> ret;
    ret.append(firstPart);
    ret.append(secondPart);
    return ret;
}

QVector<BPartition*> WordSeparator::cutGivenSeeds(const BPixelCollection &img, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds)
{
    int num_pix = img.width()*img.height();
    //double pix_vals[num_pix];
    int invDistMap[num_pix];
    
    DistanceTransform::computeInverseDistanceMap(img,invDistMap);
    QVector<int> firstImgIndexes;
    QVector<int> secondImgIndexes;
    
    GraphCut::pixelsOfSeparationExperimental(invDistMap,img.width(),img.height(),img,sourceSeeds,sinkSeeds,firstImgIndexes,secondImgIndexes);
    
    BPartition* firstPart = new BPartition(&img);
    BPartition* secondPart = new BPartition(&img);
    
    int img_width = img.width();
//    int img_height = img.height();
    
    foreach (int index, firstImgIndexes)
    {
        int x = index%img_width;
        int y = index/img_width;
        firstPart->addPixelFromSrc(x,y);
//        img.getSrc()->setPixelOwner(x+xOffset,y+yOffset,firstPart,claimPortion);
    }
    
    foreach (int index, secondImgIndexes)
    {
        int x = index%img_width;
        int y = index/img_width;
        secondPart->addPixelFromSrc(x,y);
//        img.getSrc()->setPixelOwner(x+xOffset,y+yOffset,secondPart,claimPortion);
    }
  
    
    QVector<BPartition*> ret;
    ret.append(firstPart);
    ret.append(secondPart);
    return ret;
}

QVector<BPartition*> WordSeparator::recut2D(const BPixelCollection &img, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds, const QPoint &crossOverPoint)
{
    
    
    
    int invDistMap[img.width()*img.height()];
    
    DistanceTransform::computeInverseDistanceMap(img,invDistMap);
    QVector<int> firstImgIndexes;
    QVector<int> secondImgIndexes;
    
    ///test3ddist
    
    GraphCut::pixelsOfSeparationRecut2D(img,invDistMap,img.width(),img.height(),sourceSeeds,sinkSeeds,firstImgIndexes,secondImgIndexes, crossOverPoint);
    
    BPartition* firstPart = new BPartition(&img);
    BPartition* secondPart = new BPartition(&img);
    
    int img_width = img.width();
    
    foreach (int index, firstImgIndexes)
    {
        int x = index%img_width;
        int y = index/img_width;
        firstPart->addPixelFromSrc(x,y);
    }
    
    foreach (int index, secondImgIndexes)
    {
        int x = index%img_width;
        int y = index/img_width;
        secondPart->addPixelFromSrc(x,y);
    }
    
    ///test//
//    firstPart->makeImage().save("./3d1.ppm");
//    secondPart->makeImage().save("./3d2.ppm");
//    char dump;
//    printf("3D cut done.");
//    scanf("%c",&dump);
    ///test///
   
    QVector<BPartition*> ret;
    ret.append(firstPart);
    ret.append(secondPart);
    return ret;
}





QVector<BPartition*> WordSeparator::recursiveChunkWord(const BPixelCollection *img, const BPixelCollection *root, int accumulativeXOffset, int accumulativeYOffset)
{
    QVector<BPartition*> ret;
//    BImage base(column);
//    base.save("./starting_image.ppm");
    BPartition* unfinished = new BPartition(img);
    for (int x=0; x<img->width(); x++)
    {
        for (int y=0; y<img->height(); y++)
        {
            unfinished->addPixelFromSrc(x,y);
        }
    }

    QVector<BPartition*> cuts;
    int maxflow = microMinCut(*unfinished,cuts);
    BPartition* leftUnfinished = cuts[0];
    BPartition* rightUnfinished = cuts[1];
    
    int leftCount=0;
    int rightCount=0;
    for (int x=0; x<cuts[0]->width(); x++)
    {
        for (int y=0; y<cuts[0]->height(); y++)
        {
            if (cuts[0]->pixel(x,y))
                leftCount++;
        }
    }
    for (int x=0; x<cuts[1]->width(); x++)
    {
        for (int y=0; y<cuts[1]->height(); y++)
        {
            if (cuts[1]->pixel(x,y))
                rightCount++;
        }
    }
    int leftWidth = leftUnfinished->width();
    int rightWidth = rightUnfinished->width();
    printf("cut lw=%d,\trw=%d\n",leftWidth,rightWidth);
//    leftUnfinished->makeImage().save("./test0.ppm");
//    rightUnfinished->makeImage().save("./test1.ppm");
    printf("maxflow=%d\n",maxflow);
    if (maxflow > 20000 || maxflow <= 0 || leftWidth < 9 || rightWidth < 9 || leftCount < 40 || rightCount < 40)
    {
        unfinished->changeSrc(root,accumulativeXOffset,accumulativeYOffset);
        ret += unfinished;
        delete leftUnfinished;
        delete rightUnfinished;
        return ret;
    }
    
//    char a;
//    printf("Cont? ");
//    scanf("%c",&a);
//    printf("ok\n");

    
    if (leftWidth > 20 && rightWidth!= 0)//left again
    {
        ret += recursiveChunkWord(leftUnfinished,root, accumulativeXOffset + leftUnfinished->getXOffset(), accumulativeYOffset + leftUnfinished->getYOffset());
        delete leftUnfinished;
    }
    else
    {
        leftUnfinished->changeSrc(root,accumulativeXOffset,accumulativeYOffset);
        ret += leftUnfinished;
        
    }
    
    if (rightWidth > 20 && leftWidth!= 0)//right again
    {
        ret += recursiveChunkWord(rightUnfinished,root, accumulativeXOffset + rightUnfinished->getXOffset(), accumulativeYOffset + rightUnfinished->getYOffset());
        delete rightUnfinished;
        
    }
    else
    {
        rightUnfinished->changeSrc(root,accumulativeXOffset,accumulativeYOffset);
        ret += rightUnfinished;
    }
    
    
    return ret;
}



/////DUMB///////
QVector<BPartition*> WordSeparator::recursiveHorizontalCutTwoWordsDumb(const BPixelCollection &img)
{
    QVector<BPartition*> ret;
//    BImage base(column);
//    base.save("./starting_image.ppm");
    BPartition* unfinished = new BPartition(&img);
    for (int x=0; x<img.width(); x++)
    {
        for (int y=0; y<img.height(); y++)
        {
            unfinished->addPixelFromSrc(x,y);
        }
    }

    int test_count=0;
    
    int accumulativeXOffset=0;
    int accumulativeYOffset=0;
    
    bool cont = true;
    int index=0;
    
//    QVector<char> cutStates;
//    QVector<int> maxFlows;
//    QVector<int> imgWidthsLeft;
//    QVector<int> imgWidthsRight;
//    QVector<int> pixCountsLeft;
//    QVector<int> pixCountsRight;
    QVector<BPartition*> cuts;
    int lastMaxflow = dumbMinCut(*unfinished,cuts);
    BPartition* leftUnfinished = cuts[0];
    BPartition* rightUnfinished = cuts[1];
    

    int leftWidth = leftUnfinished->width();
    int rightWidth = rightUnfinished->width();
    int leftCount=0;
    int rightCount=0;
    for (int x=0; x<cuts[0]->width(); x++)
    {
        for (int y=0; y<cuts[0]->height(); y++)
        {
            if (cuts[0]->pixel(x,y))
                leftCount++;
        }
    }
    for (int x=0; x<cuts[1]->width(); x++)
    {
        for (int y=0; y<cuts[1]->height(); y++)
        {
            if (cuts[1]->pixel(x,y))
                rightCount++;
        }
    }
    
//    char lastread='%';
    bool recurLeft;
    
    while (cont)
    {
//        leftUnfinished->makeImage().save("./test0.ppm");
//        rightUnfinished->makeImage().save("./test1.ppm");
//        char dump;
//        printf("cont");
//        scanf("%c",&dump);
        
        
        int tempLeftXOffset = accumulativeXOffset + leftUnfinished->getXOffset();
        int tempLeftYOffset = accumulativeYOffset + leftUnfinished->getYOffset();
        int tempRightXOffset = accumulativeXOffset + rightUnfinished->getXOffset();
        int tempRightYOffset = accumulativeYOffset + rightUnfinished->getYOffset();
        
        leftUnfinished->changeSrc(&img,accumulativeXOffset,accumulativeYOffset);
        rightUnfinished->changeSrc(&img,accumulativeXOffset,accumulativeYOffset);
        
        QVector<BPartition*> leftCuts;
        int leftMaxflow = dumbMinCut(*leftUnfinished,leftCuts);
        QVector<BPartition*> rightCuts;
        int rightMaxflow = dumbMinCut(*rightUnfinished,rightCuts);
        
        int leftCutLeftCount=0;
        int leftCutRightCount=0;
        for (int x=0; x<leftCuts[0]->width(); x++)
        {
            for (int y=0; y<leftCuts[0]->height(); y++)
            {
                if (leftCuts[0]->pixel(x,y))
                    leftCutLeftCount++;
            }
        }
        for (int x=0; x<leftCuts[1]->width(); x++)
        {
            for (int y=0; y<leftCuts[1]->height(); y++)
            {
                if (leftCuts[1]->pixel(x,y))
                    leftCutRightCount++;
            }
        }
        int rightCutLeftCount=0;
        int rightCutRightCount=0;
        for (int x=0; x<rightCuts[0]->width(); x++)
        {
            for (int y=0; y<rightCuts[0]->height(); y++)
            {
                if (rightCuts[0]->pixel(x,y))
                    rightCutLeftCount++;
            }
        }
        for (int x=0; x<rightCuts[1]->width(); x++)
        {
            for (int y=0; y<rightCuts[1]->height(); y++)
            {
                if (rightCuts[1]->pixel(x,y))
                    rightCutRightCount++;
            }
        }
        
        //state check//
        
        char read;
        
        //tree Newest, tree uses min 20: 83.65%
        if ((leftWidth<=15 && rightWidth<=15) || (leftWidth<=2 || rightWidth<=2))//added for sanity
        {
            read='e';
            printf("Failsafe activated (twowords)\n");
        }
        else if (leftWidth <= 224)
        {
            if(rightCount <= 1442)
                if (rightCount <= 450)
                    read='l';
                else
                    if (rightMaxflow <= 1823)
                        if (leftCount <= 1805)
                            read='e';
                        else
                        {
                            if (lastMaxflow-leftMaxflow <= -111)
                                read='e';
                            else
                                read='l';
                        }
                    else
                        read = 'e';
            else
            {
                if (leftWidth <= 109)
                    read='r';
                else
                    read='e';
            }
                
        }
        else
        {
            read='l';
        }
        //end tree
        
        if (read == 'e')
        {
            cont = false;
        }
        else if (read == 'l')
        {
            recurLeft = true;
            accumulativeXOffset = tempLeftXOffset;
            accumulativeYOffset = tempLeftYOffset;
        }
        else if (read == 'r')
        {
            recurLeft = false;
            accumulativeXOffset = tempRightXOffset;
            accumulativeYOffset = tempRightYOffset;
        }
        test_count++;
        


        
//        lastread=read;
        
        if (cont && recurLeft)
        {
            
            ret.insert(index,rightUnfinished);
            delete unfinished;
            delete rightCuts[0];
            delete rightCuts[1];
            
            unfinished = leftUnfinished;
            leftUnfinished=leftCuts[0];
            leftWidth = leftCuts[0]->width();
            leftCount = leftCutLeftCount;
            rightUnfinished=leftCuts[1];
            rightWidth = leftCuts[1]->width();
            rightCount = leftCutRightCount;
            lastMaxflow = leftMaxflow;
        }
        else if (cont)
        {
            ret.insert(index,leftUnfinished);
            index++;
            delete leftCuts[0];
            delete leftCuts[1];
            
            delete unfinished;
            unfinished = rightUnfinished;
            leftUnfinished=rightCuts[0];
            leftWidth = rightCuts[0]->width();
            leftCount = rightCutLeftCount;
            rightUnfinished=rightCuts[1];
            rightWidth = rightCuts[1]->width();
            rightCount = rightCutRightCount;
            lastMaxflow = rightMaxflow;
        }
        else
        {
//            ret.insert(index,unfinished);
            delete leftCuts[0];
            delete leftCuts[1];
            delete rightCuts[0];
            delete rightCuts[1];
//            delete leftUnfinished;
//            delete rightUnfinished;
            ret.insert(index++,leftUnfinished);
            ret.insert(index,rightUnfinished);
            delete unfinished;
            
        }
        
        
        
    }

    
    BPartition* left = new BPartition(&img);
    for (int i=0; i<index; i++)
    {
        left->join(ret[i]);
        delete ret[i];
    }
    BPartition* right = new BPartition(&img);
    for (int i=index; i<ret.size(); i++)
    {
        right->join(ret[i]);
        delete ret[i];
    }
    
    ret.clear();
    ret.append(left);
    ret.append(right);
    
    return ret;
}

QVector<BPartition*> WordSeparator::recursiveHorizontalCutTwoWordsTrainingDumb(const BPixelCollection &img)
{
    std::ofstream results ("./dumb_twowords_training_results_.dat",std::ofstream::app);
    QVector<BPartition*> ret;
//    BImage base(column);
//    base.save("./starting_image.ppm");
    BPartition* unfinished = new BPartition(&img);
    for (int x=0; x<img.width(); x++)
    {
        for (int y=0; y<img.height(); y++)
        {
            unfinished->addPixelFromSrc(x,y);
        }
    }

    int test_count=0;
    
    int accumulativeXOffset=0;
    int accumulativeYOffset=0;
    
    bool cont = true;
    int index=0;
    
//    QVector<char> cutStates;
//    QVector<int> maxFlows;
//    QVector<int> imgWidthsLeft;
//    QVector<int> imgWidthsRight;
//    QVector<int> pixCountsLeft;
//    QVector<int> pixCountsRight;
    QVector<BPartition*> cuts;
    int lastMaxflow = dumbMinCut(*unfinished,cuts);
    BPartition* leftUnfinished = cuts[0];
    BPartition* rightUnfinished = cuts[1];
    

    int leftWidth = leftUnfinished->width();
    int rightWidth = rightUnfinished->width();
    int leftCount=0;
    int rightCount=0;
    for (int x=0; x<cuts[0]->width(); x++)
    {
        for (int y=0; y<cuts[0]->height(); y++)
        {
            if (cuts[0]->pixel(x,y))
                leftCount++;
        }
    }
    for (int x=0; x<cuts[1]->width(); x++)
    {
        for (int y=0; y<cuts[1]->height(); y++)
        {
            if (cuts[1]->pixel(x,y))
                rightCount++;
        }
    }
    
//    char lastread='%';
    bool recurLeft;
    
    while (cont)
    {
//        QVector<BPartition*> cuts;
//        int maxflow = dumbMinCut(*unfinished,cuts);
//        maxFlows.append(maxflow);
        
//        BImage test = unfinished->makeImage();
        leftUnfinished->makeImage().save("./test0.ppm");
        rightUnfinished->makeImage().save("./test1.ppm");
//        test.claimOwnership(cuts[0],1);
//        test.saveOwners("./test.ppm");
        
        
        
        int tempLeftXOffset = accumulativeXOffset + leftUnfinished->getXOffset();
        int tempLeftYOffset = accumulativeYOffset + leftUnfinished->getYOffset();
        int tempRightXOffset = accumulativeXOffset + rightUnfinished->getXOffset();
        int tempRightYOffset = accumulativeYOffset + rightUnfinished->getYOffset();
        
        leftUnfinished->changeSrc(&img,accumulativeXOffset,accumulativeYOffset);
        rightUnfinished->changeSrc(&img,accumulativeXOffset,accumulativeYOffset);
        
        QVector<BPartition*> leftCuts;
        int leftMaxflow = dumbMinCut(*leftUnfinished,leftCuts);
        QVector<BPartition*> rightCuts;
        int rightMaxflow = dumbMinCut(*rightUnfinished,rightCuts);
        
        int leftCutLeftCount=0;
        int leftCutRightCount=0;
        for (int x=0; x<leftCuts[0]->width(); x++)
        {
            for (int y=0; y<leftCuts[0]->height(); y++)
            {
                if (leftCuts[0]->pixel(x,y))
                    leftCutLeftCount++;
            }
        }
        for (int x=0; x<leftCuts[1]->width(); x++)
        {
            for (int y=0; y<leftCuts[1]->height(); y++)
            {
                if (leftCuts[1]->pixel(x,y))
                    leftCutRightCount++;
            }
        }
        int rightCutLeftCount=0;
        int rightCutRightCount=0;
        for (int x=0; x<rightCuts[0]->width(); x++)
        {
            for (int y=0; y<rightCuts[0]->height(); y++)
            {
                if (rightCuts[0]->pixel(x,y))
                    rightCutLeftCount++;
            }
        }
        for (int x=0; x<rightCuts[1]->width(); x++)
        {
            for (int y=0; y<rightCuts[1]->height(); y++)
            {
                if (rightCuts[1]->pixel(x,y))
                    rightCutRightCount++;
            }
        }
        
        //state check//
        
        char read;
        char dump;
        while(true)
        {
            printf("Recur%d:",test_count);
            scanf("%c%c",&read,&dump);
            if (read == 'l')
            {
                read='e';
                cont = false;
                break;
            }
            else if (read == ',')
            {
                read='l';
                recurLeft = true;
//                tempXOffset += cuts[0]->getXOffset();
//                tempYOffset += cuts[0]->getYOffset();
                accumulativeXOffset = tempLeftXOffset;
                accumulativeYOffset = tempLeftYOffset;
                break;
            }
            else if (read == '.')
            {
                read='r';
                recurLeft = false;
//                tempXOffset += cuts[1]->getXOffset();
//                tempYOffset += cuts[1]->getYOffset();
                accumulativeXOffset = tempRightXOffset;
                accumulativeYOffset = tempRightYOffset;
                break;
            }
        }
        test_count++;
        

        char buffer[150];
        sprintf(buffer,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%c",
                lastMaxflow,
                leftWidth,
                rightWidth,
                leftCount,
                rightCount,
                leftMaxflow,
                leftCuts[0]->width(),
                leftCuts[1]->width(),
                leftCutLeftCount,
                leftCutRightCount,
                rightMaxflow,
                rightCuts[0]->width(),
                rightCuts[1]->width(),
                rightCutLeftCount,
                rightCutRightCount,
                lastMaxflow-leftMaxflow,
                lastMaxflow-rightMaxflow,
                read);
        
        results << buffer << std::endl;

        
//        lastread=read;
        
        if (cont && recurLeft)
        {
            
            ret.insert(index,rightUnfinished);
            delete unfinished;
            delete rightCuts[0];
            delete rightCuts[1];
            
            unfinished = leftUnfinished;
            leftUnfinished=leftCuts[0];
            leftWidth = leftCuts[0]->width();
            leftCount = leftCutLeftCount;
            rightUnfinished=leftCuts[1];
            rightWidth = leftCuts[1]->width();
            rightCount = leftCutRightCount;
            lastMaxflow = leftMaxflow;
        }
        else if (cont)
        {
            ret.insert(index,leftUnfinished);
            index++;
            delete leftCuts[0];
            delete leftCuts[1];
            
            delete unfinished;
            unfinished = rightUnfinished;
            leftUnfinished=rightCuts[0];
            leftWidth = rightCuts[0]->width();
            leftCount = rightCutLeftCount;
            rightUnfinished=rightCuts[1];
            rightWidth = rightCuts[1]->width();
            rightCount = rightCutRightCount;
            lastMaxflow = rightMaxflow;
        }
        else
        {
//            ret.insert(index,unfinished);
            delete leftCuts[0];
            delete leftCuts[1];
            delete rightCuts[0];
            delete rightCuts[1];
//            delete leftUnfinished;
//            delete rightUnfinished;
            ret.insert(index++,leftUnfinished);
            ret.insert(index,rightUnfinished);
            delete unfinished;
            
        }
        
        
        
    }
    results.close();
    
//    if (recurLeft)
//    {
//        index++;
//    }
    
    BPartition* left = new BPartition(&img);
    for (int i=0; i<index; i++)
    {
        left->join(ret[i]);
        delete ret[i];
    }
    BPartition* right = new BPartition(&img);
    for (int i=index; i<ret.size(); i++)
    {
        right->join(ret[i]);
        delete ret[i];
    }
    
    ret.clear();
    ret.append(left);
    ret.append(right);
    
    return ret;
}
///ENDDUMB//////
