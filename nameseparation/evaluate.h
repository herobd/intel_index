#ifndef EVALUATE_H
#define EVALUATE_H

#include "bpartition.h"
#include "BPixelCollection.h"
#include <fstream>
//#include "bimage.h"
#include "boxcleaner.h"
#include "wordseparator.h"



class Evaluate
{
public:
    Evaluate();
    static double matchedScore(BPartition &part, BPixelCollection &img);
    static double matchedScore(BPixelCollection &img1, BPixelCollection &img2);
    static double verticleSegmentationTest(QString imgPath, QString gtDirPath);
    static double horizontalSegmentationTest(QString root);
};

#endif // EVALUATE_H
