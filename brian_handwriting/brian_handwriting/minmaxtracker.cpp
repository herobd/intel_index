#include "minmaxtracker.h"



void MinMaxTracker::track(Point cur, int& thisLabel)
{
    int totSize = minima->size()+maxima->size();
    onLocals(cur);
    if (minima->size()+maxima->size()>totSize)
    {
//                            cout << "min/max breaking at point ["<<inQuestion.x<<","<<inQuestion.y<<"]"<<endl;
        int prevLabel=thisLabel;
        thisLabel=++curLabel;
        //rewrite all passed points
        for (const Point& p : before)//relabel from inQuestion up to current point
        {
            graphemes.at<unsigned char>(p)=thisLabel;
            for (int direction=0; direction<8; direction+=2)
            {
                int x = p.x+xDelta(direction);
                int y = p.y+yDelta(direction);
                if (x<0 || y<0 || x>=graphemes.cols || y>=graphemes.rows)
                    continue;
                if (graphemes.at<unsigned char>(y,x)==prevLabel)
                    graphemes.at<unsigned char>(y,x)=thisLabel;
            }
                
        }
    }
}

void MinMaxTracker::onLocals(const Point& toAdd)
{
    if (after.size()==neighborhood)
    {
        after.pop_front();
    }
    if (inQuestion.x!=-1)
    {
        after.push_back(inQuestion);
    }
    if (before.size()==neighborhood)
    {
        inQuestion=before.front();
        before.pop_front();
    }
    before.push_back(toAdd);
    
    if (after.size()==neighborhood)
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
        while ((candidateMin||candidateMax) && (*check).y==inQuestion.y && check!=after.rend())
        {
            candidateMin = candidateMin&&find(minima->rbegin(),minima->rend(),(*check)) == minima->rend();
            candidateMax = candidateMax&&find(maxima->rbegin(),maxima->rend(),(*check)) == maxima->rend();
            check++;
        }
        
        auto checkBefore = before.begin();
        while ((candidateMin||candidateMax) && (*checkBefore).y==inQuestion.y && checkBefore!=before.end())
        {
            candidateMin = candidateMin&&find(minima->rbegin(),minima->rend(),(*checkBefore)) == minima->rend();
            candidateMax = candidateMax&&find(maxima->rbegin(),maxima->rend(),(*checkBefore)) == maxima->rend();
            checkBefore++;
        }
        
        if (candidateMin && 
                find(minima->rbegin(),minima->rend(),inQuestion) == minima->rend() &&
                (std::max(inQuestion.y-after.front().y,inQuestion.y-before.back().y)>neighborhood/4.0) && 
                (std::min(inQuestion.y-after.front().y,inQuestion.y-before.back().y)>0))
        {
            minima->push_back(inQuestion);
        }
        
        if (candidateMax && 
                find(maxima->rbegin(),maxima->rend(),inQuestion) == maxima->rend() &&
                (std::max(after.front().y-inQuestion.y,before.back().y-inQuestion.y)>neighborhood/4.0) && 
                (std::min(after.front().y-inQuestion.y,before.back().y-inQuestion.y)>0))
        {
            maxima->push_back(inQuestion);
        }
    }
}
