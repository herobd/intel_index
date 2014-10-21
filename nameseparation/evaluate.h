#ifndef EVALUATE_H
#define EVALUATE_H

#include "bpartition.h"
#include "BPixelCollection.h"
#include <fstream>
//#include "bimage.h"
#include "boxcleaner.h"
#include "wordseparator.h"
#include <string>
#include <dirent.h>



class Evaluate
{
public:
    Evaluate();
    static double matchedScore(BPartition &part, BPixelCollection &img);
    static double matchedScore(BPixelCollection &img1, BPixelCollection &img2);
    static double matchedScore(const BPartition &left, const BPartition &right, const QImage &coloredGT);
    static double verticleSegmentationTest(QString imgPath, QString gtDirPath, bool dumb=false);
    static double newHorizontalSegmentationTest(QString imgDirPath, QString gtDirPath);
    static double horizontalSegmentationTest(QString root);
    static void evaluateScoreInfo(QString correctPath, QString incorrectPath);
    static void writeScores(QString imgPath, QString correctPath, QString incorrectPath);
};

#endif // EVALUATE_H
