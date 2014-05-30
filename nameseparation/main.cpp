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
    
    int vert_divide;
    QVector<QPoint> crossPoints;
    QVector<QVector<double> > probMap;
    BImage lineremoved = BoxCleaner::clearLineAndCloseLetters(bimg,40,&vert_divide,&crossPoints);
    lineremoved.save("./lineremoved.ppm");
    QVector<BPartition*> cuts = WordSeparator::horzCutEntries(lineremoved,vert_divide);
    
//    QVector<BPartition*> cuts = WordSeparator::minCut(*all);
//    lineremoved.saveOwners("./cut1.ppm");
    
//    BImage right = cuts[1]->makeImage();
    
//    right.save("cut_b_right.ppm");
    
//    QVector<BPartition*> cuts2 = WordSeparator::minCut(*cuts[0],1);
//    WordSeparator::adjustHorzCutCrossOverAreas(cuts[0],cuts[1],crossPoints);
    lineremoved.claimOwnership(cuts[1],1);
    lineremoved.claimOwnership(cuts[0],1);
    
    lineremoved.saveOwners("./cut2.ppm");
    
    BImage upper = cuts[0]->makeImage();
    upper.save("./cut1_a_top.ppm");
    BImage lower = cuts[1]->makeImage();
    lower.save("./cut1_b_bottom.ppm");
    
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
    
//    BPartition test((BPartition*)&bimg);
////    BImage print0 = test.makeImage();
//    test.addPixelFromSrc(0,0);
//    test.addPixelFromSrc(1,1);
//    test.addPixelFromSrc(2,2);
//    test.addPixelFromSrc(3,3);
//    BImage print1 = test.makeImage();
    
    
////    print0.save("./print0.ppm");
//    print1.save("./print1.ppm");
    return 0;
}


