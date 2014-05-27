#include <iostream>
#include "wordprofile.h"
#include <QImage>
#include "wordseparator.h"
#include "boxcleaner.h"
#include "imageaverager.h"

using namespace std;

int main(int argc, char** argv)
{
    cout << "Starting..." << endl;
    QImage testimg(argv[1]);
    QImage nonoise;
    bool test;
    
    
//    QVector<int> pics;
//    pics<< 1<<2<<3<<4<<5<<6<<7<<8<<9<<10<<11<<12<<13<<14<<15<<16<<17<<18<<19;
//    ImageAverager averager(120,42);
//    QImage avg = averager.averageImages("./descenders_justimage/",pics);
//    avg.save("./average.pgm");
    //Use guassian filter to smooth out prob map
    
    
    
//    QImage trimmed = BoxCleaner::trimBoundaries(testimg);
//    nonoise= trimmed;//BoxCleaner::removePixelNoise(trimmed);
//    test = nonoise.save("./noise_removed.pgm");
//    if (!test)
//        cout << "ERROR trim!!" << endl;
    int vert_divide;
    QVector<QPoint> crossPoints;
    QImage lineremoved=BoxCleaner::clearLineAndCloseLetters(testimg,40,&vert_divide,&crossPoints);
//    lineremoved=BoxCleaner::clearLineAndCloseLetters(lineremoved,88,&vert_divide);
//    lineremoved=BoxCleaner::clearLineAndCloseLetters(lineremoved,131,&vert_divide);
//    lineremoved=BoxCleaner::clearLineAndCloseLetters(lineremoved,170,&vert_divide);
//    lineremoved=BoxCleaner::clearLineAndCloseLetters(lineremoved,210,&vert_divide);
//    for (int i=0; i<crossPoints.size(); i++)
//    {
//        lineremoved.setPixel(crossPoints[i],200);
//    }
    
    test = lineremoved.save("./lineremoved.pgm");
    if (!test)
        cout << "ERROR lineremove!!" << endl;
    
    QImage trimmed = BoxCleaner::trimVerticleBoundaries(lineremoved);
    test = trimmed.save("./trimmed.pgm");
    nonoise= BoxCleaner::removeVerticlePixelNoise(trimmed);
    test &= nonoise.save("./noise_removed.pgm");
    if (!test)
        cout << "ERROR trim!!" << endl;
    
    QImage average_desc("./average_desc.pgm");
    QVector<QVector<double> > probMap = ImageAverager::produceProbabilityMap(average_desc);
    
    QVector<QImage> cuts = WordSeparator::horzCutEntries(trimmed,vert_divide,crossPoints,probMap);
    cuts[0].save("./output/cut1a_top.pgm");
    cuts[1].save("./output/cut1b_bottom.pgm");
    
//    QString pre = "./output/cut";
//    QString posta = "a_top.pgm";
//    QString postb = "b_bottom.pgm";
    
//    int lastCutSize = cuts[1].height();
//    int lastCutDif = nonoise.height() - cuts[1].height();
    
//    for (int i=2; i<=50; i++)
//    {
//        lineremoved=BoxCleaner::clearLineAndCloseLetters(cuts[1],80-lastCutDif,&vert_divide,&crossPoints);
//        cuts = WordSeparator::horzCutEntries(lineremoved,vert_divide,crossPoints,probMap);
//        QString num;
//        num.setNum(i);
//        QString savepatha;
//        savepatha += pre;
//        savepatha += num;
//        savepatha += posta;
//        QString savepathb;
//        savepathb += pre;
//        savepathb += num;
//        savepathb += postb;
//        cuts[0].save(savepatha);
//        cuts[1].save(savepathb);
        
//        lastCutDif = lastCutSize - cuts[1].height();
//        lastCutSize = cuts[1].height();
//    }
    
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


