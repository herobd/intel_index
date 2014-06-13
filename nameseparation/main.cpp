#include <iostream>
#include "wordprofile.h"
#include <QImage>
#include "wordseparator.h"
#include "boxcleaner.h"
#include "imageaverager.h"
#include "bimage.h"
#include <math.h>

using namespace std;

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
    
////    QVector<QVector<double> > slopes(bimg.width());
//    Dimension slopes(bimg.width(),bimg.height());
    
//    int numOfBins = (bimg.width()+bimg.height())/2;
//    QVector<QPoint> refPoints;
//    QVector<double> refSlopes;
//    QVector<double> refSlopes2;
//    //X
////    QPoint p1(5,5);
////    refPoints.append(p1);
////    refSlopes.append((3.0/4.0)*PI);
////    QPoint p2(19,19);
////    refPoints.append(p2);
////    refSlopes.append((3.0/4.0)*PI);
////    QPoint p3(5,19);
////    refPoints.append(p3);
////    refSlopes.append((1.0/4.0)*PI);
////    QPoint p4(19,5);
////    refPoints.append(p4);
////    refSlopes.append((1.0/4.0)*PI);
////    QPoint p5(2,2);
////    refPoints.append(p5);
////    refSlopes.append((.7)*PI);
//    //1
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
    
//    BImage mark(bimg);
//    QVector<QVector<QPoint> > stacks(refPoints.size());
//    for (int i=0; i<refPoints.size(); i++)
//    {
//        stacks[i].push_back(refPoints[i]);
//        mark.setPixel(refPoints[i],false);
//    }
//    bool cont = true;
//    int count = 0;
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
            
            
//            slopes.setValueForPixel(cur,refSlopes[i]);
//            if (refSlopes2[i]>=0)
//                slopes.setSecondValueForPixel(cur,refSlopes2[i]);
            
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
    
//    QVector<BPartition*> cuts = WordSeparator::testSlopeCut(bimg,dimensions);
//    bimg.claimOwnership(cuts[1],1);
//    bimg.claimOwnership(cuts[0],1);
//    bimg.saveOwners("./slope_separation.ppm");
//    cuts[0]->makeImage().save("./slope_separation_1.ppm");
//    cuts[1]->makeImage().save("./slope_separation_2.ppm");
//    delete cuts[0];
//    delete cuts[1];
    
    /////////////////////////////////////////////////////////////////
    
    //in progress
    BImage cleared = BoxCleaner::trimVerticleBoundaries(bimg);
    cleared = BoxCleaner::trimHorizontalBoundaries(cleared);
    cleared = BoxCleaner::removeVerticlePixelNoise(cleared);
    QVector<BPartition*> lines = WordSeparator::segmentLinesOfWords(cleared,40);
    
//    BImage clean = BoxCleaner::trimBoundaries(bimg);
//    QVector<BPartition*> cuts = WordSeparator::minCut(clean);
//    clean.claimOwnership(cuts[0],1);
//    clean.claimOwnership(cuts[1],1);
//    clean.saveOwners("./test.ppm");
    return 0;
}


