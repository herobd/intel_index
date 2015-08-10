//#include "CollabKeypoints.h"
//#include "MatchKeypoints.h"
//#include "corpus.h"

#include <opencv2/highgui/highgui.hpp>
//#include "opencv2/nonfree/nonfree.hpp"
//#include "opencv2/nonfree/features2d.hpp"
//#include "bagspotter.h"
//#include "spatialaveragespotter.h"
#include "enhancedbovw.h"
#include "liang.h"

#include <ctime>

//#include "dimage.h"
//#include "dslantangle.h"

#include <omp.h>
//#include <mpi.h>

#define LOAD 0



#include "enhancedbovwtests.h"
#include "liang.h"
#include "mog.h"



void deslant(Mat &img);

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
    if (argc==4)
    {
        Liang l;
        l.unittest();
        MOG mog;
        mog.unittest();
        //../../data/gw_20p_wannot/liangTrained.dat ../../data/gw_20p_wannot/deslant/wordimg_3.png Orders
        string save = argv[1];
        string imgPath = argv[2];
        
        l.loadCharacterModels(save);
        
        Mat testImg= imread(imgPath,0);
        threshold(testImg,testImg,IMAGE_THRESH,255,1);
        double score = l.score(argv[3],testImg);
        cout << score << endl;
    }
    else if (argc==6)
    {
        Liang l;
        l.unittest();
        MOG mog;
        mog.unittest();
        //../../data/gw_20p_wannot/deslant/ wordimg_ .png ../../data/gw_20p_wannot/annotations.txt ../../data/gw_20p_wannot/liangTrained.dat
        string imgDir=argv[1];
        string namePattern=argv[2];
        string ext=argv[3];
        string ann=argv[4];
        string save=argv[5];
        
        l.trainCharacterModels(imgDir,namePattern,ext,ann);
        l.saveCharacterModels(save);
    }
    else
    {
        //    MPI_Init(&argc,&argv);
        
        EnhancedBoVW bovw;
        
        string codebookLoc = argv[1];//"data/IAM_codebook.csv";
        
        //    bovw.makeCodebook("data/GW/words/");
        //    bovw.codebook->save(codebookLoc);
        
        bovw.codebook = new Codebook();
        bovw.codebook->readIn(codebookLoc);
        
        Mat find = imread(argv[2]);
        vector<float>* find_b = bovw.featurizeImage(find);
        Mat img = imread(argv[3]);
        double score = bovw.compareImage(img,*find_b);
        cout << score << endl;
        
        
        
        //    experiment_dist(bovw,string(argv[2]),string(argv[3]),string(argv[4]),atoi(argv[5]),atoi(argv[6]), string(argv[7]), argc==8);
//        EnhancedBoVWTests::experiment_Aldavert(bovw,string(argv[2]),string(argv[3]),atoi(argv[4]), string(argv[5]), string(argv[6]));
        //    EnhancedBoVWTests::experiment_Aldavert_dist(bovw,string(argv[2]),string(argv[3]),atoi(argv[4]), string(argv[5]));
        
        //    MPI_Finalize();
        
    }

    
    
}

