#include <iostream>
#include "wordprofile.h"
#include <QImage>
#include "wordseparator.h"
#include "boxcleaner.h"
#include "imageaverager.h"
#include "BPixelCollection.h"
//#include "bimage.h"
#include <math.h>
#include "evaluate.h"
#include <QSet>
#include <QColor>


#include "gimage.h"
#include "gpartition.h"

using namespace std;

#define ECCENTRICITY_LIMIT 1.4
#define MIN_REGION_SIZE 6

struct tracePoint
{
    int x;
    int y;
    QVector<int> connectedPoints;
    QVector<double> angleBetween;
    QVector<double> distanceBetween;
    tracePoint(int xx, int yy) {x=xx; y=yy;}
    tracePoint() {x=-1; y=-1;}
};

void testBolbFill(BImage &org, const QPoint &crossOverPoint)
{
//    QVector<QPoint> centersOfMass;
    QVector<QVector<QPoint> > regions;
    QVector<tracePoint> centersOfMass;
    int assignments[org.width()][org.height()];
    for (int x=0; x<org.width(); x++)
        for (int y=0; y<org.height(); y++)
            assignments[x][y]=-1;
    
    BImage img = org.makeImage();
    BImage mark = org.makeImage();
    QVector<QPoint> startPoints;
    startPoints.push_back(crossOverPoint);
    
    while(!startPoints.empty())
    {
        QPoint startPoint = startPoints.front();
        startPoints.pop_front();
        
        if (!mark.pixel(startPoint))
            continue;
        mark.setPixel(startPoint,false);
//        printf("Using start point: (%d,%d)\n",startPoint.x(),startPoint.y());
        
        QVector<QPoint> border;
        QVector<QPoint> collection;
        border.push_back(startPoint);
        
        int furthestDistSqrd=0;
        int killTokenLoc=-1;
        int sumX=0;
        int sumY=0;
        QSet<int> neighborRegions;
        int myRegionId = regions.size();
        
        while(!border.empty())
        {
            QPoint toAdd = border.front();
            border.pop_front();
            
            if (toAdd.x()==-1)//hit kill token
                break;
            
            int myFurthestDistSqrd=0;
            foreach (QPoint p, collection)
            {
                int distSqrd=pow(p.x()-toAdd.x(),2) + pow(p.y()-toAdd.y(),2);
                if (distSqrd>myFurthestDistSqrd)
                    myFurthestDistSqrd=distSqrd;
            }
            
            
            if (collection.size()==0 || max(myFurthestDistSqrd,furthestDistSqrd)/(1.0*collection.size()) <= ECCENTRICITY_LIMIT)
            {
                if (killTokenLoc>=0)
                {
                    border.remove(--killTokenLoc);//remove killToken
                    killTokenLoc=-1;
                }
                
                if (myFurthestDistSqrd>furthestDistSqrd)
                    furthestDistSqrd=myFurthestDistSqrd;
                
                collection.push_back(toAdd);
                assignments[toAdd.x()][toAdd.y()] = myRegionId;
                sumX+=toAdd.x();
                sumY+=toAdd.y();
                
                // 0 1 2 neighbor id table
                // 3 4 5
                // 6 7 8
                int tableIndex=8;
                for (int cc=0; cc<9; cc++)
                {
                    tableIndex=(tableIndex+2)%9;
                    if (tableIndex==4)
                        continue;
                    
                    int xDelta=(tableIndex%3)-1;
                    int yDelta=(tableIndex/3)-1;
                    int x = toAdd.x()+xDelta;
                    int y = toAdd.y()+yDelta;
                    if (x>=0 && x<mark.width() && y>=0 && y<mark.height())
                    {
                        if (mark.pixel(x,y))
                        {
                            QPoint p(x,y);
                            border.append(p);
                            mark.setPixel(p,false);
                        }
                        else if (org.pixel(x,y) && myRegionId != assignments[x][y] && -1 != assignments[x][y] && regions[assignments[x][y]].size()>=MIN_REGION_SIZE)
                        {
                            neighborRegions.insert(assignments[x][y]);
                        }
                    }
                }
              
                
            }
            else if (killTokenLoc<0)
            {
                killTokenLoc=border.size();
                QPoint killToken(-1,-1);
                border.push_back(killToken);
                border.push_back(toAdd);
            }
            else
            {
                killTokenLoc--;
                border.push_back(toAdd);
            }
            
        }
        
        tracePoint centerOfMass(sumX/collection.size(),sumY/collection.size());
        
        regions.append(collection);
//        printf("region %d found %d neighbors\n",myRegionId,neighborRegions.size());
        foreach (int regionId, neighborRegions)
        {
            double angle = atan2((centerOfMass.y-centersOfMass[regionId].y),(centerOfMass.x-centersOfMass[regionId].x));
            if (angle < 0)
                angle += PI;
            double distance = sqrt(pow(centerOfMass.x-centersOfMass[regionId].x,2) + pow(centerOfMass.y-centersOfMass[regionId].y,2));
            centerOfMass.connectedPoints.append(regionId);
            centerOfMass.angleBetween.append(angle);
            centerOfMass.distanceBetween.append(distance);
            
//            centersOfMass[regionId].connectedPoints.append(myRegionId);
//            centersOfMass[regionId].angleBetween.append(angle);
//            centersOfMass[regionId].distanceBetween.append(distance);
        }
        centersOfMass.append(centerOfMass);
        
//        printf("Center of mass found: (%d, %d)\n",centerOfMass.x,centerOfMass.y);
        
        foreach (QPoint notAdded, border)//reset points not added
        {
            if (mark.pixel(notAdded))
            {
//                printf("border (%d,%d) slipped\n",notAdded.x(),notAdded.y());
                continue;
            }
            
            QVector<QPoint> neighbors;
            neighbors.push_back(notAdded);
            mark.setPixel(notAdded,true);
            int sumX=0;
            int sumY=0;
            int count=0;
            
            while(!neighbors.empty())
            {
                QPoint cur = neighbors.front();
                neighbors.pop_front();
                sumX+=cur.x();
                sumY+=cur.y();
                count++;
                
                QPoint up(cur.x(),cur.y()-1);
                QPoint down(cur.x(),cur.y()+1);
                QPoint left(cur.x()-1,cur.y());
                QPoint right(cur.x()+1,cur.y());
                QPoint lu(cur.x()-1,cur.y()-1);
                QPoint ld(cur.x()-1,cur.y()+1);
                QPoint ru(cur.x()+1,cur.y()-1);
                QPoint rd(cur.x()+1,cur.y()+1);
                if (cur.y()>0 && !mark.pixel(up) && border.contains(up))
                {
                    neighbors.append(up);
                    mark.setPixel(up,true);
                }
                if (cur.y()+1<mark.height() && !mark.pixel(down) && border.contains(down))
                {
                    neighbors.append(down);
                    mark.setPixel(down,true);
                }
                if (cur.x()>0 && !mark.pixel(left)  && border.contains(down))
                {
                    neighbors.append(left);
                    mark.setPixel(left,true);
                }
                if (cur.x()+1<mark.width() && !mark.pixel(right) && border.contains(right))
                {
                    neighbors.append(right);
                    mark.setPixel(right,true);
                }
                if (cur.x()>0 && cur.y()>0 && !mark.pixel(lu) && border.contains(lu))
                {
                    neighbors.append(lu);
                    mark.setPixel(lu,true);
                }
                if (cur.x()>0 && cur.y()+1<mark.height() && !mark.pixel(ld) && border.contains(ld))
                {
                    neighbors.append(ld);
                    mark.setPixel(ld,true);
                }
                if (cur.x()+1<mark.width() && cur.y()>0 && !mark.pixel(ru) && border.contains(ru))
                {
                    neighbors.append(ru);
                    mark.setPixel(ru,true);
                }
                if (cur.x()+1<mark.width() && cur.y()+1<mark.height() && !mark.pixel(rd) && border.contains(rd))
                {
                    neighbors.append(rd);
                    mark.setPixel(rd,true);
                }
            }
            
            QPoint centerOfMassBorderConnectedComponent(sumX/count, sumY/count);
            if (border.contains(centerOfMassBorderConnectedComponent))
            {
                startPoints.push_back(centerOfMassBorderConnectedComponent);
            }
            else
            {
                int shortestDistance=INT_POS_INFINITY;
                QPoint closestPoint;
                foreach (QPoint p, border)
                {
                    int dist = pow(centerOfMassBorderConnectedComponent.x()-p.x(),2) + pow(centerOfMassBorderConnectedComponent.y()-p.y(),2);
                    if (dist<shortestDistance)
                    {
                        shortestDistance=dist;
                        closestPoint=p;
                    }
                    
                }
                startPoints.push_back(closestPoint);
            }
            
            
//            mark.setPixel(notAdded,true);
            
//            //this is a dumb way, just testing to see if it works
//            startPoints.push_back(notAdded);
        }
        border.clear();
        
    }
    
    
    
    //coloring
    QImage lines = org.getImage();
    QVector<QRgb> colorTable=lines.colorTable();
    int NUM_VALS = 245;
    int ctOffset=colorTable.size();
    for (int i=0; i<NUM_VALS; i++)
    {
        QColor color;
        color.setHsv(i,255,255);
        colorTable.append(color.rgb());
    }
    colorTable.append(qRgb(155,155,155));
    
    lines.setColorTable(colorTable);
    
    QVector<BPartition*> parts;
    for (int i=0; i<centersOfMass.size(); i++)
    {
        QVector<QPoint> region = regions[i];
        
        if (region.size()<MIN_REGION_SIZE)
        {
            
//            foreach(QPoint p, region)
//            {
//                img.setPixel(p,false);
//            }
            continue;
        }
        
        img.setPixel(centersOfMass[i].x,centersOfMass[i].y,false);
        BPartition* newPart = new BPartition(&img);
        foreach(QPoint p, region)
        {
            newPart->addPixelFromSrc(p);
        }
        img.claimOwnership(newPart,1);
        
//        if (lines.pixel(centersOfMass[i].x,centersOfMass[i].y)!=blue)
        {
            
            for (int j=0; j<centersOfMass[i].connectedPoints.size(); j++)
            {
                int index = centersOfMass[i].connectedPoints[j];
                
                double angle = centersOfMass[i].angleBetween[j];
//                printf("Hue used: %d\n",(int)(360*(angle/PI)));
                
                //draw line
                
                if (centersOfMass[i].x != centersOfMass[index].x)
                {
                    double shiftAngle = angle;
                    if (shiftAngle>HALF_PI)
                        shiftAngle-=PI;
                    double slope = tan(shiftAngle);
//                    printf("line of slope %f\n",slope);
                    int start = std::min(centersOfMass[i].x,centersOfMass[index].x);
                    int end = std::max(centersOfMass[i].x,centersOfMass[index].x);
                    double y;
                    if (start==centersOfMass[i].x)
                        y= centersOfMass[i].y;
                    else
                        y= centersOfMass[index].y;
                    for (int x=start; x<=end; x++)
                    {
                        lines.setPixel(x,(int)y,(int)(NUM_VALS*(angle/PI)+ctOffset));
                        for (int yDelta=1; yDelta<(int)ceil(slope); yDelta++)
                            lines.setPixel(x,(int)y + yDelta,(int)(NUM_VALS*(angle/PI)+ctOffset));
                        y+=slope;
                    }
                    lines.setPixel(centersOfMass[index].x,centersOfMass[index].y,NUM_VALS+ctOffset);
                }
                else
                {
                    int start = std::min(centersOfMass[i].y,centersOfMass[index].y);
                    int end = std::max(centersOfMass[i].y,centersOfMass[index].y);
                    int x =centersOfMass[i].x;
                    for (int y=start; y<=end; y++)
                    {
                        lines.setPixel(x,y,360/2);
                    }
                }
            }
            lines.setPixel(centersOfMass[i].x,centersOfMass[i].y,NUM_VALS+ctOffset);
        }
        
    }
    img.saveOwners("./blob_test.ppm");
    lines.save("./lines.ppm");
    
    foreach(BPartition* d,parts)
    {
        delete d;
    }
}


int main(int argc, char** argv)
{
//    cout << "Starting..." << endl;
    QImage testimg(argv[1]);

    
    
    BImage bimg(testimg);
    
//    QVector<QPoint> sourceSeeds;
//    QVector<QPoint> sinkSeeds;
    
////    QPoint p(4,29);
////    QPoint p2(6,64);
    
//    QPoint p(63,18);
//    QPoint p2(64,64);
//    QPoint p2x(1,72);
    
////    QPoint p(13,15);
////    QPoint px(64,69);
//    sourceSeeds.append(p);
////    sourceSeeds.append(px);
////    QPoint p2(15,73);
////    QPoint p2x(71,17);
////    QPoint p2xx(12,40);
//    sinkSeeds.append(p2);
//    sinkSeeds.append(p2x);
////    sinkSeeds.append(p2xx);
    
//    QVector<BPartition*> result = WordSeparator::cut3D(bimg, sourceSeeds, sinkSeeds);
//    result[0]->makeImage().save("./test0.ppm");
//    result[1]->makeImage().save("./test1.ppm");
//    bimg.claimOwnership(result[0],1);
//    bimg.claimOwnership(result[1],1);
//    bimg.saveOwners("./test.ppm");
    
    //////////////////////////
    
//    BImage cleared = BoxCleaner::trimVerticleBoundaries(bimg);
//    cleared = BoxCleaner::removeVerticlePixelNoise(cleared);
    
//    cleared.save("./cleared.ppm");
    
//    int vert_divide;
//    QVector<QPoint> crossPoints;
//    BImage lineremoved = BoxCleaner::clearLineAndCloseLetters(cleared,40,&vert_divide,&crossPoints);
//    printf("vert_divide=%d\n",vert_divide);
//    lineremoved = BoxCleaner::trimHorizontalBoundaries(lineremoved);
//    lineremoved.save("./lineremoved.ppm");
//    QVector<BPartition*> cuts = WordSeparator::horzCutEntries(lineremoved,vert_divide);
    
    
//    lineremoved.claimOwnership(cuts[0],1);
//    lineremoved.claimOwnership(cuts[1],1);
//    lineremoved.saveOwners("./cut1.ppm");
    
    
    
//    QImage probMap("./average_desc.pgm");
//    QVector<QVector<double> > descenderProbMap=ImageAverager::produceProbabilityMap(probMap);
    
//    WordSeparator::adjustHorzCutCrossOverAreas(cuts[0],cuts[1],crossPoints,descenderProbMap);
    
//    lineremoved.claimOwnership(cuts[1],1);
//    lineremoved.claimOwnership(cuts[0],1);
    
//    lineremoved.saveOwners("./cut1_adjusted.ppm");
//    BImage upper = cuts[0]->makeImage();
//    upper.save("./cut1_a_top.ppm");
//    BImage lower = cuts[1]->makeImage();
//    lower.save("./cut1_b_bottom.ppm");

    
//    delete cuts[0];
//    delete cuts[1];
    
    ///////////////////////////////
    
//    BImage cleared = BoxCleaner::trimVerticleBoundaries(bimg);
//    cleared = BoxCleaner::trimHorizontalBoundaries(cleared);
//    cleared = BoxCleaner::removeVerticlePixelNoise(cleared);
//    QVector<BPartition*> lines = WordSeparator::segmentLinesOfWords(cleared,40);
//    for (int i=0; i<lines.size(); i++)
//    {
//        cleared.claimOwnership(lines[i],1);
////        segmentation[i]->makeImage().save("./output/");
//    }
//    cleared.saveOwners("./rainbow.ppm");
//    for (int i=0; i<lines.size(); i++)
//    {
//        delete lines[i];
//    }
    
    //////////////////////////////////////////////////////
        
    
//        BImage clean = BoxCleaner::trimBoundaries(bimg);
//        QVector<BPartition*> cuts;
//        WordSeparator::minCut(clean,cuts);
//        clean.claimOwnership(cuts[0],1);
//        clean.claimOwnership(cuts[1],1);
//        clean.saveOwners("./test.ppm");
    ////////////////////////////////////////////////////////
    ///Horz seg///
    
//    QString root(argv[1]);
//    for (int i=1; i<=50; i++)
//    {
//        QString imageNumber;
//        imageNumber.setNum(i);
//        QString imgPath;
//        if (i >=10)
//            imgPath = root + "000" + imageNumber + ".pgm";
//        else
//            imgPath = root + "0000" + imageNumber + ".pgm";
//        QImage testimg(imgPath);
//        BImage bimg(testimg);
//        BImage clean = BoxCleaner::trimBoundaries(bimg);
//        printf("Two words %d\n",i);
//        QVector<BPartition*> segmentation = WordSeparator::recursiveHorizontalCutTwoWordsTraining(clean);
//        for (int i=0; i<segmentation.size(); i++)
//        {
//            clean.claimOwnership(segmentation[i],1);
//    //        segmentation[i]->makeImage().save("./output/");
//        }
//        clean.saveOwners("./test.ppm");
        
//        BImage lastname =  segmentation[1]->makeImage();
//        printf("First letter %d\n",i);
//        QVector<BPartition*> segmentation2 = WordSeparator::recursiveHorizontalCutFirstLetterTraining(lastname);
//        for (int i=0; i<segmentation2.size(); i++)
//        {
//            lastname.claimOwnership(segmentation2[i],1);
//        }
//        lastname.saveOwners("./test.ppm");
        
//        for (int i=0; i<segmentation2.size(); i++)
//        {
//            delete segmentation2[i];
//        }
//        for (int i=0; i<segmentation.size(); i++)
//        {
//            delete segmentation[i];
//        }
//    }
    
    /////////////////////////////////////////////////////////
    
//    //ICDAR handler
//    QString icdarRoot(argv[1]);
//    int start = 205;
//    int end = 350;
//    if (argc>2)
//    {
//        start=QString(argv[2]).toInt();
//        end=QString(argv[2]).toInt();
//    }
    
//    for (int i=start; i<=end; i++)    
//    {
//        QString imageNumber;
//        imageNumber.setNum(i);
//        if (i<100)
//            imageNumber = "0" + imageNumber;
//        if (i<10)
//            imageNumber = "0" + imageNumber;
//    //    QString saveResultsRoot(argv[3]);
//        QString imgPath = icdarRoot + "images_test/" + imageNumber + ".tif";
//        QString gtPath = icdarRoot + "gt_lines_test/" + imageNumber + ".tif.dat";
        
        
//        QImage img(imgPath);
//    //    bool test = img.save("./icdartest.ppm");
//    //    if (!test)
//    //        printf("fail(%d,%d): %s, %s\n",img.width(),img.height(),imgPath.toLocal8Bit().data(),gtPath.toLocal8Bit().data());
//        BImage bimg(img);
        
//        int Ix=bimg.width(); //Image Width (x=0...Ix-1)
//        int Iy=bimg.height(); //Image Height (y=0...Iy-1)
//        unsigned int *IM_SegmResult; //Pointer to store raw data
//        FILE *f1;
        
//        IM_SegmResult = new unsigned int[Ix*Iy];//(unsigned int *) calloc (Ix*Iy,sizeof(int));
//        f1 = fopen(gtPath.toLocal8Bit().data(),"rb");
//        fread(IM_SegmResult,Ix*Iy,sizeof(int),f1);
//        fclose(f1);
        
//        QMap<int,BPartition*> lines;
        
//        for (int y=0; y<Iy; y++)
//        {
//           for (int x=0; x<Ix; x++)
//           {
//               if (IM_SegmResult[x+y*Ix]>0)
//               {
//                   if (lines.value(IM_SegmResult[x+y*Ix],NULL)==NULL)
//                   {
//                       lines[IM_SegmResult[x+y*Ix]] = new BPartition(&bimg);
//                   }
//                   lines[IM_SegmResult[x+y*Ix]]->addPixelFromSrc(x,y);
//               }
//           }
//        }
        
//        delete[] IM_SegmResult;
        
////        QVector<BPartition*> allSegmentations;
        
//        foreach(BPartition* part, lines.values())
//        {
////            bimg.claimOwnership(part,1);
////            BImage temp = part->makeImage();
//            QVector<BPartition*> segmentation = WordSeparator::recursiveHorizontalCutFull(*part);
//            for (int i=0; i<segmentation.size(); i++)
//            {
////                temp.claimOwnership(segmentation[i],1);
//                bimg.claimOwnershipVia(part,segmentation[i],1);
////                allSegmentations.append(segmentation[i]);
//            }
////            temp.saveOwners("./test.ppm");
////            printf("finished line %d\n",part->id());
            
//            for (int i=0; i<segmentation.size(); i++)
//            {
//                delete segmentation[i];
//            }
//            delete part;
            
//        }
        
        
////        bimg.saveOwners("./icdartest_words_result.ppm");
        
//        QString savePath = icdarRoot + "results/" + imageNumber + ".tif.dat";
//        bimg.saveICDAR(savePath);
////        for (int i=0; i<allSegmentations.size(); i++)
////        {
////            delete allSegmentations[i];
////        }
//        printf("Finished image %d\n",i);
//    }
    
    
    ////////////
//    Evaluate::horizontalSegmentationTest(QString(argv[1]));
//    Evaluate::verticleSegmentationTest(QString(argv[1]), QString(argv[2]));
    /////////////
//    QImage img(argv[1]);
//    BImage bimg(img);
//    BImage clean = BoxCleaner::trimBoundaries(bimg);
//    QVector<BPartition*> segmentation = WordSeparator::recursiveHorizontalCutFull(clean);
//    for (int j=0; j<segmentation.size(); j++)
//    {
//        clean.claimOwnership(segmentation[j],1);
////        segmentation[i]->makeImage().save("./output/");
//    }
//    clean.saveOwners("./test.ppm");
    
    
    
    
    ///test blob_ecc fill///////////////////
    QPoint cop(41,4);
    testBolbFill(bimg,cop);
    ///test/////////////////////////////////
    return 0;
}


