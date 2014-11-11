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
