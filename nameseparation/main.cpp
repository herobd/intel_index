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
    
    /////////////////////////////////////////////////////////////////////
    
//    QVector<QVector<double> > slopes(bimg.width());
    Dimension slopes(bimg.width(),bimg.height());
    
    int numOfBins = (bimg.width()+bimg.height())/2;
    QVector<QPoint> refPoints;
    QVector<QVector<double> > refSlopesM;
//    QVector<double> refSlopes;
//    QVector<double> refSlopes2;
    
    //1
//    QPoint p1(6,64);
//    refPoints.append(p1);
//    refSlopes.append((.5)*PI);
//    refSlopes2.append(-1);
//    QPoint p2(36,63);
//    refPoints.append(p2);
//    refSlopes.append((.5)*PI);
//    refSlopes2.append(-1);
//    QPoint p3(62,53);
//    refPoints.append(p3);
//    refSlopes.append((.35)*PI);
//    refSlopes2.append(-1);
//    QPoint p4(49,70);
//    refPoints.append(p4);
//    refSlopes.append((.75)*PI);
//    refSlopes2.append(-1);
//    QPoint p5(79,60);
//    refPoints.append(p5);
//    refSlopes.append((.45)*PI);
//    refSlopes2.append(-1);
//    QPoint p6(83,45);
//    refPoints.append(p6);
//    refSlopes.append((.4)*PI);
//    refSlopes2.append(-1);
//    QPoint p7(101,32);
//    refPoints.append(p7);
//    refSlopes.append((.18)*PI);
//    refSlopes2.append(-1);
//    QPoint p8(112,35);
//    refPoints.append(p8);
//    refSlopes.append((.4)*PI);
//    refSlopes2.append(-1);
//    QPoint p9(51,62);
//    refPoints.append(p9);
//    refSlopes.append(.5*PI);
//    refSlopes2.append(.2*PI);
//    QPoint p10(95,42);
//    refPoints.append(p10);
//    refSlopes.append(.38*PI);
//    refSlopes2.append(.13*PI);
//    QPoint p11(102,24);
//    refPoints.append(p11);
//    refSlopes.append(-1);
//    refSlopes2.append(-1);
//    QPoint p12(58,55);
//    refPoints.append(p12);
//    refSlopes.append(.25*PI);
//    refSlopes2.append(-1);
//    QPoint p13(55,70);
//    refPoints.append(p13);
//    refSlopes.append(.5*PI);
//    refSlopes2.append(-1);
//    QPoint p14(49,66);
//    refPoints.append(p14);
//    refSlopes.append(0);
//    refSlopes2.append(-1);
//    QPoint p15(46,62);
//    refPoints.append(p15);
//    refSlopes.append(.5*PI);
//    refSlopes2.append(-1);
    
    
    
    
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
        nextPoint.x=re.cap(2).toInt();
        nextPoint.y=re.cap(3).toInt();
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
    slopes.setMinMax(0,180);
    
    NDimensions dimensions;
    dimensions.addDimension(slopes);
    
    QVector<BPartition*> cuts = WordSeparator::testSlopeCut(bimg,dimensions);
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


