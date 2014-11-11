#include "evaluate.h"
#include <assert.h>

Evaluate::Evaluate()
{
}

double Evaluate::matchedScore(BPartition &part, BPixelCollection &img)
{
    assert(part.getSrc()->width()==img.width() && part.getSrc()->height()==img.height());
    int intersectCount=0;
    int joinCount=0;
    for (int y=0; y<part.getSrc()->height(); y++)
    {
        for (int x=0; x<part.getSrc()->width(); x++)
        {
            if (part.pixelSrc(x,y) && img.pixel(x,y))
                intersectCount++;
            if (part.pixelSrc(x,y) || img.pixel(x,y))
                joinCount++;
        }
    }
    return ((double) intersectCount) / ((double) joinCount);
}

double Evaluate::matchedScore(BPixelCollection &img1, BPixelCollection &img2)
{
    assert(img1.width()==img2.width() && img1.height()==img2.height());
    int intersectCount=0;
    int joinCount=0;
    for (int y=0; y<img1.height(); y++)
    {
        for (int x=0; x<img1.width(); x++)
        {
            if (img1.pixel(x,y) && img2.pixel(x,y))
                intersectCount++;
            if (img1.pixel(x,y) || img2.pixel(x,y))
                joinCount++;
        }
    }
    return ((double) intersectCount) / ((double) joinCount);
}

double Evaluate::matchedScore(const BPartition &left, const BPartition &right, const QImage &coloredGT)
{
    assert(left.getSrc()->width()==coloredGT.width() && left.getSrc()->height()==coloredGT.height());
    assert(right.getSrc()->width()==coloredGT.width() && right.getSrc()->height()==coloredGT.height());
    int intersectCountL=0;
    int joinCountL=0;
    int intersectCountR=0;
    int joinCountR=0;
    QRgb leftColor = coloredGT.pixel(0,0);//should be red
    QRgb rightColor = coloredGT.pixel(1,0);//should be blue
    assert(qRed(leftColor)==255 && qBlue(rightColor)==255);
    for (int y=0; y<coloredGT.height(); y++)
    {
        for (int x=0; x<coloredGT.width(); x++)
        {
            if (coloredGT.pixel(x,y)==leftColor || coloredGT.pixel(x,y)==rightColor)
            {
                if (left.pixelSrc(x,y) && coloredGT.pixel(x,y)==leftColor)
                    intersectCountL++;
                if (left.pixelSrc(x,y) || coloredGT.pixel(x,y)==leftColor)
                    joinCountL++;
                
                if (right.pixelSrc(x,y) && coloredGT.pixel(x,y)==rightColor)
                    intersectCountR++;
                if (right.pixelSrc(x,y) || coloredGT.pixel(x,y)==rightColor)
                    joinCountR++;
            }
            
            
        }
    }
    return (((double) intersectCountL) / ((double) joinCountL)) + (((double) intersectCountR) / ((double) joinCountR));
}

//no recal pentalty for 2nd portion on loose score
void Evaluate::matchedScoreFirstLetter(bool left, const BPartition &part, const QImage &coloredGT, double &score, double &looseScore)
{
    assert(part.getSrc()->width()==coloredGT.width() && part.getSrc()->height()==coloredGT.height());
    int intersectCount1=0;
    int joinCount1=0;
    int intersectCount2=0;
//    int joinCount2=0;
    QRgb leftColor = coloredGT.pixel(0,0);//should be red
    QRgb rightColor = coloredGT.pixel(1,0);//should be blue
    QRgb firstColor = coloredGT.pixel(2,0);//should be green
    QRgb secondColor = coloredGT.pixel(3,0);//should be magenta
    assert(qRed(leftColor)==255 && qBlue(rightColor)==255);
    for (int y=0; y<coloredGT.height(); y++)
    {
        for (int x=0; x<coloredGT.width(); x++)
        {
            if ((coloredGT.pixel(x,y)==leftColor&&left || coloredGT.pixel(x,y)==rightColor&&!left) || coloredGT.pixel(x,y)==firstColor || coloredGT.pixel(x,y)==secondColor)
            {
                if (part.pixelSrc(x,y) && coloredGT.pixel(x,y)==firstColor)
                    intersectCount1++;
                if (part.pixelSrc(x,y) || (coloredGT.pixel(x,y)==firstColor))
                    joinCount1++;
                
                if (part.pixelSrc(x,y) && (coloredGT.pixel(x,y)==firstColor || coloredGT.pixel(x,y)==secondColor))
                    intersectCount2++;
            }
            
            
        }
    }
    score = ((double)intersectCount1)/joinCount1;
    looseScore = ((double)intersectCount2)/joinCount1;
}


double Evaluate::verticleSegmentationTest(QString imgPath, QString gtDirPath, bool dumb)
{
    QString outPath;
    if (dumb)
        outPath= "/home/brian/intel_index/testing/results/newDumbVertSegResults.dat"; 
    else
        outPath= "/home/brian/intel_index/testing/results/newVertSegResults.dat"; 
    std::ofstream results (outPath.toLocal8Bit().data(),std::ofstream::app);
    QImage img(imgPath);
    BImage bimg(img);
    BImage cleared = BoxCleaner::trimVerticleBoundaries(bimg);
    cleared = BoxCleaner::trimHorizontalBoundaries(cleared);
    cleared = BoxCleaner::removeVerticlePixelNoise(cleared);
    double summedScore=0;
    QVector<BPartition*> lines= WordSeparator::segmentLinesOfWords(cleared,40,dumb);
    for (int i=0; i<lines.size(); i++)
    {
        if (i!=0)
            results << ", ";
        cleared.claimOwnership(lines[i],1);
        
        
        QString imageNumber;
        imageNumber.setNum(1+i);
        if (i+1<10)
            imageNumber = "0" + imageNumber;
        QString gtPath = gtDirPath + imageNumber + ".pgm";
//        printf("gt:%s\n",gtPath.toLocal8Bit().data())
        QImage gt(gtPath);
        BImage bgt(gt);
        double score=matchedScore(*lines[i],bgt);
        summedScore+=score;
        results << score;
//        printf ("line %d finished\n",i);
        lines[i]->makeImage().save("./vert_seg_res/" + imageNumber + ".ppm");
    }
    double avgScore = summedScore/lines.size();
    results << ": " << avgScore << std::endl;
    results.close();
    cleared.saveOwners("./rainbow.ppm");
    for (int i=0; i<lines.size(); i++)
    {
        delete lines[i];
    }
    return avgScore;
}

double Evaluate::newTwoNameHorizontalSegmentationTest(QString imgDirPath, QString gtDirPath)
{
    if (imgDirPath[imgDirPath.size()-1]!='/')
        imgDirPath+='/';
    if (gtDirPath[gtDirPath.size()-1]!='/')
        gtDirPath+='/';
    QString outPath = "/home/brian/intel_index/testing/results/newTwoNameHorzSegResults.dat"; 
    std::ofstream results (outPath.toLocal8Bit().data(),std::ofstream::app);
    double summedScore=0;
//    QVector<BPartition*> lines = WordSeparator::segmentLinesOfWords(cleared,40);
    DIR* dirp = opendir(qPrintable(imgDirPath));
    struct dirent *dp;
    int count=0;
    
    QRegExp re("(.+)\\.pgm");
    while ((dp = readdir(dirp)) != NULL)
    {
        if (count!=0)
            results << ", ";
        QString file(dp->d_name);
        re.indexIn(file);
        if (re.matchedLength()==-1)
            continue;
        QString fileName=re.cap(1);
        QString imgPath = imgDirPath + fileName + ".pgm";
        QString gtPath = gtDirPath + fileName + ".ppm";
//        printf("img: %s\n",imgPath.toLocal8Bit().data());
//        printf("gt: %s\n",gtPath.toLocal8Bit().data());
        QImage gt(gtPath);
        QImage img(imgPath);
        BImage bimg(img);
//        BImage clean = BoxCleaner::trimBoundaries(bimg);
        QVector<BPartition*> segmentation = WordSeparator::recursiveHorizontalCutTwoWords(bimg);
        
        double score=matchedScore(*segmentation[0], *segmentation[1], gt);
        summedScore+=score;
        results << score;
        count++;
        
        segmentation[0]->changeSrc(&bimg,0,0);
        segmentation[1]->changeSrc(&bimg,0,0);
        bimg.claimOwnership(segmentation[0],1.0);
        bimg.claimOwnership(segmentation[1],1.0);
        bimg.saveOwners("./horz_seg_res/" + fileName + ".ppm");
    }
    double avgScore = summedScore/(count*2);
    results << ": " << avgScore << std::endl;
    results.close();
    return avgScore;
}

double Evaluate::newDumbTwoNameHorizontalSegmentationTest(QString imgDirPath, QString gtDirPath)
{
    if (imgDirPath[imgDirPath.size()-1]!='/')
        imgDirPath+='/';
    if (gtDirPath[gtDirPath.size()-1]!='/')
        gtDirPath+='/';
    QString outPath = "/home/brian/intel_index/testing/results/newDumbTwoNameHorzSegResults.dat"; 
    std::ofstream results (outPath.toLocal8Bit().data(),std::ofstream::app);
    double summedScore=0;
//    QVector<BPartition*> lines = WordSeparator::segmentLinesOfWords(cleared,40);
    DIR* dirp = opendir(qPrintable(imgDirPath));
    struct dirent *dp;
    int count=0;
    
    QRegExp re("(.+)\\.pgm");
    while ((dp = readdir(dirp)) != NULL)
    {
        if (count!=0)
            results << ", ";
        QString file(dp->d_name);
        re.indexIn(file);
        if (re.matchedLength()==-1)
            continue;
        QString fileName=re.cap(1);
        QString imgPath = imgDirPath + fileName + ".pgm";
        QString gtPath = gtDirPath + fileName + ".ppm";
//        printf("img: %s\n",imgPath.toLocal8Bit().data());
//        printf("gt: %s\n",gtPath.toLocal8Bit().data());
        QImage gt(gtPath);
        QImage img(imgPath);
        BImage bimg(img);
//        BImage clean = BoxCleaner::trimBoundaries(bimg);
        QVector<BPartition*> segmentation = WordSeparator::recursiveHorizontalCutTwoWordsDumb(bimg);
        
        double score=matchedScore(*segmentation[0], *segmentation[1], gt);
        summedScore+=score;
        results << score;
        count++;
        
        segmentation[0]->changeSrc(&bimg,0,0);
        segmentation[1]->changeSrc(&bimg,0,0);
        bimg.claimOwnership(segmentation[0],1.0);
        bimg.claimOwnership(segmentation[1],1.0);
        bimg.saveOwners("./horz_seg_res/" + fileName + ".ppm");
    }
    double avgScore = summedScore/(count*2);
    results << ": " << avgScore << std::endl;
    results.close();
    return avgScore;
}

double Evaluate::newFirstLetterHorizontalSegmentationTest(QString twoNamesGtDirPath, QString firstLetterLastNameGtDirPath, QString firstLetterFirstNameGtDirPath)
{
    if (twoNamesGtDirPath[twoNamesGtDirPath.size()-1]!='/')
        twoNamesGtDirPath+='/';
    if (firstLetterLastNameGtDirPath[firstLetterLastNameGtDirPath.size()-1]!='/')
        firstLetterLastNameGtDirPath+='/';
    if (firstLetterFirstNameGtDirPath[firstLetterFirstNameGtDirPath.size()-1]!='/')
        firstLetterFirstNameGtDirPath+='/';
    QString outPath = "/home/brian/intel_index/testing/results/newFirstLetterHorzSegResults.dat"; 
    std::ofstream results (outPath.toLocal8Bit().data(),std::ofstream::app);
    double summedScore=0;
    double summedScoreLoose=0;
//    QVector<BPartition*> lines = WordSeparator::segmentLinesOfWords(cleared,40);
    DIR* dirp = opendir(qPrintable(twoNamesGtDirPath));
    struct dirent *dp;
    int count=0;
    
    QRegExp re("(.+)\\.ppm");
    while ((dp = readdir(dirp)) != NULL)
    {
        if (count!=0)
            results << ", ";
        QString file(dp->d_name);
        re.indexIn(file);
        if (re.matchedLength()==-1)
            continue;
        QString fileName=re.cap(1);
        QString imgPath = twoNamesGtDirPath + fileName + ".ppm";
        QString gtPathL = firstLetterLastNameGtDirPath + fileName + ".ppm";
        QString gtPathR = firstLetterFirstNameGtDirPath + fileName + ".ppm";
//        printf("img: %s\n",imgPath.toLocal8Bit().data());
//        printf("gt: %s\n",gtPathR.toLocal8Bit().data());
        QImage gtL(gtPathL);
        QImage gtR(gtPathR);
        QImage img(imgPath);
        BImage leftName(img.width(), img.height());
        BImage rightName(img.width(), img.height());
        QRgb leftColor = img.pixel(0,0);//should be red
        QRgb rightColor = img.pixel(1,0);//should be blue
        assert(qRed(leftColor)==255 && qBlue(rightColor)==255);
        for (int x=0; x<img.width(); x++)
        {
            for (int y=0; y<img.height(); y++)
            {
                if (img.pixel(x,y)==leftColor)
                    leftName.setPixel(x,y,true);
                if (img.pixel(x,y)==rightColor)
                    rightName.setPixel(x,y,true);
            }
        }
        
        
//        BImage clean = BoxCleaner::trimBoundaries(bimg);
        QVector<BPartition*> segmentationL = WordSeparator::recursiveHorizontalCutFirstLetter(leftName);
        QVector<BPartition*> segmentationR = WordSeparator::recursiveHorizontalCutFirstLetter(rightName);
        
        double scoreL;
        double looseScoreL;
        double scoreR;
        double looseScoreR;
//        printf("test %dx%d, %dx%d, %dx%d, %dx%d\n",segmentationL[0]->width(),segmentationL[0]->height(),gtL.width(), gtL.height(),segmentationR[0]->width(),segmentationR[0]->height(),gtR.width(), gtR.height());
        matchedScoreFirstLetter(true,*segmentationL[0], gtL, scoreL, looseScoreL);
        matchedScoreFirstLetter(false,*segmentationR[0], gtR, scoreR, looseScoreR);
        summedScore+=scoreL+scoreR;
        summedScoreLoose+=looseScoreL+looseScoreR;
        results << scoreL << ":" << looseScoreL << ", " << scoreR << ":" << looseScoreR;
        count++;
        
//        segmentation[0]->changeSrc(&bimg,0,0);
//        segmentation[1]->changeSrc(&bimg,0,0);
//        bimg.claimOwnership(segmentation[0],1.0);
//        bimg.claimOwnership(segmentation[1],1.0);
//        bimg.saveOwners("./horz_seg_res/" + fileName + ".ppm");
    }
    double avgScore = summedScore/(count*2);
    double avgScoreLoose = summedScoreLoose/(count*2);
    results << "AVG= " << avgScore << ":" << avgScoreLoose << std::endl;
    results.close();
    return avgScore;
}


double Evaluate::horizontalSegmentationTest(QString root)
{
    int correctNames=0;
    int totalFirstLetters=0;
    int correctFirstLetters=0;
    int correctFirstTwoLetters=0;
    for (int i=1; i<=50; i++)
    {
        QString imageNumber;
        imageNumber.setNum(i);
        QString imgPath;
        if (i >=10)
            imgPath = root + "000" + imageNumber + ".pgm";
        else
            imgPath = root + "0000" + imageNumber + ".pgm";
        QImage testimg(imgPath);
        BImage bimg(testimg);
        BImage clean = BoxCleaner::trimBoundaries(bimg);
//        printf("Two words %d\n",i);
        QVector<BPartition*> segmentation = WordSeparator::recursiveHorizontalCutTwoWords(clean);
        for (int j=0; j<segmentation.size(); j++)
        {
            clean.claimOwnership(segmentation[j],1);
    //        segmentation[i]->makeImage().save("./output/");
        }
        clean.saveOwners("./horzTest.ppm");
        
        char read;
        char dump;
        while (true)
        {
            printf("%d Correct?:",i);
            scanf("%c%c",&read,&dump);
            if (read=='m')
            {
                correctNames++;
                break;
            }
            else if (read=='n')
            {
                break;
            }
        }
        
        totalFirstLetters++;
        BImage firstname =  segmentation[0]->makeImage();
//        printf("First letter %d\n",i);
        QVector<BPartition*> segmentation2 = WordSeparator::recursiveHorizontalCutFirstLetter(firstname);
//        for (int i=0; i<segmentation2.size(); i++)
//        {
            firstname.claimOwnership(segmentation2[0],1);
//        }
        firstname.saveOwners("./horzTest.ppm");
        char read2;
        while (true)
        {
            printf("Letter correct?(one):");
            scanf("%c%c",&read2,&dump);
            if (read2=='m')
            {
                correctFirstLetters++;
                break;
            }
            else if (read2=='j')
            {
                correctFirstTwoLetters++;
                break;
            }
            else if (read2=='n')
            {
                break;
            }
            else if (read2=='s' || read2==' ')
            {
                totalFirstLetters--;
                break;
            }
        }
        for (int i=0; i<segmentation2.size(); i++)
        {
            delete segmentation2[i];
        }
        
        if (read=='m')
        {
            totalFirstLetters++;
            BImage lastname =  segmentation[1]->makeImage();
//            printf("First letter %d\n",i);
            QVector<BPartition*> segmentation3 = WordSeparator::recursiveHorizontalCutFirstLetter(lastname);
//            for (int i=0; i<segmentation3.size(); i++)
//            {
                lastname.claimOwnership(segmentation3[0],1);
//            }
            lastname.saveOwners("./horzTest.ppm");
            char read3;
            while (true)
            {
                printf("Letter correct?(two):");
                scanf("%c%c",&read3,&dump);
                if (read3=='m')
                {
                    correctFirstLetters++;
                    break;
                }
                else if (read3=='j')
                {
                    correctFirstTwoLetters++;
                    break;
                }
                else if (read3=='n')
                {
                    break;
                }
                else if (read3=='s' || read3==' ')
                {
                    totalFirstLetters--;
                    break;
                }
            }
            for (int i=0; i<segmentation3.size(); i++)
            {
                delete segmentation3[i];
            }
        }
        for (int i=0; i<segmentation.size(); i++)
        {
            delete segmentation[i];
        }
    }
    double perWords = correctNames/50.0;
    double perLetters = ((double) correctFirstLetters) / ((double) totalFirstLetters);
    double perLooseLetters = ((double) correctFirstLetters + correctFirstTwoLetters) / ((double) totalFirstLetters);
    printf("Names: %f, Letters(strict): %f, Letters(loose): %f\n",perWords,perLetters,perLooseLetters);
    return perWords;
}

QVector<int> makeHistogram(QVector<double> data, double min, double max)
{
    int numBins=5000;
    
    
    QVector<int> bins(numBins);
    bins.fill(0);
    foreach (double d, data)
    {
        int bin = (numBins-1)*(d-min)/(max-min);
        bins[bin]++;
    }
    return bins;
}

double otsu(QVector<int> histogram, int total)
{
    int numBins=5000;
    int sum =0;
    for (int i = 1; i < numBins; ++i)
            sum += i * histogram[i];
    int sumB = 0;
    int wB = 0;
    int wF = 0;
    double mB;
    double mF;
    double max = 0.0;
    double between = 0.0;
    double threshold1 = 0.0;
    double threshold2 = 0.0;
    for (int i = 0; i < numBins; ++i) {
        wB += histogram[i];
        if (wB == 0)
            continue;
        wF = total - wB;
        if (wF == 0)
            break;
        sumB += i * histogram[i];
        mB = sumB / wB;
        mF = (sum - sumB) / wF;
        between = wB * wF * pow(mB - mF, 2);
        if ( between >= max ) {
            threshold1 = i;
            if ( between > max ) {
                threshold2 = i;
            }
            max = between;            
        }
    }
    return ( threshold1 + threshold2 ) / 2.0;
}

void Evaluate::evaluateScoreInfo(QString correctPath, QString incorrectPath)
{
    QVector<double> correctScores;
    std::ifstream correctFile(correctPath.toLocal8Bit().data());
    std::string line;
    
    while (getline(correctFile, line))
    {
        double score = std::atof(line.c_str());
        correctScores.append(score);
    }
    
    QVector<double> incorrectScores;
    std::ifstream incorrectFile(incorrectPath.toLocal8Bit().data());
    
    while (getline(incorrectFile, line))
    {
        double score = std::atof(line.c_str());
        incorrectScores.append(score);
    }
    
    double meanCorrect=0;
    foreach (double f, correctScores)
        meanCorrect+=f;
    meanCorrect/=correctScores.size();
    double sqrSumCorrect=0;
    foreach (double f, correctScores)
        sqrSumCorrect +=  pow(f-meanCorrect,2);
    double stdDevCorrect = pow(sqrSumCorrect/correctScores.size(),.5);
    
    double meanIncorrect=0;
    foreach (double f, incorrectScores)
        meanIncorrect+=f;
    meanIncorrect/=incorrectScores.size();
    double sqrSumIncorrect=0;
    foreach (double f, incorrectScores)
        sqrSumIncorrect +=  pow(f-meanIncorrect,2);
   double stdDevIncorrect = pow(sqrSumIncorrect/incorrectScores.size(),.5);
   
   printf("Mean correct score=   %f, std dev=%f\n",meanCorrect,stdDevCorrect);
   printf("Mean incorrect score= %f, std dev=%f\n",meanIncorrect,stdDevIncorrect);
   double max=0;
   double min=DOUBLE_POS_INFINITY;
   foreach (double d, correctScores+incorrectScores)
   {
       if (d>max)
           max=d;
       else if (d<min)
           min=d;
   }
   QVector<double> fullData = correctScores+incorrectScores;
   double otsur = (max-min)*otsu(makeHistogram(fullData,min,max),fullData.size())/5000 + min;
   printf("Otsu threshold=%f\n",otsur);
}

void Evaluate::writeScores(QString imgPath, QString correctPath, QString incorrectPath)
{
    assert(WRITE_SCORES);
    std::ofstream correctResults (correctPath.toLocal8Bit().data(),std::ofstream::app);
    std::ofstream incorrectResults (incorrectPath.toLocal8Bit().data(),std::ofstream::app);
    QImage testimg(imgPath);
    BImage bimg(testimg);
    BImage cleared = BoxCleaner::trimVerticleBoundaries(bimg);
    cleared = BoxCleaner::trimHorizontalBoundaries(cleared);
    cleared = BoxCleaner::removeVerticlePixelNoise(cleared);
    QVector<BPartition*> lines = WordSeparator::segmentLinesOfWords(cleared,40);
    for (int i=0; i<lines.size(); i++)
    {
        cleared.claimOwnership(lines[i],1);
    }
    cleared.saveOwners("./rainbow.ppm");
    for (int i=0; i<lines.size(); i++)
    {
        delete lines[i];
    }
    
    foreach (double d, GraphCut::correctScores)
    {
        correctResults << d << std::endl;
    }
    
    foreach (double d, GraphCut::incorrectScores)
    {
        incorrectResults << d << std::endl;
    }
    correctResults.close();
    incorrectResults.close();
}

