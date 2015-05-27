#ifndef SPATIALAVERAGESPOTTER_H
#define SPATIALAVERAGESPOTTER_H

#include "opencv2/core/core.hpp"
#include "codebook.h"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <tuple>
#include "bagspotter.h"

using namespace std;
using namespace cv;

class SpatialAverageSpotter : BagSpotter
{
public:
//    SpatialAverageSpotter(const Codebook *codebook);
    bool train(string dirPath);
//    bool testSpotting(string dirPath);
//    void produceHeatMap(string fileName);
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
    
    vector<Mat> featureAverages;
    vector<Mat> adjustedTrainingImages;
    float penalty;
    
//    map< int, map< int, vector<int> > >* buildFeatureMap(Mat img);
    double detect(map< int, map< int, vector<int> > >* fm, Point2i corner);
    float score(map< int, map< int, vector<int> > >* fm, Point2i corner, double scaling);
//    void putPointsOn(Mat &img, map< int, map< int, vector<int> > >* fm, Point2i corner);
    Point2f findCenterOfMass(Mat &img);
    void maximizeAlignment(vector<vector<tuple<int,Point2f> > > &features);
    float getUsingOffset(Mat &featureAverage, double xOff, double yOff);
    void addUsingOffset(Mat &featureAverage, int xOff, int yOff, float add);
    void guassColorIn(const vector<tuple<int,Point2f> > &feature);
    void guassColorOut(const vector<tuple<int,Point2f> > &feature);
//    void detectKeypoints(Mat &img, vector<KeyPoint> &keypoints, Mat &desc);
    
    void showAverages();
    
    enum ExemplarTransform {STAY, LEFT, RIGHT, UP, DOWN, SHRINK, GROW, LEFT_SHRINK, RIGHT_SHRINK, UP_SHRINK, DOWN_SHRINK, LEFT_GROW, RIGHT_GROW, UP_GROW, DOWN_GROW, HIGHTEN, SHORTEN, FATTEN, SLIM};
};

#endif // SPATIALAVERAGESPOTTER_H
