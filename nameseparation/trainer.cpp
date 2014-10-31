#include "trainer.h"

static void Trainer::trainTwoWordSeparation(QString dir)
{
    //read in dir, anal all files
    QVector<BPartition*> segmentation = WordSeparator::recursiveHorizontalCutTwoWordsTraining(clean);
    //with a GT, this possibly could be automated.
}
