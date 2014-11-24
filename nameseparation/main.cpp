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
#include "lrmfeaturevector.h"
#include "trainer.h"

#include "gimage.h"
#include "gpartition.h"

#include "gsl/gsl_statistics.h"

#include <opencv2/viz/vizcore.hpp>


using namespace std;



int main(int argc, char** argv)
{
    QImage testimg(argv[1]);

    
    
    BImage bimg(testimg);
    
    ///Test descender identification/////
//    BImage cleared = BoxCleaner::trimVerticleBoundaries(bimg);
//    cleared = BoxCleaner::trimHorizontalBoundaries(cleared);
//    cleared = BoxCleaner::removeVerticlePixelNoise(cleared);
//    QVector<BPartition*> lines = WordSeparator::segmentLinesOfWords(cleared,40);
//    for (int i=0; i<lines.size(); i++)
//    {
//        cleared.claimOwnership(lines[i],1);
//    }
//    cleared.saveOwners("./rainbow.ppm");
//    for (int i=0; i<lines.size(); i++)
//    {
//        delete lines[i];
//    }
    //9
//    QPoint cross(60,29);
    
    //10
//    QPoint cross(31,28);
    
    //12
//    QPoint cross(21,45);
    
    //13
//    QPoint cross(29,39);
    
    //14
//    QPoint cross(28,35);
    
    //15
//    QPoint cross(23,33);
    
    //16
//    QPoint cross(44,28);
    
    //18
//    QPoint cross(44,30);
    
    //19
//    QPoint cross(24,31);
    
    //20
//    QPoint cross(50,30);
    
//    Indexer3D indexer(ai.width(),ai.height());
//    GraphCut::strengthenDescenderComponent(ai,cross,NULL,indexer);
    ////////////////////////////////////
    
    
    ///Test fitting//////////////
//    QVector<double> xs;
//    QVector<double> ys;
//    for (int x=0; x<bimg.width(); x++)
//    {
//        for (int y=0; y<bimg.height(); y++)
//        {
//            if (bimg.pixel(x,y))
//            {
//                xs.append((double)x);
//                ys.append((double)y);
//            }
//        }
//    }
    
//    double cov[9];
//    double quadOut[3];
//    double chisqCurve = GraphCut::polynomialfit(xs.size(),3,ys.data(),xs.data(),quadOut,cov);
//    double rsq=1-chisqCurve/gsl_stats_tss(xs.data(),1,xs.size());
//    printf("[%f][%f][%f]\tchi^2=%f\tR^2=%f\n",quadOut[0],quadOut[1],quadOut[2],chisqCurve,rsq);
//    printf("%f\t%f\t%f\n",cov[0],cov[1],cov[2]);
//    printf("%f\t%f\t%f\n",cov[3],cov[4],cov[5]);
//    printf("%f\t%f\t%f\n",cov[6],cov[7],cov[8]);
    
//    QVector<QRgb> colors;//=testimg.colorTable();
//    colors.append(qRgb(255,255,255));
//    colors.append(qRgb(0,0,0));
//    colors.append(qRgb(255,0,0));
//    testimg.setColorTable(colors);
//    for (int y=0; y<testimg.height(); y++)
//    {
//        int x = quadOut[0] + y*quadOut[1] + y*y*quadOut[2];
//        testimg.setPixel(x,y,2);
//    }
//    testimg.save("./test.ppm");
//////////////////
    
    ////////////////////////
    //test recut3d//////////
    ////////////////////////
    QVector<QPoint> sourceSeeds;
    QVector<QPoint> sinkSeeds;
//    std::string xx = "angleImg";
//    cv::viz::Viz3d myWindow(xx);
    
    //horz
//    QPoint p(2,36);
//    QPoint px(35,33);
//    QPoint p2(352,11);
//    QPoint p2x(325,22);
//    QPoint cross(0,0);
    
    //1
//    QPoint p(4,29);
//    QPoint p2(6,64);
//    QPoint cross(102,33);
    
    //?
//    QPoint p(10,32);
//    QPoint p2(83,70);
    
    //2
//    QPoint p(63,18);
//    QPoint p2(64,64);
//    QPoint p2x(1,72);
//    QPoint cross(31,36);
    
    //4
//    QPoint p(86,29);
//    QPoint p2(13,72);
//    QPoint cross(36,39);
    
    //10
//    QPoint p(1,19);
//    QPoint px(82,28);
//    QPoint p2(99,66);
//    QPoint cross(42,28);
    
    //14
//    QPoint p(78,24);
//    QPoint p2(1,67);
//    QPoint p2x(52,66);
//    QPoint cross(28,35);
    
    //18
//    QPoint p(94,19);
//    QPoint p2(22,79);
//    QPoint cross(43,29);

    //20
//    QPoint p(17,2);
//    QPoint p2(49,68);
//    QPoint cross(50,29);
    
    //21
//    QPoint p(65,25);
//    QPoint p2(35,66);
//    QPoint cross(14,41);
    
    //22 this is a good example of 3d cut vanilla working (e=60, b=200, m=10000)
//    QPoint p(1,18);
//    QPoint px(69,12);
//    QPoint p2(0,47);
//    QPoint p2x(53,48);
////    QPoint p2xx(28,63);
//    QPoint cross(35,23);
    
    //23 
//    QPoint p(5,10);
//    QPoint px(80,14);
//    QPoint p2(66,50);
//    QPoint cross(35,24);
    
    //24 
//    QPoint p(1,23);
//    QPoint px(18,4);
//    QPoint p2(14,35);
//    QPoint p2x(37,66);
//    QPoint p2xx(70,58);
//    QPoint cross(58,24);
    
    //25
//    QPoint p(17,17);
//    QPoint px(99,14);
//    QPoint p2(3,52);
//    QPoint cross(60,28);
    
    //26
//    QPoint p(41,0);
//    QPoint p2(51,58);
//    QPoint cross(6,18);
    
    //27
//    QPoint p(39,14);
//    QPoint p2(0,52);
//    QPoint p2x(112,51);
//    QPoint cross(0,0);
    
    //x2
//    QPoint p(12,14);
//    QPoint p2(70,16);
//    QPoint cross(0,0);
    
    //x3
    QPoint p(73,1);
    QPoint p2(29,54);
    QPoint p2x(67,57);
    QPoint cross(56,25);
    
    sourceSeeds.append(p);
//    sourceSeeds.append(px);
    
    sinkSeeds.append(p2);
    sinkSeeds.append(p2x);
//    sinkSeeds.append(p2xx);
    QVector<QPoint> crossing;
    crossing.append(cross);
    QVector<BPartition*> result = WordSeparator::recut3D(bimg, sourceSeeds, sinkSeeds,cross.y(),crossing);
    bool test = result[0]->makeImage().save("./test0.ppm");
    test &= result[1]->makeImage().save("./test1.ppm");
    if (!test)
        printf("Save image failure.\n");
    bimg.claimOwnership(result[0],1);
    bimg.claimOwnership(result[1],1);
    bimg.saveOwners("./test.ppm");
    
//    /// Start event loop
////    myWindow.spin();
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
    //segment full column
    
//    BImage cleared = BoxCleaner::trimVerticleBoundaries(bimg);
//    cleared = BoxCleaner::trimHorizontalBoundaries(cleared);
//    cleared = BoxCleaner::removeVerticlePixelNoise(cleared);
//    QVector<BPartition*> lines = WordSeparator::segmentLinesOfWords(cleared,40);
//    for (int i=0; i<lines.size(); i++)
//    {
//        cleared.claimOwnership(lines[i],1);
////        segmentation[i]->makeImage().save("./output/");
//        QString imageNumber;
//        imageNumber.setNum(1+i);
//        if (i+1<10)
//            imageNumber = "0" + imageNumber;
//        lines[i]->makeImage().save("./vert_seg_res/" + imageNumber + ".ppm");
//    }
//    cleared.saveOwners("./rainbow.ppm");
//    for (int i=0; i<lines.size(); i++)
//    {
//        delete lines[i];
//    }
    
    //////////////////////////////////////////////////////
        
    
//        BImage clean = bimg;//BoxCleaner::trimBoundaries(bimg);
//        QVector<BPartition*> cuts;
//        WordSeparator::minCut(clean,cuts);
//        clean.claimOwnership(cuts[0],1);
//        clean.claimOwnership(cuts[1],1);
//        clean.saveOwners("./test.ppm");
    
//    QVector<BPartition*> cuts = WordSeparator::recursiveHorizontalCutTwoWordsTraining(bimg);
//    bimg.claimOwnership(cuts[0],1);
//    bimg.claimOwnership(cuts[1],1);
//    bimg.saveOwners("./test.ppm");
    
    ////////////////////////////////////////////////////////
    ///Horz seg///
    
    ////NEW TESTING/////
//    BImage clean = BoxCleaner::trimBoundaries(bimg);
//    QVector<BPartition*> chunks = WordSeparator::recursiveChunkWord(&clean, &clean,0,0);
//    foreach (BPartition *p, chunks)
//        clean.claimOwnership(p,1.0);
//    clean.saveOwners("./test.ppm");
    
    ///old
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
    
    //New ICDAR trainer
//    int only=-1;
//    if (argc>2)
//    {
//        only=QString(argv[2]).toInt();
//    }
//    QString root(argv[1]);
//    Trainer::trainIcdar(root,only);
    
    //ICDAR handler
//    QString icdarRoot(argv[1]);
//    int start = 3; 
//    int end = 10;
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
//        QString imgPath = icdarRoot + "images_train/" + imageNumber + ".tif";
//        QString gtPath = icdarRoot + "gt_lines_train/" + imageNumber + ".tif.dat";
        
        
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
//            QVector<BPartition*> segmentation = WordSeparator::recursiveHorizontalCutFullTraining(*part);
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
        
////        QString savePath = icdarRoot + "results/" + imageNumber + ".tif.dat";
////        bimg.saveICDAR(savePath);
////        for (int i=0; i<allSegmentations.size(); i++)
////        {
////            delete allSegmentations[i];
////        }
//        printf("Finished image %d\n",i);
//    }
////END ICDAR/////
    
    ////////////
//    Evaluate::horizontalSegmentationTest(QString(argv[1]));
//    Evaluate::verticleSegmentationTest(QString(argv[1]), QString(argv[2]),false);
//    Evaluate::evaluateScoreInfo(QString("/home/brian/intel_index/testing/results/correctScores.dat"),QString("/home/brian/intel_index/testing/results/incorrectScores.dat"));
//    Evaluate::writeScores(QString(argv[1]),QString("/home/brian/intel_index/testing/results/correctScores.dat"),QString("/home/brian/intel_index/testing/results/incorrectScores.dat"));
//    Evaluate::newTwoNameHorizontalSegmentationTest(argv[1],argv[2]);
//    Evaluate::newFirstLetterHorizontalSegmentationTest(argv[1],argv[2],argv[3]);
//    Evaluate::newDumbTwoNameHorizontalSegmentationTest(argv[1],argv[2]);
    
//    Trainer::trainTwoWordSeparation(argv[1]);
//    Trainer::trainTwoWordSeparationDumb(argv[1]);
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
//    QPoint cop(31,4);

//    testBolbFill(bimg,cop);
    ///test/////////////////////////////////
    
    /////feature stuff
//    LRMFeatureVector fv(bimg);
    
    
    return 0;
}


