#include <iostream>
#include "wordprofile.h"
#include <QImage>
#include "wordseparator.h"
#include "boxcleaner.h"
#include "imageaverager.h"
#include "bimage.h"
#include <math.h>
#include <fstream>
#include <QRegExp>
#include "gimage.h"
#include "gpartition.h"

using namespace std;
struct tracePoint
{
    int x;
    int y;
    QVector<int> connectedPoints;
    QVector<double> angleBetween;
};

QPoint findClosestPointOn(BPixelCollection &img, QPoint &start)
{
    QVector<QPoint> searchQueue;
    searchQueue.append(start);
    
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
        searchQueue.append(up);
        searchQueue.append(down);
        searchQueue.append(left);
        searchQueue.append(right);
        searchQueue.append(lu);
        searchQueue.append(ld);
        searchQueue.append(ru);
        searchQueue.append(rd);
    }
}

int main(int argc, char** argv)
{
    cout << "Starting..." << endl;
    QImage testimg(argv[1]);
//    bool test;

    
    
    BImage bimg(testimg);
    
//    BPartition* top = WordSeparator::chopOutTop(bimg);
//    top->makeImage().save("./chop_top.ppm");
    
//    bimg.claimOwnership(top,1);
//    bimg.saveOwners("./chopped.ppm");
    
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
    
//    BImage upper = cuts[0]->makeImage();
//    upper.save("./cut1_a_top.ppm");
//    BImage lower = cuts[1]->makeImage();
//    lower.save("./cut1_b_bottom.ppm");
    
//    QImage probMap("./average_desc.pgm");
//    QVector<QVector<double> > descenderProbMap=ImageAverager::produceProbabilityMap(probMap);
    
//    WordSeparator::adjustHorzCutCrossOverAreas(cuts[0],cuts[1],crossPoints,descenderProbMap);
    
//    lineremoved.claimOwnership(cuts[1],1);
//    lineremoved.claimOwnership(cuts[0],1);
    
//    lineremoved.saveOwners("./cut1_adjusted.ppm");
    

    
//    delete cuts[0];
//    delete cuts[1];
//    delete top;
    
    //////3D/////////////////////////////////3D//////////////////////////////3D
    
    Dimension slopes(bimg.width(),bimg.height());
    
    int numOfBins = (bimg.width()+bimg.height())/2;
    QVector<QPoint> refPoints;
    QVector<QVector<double> > refSlopesM;
//    QVector<double> refSlopes;
//    QVector<double> refSlopes2;
    
//    //1
//    QPoint p1(3,51);
//    refPoints.append(p1);
//    refSlopes.append((.5)*PI);
//    refSlopes2.append(-1);
//    QPoint p2(17,50);
//    refPoints.append(p2);
//    refSlopes.append((.5)*PI);
//    refSlopes2.append(.15*PI);
//    QPoint p3(38,37);
//    refPoints.append(p3);
//    refSlopes.append((.35)*PI);
//    refSlopes2.append(-1);
//    QPoint p4(66,23);
//    refPoints.append(p4);
//    refSlopes.append((.1)*PI);
//    refSlopes2.append(-1);
//    QPoint p5(29,58);
//    refPoints.append(p5);
//    refSlopes.append((.45)*PI);
//    refSlopes2.append(-1);
//    QPoint p6(74,26);
//    refPoints.append(p6);
//    refSlopes.append((.4)*PI);
//    refSlopes2.append(-1);
//    QPoint p7(53,41);
//    refPoints.append(p7);
//    refSlopes.append((.18)*PI);
//    refSlopes2.append(-1);
//    QPoint p8(49,34);
//    refPoints.append(p8);
//    refSlopes.append((.4)*PI);
//    refSlopes2.append(-1);
//    QPoint p10(62,29);
//    refPoints.append(p10);
//    refSlopes.append(.38*PI);
//    refSlopes2.append(.13*PI);
//    QPoint p11(67,17);
//    refPoints.append(p11);
//    refSlopes.append(-1);
//    refSlopes2.append(-1);
////    QPoint p12(58,55);
////    refPoints.append(p12);
////    refSlopes.append(.25*PI);
////    refSlopes2.append(-1);
////    QPoint p13(55,70);
////    refPoints.append(p13);
////    refSlopes.append(.5*PI);
////    refSlopes2.append(-1);
////    QPoint p14(49,66);
////    refPoints.append(p14);
////    refSlopes.append(0);
////    refSlopes2.append(-1);
////    QPoint p15(46,62);
////    refPoints.append(p15);
////    refSlopes.append(.5*PI);
////    refSlopes2.append(-1);
    
//    for (int i =0; i<refSlopes.size(); i++)
//    {
//        QVector<double> slope;
//        if (refSlopes[i]>0)
//        slope.append(refSlopes[i]);
//        if (refSlopes2[i]>0)
//        slope.append(refSlopes2[i]);
//        refSlopesM.append(slope);
//    }
    
   //readfile 
    QVector<tracePoint> tracePoints;
    
    ifstream infile(argv[2]);
    string line;
    getline(infile, line);
    QRegExp rei("(\\d+)(?:[^\\d]+)(\\d+)(?:[^\\d]+)(\\d+)");
    QString qLine(line.c_str());
    rei.indexIn(qLine);
    tracePoint init;
    init.x=rei.cap(2).toInt();
    init.y=rei.cap(3).toInt();
    tracePoints.append(init);
    
    QRegExp re("(\\d+)(?:[^\\d]+)(\\d+)(?:[^\\d]+)(\\d+)(?:[^\\d]+)(\\d+)(?:[^\\d]+)(\\d+\\.\\d+)(?:[^\\d]+)(\\d+\\.\\d+)");
    while (getline(infile, line))
    {
        QString qLine(line.c_str());
        re.indexIn(qLine);
        tracePoint nextPoint;
        int index=re.cap(1).toInt();
//        printf("read index %d\n",index);
        if (index >= tracePoints.size())
        {
            nextPoint.x=re.cap(2).toInt()-1;
            nextPoint.y=re.cap(3).toInt()-1;
            int last = re.cap(4).toInt();
            double angle = re.cap(6).toDouble();
            if (angle < 0)
                angle += 180;
            nextPoint.connectedPoints.append(last);
            nextPoint.angleBetween.append(angle);
            tracePoints.append(nextPoint);
            
            tracePoints[last].connectedPoints.append(index);
            tracePoints[last].angleBetween.append(angle);
        }
        else//allow double point for looping
        {
//            printf("added loop (%d)\n",index);
            int last = re.cap(4).toInt();
            double angle = re.cap(6).toDouble();
            if (angle < 0)
                angle += 180;
            tracePoints[index].connectedPoints.append(last);
            tracePoints[index].angleBetween.append(angle);
            tracePoints[last].connectedPoints.append(index);
            tracePoints[last].angleBetween.append(angle);
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
        
        QPoint toAdd(tracePoints[curIndex].x,tracePoints[curIndex].y);
        
        QVector<double> slope;
        for (int i=0; i<tracePoints[curIndex].connectedPoints.size(); i++)
        {
            slope.append(tracePoints[curIndex].angleBetween[i]);
            if (!visited[tracePoints[curIndex].connectedPoints[i]])
            {
                pointStack.append(tracePoints[curIndex].connectedPoints[i]);
                int midX = (tracePoints[curIndex].x + tracePoints[ tracePoints[curIndex].connectedPoints[i] ].x)/2;
                int midY = (tracePoints[curIndex].y + tracePoints[ tracePoints[curIndex].connectedPoints[i] ].y)/2;
                QPoint mid(midX,midY);
                if (!bimg.pixel(midX,midY))
                {
                    //find closest
                    mid = findClosestPointOn(bimg,mid);
                }
                
                refPoints.append(mid);
                QVector<double> slopeMid;
                slopeMid.append(tracePoints[curIndex].angleBetween[i]);
                refSlopesM.append(slopeMid);
//                refSlopes.append(tracePoints[curIndex].angleBetween[0]);
//                refSlopes2.append(-1);
            }
        }
        refPoints.append(toAdd);
        refSlopesM.append(slope);
        
        
    }
    
    
    BImage mark(bimg);
    QVector<QVector<QPoint> > stacks(refPoints.size());
    for (int i=0; i<refPoints.size(); i++)
    {
        stacks[i].push_back(refPoints[i]);
        mark.setPixel(refPoints[i],false);
    }
    bool cont = true;
    int count = 0;
    while(cont)
    {
        cont = false;
        for (int i=0; i<stacks.size(); i++)
        {
            if (stacks[i].empty())
            {
                continue;
            }
            cont = true;
            QPoint cur = stacks[i].front();
            stacks[i].pop_front();
            
            
//            slopes.setValueForPixel(cur,refSlopes[i]);
//            if (refSlopes2[i]>=0)
//                slopes.setSecondValueForPixel(cur,refSlopes2[i]);
            
            slopes.setValuesForPixel(cur,refSlopesM[i]);
            
            if (cur.x()<mark.width()-1 && mark.pixel(cur.x()+1,cur.y()))
            {
                QPoint pp(cur.x()+1,cur.y());
                stacks[i].push_back(pp);
                mark.setPixel(pp,false);
            }
            if (cur.y()<mark.height()-1 && mark.pixel(cur.x(),cur.y()+1))
            {
                QPoint pp(cur.x(),cur.y()+1);
                stacks[i].push_back(pp);
                mark.setPixel(pp,false);
            }
            if (cur.x()>0 && mark.pixel(cur.x()-1,cur.y()))
            {
                QPoint pp(cur.x()-1,cur.y());
                stacks[i].push_back(pp);
                mark.setPixel(pp,false);
            }
            if (cur.y()>0 && mark.pixel(cur.x(),cur.y()-1))
            {
                QPoint pp(cur.x(),cur.y()-1);
                stacks[i].push_back(pp);
                mark.setPixel(pp,false);
            }
        }
    }

    slopes.setNumOfBins(numOfBins);
    slopes.setMinMax(0,179);
    
    NDimensions dimensions;
    dimensions.addDimension(slopes);
    
    
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
    
    QVector<QPoint> sourceSeeds;
    QVector<QPoint> sinkSeeds;
    
    //for subsection 2
    QPoint pa(71,18);
    sourceSeeds.append(pa);
    QPoint pb(17,69);
    sinkSeeds.append(pb);
    //for subsection 3
//    QPoint pa(33,16);
//    sourceSeeds.append(pa);
//    QPoint pb(0,48);
//    sinkSeeds.append(pb);
    //for subsection 4 (has slope issue, color code
//    QPoint pa(0,37);
//    sourceSeeds.append(pa);
//    QPoint pb(59,70);
//    sinkSeeds.append(pb);
    //for subsection 5
//    QPoint pa(68,21);
//    sourceSeeds.append(pa);
//    QPoint pb(42,70);
//    sinkSeeds.append(pb);
    //for subsection 6
//    QPoint pa(17,20);
//    QPoint paa(66,20);
//    sourceSeeds.append(pa);
//    sourceSeeds.append(paa);
//    QPoint pb(0,71);
//    QPoint pbb(48,66);
//    sinkSeeds.append(pb);
//    sinkSeeds.append(pbb);
    //for subsection 7
//    QPoint pa(89,6);
//    QPoint paa(0,12);
//    sourceSeeds.append(pa);
//    sourceSeeds.append(paa);
//    QPoint pb(4,55);
//    QPoint pbb(68,53);
//    sinkSeeds.append(pb);
//    sinkSeeds.append(pbb);
    //for subsection 8
//    QPoint pa(8,31);
//    QPoint paa(74,16);
//    sourceSeeds.append(pa);
//    sourceSeeds.append(paa);
//    QPoint pb(0,62);
//    QPoint pbb(48,64);
//    sinkSeeds.append(pb);
//    sinkSeeds.append(pbb);
    //for subsection 9
//    QPoint pa(11,16);
//    sourceSeeds.append(pa);
//    QPoint pb(7,53);
//    QPoint pbb(43,43);
//    sinkSeeds.append(pb);
//    sinkSeeds.append(pbb);
    //for subsection 16
//    QPoint pa(1,13);
//    sourceSeeds.append(pa);
//    QPoint pb(6,59);
//    sinkSeeds.append(pb);
    
    //for subsection EE cells
//    QPoint pa(588-x1,2083-y1);
//    QPoint paa(735-x1,2065-y1);
//    QPoint paaa(805-x1,2089-y1);
//    sourceSeeds.append(pa);
//    sourceSeeds.append(paa);
//    sourceSeeds.append(paaa);
//    QPoint pb(500-x1,2124-y1);
//    QPoint pbb(708-x1,2131-y1);
//    QPoint pbbb(881-x1,2129-y1);
//    sinkSeeds.append(pb);
//    sinkSeeds.append(pbb);
//    sinkSeeds.append(pbbb);
    
    QVector<BPartition*> cuts = WordSeparator::testSlopeCut(bimg,dimensions,sourceSeeds,sinkSeeds);
    bimg.claimOwnership(cuts[0],1);
    bimg.claimOwnership(cuts[1],1);
    bimg.saveOwners("./slope_separation.ppm");
    cuts[0]->makeImage().save("./slope_separation_1.ppm");
    cuts[1]->makeImage().save("./slope_separation_2.ppm");
    delete cuts[0];
    delete cuts[1];
    
    /////////////////////////////////////////////////////////////////
    
    
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
        
    
//    BImage clean = BoxCleaner::trimBoundaries(bimg);
//    QVector<BPartition*> cuts;
//    WordSeparator::minCut(clean,cuts);
//    clean.claimOwnership(cuts[0],1);
//    clean.claimOwnership(cuts[1],1);
//    clean.saveOwners("./test.ppm");
    
//    BImage clean = BoxCleaner::trimBoundaries(bimg);
//    QVector<BPartition*> segmentation = WordSeparator::recursiveHorizontalCutWords(clean);
//    for (int i=0; i<segmentation.size(); i++)
//    {
//        clean.claimOwnership(segmentation[i],1);
////        segmentation[i]->makeImage().save("./output/");
//    }
//    clean.saveOwners("./test.ppm");
//    for (int i=0; i<segmentation.size(); i++)
//    {
//        delete segmentation[i];
//    }
    return 0;
}


