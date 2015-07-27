#include "minmaxtracker.h"



void MinMaxTracker::track(Point cur, int& thisLabel)
{
    int totSize = minima->size()+maxima->size();
    onLocals(cur,BREAK_NEIGHBORHOOD_SIZE);
    if (minima->size()+maxima->size()>totSize)
    {
//                            cout << "min/max breaking at point ["<<inQuestion.x<<","<<inQuestion.y<<"]"<<endl;
        thisLabel=++curLabel;
        //rewrite all passed points
        for (const Point& p : before)//relabel from inQuestion up to current point
            graphemes.at<unsigned char>(p)=thisLabel;
    }
}

void MinMaxTracker::onLocals(const Point& toAdd, int neighborhoodSize)
{
    if (after.size()==neighborhoodSize)
    {
        after.pop_front();
    }
    if (inQuestion.x!=-1)
    {
        after.push_back(inQuestion);
    }
    if (before.size()==neighborhoodSize)
    {
        inQuestion=before.front();
        before.pop_front();
    }
    before.push_back(toAdd);
    
    if (after.size()==neighborhoodSize)
    {
        bool candidateMax=true;
        bool candidateMin=true;
        
        
        
        auto prev = after.begin();
        auto cur = after.begin(); cur++;
        for (; cur!=after.end(); prev++,cur++)
        {
            if (prev->y > cur->y) candidateMin=false;
            if (prev->y < cur->y) candidateMax=false;
        }
        if (after.back().y > inQuestion.y) candidateMin=false;
        if (after.back().y < inQuestion.y) candidateMax=false;
        
        prev = before.begin();
        cur = before.begin(); cur++;
        for (; cur!=before.end(); prev++,cur++)
        {
            if (prev->y < cur->y) candidateMin=false;
            if (prev->y > cur->y) candidateMax=false;
        }
        if (inQuestion.y < before.front().y) candidateMin=false;
        if (inQuestion.y > before.front().y) candidateMax=false;
        

        auto check = after.rbegin();
        while (candidateMin && (*check).y==inQuestion.y && check!=after.rend())
        {
            candidateMin = find(minima->rbegin(),minima->rend(),(*check)) == minima->rend();
            check++;
        }
        check = after.rbegin();
        while (candidateMax && (*check).y==inQuestion.y && check!=after.rend())
        {
            candidateMax = find(maxima->rbegin(),maxima->rend(),(*check)) == maxima->rend();
            check++;
        }
        
        if (candidateMin && 
                (std::max(inQuestion.y-after.front().y,inQuestion.y-before.back().y)>neighborhoodSize/4.0) && 
                (std::min(inQuestion.y-after.front().y,inQuestion.y-before.back().y)>0))
        {
            minima->push_back(inQuestion);
        }
        
        if (candidateMax && 
                (std::max(after.front().y-inQuestion.y,before.back().y-inQuestion.y)>neighborhoodSize/4.0) && 
                (std::min(after.front().y-inQuestion.y,before.back().y-inQuestion.y)>0))
        {
            maxima->push_back(inQuestion);
        }
    }
}
