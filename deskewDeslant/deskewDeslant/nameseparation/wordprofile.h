#ifndef WORDPROFILE_H
#define WORDPROFILE_H

#include <QImage>
#include <stdio.h>
#include <QVector>

class WordProfile
{
public:
    WordProfile();
    WordProfile(const QImage &from, bool bianized, int on);
    ~WordProfile();
    QVector<int> getLocalMins();
    double similarity(const WordProfile &compareTo);
    void print();
    int getWidth();
    int getValue(int i);
private:
    int* profile;
    int width;
    QVector<int> minima;
};

#endif // WORDPROFILE_H
