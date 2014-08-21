#include "pathstackmap.h"

PathStackMap::PathStackMap()
{
}

void PathStackMap::pop_back()
{
    paths[stack.back()].pop_back();
    sortedPathIndexes[stack.back()].clear();
    stack.pop_back();
}

void PathStackMap::push_back(pathAndScore toAdd)
{
    paths[toAdd.path.back()].push_back(toAdd);
    sortedPathIndexes[toAdd.path.back()].clear();
    stack.push_back(toAdd.path.back());
}

const pathAndScore& PathStackMap::operator[] (unsigned int endingPoint) const
{
    double lowestScore = DOUBLE_POS_INFINITY;
    int index=-1;
    for (int i=0; i<paths[endingPoint].size(); i++)
    {
        if (paths[endingPoint][i].score < lowestScore)
        {
            lowestScore = paths[endingPoint][i].score;
            index=i;
        }
    }
    if (index!=-1)
        return paths[endingPoint][index];
    
    pathAndScore ret;
    ret.score=-1;
    return ret;
}

//bool PathStackMap::scoreCompare(unsigned int a, unsigned int b)
//{
    
//}



const pathAndScore& PathStackMap::at(unsigned int endingPoint, unsigned int rank)
{
    if (sortedPathIndexes[endingPoint].size()==0)
    {
        for (unsigned int i=0; i<paths[endingPoint].size(); i++)
            sortedPathIndexes[endingPoint].append(i);
        
        qSort(sortedPathIndexes[endingPoint].begin(),sortedPathIndexes[endingPoint].end(),ScoreCompare(this,endingPoint));
    }
    
    return paths[endingPoint][sortedPathIndexes[endingPoint][rank]];
}
