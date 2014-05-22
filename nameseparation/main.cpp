#include <iostream>
#include "wordprofile.h"
#include <QImage>
#include "wordseparator.h"
#include "boxcleaner.h"
#include "imageaverager.h"

using namespace std;

int main(int argc, char** argv)
{
    cout << "Starting" << endl;
    QImage testimg(argv[1]);
    QImage nonoise;
    bool test;
    
    
//    QVector<int> pics;
//    pics<< 1<<2<<3<<5<<8<<9<<10<<11<<12<<13<<14<<15<<16<<17<<18;
//    ImageAverager averager(120,44);
//    QImage avg = averager.averageImages("./descenders_edited/",pics);
//    avg.save("./average.pgm");
    
    
    
//    QImage trimmed = BoxCleaner::trimBoundaries(testimg);
//    nonoise= trimmed;//BoxCleaner::removePixelNoise(trimmed);
//    test = nonoise.save("./noise_removed.pgm");
//    if (!test)
//        cout << "ERROR trim!!" << endl;
    int vert_divide;
    QVector<QPoints> aboveBoundaryPoints;
    QVector<QPoints> belowBoundaryPoints;
    QImage lineremoved=BoxCleaner::clearLineAndCloseLetters(testimg,40,&vert_divide,&aboveBoundaryPoints,&belowBoundaryPoints);
//    lineremoved=BoxCleaner::clearLineAndCloseLetters(lineremoved,88,&vert_divide);
//    lineremoved=BoxCleaner::clearLineAndCloseLetters(lineremoved,131,&vert_divide);
//    lineremoved=BoxCleaner::clearLineAndCloseLetters(lineremoved,170,&vert_divide);
//    lineremoved=BoxCleaner::clearLineAndCloseLetters(lineremoved,210,&vert_divide);
    test = lineremoved.save("./lineremoved.pgm");
    if (!test)
        cout << "ERROR lineremove!!" << endl;
    
    QImage trimmed = BoxCleaner::trimBoundaries(lineremoved);
    nonoise= BoxCleaner::removePixelNoise(trimmed);
    test = nonoise.save("./noise_removed.pgm");
    if (!test)
        cout << "ERROR trim!!" << endl;
    
    QVector<QImage> cuts = WordSeparator::horzCutEntries(nonoise,vert_divide);
    cuts[0].save("./cut1_top.pgm");
    cuts[1].save("./cut2_bottom.pgm");
    
    
    
//    if (argc < 3 || argv[2][0]!='n')
//    {
//        QImage trimmed = BoxCleaner::trimBoundaries(testimg);
//        test = trimmed.save("./testimg_trimmed.pgm");
//        if (!test)
//           cout << "ERROR trim!!" << endl;
        
        
//        nonoise= BoxCleaner::removePixelNoise(trimmed);
//        test = nonoise.save("./output/full_img.pgm");
//        test = nonoise.save("./noise_removed.pgm");
//        if (!test)
//           cout << "ERROR nonoise!!" << endl;
//    }
//    else
//    {
//        nonoise=testimg;
//    }
    
//    if (argc < 3)
//    {
//        QVector<QImage> names = WordSeparator::cutNames(nonoise);
//        if (names.size()==3)
//        {
//            names[0].save("./output/last_name.pgm");
//            names[1].save("./output/first_name.pgm");
//            names[2].save("./output/middle_initail.pgm");
            
//            QVector<QImage> firstLetterGuesses = WordSeparator::recursiveCutWordToFirstLetter(names[1]);
//            firstLetterGuesses[0].save("./output/first_letter_guess1.pgm");
//            firstLetterGuesses[1].save("./output/first_letter_guess2.pgm");
//        }
//        else if (names.size()==2)
//        {
//            names[0].save("./output/last_name.pgm");
//            names[1].save("./output/first_name.pgm");
            
//            QVector<QImage> firstLetterGuesses = WordSeparator::recursiveCutWordToFirstLetter(names[1]);
//            firstLetterGuesses[0].save("./output/first_letter_guess1.pgm");
//            firstLetterGuesses[1].save("./output/first_letter_guess2.pgm");
//        }
//        else
//        {
//            names[0].save("./output/failed.pgm");
//        }
//    }
//    else
//    {
//        QVector<QImage> split = WordSeparator::minCut(nonoise);
//        //cout << "Save first" << endl;
//        test = split[0].save("./testimg_first_left.pgm");
//        // cout << "Save second" << endl;
//        test &= split[1].save("./testimg_1first_right.pgm");
//        if (!test)
//            cout << "ERROR split!!" << endl;
        
////        QVector<QImage> split2 = WordSeparator::minCut(split[0]);
////        test = split2[0].save("./testimg_second0_left.pgm");
////        test &= split2[1].save("./testimg_second0_right.pgm");
////        if (!test)
////            cout << "ERROR split!!" << endl;
//    }
    
    
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


