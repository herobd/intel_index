#ifndef LIANG_H
#define LIANG_H

///A synthesised word approach to word retrieval in handwritten documents
/// Y. Liang, M.C. Fairhurst, R.M. Guest
///Pattern Recognition 45 (2012) 4225-4236

#include <vector>
#include <list>
#include <map>
#include <fstream>
#include <functional>
#include <regex>
#include <stdlib.h>
#include <algorithm>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "grapheme.h"
#include "mog.h"
#include "minmaxtracker.h"
#include "defines.h"
#include "omp.h"

#define MARKED_POINT 254
#define UNMARKED_POINT 255
#define JUNCTION_POINT 253

#define BAD_SCORE -9999
#define MAX_ITER_CHAR_SEG 100

#define SMALL_LOCAL_NEIGHBORHOOD 4
#define MED_LOCAL_NEIGHBORHOOD 6
#define BIG_LOCAL_NEIGHBORHOOD 8

#define SCRUB_THRESH 30

#define IMAGE_THRESH 150

using namespace std;
using namespace cv;

struct PointComp {
  bool operator() (const Point& lhs, const Point& rhs) const
  {
      if (lhs.x<rhs.x) return true;
      else if (lhs.x > rhs.x) return false;
      else return lhs.y<rhs.y;
  }
};

class Liang
{
public:
    Liang();
    
    double score(string query, const Mat &img);
    void trainCharacterModels(string imgDirPath, string imgNamePattern, string imgExt, const vector<pair<int, string> > &examples);
    void trainCharacterModels(string imgDirPath, string imgNamePattern, string imgExt, string annotationsPath);
    void saveCharacterModels(string filePath);
    void loadCharacterModels(string filePath);
    
    void showCharacterModels(string imgDirPath, string imgNamePattern, string imgExt, const vector< pair<int,string> >& examples, string modelMOG);
    bool unittest();
    
private:
    map<char, vector<float> > graphemeSpectrums;
    map<char, float> charWidthAvgs;
    map<char, float> unitCharWidthStdDevs;
    MOG mog;
    
    bool trained;
   
    vector<Grapheme*> extractGraphemes(const Mat &skel, list<Point>* localMaxs, list<Point>* localMins, int *centerLine); 
    vector<list<const Grapheme*> > learnCharacterSegmentation(string query, const vector<Grapheme *> &graphemes, const list<Point>& localMaxs, const list<Point>& localMins, int centerLine, const Mat &img, map<char, list<const Grapheme*> >* charGraphemes=NULL, map<char, list<int> >* accumWidths=NULL, map<char, list<double> > *unitCharWidths=NULL, map<char, int>* charCounts=NULL, omp_lock_t *writelock=NULL);
    void thinning(const cv::Mat& src, cv::Mat& dst);
    void scrubCC(Mat& skel, int xStart, int yStart);
    void cleanNoiseFromEdges(Mat &skel);
    Mat breakSegments(const Mat& skel, list<Point>* maxima=NULL, list<Point>* minima=NULL, int *centerLine=NULL);
    void prune(list<Point>* points, bool checkBelow, const Mat& skel);
    list<int> repairLoops(Mat& graphemes);
    void exploreChain(Point cur, int curLabel, list<int> chain, Mat &graphemes, Mat& visited, map<int, int> &mergeTable, int &juncLabel, map<Point, int,PointComp> &juncMap, int prevDir=-1);
    void relabel(int from, int to, list<int>& chain, map<int, int> &mergeTable);
    int endMerge(int start, const map<int, int> &mergeTable);
    
//    void onLocals(Point &toAdd, list<Point>& after, list<Point>& before, Point& inQuestion, int neighborhoodSize, list<Point>& minima, list<Point>& maxima);
    bool hasAscender(const string& word);
    bool hasDescender(const string& word);
    bool isAscender(char c);
    bool isDescender(char c);
    void findBaselines(bool hasAscender, bool hasDescender, int* upperBaseline, int* lowerBaseline, const list<Point>& localMins, const list<Point>& localMaxs, int centerLine, const Mat &img);
    void findMinMaxInBaselines(const list<const Grapheme*>& graphemes, int upperBaseline, int lowerBaseline, int& min, int& max);
    void adjustCharWidths(const vector<list<const Grapheme*> >& characters, const string& query, int upperBaseline, int lowerBaseline, int leftX, vector<int>& charWidths);
    bool directionForbidden(int direction, int prevDir);
    void lookAheadJunctions(int x, int y, int prevDirection, Point cur, const list<int>& chain, const Mat& graphemes, int &doRelabel, int &bestLoc);
    void countJunction(int fromX, int fromY, int prevDir, bool first, bool last, int& firstLabel, const Mat& graphemes, const map<int, int>& mergeTable, map<int,int>& counts, int& prevLabel, Mat &mark);
    bool relabelStrech(int label, Point from, Mat& ret, MinMaxTracker& tracker, MinMaxTracker &tracker2, MinMaxTracker &tracker3);
//    void findAllMaxMin(const Mat& skel, list<Point>& localMins, list<Point>& localMaxs);
    bool checkCorner(int fromX, int fromY, int prevDir, const Mat& skel);
    bool branchPoint(int cur_x, int cur_y, Mat& ret, list<Point>& startingPointQueue, int& curLabel);
    void writeGraphemes(const Mat& img);
    void writeSegmentation(const vector<list<const Grapheme*> >& segmentation);
    void showMOG(const map<char, list<const Grapheme*> >& charGraphemes);
    
};

#endif // LIANG_H
