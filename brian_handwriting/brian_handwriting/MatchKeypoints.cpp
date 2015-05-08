#include <stdio.h>
#include <iostream>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include <assert.h>
#include "codebook.h"

using namespace cv;



/** @function main */
int matchKeypoints( int argc, char** argv )
{
//  if( argc != 3 )
//  { readme(); return -1; }
  cv::initModule_nonfree();

  Mat img_1 = imread( argv[1], CV_LOAD_IMAGE_GRAYSCALE );
  Mat img_2 = imread( argv[2], CV_LOAD_IMAGE_GRAYSCALE );
  Codebook codebook;
  //codebook.readInCSV(string(argv[3]));

  if( !img_1.data || !img_2.data )
  { std::cout<< " --(!) Error reading images " << std::endl; return -1; }

  //-- Step 1: Detect the keypoints using SURF Detector
  int minHessian = 800;

  //SurfFeatureDetector detector( minHessian);
  SURF* detector = new SURF(minHessian,4,2,true,true);

  std::vector<KeyPoint> keypoints_1, keypoints_2;

  assert(img_1.size[0]>0 && img_1.size[1]>0 && img_2.size[0]>0 && img_2.size[1]>0);
  
  (*detector)( img_1, Mat(), keypoints_1 );
  (*detector)( img_2, Mat(), keypoints_2 );
  
  Mat img_keypoints_1; Mat img_keypoints_2;
  drawKeypoints( img_1, keypoints_1, img_keypoints_1, Scalar::all(-1), DrawMatchesFlags::DEFAULT );
  drawKeypoints( img_2, keypoints_2, img_keypoints_2, Scalar::all(-1), DrawMatchesFlags::DEFAULT );
  
    //-- Show detected (drawn) keypoints
    imshow("Keypoints 1", img_keypoints_1 );
    imshow("Keypoints 2", img_keypoints_2 );
    waitKey(0);

  //-- Step 2: Calculate descriptors (feature vectors)
    //SurfDescriptorExtractor extractor;
  
    Mat descriptors_1, descriptors_2;
  
    detector->compute( img_1, keypoints_1, descriptors_1 );
    detector->compute( img_2, keypoints_2, descriptors_2 );
  
    //-- Step 3: Matching descriptor vectors using FLANN matcher
    FlannBasedMatcher matcher;
    std::vector< std::vector< DMatch > > matches;
    matcher.knnMatch( descriptors_1, descriptors_2, matches, 10 );
  
    double max_dist = 0; double min_dist = 100;
  
    //-- Quick calculation of max and min distances between keypoints
    for( int i = 0; i < matches.size(); i++ )
    {
        for (int j=0; j < matches[i].size(); j++)
        {
            double dist = matches[i][j].distance;
            if( dist < min_dist ) min_dist = dist;
            if( dist > max_dist ) max_dist = dist;
        }
    }
  
    printf("-- Max dist : %f \n", max_dist );
    printf("-- Min dist : %f \n", min_dist );
  
    //-- Draw only "good" matches (i.e. whose distance is less than 2*min_dist,
    //-- or a small arbitary value ( 0.02 ) in the event that min_dist is very
    //-- small)
    //-- PS.- radiusMatch can also be used here.
    std::vector< DMatch > good_matches;
  
    for( int i = 0; i < matches.size(); i++ )
    {
        for (int j=0; j < matches[i].size(); j++)
            //if( matches[i][j].distance <= max(2*min_dist, 0.02) )
            if( matches[i][j].distance <= max((max_dist-min_dist)/4.0 + min_dist, 0.02) )
            { good_matches.push_back( matches[i][j]); }
            else
                printf("discard(%d,%d)\n",i,j);
    }
  
    //-- Draw only "good" matches
    Mat img_matches;
    drawMatches( img_1, keypoints_1, img_2, keypoints_2,
                 good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
                 vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
  
    //-- Show detected matches
    imshow( ".... Matches", img_matches );
  
    for( int i = 0; i < (int)good_matches.size(); i++ )
    { printf( "-- .... Match [%d] Keypoint 1: %d  -- Keypoint 2: %d  \n", i, good_matches[i].queryIdx, good_matches[i].trainIdx ); }
  
    waitKey(0);
    
//    vector<Point2f> corners;
//      double qualityLevel = 0.01;
//      double minDistance = 10;
//      int blockSize = 3;
//      bool useHarrisDetector = false;
//      double k = 0.04;
//      int maxCorners = 23;
//      int maxTrackbar = 100;
//    goodFeaturesToTrack( src_gray,
//                   corners,
//                   maxCorners,
//                   qualityLevel,
//                   minDistance,
//                   Mat(),
//                   blockSize,
//                   useHarrisDetector,
//                   k );
    
  return 0;
  }

  /** @function readme */
//  void readme2()
//  { std::cout << " Usage: ./SURF_detector <img1> <img2>" << std::endl; }

