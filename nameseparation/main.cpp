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
//    QImage testimg(argv[1]);

    
    
//    BImage bimg(testimg);
    
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
    
    /////////////////////////////////////////////////////////
    
    //line reader
    QString icdarRoot(argv[1]);
    QString imageNumber(argv[2]);
    QString imgPath = icdarRoot + "images_train/" + imageNumber + ".tif";
    QString gtPath = icdarRoot + "gt_lines_train/" + imageNumber + ".tif.dat";
    
    QImage img(imgPath);
    bool test = img.save("./icdartest.ppm");
    if (!test)
        printf("fail(%d,%d): %s, %s\n",img.width(),img.height(),imgPath.toLocal8Bit().data(),gtPath.toLocal8Bit().data());
    BImage bimg(img);
    
    int Ix=bimg.width(); //Image Width (x=0...Ix-1)
    int Iy=bimg.height(); //Image Height (y=0...Iy-1)
    unsigned int *IM_SegmResult; //Pointer to store raw data
    FILE *f1;
    
    IM_SegmResult = (unsigned int *) calloc (Ix*Iy,sizeof(int));
    f1 = fopen(gtPath.toLocal8Bit().data(),"rb");
    fread(IM_SegmResult,Ix*Iy,sizeof(int),f1);
    fclose(f1);
    
    QMap<int,BPartition*> lines;
    
    for (int y=0; y<Iy; y++)
    {
       for (int x=0; x<Ix; x++)
       {
           if (IM_SegmResult[x+y*Ix]>0)
           {
               if (lines.value(IM_SegmResult[x+y*Ix],NULL)==NULL)
               {
                   lines[IM_SegmResult[x+y*Ix]] = new BPartition(&bimg);
               }
               lines[IM_SegmResult[x+y*Ix]]->addPixelFromSrc(x,y);
           }
       }
    }
    
    foreach(BPartition* part, lines.values())
    {
        bimg.claimOwnership(part,1);
        BImage temp = part->makeImage();
        QVector<BPartition*> segmentation = WordSeparator::recursiveHorizontalCutFull(temp);
        for (int i=0; i<segmentation.size(); i++)
        {
            temp.claimOwnership(segmentation[i],1);
    //        segmentation[i]->makeImage().save("./output/");
        }
        temp.saveOwners("./test.ppm");
        for (int i=0; i<segmentation.size(); i++)
        {
            delete segmentation[i];
        }
    }
    
    bimg.saveOwners("./icdartest_gt_lines.ppm");
    
    return 0;
}


