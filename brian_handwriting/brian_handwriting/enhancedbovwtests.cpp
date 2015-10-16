#include "enhancedbovwtests.h"
#include "enhancedbovw.h"
#include <vector>
#include <regex>

/*
void EnhancedBoVWTests::experiment_dist(EnhancedBoVW &bovw, string locationCSVPath, string exemplarDirPath, string dataDirPath, int dataSize, int numExemplarsPer, string fileExt, bool skip)
{
    int numNodes=20;
//    MPI_Comm_size (MPI_COMM_WORLD,&numNodes);
    int iproc=1;
//    MPI_Comm_rank (MPI_COMM_WORLD,&iproc);
    
    
    
    //string locationCSVPath="data/IAM/bigramLocations_Almazan.csv";
    //string exemplarDirPath="data/IAM/Almazan_bigrams/";
    try{
//    regex imageNameNumExtract("(?:wordimg_)(\d+)");
    regex imageNameNumExtract("(?:wordimg_)(\\d+)");

//    string imageNameFormat="wordimg_%d.png";
    //string exemplarNameFormat="%s%03d.png";
    //string dataDirPath = "data/IAM/Almazan_words/";
//    int dataSize=4860;
//    int dataSize=13752;
//    int numExemplarsPer=10;
//    int numExemplarsPer=5;
    
    map<string,vector<int> > locations;
    
    ifstream file;
    file.open (locationCSVPath, ios::in);
    assert(file.is_open());
//    string line;
    
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
        while(regex_search(fileList,sm,imageNameNumExtract))
        {
            int idx = stoi(sm[1]);
            locations[ngram].push_back(idx);
            
//            for (auto x:sm) std::cout << x << " ";
//            std::cout << std::endl;
            fileList = sm.suffix().str();
        }
    }
    
    
    file.close();
    
    
//    map<> fullResults;
//    string fullResults="";
    
    double map=0;
    int mapCount=locations.size()*numExemplarsPer;
    assert(numNodes==locations.size());
    
    
    auto iter = locations.begin();
    for (int i=0; i<iproc; i++)
        iter++;
    const auto &ngramLocPair = *(iter);
//    for (const auto &ngramLocPair : locations)
    {
//        cout << "[" << iproc <<  "] on ngram: " <<  ngramLocPair.first << endl;
        string dirPath = exemplarDirPath + ngramLocPair.first + "/";
        const vector<int> &ngram_locations = locations[ngramLocPair.first];
        
//#pragma omp parallel for num_threads(2)
        for (int exemplarIdx=0; exemplarIdx<numExemplarsPer; exemplarIdx++)
        {
//            double startEx = omp_get_wtime();//clock();
            vector<pair<int,double> >scores;
            scores.reserve(dataSize-1);

            string imagePath=dirPath;
            if (exemplarIdx+1<100) imagePath+="0";
            if (exemplarIdx+1<10) imagePath+="0";
            imagePath+=to_string(exemplarIdx+1);
            imagePath+=".png";
            Mat exemplar = imread(imagePath);
            assert(exemplar.rows>0);
            vector<float>* exemplar_b = bovw.featurizeImage(exemplar);
            
            
            
            #pragma omp parallel
            {
                vector<pair<int,double> >myScores;//(dataSize);
                

                #pragma omp for nowait schedule(dynamic,200)
                for (int imageIdx=1; imageIdx<=dataSize; imageIdx++)
                {
    //                double startImg = clock();
                    if (skip && imageIdx == ngram_locations[exemplarIdx])
                        continue;
                    
                    string imagePath = dataDirPath + "wordimg_" + to_string(imageIdx) + fileExt;
                    Mat word = imread(imagePath);
                    assert(word.rows>0);
                    double score = bovw.scanImage(word,*exemplar_b,exemplar.size());
                    
                    
                    myScores.push_back(pair<int,double>(imageIdx,score));
//                    myScores[imageIdx-1] = (pair<int,double>(imageIdx,score));
    
    //                double endImg = clock();
    //                double durImg = (endImg-startImg)/(double)CLOCKS_PER_SEC;
    //                cout << "   image " << imageIdx << " took " << durImg << " seconds."<<endl;
                }
                
                #pragma omp critical
                scores.insert(scores.end(),myScores.begin(),myScores.end());
            }
            delete exmeplar_b;
            if (scores.size() != dataSize-1 && scores.size() != dataSize)
            {
                cout << "Error, scores size:" << scores.size() << ", dataSize:" << dataSize << endl;
                assert(false);
            }
            sort(scores.begin(),scores.end(),[](const pair<int,double> &l, const pair<int,double> &r)->bool {return l.second < r.second;});
//            fullResults(strcat(ngram,num2str(exemplarIdx)))=scores;
            

                
            
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
                    double precision = foundRelevent/(double)(top+1);
                    avgPrecision += precision;
                }
            }
            
            avgPrecision = avgPrecision/totalRelevent;
            
//#pragma omp critical
            {
//                fullResults += ngramLocPair.first+to_string(exemplarIdx+1) + "{" + to_string(scores[0].first);
//                for (int i=1; i<scores.size(); i++)
//                {
//                    fullResults += "," + to_string(scores[i].first);
//                }
//                fullResults += "}\n";
                
                map += avgPrecision;
//                mapCount++;
            }
            
//            double endEx = omp_get_wtime();//clock();
//            double durEx = (endEx-startEx);///(double)CLOCKS_PER_SEC;
//            cout << "[" << iproc <<  "] exemplar " << exemplarIdx << " took " << durEx << " seconds."<<endl;
        }
    }
    
    if (iproc == 0)
    {
//        MPI_Status stat;
        for (int i=1; i<numNodes; i++)
        {
//            double tmp;
//            MPI_Recv(&tmp,1,MPI_DOUBLE,i,1,MPI_COMM_WORLD,&stat);
//            map += tmp;
        }
        
        map = map/mapCount;
        cout << "mAP: "<<map<<endl;
    }
    else
    {
//        MPI_Send(&map,1,MPI_DOUBLE,0,1,MPI_COMM_WORLD);
        cout << map << endl;
    }
//    save("results.mat","fullResults","map");
//    ofstream out;
//    out.open ("./results_IAM_bigram.dat", ios::out);
//    out << fullResults;
//    out.close();
    
    } catch (std::regex_error& e) {
        cout << "regex error:"<<e.code() << endl;
        return;
    }
    
    
}

void EnhancedBoVWTests::experiment_Aldavert_dist(EnhancedBoVW &bovw, string locationCSVPath, string dataDirPath, int dataSize, string fileExt)
{
    int numNodes;
    MPI_Comm_size (MPI_COMM_WORLD,&numNodes);
    int iproc;
    MPI_Comm_rank (MPI_COMM_WORLD,&iproc);
    
    
    //string locationCSVPath="data/IAM/bigramLocations_Almazan.csv";
    //string exemplarDirPath="data/IAM/Almazan_bigrams/";
    try{
//    regex imageNameNumExtract("(?:wordimg_)(\d+)");
    regex imageNameNumExtract("\\d+");

//    string imageNameFormat="wordimg_%d.png";
    //string exemplarNameFormat="%s%03d.png";
    //string dataDirPath = "data/IAM/Almazan_words/";
//    int dataSize=4860;
//    int dataSize=13752;
//    int numExemplarsPer=10;
//    int numExemplarsPer=5;
    
    map<string,vector<int> > locations;
    
    ifstream file;
    file.open (locationCSVPath, ios::in);
    assert(file.is_open());
//    string line;
    
    string wordText;
//    getline(file,ngram);
    string fileList;
//    getline(file,fileList);
    
//    smatch sm;
//    regex_match(fileList,sm,imageNameNumExtract);
//    for (unsigned i=0; i<sm.size(); ++i) {
//        int idx = stoi(sm[i]);
//        locations[ngram].push_back(idx);
//    }
    
    while (getline(file,wordText))
    {
        getline(file,fileList);
        
        smatch sm;
        while(regex_search(fileList,sm,imageNameNumExtract))
        {
            int idx = stoi(sm[0]);
            locations[wordText].push_back(idx);
            
//            for (auto x:sm) std::cout << x << " ";
//            std::cout << std::endl;
            fileList = sm.suffix().str();
        }
    }
    
    
    file.close();
    
    
//    map<> fullResults;
//    string fullResults="";
    
    double map=0;
    int mapCount=0;
    
    int taskSize = locations.size() / numNodes;
    int mybegin = iproc * taskSize;
    int myend = (iproc+1) * taskSize;
    if (myend > locations.size())
        myend = locations.size();
    if (iproc+1 == numNodes)
        myend = locations.size();
    
    auto iter = locations.begin();
    for (int i=0; i<mybegin; i++)
        iter++;
    
//    for (const auto &wordTextLocPair : locations)
    for (int i=mybegin; i<myend; i++, iter++)
    {
        const auto &wordTextLocPair = *(iter);
//        cout << "on word: " <<  wordTextLocPair.first << endl;
        
        const vector<int> &word_locations = locations[wordTextLocPair.first];
        
//#pragma omp parallel for num_threads(2)
        if (word_locations.size()>1)
        for (int exemplarIdx=0; exemplarIdx<word_locations.size(); exemplarIdx++)
        {
//            double startEx = omp_get_wtime();//clock();
            vector<pair<int,double> >scores;
            scores.reserve(dataSize-1);

            string imagePath = dataDirPath + "wordimg_" + to_string(word_locations[exemplarIdx]) + fileExt;
            Mat exemplar = imread(imagePath);
            assert(exemplar.rows>0);
            vector<float>* exemplar_b = bovw.featurizeImage(exemplar);
            
            
            
            #pragma omp parallel
            {
                vector<pair<int,double> >myScores;//(dataSize);
                

                #pragma omp for nowait schedule(dynamic,200)
                for (int imageIdx=1; imageIdx<=dataSize; imageIdx++)
                {
    //                double startImg = clock();
                    if (imageIdx == word_locations[exemplarIdx])
                        continue;
                    
                    string imagePath = dataDirPath + "wordimg_" + to_string(imageIdx) + fileExt;
                    Mat word = imread(imagePath);
                    assert(word.rows>0);
                    double score = bovw.scanImage(word,*exemplar_b,exemplar.size());
                    
                    
                    myScores.push_back(pair<int,double>(imageIdx,score));
//                    myScores[imageIdx-1] = (pair<int,double>(imageIdx,score));
    
    //                double endImg = clock();
    //                double durImg = (endImg-startImg)/(double)CLOCKS_PER_SEC;
    //                cout << "   image " << imageIdx << " took " << durImg << " seconds."<<endl;
                }
                
                #pragma omp critical
                scores.insert(scores.end(),myScores.begin(),myScores.end());
            }
            delete exemplar_b;
//            if (scores.size() != dataSize-1 && scores.size() != dataSize)
//            {
//                cout << "Error, scores size:" << scores.size() << ", dataSize:" << dataSize << endl;
//                assert(false);
//            }
            sort(scores.begin(),scores.end(),[](const pair<int,double> &l, const pair<int,double> &r)->bool {return l.second < r.second;});
//            fullResults(strcat(ngram,num2str(exemplarIdx)))=scores;
            

                
            
            //compute average precision
            int foundRelevent = 0;
            double avgPrecision = 0.0;
            int totalRelevent=word_locations.size();
            
            for (int top=0; top<scores.size(); top++)
            {
                int ii = scores[top].first;
                
                if (find(word_locations.begin(),word_locations.end(),ii)!=word_locations.end())
                {
                    foundRelevent++;
                    double precision = foundRelevent/(double)(top+1);
                    avgPrecision += precision;
                }
            }
            
            avgPrecision = avgPrecision/totalRelevent;
            
//#pragma omp critical
            {
//                fullResults += wordTextLocPair.first+to_string(exemplarIdx+1) + "{" + to_string(scores[0].first);
//                for (int i=1; i<scores.size(); i++)
//                {
//                    fullResults += "," + to_string(scores[i].first);
//                }
//                fullResults += "}\n";
                
                map += avgPrecision;
                mapCount++;
            }
            
//            double endEx = omp_get_wtime();//clock();
//            double durEx = (endEx-startEx);///(double)CLOCKS_PER_SEC;
//            cout << "exemplar " << exemplarIdx << " took " << durEx << " seconds."<<endl;
        }
//        break;
    }
    
    
    if (iproc == 0)
    {
        MPI_Status stat;
        for (int i=1; i<numNodes; i++)
        {
            double tmp;
            int tmp2;
            MPI_Recv(&tmp,1,MPI_DOUBLE,i,1,MPI_COMM_WORLD,&stat);
            MPI_Recv(&tmp2,1,MPI_INT,i,2,MPI_COMM_WORLD,&stat);
            map += tmp;
            mapCount += tmp2;
        }
        
        map = map/mapCount;
        cout << "mAP: "<<map<<endl;
    }
    else
    {
        MPI_Request req;
        MPI_Isend(&map,1,MPI_DOUBLE,0,1,MPI_COMM_WORLD,&req);
        MPI_Send(&mapCount,1,MPI_INT,0,2,MPI_COMM_WORLD);
    }
//    save("results.mat","fullResults","map");
//    ofstream out;
//    out.open (outfile, ios::out);
//    out << fullResults;
//    out.close();
    
    } catch (std::regex_error& e) {
        cout << "regex error:"<<e.code() << endl;
        return;
    }
}
*/

void EnhancedBoVWTests::experiment(EnhancedBoVW &bovw, string locationCSVPath, string exemplarDirPath, string dataDirPath, int dataSize, int numExemplarsPer, string fileExt, bool skip)
{
    
    
    
    //string locationCSVPath="data/IAM/bigramLocations_Almazan.csv";
    //string exemplarDirPath="data/IAM/Almazan_bigrams/";
    try{
//    regex imageNameNumExtract("(?:wordimg_)(\d+)");
    regex imageNameNumExtract("(?:wordimg_)(\\d+)");

//    string imageNameFormat="wordimg_%d.png";
    //string exemplarNameFormat="%s%03d.png";
    //string dataDirPath = "data/IAM/Almazan_words/";
//    int dataSize=4860;
//    int dataSize=13752;
//    int numExemplarsPer=10;
//    int numExemplarsPer=5;
    
    map<string,vector<int> > locations;
    
    ifstream file;
    file.open (locationCSVPath, ios::in);
    assert(file.is_open());
//    string line;
    
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
        while(regex_search(fileList,sm,imageNameNumExtract))
        {
            int idx = stoi(sm[1]);
            locations[ngram].push_back(idx);
            
//            for (auto x:sm) std::cout << x << " ";
//            std::cout << std::endl;
            fileList = sm.suffix().str();
        }
    }
    
    
    file.close();
    
    
//    map<> fullResults;
    string fullResults="";
    
    double map=0;
    int mapCount=0;
    
    
//    int mpi_id, mpi_size;
//    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_id);
//    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
    
    for (const auto &ngramLocPair : locations)
    {
        cout << "on ngram: " <<  ngramLocPair.first << endl;
        string dirPath = exemplarDirPath + ngramLocPair.first + "/";
        const vector<int> &ngram_locations = locations[ngramLocPair.first];
        
//#pragma omp parallel for num_threads(2)
        for (int exemplarIdx=0; exemplarIdx<numExemplarsPer; exemplarIdx++)
        {
            double startEx = omp_get_wtime();//clock();
            vector<pair<int,double> >scores;
            scores.reserve(dataSize-1);

            string imagePath=dirPath;
            if (exemplarIdx+1<100) imagePath+="0";
            if (exemplarIdx+1<10) imagePath+="0";
            imagePath+=to_string(exemplarIdx+1);
            imagePath+=".png";
            Mat exemplar = imread(imagePath,CV_LOAD_IMAGE_GRAYSCALE);
            assert(exemplar.rows>0);
            vector<float>* exemplar_b = bovw.featurizeImage(exemplar);
            
            
            
            #pragma omp parallel
            {
                vector<pair<int,double> >myScores;//(dataSize);
                

                #pragma omp for nowait schedule(dynamic,200)
                for (int imageIdx=1; imageIdx<=dataSize; imageIdx++)
                {
    //                double startImg = clock();
                    if (skip && imageIdx == ngram_locations[exemplarIdx])
                        continue;
                    
                    string imagePath = dataDirPath + "wordimg_" + to_string(imageIdx) + fileExt;
                    Mat word = imread(imagePath,CV_LOAD_IMAGE_GRAYSCALE);
                    assert(word.rows>0);
                    double score = bovw.scanImage(word,*exemplar_b,exemplar.size());
                    
                    
                    myScores.push_back(pair<int,double>(imageIdx,score));
//                    myScores[imageIdx-1] = (pair<int,double>(imageIdx,score));
    
    //                double endImg = clock();
    //                double durImg = (endImg-startImg)/(double)CLOCKS_PER_SEC;
    //                cout << "   image " << imageIdx << " took " << durImg << " seconds."<<endl;
                }
                
                #pragma omp critical
                scores.insert(scores.end(),myScores.begin(),myScores.end());
            }
            delete exemplar_b;
            if (scores.size() != dataSize-1 && scores.size() != dataSize)
            {
                cout << "Error, scores size:" << scores.size() << ", dataSize:" << dataSize << endl;
                assert(false);
            }
            sort(scores.begin(),scores.end(),[](const pair<int,double> &l, const pair<int,double> &r)->bool {return l.second < r.second;});
//            fullResults(strcat(ngram,num2str(exemplarIdx)))=scores;
            

                
            
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
                    double precision = foundRelevent/(double)(top+1);
                    avgPrecision += precision;
                }
            }
            
            avgPrecision = avgPrecision/totalRelevent;
            
//#pragma omp critical
            {
                fullResults += ngramLocPair.first+to_string(exemplarIdx+1) + "{" + to_string(scores[0].first);
                for (int i=1; i<scores.size(); i++)
                {
                    fullResults += "," + to_string(scores[i].first);
                }
                fullResults += "}\n";
                
                map += avgPrecision;
                mapCount++;
            }
            
            double endEx = omp_get_wtime();//clock();
            double durEx = (endEx-startEx);///(double)CLOCKS_PER_SEC;
            cout << "exemplar " << exemplarIdx << " took " << durEx << " seconds."<<endl;
        }
//        break;
    }
    map = map/mapCount;
    cout << "[IAM/bigram] mAP: "<<map<<endl;
//    save("results.mat","fullResults","map");
    ofstream out;
    out.open ("./results_IAM_bigram.dat", ios::out);
    out << fullResults;
    out.close();
    
    } catch (std::regex_error& e) {
        cout << "regex error:"<<e.code() << endl;
        return;
    }
}

/*
void deslant(Mat &img)
{
    DImage img1;
    img1.setLogicalSize(img.cols,img.rows);
    unsigned char* data1 = img1.dataPointer_u8();
    for (int i=0; i< img.cols * img.rows; i++)
    {
        data1[i]=img.data[i];
    }
    DImage img2;
    img2.setLogicalSize(img.cols,img.rows);
    unsigned char* data2 = img2.dataPointer_u8();
    for (int i=0; i< img.cols * img.rows; i++)
    {
        data2[i]=img.data[i]>150?0:255;
    }
    
    double slant = DSlantAngle::getTextlineSlantAngleDeg(img2,3);
    std::cout << "slant: " << slant << std::endl;
    img1 = img1.shearedH(slant,250,false);
//    unsigned char* data3 = img1.dataPointer_u8();
    
    
    
    for (int ii=0; ii< img1.width(); ii++)
        for (int jj=0; jj< img1.height(); jj++)
            img.data[ii+(jj)*img.cols]=data1[ii+jj*img1.width()];
}
*/


void EnhancedBoVWTests::experiment_Aldavert(EnhancedBoVW &bovw, string locationCSVPath, string dataDirPath, int dataSize, string fileExt, string outfile)
{
    
    
    
    //string locationCSVPath="data/IAM/bigramLocations_Almazan.csv";
    //string exemplarDirPath="data/IAM/Almazan_bigrams/";
    try{
//    regex imageNameNumExtract("(?:wordimg_)(\d+)");
    regex imageNameNumExtract("\\d+");

//    string imageNameFormat="wordimg_%d.png";
    //string exemplarNameFormat="%s%03d.png";
    //string dataDirPath = "data/IAM/Almazan_words/";
//    int dataSize=4860;
//    int dataSize=13752;
//    int numExemplarsPer=10;
//    int numExemplarsPer=5;
    
    map<string,vector<int> > locations;
    
    ifstream file;
    file.open (locationCSVPath, ios::in);
    assert(file.is_open());
//    string line;
    
    string wordText;
//    getline(file,ngram);
    string fileList;
//    getline(file,fileList);
    
//    smatch sm;
//    regex_match(fileList,sm,imageNameNumExtract);
//    for (unsigned i=0; i<sm.size(); ++i) {
//        int idx = stoi(sm[i]);
//        locations[ngram].push_back(idx);
//    }
    
    while (getline(file,wordText))
    {
        getline(file,fileList);
        
        smatch sm;
        while(regex_search(fileList,sm,imageNameNumExtract))
        {
            int idx = stoi(sm[0]);
            locations[wordText].push_back(idx);
            
//            for (auto x:sm) std::cout << x << " ";
//            std::cout << std::endl;
            fileList = sm.suffix().str();
        }
    }
    
    
    file.close();
    
    
//    map<> fullResults;
    string fullResults="";
    
    double map=0;
    int mapCount=0;
    
    
//    int mpi_id, mpi_size;
//    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_id);
//    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
    
    for (const auto &wordTextLocPair : locations)
    {
//        cout << "on word: " <<  wordTextLocPair.first << endl;
        
        const vector<int> &word_locations = locations[wordTextLocPair.first];
        
//#pragma omp parallel for num_threads(2)
        if (word_locations.size()>1)
        for (int exemplarIdx=0; exemplarIdx<word_locations.size(); exemplarIdx++)
        {
//            double startEx = omp_get_wtime();//clock();
            vector<pair<int,double> >scores;
            scores.reserve(dataSize-1);

            string imagePath = dataDirPath + "wordimg_" + to_string(word_locations[exemplarIdx]) + fileExt;
            Mat exemplar = imread(imagePath,CV_LOAD_IMAGE_GRAYSCALE);
            assert(exemplar.rows>0);
            vector<float>* exemplar_b = bovw.featurizeImage(exemplar);
            
            
            
            #pragma omp parallel
            {
                vector<pair<int,double> >myScores;//(dataSize);
                

                #pragma omp for nowait schedule(dynamic,200)
                for (int imageIdx=1; imageIdx<=dataSize; imageIdx++)
                {
    //                double startImg = clock();
                    if (imageIdx == word_locations[exemplarIdx])
                        continue;
                    
                    string imagePath = dataDirPath + "wordimg_" + to_string(imageIdx) + fileExt;
                    Mat word = imread(imagePath,CV_LOAD_IMAGE_GRAYSCALE);
                    assert(word.rows>0);
                    double score = bovw.compareImage(word,*exemplar_b);
                    
                    
                    myScores.push_back(pair<int,double>(imageIdx,score));
//                    myScores[imageIdx-1] = (pair<int,double>(imageIdx,score));
    
    //                double endImg = clock();
    //                double durImg = (endImg-startImg)/(double)CLOCKS_PER_SEC;
    //                cout << "   image " << imageIdx << " took " << durImg << " seconds."<<endl;
                }
                
                #pragma omp critical
                scores.insert(scores.end(),myScores.begin(),myScores.end());
            }
            delete exemplar_b;
            
//            if (scores.size() != dataSize-1 && scores.size() != dataSize)
//            {
//                cout << "Error, scores size:" << scores.size() << ", dataSize:" << dataSize << endl;
//                assert(false);
//            }
            sort(scores.begin(),scores.end(),[](const pair<int,double> &l, const pair<int,double> &r)->bool {return l.second < r.second;});
//            fullResults(strcat(ngram,num2str(exemplarIdx)))=scores;
            

                
            
            //compute average precision
            int foundRelevent = 0;
            double avgPrecision = 0.0;
            int totalRelevent=word_locations.size();
            
            for (int top=0; top<scores.size(); top++)
            {
                int ii = scores[top].first;
                
                if (find(word_locations.begin(),word_locations.end(),ii)!=word_locations.end())
                {
                    foundRelevent++;
                    double precision = foundRelevent/(double)(top+1);
                    avgPrecision += precision;
                }
            }
            
            avgPrecision = avgPrecision/totalRelevent;
            
//#pragma omp critical
            {
                fullResults += wordTextLocPair.first+to_string(exemplarIdx+1) + "{" + to_string(scores[0].first);
                for (int i=1; i<scores.size(); i++)
                {
                    fullResults += "," + to_string(scores[i].first);
                }
                fullResults += "}\n";
                
                map += avgPrecision;
                mapCount++;
            }
            
//            double endEx = omp_get_wtime();//clock();
//            double durEx = (endEx-startEx);///(double)CLOCKS_PER_SEC;
//            cout << "exemplar " << exemplarIdx << " took " << durEx << " seconds."<<endl;
        }
//        break;
    }
    map = map/mapCount;
    cout << "mAP: "<<map<<endl;
//    save("results.mat","fullResults","map");
    ofstream out;
    out.open (outfile, ios::out);
    out << fullResults;
    out.close();
    
    } catch (std::regex_error& e) {
        cout << "regex error:"<<e.code() << endl;
        return;
    }
}

void EnhancedBoVWTests::experiment_Aldavert_dist_batched(EnhancedBoVW &bovw, string locationCSVPath, string dataDirPath, int dataSize, string fileExt, int batchNum, int numOfBatches, string outfile)
{
    int numNodes=numOfBatches;
    //MPI_Comm_size (MPI_COMM_WORLD,&numNodes);
    int iproc=batchNum;
    //MPI_Comm_rank (MPI_COMM_WORLD,&iproc);
    
    
    //string locationCSVPath="data/IAM/bigramLocations_Almazan.csv";
    //string exemplarDirPath="data/IAM/Almazan_bigrams/";
    try{
//    regex imageNameNumExtract("(?:wordimg_)(\d+)");
    regex imageNameNumExtract("\\d+");

//    string imageNameFormat="wordimg_%d.png";
    //string exemplarNameFormat="%s%03d.png";
    //string dataDirPath = "data/IAM/Almazan_words/";
//    int dataSize=4860;
//    int dataSize=13752;
//    int numExemplarsPer=10;
//    int numExemplarsPer=5;
    
    map<string,vector<int> > locations;
    
    ifstream file;
    file.open (locationCSVPath, ios::in);
    assert(file.is_open());
//    string line;
    
    string wordText;
//    getline(file,ngram);
    string fileList;
//    getline(file,fileList);
    
//    smatch sm;
//    regex_match(fileList,sm,imageNameNumExtract);
//    for (unsigned i=0; i<sm.size(); ++i) {
//        int idx = stoi(sm[i]);
//        locations[ngram].push_back(idx);
//    }
    
    while (getline(file,wordText))
    {
        getline(file,fileList);
        
        smatch sm;
        while(regex_search(fileList,sm,imageNameNumExtract))
        {
            int idx = stoi(sm[0]);
            locations[wordText].push_back(idx);
            
//            for (auto x:sm) std::cout << x << " ";
//            std::cout << std::endl;
            fileList = sm.suffix().str();
        }
    }
    
    
    file.close();
    
    
//    map<> fullResults;
    string fullResults="";
    
    double map=0;
    double prevMap=0;
    int mapCount=0;
    int prevCount=0;
    
    int taskSize = locations.size() / numNodes;
    int mybegin = iproc * taskSize;
    int myend = (iproc+1) * taskSize;
    if (myend > locations.size())
        myend = locations.size();
    if (iproc+1 == numNodes)
        myend = locations.size();   

    cout << "["<<iproc<<"] is going from "<<mybegin<<" to "<<myend<<endl; 
    
    auto iter = locations.begin();
    for (int i=0; i<mybegin; i++)
        iter++;
    
//    for (const auto &wordTextLocPair : locations)
    for (int i=mybegin; i<myend; i++, iter++)
    {
        const auto &wordTextLocPair = *(iter);
        
        const vector<int> &word_locations = locations[wordTextLocPair.first];
        
//#pragma omp parallel for num_threads(2)
        if (word_locations.size()>9 && wordTextLocPair.first.size()>2)
        {
        cout << "["<<iproc<<"] on word["<<i<<"]: " <<  wordTextLocPair.first << endl;
        for (int exemplarIdx=0; exemplarIdx<word_locations.size(); exemplarIdx++)
        {
//            double startEx = omp_get_wtime();//clock();
            vector<pair<int,double> >scores;
            //scores.reserve(dataSize-1);

            std::string imagePath = dataDirPath + "wordimg_" + to_string(word_locations[exemplarIdx]) + fileExt;
            Mat exemplar = imread(imagePath,CV_LOAD_IMAGE_GRAYSCALE);
            assert(exemplar.rows>0);
            vector<float>* exemplar_b = bovw.featurizeImage(exemplar);
            
            
            
            #pragma omp parallel
            {
                vector<pair<int,double> >myScores;//(dataSize);
                

                #pragma omp for nowait //schedule(dynamic,200)
                for (int imageIdx=1; imageIdx<=dataSize; imageIdx++)
                {
                    //double startImg = clock();
                    if (imageIdx == word_locations[exemplarIdx])
                        continue;
                    
                    std::string imagePath = dataDirPath + "wordimg_" + to_string(imageIdx) + fileExt;
                    Mat word = imread(imagePath,CV_LOAD_IMAGE_GRAYSCALE);
                    assert(word.rows>0);
                    double score = bovw.compareImage(word,*exemplar_b);
                    
                    
                    myScores.push_back(pair<int,double>(imageIdx,score));
//                    myScores[imageIdx-1] = (pair<int,double>(imageIdx,score));
    
                    //double endImg = clock();
                    //double durImg = (endImg-startImg)/(double)CLOCKS_PER_SEC;
                    //cout << " ex " << exemplarIdx << ",  image " << imageIdx << " took " << durImg << " seconds."<<endl;
                }
                
                #pragma omp critical
                scores.insert(scores.end(),myScores.begin(),myScores.end());
            }
            delete exemplar_b;
//            if (scores.size() != dataSize-1 && scores.size() != dataSize)
//            {
//                cout << "Error, scores size:" << scores.size() << ", dataSize:" << dataSize << endl;
//                assert(false);
//            }
            sort(scores.begin(),scores.end(),[](const pair<int,double> &l, const pair<int,double> &r)->bool {return l.second < r.second;});
//            fullResults(strcat(ngram,num2str(exemplarIdx)))=scores;
            

                
            
            //compute average precision
            int foundRelevent = 0;
            double avgPrecision = 0.0;
            int totalRelevent=word_locations.size()-1;
            
            for (int top=0; top<scores.size(); top++)
            {
                int ii = scores[top].first;
                if (top==0)
                {
                    cout << "top match is " << ii << endl;
                }
                if (find(word_locations.begin(),word_locations.end(),ii)!=word_locations.end())
                {
                    foundRelevent++;
                    double precision = foundRelevent/(double)(top+1);
                    avgPrecision += precision;
                }
            }
            
            avgPrecision = avgPrecision/totalRelevent;
            
//#pragma omp critical
            {
                fullResults += wordTextLocPair.first+"_"+to_string(exemplarIdx+1) + "{" + to_string(scores[0].first);
                for (int i=1; i<scores.size(); i++)
                {
                    fullResults += "," + to_string(scores[i].first);
                }
                fullResults += "}\n";
                
                map += avgPrecision;
                mapCount++;
            }
            
//            double endEx = omp_get_wtime();//clock();
//            double durEx = (endEx-startEx);///(double)CLOCKS_PER_SEC;
//            cout << "[" << iproc << "] exemplar " << exemplarIdx << " took " << durEx << " seconds."<<endl;
        }
        
        cout << "finished word " << i << " with " << (map-prevMap)/(mapCount-prevCount) << " mAP" << endl;
        prevMap=map;
        prevCount=mapCount;
        }
//        break;
    }
    
    
    /*if (iproc == 0)
    {
        MPI_Status stat;
        for (int i=1; i<numNodes; i++)
        {
            double tmp;
            int tmp2;
            MPI_Recv(&tmp,1,MPI_DOUBLE,i,1,MPI_COMM_WORLD,&stat);
            MPI_Recv(&tmp2,1,MPI_INT,i,2,MPI_COMM_WORLD,&stat);
            map += tmp;
            mapCount += tmp2;
        }
        
        map = map/mapCount;
        cout << "mAP: "<<map<<endl;
    }
    else
    {
        MPI_Request req;
        MPI_Isend(&map,1,MPI_DOUBLE,0,1,MPI_COMM_WORLD,&req);
        MPI_Send(&mapCount,1,MPI_INT,0,2,MPI_COMM_WORLD);
    }*/
    cout << "for batch " << batchNum << " mAP sum:"<<map<< " count:" << mapCount << endl;
    //save("results"+iproc=".mat","fullResults","map","mapCount");
    ofstream out;
    out.open (outfile+to_string(iproc), ios::out);
    out << fullResults;
    out.close();
    
    } catch (std::regex_error& e) {
        cout << "regex error:"<<e.code() << endl;
        return;
    }
}


void EnhancedBoVWTests::experiment_Aldavert_dist_batched_test(int scenario)
{
    int numNodes=1;
    int iproc=0;

    try{
    regex imageNameNumExtract("\\d+");

    int dataSize=40;
    
    map<string,vector<int> > locations;
    for (int i=0; i<10; i++)
        locations["first"].push_back(i+1);
    for (int i=0; i<10; i++)
        locations["second"].push_back(10+i+1);
    for (int i=0; i<10; i++)
        locations["third"].push_back(20+i+1);
    for (int i=0; i<10; i++)
        locations["fourth"].push_back(30+i+1);
    
    
//    map<> fullResults;
    string fullResults="";
    
    double map=0;
    double prevMap=0;
    int mapCount=0;
    
    int taskSize = locations.size() / numNodes;
    int mybegin = iproc * taskSize;
    int myend = (iproc+1) * taskSize;
    if (myend > locations.size())
        myend = locations.size();
    if (iproc+1 == numNodes)
        myend = locations.size();   

    //cout << "["<<iproc<<"] is going from "<<mybegin<<" to "<<myend<<endl; 
    
    auto iter = locations.begin();
    for (int i=0; i<mybegin; i++)
        iter++;
    
    for (int i=mybegin; i<myend; i++, iter++)
    {
        const auto &wordTextLocPair = *(iter);
        
        const vector<int> &word_locations = locations[wordTextLocPair.first];
        
        if (word_locations.size()>9 && wordTextLocPair.first.size()>2)
        {
        //cout << "["<<iproc<<"] on word["<<i<<"]: " <<  wordTextLocPair.first << endl;
        for (int exemplarIdx=0; exemplarIdx<word_locations.size(); exemplarIdx++)
        {
            vector<pair<int,double> >scores;
            
            
            
            {
                vector<pair<int,double> >myScores;//(dataSize);
                

                #pragma omp for nowait //schedule(dynamic,200)
                for (int imageIdx=1; imageIdx<=dataSize; imageIdx++)
                {
                    //double startImg = clock();
                    if (imageIdx == word_locations[exemplarIdx])
                        continue;
 
                    double score;
                    if (scenario==0)
                    {
                        if ((wordTextLocPair.first.compare("first")==0 && imageIdx<11) ||
                                (wordTextLocPair.first.compare("second")==0 && 11<=imageIdx && imageIdx<21) ||
                                (wordTextLocPair.first.compare("third")==0 && 21<=imageIdx && imageIdx<31) ||
                                (wordTextLocPair.first.compare("fourth")==0 && 31<=imageIdx && imageIdx<41))
                        {
                            score= .1;
                        }
                        else
                        {
                            score = .5;
                        }
                    }
                    else if (scenario==1)
                    {
                        if ((wordTextLocPair.first.compare("first")==0 && imageIdx<11) ||
                                (wordTextLocPair.first.compare("second")==0 && 11<=imageIdx && imageIdx<21) ||
                                (wordTextLocPair.first.compare("third")==0 && 21<=imageIdx && imageIdx<31) 
                                )
                        {
                            score= .1;
                        }
                        else if (wordTextLocPair.first.compare("fourth")==0 && 31<=imageIdx && imageIdx<41)
                        {
                            score = .9;
                        }
                        else
                        {
                            score = .5;
                        }
                    }
                    else if (scenario==2)
                    {
                        if ((wordTextLocPair.first.compare("first")==0 && imageIdx<11) ||
                                (wordTextLocPair.first.compare("second")==0 && 11<=imageIdx && imageIdx<21) ||
                                (wordTextLocPair.first.compare("third")==0 && 21<=imageIdx && imageIdx<31) ||
                                (wordTextLocPair.first.compare("fourth")==0 && 31<=imageIdx && imageIdx<36))
                        {
                            score= .1;
                        }
                        else if (wordTextLocPair.first.compare("fourth")==0 && 36<=imageIdx && imageIdx<41)
                        {
                            score = .9;
                        }
                        else
                        {
                            score = .5;
                        }
                    }
                    else if (scenario==3)
                    {
                        if ((wordTextLocPair.first.compare("first")==0 && imageIdx<6) ||
                                (wordTextLocPair.first.compare("second")==0 && 11<=imageIdx && imageIdx<16) ||
                                (wordTextLocPair.first.compare("third")==0 && 21<=imageIdx && imageIdx<26) ||
                                (wordTextLocPair.first.compare("fourth")==0 && 31<=imageIdx && imageIdx<36))
                        {
                            score= .1;
                        }
                        else if ((wordTextLocPair.first.compare("first")==0 && 6<=imageIdx && imageIdx<11) ||
                                 (wordTextLocPair.first.compare("second")==0 && 16<=imageIdx && imageIdx<21) ||
                                 (wordTextLocPair.first.compare("third")==0 && 26<=imageIdx && imageIdx<31) ||
                            (wordTextLocPair.first.compare("fourth")==0 && 36<=imageIdx && imageIdx<41))
                        {
                            score = .9;
                        }
                        else
                        {
                            score = .5;
                        }
                    }
                    else if (scenario==4)
                    {
                        score = rand();
                    }
                    
                    
                    myScores.push_back(pair<int,double>(imageIdx,score));
                }
                
                scores.insert(scores.end(),myScores.begin(),myScores.end());
            }
            sort(scores.begin(),scores.end(),[](const pair<int,double> &l, const pair<int,double> &r)->bool {return l.second < r.second;});
            

                
            
            //compute average precision
            int foundRelevent = 0;
            double avgPrecision = 0.0;
            int totalRelevent=word_locations.size()-1;
            
            for (int top=0; top<scores.size(); top++)
            {
                int ii = scores[top].first;
                
                if (find(word_locations.begin(),word_locations.end(),ii)!=word_locations.end())
                {
                    foundRelevent++;
                    double precision = foundRelevent/(double)(top+1);
                    avgPrecision += precision;
                }
            }
            
            avgPrecision = avgPrecision/totalRelevent;
            
            {
                fullResults += wordTextLocPair.first+"_"+to_string(exemplarIdx+1) + "{" + to_string(scores[0].first);
                for (int i=1; i<scores.size(); i++)
                {
                    fullResults += "," + to_string(scores[i].first);
                }
                fullResults += "}\n";
                
                map += avgPrecision;
                mapCount++;
            }
            
        }
        
        //cout << "finished word " << i << " with " << map-prevMap << " mAP" << endl;
        prevMap=map;
        }
    }
    
    

    //cout << "mAP " << map/mapCount << " mAP sum:"<<map<< " count:" << mapCount << endl;
    if (scenario == 0)
        assert(map/mapCount == 1.0);
    else if (scenario == 1)
        assert(map/mapCount >= 0.75);
    else if (scenario == 2)
        assert(map/mapCount > 0.75);
    else if (scenario == 3)
        assert(map/mapCount >= 0.5);
    else if (scenario == 4)
        assert(map/mapCount < 0.33);
    
    } catch (std::regex_error& e) {
        cout << "regex error:"<<e.code() << endl;
        return;
    }
}


void EnhancedBoVWTests::test(EnhancedBoVW& bovw)
{
    bovw.codebook = new Codebook();
    bovw.codebook->readIn("/home/brian/intel_index/data/gw_20p_wannot/codebook_deslant.dat");
    string locationCSVPath = "/home/brian/intel_index/data/gw_20p_wannot/C_wordLocations.csv";
    string dataDirPath = "/home/brian/intel_index/data/gw_20p_wannot/deslant/";
    string fileExt = ".png";
    regex imageNameNumExtract("\\d+");
    
    map<string,vector<int> > locations;
    
    ifstream file;
    file.open (locationCSVPath, ios::in);
    assert(file.is_open());
    
    string wordText;
    string fileList;

    
    while (getline(file,wordText))
    {
        getline(file,fileList);
        
        smatch sm;
        while(regex_search(fileList,sm,imageNameNumExtract))
        {
            int idx = stoi(sm[0]);
            locations[wordText].push_back(idx);
            
//            for (auto x:sm) std::cout << x << " ";
//            std::cout << std::endl;
            fileList = sm.suffix().str();
        }
    }
    
    
    file.close();
    
    double map=0;
    int mapCount=0;
    
    auto iter = locations.begin();
    int count=5;
    for (int i=0; count>0; i++, iter++)
    {
        const auto &wordTextLocPair = *(iter);
        
        const vector<int> &word_locations = locations[wordTextLocPair.first];
        
        if (word_locations.size()>9 && wordTextLocPair.first.size()>2)
        {
            count--;
        //cout << "["<<iproc<<"] on word["<<i<<"]: " <<  wordTextLocPair.first << endl;
        for (int exemplarIdx=0; exemplarIdx<std::min((int)5,(int)word_locations.size()); exemplarIdx++)
        {
            vector<pair<int,double> >scores;

            std::string imagePath = dataDirPath + "wordimg_" + to_string(word_locations[exemplarIdx]) + fileExt;
            Mat exemplar = imread(imagePath,CV_LOAD_IMAGE_GRAYSCALE);
            assert(exemplar.rows>0);
            vector<float>* exemplar_b = bovw.featurizeImage(exemplar);
            
            
            
            #pragma omp parallel
            {
                vector<pair<int,double> >myScores;//(dataSize);
                

                #pragma omp for nowait //schedule(dynamic,200)
                for (int imageIdx=1; imageIdx<=100; imageIdx++)
                {
                    //double startImg = clock();
                    if (imageIdx == word_locations[exemplarIdx])
                        continue;
                    
                    std::string imagePath = dataDirPath + "wordimg_" + to_string(imageIdx) + fileExt;
                    Mat word = imread(imagePath,CV_LOAD_IMAGE_GRAYSCALE);
                    assert(word.rows>0);
                    double score = bovw.compareImage(word,*exemplar_b);
                    
                    
                    myScores.push_back(pair<int,double>(imageIdx,score));
//                    myScores[imageIdx-1] = (pair<int,double>(imageIdx,score));
    
                    //double endImg = clock();
                    //double durImg = (endImg-startImg)/(double)CLOCKS_PER_SEC;
                    //cout << " ex " << exemplarIdx << ",  image " << imageIdx << " took " << durImg << " seconds."<<endl;
                }
                
                #pragma omp critical
                scores.insert(scores.end(),myScores.begin(),myScores.end());
            }
            delete exemplar_b;
//            if (scores.size() != dataSize-1 && scores.size() != dataSize)
//            {
//                cout << "Error, scores size:" << scores.size() << ", dataSize:" << dataSize << endl;
//                assert(false);
//            }
            sort(scores.begin(),scores.end(),[](const pair<int,double> &l, const pair<int,double> &r)->bool {return l.second < r.second;});
//            fullResults(strcat(ngram,num2str(exemplarIdx)))=scores;
            

                
            
            //compute average precision
            int foundRelevent = 0;
            double avgPrecision = 0.0;
            int totalRelevent=word_locations.size()-1;
            
            for (int top=0; top<scores.size(); top++)
            {
                int ii = scores[top].first;
                if (top==0)
                {
                    //cout << "top match is " << ii << endl;
                }
                if (find(word_locations.begin(),word_locations.end(),ii)!=word_locations.end())
                {
                    foundRelevent++;
                    double precision = foundRelevent/(double)(top+1);
                    avgPrecision += precision;
                }
            }
            
            avgPrecision = avgPrecision/totalRelevent;
            
//#pragma omp critical
            {
                cout << "average precision = "<<avgPrecision <<endl;
                map += avgPrecision;
                mapCount++;
            }
            
//            double endEx = omp_get_wtime();//clock();
//            double durEx = (endEx-startEx);///(double)CLOCKS_PER_SEC;
//            cout << "[" << iproc << "] exemplar " << exemplarIdx << " took " << durEx << " seconds."<<endl;
        }
        
        }
        
//        break;
    }
    cout << "mAP = " << map/mapCount << endl;
}
