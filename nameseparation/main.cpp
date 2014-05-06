#include <iostream>
#include "wordprofile.h"
#include <QImage>
#include "wordseparator.h"

using namespace std;

int main(int argc, char** argv)
{
    cout << "Starting" << endl;
    QImage testimg(argv[1]);
    
    QImage trimmed = WordSeparator::trimBoundaries(testimg);
    bool test = trimmed.save("./testimg_trimmed.pgm");
    if (!test)
       cout << "ERROR trim!!" << endl;
    
    QImage nonoise = WordSeparator::removePixelNoise(trimmed);
    test = nonoise.save("./testimg_noiseRemoved.pgm");
    if (!test)
       cout << "ERROR nonoise!!" << endl;
    
    
    QVector<QImage> split = WordSeparator::minCut(nonoise);
    cout << "Save first" << endl;
    test = split[0].save("./testimg_first_left.pgm");
     cout << "Save second" << endl;
    test &= split[1].save("./testimg_first_right.pgm");
    if (!test)
        cout << "ERROR split!!" << endl;
    
    QVector<QImage> split2 = WordSeparator::minCut(split[0]);
    test = split2[0].save("./testimg_second0_left.pgm");
    test &= split2[1].save("./testimg_second0_right.pgm");
    if (!test)
        cout << "ERROR split!!" << endl;
    
    split2 = WordSeparator::minCut(split[1]);
    test = split2[0].save("./testimg_second1_left.pgm");
    test &= split2[1].save("./testimg_second1_right.pgm");
    if (!test)
        cout << "ERROR split!!" << endl;
    
    return 0;
}


