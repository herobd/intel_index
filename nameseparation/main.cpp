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
    test = nonoise.save("./output/full_img.pgm");
    if (!test)
       cout << "ERROR nonoise!!" << endl;
    
    QVector<QImage> names = WordSeparator::cutNames(nonoise);
    if (names.size()==3)
    {
        names[0].save("./output/last_name.pgm");
        names[1].save("./output/first_name.pgm");
        names[2].save("./output/middle_initail.pgm");
        
        QVector<QImage> firstLetterGuesses = WordSeparator::recursiveCutWordToFirstLetter(names[1]);
        firstLetterGuesses[0].save("./output/first_letter_guess1.pgm");
        firstLetterGuesses[1].save("./output/first_letter_guess2.pgm");
    }
    else if (names.size()==2)
    {
        names[0].save("./output/last_name.pgm");
        names[1].save("./output/first_name.pgm");
        
        QVector<QImage> firstLetterGuesses = WordSeparator::recursiveCutWordToFirstLetter(names[1]);
        firstLetterGuesses[0].save("./output/first_letter_guess1.pgm");
        firstLetterGuesses[1].save("./output/first_letter_guess2.pgm");
    }
    else
    {
        names[0].save("./output/failed.pgm");
    }
//    QVector<QImage> split = WordSeparator::minCut(nonoise);
//    //cout << "Save first" << endl;
//    test = split[0].save("./testimg_first_left.pgm");
//    // cout << "Save second" << endl;
//    test &= split[1].save("./testimg_1first_right.pgm");
//    if (!test)
//        cout << "ERROR split!!" << endl;
    
//    QVector<QImage> split2 = WordSeparator::minCut(split[0]);
//    test = split2[0].save("./testimg_second0_left.pgm");
//    test &= split2[1].save("./testimg_second0_right.pgm");
//    if (!test)
//        cout << "ERROR split!!" << endl;
    
    //WordSeparator::recursiveCutWordToFirstLetter(split[1]);
    
//    QVector<QImage> split2 = WordSeparator::minCut(split[1]);
//    test = split2[0].save("./testimg_2second1_left.pgm");
//    //test &= split2[1].save("./testimg_second1_right.pgm");
//    if (!test)
//        cout << "ERROR split!!" << endl;
    
//    QVector<QImage> split3 = WordSeparator::minCut(split2[0]);
//    test = split3[0].save("./testimg_3third0_left.pgm");
//    //test &= split2[1].save("./testimg_second0_right.pgm");
//    if (!test)
//        cout << "ERROR split!!" << endl;
    
//    QVector<QImage> split4 = WordSeparator::minCut(split3[0]);
//    test = split4[0].save("./testimg_4fourth0_left.pgm");
//    //test &= split2[1].save("./testimg_second0_right.pgm");
//    if (!test)
//        cout << "ERROR split!!" << endl;
    
//    QVector<QImage> split5 = WordSeparator::minCut(split4[0]);
//    test = split5[0].save("./testimg_5fifth0_left.pgm");
//    //test &= split2[1].save("./testimg_second0_right.pgm");
//    if (!test)
//        cout << "ERROR split!!" << endl;
    /*Typically, the first cut (along a space) is <5000, often close to 0
      A second cut along a dash is around 20000
      A second cut between letters is >20000
      These last two are often relative to each other. Meaning if both are below 20000, the smaller is the dash
      */
    
    return 0;
}


