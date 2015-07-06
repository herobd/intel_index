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
    unsigned int size(unsigned int endingPoint){return paths[endingPoint].size();}
    
    const pathAndScore& at(unsigned int endingPoint, unsigned int rank);
    
private:
    QMap<unsigned int, QVector<pathAndScore> > paths;
    QMap<unsigned int, QVector<unsigned int> > sortedPathIndexes;
    QVector<unsigned int> stack;
    
    struct ScoreCompare {
        PathStackMap* owner;
        unsigned int endingPoint;
        ScoreCompare(PathStackMap* o, unsigned int e):owner(o), endingPoint(e) {}
        bool operator () (unsigned int a, unsigned int b)
        {
            return owner->paths[endingPoint][a].score < owner->paths[endingPoint][b].score;
        }
    };
};

#endif // PATHSTACKMAP_H
