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
    BPartition* all = lineremoved.getFullPartition();
    QVector<BPartition*> cuts = WordSeparator::horzCutEntries(*all,vert_divide,crossPoints,probMap);
    
//    QVector<BPartition*> cuts = WordSeparator::minCut(*all);
    lineremoved.saveOwners("./cut1.ppm");
    
//    BImage right = cuts[1]->makeImage();
    
//    right.save("cut_b_right.ppm");
    
    QVector<BPartition*> cuts2 = WordSeparator::minCut(*cuts[0]);
    lineremoved.saveOwners("./cut2.ppm");
    
    
//    BImage left = cuts[0]->makeImage();
//    left.save("cut_a_left.ppm");
    
//    BImage left2 = cuts2[0]->makeImage();
//    BImage right2 = cuts2[1]->makeImage();
//    left2.save("cut2_a_left.ppm");
//    right2.save("cut2_b_right.ppm");
    
    delete all;
    delete cuts[0];
    delete cuts[1];
    delete cuts2[0];
    delete cuts2[1];
    return 0;
}


