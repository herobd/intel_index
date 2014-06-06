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
    
    
    QVector<QVector<double> > slopes(bimg.width());
    for (int x=0; x<slopes.size(); x++)
    {
        for (int y=0; y<bimg.height(); y++)
        {
            if (bimg.pixel(x,y))
            {
                if (x>=10 && x<=14 &&y>=10 && y<=14)
                {
                    slopes[x].append(-1);
                }
                else if ((x<12 && y <12) || (x>12 && y>12))
                {
                    slopes[x].append((3.0/4.0)*PI);
                }
                else
                {
                    slopes[x].append((1.0/4.0)*PI);
                }
//                printf("(%d,%d)=%f\n",x,y,slopes[x][y]);
            }
            else
            {
                slopes[x].append(-1);
            }
            
        }
    }
    
    QVector<BPartition*> cuts = WordSeparator::testSlopeCut(bimg,slopes);
    bimg.claimOwnership(cuts[0],1);
    bimg.claimOwnership(cuts[1],1);
    bimg.saveOwners("./slope_separation.ppm");
    delete cuts[0];
    delete cuts[1];
    
    return 0;
}


