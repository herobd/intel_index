#include <stdio.h>
#include <iostream>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include <assert.h>
#include <CollabKeypoints.h>

using namespace cv;

void readme();

/** @function main */
int collabKeypoints( int argc, char** argv )
{
    if( argc < 2 )
    { readme(); return -1; }
    cv::initModule_nonfree();
    
    int minHessian = 400;
    SURF* detector = new SURF(minHessian,5,3,true,true);
    
    //HOGDescriptor hog;
    
    std::vector< std::vector<KeyPoint> > keypoints(argc-2);
    std::vector<KeyPoint> aggregate_keypoints;
    
    std::vector<Mat> images;
    Mat aggregate_descriptors;
    for (int i=1; i<argc-1; i++)
    {
        images.push_back(imread( argv[i], CV_LOAD_IMAGE_GRAYSCALE ));
        
        if( !images[i-1].data)
        { std::cout<< " --(!) Error reading image " << i << std::endl; return -1; }
        assert(images[i-1].size[0]>0 && images[i-1].size[1]>0);
        
        Mat sem_descriptors;
        (*detector)( images[i-1], Mat(), keypoints[i-1] );
        
        int minX=images[i-1].size[1]/3;
        int maxX=2*images[i-1].size[1]/3;
        int minY=images[i-1].size[0]/3;
        int maxY=2*images[i-1].size[0]/3;
        for (int keyIdx=0; keyIdx < keypoints[i-1].size(); keyIdx++)
        {
            if (keypoints[i-1][keyIdx].pt.x < minX ||
                    keypoints[i-1][keyIdx].pt.x > maxX ||
                    keypoints[i-1][keyIdx].pt.y < minY ||
                    keypoints[i-1][keyIdx].pt.y > maxY)
            {
                keypoints[i-1].erase (keypoints[i-1].begin()+keyIdx);
            }
        }
        
        detector->compute( images[i-1], keypoints[i-1], sem_descriptors );
//        std::cout << "desc " << sem_descriptors << std::endl;
//        for (int j=0; j<sem_descriptors.size[0]; j++)
//        {
//            aggregate_descriptors.push_back(sem_descriptors.at<Mat>(j));
//        }
        if (aggregate_descriptors.size[0] == 0)
            aggregate_descriptors = sem_descriptors;
        else
            vconcat(aggregate_descriptors,sem_descriptors,aggregate_descriptors);
    }
    Mat test_img = imread( argv[argc-1], CV_LOAD_IMAGE_GRAYSCALE );
    std::vector<KeyPoint> test_keypoints;
    (*detector)( test_img, Mat(), test_keypoints );
    Mat test_descriptors;
    detector->compute( test_img, test_keypoints, test_descriptors );
    
    std::cout << "Total in aggregate: " << aggregate_descriptors.size[0] << std::endl;
    Mat best_aggregate_descriptors;
    TermCriteria crit(0,10,.99);
    Mat aggregate_idx;
    int k =20;
    kmeans(aggregate_descriptors,k,aggregate_idx,crit,5,KMEANS_RANDOM_CENTERS,best_aggregate_descriptors);
    
    
    
    //Get consunsus
    FlannBasedMatcher matcher;
    Mat descriptors_1, descriptors_2;
    detector->compute( images[0], keypoints[0], descriptors_1 );
    std::cout << "Start with " << descriptors_1.size[0] << " features." << std::endl;
    for (int i =1; i < keypoints.size(); i++)
    {
        std::vector< DMatch > matches;
        detector->compute( images[i], keypoints[i], descriptors_2 );
        matcher.match( descriptors_1, descriptors_2, matches);
        double max_dist = 0; double min_dist = 100;
        for( int i = 0; i < matches.size(); i++ )
        {
            double dist = matches[i].distance;
            if( dist < min_dist ) min_dist = dist;
            if( dist > max_dist ) max_dist = dist;
            
        }
        
        //std::vector< DMatch > good_matches;
        Mat descriptors_2;
        for( int i = 0; i < matches.size(); i++ )
        {
            //if( matches[i][j].distance <= max(2*min_dist, 0.02) )
            if( matches[i].distance <= max((max_dist-min_dist)/1.01 + min_dist, 0.02) )
            { //good_matches.push_back( matches[i][j]); 
                Mat temp = descriptors_1.row(matches[i].queryIdx);
                descriptors_2.push_back( temp );
            }
        }
        
        descriptors_1 = descriptors_2;
        std::cout << "Now at " << descriptors_1.size[0] << " features." << std::endl;
    }
    
    
    //Using consensus features
    std::vector< DMatch > matches_consensus;
    matcher.match( descriptors_1, test_descriptors, matches_consensus);
    
       
    double max_dist = 0; double min_dist = 100;
    
    //-- Quick calculation of max and min distances between keypoints
    for( int i = 0; i < matches_consensus.size(); i++ )
    {

            double dist = matches_consensus[i].distance;
            if( dist < min_dist ) min_dist = dist;
            if( dist > max_dist ) max_dist = dist;
        
    }
    
    printf("-- Max dist : %f \n", max_dist );
    printf("-- Min dist : %f \n", min_dist );
    
    //-- Draw only "good" matches (i.e. whose distance is less than 2*min_dist,
    //-- or a small arbitary value ( 0.02 ) in the event that min_dist is very
    //-- small)
    //-- PS.- radiusMatch can also be used here.
    std::vector< DMatch > good_matches_consensus;
    
    for( int i = 0; i < matches_consensus.size(); i++ )
    {
        //if( matches[i][j].distance <= max(2*min_dist, 0.02) )
        if( matches_consensus[i].distance <= max((max_dist-min_dist)/4.0 + min_dist, 0.02) )
        { good_matches_consensus.push_back( matches_consensus[i]); }
//        else
//            printf("discard(%d,%d)\n",i,j);
    }
    std::cout << "Number of features consensus: " << descriptors_1.size[0] << " number of matches: " << good_matches_consensus.size() << std::endl;
    
    //Using aggreagate features
    std::vector< DMatch > matches_aggregate;
    matcher.match( best_aggregate_descriptors, test_descriptors, matches_aggregate);
    
       
    max_dist = 0; min_dist = 100;
    
    //-- Quick calculation of max and min distances between keypoints
    for( int i = 0; i < matches_aggregate.size(); i++ )
    {

            double dist = matches_aggregate[i].distance;
            if( dist < min_dist ) min_dist = dist;
            if( dist > max_dist ) max_dist = dist;
        
    }
    
    printf("-- Max dist : %f \n", max_dist );
    printf("-- Min dist : %f \n", min_dist );
    
    //-- Draw only "good" matches (i.e. whose distance is less than 2*min_dist,
    //-- or a small arbitary value ( 0.02 ) in the event that min_dist is very
    //-- small)
    //-- PS.- radiusMatch can also be used here.
    std::vector< DMatch > good_matches_aggregate;
    
    for( int i = 0; i < matches_aggregate.size(); i++ )
    {
        //if( matches[i][j].distance <= max(2*min_dist, 0.02) )
        if( matches_aggregate[i].distance <= max((max_dist-min_dist)/4.0 + min_dist, 0.02) )
        { good_matches_aggregate.push_back( matches_aggregate[i]); }
//        else
//            printf("discard(%d,%d)\n",i,j);
    }
    std::cout << "Number of features aggregate: " << best_aggregate_descriptors.size[0] << " number of matches: " << good_matches_aggregate.size() << std::endl;
    
    //Draw
    Mat img_matches_consensus; Mat img_keypoints_aggregate;
    drawMatches(images[0],keypoints[0],test_img,test_keypoints,good_matches_consensus,img_matches_consensus,Scalar::all(-1),Scalar::all(-1),vector<char>(),DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

    std:vector<KeyPoint> test_keypoints_matched;
    for (auto match : good_matches_aggregate)
    {
        test_keypoints_matched.push_back(test_keypoints[match.trainIdx]);
    }
    drawKeypoints( test_img, test_keypoints_matched, img_keypoints_aggregate, Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS );
    
    //-- Show detected (drawn) keypoints
    imshow("Matches Consensus", img_matches_consensus );
    imshow("Keypoints Aggregate", img_keypoints_aggregate );
    
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
    
    delete detector;
    return 0;
}

  /** @function readme */
  void readme()
  { std::cout << " Usage: ./<> <img1> <img2> ... <test_img>" << std::endl; }

