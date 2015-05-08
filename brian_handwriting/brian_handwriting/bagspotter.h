#ifndef BAGSPOTTER_H
#define BAGSPOTTER_H

#include "opencv2/core/core.hpp"
#include "codebook.h"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>

using namespace std;
using namespace cv;

class BagSpotter
{
public:
    BagSpotter(const Codebook *codebook);
    bool train(string dirPath);
    bool testSpotting(string dirPath);
    void produceHeatMap(string fileName);
    double detect(Mat img);
private:
    const Codebook * codebook;
    vector<double> learned;
    vector<double> learned_tfidf;
    int windowWidth;
    int windowHeight;
    Feature2D* detector;
    int stepSize;
    
    vector<Vec3b> colorTable;
    
    map< int, map< int, vector<int> > >* buildFeatureMap(Mat img);
    double detect(map< int, map< int, vector<int> > >* fm, Point2i corner);
    void putPointsOn(Mat &img, map< int, map< int, vector<int> > >* fm, Point2i corner);
    void detectKeypoints(Mat &img, vector<KeyPoint> &keypoints, Mat &desc);
    void color(Mat &heatMap, double score, double max, int midI, int midJ);
};

#endif // BAGSPOTTER_H
