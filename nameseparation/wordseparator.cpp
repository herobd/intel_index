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
    int ANCHOR_WEIGHT = 270;
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
    
    
//    printf("first height=%d, h=(%d,%d)\n",firstPart->height(),w,h);
   
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
    if (crossPoints.empty())
        return;
    int SUBSECTION_WIDTH_FROM_KEYPOINT = 80;
    int SUBSECTION_HEIGHT_FROM_KEYPOINT = 10 + crossPoints[0].y();
    int SUBSECTION_PIXEL_COUNT_MAX = 750;
    int SOURCE_SINK_SEED_BUFFER = 5;
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
            QVector<QPoint> sourceStart;
            QVector<QPoint> sinkStart;
            
            while (!workingStack.isEmpty())
            {   
                QPoint cur = workingStack.front();
                workingStack.pop_front();
                
                int bottomX = cur.x()-bottom->getXOffset();
                int bottomY = cur.y()-bottom->getYOffset();
                if ((bottomX>=0 && bottomX<bottom->width() && bottomY>=0 && bottomY<bottom->height() && bottom->pixelIsMine(bottomX, bottomY)))
                    lowerPoints.append(cur);
                
                
                if (abs(cur.x()-keyPoint.x()) < SUBSECTION_WIDTH_FROM_KEYPOINT && abs(cur.y()-keyPoint.y()) < SUBSECTION_HEIGHT_FROM_KEYPOINT)
                {
                    
                    if (cur.y() <= keyPoint.y() && (subsectionTopPixelCount < SUBSECTION_PIXEL_COUNT_MAX /*|| abs(cur.x()-keyPoint.x())<10*/))
                    {
                        if (!crossover && top->pixelIsMineSrc(cur) != topStarted)
                        {
                            crossover=true;
                        }
                        
                        subsection.addPixelFromSrc(cur);
                        subsectionTopPixelCount++;
                    }
                    else if (cur.y() > keyPoint.y() && (subsectionBottomPixelCount < SUBSECTION_PIXEL_COUNT_MAX /*|| abs(cur.x()-keyPoint.x())<10*/))
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
                        if (cur.y() <= keyPoint.y() - SOURCE_SINK_SEED_BUFFER)
                            sourceStart.append(cur);
                        else if (cur.y() > keyPoint.y() + SOURCE_SINK_SEED_BUFFER)
                            sinkStart.append(cur);
                        continue;
                    }
                }
                else
                {
                    //
                    if (cur.y() <= keyPoint.y() - SOURCE_SINK_SEED_BUFFER)
                        sourceStart.append(cur);
                    else if (cur.y() > keyPoint.y() + SOURCE_SINK_SEED_BUFFER)
                        sinkStart.append(cur);
                    continue;
                }
                
//                if ((cur.x()-keyPoint.x()) == SUBSECTION_WIDTH_FROM_KEYPOINT-1 && )
                
                
                
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
                
//                printf("total score for point (%d,%d): %f\n",keyPoint.x(),keyPoint.y(),scoreTotal);
                double DESCENDER_SCORE_THRESH = 45;
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
                {//do something fancy, like a 3D cut
                    int NEW_SUBSECTION_WIDTH_FROM_KEYPOINT = 60;
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
                            continue;
                        
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
                    
                    QVector<BPartition*> result3DCut = cut3D(newSubsection, sourceSeeds, sinkSeeds);
                    
                    ///test///start
//                    xs.setNum(keyPoint.x());
//                    ys.setNum(keyPoint.y());
//                    loc = "./subsection/subsection";
//                    loc+=xs;
//                    loc+="_";
//                    loc+=ys;
//                    loc+=".ppm";
                    
//                    newSubsection.makeImage().save(loc);
                    ///test///end
                    
                    
                    
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
void WordSeparator::computeInverseDistanceMap(const BPixelCollection &src, int* out)
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
    double a = INV_A;
    int max_cc_size=500;
    
//    double normalizer = (b/m);
    BImage mark = src.makeImage();
    QVector<QPoint> workingStack;
    QVector<QPoint> growingComponent;
    
    
//    int newmax = 0;
//    int newmax2 = 0;
//    int newmin = INT_MAX;
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
                
//                if (out[index]>newmax)
//                    newmax=out[index];
                
//                if (out[index]>newmax2 && out[index]<newmax)
//                    newmax2=out[index];
                
//                if (out[index]<newmin)
//                    newmin=out[index];
            }
        }
        else if (!src.pixel(q%src.width(),q/src.width()))
        {
//            out[q] = pow(3,25-out[q]*normalizer)/pow(3,21);
//            out[q] = pow(b-std::min(out[q]*(b/m),b),e)/(pow(b,e)*(b/m));
            out[q] = pow(b-std::min(out[q]*(b/m),b),e)*a/(pow(b,e)) + 1;
        }

//        if (out[q]>newmax)
//            newmax=out[q];
//        if (out[q]>newmax2 && out[q]<newmax)
//            newmax2=out[q];
//        if (out[q]<newmin)
//            newmin=out[q];
    }
    
//    printf("newMax:%d, 2nd max:%d, newMin:%d\n",newmax,newmax2,newmin);
//    QImage debug(src.width(),src.height(),QImage::Format_Indexed8);
//    QVector<QRgb> default_color_table;
//    for (int i=0; i<255; i++)
//    {
//        default_color_table.append(qRgb(i,i,i));
//    }
//    for (int i=0; i<255; i++)
//    {
//        default_color_table.append(qRgb(255,255,255));
//    }
//    for (int i=0; i<255; i++)
//    {
//        default_color_table.append(qRgb(255,255,255));
//    }
//    debug.setColorTable(default_color_table);
//    for (int i=0; i<debug.width(); i++)
//    {
//        for (int j=0; j<debug.height(); j++)
//            debug.setPixel(i,j,(int)((out[i+j*debug.width()]/((double)newmax))*254));
        
//    }
//    debug.save("./inv_dist_map.ppm");
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
        
        //tree
        if ((leftWidth<=15 && rightWidth<=15) || (leftWidth<=2 || rightWidth<=2))//added for sanity
        {
            read='e';
            printf("Failsafe activated (twowords)\n");
        }
        else if (rightCount <= 853)
        {
            if(leftCount <= 866)
                read='e';
            else
                read='l';
        }
        else
        {
            if (leftWidth <= 90)
            {
                if (rightCutLeftCount <= 310)
                {
                    read='r';
                }
                else
                {
                    if (rightCuts[0]->width() <= 113)
                        read='e';
                    else
                        read='r';
                }
            }
            else
            {
                read='e';
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
    
    
    while (cont)
    {
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
        //tree New 15bin
        if (leftCutLeftCount <= 316)
        {
            read='e';
        }
        else
        {
            if (leftMaxflow <= 15754)
            {
                if (leftCount <= 1003)
                {
                    if (leftMaxflow <= 12613)
                    {
                        read='l';
                    }
                    else
                    {
                        if (leftCutLeftCount <= 523)
                            read='e';
                        else
                            read='l';
                    }
                }
                else
                {
                    read='l';
                }
            }
            else
            {
                if (leftCutLeftCount <= 605)
                {
                    read='e';
                }
                else
                {
                    if (leftCount <= 954)
                        read='e';
                    else
                        read='l';
                }
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
    return ret;
}

QVector<BPartition*> WordSeparator::recursiveHorizontalCutFirstLetterTraining(const BPixelCollection &img)
{
    std::ofstream results ("./firstletter_training_results.dat",std::ofstream::app);
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
    
    char lastread='%';
    
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
                results.close();
                return ret;
            }
        }
        test_count++;
        
        if (cont)
        {
            char buffer[150];
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
            
            results << buffer;
        }
        else
        {
//            char buffer[5];
//            sprintf(buffer,"%c",lastread);
            
            results << 'e' << std::endl;
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
    return ret;
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
    
    while (!stack.empty())
    {
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

    return ret;
}

QVector<BPartition*> WordSeparator::cut3D(const BPixelCollection &img, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds)
{
//    Dimension slopes(img.width(),img.height());
    
    int numOfBins = (img.width()+img.height())/2;
    AngleImage angleImage(&img,numOfBins,0.0,PI);
    QVector<QPoint> refPoints;
    QVector<double > refSlopesM;
    QVector<double > refLengthsM;
    
   //readfile 
    QVector<tracePoint> tracePoints;
    
    std::ofstream myfile ("matrix.txt");
    if (myfile.is_open())
    {
        myfile << img.width() << " ";
        myfile << img.height();
        myfile << "\n";
        for (int i =0; i<img.width(); i++)
        {
            for (int j = 0; j<img.height(); j++) {
                myfile << img.pixel(i,j) << " ";
            }
            myfile << "\n";
        }
        myfile.close();
    }
    else printf("Unable to open file\n");
//2.2
    system("java -Djava.io.tmpdir=/home/brian/tmp/ -jar ~/intel_index/nameseparation/ScottsCode/slopeGen.jar matrix slopedata 2.2 4");
    
    std::ifstream infile("slopedata.txt");
    std::string line;
    getline(infile, line);
    QRegExp rei("(\\d+)(?:[^\\d]+)(\\d+)(?:[^\\d]+)(\\d+)");
    QString qLine(line.c_str());
    rei.indexIn(qLine);
    tracePoint init;
    init.x=rei.cap(2).toInt();
    init.y=rei.cap(3).toInt();
    tracePoints.append(init);
    
    //(id)(x)(y)...
    QRegExp re("(\\d+)(?:[^\\d]+)(\\d+)(?:[^\\d]+)(\\d+)+((?:[^\\d]+)(\\d+))");
    while (getline(infile, line))
    {
        QString qLine(line.c_str());
        re.indexIn(qLine);
        tracePoint nextPoint;
        int index=re.cap(1).toInt()-1;
//        printf("read index %d\n",index);
        if (index >= tracePoints.size())
        {
            nextPoint.x=re.cap(2).toInt()-1;
            nextPoint.y=re.cap(3).toInt()-1;
            for (int i=4; i<re.captureCount(); i++)
            {
                int connectionId = re.cap(i).toInt()-1;
                double angle = atan2((nextPoint.y-tracePoints[connectionId].y),(nextPoint.x-tracePoints[connectionId].x));
    //            double angle = re.cap(6).toDouble();
                if (angle < 0)
                    angle += PI;
                double distance = sqrt(pow(nextPoint.x-tracePoints[connectionId].x,2) + pow(nextPoint.y-tracePoints[connectionId].y,2));
                nextPoint.connectedPoints.append(connectionId);
                nextPoint.angleBetween.append(angle);
                nextPoint.distanceBetween.append(distance);
                tracePoints.append(nextPoint);
                
                tracePoints[connectionId].connectedPoints.append(index);
                tracePoints[connectionId].angleBetween.append(angle);
                tracePoints[connectionId].distanceBetween.append(distance);
            }
            
        }
        else
        {
            printf("ERROR repeat index read in\n",index);
//            int last = re.cap(4).toInt();
//            double angle = re.cap(6).toDouble();
//            if (angle < 0)
//                angle += 180;
//            tracePoints[index].connectedPoints.append(last);
//            tracePoints[index].angleBetween.append(angle);
//            tracePoints[last].connectedPoints.append(index);
//            tracePoints[last].angleBetween.append(angle);
        }
    }
    
    QVector<bool> visited(tracePoints.size());
    for (int i=0; i<visited.size(); i++)
    {
        visited[i]=false;
    }
    QVector<int> pointStack;
    pointStack.append(0);
    while (!pointStack.empty())
    {
        int curIndex=pointStack.back();
        pointStack.pop_back();
        if (visited[curIndex])
            continue;
        visited[curIndex]=true;
        
//        QPoint toAdd(tracePoints[curIndex].x,tracePoints[curIndex].y);
        
//        QVector<double> slope;
        for (int i=0; i<tracePoints[curIndex].connectedPoints.size(); i++)
        {
//            slope.append(tracePoints[curIndex].angleBetween[i]);
            if (!visited[tracePoints[curIndex].connectedPoints[i]])
            {
                pointStack.append(tracePoints[curIndex].connectedPoints[i]);
                int midX = (tracePoints[curIndex].x + tracePoints[ tracePoints[curIndex].connectedPoints[i] ].x)/2;
                int midY = (tracePoints[curIndex].y + tracePoints[ tracePoints[curIndex].connectedPoints[i] ].y)/2;
                QPoint mid(midX,midY);
                if (!img.pixel(midX,midY))
                {
                    //find closest
                    mid = findClosestPointOn(img,mid);
                }
                
                if(mid.x()>2000 || mid.y()>2000)
                {
                    printf("Error on midpoint for (%d,%d) and (%d,%d)\n",tracePoints[curIndex].x,tracePoints[curIndex].y,tracePoints[ tracePoints[curIndex].connectedPoints[i] ].x,tracePoints[ tracePoints[curIndex].connectedPoints[i] ].y);
                }
                
                refPoints.append(mid);
                double slopeMid=tracePoints[curIndex].angleBetween[i];
//                slopeMid.append(tracePoints[curIndex].angleBetween[i]);
                refSlopesM.append(slopeMid);
                refLengthsM.append(tracePoints[curIndex].distanceBetween[i]);
            }
        }
//        refPoints.append(toAdd);
//        refSlopesM.append(slope);
        
        
    }
    
    QVector<QPoint> edgeStack;
    QVector<double> angleEdgeStack;
    QVector<double> distanceEdgeStack;
    
    double FILL_RADIOUS_CONST = 0.75;//I'd rather these be variable, dependant on the length of the slope line
    for (int i=0; i<refPoints.size(); i++)
    {
        QVector<QPoint> workingStack;
        QVector<double> distStack;
        workingStack.append(refPoints[i]);
        distStack.append(0);
        double angle = refSlopesM[i];
        angleImage.setPixelSlope(refPoints[i].x(),refPoints[i].y(),angle,1);
        
        double fillRadious = FILL_RADIOUS_CONST * refLengthsM[i];
        
        while(!workingStack.empty())
        {
            QPoint cur = workingStack.front();
            workingStack.pop_front();
            double curDist = distStack.front();
            distStack.pop_front();
            
            if (curDist > fillRadious)
            {
                edgeStack.append(cur);
                angleEdgeStack.append(angle);
                distanceEdgeStack.append(curDist);
            }
            else
            {
                double newDist = curDist+1;
                double str = (newDist+1)/(2*newDist);
                if (cur.x()<angleImage.width()-1 && angleImage.noStrongerAngleForPixel(cur.x()+1,cur.y(),angle,str))
                {
                    QPoint pp(cur.x()+1,cur.y());
                    workingStack.push_back(pp);
                    
                    distStack.push_back(newDist);
                    
                    angleImage.setPixelSlope(pp.x(),pp.y(),angle,str);
                }
                if (cur.y()<angleImage.height()-1 && angleImage.noStrongerAngleForPixel(cur.x(),cur.y()+1,angle,str))
                {
                    QPoint pp(cur.x(),cur.y()+1);
                    workingStack.push_back(pp);
                    double newDist = curDist+1;
                    distStack.push_back(newDist);
                    double str = (newDist+1)/(2*newDist);
                    angleImage.setPixelSlope(pp.x(),pp.y(),angle,str);
                }
                if (cur.x()>0 && angleImage.noStrongerAngleForPixel(cur.x()-1,cur.y(),angle,str))
                {
                    QPoint pp(cur.x()-1,cur.y());
                    workingStack.push_back(pp);
                    double newDist = curDist+1;
                    distStack.push_back(newDist);
                    double str = (newDist+1)/(2*newDist);
                    angleImage.setPixelSlope(pp.x(),pp.y(),angle,str);
                }
                if (cur.y()>0 && angleImage.noStrongerAngleForPixel(cur.x(),cur.y()-1,angle,str))
                {
                    QPoint pp(cur.x(),cur.y()-1);
                    workingStack.push_back(pp);
                    double newDist = curDist+1;
                    distStack.push_back(newDist);
                    double str = (newDist+1)/(2*newDist);
                    angleImage.setPixelSlope(pp.x(),pp.y(),angle,str);
                }
                //diagonals
                newDist = curDist+SQRT_2;
                str = (newDist+1)/(2*newDist);
                if (cur.x()<angleImage.width()-1 && cur.y()<angleImage.height()-1 && angleImage.noStrongerAngleForPixel(cur.x()+1,cur.y()+1,angle,str))
                {
                    QPoint pp(cur.x()+1,cur.y()+1);
                    workingStack.push_back(pp);
                    distStack.push_back(newDist);
                    angleImage.setPixelSlope(pp.x(),pp.y(),angle,str);
                }
                if (cur.y()<angleImage.height()-1 && cur.x()>0 && angleImage.noStrongerAngleForPixel(cur.x()-1,cur.y()+1,angle,str))
                {
                    QPoint pp(cur.x()-1,cur.y()+1);
                    workingStack.push_back(pp);
                    distStack.push_back(newDist);
                    angleImage.setPixelSlope(pp.x(),pp.y(),angle,str);
                }
                if (cur.x()<angleImage.width()-1 && cur.y()>0 && angleImage.noStrongerAngleForPixel(cur.x()+1,cur.y()-1,angle,str))
                {
                    QPoint pp(cur.x()+1,cur.y()-1);
                    workingStack.push_back(pp);
                    distStack.push_back(newDist);
                    angleImage.setPixelSlope(pp.x(),pp.y(),angle,str);
                }
                if (cur.y()>0 && cur.x()>0 && angleImage.noStrongerAngleForPixel(cur.x()-1,cur.y()-1,angle,str))
                {
                    QPoint pp(cur.x()-1,cur.y()-1);
                    workingStack.push_back(pp);
                    distStack.push_back(newDist);
                    angleImage.setPixelSlope(pp.x(),pp.y(),angle,str);
                }
            }
        }
    }
    
    
    //after fill
    while (!edgeStack.empty())
    {
        QPoint cur = edgeStack.front();
        edgeStack.pop_front();
        double curDist = distanceEdgeStack.front();
        distanceEdgeStack.pop_front();
        double angle = angleEdgeStack.front();
        angleEdgeStack.pop_front();
        

        if (cur.x()<angleImage.width()-1 && angleImage.noAnglesForPixel(cur.x()+1,cur.y()))
        {
            QPoint pp(cur.x()+1,cur.y());
            edgeStack.push_back(pp);
            double newDist = curDist+1;
            distanceEdgeStack.push_back(newDist);
            angleEdgeStack.push_back(angle);
            double str = (newDist+1)/(2*newDist);
            angleImage.setPixelSlope(pp.x(),pp.y(),angle,str);
        }
        if (cur.y()<angleImage.height()-1 && angleImage.noAnglesForPixel(cur.x(),cur.y()+1))
        {
            QPoint pp(cur.x(),cur.y()+1);
            edgeStack.push_back(pp);
            double newDist = curDist+1;
            distanceEdgeStack.push_back(newDist);
            angleEdgeStack.push_back(angle);
            double str = (newDist+1)/(2*newDist);
            angleImage.setPixelSlope(pp.x(),pp.y(),angle,str);
        }
        if (cur.x()>0 && angleImage.noAnglesForPixel(cur.x()-1,cur.y()))
        {
            QPoint pp(cur.x()-1,cur.y());
            edgeStack.push_back(pp);
            double newDist = curDist+1;
            distanceEdgeStack.push_back(newDist);
            angleEdgeStack.push_back(angle);
            double str = (newDist+1)/(2*newDist);
            angleImage.setPixelSlope(pp.x(),pp.y(),angle,str);
        }
        if (cur.y()>0 && angleImage.noAnglesForPixel(cur.x(),cur.y()-1))
        {
            QPoint pp(cur.x(),cur.y()-1);
            edgeStack.push_back(pp);
            double newDist = curDist+1;
            distanceEdgeStack.push_back(newDist);
            angleEdgeStack.push_back(angle);
            double str = (newDist+1)/(2*newDist);
            angleImage.setPixelSlope(pp.x(),pp.y(),angle,str);
        }
        //diagonals
        if (cur.x()<angleImage.width()-1 && cur.y()<angleImage.height()-1 && angleImage.noAnglesForPixel(cur.x()+1,cur.y()+1))
        {
            QPoint pp(cur.x()+1,cur.y()+1);
            edgeStack.push_back(pp);
            double newDist = curDist+SQRT_2;
            distanceEdgeStack.push_back(newDist);
            angleEdgeStack.push_back(angle);
            double str = (newDist+1)/(2*newDist);
            angleImage.setPixelSlope(pp.x(),pp.y(),angle,str);
        }
        if (cur.y()<angleImage.height()-1 && cur.x()>0 && angleImage.noAnglesForPixel(cur.x()-1,cur.y()+1))
        {
            QPoint pp(cur.x()-1,cur.y()+1);
            edgeStack.push_back(pp);
            double newDist = curDist+SQRT_2;
            distanceEdgeStack.push_back(newDist);
            angleEdgeStack.push_back(angle);
            double str = (newDist+1)/(2*newDist);
            angleImage.setPixelSlope(pp.x(),pp.y(),angle,str);
        }
        if (cur.x()<angleImage.width()-1 && cur.y()>0 && angleImage.noAnglesForPixel(cur.x()+1,cur.y()-1))
        {
            QPoint pp(cur.x()+1,cur.y()-1);
            edgeStack.push_back(pp);
            double newDist = curDist+SQRT_2;
            distanceEdgeStack.push_back(newDist);
            angleEdgeStack.push_back(angle);
            double str = (newDist+1)/(2*newDist);
            angleImage.setPixelSlope(pp.x(),pp.y(),angle,str);
        }
        if (cur.y()>0 && cur.x()>0 && angleImage.noAnglesForPixel(cur.x()-1,cur.y()-1))
        {
            QPoint pp(cur.x()-1,cur.y()-1);
            edgeStack.push_back(pp);
            double newDist = curDist+SQRT_2;
            distanceEdgeStack.push_back(newDist);
            angleEdgeStack.push_back(angle);
            double str = (newDist+1)/(2*newDist);
            angleImage.setPixelSlope(pp.x(),pp.y(),angle,str);
        }
        
    }
    
//    BImage mark(img);
//    QVector<QVector<QPoint> > stacks(refPoints.size());
//    for (int i=0; i<refPoints.size(); i++)
//    {
//        stacks[i].push_back(refPoints[i]);
//        mark.setPixel(refPoints[i],false);
//    }
//    bool cont = true;
//    while(cont)
//    {
//        cont = false;
//        for (int i=0; i<stacks.size(); i++)
//        {
//            if (stacks[i].empty())
//            {
//                continue;
//            }
//            cont = true;
//            QPoint cur = stacks[i].front();
//            stacks[i].pop_front();
            
            
////            slopes.setValueForPixel(cur,refSlopes[i]);
////            if (refSlopes2[i]>=0)
////                slopes.setSecondValueForPixel(cur,refSlopes2[i]);
            
//            slopes.setValuesForPixel(cur,refSlopesM[i]);
            
//            if (cur.x()<mark.width()-1 && mark.pixel(cur.x()+1,cur.y()))
//            {
//                QPoint pp(cur.x()+1,cur.y());
//                stacks[i].push_back(pp);
//                mark.setPixel(pp,false);
//            }
//            if (cur.y()<mark.height()-1 && mark.pixel(cur.x(),cur.y()+1))
//            {
//                QPoint pp(cur.x(),cur.y()+1);
//                stacks[i].push_back(pp);
//                mark.setPixel(pp,false);
//            }
//            if (cur.x()>0 && mark.pixel(cur.x()-1,cur.y()))
//            {
//                QPoint pp(cur.x()-1,cur.y());
//                stacks[i].push_back(pp);
//                mark.setPixel(pp,false);
//            }
//            if (cur.y()>0 && mark.pixel(cur.x(),cur.y()-1))
//            {
//                QPoint pp(cur.x(),cur.y()-1);
//                stacks[i].push_back(pp);
//                mark.setPixel(pp,false);
//            }
//        }
//    }

//    slopes.setNumOfBins(numOfBins);
//    slopes.setMinMax(0,PI);
    
//    NDimensions dimensions;
//    dimensions.addDimension(slopes);
    
    
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
    
    
    
    int num_pix = img.width()*img.height();
    //double pix_vals[num_pix];
    int invDistMap[num_pix];
    
    computeInverseDistanceMap(img,invDistMap);
    QVector<int> firstImgIndexes;
    QVector<int> secondImgIndexes;
    
//    int maxflow = GraphCut::pixelsOfSeparationWithSlope(invDistMap,img.width(),img.height(),img, slopes,firstImgIndexes,secondImgIndexes);
//    int maxflow = GraphCut::pixelsOfSeparationNDimensions(invDistMap,img.width(),img.height(),img, dimensions,sourceSeeds,sinkSeeds,firstImgIndexes,secondImgIndexes);
    int maxflow = GraphCut::pixelsOfSeparation(invDistMap,img.width(),img.height(),img, angleImage,sourceSeeds,sinkSeeds,firstImgIndexes,secondImgIndexes);
    
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
    
    computeInverseDistanceMap(img,invDistMap);
    QVector<int> firstImgIndexes;
    QVector<int> secondImgIndexes;
    
    int maxflow = GraphCut::pixelsOfSeparation(invDistMap,img.width(),img.height(),img,sourceSeeds,sinkSeeds,firstImgIndexes,secondImgIndexes);
    
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

QPoint WordSeparator::findClosestPointOn(const BPixelCollection &img, QPoint &start)
{
    QVector<QPoint> searchQueue;
    searchQueue.append(start);
    BImage mark = img.makeImage();
    mark.setPixel(start,false);
    while (!searchQueue.empty())
    {
        QPoint cur = searchQueue.front();
        searchQueue.pop_front();
        if (img.pixel(cur))
            return cur;
        
        QPoint up(cur.x(),cur.y()-1);
        QPoint down(cur.x(),cur.y()+1);
        QPoint left(cur.x()-1,cur.y());
        QPoint right(cur.x()+1,cur.y());
        QPoint lu(cur.x()-1,cur.y()-1);
        QPoint ld(cur.x()-1,cur.y()+1);
        QPoint ru(cur.x()+1,cur.y()-1);
        QPoint rd(cur.x()+1,cur.y()+1);
        if (cur.y()>0 && mark.pixel(up))
        {
            searchQueue.append(up);
            mark.setPixel(up,false);
        }
        if (cur.y()+1<mark.height() && mark.pixel(down))
        {
            searchQueue.append(down);
            mark.setPixel(down,false);
        }
        if (cur.x()>0 && mark.pixel(left))
        {
            searchQueue.append(left);
            mark.setPixel(left,false);
        }
        if (cur.x()+1<mark.width() && mark.pixel(right))
        {
            searchQueue.append(right);
            mark.setPixel(right,false);
        }
        if (cur.x()>0 && cur.y()>0 &&mark.pixel(lu))
        {
            searchQueue.append(lu);
            mark.setPixel(lu,false);
        }
        if (cur.x()>0 && cur.y()+1<mark.height() && mark.pixel(ld))
        {
            searchQueue.append(ld);
            mark.setPixel(ld,false);
        }
        if (cur.x()+1<mark.width() && cur.y()>0 && mark.pixel(ru))
        {
            searchQueue.append(ru);
            mark.setPixel(ru,false);
        }
        if (cur.x()+1<mark.width() && cur.y()+1<mark.height() && mark.pixel(rd))
        {
            searchQueue.append(rd);
            mark.setPixel(rd,false);
        }
        
    }
}
