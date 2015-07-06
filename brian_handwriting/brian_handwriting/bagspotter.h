#ifndef BAGSPOTTER_H
#define BAGSPOTTER_H

#include "opencv2/core/core.hpp"
#include "codebook.h"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <functional>

using namespace std;
using namespace cv;

class BagSpotter
{
public:
    BagSpotter(const Codebook *codebook);
    virtual bool train(string dirPath);
    virtual bool testSpotting(string dirPath);
    virtual void produceHeatMap(string fileName);
private:
    vector<double> learned;
    vector<double> learned_tfidf;
    Feature2D* detector;
    
protected:
    const Codebook * codebook;
    
    int windowWidth;
    int windowHeight;
    
    int stepSize;
    
    vector<Vec3b> colorTable;
    
    map< int, map< int, vector<int> > >* buildFeatureMap(Mat img);
    void forAllFeaturesInWindow(int windowWidth, int windowHeight, map< int, map< int, vector<int> > >* fm, Point2i corner, function<void(int,int,int)> doThis);
    virtual double detect(map< int, map< int, vector<int> > >* fm, Point2i corner);
    void putPointsOn(Mat &img, map< int, map< int, vector<int> > >* fm, Point2i corner);
    void detectKeypoints(Mat &img, vector<KeyPoint> &keypoints, Mat &desc);
    void color(Mat &heatMap, double score, double max, int midI, int midJ);
};

#endif // BAGSPOTTER_H
