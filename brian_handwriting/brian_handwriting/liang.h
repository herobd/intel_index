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

#define MARKED_POINT 254
#define UNMARKED_POINT 255
#define JUNCTION_POINT 253

#define BAD_SCORE 9999
#define MAX_ITER_CHAR_SEG 100

#define SMALL_LOCAL_NEIGHBORHOOD 4
#define MED_LOCAL_NEIGHBORHOOD 6
#define BIG_LOCAL_NEIGHBORHOOD 8

#define SCRUB_THRESH 30

using namespace std;
using namespace cv;

class Liang
{
public:
    Liang();
    
    double score(string query, const Mat &img);
    void trainCharacterModels(string imgDirPath, string imgNamePattern, string imgExt, string annotationsPath);
    void saveCharacterModels(string filePath);
    void loadCharacterModels(string filePath);
    
    bool unittest();
    
private:
    map<char, vector<float> > graphemeSpectrums;
    map<char, float> charWidthAvgs;
    map<char, float> charWidthStdDevs;
    MOG mog;
    
    bool trained;
   
    vector<Grapheme*> extractGraphemes(const Mat &skel, list<Point>* localMaxs, list<Point>* localMins); 
    vector<list<const Grapheme*> > learnCharacterSegmentation(string query, vector<Grapheme*> graphemes, const list<Point>& localMaxs, const list<Point>& localMins, map<char, list<const Grapheme*> >* charGraphemes=NULL, map<char, list<int> >* accumWidths=NULL, map<char, int>* charCounts=NULL );
    void thinning(const cv::Mat& src, cv::Mat& dst);
    void scrubCC(Mat& skel, int xStart, int yStart);
    void cleanNoiseFromEdges(Mat &skel);
    Mat breakSegments(const Mat& skel, list<Point>* maxima=NULL, list<Point>* minima=NULL);
    list<int> repairLoops(Mat& graphemes);
    void exploreChain(Point cur, int curLabel, list<int> chain, Mat &graphemes, Mat& visited, map<int, int> &mergeTable, int prevDir=-1);
    void relabel(int from, int to, list<int>& chain, map<int, int> &mergeTable);
    int endMerge(int start, const map<int, int> &mergeTable);
    
//    void onLocals(Point &toAdd, list<Point>& after, list<Point>& before, Point& inQuestion, int neighborhoodSize, list<Point>& minima, list<Point>& maxima);
    bool hasAscender(const string& word);
    bool hasDescender(const string& word);
    bool isAscender(char c);
    bool isDescender(char c);
    void findBaselines(bool hasAscender, bool hasDescender, int* upperBaseline, int* lowerBaseline, const list<Point>& localMins, const list<Point>& localMaxs);
    bool directionForbidden(int direction, int prevDir);
    void lookAheadJunctions(int x, int y, int prevDirection, Point cur, const list<int>& chain, const Mat& graphemes, int &doRelabel, int &bestLoc);
    void countJunction(int fromX, int fromY, int prevDir, bool first, bool last, int& firstLabel, const Mat& graphemes, const map<int, int>& mergeTable, map<int,int>& counts, int& prevLabel);
    void relabelStrech(int label, Point from, Mat& ret, MinMaxTracker& tracker, MinMaxTracker &tracker2, MinMaxTracker &tracker3);
//    void findAllMaxMin(const Mat& skel, list<Point>& localMins, list<Point>& localMaxs);
    bool checkCorner(int fromX, int fromY, int prevDir, const Mat& skel);
    bool branchPoint(int cur_x, int cur_y, Mat& ret, list<Point>& startingPointQueue, int& curLabel);
    void writeGraphemes(const Mat& img);
    
};

#endif // LIANG_H
