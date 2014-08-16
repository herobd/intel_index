#ifndef PATHSTACKMAP_H
#define PATHSTACKMAP_H

#include <QVector>
#include <QPoint>
#include <QMap>
#include "Constants.h"

struct pathAndScore
{
    QVector<unsigned int> path;
    double score;
};

class PathStackMap
{
public:
    PathStackMap();
    void pop_back();
    void push_back(pathAndScore toAdd);
    const pathAndScore& operator[] (unsigned int endingPoint) const;
    unsigned int size() {return stack.size();}
    
private:
    QMap<unsigned int, QVector<pathAndScore> > paths;
    QVector<unsigned int> stack;
};

#endif // PATHSTACKMAP_H
