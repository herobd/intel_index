#ifndef MINMAXTRACKER_H
#define MINMAXTRACKER_H

#include <list>
#include "opencv2/core/core.hpp"

#define BREAK_NEIGHBORHOOD_SIZE 4

using namespace std;
using namespace cv;

class MinMaxTracker
{
public:
    MinMaxTracker(list<Point>* minima, list<Point>* maxima, int& curLabel, Mat& graphemes):
        minima(minima), maxima(maxima), curLabel(curLabel), graphemes(graphemes), inQuestion(Point(-1,-1))
    {}
    void track(Point cur, int& thisLabel);
    void onLocals(const Point& toAdd, int neighborhoodSize);
    
private:
    list<Point> after;
    list<Point> before;
    Point inQuestion;
    
    list<Point> *minima, *maxima;
    int& curLabel;
    Mat& graphemes;
};

#endif // MINMAXTRACKER_H
