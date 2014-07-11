#include <iostream>
#include "wordprofile.h"
#include <QImage>
#include "wordseparator.h"
#include "boxcleaner.h"
#include "imageaverager.h"
#include "bimage.h"
#include <math.h>
#include "evaluate.h"


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
    
    ///////////////////////////////
    
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
        
    
//        BImage clean = BoxCleaner::trimBoundaries(bimg);
//        QVector<BPartition*> cuts;
//        WordSeparator::minCut(clean,cuts);
//        clean.claimOwnership(cuts[0],1);
//        clean.claimOwnership(cuts[1],1);
//        clean.saveOwners("./test.ppm");
    ////////////////////////////////////////////////////////
    ///Horz seg///
    
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
//        QVector<BPartition*> segmentation = WordSeparator::recursiveHorizontalCutTwoWords(clean);
//        for (int i=0; i<segmentation.size(); i++)
//        {
//            clean.claimOwnership(segmentation[i],1);
//    //        segmentation[i]->makeImage().save("./output/");
//        }
//        clean.saveOwners("./test.ppm");
        
//        BImage lastname =  segmentation[1]->makeImage();
//        printf("First letter %d\n",i);
//        QVector<BPartition*> segmentation2 = WordSeparator::recursiveHorizontalCutFirstLetter(lastname);
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
    
//    //ICDAR handler
//    QString icdarRoot(argv[1]);
//    int start = 205;
//    int end = 350;
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
//        QString imgPath = icdarRoot + "images_test/" + imageNumber + ".tif";
//        QString gtPath = icdarRoot + "gt_lines_test/" + imageNumber + ".tif.dat";
        
        
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
//            QVector<BPartition*> segmentation = WordSeparator::recursiveHorizontalCutFull(*part);
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
        
//        QString savePath = icdarRoot + "results/" + imageNumber + ".tif.dat";
//        bimg.saveICDAR(savePath);
////        for (int i=0; i<allSegmentations.size(); i++)
////        {
////            delete allSegmentations[i];
////        }
//        printf("Finished image %d\n",i);
//    }
    
    
    ///switcher//////////////////////////////////////////////////
//    ifstream infile("training_results.dat");
//    ofstream myfile ("redone_training_results.dat");
//    string line;
//    for (int i=0; i<23; i++)
//        getline(infile, line);
//    QRegExp re("(-?\\d+)(,\\d+,\\d+,\\d+,\\d+,)(-?\\d+)(,\\d+,\\d+,\\d+,\\d+,)(-?\\d+)(,\\d+,\\d+,\\d+,\\d+,)(.)");
//    while (getline(infile, line))
//    {
//        QString qLine(line.c_str());
//        re.indexIn(qLine);
//        int cut1_maxflow=re.cap(1).toInt();
//        int cut2L_maxflow=re.cap(3).toInt();
//        int cut2R_maxflow=re.cap(5).toInt();
//        int difL = cut1_maxflow-cut2L_maxflow;
//        int difR = cut1_maxflow-cut2R_maxflow;
        
//        myfile << re.cap(1).toLocal8Bit().data() << re.cap(2).toLocal8Bit().data() << re.cap(3).toLocal8Bit().data() << re.cap(4).toLocal8Bit().data() << re.cap(5).toLocal8Bit().data() << re.cap(6).toLocal8Bit().data() << difL << "," << difR << "," << re.cap(7).toLocal8Bit().data() << endl;
//    }
    
    
//    myfile.close();
//    infile.close();
    ///
    
    Evaluate::horizontalSegmentationTest(QString(argv[1]));
    
    return 0;
}


