#include "trainer.h"
#include <dirent.h>
#include <iostream>
#include <errno.h>
#include "wordseparator.h"

//Two word:150	first letter:277
void Trainer::trainTwoWordSeparation(QString dir)
{
    int numTwoWord=0;
    int numFirstLetter=0;
    if (dir[dir.size()-1]!='/')
        dir+='/';
    //read in dir, anal all files
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(qPrintable(dir))) == NULL) {
        std::cout << "Error opening " << qPrintable(dir) << std::endl;
        return;
    }

    while ((dirp = readdir(dp)) != NULL) {
        
        if (dirp->d_name[0] != '.')
        {
            QString filePath = dir;
            filePath+=dirp->d_name;
            
            QImage img(filePath);
            
            BImage bimg(img);
            printf("Two words %s\n",dirp->d_name);
            QVector<BPartition*> segmentation = WordSeparator::recursiveHorizontalCutTwoWordsTraining(bimg);
            numTwoWord++;
            for (int i=0; i<segmentation.size(); i++)
            {
                bimg.claimOwnership(segmentation[i],1);
            }
            bimg.saveOwners("./test.ppm");
            
            BImage lastname =  segmentation[0]->makeImage();
            BImage firstname =  segmentation[1]->makeImage();
            printf("First letter %s\n",dirp->d_name);
            QVector<BPartition*> segmentation2;
            bool used = WordSeparator::recursiveHorizontalCutFirstLetterTraining(lastname,segmentation2);
            numFirstLetter += used?1:0;
            for (int i=0; i<segmentation2.size(); i++)
            {
                lastname.claimOwnership(segmentation2[i],1);
            }
            lastname.saveOwners("./test.ppm");
            for (int i=0; i<segmentation2.size(); i++)
            {
                delete segmentation2[i];
            }
            segmentation2.clear();
            
            used = WordSeparator::recursiveHorizontalCutFirstLetterTraining(firstname,segmentation2);
            numFirstLetter += used?1:0;
            for (int i=0; i<segmentation2.size(); i++)
            {
                firstname.claimOwnership(segmentation2[i],1);
            }
            firstname.saveOwners("./test.ppm");
            
            for (int i=0; i<segmentation2.size(); i++)
            {
                delete segmentation2[i];
            }
            for (int i=0; i<segmentation.size(); i++)
            {
                delete segmentation[i];
            }
        }
    }
    closedir(dp);
    
    printf("Two word:%d\tfirst letter:%d\n",numTwoWord,numFirstLetter);
//    QVector<BPartition*> segmentation = WordSeparator::recursiveHorizontalCutTwoWordsTraining(clean);
    //with a GT, this possibly could be automated.
}

//Two word:150
void Trainer::trainTwoWordSeparationDumb(QString dir)
{
    int numTwoWord=0;
    if (dir[dir.size()-1]!='/')
        dir+='/';
    //read in dir, anal all files
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(qPrintable(dir))) == NULL) {
        std::cout << "Error opening " << qPrintable(dir) << std::endl;
        return;
    }

    while ((dirp = readdir(dp)) != NULL) {
        
        if (dirp->d_name[0] != '.')
        {
            QString filePath = dir;
            filePath+=dirp->d_name;
            
            QImage img(filePath);
            
            BImage bimg(img);
            printf("Two words %s\n",dirp->d_name);
            QVector<BPartition*> segmentation = WordSeparator::recursiveHorizontalCutTwoWordsTrainingDumb(bimg);
            numTwoWord++;
            for (int i=0; i<segmentation.size(); i++)
            {
                bimg.claimOwnership(segmentation[i],1);
            }
            bimg.saveOwners("./test.ppm");
            
           
            for (int i=0; i<segmentation.size(); i++)
            {
                delete segmentation[i];
            }
        }
    }
    closedir(dp);
    
    printf("Two word:%d\n",numTwoWord);
//    QVector<BPartition*> segmentation = WordSeparator::recursiveHorizontalCutTwoWordsTraining(clean);
    //with a GT, this possibly could be automated.
}

void Trainer::trainIcdar(QString icdarRoot, int only)
{
    int start = 13; 
    int end = 23;
    if (only!=-1)
    {
        start=only;
        end=only;
    }
    
    for (int i=start; i<=end; i++)     
    {
        QString imageNumber;
        imageNumber.setNum(i);
        if (i<100)
            imageNumber = "0" + imageNumber;
        if (i<10)
            imageNumber = "0" + imageNumber;
    //    QString saveResultsRoot(argv[3]);
        QString imgPath = icdarRoot + "images_train/" + imageNumber + ".tif";
        QString gtPath = icdarRoot + "gt_lines_train/" + imageNumber + ".tif.dat";
        
        
        QImage img(imgPath);
    //    bool test = img.save("./icdartest.ppm");
    //    if (!test)
    //        printf("fail(%d,%d): %s, %s\n",img.width(),img.height(),imgPath.toLocal8Bit().data(),gtPath.toLocal8Bit().data());
        BImage bimg(img);
        
        int Ix=bimg.width(); //Image Width (x=0...Ix-1)
        int Iy=bimg.height(); //Image Height (y=0...Iy-1)
        unsigned int *IM_SegmResult; //Pointer to store raw data
        FILE *f1;
        
        IM_SegmResult = new unsigned int[Ix*Iy];//(unsigned int *) calloc (Ix*Iy,sizeof(int));
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
        
        delete[] IM_SegmResult;
        
//        QVector<BPartition*> allSegmentations;
        
        foreach(BPartition* part, lines.values())
        {
//            bimg.claimOwnership(part,1);
//            BImage temp = part->makeImage();
            QVector<BPartition*> segmentation = WordSeparator::recursiveHorizontalCutFullTraining(*part);
            for (int i=0; i<segmentation.size(); i++)
            {
//                temp.claimOwnership(segmentation[i],1);
                bimg.claimOwnershipVia(part,segmentation[i],1);
//                allSegmentations.append(segmentation[i]);
            }
//            temp.saveOwners("./test.ppm");
//            printf("finished line %d\n",part->id());
            
            for (int i=0; i<segmentation.size(); i++)
            {
                delete segmentation[i];
            }
            delete part;
            
        }
        
        
//        bimg.saveOwners("./icdartest_words_result.ppm");
        
//        QString savePath = icdarRoot + "results/" + imageNumber + ".tif.dat";
//        bimg.saveICDAR(savePath);
//        for (int i=0; i<allSegmentations.size(); i++)
//        {
//            delete allSegmentations[i];
//        }
        printf("Finished image %d\n",i);
    }
}
