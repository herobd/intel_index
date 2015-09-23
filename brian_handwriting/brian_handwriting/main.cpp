//#include "CollabKeypoints.h"
//#include "MatchKeypoints.h"
//#include "corpus.h"

#include <opencv2/highgui/highgui.hpp>
//#include "opencv2/nonfree/nonfree.hpp"
//#include "opencv2/nonfree/features2d.hpp"
//#include "bagspotter.h"
//#include "spatialaveragespotter.h"
#include "enhancedbovw.h"

#include <ctime>

//#include "dimage.h"
//#include "dslantangle.h"

#include <omp.h>
//#include <mpi.h>

#define LOAD 0



#include "enhancedbovwtests.h"
#include "liang.h"
#include "liangtests.h"
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
    string simple_corpus = "/home/brian/intel_index/data/simple_corpus2/";
    
    string option = argv[1];
    if (option.compare("score")==0)
    {
        Liang l;
        l.unittest();
        MOG mog;
        mog.unittest();
        //../../data/gw_20p_wannot/liangTrained.dat ../../data/gw_20p_wannot/deslant/wordimg_3.png Orders
        string save = argv[2];
        string imgPath = argv[3];
        
        l.loadCharacterModels(save);
        
        Mat testImg= imread(imgPath, CV_LOAD_IMAGE_GRAYSCALE);
        threshold(testImg,testImg,IMAGE_THRESH,255,1);
        double score = l.score(argv[4],testImg);
        cout << score << endl;
    }
    else if (option.compare("train")==0)
    {
        Liang l;
        l.unittest();
        MOG mog;
        mog.unittest();
        //train ../../data/gw_20p_wannot/deslant/ wordimg_ .png ../../data/gw_20p_wannot/annotations.txt ../../data/gw_20p_wannot/liangTrained.dat
        string imgDir=argv[2];
        string namePattern=argv[3];
        string ext=argv[4];
        string ann=argv[5];
        string save=argv[6];
        
        l.trainCharacterModels(imgDir,namePattern,ext,ann);
        l.saveCharacterModels(save);
    }
    else if (option.compare("unittest")==0)
    {
        Liang l;
        l.unittest();
        MOG mog;
        mog.unittest();
    }
    else if (option.compare("test")==0)
    {
        cout << "testing" << endl;
        int keyword = stoi(argv[2]);
        int testNum = stoi(argv[3]);
        string annFilePath = argv[4];
        string imgDirPath = argv[5];
        string imgNamePater = argv[6];
        string imgExt = argv[7];
        LiangTests::keyword_duplicationTest(keyword,testNum,annFilePath,imgDirPath,imgNamePater,imgExt);
        //test 0 1 ../../data/gw_20p_wannot/annotations.txt ../../data/gw_20p_wannot/deslant/ wordimg_ .png
    }
    else if(option.compare("mog")==0)
    {
        
        string annPath = argv[2];
        string imgDirPath = argv[3];
        string imgNamePater = argv[4];
        string imgExt = argv[5];
        string modelMOG = argv[6];
        
        
        ifstream file;
        file.open (annPath, ios::in);
        assert(file.is_open());
        
        string word;
        regex clean("[^a-zA-Z0-9]");
        
        vector< pair<int, string> > samples;
        int num=0;
        while (getline(file,word) && num<1000)
        {
            word = regex_replace(word,clean,"");
            samples.push_back(pair<int,string>(++num,word));
        }
        
        Liang l;
        l.showCharacterModels(imgDirPath, imgNamePater, imgExt, samples, modelMOG);
    }
    else if (option.compare("bovwunittest")==0)
    {
        EnhancedBoVW bovw;
        bovw.unittests();
        
        for (int s=0; s<5; s++)
            EnhancedBoVWTests::experiment_Aldavert_dist_batched_test(s);
    }
    else if (option.compare("bovwscore")==0)
    {
        //    MPI_Init(&argc,&argv);
        
        EnhancedBoVW bovw;
        
        string codebookLoc = argv[2];//"data/IAM_codebook.csv";
        
        //    bovw.makeCodebook("data/GW/words/");
        //    bovw.codebook->save(codebookLoc);
        
        bovw.codebook = new Codebook();
        bovw.codebook->readIn(codebookLoc);
        
        Mat find = imread(argv[3], CV_LOAD_IMAGE_GRAYSCALE);
        vector<float>* find_b = bovw.featurizeImage(find);
        Mat img = imread(argv[4], CV_LOAD_IMAGE_GRAYSCALE);
        double score = bovw.compareImage(img,*find_b);
        cout << score << endl;
        
        
        
        //    experiment_dist(bovw,string(argv[2]),string(argv[3]),string(argv[4]),atoi(argv[5]),atoi(argv[6]), string(argv[7]), argc==8);
//        EnhancedBoVWTests::experiment_Aldavert(bovw,string(argv[2]),string(argv[3]),atoi(argv[4]), string(argv[5]), string(argv[6]));
        //    EnhancedBoVWTests::experiment_Aldavert_dist(bovw,string(argv[2]),string(argv[3]),atoi(argv[4]), string(argv[5]));
        
        //    MPI_Finalize();
        
    }
    else if (option.compare("experiment_Aldavert_dist_batched")==0)
    {
        EnhancedBoVW bovw;
            
        string codebookLoc = argv[2];
        bovw.codebook = new Codebook();
        bovw.codebook->readIn(codebookLoc);
        
        EnhancedBoVWTests::experiment_Aldavert_dist_batched(bovw,string(argv[3]),string(argv[4]),atoi(argv[5]), string(argv[6]),atoi(argv[7]),atoi(argv[8]),string(argv[9]));
    }
    else if (option.compare("train_codebook")==0)
    {
            
        string imgDir = argv[2];
        string codebookLoc = argv[3];
        EnhancedBoVW bovw;
        Codebook *cb = bovw.makeCodebook(imgDir);
        cb->save(codebookLoc);
    }
    else if (option.compare("train_codebook_simple")==0)
    {
            
        string imgDir = simple_corpus + "words/";
        string codebookLoc = simple_corpus + "codebook.csv";
        vector<Vec2i> spatialPyramids={Vec2i(1,1), Vec2i(2,1)};
        EnhancedBoVW bovw(spatialPyramids,3500,1,10,10,10,2,2,2,1);
        Codebook *cb = bovw.makeCodebook(imgDir,4);
        cb->save(codebookLoc);
        cb->print();
    }
    else if (option.compare("bovwscore_simple")==0)
    {
        vector<Vec2i> spatialPyramids={Vec2i(1,1), Vec2i(2,1)};
        EnhancedBoVW bovw(spatialPyramids,3500,1,10,10,10,2,2,2,1);
            
        string codebookLoc = simple_corpus + "codebook.csv";
        bovw.codebook = new Codebook();
        bovw.codebook->readIn(codebookLoc);
        
        
        string f = argv[2];
        string s = argv[3];
        Mat find = imread(f, CV_LOAD_IMAGE_GRAYSCALE);
        vector<float>* find_b = bovw.featurizeImage(find);
        Mat img = imread(s, CV_LOAD_IMAGE_GRAYSCALE);
        double score = bovw.compareImage(img,*find_b);
        cout << score << endl;
    }
    else if (option.compare("show_simple")==0)
    {
        vector<Vec2i> spatialPyramids={Vec2i(1,1), Vec2i(2,1)};
        EnhancedBoVW bovw(spatialPyramids,3500,1,10,10,10,2,2,2,1);
            
        string codebookLoc = simple_corpus + "codebook.csv";
        bovw.codebook = new Codebook();
        bovw.codebook->readIn(codebookLoc);
        
        
        string f = argv[2];
        Mat find = imread(f, CV_LOAD_IMAGE_GRAYSCALE);
        bovw.showEncoding(find);
    }
    
    
}

