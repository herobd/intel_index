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
#include "preprocessor.h"


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
    
#if !NO_SP_PY
    vector<Vec2i> simpleSpatialPyramids={Vec2i(2,1),Vec2i(4,1)};
#else
    vector<Vec2i> simpleSpatialPyramids={Vec2i(1,1)};
#endif
    
#if !NO_LLC
    int simple_LLC=3;
#else
    int simple_LLC=1;
#endif
    
    string option = argv[1];
    if (option.compare("liangscore")==0)
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
    else if (option.compare("liangtrain")==0)
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
    else if (option.compare("liangunittest")==0)
    {
        Liang l;
        l.unittest();
        MOG mog;
        mog.unittest();
    }
    else if (option.compare("liangtest")==0)
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
    else if(option.compare("liangmog")==0)
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
        
        
        //EnhancedBoVWTests::test(bovw);
    }
    else if (option.compare("bovwscore")==0)
    {
        //    MPI_Init(&argc,&argv);
        Preprocessor pre(PP_BASELINE_CENTER | PP_BASELINE_NORMALIZE);
        EnhancedBoVW bovw;
        bovw.setPre(pre);
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
    else if (option.compare("bovwscore_single")==0)
    {
        vector<Vec2i> spatialPyramids={Vec2i(1,1)};
        EnhancedBoVW bovw(spatialPyramids);
        
        string codebookLoc = argv[2];
        
        bovw.codebook = new Codebook();
        bovw.codebook->readIn(codebookLoc);
//        bovw.readCodebooks(codebookLoc);
        
        Mat find = imread(argv[3], CV_LOAD_IMAGE_GRAYSCALE);
        vector<float>* find_b = bovw.featurizeImage(find);
        Mat img = imread(argv[4], CV_LOAD_IMAGE_GRAYSCALE);
        double score = bovw.compareImage(img,*find_b);
        
        
        auto desc = bovw.getDescriptors(find);
        auto codedImg= bovw.codeDescriptorsIntegralImageSkip(desc,find.size,1);
        vector<float>* featureVector = bovw.getPooledDescFastSkip(codedImg,Rect(0,0,find.cols,find.rows),spatialPyramids,1);
        auto desc2 = bovw.getDescriptors(img);
        auto codedImg2= bovw.codeDescriptorsIntegralImageSkip(desc2,img.size,1);
        vector<float>* featureVector2 = bovw.getPooledDescFastSkip(codedImg2,Rect(0,0,img.cols,img.rows),spatialPyramids,1);
        bovw.normalizeDesc(featureVector2);
        bovw.normalizeDesc(featureVector);
        double score2=0;
        
        for (int i=0; i<featureVector->size(); i++)
        {
            if (pow(featureVector->at(i)-featureVector2->at(i),2) > .001)
            {
                cout << "["<<i<<"] " << featureVector->at(i) <<  " != " << featureVector2->at(i) << endl;
            }
            score2 += pow(featureVector->at(i)-featureVector2->at(i),2);
        }
        cout << "score  " << score << endl;
        cout << "score2 " << score2 << endl;
    }    
    else if (option.compare("showhog")==0)
    {

        Mat img = imread(argv[2], CV_LOAD_IMAGE_GRAYSCALE);
        EnhancedBoVW bovw(simpleSpatialPyramids,stoi(argv[3]));
        //bovw.printDescThreshContours(find);
        bovw.printDescThreshContours(img,stoi(argv[3]));
    }
    else if (option.compare("experiment_subword")==0)
    {
        EnhancedBoVW bovw;
            
        string codebookLoc = argv[2];
        bovw.codebook = new Codebook();
        bovw.codebook->readIn(codebookLoc);
        
        string locationCSVPath = argv[3];
        string exemplarDirPath = argv[4];
        string dataDirPath = argv[5];
        int dataSize = atoi(argv[6]);
        int numExemplarsPer = atoi(argv[7]);
        string fileExt = argv[8];
        string outfile = argv[9];
        
        EnhancedBoVWTests::experiment(bovw, locationCSVPath, exemplarDirPath, dataDirPath, dataSize, numExemplarsPer, fileExt, outfile, true);
    }
    else if (option.compare("experiment_dist_batched")==0)
    {
        vector<Vec2i> spp={Vec2i(1,1),Vec2i(2,2)};
        EnhancedBoVW bovw(spp);
            
        string codebookLoc = argv[2];
        bovw.codebook = new Codebook();
        bovw.codebook->readIn(codebookLoc);
        
        string locationCSVPath = argv[3];
        string exemplarDirPath = argv[4];
        string dataDirPath = argv[5];
        int dataSize = atoi(argv[6]);
        int numExamplesPer = atoi(argv[7]);
        string fileExt = argv[8];
        int batchNum = atoi(argv[9]);
        int numOfBatches = atoi(argv[10]);
        string outfile = argv[11];
        
        EnhancedBoVWTests::experiment_dist_batched(bovw,locationCSVPath, exemplarDirPath, dataDirPath, dataSize, numExamplesPer, fileExt, batchNum, numOfBatches, outfile);
    }
    /*else if (option.compare("experiment_dist_batched_reuse")==0)
    {
        vector<Vec2i> spp={Vec2i(1,1),Vec2i(2,2)};
        EnhancedBoVW bovw(spp);
            
        string codebookLoc = argv[2];
        bovw.codebook = new Codebook();
        bovw.codebook->readIn(codebookLoc);
        
        string locationCSVPath = argv[3];
        string exemplarDirPath = argv[4];
        string dataDirPath = argv[5];
        int dataSize = atoi(argv[6]);
        int numExamplesPer = atoi(argv[7]);
        string fileExt = argv[8];
        int batchNum = atoi(argv[9]);
        int numOfBatches = atoi(argv[10]);
        string outfile = argv[11];
        
        EnhancedBoVWTests::experiment_dist_batched(bovw,locationCSVPath, exemplarDirPath, dataDirPath, dataSize, numExamplesPer, fileExt, batchNum, numOfBatches, outfile,false);
    }*/
    else if (option.compare("experiment_Aldavert_dist_batched")==0)
    {
        EnhancedBoVW bovw;
            
        string codebookLoc = argv[2];
        bovw.codebook = new Codebook();
        bovw.codebook->readIn(codebookLoc);
        
        string locationCSVPath = argv[3];
        string dataDirPath = argv[4];
        int dataSize = atoi(argv[5]);
        string fileExt = argv[6];
        int batchNum = atoi(argv[7]);
        int numOfBatches = atoi(argv[8]);
        string outfile = argv[9];
        
        EnhancedBoVWTests::experiment_Aldavert_dist_batched(bovw,locationCSVPath, dataDirPath, dataSize, fileExt, batchNum, numOfBatches, outfile);
    }
    else if (option.compare("experiment_Aldavert_dist_batched_baseCenter")==0)
    {
        EnhancedBoVW bovw;
        bovw.setPre(Preprocessor(PP_BASELINE_CENTER));
        string codebookLoc = argv[2];
        bovw.codebook = new Codebook();
        bovw.codebook->readIn(codebookLoc);
        
        string locationCSVPath = argv[3];
        string dataDirPath = argv[4];
        int dataSize = atoi(argv[5]);
        string fileExt = argv[6];
        int batchNum = atoi(argv[7]);
        int numOfBatches = atoi(argv[8]);
        string outfile = argv[9];
        
        EnhancedBoVWTests::experiment_Aldavert_dist_batched(bovw,locationCSVPath, dataDirPath, dataSize, fileExt, batchNum, numOfBatches, outfile);
    }
    else if (option.compare("experiment_Aldavert_dist_batched_baseCenterNorm")==0)
    {
        EnhancedBoVW bovw;
        bovw.setPre(Preprocessor(PP_BASELINE_CENTER | PP_BASELINE_NORMALIZE));
        string codebookLoc = argv[2];
        bovw.codebook = new Codebook();
        bovw.codebook->readIn(codebookLoc);
        
        string locationCSVPath = argv[3];
        string dataDirPath = argv[4];
        int dataSize = atoi(argv[5]);
        string fileExt = argv[6];
        int batchNum = atoi(argv[7]);
        int numOfBatches = atoi(argv[8]);
        string outfile = argv[9];
        
        EnhancedBoVWTests::experiment_Aldavert_dist_batched(bovw,locationCSVPath, dataDirPath, dataSize, fileExt, batchNum, numOfBatches, outfile);
    }
    else if (option.compare("experiment_Aldavert_dist_batched_pyramid")==0)
    {
        vector<Vec2i> spatialPyramid;//={Vec2i(2,1),Vec2i(4,1)};
        string layers = argv[2];
        regex getLayer("\\((\\d*),(\\d*)\\)");
        smatch m;
        cout <<"Pyramid: ";
        while (regex_search (layers,m,getLayer)) 
        {
            Vec2i layer(stoi(m[1]),stoi(m[2]));
            spatialPyramid.push_back(layer);
            layers=m.suffix().str();
            cout <<"("<<layer[0]<<", "<<layer[1]<<") ";
        }
        cout << endl;

        EnhancedBoVW bovw(spatialPyramid);
        bovw.setPre(Preprocessor(PP_BASELINE_CENTER | PP_BASELINE_NORMALIZE));//This is best preproessing so far
        string codebookLoc = argv[3];
        bovw.codebook = new Codebook();
        bovw.codebook->readIn(codebookLoc);
        
        string locationCSVPath = argv[4];
        string dataDirPath = argv[5];
        int dataSize = atoi(argv[6]);
        string fileExt = argv[7];
        int batchNum = atoi(argv[8]);
        int numOfBatches = atoi(argv[9]);
        string outfile = argv[10];
        
        EnhancedBoVWTests::experiment_Aldavert_dist_batched(bovw,locationCSVPath, dataDirPath, dataSize, fileExt, batchNum, numOfBatches, outfile);
    }
    else if (option.compare("show_best")==0)
    {
        EnhancedBoVW bovw;
        Preprocessor pre(PP_BASELINE_CENTER | PP_BASELINE_NORMALIZE);
        bovw.setPre(pre);
        string codebookLoc = argv[2];
        bovw.codebook = new Codebook();
        bovw.codebook->readIn(codebookLoc);
        
        string locationCSVPath = argv[3];
        string dataDirPath = argv[4];
        int dataSize = atoi(argv[5]);
        string fileExt = argv[6];
        Mat find = imread(argv[7], CV_LOAD_IMAGE_GRAYSCALE);
        string gt = argv[8];
        
        multimap<double,int> scores = EnhancedBoVWTests::experiment_Aldavert_single(bovw,locationCSVPath, dataDirPath, dataSize, fileExt, find,gt);
        auto iter = scores.begin();
        for (int i=0; i<10; i++, iter++)
        {
            cout<<"with score "<<iter->first<<", word "<<iter->second<<endl;
            string imagePath = dataDirPath + "wordimg_" + to_string(iter->second) + fileExt;
            Mat found = imread(imagePath, CV_LOAD_IMAGE_GRAYSCALE);
            imshow(to_string(i+1),found);
            waitKey();
        }

    }
    else if (option.compare("experiment_Aldavert_dist_batched_noLLC")==0)
    {
        EnhancedBoVW bovw;
        bovw.setLLC(1);
            
        string codebookLoc = argv[2];
        bovw.codebook = new Codebook();
        bovw.codebook->readIn(codebookLoc);
        
        string locationCSVPath = argv[3];
        string dataDirPath = argv[4];
        int dataSize = atoi(argv[5]);
        string fileExt = argv[6];
        int batchNum = atoi(argv[7]);
        int numOfBatches = atoi(argv[8]);
        string outfile = argv[9];
        
        EnhancedBoVWTests::experiment_Aldavert_dist_batched(bovw, locationCSVPath, dataDirPath, dataSize, fileExt, batchNum, numOfBatches, outfile);
    }
    else if (option.compare("experiment_Aldavert_dist_batched_LLC")==0)
    {
        EnhancedBoVW bovw;
        bovw.setLLC(atoi(argv[2]));
            
        string codebookLoc = argv[3];
        bovw.codebook = new Codebook();
        bovw.codebook->readIn(codebookLoc);
        
        string locationCSVPath = argv[4];
        string dataDirPath = argv[5];
        int dataSize = atoi(argv[6]);
        string fileExt = argv[7];
        int batchNum = atoi(argv[8]);
        int numOfBatches = atoi(argv[9]);
        string outfile = argv[10];
        
        EnhancedBoVWTests::experiment_Aldavert_dist_batched(bovw, locationCSVPath, dataDirPath, dataSize, fileExt, batchNum, numOfBatches, outfile);
    }
    else if (option.compare("experiment_Aldavert_dist_batched_short")==0)
    {
        EnhancedBoVW bovw;
        
        string codebookLoc = "/home/brian/intel_index/data/gw_20p_wannot/codebook_deslant.dat";
//        bovw.readCodebooks(codebookLoc);
        bovw.codebook = new Codebook();
        bovw.codebook->readIn(codebookLoc);
        
        EnhancedBoVWTests::experiment_Aldavert_dist_batched(bovw,
                                                            "/home/brian/intel_index/data/gw_20p_wannot/C_wordLocations.csv",
                                                            "/home/brian/intel_index/data/gw_20p_wannot/deslant/",
                                                            4860,
                                                            ".png",
                                                            0,
                                                            150,
                                                            "simpleOut.out");
    }
    else if (option.compare("bovw_ggobi")==0)
    {
        EnhancedBoVW bovw;
            
        string codebookLoc = argv[2];
        bovw.codebook = new Codebook();
        bovw.codebook->readIn(codebookLoc);
        
        string locationCSVPath = argv[3];
        string dataDirPath = argv[4];
        string fileExt = argv[5];
        int numWords = atoi(argv[6]);
        string outfile = argv[7];
        
        EnhancedBoVWTests::createGGobiFile(bovw, locationCSVPath, dataDirPath, fileExt, numWords, outfile);
    }
    else if (option.compare("bovw_draw")==0)
    {
        EnhancedBoVW bovw;
            
        string codebookLoc = argv[2];
        bovw.codebook = new Codebook();
        bovw.codebook->readIn(codebookLoc);
        
        string locationCSVPath = argv[3];
        string dataDirPath = argv[4];
        string fileExt = argv[5];
        int numWords = atoi(argv[6]);
        string outfile = argv[7];
        
        EnhancedBoVWTests::drawData(bovw, locationCSVPath, dataDirPath, fileExt, numWords, outfile);
    }
    else if (option.compare("bovw_draw_words")==0)
    {
        EnhancedBoVW bovw;
            
        string codebookLoc = argv[2];
        bovw.codebook = new Codebook();
        bovw.codebook->readIn(codebookLoc);
        
        string locationCSVPath = argv[3];
        string dataDirPath = argv[4];
        string fileExt = argv[5];
        string outfile = argv[6];
        vector<string> words;
        for (int i=7; i<argc; i++)
            words.push_back(argv[i]);
        EnhancedBoVWTests::drawDataForWords(bovw, locationCSVPath, dataDirPath, fileExt, words, outfile);
    }
    else if (option.compare("bovw_compare_words")==0)
    {
        EnhancedBoVW bovw;
            
        string codebookLoc = argv[2];
        bovw.codebook = new Codebook();
        bovw.codebook->readIn(codebookLoc);
        
        string locationCSVPath = argv[3];
        string dataDirPath = argv[4];
        string fileExt = argv[5];
        string exampleFile = argv[6];
        string outfile = argv[7];
        vector<string> words;
        for (int i=8; i<argc; i++)
            words.push_back(argv[i]);
        EnhancedBoVWTests::compareDataForWords(bovw, locationCSVPath, dataDirPath, fileExt, words, exampleFile, outfile);
    }
    else if (option.compare("train_codebook")==0)
    {
            
        string imgDir = argv[2];
        string codebookLoc = argv[3];
        EnhancedBoVW bovw;
        Codebook *cb = bovw.makeCodebook(imgDir);
        cb->save(codebookLoc);
    }
    else if (option.compare("train_1024_codebook")==0)
    {
            
        string imgDir = argv[2];
        string codebookLoc = argv[3];
        EnhancedBoVW bovw;
        bovw.setPre(Preprocessor(PP_BASELINE_CENTER | PP_BASELINE_NORMALIZE));
        Codebook *cb = bovw.makeCodebook(imgDir,1024);
        cb->save(codebookLoc);
    }
    else if (option.compare("train_1024_codebook_noBaseNorm")==0)
    {
            
        string imgDir = argv[2];
        string codebookLoc = argv[3];
        EnhancedBoVW bovw;
        bovw.setPre(Preprocessor(PP_BASELINE_CENTER));
        Codebook *cb = bovw.makeCodebook(imgDir,1024);
        cb->save(codebookLoc);
    }
    else if (option.compare("train_short_codebook")==0)
    {
            
        string imgDir = argv[2];
        string codebookLoc = argv[3];
        EnhancedBoVW bovw;
        Codebook *cb = bovw.makeCodebook(imgDir,200);
        cb->save(codebookLoc);
    }
    else if (option.compare("train_codebooks")==0)
    {
            
        string imgDir = argv[2];
        string codebookLoc = argv[3];
        EnhancedBoVW bovw;
        bovw.make3Codebooks(imgDir);
        bovw.writeCodebooks(codebookLoc);
    }
    else if (option.compare("examine_codebook")==0)
    {
            
        string codebookLoc = argv[2];
        int ex1 = atoi(argv[3]);
        int ex2 = atoi(argv[4]);
        int ex3 = atoi(argv[5]);
        Codebook cb;
        cb.readIn(codebookLoc);
        vector<double> hog1 = cb.getEx(ex1-cb.size());
        vector<double> hog2 = cb.getEx(ex2-cb.size());
        vector<double> hog3 = cb.getEx(ex3-cb.size());
        
        for (int i=0; i<cb.depth(); i++)
        {
            cout << hog1[i] << "\t" << hog2[i] << "\t" << hog3[i] << endl;
        }
    }
    else if (option.compare("train_codebook_simple")==0)
    {
            
        string imgDir = simple_corpus + "words_lots/";
        string codebookLoc = simple_corpus + "codebook.csv";
        EnhancedBoVW bovw(simpleSpatialPyramids,DESC_THRESH,simple_LLC,6,8,10,2,2,2,2);
        Codebook *cb = bovw.makeCodebook(imgDir,160);
        cb->save(codebookLoc);
        //cb->print();
    }
    else if (option.compare("train_codebooks_simple")==0)
    {
            
        string imgDir = simple_corpus + "words_lots/";
        string codebookLoc = simple_corpus + "codebook.csv";
        EnhancedBoVW bovw(simpleSpatialPyramids,DESC_THRESH,simple_LLC,6,8,10,2,2,2,2);
        bovw.make3Codebooks(imgDir,160);
        bovw.writeCodebooks(codebookLoc);
    }
    else if (option.compare("bovwscore_simple")==0)
    {
        EnhancedBoVW bovw(simpleSpatialPyramids,DESC_THRESH,simple_LLC,6,8,10,2,2,2,2);
        
        string codebookLoc = simple_corpus + "codebook.csv";
//        bovw.readCodebooks(codebookLoc);  
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
    else if (option.compare("bovwscore_single_simple")==0)
    {
        vector<Vec2i> spatialPyramids={Vec2i(1,1)};
        EnhancedBoVW bovw(spatialPyramids,DESC_THRESH,simple_LLC,6,8,10,2,2,2,2);
            
        string codebookLoc = simple_corpus + "codebook.csv";
//        bovw.readCodebooks(codebookLoc);
        bovw.codebook = new Codebook();
        bovw.codebook->readIn(codebookLoc);
        
        
        string f = argv[2];
        string s = argv[3];
        Mat find = imread(f, CV_LOAD_IMAGE_GRAYSCALE);
        vector<float>* find_b = bovw.featurizeImage(find);
        Mat img = imread(s, CV_LOAD_IMAGE_GRAYSCALE);
        double score = bovw.compareImage(img,*find_b);
        
        auto desc = bovw.getDescriptors(find);
        auto codedImg= bovw.codeDescriptorsIntegralImageSkip(desc,find.size,1);
        vector<float>* featureVector = bovw.getPooledDescFastSkip(codedImg,Rect(0,0,find.cols,find.rows),spatialPyramids,1);
        auto desc2 = bovw.getDescriptors(img);
        auto codedImg2= bovw.codeDescriptorsIntegralImageSkip(desc2,img.size,1);
        vector<float>* featureVector2 = bovw.getPooledDescFastSkip(codedImg2,Rect(0,0,img.cols,img.rows),spatialPyramids,1);
        bovw.normalizeDesc(featureVector2);
        bovw.normalizeDesc(featureVector);
        double score2=0;
        
        for (int i=0; i<featureVector->size(); i++)
        {
            if (pow(featureVector->at(i)-featureVector2->at(i),2) > .001)
            {
                cout << "["<<i<<"] " << featureVector->at(i) <<  " != " << featureVector2->at(i) << endl;
            }
            score2 += pow(featureVector->at(i)-featureVector2->at(i),2);
        }
        cout << "score  " << score << endl;
        cout << "score2 " << score2 << endl;
    }
    else if (option.compare("show_simple")==0)
    {
        EnhancedBoVW bovw(simpleSpatialPyramids,DESC_THRESH,simple_LLC,6,8,10,2,2,2,2);
            
        string codebookLoc = simple_corpus + "codebook.csv";
        bovw.readCodebooks(codebookLoc);
//        bovw.codebook = new Codebook();
//        bovw.codebook->readIn(codebookLoc);
        
        
        string f = argv[2];
        Mat find = imread(f, CV_LOAD_IMAGE_GRAYSCALE);
        bovw.showEncoding(find);
    }
    else if (option.compare("experiment_Aldavert_dist_batched_simple")==0)
    {
        EnhancedBoVW bovw(simpleSpatialPyramids,DESC_THRESH,simple_LLC,6,8,10,2,2,2,2);
        
        string codebookLoc = simple_corpus + "codebook.csv";
//        bovw.readCodebooks(codebookLoc);
        bovw.codebook = new Codebook();
        bovw.codebook->readIn(codebookLoc);
        
        EnhancedBoVWTests::experiment_Aldavert_dist_batched(bovw,
                                                            simple_corpus + "wordLocations.csv",
                                                            simple_corpus + "words/",
                                                            144,
                                                            ".png",
                                                            0,
                                                            1,
                                                            "simpleOut.out");
    }
    else if (option.compare("experiment_Aldavert_dist_batched_simple_LLC")==0)
    {
        EnhancedBoVW bovw(simpleSpatialPyramids,DESC_THRESH,atoi(argv[2]),6,8,10,2,2,2,2);
        
        string codebookLoc = simple_corpus + "codebook.csv";
//        bovw.readCodebooks(codebookLoc);
        bovw.codebook = new Codebook();
        bovw.codebook->readIn(codebookLoc);
        
        EnhancedBoVWTests::experiment_Aldavert_dist_batched(bovw,
                                                            simple_corpus + "wordLocations.csv",
                                                            simple_corpus + "words/",
                                                            144,
                                                            ".png",
                                                            0,
                                                            1,
                                                            "simpleOut.out");
    }
    else if (option.compare("bovwdebug")==0)
    {
        string first = argv[2];
        string second = argv[3];
        vector<Vec2i> spatialPyramidsSingle={Vec2i(1,1)};
        
        EnhancedBoVW bovw(simpleSpatialPyramids,DESC_THRESH,simple_LLC,6,8,10,2,2,2,2);
            
        string codebookLoc = simple_corpus + "codebook.csv";
//        bovw.readCodebooks(codebookLoc);
        bovw.codebook = new Codebook();
        bovw.codebook->readIn(codebookLoc);
        
        Vec3b class0(255,255,255);
        Vec3b class1(255,0,0);
        Vec3b class2(0,255,0);
        Vec3b class3(0,0,255);
        Vec3b none(0,0,0);
        
        Mat ex = imread(simple_corpus + first, CV_LOAD_IMAGE_GRAYSCALE);
        cout << "width=" << ex.cols << " height=" << ex.rows << endl;
        auto desc = bovw.getDescriptors(ex);
        
        auto codedImg= bovw.codeDescriptorsIntegralImageSkip(desc,ex.size,1);
        
        Mat out(codedImg->at(0).size(), codedImg->size(), CV_8UC3);
        for (int y=0; y<codedImg->at(0).size(); y++)
            for (int x=0; x<codedImg->size(); x++)
            {
                Rect win;
                win.x=x;
                win.y=y;
                win.width=1;
                win.height=1;
                vector<float>* featureVector = bovw.getPooledDescFastSkip(codedImg,win,spatialPyramidsSingle,1);
                int greatest=-1;
                int max=0;
                for (int i=0; i<featureVector->size(); i++)
                {
                    float f = featureVector->at(i);
                    if (f>max)
                    {
                        greatest=i;
                        max=f;
                    }
                }
                switch (greatest) {
//                case 0:
//                    out.at<Vec3b>(y,x)=class0;
//                    break;
//                case 1:
//                    out.at<Vec3b>(y,x)=class1;
//                    break;
//                case 2:
//                    out.at<Vec3b>(y,x)=class2;
//                    break;
//                case 3:
//                    out.at<Vec3b>(y,x)=class3;
//                    break;
//                case 4:
//                    out.at<Vec3b>(y,x)=class0;
//                    break;
//                case 5:
//                    out.at<Vec3b>(y,x)=class1;
//                    break;
//                case 6:
//                    out.at<Vec3b>(y,x)=class2;
//                    break;
//                case 7:
//                    out.at<Vec3b>(y,x)=class3;
//                    break;
//                case 8:
//                    out.at<Vec3b>(y,x)=class0;
//                    break;
//                case 9:
//                    out.at<Vec3b>(y,x)=class1;
//                    break;
//                case 10:
//                    out.at<Vec3b>(y,x)=class2;
//                    break;
//                case 11:
//                    out.at<Vec3b>(y,x)=class3;
//                    break;
                case 147:
                    out.at<Vec3b>(y,x)=class1;
                    break;
                case 147+160:
                    out.at<Vec3b>(y,x)=class2;
                    break;
                case 147+320:
                    out.at<Vec3b>(y,x)=class3;
                    break;
                default:
                    out.at<Vec3b>(y,x)=none;
                    break;
                }
                
            }
        imshow("composed",out);
        waitKey(1);
        
        Mat ex2 = imread(simple_corpus + second, CV_LOAD_IMAGE_GRAYSCALE);
        cout << "width=" << ex2.cols << " height=" << ex2.rows << endl;
        auto desc2 = bovw.getDescriptors(ex2);
        
        auto codedImg2= bovw.codeDescriptorsIntegralImageSkip(desc2,ex2.size,1);
        Mat out2(codedImg2->at(0).size(), codedImg2->size(), CV_8UC3);
        for (int y=0; y<codedImg2->at(0).size(); y++)
            for (int x=0; x<codedImg2->size(); x++)
            {
                Rect win;
                win.x=x;
                win.y=y;
                win.width=1;
                win.height=1;
                vector<float>* featureVector = bovw.getPooledDescFastSkip(codedImg2,win,spatialPyramidsSingle,1);
                int greatest=-1;
                int max=0;
                for (int i=0; i<featureVector->size(); i++)
                {
                    float f = featureVector->at(i);
                    if (f>max)
                    {
                        greatest=i;
                        max=f;
                    }
                }
                switch (greatest) {
//                case 0:
//                    out2.at<Vec3b>(y,x)=class0;
//                    break;
//                case 1:
//                    out2.at<Vec3b>(y,x)=class1;
//                    break;
//                case 2:
//                    out2.at<Vec3b>(y,x)=class2;
//                    break;
//                case 3:
//                    out2.at<Vec3b>(y,x)=class3;
//                    break;
                case 147:
                    out2.at<Vec3b>(y,x)=class1;
                    break;
                case 147+160:
                    out2.at<Vec3b>(y,x)=class2;
                    break;
                case 147+320:
                    out2.at<Vec3b>(y,x)=class3;
                    break;
                default:
                    out2.at<Vec3b>(y,x)=none;
                    break;
                }
                
            }
        imshow("composed2",out2);
        waitKey(1);
        
        vector<float>* exf = bovw.featurizeImage(ex);
        double score = bovw.compareImage(ex2,*exf);
        cout << "score  " << score << endl;
        
        
        vector<float>* featureVector2 = bovw.getPooledDescFastSkip(codedImg2,Rect(0,0,ex2.cols,ex2.rows),simpleSpatialPyramids,1);
        vector<float>* featureVector = bovw.getPooledDescFastSkip(codedImg,Rect(0,0,ex.cols,ex.rows),simpleSpatialPyramids,1);
        bovw.normalizeDesc(featureVector2);
        bovw.normalizeDesc(featureVector);
        double score2=0;
        
        for (int i=0; i<featureVector->size(); i++)
        {
            score2 += pow(featureVector->at(i)-featureVector2->at(i),2);
        }
        cout << "score2 " << score2 << endl;
        
//        float scoreWhole=0;
//        for (int i=0; i<featureVector->size()/3; i++)
//        {
//            scoreWhole += pow(featureVector->at(i)-featureVector2->at(i),2);
//        }
//        float score1Half=0;
//        for (int i=featureVector->size()/3; i<2*featureVector->size()/3; i++)
//        {
//            score1Half += pow(featureVector->at(i)-featureVector2->at(i),2);
//        }
//        float score2Half=0;
//        for (int i=2*featureVector->size()/3; i<featureVector->size(); i++)
//        {
//            score2Half += pow(featureVector->at(i)-featureVector2->at(i),2);
//        }
//        cout << "whole: " << scoreWhole << ", 1/2: " << score1Half << ", 2/2: " << score2Half << endl;
        float score1Half=0;
        for (int i=0; i<featureVector->size()/6; i++)
        {
            score1Half += pow(featureVector->at(i)-featureVector2->at(i),2);
        }
        float score2Half=0;
        for (int i=featureVector->size()/6; i<2*featureVector->size()/6; i++)
        {
            score2Half += pow(featureVector->at(i)-featureVector2->at(i),2);
            
        }
        float score1q=0;
        for (int i=2*featureVector->size()/6; i<3*featureVector->size()/6; i++)
        {
            score1q += pow(featureVector->at(i)-featureVector2->at(i),2);
        }
        float score2q=0;
        for (int i=3*featureVector->size()/6; i<4*featureVector->size()/6; i++)
        {
            score2q += pow(featureVector->at(i)-featureVector2->at(i),2);
            //cout << featureVector->at(i) << "\t" << featureVector2->at(i) << endl;
        }
        float score3q=0;
        for (int i=4*featureVector->size()/6; i<5*featureVector->size()/6; i++)
        {
            score3q += pow(featureVector->at(i)-featureVector2->at(i),2);
            //cout << featureVector->at(i) << "\t" << featureVector2->at(i) << endl;
        }
        float score4q=0;
        for (int i=5*featureVector->size()/6; i<6*featureVector->size()/6; i++)
        {
            score4q += pow(featureVector->at(i)-featureVector2->at(i),2);
            //cout << featureVector->at(i) << "\t" << featureVector2->at(i) << endl;
        }
        cout << "[1/2]: " << score1Half << ", [2/2]: " << score2Half << ", [1/4]: " << score1q << ", [2/4]: " << score2q << ", [3/4]: " << score3q << ", [4/4]: " << score4q << endl;
        
        
        
        waitKey();
    }
    else if (option.compare("testpreprocess")==0)
    {
        string imagePath = argv[2];
        Preprocessor pre(PP_BASELINE_CENTER | PP_BASELINE_NORMALIZE);
        Mat done = pre.process(imread(imagePath));
        imshow("prep",done);
        waitKey();
    }
    else if (option.compare("testhog")==0)
    {
        string imagePath = argv[2];
        HOG hog(0,18,14,16);
        hog.show(imread(imagePath,CV_LOAD_IMAGE_GRAYSCALE));
    }
    else
    {
        cout << "Error, no option" << endl;
    }
    
    
}

