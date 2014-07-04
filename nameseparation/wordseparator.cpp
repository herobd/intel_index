#include "wordseparator.h"
 
#include <math.h>

#include <stdlib.h>





WordSeparator::WordSeparator()
{
}


//This performs a horizontal separation of the image by creating a distance map and then doing a graph cut on it.
int WordSeparator::minCut(BPixelCollection &toCut, QVector<BPartition*> &ret)
{
    int toCut_width = toCut.width();
    int toCut_height = toCut.height();
    int num_pix = toCut_width*toCut_height;
    int invDistMap[num_pix];
    
    computeInverseDistanceMap(toCut,invDistMap);
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

//This performs a verticle separation of two words, it identifys descenders in an attempt to increase accuracy
QVector<BPartition*> WordSeparator::horzCutEntries(BPixelCollection &img, int vert_divide)
{
    int num_pix = img.width()*img.height();
    //double pix_vals[num_pix];
    int invDistMap[num_pix];
    
    computeInverseDistanceMap(img,invDistMap);
    QVector<int> firstImgIndexes;
    QVector<int> secondImgIndexes;
    
    //best anchor weight:300 - 425
    int ANCHOR_WEIGHT = 350;
    int maxflow = GraphCut::pixelsOfSeparation(invDistMap,img.width(),img.height(),img,firstImgIndexes,secondImgIndexes,ANCHOR_WEIGHT,SPLIT_VERT,vert_divide);
    
    BPartition* firstPart = new BPartition(&img);
    BPartition* secondPart = new BPartition(&img);
    
    int img_width = img.width();
//    int img_height = img.height();
    int h=0;
    int w=0;
    foreach (int index, firstImgIndexes)
    {
        int x = index%img_width;
        int y = index/img_width;
        if (y>h)
        {
            h=y;
            w=x;
        }
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
    
    
    printf("first height=%d, h=(%d,%d)\n",firstPart->height(),w,h);
   
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
    
    computeInverseDistanceMap(img,invDistMap);
    QVector<int> firstImgIndexes;
    QVector<int> secondImgIndexes;
    
//    int maxflow = GraphCut::pixelsOfSeparationWithSlope(invDistMap,img.width(),img.height(),img, slopes,firstImgIndexes,secondImgIndexes);
    int maxflow = GraphCut::pixelsOfSeparationNDimensions(invDistMap,img.width(),img.height(),img, dimensions,sourceSeeds,sinkSeeds,firstImgIndexes,secondImgIndexes);
    
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
    int SUBSECTION_WIDTH_FROM_KEYPOINT = 75;
    int SUBSECTION_PIXEL_COUNT_MAX = 750;
    //Find cross-over contected components
    assert(top->getSrc() == bottom->getSrc());
    
    
    QVector<QPoint> workingStack;
    QVector<QPoint> ccccKeyPoints;
    QVector<QVector<QPoint> > ccccLowerPoints;
    
    //clear dvide line
//    int vert_divide = crossPoints[0].y();
    
    
//    mark.save("./mark.ppm");
    
    foreach (QPoint keyPoint, crossPoints)
    {
        BImage mark = (top->getSrc())->makeImage();
//        for (int x=0; x<top->getSrc()->width(); x++)
//        {
//            mark.setPixel(x,vert_divide,false);
//        }
//        if (mark.pixel(keyPoint))
        {
            workingStack.push_back(keyPoint);
            bool topStarted = top->pixelIsMineSrc(keyPoint);
            QVector<QPoint> lowerPoints;
            BPartition subsection(top->getSrc());
            bool crossover=false;
            int subsectionTopPixelCount=0;
            int subsectionBottomPixelCount=0;
            while (!workingStack.isEmpty())
            {   
                QPoint cur = workingStack.front();
                workingStack.pop_front();
                
                
                int bottomX = cur.x()-bottom->getXOffset();
                int bottomY = cur.y()-bottom->getYOffset();
                if ((bottomX>=0 && bottomX<bottom->width() && bottomY>=0 && bottomY<bottom->height() && bottom->pixelIsMine(bottomX, bottomY)))
                    lowerPoints.append(cur);
                
                if (abs(cur.x()-keyPoint.x()) < SUBSECTION_WIDTH_FROM_KEYPOINT)
                {
                    subsection.addPixelFromSrc(cur);
                    if (cur.y() <= keyPoint.y() && (subsectionTopPixelCount < SUBSECTION_PIXEL_COUNT_MAX || abs(cur.x()-keyPoint.x())<10))
                    {
                        subsectionTopPixelCount++;
                    }
                    else if (cur.y() > keyPoint.y() && (subsectionBottomPixelCount < SUBSECTION_PIXEL_COUNT_MAX || abs(cur.x()-keyPoint.x())<10))
                    {
                        subsectionBottomPixelCount++;
                    }
                }
                
//                if ((cur.x()-keyPoint.x()) == SUBSECTION_WIDTH_FROM_KEYPOINT-1 && )
                
                //this needs changed
                if (!crossover && top->pixelIsMineSrc(cur) != topStarted)
                {
                    crossover=true;
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
                if (cur.x()>0 && mark.pixel(cur.x()-1,cur.y()))
                {
                    QPoint pp(cur.x()-1,cur.y());
                    workingStack.push_back(pp);
                    mark.setPixel(pp,false);
                }
                if (cur.y()>0 && mark.pixel(cur.x(),cur.y()-1))
                {
                    QPoint pp(cur.x(),cur.y()-1);
                    workingStack.push_back(pp);
                    mark.setPixel(pp,false);
                }
                //diagonals
                if (cur.x()<mark.width()-1 && cur.y()<mark.height()-1 && mark.pixel(cur.x()+1,cur.y()+1))
                {
                    QPoint pp(cur.x()+1,cur.y()+1);
                    workingStack.push_back(pp);
                    mark.setPixel(pp,false);
                }
                if (cur.y()<mark.height()-1 && cur.x()>0 && mark.pixel(cur.x()-1,cur.y()+1))
                {
                    QPoint pp(cur.x()-1,cur.y()+1);
                    workingStack.push_back(pp);
                    mark.setPixel(pp,false);
                }
                if (cur.x()<mark.width()-1 && cur.y()>0 && mark.pixel(cur.x()+1,cur.y()-1))
                {
                    QPoint pp(cur.x()+1,cur.y()-1);
                    workingStack.push_back(pp);
                    mark.setPixel(pp,false);
                }
                if (cur.y()>0 && cur.x()>0 && mark.pixel(cur.x()-1,cur.y()-1))
                {
                    QPoint pp(cur.x()-1,cur.y()-1);
                    workingStack.push_back(pp);
                    mark.setPixel(pp,false);
                }
            }
            
            if (crossover)
            {
                ccccKeyPoints.append(keyPoint);
                ccccLowerPoints.append(lowerPoints);
                
                //test
                BPartition test(top->getSrc());
                foreach (QPoint p, lowerPoints)
                {
                    test.addPixelFromSrc(p.x(),p.y());
                }
                QString xs;
               QString ys;
               xs.setNum(keyPoint.x());
               ys.setNum(keyPoint.y());
               QString loc = "./lowerPoints/lowerPoints";
               loc+=xs;
               loc+="_";
               loc+=ys;
               loc+=".ppm";
                test.makeImage().save(loc);
                /////
                
                QVector<QVector<double> > scoreMap(descenderProbMap.size());
                QVector<double> temp(descenderProbMap[0].size());
                temp.fill(0);
                scoreMap.fill(temp);
                double scoreTotal=0;
                int num_points_outside=0;
                foreach (QPoint ccccP, lowerPoints)
                {
                    int x = ccccP.x()-keyPoint.x() + descenderProbMap.size()/2;
                    int y = ccccP.y()-keyPoint.y();
                    
                    if(x>=0 && x<descenderProbMap.size() && y>=0 && y<descenderProbMap[0].size())
                    {
                        scoreMap[x][y] = descenderProbMap[x][y]-.4;
                        scoreTotal+=scoreMap[x][y];
                    }
                    else
                    {
                        scoreTotal-=.4;
                        num_points_outside++;
                    }
                    
                    
                }
                
                printf("total score for point (%d,%d): %f\n",keyPoint.x(),keyPoint.y(),scoreTotal);
                double DESCENDER_SCORE_THRESH = 18;
                if (scoreTotal>DESCENDER_SCORE_THRESH)
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
                }
                else
                {
                
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
                        newSubsection.addPixelFromSrc(src_cur);
                        
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
                        if (cur.x()>0 && mark.pixel(cur.x()-1,cur.y()))
                        {
                            QPoint pp(cur.x()-1,cur.y());
                            workingStack.push_back(pp);
                            mark.setPixel(pp,false);
                        }
                        if (cur.y()>0 && mark.pixel(cur.x(),cur.y()-1))
                        {
                            QPoint pp(cur.x(),cur.y()-1);
                            workingStack.push_back(pp);
                            mark.setPixel(pp,false);
                        }
                        //diagonals
                        if (cur.x()<mark.width()-1 && cur.y()<mark.height()-1 && mark.pixel(cur.x()+1,cur.y()+1))
                        {
                            QPoint pp(cur.x()+1,cur.y()+1);
                            workingStack.push_back(pp);
                            mark.setPixel(pp,false);
                        }
                        if (cur.y()<mark.height()-1 && cur.x()>0 && mark.pixel(cur.x()-1,cur.y()+1))
                        {
                            QPoint pp(cur.x()-1,cur.y()+1);
                            workingStack.push_back(pp);
                            mark.setPixel(pp,false);
                        }
                        if (cur.x()<mark.width()-1 && cur.y()>0 && mark.pixel(cur.x()+1,cur.y()-1))
                        {
                            QPoint pp(cur.x()+1,cur.y()-1);
                            workingStack.push_back(pp);
                            mark.setPixel(pp,false);
                        }
                        if (cur.y()>0 && cur.x()>0 && mark.pixel(cur.x()-1,cur.y()-1))
                        {
                            QPoint pp(cur.x()-1,cur.y()-1);
                            workingStack.push_back(pp);
                            mark.setPixel(pp,false);
                        }
                    }
    //                {
    //                    int cutOffLeft = (keyPoint.x()-subsection.getXOffset())-SUBSECTION_WIDTH_FROM_KEYPOINT;
    //                    int cutOffRight = (subsection.width() - (keyPoint.x()-subsection.getXOffset()))-SUBSECTION_WIDTH_FROM_KEYPOINT;
                        
    //                    if (cutOffLeft < 0)
    //                        cutOffLeft = 0;
                        
    //                    if (cutOffRight < 0)
    //                        cutOffRight = 0;
                        
    //                    printf("trim left:%d, trim right:%d\n",cutOffLeft,cutOffRight);
    //                    subsection.trim(cutOffLeft,cutOffRight,0,0);
    //                }
                    
                    //find border points for anchoring 3D cut
                    QVector<QPoint> sourceStart;
                    QVector<QPoint> sinkStart;
                    //top
                    for (int x=0; x<newSubsection.width(); x++)
                    {
                        if (newSubsection.pixel(x,0) && newSubsection.getSrc()->pixel(x+newSubsection.getXOffset(),newSubsection.getYOffset() - 1))
                        {
                            QPoint toAdd(x,0);
                            sourceStart.append(toAdd);
                        }
                    }
                    //bottom
                    for (int x=0; x<newSubsection.width(); x++)
                    {
                        if (newSubsection.pixel(x,newSubsection.height()-1) && newSubsection.getSrc()->pixel(x+newSubsection.getXOffset(),newSubsection.getYOffset() + newSubsection.height()))
                        {
                            QPoint toAdd(x,newSubsection.height()-1);
                            sinkStart.append(toAdd);
                        }
                    }
                    //left
                    for (int y=0; y<newSubsection.height(); y++)
                    {
                        if (newSubsection.pixel(0,y) && newSubsection.getSrc()->pixel(newSubsection.getXOffset()-1,newSubsection.getYOffset() + y))
                        {
                            QPoint toAdd(0,y);
                            if (y+newSubsection.getYOffset() <= keyPoint.y())
                                sourceStart.append(toAdd);
                            else
                                sinkStart.append(toAdd);
                        }
                    }
                    //right
                    for (int y=0; y<newSubsection.height(); y++)
                    {
                        if (newSubsection.pixel(newSubsection.width()-1,y) && newSubsection.getSrc()->pixel(newSubsection.getXOffset() + newSubsection.width(),newSubsection.getYOffset() + y))
                        {
                            QPoint toAdd(newSubsection.width()-1,y);
                            if (y+newSubsection.getYOffset() <= keyPoint.y())
                                sourceStart.append(toAdd);
                            else
                                sinkStart.append(toAdd);
                        }
                    }
                    
                    xs.setNum(keyPoint.x());
                    ys.setNum(keyPoint.y());
                    loc = "./subsection/subsection";
                    loc+=xs;
                    loc+="_";
                    loc+=ys;
                    loc+=".ppm";
                    
                    newSubsection.makeImage().save(loc);
                    
                    //Do 3D cut.
                    
    //                printf("sub(%d,%d):[%d,%d]\n", keyPoint.x(), keyPoint.y(), subsection.width(),subsection.height());
                    
    //                QVector<BPartition*> newTopBottom = horzCutEntries(subsection,keyPoint.y());
    //                top->clear(&subsection);
    //                bottom->clear(&subsection);
    ////                top->add(newTopBottom[0]);
    ////                bottom->add(newTopBottom[1]);
    //                for (int x=0; x<newTopBottom[0]->width(); x++)
    //                {
    //                    for (int y=0; y<newTopBottom[0]->height(); y++)
    //                    {
    //                        if (newTopBottom[0]->pixelIsMine(x,y))
    //                            top->addPixelFromSrc(x+newTopBottom[0]->getXOffset()+subsection.getXOffset(),y+newTopBottom[0]->getYOffset()+subsection.getYOffset());
    //                    }
    //                }
                    
    //                for (int x=0; x<newTopBottom[1]->width(); x++)
    //                {
    //                    for (int y=0; y<newTopBottom[1]->height(); y++)
    //                    {
    //                        if (newTopBottom[1]->pixelIsMine(x,y))
    //                            bottom->addPixelFromSrc(x+newTopBottom[1]->getXOffset()+subsection.getXOffset(),y+newTopBottom[1]->getYOffset()+subsection.getYOffset());
    //                    }
    //                }
                    
    //                delete newTopBottom[0];
    //                delete newTopBottom[1];
                }
            }
            
        }
//        else
//        {
//            //TODO this case still needs handled
//        }
        
        
        
    }
}


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
    
//    maxFlow_firstCut = GraphCut::pixelsOfSeparation(invDistMap,img.width(),img.height(),img,firstImgBlackPixelIndexes);
    
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
    
//    maxFlow_secondCutL = GraphCut::pixelsOfSeparation(invDistMap,leftImg.width(),leftImg.height(),leftImg,firstImgBlackPixelIndexes);
    
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
    
//    maxFlow_secondCutR = GraphCut::pixelsOfSeparation(invDistMap,rightImg.width(),rightImg.height(),rightImg,firstImgBlackPixelIndexes);
    
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
        
//        int maxflow = GraphCut::pixelsOfSeparation(invDistMap,cuts[numOfCuts].width(),cuts[numOfCuts].height(),cuts[numOfCuts],firstImgBlackPixelIndexes);
        
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
void WordSeparator::computeInverseDistanceMap(BPixelCollection &src, int* out)
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
//    double normalizer = (25.0/maxDist);
    double e = 10;
    double b = 25;
    double m = 2000;
    double a = 100;
    int max_cc_size=500;
    
//    double normalizer = (b/m);
    BImage mark = src.makeImage();
    QVector<QPoint> workingStack;
    QVector<QPoint> growingComponent;
    
    
    int newmax = 0;
    int newmin = INT_MAX;
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
                //diagonals
                if (cur.x()>0 && cur.y()>0 && mark.pixel(cur.x()-1,cur.y()-1))
                {
                    QPoint pp(cur.x()-1,cur.y()-1);
                    workingStack.push_back(pp);
                    mark.setPixel(pp,false);
                }
                
                
                if (cur.x()<mark.width()-1 && cur.y()>0 && mark.pixel(cur.x()+1,cur.y()-1))
                {
                    QPoint pp(cur.x()+1,cur.y()-1);
                    workingStack.push_back(pp);
                    mark.setPixel(pp,false);
                    
                }
                if (cur.x()>0 && cur.y()<mark.height()-1 && mark.pixel(cur.x()-1,cur.y()+1))
                {
                    QPoint pp(cur.x()-1,cur.y()+1);
                    workingStack.push_back(pp);
                    mark.setPixel(pp,false);
                }
                if (cur.x()<mark.width()-1 && cur.y()>0 && mark.pixel(cur.x()+1,cur.y()-1))
                {
                    QPoint pp(cur.x()+1,cur.y()-1);
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
//                out[index] = pow(3,25-out[index]*normalizer)/pow(3,21) + 4*std::min(cc_size,500);
//                out[index] = pow(b-std::min(out[index]*(b/m),b),e)/(pow(b,e)*(b/m)) + a*std::min(cc_size,max_cc_size);
                out[index] = pow(b-std::min(out[index]*(b/m),b),e)*a/(pow(b,e)) + std::min(cc_size,max_cc_size) + 1;
                
                if (out[index]>newmax)
                    newmax=out[index];
                if (out[index]<newmin)
                    newmin=out[index];
            }
        }
        else if (!src.pixel(q%src.width(),q/src.width()))
        {
//            out[q] = pow(3,25-out[q]*normalizer)/pow(3,21);
//            out[q] = pow(b-std::min(out[q]*(b/m),b),e)/(pow(b,e)*(b/m));
            out[q] = pow(b-std::min(out[q]*(b/m),b),e)*a/(pow(b,e)) + 1;
        }

        if (out[q]>newmax)
            newmax=out[q];
        if (out[q]<newmin)
            newmin=out[q];
    }
    
    printf("newMax:%d, newMin:%d\n",newmax,newmin);
    QImage debug(src.width(),src.height(),QImage::Format_Indexed8);
    QVector<QRgb> default_color_table;
    for (int i=0; i<255; i++)
    {
        default_color_table.append(qRgb(i,i,i));
    }
//    for (int i=0; i<255; i++)
//    {
//        default_color_table.append(qRgb(0,i,255));
//    }
    for (int i=0; i<255; i++)
    {
//        default_color_table.append(qRgb(0,i,255-i));
        default_color_table.append(qRgb(255,255,255));
    }
//    for (int i=0; i<255; i++)
//    {
//        default_color_table.append(qRgb(i,255,0));
//    }
    for (int i=0; i<255; i++)
    {
//        default_color_table.append(qRgb(i,255-i,0));
        default_color_table.append(qRgb(255,255,255));
    }
    debug.setColorTable(default_color_table);
    for (int i=0; i<debug.width(); i++)
    {
        for (int j=0; j<debug.height(); j++)
            debug.setPixel(i,j,(int)((out[i+j*debug.width()]/((double)newmax))*254));
        
    }
    debug.save("./inv_dist_map.ppm");
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





BPartition* WordSeparator::chopOutTop(BPixelCollection &src)
{
    int num_pix = src.width()*src.height();
    //double pix_vals[num_pix];
    int invDistMap[num_pix];
    
    computeInverseDistanceMap(src,invDistMap);
    QVector<int> cutIndexes;
    QVector<int> unused;
    
    //best anchor weight:300 - 425
    int ANCHOR_WEIGHT = 1200;
    int maxflow = GraphCut::pixelsOfSeparation(invDistMap,src.width(),src.height(),src,cutIndexes,unused,ANCHOR_WEIGHT,CHOP_TOP);
    
    BPartition* ret = new BPartition(&src);
    
    
    foreach (int index, cutIndexes)
    {
        int x = index%src.width();
        int y = index/src.width();
        ret->addPixelFromSrc(x,y);
    }
    
    return ret;
}

QVector<BPartition*> WordSeparator::segmentLinesOfWords(const BPixelCollection &column, int spacingEstimate)
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
        int cutY = dividingLines[i] - accumulativeYOffset;
        QVector<BPartition*> cuts = horzCutEntries(*unfinishedCol,cutY);
        
//        cuts[0]->makeImage().save("./test0.ppm");
//        cuts[1]->makeImage().save("./test1.ppm");
//        lineremoved.claimOwnership(cuts[0],1);
//        lineremoved.saveOwners("./test.ppm");
//        printf("vert:%d, cuts[0].height=%d\n",dividingLines[i],cuts[0]->height());
        
        int tempXOffset = accumulativeXOffset + cuts[1]->getXOffset();
        int tempYOffset = accumulativeYOffset + cuts[1]->getYOffset();
        cuts[0]->changeSrc(&linesRemoved,accumulativeXOffset,accumulativeYOffset);
        cuts[1]->changeSrc(&linesRemoved,accumulativeXOffset,accumulativeYOffset);
        adjustHorzCutCrossOverAreas(cuts[0],cuts[1],crossPointsForLine[i],descenderProbMap);
        accumulativeXOffset = cuts[1]->getXOffset();
        accumulativeYOffset = cuts[1]->getYOffset();
        cuts[0]->changeSrc(&column,0,0);
//        cuts[1]->changeSrc(&column,tempXOffset,tempYOffset);
        
//        printf("cuts[0].height after change=%d\n",cuts[0]->height());
        //test//
        QString debugfile = "./output/cut_";
        QString num;
        num.setNum(test_count++);
        debugfile.append(num);
        debugfile.append(".ppm");;
        cuts[0]->makeImage().save(debugfile);
        //test//
        
        ret.append(cuts[0]);
        delete unfinishedCol;
        unfinishedCol = cuts[1];
        
        
//        cutEstimate = (unfinishedCol->height() - cut_dist_from_bottom) + spacingEstimate;
    }
    unfinishedCol->changeSrc(&column,0,0);
    ret.append(unfinishedCol);
    
    return ret;
}

QVector<BPartition*> WordSeparator::recursiveHorizontalCutWords(const BPixelCollection &img)
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
    int insertIndex=0;
    
    QVector<char> cutStates;
    QVector<int> maxFlows;
    QVector<int> imgWidthsLeft;
    QVector<int> imgWidthsRight;
    QVector<int> pixCountsLeft;
    QVector<int> pixCountsRight;
    
    bool recurLeft;
    
    while (cont)
    {
        QVector<BPartition*> cuts;
        int maxflow = minCut(*unfinished,cuts);
        maxFlows.append(maxflow);
        
//        BImage test = unfinished->makeImage();
        cuts[0]->makeImage().save("./test0.ppm");
        cuts[1]->makeImage().save("./test1.ppm");
//        test.claimOwnership(cuts[0],1);
//        test.saveOwners("./test.ppm");
        
        imgWidthsLeft.append(cuts[0]->width());
        imgWidthsRight.append(cuts[1]->width());
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
        pixCountsLeft.append(leftCount);
        pixCountsRight.append(rightCount);
        
        int tempXOffset = accumulativeXOffset;
        int tempYOffset = accumulativeYOffset;
        
        //state check//
        
        char read;
        while(true)
        {
            printf("Recur%d:",test_count);
            scanf("%c",&read);
            if (read == 'l')
            {
                cutStates.append('e');
                cont = false;
                break;
            }
            else if (read == ',')
            {
                cutStates.append('l');
                recurLeft = true;
                tempXOffset += cuts[0]->getXOffset();
                tempYOffset += cuts[0]->getYOffset();
                break;
            }
            else if (read == '.')
            {
                cutStates.append('r');
                recurLeft = false;
                tempXOffset += cuts[1]->getXOffset();
                tempYOffset += cuts[1]->getYOffset();
                break;
            }
        }
        
        
        
        
        
        cuts[0]->changeSrc(&img,accumulativeXOffset,accumulativeYOffset);
        cuts[1]->changeSrc(&img,accumulativeXOffset,accumulativeYOffset);
        
        
        accumulativeXOffset = tempXOffset;
        accumulativeYOffset = tempYOffset;
        
        
        if (cont && recurLeft)
        {
            
            ret.insert(insertIndex,cuts[1]);
            delete unfinished;
            unfinished = cuts[0];
        }
        else if (cont)
        {
            ret.insert(insertIndex,cuts[0]);
            delete unfinished;
            unfinished = cuts[1];
            insertIndex++;
            
        }
        else//this is based on an overshoot approach
        {
            delete cuts[0];
            delete cuts[1];
            ret.insert(insertIndex,unfinished);
        }
        
        
    }
    
    if (recurLeft)
    {
        insertIndex++;
    }
    
    BPartition* left = new BPartition(&img);
    for (int i=0; i<insertIndex; i++)
    {
        left->join(ret[i]);
        delete ret[i];
    }
    BPartition* right = new BPartition(&img);
    for (int i=insertIndex; i<ret.size(); i++)
    {
        right->join(ret[i]);
        delete ret[i];
    }
    
    ret.clear();
    ret.append(left);
    ret.append(right);
    
    int magic = 10;
    QString p;
    for (int i=0; i<magic; i++)
    {
        if (cutStates.size()>i)
        {
            p += QString(cutStates[i]);
            p += QString(",");
            p += QString::number(maxFlows[i]);
            p += QString(",");
            p += QString::number(imgWidthsLeft[i]);
            p += QString(",");
            p += QString::number(imgWidthsRight[i]);
            p += QString(",");
            p += QString::number(pixCountsLeft[i]);
            p += QString(",");
            p+= QString::number(pixCountsRight[i]);
            p += QString(",");
        }
        else
        {
            p += QString(cutStates.last());
            p += QString(",");
            p += QString::number(maxFlows.last());
            p += QString(",");
            p += QString::number(imgWidthsLeft.last());
            p += QString(",");
            p += QString::number(imgWidthsRight.last());
            p += QString(",");
            p += QString::number(pixCountsLeft.last());
            p += QString(",");
            p+= QString::number(pixCountsRight.last());
            p += QString(",");
            
        }
    }
    p[p.size()-1]='\n';
    printf("%s",qPrintable(p) );
    return ret;
}


//QVector<BPartition*> WordSeparator::recursiveHorizontalCutLetters(const BPixelCollection &img)
//{
//    QVector<BPartition*> ret;
////    BImage base(column);
////    base.save("./starting_image.ppm");
//    BPartition* unfinished = new BPartition(&img);
//    for (int x=0; x<img.width(); x++)
//    {
//        for (int y=0; y<img.height(); y++)
//        {
//            unfinished->addPixelFromSrc(x,y);
//        }
//    }

//    int test_count=0;
    
//    int accumulativeXOffset=0;
//    int accumulativeYOffset=0;
    
//    bool cont = true;
//    int insertIndex=0;
    
//    QVector<char> cutStates;
//    QVector<int> maxFlows;
//    QVector<int> imgWidthsLeft;
//    QVector<int> imgWidthsRight;
//    QVector<int> pixCountsLeft;
//    QVector<int> pixCountsRight;
    
//    while (cont)
//    {
//        QVector<BPartition*> cuts;
//        int maxflow = minCut(*unfinished,cuts);
//        maxFlows.append(maxflow);
        
////        BImage test = unfinished->makeImage();
//        cuts[0]->makeImage().save("./test0.ppm");
//        cuts[1]->makeImage().save("./test1.ppm");
////        test.claimOwnership(cuts[0],1);
////        test.saveOwners("./test.ppm");
        
//        imgWidthsLeft.append(cuts[0]->width());
//        imgWidthsRight.append(cuts[1]->width());
//        int leftCount=0;
//        int rightCount=0;
//        for (int x=0; x<cuts[0]->width(); x++)
//        {
//            for (int y=0; y<cuts[0]->height(); y++)
//            {
//                if (cuts[0]->pixel(x,y))
//                    leftCount++;
//            }
//        }
//        for (int x=0; x<cuts[1]->width(); x++)
//        {
//            for (int y=0; y<cuts[1]->height(); y++)
//            {
//                if (cuts[1]->pixel(x,y))
//                    rightCount++;
//            }
//        }
//        pixCountsLeft.append(leftCount);
//        pixCountsRight.append(rightCount);
        
//        int tempXOffset = accumulativeXOffset;
//        int tempYOffset = accumulativeYOffset;
        
//        //state check//
//        bool recurLeft;
//        char read;
//        while(true)
//        {
//            printf("Recur%d:",test_count);
//            scanf("%c",&read);
//            if (read == 'l')
//            {
//                cutStates.append('e');
//                cont = false;
//                break;
//            }
//            else if (read == ',')
//            {
//                cutStates.append('l');
//                recurLeft = true;
//                tempXOffset += cuts[0]->getXOffset();
//                tempYOffset += cuts[0]->getYOffset();
//                break;
//            }
//            else if (read == '.')
//            {
//                cutStates.append('r');
//                recurLeft = false;
//                tempXOffset += cuts[1]->getXOffset();
//                tempYOffset += cuts[1]->getYOffset();
//                break;
//            }
//        }
        
        
        
        
        
//        cuts[0]->changeSrc(&img,accumulativeXOffset,accumulativeYOffset);
//        cuts[1]->changeSrc(&img,accumulativeXOffset,accumulativeYOffset);
        
        
//        accumulativeXOffset = tempXOffset;
//        accumulativeYOffset = tempYOffset;
        
        
//        if (cont && recurLeft)
//        {
            
//            ret.insert(insertIndex,cuts[1]);
//            delete unfinished;
//            unfinished = cuts[0];
//        }
//        else if (cont)
//        {
//            ret.insert(insertIndex,cuts[0]);
//            delete unfinished;
//            unfinished = cuts[1];
//            insertIndex++;
            
//        }
//        else//this is based on an overshoot approach
//        {
//            delete cuts[0];
//            delete cuts[1];
//            ret.insert(insertIndex,unfinished);
//        }
        
        
        
//    }
    
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
//    return ret;
//}
