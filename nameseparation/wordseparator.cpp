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
                }
                else /*if(false)//skipping*/
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

void WordSeparator::compute3DInverseDistanceMap(const bool* src, int* out, int width, int height, int depth)
{
    Indexer3D ind(width,height);
    int maxDist=0;
    int g[width*height*depth];
    
    
    
    //First pass
    for (int z=0; z<depth; z++)
    {
        for (int x=0; x<width; x++)
        {
            if (src[ind.getIndex(x,0,z)])
            {
                g[ind.getIndex(x,0,z)]=0;
            }
            else
            {
                g[ind.getIndex(x,0,z)]=INT_POS_INFINITY;//width*src.height();
            }
            
            for (int y=1; y<height; y++)
            {
                if (src[ind.getIndex(x,y,z)])
                {
                    g[ind.getIndex(x,y,z)]=0;
                }
                else
                {
                    if (g[ind.getIndex(x,y-1,z)] != INT_POS_INFINITY)
                        g[ind.getIndex(x,y,z)]=1+g[ind.getIndex(x,y-1,z)];
                    else
                        g[ind.getIndex(x,y,z)] = INT_POS_INFINITY;
                }
            }
            
            for (int y=height-2; y>=0; y--)
            {
                if (g[ind.getIndex(x,y+1,z)]<g[ind.getIndex(x,y,z)])
                {
                    if (g[ind.getIndex(x,y+1,z)] != INT_POS_INFINITY)
                        g[ind.getIndex(x,y,z)]=1+g[ind.getIndex(x,y+1,z)];
                    else
                        g[ind.getIndex(x,y,z)] = INT_POS_INFINITY;
                }
            }
        }
    }
    
    //second pass, compute dist map for each depth
    int g2[width*height*depth];
    
    for (int z=0; z<depth; z++)
    {
        int q;
        int s[width];
        int t[width];
        int w;
        for (int y=0; y<height; y++)
        {
            q=0;
            s[0]=0;
            t[0]=0;
            for (int u=1; u<width;u++)
            {
                while (q>=0 && f2D(t[q],s[q],y,z,ind,g) > f2D(t[q],u,y,z,ind,g))
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
                    w = SepPlusOne2D(s[q],u,y,z,ind,g);
                    if (w<width)
                    {
                        q++;
                        s[q]=u;
                        t[q]=w;
                    }
                }
            }
            
            for (int u=width-1; u>=0; u--)
            {
               g2[ind.getIndex(u,y,z)]= f2D(u,s[q],y,z,ind,g);
//                if (out[ind.getIndex(u,y,z)] > maxDist)
//                    maxDist = out[ind.getIndex(u,y,z)];
                if (u==t[q])
                    q--;
            }
        }
    }
    
    //third pass
    for (int x=0; x<width; x++)
    {
        int q;
        int s[depth];
        int t[depth];
        int w;
        for (int y=0; y<height; y++)
        {
            q=0;
            s[0]=0;
            t[0]=0;
            for (int u=1; u<depth;u++)
            {
                while (q>=0 && f3D(x,y,t[q],s[q],ind,g2) > f3D(x,y,t[q],u,ind,g2))
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
                    w = SepPlusOne3D(x,y,s[q],u,ind,g2);
                    if (w<depth)
                    {
                        q++;
                        s[q]=u;
                        t[q]=w;
                    }
                }
            }
            
            for (int u=depth-1; u>=0; u--)
            {
                out[ind.getIndex(x,y,u)]= f3D(x,y,u,s[q],ind,g2);
                if (out[ind.getIndex(x,y,u)] > maxDist)
                    maxDist = out[ind.getIndex(x,y,u)];
                if (u==t[q])
                    q--;
            }
        }
    }
    printf("maxdist:%d\n",maxDist);
    
    int newmax=0;
    double e = 15;
    double b = 30;
    double m = 444000;
    double a = INV_A;
    for (int i=0; i<width*height*depth; i++)
    {
        out[i] = pow(b-std::min(out[i]*(b/m),b),e)*a/(pow(b,e));
        if (out[i]>newmax)
            newmax=out[i];
    }
    
    QVector<QRgb> default_color_table;
    for (int i=0; i<255; i++)
    {
        default_color_table.append(qRgb(i,i,i));
    }
//    for (int i=0; i<255; i++)
//    {
//        default_color_table.append(qRgb(255,255,255));
//    }
//    for (int i=0; i<255; i++)
//    {
//        default_color_table.append(qRgb(255,255,255));
//    }
    for (int z=0; z<depth; z++)
    {
        QImage debug(width,height,QImage::Format_Indexed8);
        QImage debug2(width,height,QImage::Format_Indexed8);
        
        debug.setColorTable(default_color_table);
        debug2.setColorTable(default_color_table);
        for (int x=0; x<width; x++)
        {
            for (int y=0; y<debug.height(); y++)
            {
                debug.setPixel(x,y,(int)((out[ind.getIndex(x,y,z)]/((double)newmax))*254));
                if (src[ind.getIndex(x,y,z)])
                    debug2.setPixel(x,y,0);
                else
                    debug2.setPixel(x,y,254);
            }
            
        }
        QString debugfile = "./dist_3d/layer_";
        QString debugfile2 = "./output/layer_";
        QString num;
        num.setNum(z);
        debugfile.append(num);
        debugfile.append(".ppm");
        debugfile2.append(num);
        debugfile2.append(".ppm");
        debug.save(debugfile);
        debug2.save(debugfile2);
    }
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

int WordSeparator::f2D(int x, int i, int y, int z, Indexer3D &ind, int* g)
{
    if (g[ind.getIndex(i,y,z)]==INT_POS_INFINITY || x==INT_POS_INFINITY)
        return INT_POS_INFINITY;
    return pow((x-i),2) + pow(g[ind.getIndex(i,y,z)],2);
}

int WordSeparator::SepPlusOne2D(int i, int u, int y, int z, Indexer3D &ind, int* g)
{
    if (g[ind.getIndex(u,y,z)] == INT_POS_INFINITY)// && g[i+y*m] != INT_POS_INFINITY)
    {
        return INT_POS_INFINITY;
    }
    return 1 + ((u*u)-(i*i)+pow(g[ind.getIndex(u,y,z)],2)-pow(g[ind.getIndex(i,y,z)],2)) / (2*(u-i));
}

int WordSeparator::f3D(int x, int y, int z, int i, Indexer3D &ind, int* g)
{
    if (g[ind.getIndex(x,y,i)]==INT_POS_INFINITY || y==INT_POS_INFINITY)
        return INT_POS_INFINITY;
    return pow((z-i),2) + pow(g[ind.getIndex(x,y,i)],2);
}

int WordSeparator::SepPlusOne3D(int x, int y, int i, int u, Indexer3D &ind, int* g)
{
    if (g[ind.getIndex(x,y,u)] == INT_POS_INFINITY)// && g[i+y*m] != INT_POS_INFINITY)
    {
        return INT_POS_INFINITY;
    }
    return 1 + ((u*u)-(i*i)+pow(g[ind.getIndex(x,y,u)],2)-pow(g[ind.getIndex(x,y,i)],2)) / (2*(u-i));
}

///test///////////////

void WordSeparator::compute3DInverseDistanceMapTest(const bool* src, int* out, int width, int height, int depth)
{
    Indexer3D ind(width,height);
    int maxDist=0;
    int g[width*height*depth];
    
    
    
    //First pass
    for (int y=0; y<height; y++)
    {
        for (int x=0; x<width; x++)
        {
            if (src[ind.getIndex(x,y,0)])
            {
                g[ind.getIndex(x,y,0)]=0;
            }
            else
            {
                g[ind.getIndex(x,y,0)]=INT_POS_INFINITY;//width*src.height();
            }
            
            for (int z=1; z<depth; z++)
            {
                if (src[ind.getIndex(x,y,z)])
                {
                    g[ind.getIndex(x,y,z)]=0;
                }
                else
                {
                    if (g[ind.getIndex(x,y,z-1)] != INT_POS_INFINITY)
                        g[ind.getIndex(x,y,z)]=1+g[ind.getIndex(x,y,z-1)];
                    else
                        g[ind.getIndex(x,y,z-1)] = INT_POS_INFINITY;
                }
            }
            
            for (int z=depth-2; z>=0; z--)
            {
                if (g[ind.getIndex(x,y,z+1)]<g[ind.getIndex(x,y,z)])
                {
                    if (g[ind.getIndex(x,y+1,z)] != INT_POS_INFINITY)
                        g[ind.getIndex(x,y,z)]=1+g[ind.getIndex(x,y,z+1)];
                    else
                        g[ind.getIndex(x,y,z)] = INT_POS_INFINITY;
                }
            }
        }
    }
    
    //second pass, compute dist map for each depth
    int g2[width*height*depth];
    
    for (int y=0; y<height; y++)
    {
        int q;
        int s[width];
        int t[width];
        int w;
        for (int z=0; z<depth; z++)
        {
            q=0;
            s[0]=0;
            t[0]=0;
            for (int u=1; u<width;u++)
            {
                while (q>=0 && f2D(t[q],s[q],y,z,ind,g) > f2D(t[q],u,y,z,ind,g))
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
                    w = SepPlusOne2D(s[q],u,y,z,ind,g);
                    if (w<width)
                    {
                        q++;
                        s[q]=u;
                        t[q]=w;
                    }
                }
            }
            
            for (int u=width-1; u>=0; u--)
            {
               g2[ind.getIndex(u,y,z)]= f2D(u,s[q],y,z,ind,g);
//                if (out[ind.getIndex(u,y,z)] > maxDist)
//                    maxDist = out[ind.getIndex(u,y,z)];
                if (u==t[q])
                    q--;
            }
        }
    }
    
    //third pass
    for (int x=0; x<width; x++)
    {
        int q;
        int s[height];
        int t[height];
        int w;
        for (int z=0; z<depth; z++)
        {
            q=0;
            s[0]=0;
            t[0]=0;
            for (int u=1; u<height;u++)
            {
                while (q>=0 && f3DTest(x,t[q],s[q],z,ind,g2) > f3DTest(x,t[q],u,z,ind,g2))
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
                    w = SepPlusOne3DTest(x,s[q],u,z,ind,g2);
                    if (w<height)
                    {
                        q++;
                        s[q]=u;
                        t[q]=w;
                    }
                }
            }
            
            for (int u=height-1; u>=0; u--)
            {
                out[ind.getIndex(x,u,z)]= f3DTest(x,u,s[q],z,ind,g2);
                if (out[ind.getIndex(x,u,z)] > maxDist)
                    maxDist = out[ind.getIndex(x,u,z)];
                if (u==t[q])
                    q--;
            }
        }
    }
    printf("maxdist:%d\n",maxDist);
    
    int newmax=0;
    double e = 15;
    double b = 30;
    double m = 444000;
    double a = INV_A;
    for (int i=0; i<width*height*depth; i++)
    {
        out[i] = pow(b-std::min(out[i]*(b/m),b),e)*a/(pow(b,e));
        if (out[i]>newmax)
            newmax=out[i];
    }
    
    QVector<QRgb> default_color_table;
    for (int i=0; i<255; i++)
    {
        default_color_table.append(qRgb(i,i,i));
    }
//    for (int i=0; i<255; i++)
//    {
//        default_color_table.append(qRgb(255,255,255));
//    }
//    for (int i=0; i<255; i++)
//    {
//        default_color_table.append(qRgb(255,255,255));
//    }
    for (int z=0; z<depth; z++)
    {
        QImage debug(width,height,QImage::Format_Indexed8);
        QImage debug2(width,height,QImage::Format_Indexed8);
        
        debug.setColorTable(default_color_table);
        debug2.setColorTable(default_color_table);
        for (int x=0; x<width; x++)
        {
            for (int y=0; y<debug.height(); y++)
            {
                debug.setPixel(x,y,(int)((out[ind.getIndex(x,y,z)]/((double)newmax))*254));
                if (src[ind.getIndex(x,y,z)])
                    debug2.setPixel(x,y,0);
                else
                    debug2.setPixel(x,y,254);
            }
            
        }
        QString debugfile = "./dist_3d/layer_";
        QString debugfile2 = "./output/layer_";
        QString num;
        num.setNum(z);
        debugfile.append(num);
        debugfile.append(".ppm");
        debugfile2.append(num);
        debugfile2.append(".ppm");
        debug.save(debugfile);
        debug2.save(debugfile2);
    }
}

int WordSeparator::f3DTest(int x, int y, int i, int z, Indexer3D &ind, int* g)
{
    if (g[ind.getIndex(x,i,z)]==INT_POS_INFINITY || y==INT_POS_INFINITY)
        return INT_POS_INFINITY;
    return pow((y-i),2) + pow(g[ind.getIndex(x,i,z)],2);
}

int WordSeparator::SepPlusOne3DTest(int x, int i, int u, int z, Indexer3D &ind, int* g)
{
    if (g[ind.getIndex(x,u,z)] == INT_POS_INFINITY)// && g[i+y*m] != INT_POS_INFINITY)
    {
        return INT_POS_INFINITY;
    }
    return 1 + ((u*u)-(i*i)+pow(g[ind.getIndex(x,u,z)],2)-pow(g[ind.getIndex(x,i,z)],2)) / (2*(u-i));
}

///test//////////////

void WordSeparator::computeKDInverseDistanceMap(const bool* in, int* out, int k, const int* dim)
{
    IndexerKD ind(k,dim);
    int pass[k];
    int maxDist = ComputeEDT(in,out,k,dim,ind,k,pass);
    
    printf("maxdist:%d,    inf:%d\n",maxDist,INT_POS_INFINITY);
    int newmax=0;
//    int newmax=0;
//    double e = 15;
//    double b = 30;
//    double m = 1156;
//    double a = INV_A;
    int total=1;
    for (int i=0; i<k; i++)
    {
        total*=dim[i];
    }
    for (int i=0; i<total; i++)
    {
//        out[i] = pow(b-std::min(out[i]*(b/m),b),e)*a/(pow(b,e));
        if (out[i]>newmax)
            newmax=out[i];
    }
    printf("new maxdist:%d\n",newmax);
    
    QVector<QRgb> default_color_table;
    for (int i=0; i<255; i++)
    {
        default_color_table.append(qRgb(i,i,i));
    }
    default_color_table.append(qRgb(255,0,0));
    default_color_table.append(qRgb(0,255,0));
//    for (int z=0; z<dim[2]; z++)
    {
        QImage debug(dim[0],dim[1],QImage::Format_Indexed8);
        QImage debug2(dim[0],dim[1],QImage::Format_Indexed8);
        
        debug.setColorTable(default_color_table);
        debug2.setColorTable(default_color_table);
        for (int x=0; x<dim[0]; x++)
        {
            for (int y=0; y<dim[1]; y++)
            {
                int pass[3];
                pass[0]=x; pass[1]=y; //pass[2]=z;
                int v = (int)((out[ind.getIndex(pass)]/((double)newmax))*254);
                if (out[ind.getIndex(pass)]==INT_POS_INFINITY)
                {
                    v=255;
                }
                else if (v >255)
                {
//                    printf ("error:%d\n",out[ind.getIndex(pass)]);
                    v=255;
                }
                if (v <0)
                {
                    v=256;
                    printf ("error:%d\n",out[ind.getIndex(pass)]);
                }
                debug.setPixel(x,y,v);
                if (in[ind.getIndex(pass)])
                    debug2.setPixel(x,y,0);
                else
                    debug2.setPixel(x,y,254);
            }
            
        }
        QString debugfile = "./dist_3d/layer_";
        QString debugfile2 = "./output/layer_";
        QString num;
//        num.setNum(z);
        debugfile.append(num);
        debugfile.append(".ppm");
        debugfile2.append(num);
        debugfile2.append(".ppm");
        debug.save(debugfile);
        debug2.save(debugfile2);
    }
}
//<http://www.csd.uwo.ca/~olga/Courses/Fall2009/9840/Chosen/linearExactEucl.pdf>
int WordSeparator::ComputeEDT(const bool* I, int* D, int k, const int* n, const IndexerKD &ind, int d, int* i)
{
    int maxDist=0;
    if (d==1)
    {
        for (i[0]=0; i[0]<n[0]; i[0]++)
        {
            if (I[ind.getIndex(i)])
                D[ind.getIndex(i)]=0;
            else
                D[ind.getIndex(i)]=INT_POS_INFINITY;
        }
    }
    else
    {
        for (i[d-1]=0; i[d-1]<n[d-1]; i[d-1]++)
        {
            int pass[k];
            copyArray(i,pass,k);
            int temp = ComputeEDT(I,D,k,n,ind,d-1,pass);
            if (temp>maxDist)
                maxDist=temp;
        }
    }
    
    int temp = recursiveFor(I,D,k,n,ind,d,i,0);
    if (temp>maxDist)
        maxDist=temp;
    
    return maxDist;
}

int WordSeparator::recursiveFor(const bool* I, int* D, int k, const int* n, const IndexerKD &ind, int d, int* i, int level)
{
    int maxDist=0;
    for (i[level]=0; i[level]<n[level]; i[level]++)
    {
        if (level<d-2)
        {
            int temp = recursiveFor(I,D,k,n,ind,d,i,level+1);
            if (temp>maxDist)
                maxDist=temp;
        }
        else
        {
            int pass[k];
            copyArray(i,pass,k);
            int temp = VoronoiEDT(I,D,k,n,ind,d,pass);
            if (temp>maxDist)
                maxDist=temp;
        }
    }
    return maxDist;
}

int WordSeparator::VoronoiEDT(const bool* I, int* D, int k, const int* n, const IndexerKD &ind, int d, int* j)
{
    int maxDist=0;
    int l=0;
    int f[n[d-1]];
    int g[2*n[d-1]];
    int h[2*n[d-1]];
    int X[n[d-1]][k];
    
    for(int i=0; i<n[d-1]; i++)
    {
        copyArray(j,X[i],k);
        X[i][d-1]=i;
        f[i]=D[ind.getIndex(X[i])];
        if (f[i]!=INT_POS_INFINITY)
        {
            if (l<2)
            {
                l++;
                g[l]=f[i];
                h[l]=i+1;
            }
            else
            {
                while (l>=2 && RemoveEDT(g[l-1],g[l],f[i],h[l-1],h[l],i+1))
                {
                    l--;
                }
                l++;
                g[l]=f[i];
                h[l]=i+1;
            }
        }
    }
    
    int n_s=l;
    if (n_s==0)
        return maxDist;
    
    l=1;
    for (int i=1; i<=n[d-1]; i++)
    {
        while (l<n_s && g[l]+pow(h[l]-i,2) > g[l+1]+pow(h[l+1]-i,2))
            l++;
        D[ind.getIndex(X[i-1])]=g[l]+(int)pow(h[l]-i,2);
        if (D[ind.getIndex(X[i-1])]>maxDist)
            maxDist=D[ind.getIndex(X[i-1])];
    }
    return maxDist;
}

bool WordSeparator::RemoveEDT(int dis_sqr_u_Rd, int dis_sqr_v_Rd, int dis_sqr_w_Rd, int u_d, int v_d, int w_d)
{
    int a = v_d-u_d;
    int b = w_d-v_d;
    int c = w_d-u_d;
    return c*dis_sqr_v_Rd-b*dis_sqr_u_Rd-a*dis_sqr_w_Rd-a*b*c > 0;
}

void WordSeparator::copyArray(int* from, int* to, int c)
{
    for (int i=0; i<c; i++)
        to[i]=from[i];
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

inline int mod(int a, int b)
{
    while (a<0)
        a+=b;
    return a%b;
}

QVector<BPartition*> WordSeparator::cut3D(const BPixelCollection &img, QVector<QPoint> sourceSeeds, QVector<QPoint> sinkSeeds)
{
//    Dimension slopes(img.width(),img.height());
    
    int numOfBins = (img.width()+img.height())/2;
    AngleImage angleImage(&img,numOfBins,0.0,PI);
    
    
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
//    int maxflow = GraphCut::pixelsOfSeparation(invDistMap,img.width(),img.height(),img, angleImage,sourceSeeds,sinkSeeds,firstImgIndexes,secondImgIndexes);
    
    ///test 3ddist
    bool img3d[angleImage.width()*angleImage.height()*numOfBins];
    int dim[3];
    dim[0]=angleImage.width();
    dim[1]=angleImage.height();
    dim[2]=numOfBins;
    IndexerKD ind(2,dim);
    for (int x=0; x<angleImage.width(); x++)
    {
        for (int y=0; y<angleImage.height(); y++)
        {
            int pass[3];
            pass[0]=x; pass[1]=y;
            img3d[ind.getIndex(pass)]=angleImage.pixel(x,y);
//            for (int z=0; z<numOfBins; z++)
//            {
//                int pass[3];
//                pass[0]=x; pass[1]=y; pass[2]=z;
//                img3d[ind.getIndex(pass)]=false;
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
//                    }
//                }
//            }
        }
    }
    int distmap[angleImage.width()*angleImage.height()*numOfBins];
    computeKDInverseDistanceMap(img3d,distmap,2,dim);
//    compute3DInverseDistanceMapTest(img3d,distmap,angleImage.width(),angleImage.height(),numOfBins);
    ///test3ddist
    
    
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


