#include "CollabKeypoints.h"
#include "corpus.h"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "bagspotter.h"
#include "spatialaveragespotter.h"

#define LOAD 0


//void dumbtest(char* arg1, char* arg2)
//{
//    imgRef=img;
//    fWidth = (img.size[1]-0)/WINDOW_SIZE;
//    fHeight = (img.size[0]-0)/WINDOW_SIZE;
//    vector<KeyPoint> keyPoints;//(fWidth*fHeight);
//    for (int j =0; j < img.size[0]; j+=WINDOW_SIZE)
//    {
//        for (int i =0; i < img.size[1]; i+=WINDOW_SIZE)
//        {
//            //does angle need to be set? or is -1 proper?
//            KeyPoint p(i,j,SIFT_WINDOW_SIZE);
//            //keyPoints[(i/5)+(j/5)*fWidth] = (p);
//            //assert((i/5)+(j/5)*fWidth < fWidth*fHeight);
//            keyPoints.push_back(p);
//        }
//    }
    
    
//    SIFT detector;// = new SIFT();
    
//    detector(img,Mat(),keyPoints,descriptors,true);
//}

int main( int argc, char** argv )
{
    initModule_nonfree();
////    dumbtest(argv[1],argv[2]);
////    return 0;
//    //collabKeypoints(argc,argv);
//#if LOAD
//    Corpus c(argv[1]);
//    c.readCodeBook(argv[2]);
//#else
//    Corpus c;
//#endif
//    for (int i=3; i < argc-1; i++)
//    {
//        Mat img_1 = imread( argv[i], CV_LOAD_IMAGE_GRAYSCALE );
//        cout <<"loading image: "<<argv[i]<<endl;
//#if LOAD
//        c.addPrevImage(img_1,i-3);
//#else
//        c.addImage(img_1);
//#endif
        
//    }
//#if !LOAD
//    c.generateCodebook();
//    c.saveCodedCorpus(argv[1]);
//    c.saveCodeBook(argv[2]);
//#endif
    
//    Mat q = imread( argv[argc-1], CV_LOAD_IMAGE_GRAYSCALE );
////    Mat q2 = imread( argv[argc-1], CV_LOAD_IMAGE_GRAYSCALE );
//    c.saveQuantizedImage("./test.png",23);
//    c.makeHeatMap(q,23);
////    c.makeHeatMap(q2,23);
    
    Codebook c;
    c.readInCSV(argv[1]);
    SpatialAverageSpotter b(&c);
    b.train(argv[2]);
    //b.testSpotting(argv[3]);
    b.produceHeatMap(argv[3]);
    
    
}
