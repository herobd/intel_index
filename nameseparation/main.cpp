#include <iostream>
#include "wordprofile.h"
#include <QImage>
#include "wordseparator.h"
#include "boxcleaner.h"
#include "imageaverager.h"
#include "bimage.h"

using namespace std;

int main(int argc, char** argv)
{
    cout << "Starting..." << endl;
    QImage testimg(argv[1]);
    bool test;

    BImage bimg(testimg);
    BPartition* all = bimg.getFullPartition();
    QVector<BPartition*> cuts = WordSeparator::minCut(*all);
    
    bimg.save("./test.ppm");
    return 0;
}


