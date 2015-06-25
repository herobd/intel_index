#include "CollabKeypoints.h"
#include "MatchKeypoints.h"
#include "corpus.h"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "bagspotter.h"
#include "spatialaveragespotter.h"
#include "enhancedbovw.h"

#define LOAD 0

void experiment(EnhancedBoVW &bovw);

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
//    matchKeypoints(argc,argv);
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

  
    //~/intel_index/data/enhancedBoVW/gw_20_cb.csv ~/intel_index/data/gw_20p_wannot/bigrams/an/002.png ~/intel_index/data/gw_20p_wannot/words/wordimg_4.tif
    
    Mat img=imread(argv[3],CV_LOAD_IMAGE_GRAYSCALE);
    Mat find=imread(argv[2],CV_LOAD_IMAGE_GRAYSCALE);
    assert(img.cols>0 && find.cols>0);
    
    EnhancedBoVW bovw;
    
//    bovw.makeCodebook(argv[1]);
//    bovw.codebook->save(string(argv[2]));
    
    bovw.codebook = new Codebook();
    bovw.codebook->readIn(argv[1]);
    
    bovw.scanImage(img,find);
    
//    experiment(bovw);
    
//    Codebook c;
//    c.readInCSV(argv[1]);
////    SpatialAverageSpotter b(&c);
//    BagSpotter b(&c);
//    b.train(argv[2]);
//    //b.testSpotting(argv[3]);
//    b.produceHeatMap(argv[3]);
    
    
}

#include <regex>

void experiment(EnhancedBoVW &bovw)
{
    string locationCSVPath="~/intel_index/data/gw_20p_wannot/bigramLocations.csv";
    string exemplarDirPath="~/intel_index/data/gw_20p_wannot/bigrams/";
    regex imageNameNumExtract("(?<=wordimg_)\d+");
    string imageNameFormat="wordimg_%d.tif";
    //string exemplarNameFormat="%s%03d.png";
    string dataDirPath = "~/intel_index/data/gw_20p_wannot/words/";
    int dataSize=4860;
    int numExemplarsPer=10;
    
    map<string,vector<int> > locations;
    
    ifstream file;
    file.open (locationCSVPath, ios::in);
    string line;
    
    string ngram;
//    getline(file,ngram);
    string fileList;
//    getline(file,fileList);
    
//    smatch sm;
//    regex_match(fileList,sm,imageNameNumExtract);
//    for (unsigned i=0; i<sm.size(); ++i) {
//        int idx = stoi(sm[i]);
//        locations[ngram].push_back(idx);
//    }
    
    while (getline(file,ngram))
    {
        getline(file,fileList);
        
        smatch sm;
        regex_match(fileList,sm,imageNameNumExtract);
        for (unsigned i=0; i<sm.size(); ++i) {
            int idx = stoi(sm[i]);
            locations[ngram].push_back(idx);
        }
    }
    
    
    file.close();
    
    
//    map<> fullResults;
    string fullResults="";
    
    double map=0;
    int mapCount=0;
    
    for (auto const ngramLocPair : locations)
    {
        
        string dirPath = exemplarDirPath + ngramLocPair.first + "/";
        const vector<int> &ngram_locations = locations[ngramLocPair.first];
        
        for (int exemplarIdx=0; exemplarIdx<numExemplarsPer; exemplarIdx++)
        {
            vector<pair<int,double> >scores(dataSize);
            
            string imagePath=dirPath;
            if (exemplarIdx<100) imagePath+="0";
            if (exemplarIdx<10) imagePath+="0";
            imagePath+=to_string(exemplarIdx);
            Mat exemplar = imread(imagePath);
            vector<float>* exemplar_b = bovw.featurizeImage(exemplar);
            
            for (int imageIdx=0; imageIdx<dataSize; imageIdx++)
            {
                if (imageIdx == ngram_locations[exemplarIdx])
                    continue;
                
                string imagePath = dataDirPath + "wordimg_" + to_string(imageIdx) + ".tif";
                Mat word = imread(imagePath);
                
                double score = bovw.scanImage(word,*exemplar_b,exemplar.size());
                scores.push_back(pair<int,double>(imageIdx,score));
                
                
            }
            
            sort(scores.begin(),scores.end(),[](const pair<int,double> &l, const pair<int,double> &r)->bool {return l.second < r.second;});
//            fullResults(strcat(ngram,num2str(exemplarIdx)))=scores;
            fullResults += ngram+to_string(exemplarIdx) + "{" + to_string(scores[0].first);
            for (int i=0; i<scores.size(); i++)
            {
                fullResults += "," + to_string(scores[i].first);
            }
            fullResults += "}\n";
                
            
            //compute average precision
            int foundRelevent = 0;
            double avgPrecision = 0.0;
            int totalRelevent=ngram_locations.size();
            
            for (int top=0; top<scores.size(); top++)
            {
                int ii = scores[top].first;
                
                if (find(ngram_locations.begin(),ngram_locations.end(),ii)!=ngram_locations.end())
                {
                    foundRelevent++;
                    double precision = foundRelevent/top;
                    avgPrecision += precision;
                }
            }
            avgPrecision = avgPrecision/totalRelevent;
            map += avgPrecision;
            mapCount++;
            
        }
    }
    map = map/mapCount;
    cout << "mAP: "<<map<<endl;
//    save("results.mat","fullResults","map");
    ofstream out;
    out.open ("./results.dat", ios::out);
    out << fullResults;
    out.close();
}
