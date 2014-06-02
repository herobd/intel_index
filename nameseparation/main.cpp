#include <iostream>
#include "wordprofile.h"
#include <QImage>
#include "wordseparator.h"
#include "boxcleaner.h"
#include "imageaverager.h"
#include "bimage.h"

using namespace std;

int main(int argc, char** argv)
{
    cout << "Starting..." << endl;
    QImage testimg(argv[1]);
//    bool test;

    
    
    BImage bimg(testimg);
    
    BImage cleared = BoxCleaner::trimVerticleBoundaries(bimg);
    cleared = BoxCleaner::removeVerticlePixelNoise(cleared);
//    cleared = BoxCleaner::trimHorizontalBoundaries(cleared);
//    cleared = BoxCleaner::removePixelNoise(cleared);
    
    cleared.save("./cleared.ppm");
    
    int vert_divide;
    QVector<QPoint> crossPoints;
    BImage lineremoved = BoxCleaner::clearLineAndCloseLetters(cleared,40,&vert_divide,&crossPoints);
    
    lineremoved = BoxCleaner::trimHorizontalBoundaries(lineremoved);
    lineremoved = BoxCleaner::removePixelNoise(lineremoved);
    lineremoved.save("./lineremoved.ppm");
    QVector<BPartition*> cuts = WordSeparator::horzCutEntries(lineremoved,vert_divide);
    
    
    lineremoved.claimOwnership(cuts[1],1);
    lineremoved.claimOwnership(cuts[0],1);
    lineremoved.saveOwners("./cut1.ppm");
    
    BImage upper = cuts[0]->makeImage();
    upper.save("./cut1_a_top.ppm");
    BImage lower = cuts[1]->makeImage();
    lower.save("./cut1_b_bottom.ppm");
    
    QImage probMap("./average_desc.pgm");
    QVector<QVector<double> > descenderProbMap=ImageAverager::produceProbabilityMap(probMap);
    
    WordSeparator::adjustHorzCutCrossOverAreas(cuts[0],cuts[1],crossPoints,descenderProbMap);
    
    lineremoved.claimOwnership(cuts[1],1);
    lineremoved.claimOwnership(cuts[0],1);
    
    lineremoved.saveOwners("./cut1_adjusted.ppm");
    
//    BImage left = cuts[0]->makeImage();
//    left.save("cut_a_left.ppm");
    
//    BImage left2 = cuts2[0]->makeImage();
//    BImage right2 = cuts2[1]->makeImage();
//    left2.save("cut2_a_left.ppm");
//    right2.save("cut2_b_right.ppm");
    
    delete cuts[0];
    delete cuts[1];
//    delete cuts2[0];
//    delete cuts2[1];
    
    return 0;
}


