#include <iostream>
#include "wordprofile.h"
#include <QImage>
#include "wordseparator.h"
#include "boxcleaner.h"
#include "imageaverager.h"
#include "bimage.h"
#include <math.h>


#include "gimage.h"
#include "gpartition.h"

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
    
    BImage cleared = BoxCleaner::trimVerticleBoundaries(bimg);
    cleared = BoxCleaner::removeVerticlePixelNoise(cleared);
    
    cleared.save("./cleared.ppm");
    
    int vert_divide;
    QVector<QPoint> crossPoints;
    BImage lineremoved = BoxCleaner::clearLineAndCloseLetters(cleared,40,&vert_divide,&crossPoints);
    printf("vert_divide=%d\n",vert_divide);
    lineremoved = BoxCleaner::trimHorizontalBoundaries(lineremoved);
    lineremoved.save("./lineremoved.ppm");
    QVector<BPartition*> cuts = WordSeparator::horzCutEntries(lineremoved,vert_divide);
    
    
    lineremoved.claimOwnership(cuts[0],1);
    lineremoved.claimOwnership(cuts[1],1);
    lineremoved.saveOwners("./cut1.ppm");
    
    
    
    QImage probMap("./average_desc.pgm");
    QVector<QVector<double> > descenderProbMap=ImageAverager::produceProbabilityMap(probMap);
    
    WordSeparator::adjustHorzCutCrossOverAreas(cuts[0],cuts[1],crossPoints,descenderProbMap);
    
    lineremoved.claimOwnership(cuts[1],1);
    lineremoved.claimOwnership(cuts[0],1);
    
    lineremoved.saveOwners("./cut1_adjusted.ppm");
    BImage upper = cuts[0]->makeImage();
    upper.save("./cut1_a_top.ppm");
    BImage lower = cuts[1]->makeImage();
    lower.save("./cut1_b_bottom.ppm");

    
//    delete cuts[0];
//    delete cuts[1];
//    delete top;
    
    //////3D/////////////////////////////////3D//////////////////////////////3D
    
    ///gggg
    
//    QVector<BPartition*> cuts = WordSeparator::cut3D();
//    bimg.claimOwnership(cuts[0],1);
//    bimg.claimOwnership(cuts[1],1);
//    bimg.saveOwners("./slope_separation.ppm");
//    cuts[0]->makeImage().save("./slope_separation_1.ppm");
//    cuts[1]->makeImage().save("./slope_separation_2.ppm");
//    delete cuts[0];
//    delete cuts[1];
    
    ///////////////////////END3D//////////////////////////////////////////
    
    
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


