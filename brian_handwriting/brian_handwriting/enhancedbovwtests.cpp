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

void EnhancedBoVWTests::experiment(EnhancedBoVW &bovw, string locationCSVPath, string exemplarDirPath, string dataDirPath, int dataSize, int numExemplarsPer, string fileExt, string outfile, bool skip)
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
                    double score = bovw.scanImageHorz(word,*exemplar_b,exemplar.size());
                    
                    
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
    cout << "[] mAP: "<<map<<endl;
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

void EnhancedBoVWTests::experiment_dist_batched(EnhancedBoVW &bovw, string locationCSVPath, string exemplarDirPath, string dataDirPath, int dataSize, int numExemplarsPer, string fileExt, int batchNum, int numOfBatches, string outfile, bool process)
{
    int numNodes=numOfBatches;
    int iproc=batchNum;
    try{
//    regex imageNameNumExtract("(?:wordimg_)(\d+)");
    regex imageNameNumExtract("\\d+");

    
    map<string,vector<int> > locations;
    
    ifstream file;
    file.open (locationCSVPath, ios::in);
    assert(file.is_open());
    
    string ngram;
    string fileList;
    

    
    int fileCount=0; 
    while (getline(file,ngram))
    {
        getline(file,fileList);
        
        smatch sm;
        while(regex_search(fileList,sm,imageNameNumExtract))
        {
            int idx = stoi(sm[0]);
            locations[ngram].push_back(idx);
            fileList = sm.suffix().str();
            fileCount++;
        }
    }
    
    
    file.close();
    
    
//    map<> fullResults;
    string fullResults="";
    
    double map=0;
    double prevMap=0;
    int mapCount=0;
    int prevCount=0;
    

    //Doing this by the fileCount rather than locations.size() keeps the work distributed more evenly.
    int taskSize = fileCount/numNodes; //locations.size() / numNodes;
    auto iter = locations.begin();
    int accum=0;
    int onNode=0;
    vector<int>begins(numNodes);
    begins[0]=0;
    vector<int>ends(numNodes);
    for (int i=0; i<locations.size() && onNode<numNodes && taskSize>0; i++, iter++)
    {
        int size = iter->second.size(); //locations.at(words[i]).size();
        accum+=size;
        cout <<i<<endl;
        cout << "accum: "<<accum<<endl;
        cout <<"taskSize: "<<taskSize<<endl;
        fileCount-=size;
        //if (fileCount==0)
        //    break;
        if (accum>taskSize && (accum)/taskSize<2)
        {
            ends.at(onNode)=i+1;
            if (++onNode<numNodes)
            {
                begins.at(onNode)=i+1;
                taskSize=fileCount/(numNodes-onNode);
            }
            accum=0;
        }
        else if (accum>taskSize)
        {
            ends.at(onNode)=i;
            if (++onNode<numNodes)
            {
                begins.at(onNode)=i;
                taskSize=(fileCount+size)/(numNodes-onNode);
            }
            accum=size;//As I've already "added" it, in a sense
        }
            
    }
    ends.back()=locations.size();//just in case
    int mybegin=begins[iproc];
    int myend=ends[iproc];
    cout << "["<<iproc<<"] is going from "<<mybegin<<" to "<<myend<<endl; 
   
    if (process)
    {
        cout<<"reading and encoding corpus"<<endl; 
        //vector< vector< vector< Mat/*< float >*/ > >* > corpusCoded(dataSize);
        //vector< Size > corpusSizes(dataSize);
        #pragma omp parallel for 
        for (int imageIdx=1; imageIdx<=dataSize; imageIdx++)
        {
            
            std::string imagePath = dataDirPath + "wordimg_" + to_string(imageIdx) + fileExt;
            Mat word = imread(imagePath,CV_LOAD_IMAGE_GRAYSCALE);
            assert(word.rows>0);
            //Size sz;
            bovw.encodeAndSaveImage(word,imageIdx);
            //corpusCoded[imageIdx-1] = bovw.encodeImage(word,&sz);
            //corpusSizes[imageIdx-1] = sz;
            
        }
    }

    iter = locations.begin();
    for (int i=0; i<mybegin; i++)
        iter++;
    
//    for (const auto &ngramLocPair : locations)
    for (int i=mybegin; i<myend; i++, iter++)
    {
        const auto &ngramLocPair = *(iter);
        
        const vector<int> &ngram_locations = ngramLocPair.second;//locations[ngramLocPair.first];
        string dirPath = exemplarDirPath + ngramLocPair.first + "/";
        cout << "["<<iproc<<"] on ngram["<<i<<"]: " <<  ngramLocPair.first << endl;
        for (int exemplarIdx=0; exemplarIdx<numExemplarsPer; exemplarIdx++)
        {
            vector<pair<int,double> >scores;

            
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
                

                #pragma omp for nowait //schedule(dynamic,200)
                for (int imageIdx=1; imageIdx<=dataSize; imageIdx++)
                {
                    //double startImg = clock();
                    if (imageIdx == ngram_locations[exemplarIdx])
                        continue;
                    
                    //double score = bovw.scanImageHorz(corpusCoded[imageIdx-1],corpusSizes[imageIdx-1],*exemplar_b,exemplar.size());
                    double score = bovw.scanImageHorz(imageIdx,*exemplar_b,exemplar.size());
                    
                    
                    myScores.push_back(pair<int,double>(imageIdx,score));
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
            int totalRelevent=ngram_locations.size()-1;
            
            for (int top=0; top<scores.size(); top++)
            {
                int ii = scores[top].first;
                if (top==0)
                {
                    cout << "top match is " << ii << endl;
                }
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
                fullResults += ngramLocPair.first+"_"+to_string(exemplarIdx+1) + "{" + to_string(scores[0].first);
                for (int i=1; i<scores.size(); i++)
                {
                    fullResults += "," + to_string(scores[i].first);
                }
                fullResults += "}\n";
                
                map += avgPrecision;
                mapCount++;
            }
            
        }
        
        cout << "finished ngram '" << ngramLocPair.first << "'' with " << (map-prevMap)/(mapCount-prevCount) << " mAP" << endl;
        prevMap=map;
        prevCount=mapCount;
        
//        break;
    }
    
    
    cout << "for batch " << batchNum << " mAP sum:"<<map<< " count:" << mapCount << endl;
    //save("results"+iproc=".mat","fullResults","map","mapCount");
    ofstream out;
    out.open (outfile+to_string(iproc), ios::out);
    out << fullResults;
    out.close();

    //for (auto pointer : corpusCoded)
    //    delete pointer;
    
    } catch (std::regex_error& e) {
        cout << "regex error:"<<e.code() << endl;
        return;
    }
}

void EnhancedBoVWTests::experiment_Aldavert_dist_batched(EnhancedBoVW &bovw, string locationCSVPath, string dataDirPath, int dataSize, string fileExt, int batchNum, int numOfBatches, string outfile)
{
    int numNodes=numOfBatches;
    int iproc=batchNum;
    try{
//    regex imageNameNumExtract("(?:wordimg_)(\d+)");
    regex imageNameNumExtract("\\d+");

    //vector<string> words;
    map<string,vector<int> > locations;
    
    ifstream file;
    file.open (locationCSVPath, ios::in);
    assert(file.is_open());
    
    string wordText;
    string fileList;
    

    int fileCount=0; 
    while (getline(file,wordText))
    {
        getline(file,fileList);
        //words.push_back(wordText);
        smatch sm;
        while(regex_search(fileList,sm,imageNameNumExtract))
        {
            int idx = stoi(sm[0]);
            locations[wordText].push_back(idx);
            fileList = sm.suffix().str();
            fileCount++;
        }
    }
    
    
    file.close();
    
    
//    map<> fullResults;
    string fullResults="";
    
    double map=0;
    double prevMap=0;
    int mapCount=0;
    int prevCount=0;
    
    //Doing this by the fileCount rather than locations.size() keeps the work distributed more evenly.
    int taskSize = fileCount/numNodes; //locations.size() / numNodes;
    auto iter = locations.begin();
    int accum=0;
    int onNode=0;
    vector<int>begins(numNodes);
    begins[0]=0;
    vector<int>ends(numNodes);
    for (int i=0; i<locations.size() && onNode<numNodes; i++, iter++)
    {
        int size = iter->second.size(); //locations.at(words[i]).size();
        accum+=size;
        cout <<i<<endl;
        cout << "accum: "<<accum<<endl;
        cout <<"taskSize: "<<taskSize<<endl;
        fileCount-=size;
        if (accum>taskSize && (accum)/taskSize<2)
        {
            ends.at(onNode)=i+1;
            if (++onNode<numNodes)
            {
                begins.at(onNode)=i+1;
                taskSize=fileCount/(numNodes-onNode);
            }
            accum=0;
        }
        else if (accum>taskSize)
        {
            ends.at(onNode)=i;
            if (++onNode<numNodes)
            {
                begins.at(onNode)=i;
                taskSize=(fileCount+size)/(numNodes-onNode);
            }
            accum=size;//As I've already "added" it, in a sense
        }
            
    }
    ends.back()=locations.size();//just in case
    int mybegin=begins[iproc];
    int myend=ends[iproc];

    cout << "["<<iproc<<"] is going from "<<mybegin<<" to "<<myend<<endl; 
    
    iter = locations.begin();
    for (int i=0; i<mybegin; i++)
        iter++;
    
//    for (const auto &wordTextLocPair : locations)
    for (int i=mybegin; i<myend; i++, iter++)
    {
        const auto &wordTextLocPair = *(iter);
        
        const vector<int> &word_locations = wordTextLocPair.second;//locations[wordTextLocPair.first];
        
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
            
        }
        
        cout << "finished word " << i << " with " << (map-prevMap)/(mapCount-prevCount) << " mAP" << endl;
        prevMap=map;
        prevCount=mapCount;
        }
//        break;
    }
    
    
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

multimap<double,int> EnhancedBoVWTests::experiment_Aldavert_single(EnhancedBoVW &bovw, string locationCSVPath, string dataDirPath, int dataSize, string fileExt, Mat query, string gt)
{
    multimap< double,int> scores;
    assert(query.rows > 0);
    try{
//    regex imageNameNumExtract("(?:wordimg_)(\d+)");
    regex imageNameNumExtract("\\d+");

    //vector<string> words;
    map<string,vector<int> > locations;
    ifstream file;
    file.open (locationCSVPath, ios::in);
    assert(file.is_open());
    
    string wordText;
    string fileList;
    

    int fileCount=0; 
    while (getline(file,wordText))
    {
        getline(file,fileList);
        //words.push_back(wordText);
        smatch sm;
        while(regex_search(fileList,sm,imageNameNumExtract))
        {
            int idx = stoi(sm[0]);
            locations[wordText].push_back(idx);
            fileList = sm.suffix().str();
            fileCount++;
        }
    }
    
    
    file.close();
    
    vector<int> word_locations = locations[gt];
    
    
    
    vector<float>* exemplar_b = bovw.featurizeImage(query);
    
    
    
    #pragma omp parallel
    {
        multimap<double,int> myScores;//(dataSize);
        

        #pragma omp for nowait //schedule(dynamic,200)
        for (int imageIdx=1; imageIdx<=dataSize; imageIdx++)
        {
            //double startImg = clock();
            
            std::string imagePath = dataDirPath + "wordimg_" + to_string(imageIdx) + fileExt;
            Mat word = imread(imagePath,CV_LOAD_IMAGE_GRAYSCALE);
            assert(word.rows>0);
            double score = bovw.compareImage(word,*exemplar_b);

            /*//debug
            if (imageIdx==1566)
                cout <<"score at "<<imageIdx<<" is "<<score<<endl;
            string qPath  = dataDirPath + "wordimg_81" + fileExt;
            Mat q = imread(qPath,CV_LOAD_IMAGE_GRAYSCALE);
            vector<float>* q_b = bovw.featurizeImage(q);
            for (int i=0; i<q_b->size(); i++)
                assert(q_b->at(i) == exemplar_b->at(i));
            double score2 = bovw.compareImage(word,*q_b);
            assert(score==score2);*/
            
            myScores.emplace(score,imageIdx);
        }
        
        #pragma omp critical
        scores.insert(myScores.begin(),myScores.end());
    }
    delete exemplar_b;
    
    /*bool testHas=false;
    for (auto p : scores)
        if (p.second==1566)
        {
            testHas=true;
            break;
        }
    assert(testHas);*/
        
    
    //compute average precision
    int foundRelevent = 0;
    double avgPrecision = 0.0;
    int totalRelevent=word_locations.size();
    auto iter=scores.begin(); 
    for (int top=0; top<scores.size(); top++, iter++)
    {
        int ii = iter->second;
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
    assert(totalRelevent == foundRelevent);
    avgPrecision = avgPrecision/totalRelevent;
    
        
    
    cout <<"map: "<<avgPrecision<<endl;
    
    return scores;
    
    } catch (std::regex_error& e) {
        cout << "regex error:"<<e.code() << endl;
        return scores;
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

void EnhancedBoVWTests::createGGobiFile(EnhancedBoVW &bovw, string locationCSVPath, string dataDirPath, string fileExt, int numWords, string outfile)
{

    regex imageNameNumExtract("\\d+");


    
    map<string,vector<int> > locations;
    
    ifstream file;
    file.open (locationCSVPath, ios::in);
    assert(file.is_open());
    
    string wordText;
    string fileList;

    vector< tuple<string,int> > most(1);
    most[0] = make_tuple(string("ERROR"),0);
    while (getline(file,wordText))
    {
        getline(file,fileList);
        
        smatch sm;
        while(regex_search(fileList,sm,imageNameNumExtract))
        {
            int idx = stoi(sm[0]);
            locations[wordText].push_back(idx);
            

            fileList = sm.suffix().str();
        }
        auto iter = most.begin();
        while (locations[wordText].size() < get<1>(*iter) && iter != most.end())
            iter++;
        most.insert(iter,make_tuple(wordText,locations[wordText].size()));
        if (most.size() > numWords && numWords!=-1)
            most.pop_back();
    }
    
    
    file.close();
    
    int count=0;
    for (auto t : most)
    {
        count += locations[get<0>(t)].size();
    }
    
    ofstream o(outfile);
    if (!o)
    {
        cout << "ERROR: could not open for writing GGobi file: " << outfile << endl;
        exit(-1);
    }
    
    o << "<?xml version=\"1.0\"?>" << endl;
    o << "<!DOCTYPE ggobidata SYSTEM \"ggobi.dtd\">" << endl;
    o << "<ggobidata>" << endl;
    
    o << "<data name=\"BoVW\">" << endl;
    o << "<variables count=\""+to_string(bovw.featureSize())+"\">" << endl;
    for (int i=0; i<bovw.featureSize(); i++)
    {
        o << "<realvariable>" << endl;
        o << "<name> v"<<i<<" </name>" << endl;
        o << "</realvariable>" << endl;
    }
    o << "</variables>" << endl;
    
    o << "<records count=\""<<count<<"\">" << endl;
    for (auto t : most)
    {
        for (int loc : locations[get<0>(t)])
        {
            std::string imagePath = dataDirPath + "wordimg_" + to_string(loc) + fileExt;
            Mat exemplar = imread(imagePath,CV_LOAD_IMAGE_GRAYSCALE);
            assert(exemplar.rows>0);
            vector<float>* exemplar_b = bovw.featurizeImage(exemplar);
            assert(exemplar_b->size() == bovw.featureSize());
            o << "<record label=\""<<get<0>(t)<<"\">" << endl;
            for (int i=0; i<exemplar_b->size(); i++)
            {
                o << "<real>" << exemplar_b->at(i) << "</real> ";
            }
            o << "\n</record>" << endl;
        }
    }
    o << "</records>" << endl;
    
    o << "</ggobidata>" << endl;
    o.close();
}

void EnhancedBoVWTests::drawData(EnhancedBoVW &bovw, string locationCSVPath, string dataDirPath, string fileExt, int numWords, string outfile)
{

    regex imageNameNumExtract("\\d+");


    
    map<string,vector<int> > locations;
    
    ifstream file;
    file.open (locationCSVPath, ios::in);
    assert(file.is_open());
    
    string wordText;
    string fileList;

    vector< tuple<string,int> > most(1);
    most[0] = make_tuple(string("ERROR"),0);
    while (getline(file,wordText))
    {
        getline(file,fileList);
        
        smatch sm;
        while(regex_search(fileList,sm,imageNameNumExtract))
        {
            int idx = stoi(sm[0]);
            locations[wordText].push_back(idx);
            

            fileList = sm.suffix().str();
        }
        auto iter = most.begin();
        while (locations[wordText].size() < get<1>(*iter) && iter != most.end())
            iter++;
        most.insert(iter,make_tuple(wordText,locations[wordText].size()));
        if (most.size() > numWords && numWords!=-1)
            most.pop_back();
    }
    
    
    file.close();
    vector<string> wordsToDraw;
    for (auto t : most)
    {
        wordsToDraw.push_back(get<0>(t));
    }
    constructHistograms(bovw, locationCSVPath, dataDirPath, fileExt, outfile, locations, wordsToDraw);
}

void EnhancedBoVWTests::drawDataForWords(EnhancedBoVW &bovw, string locationCSVPath, string dataDirPath, string fileExt, const vector<string>& wordsToDraw, string outfile)
{

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
            

            fileList = sm.suffix().str();
        }
    }
    
    
    file.close();
    constructHistograms(bovw, locationCSVPath, dataDirPath, fileExt, outfile, locations, wordsToDraw);
}


void EnhancedBoVWTests::constructHistograms(EnhancedBoVW &bovw, string locationCSVPath, string dataDirPath, string fileExt, string outfile, const map<string,vector<int> >& locations, const vector<string>& wordsToDraw)
{
    //int count=0;
    vector<double> maxs;
    maxs.resize(bovw.featureSize(),0);
    vector<double> mins;
    mins.resize(bovw.featureSize(),numeric_limits<double>::max());
    vector< vector<float>* > images;
    for (string word : wordsToDraw)
    {
        //count += locations[get<0>(t)].size();
        vector<float>* avg = new vector<float>(bovw.featureSize());
        for (int loc : locations.at(word))
        {
            std::string imagePath = dataDirPath + "wordimg_" + to_string(loc) + fileExt;
            Mat exemplar = imread(imagePath,CV_LOAD_IMAGE_GRAYSCALE);
            assert(exemplar.rows>0);
            vector<float>* exemplar_b = bovw.featurizeImage(exemplar);
            assert(exemplar_b->size() > 0);
            images.push_back(exemplar_b);
            
            for (int i=0; i<exemplar_b->size(); i++)
            {
                if (exemplar_b->at(i) > maxs[i])
                    maxs[i] = exemplar_b->at(i);
                if (exemplar_b->at(i) < mins[i])
                    mins[i] = exemplar_b->at(i);
                avg->at(i)+=exemplar_b->at(i);
            }
        }
        for (int i=0; i<avg->size(); i++)
            avg->at(i)/=locations.at(word).size();
        images.push_back(NULL);
        images.push_back(avg);
        images.push_back(NULL);
    }
    
    int row=0;
    
    int numBins=bovw.featureSize()/(3*bovw.codebook->size());
    Mat hists[numBins];
    for (int i=0; i<numBins; i++)
    {
        hists[i] = (Mat_<Vec3b>(images.size(),3*bovw.codebook->size()));
    }
    for (vector<float>* exemplar_b : images)
    {
        //assert(exemplar_b->size() == bovw.featureSize());
        //assert(row<images.size());
        //_colorCylcleIndex=0;
        for (int i=0; i<bovw.featureSize(); i++)
        {
            int bin = i/(3*bovw.codebook->size());
            int feature = i%(3*bovw.codebook->size());
            double weight;
            if (exemplar_b!=NULL)
            {
                weight = ((exemplar_b->at(i)-mins[i])/(maxs[i]-mins[i]!=0?maxs[i]-mins[i]:1));
                int r = weight>.5?255*(weight-.5)/.5:0;
                int g = weight>.5?255*(1.0-weight)/.5:255*(weight)/.5;
                int b = weight<.5?((weight+.2)/.7)*255*(.5-weight)/.5:0;
                Vec3b color=Vec3b(b,g,r);
                hists[bin].at<Vec3b>(row,feature) = color;
            }
            else
            {
                Vec3b color=Vec3b(255,255,255);
                hists[bin].at<Vec3b>(row,feature) = color;
            }
            
            //hists[bin].at<Vec3b>(row,3*feature) = color;
            //hists[bin].at<Vec3b>(1+row,1+3*feature) = color;
            //hists[bin].at<Vec3b>(row,1+3*feature) = color;
            //hists[bin].at<Vec3b>(1+row,3*feature) = color;
        }
        
        row+=1;
        //delete exemplar_b;
    }
    for (int i=0; i<numBins; i++)
    {
        imwrite(to_string(i)+"_"+outfile,hists[i]);
    }
    cout << "done" << endl;
}

void EnhancedBoVWTests::compareDataForWords(EnhancedBoVW &bovw, string locationCSVPath, string dataDirPath, string fileExt, const vector<string>& wordsToDraw, string ex_file, string outfile)
{

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
            

            fileList = sm.suffix().str();
        }
    }
    
    
    file.close();
    Mat ex_im=imread(ex_file,CV_LOAD_IMAGE_GRAYSCALE);
    compareHistograms(bovw, locationCSVPath, dataDirPath, fileExt, outfile, locations, wordsToDraw,ex_im);
}

void EnhancedBoVWTests::compareHistograms(EnhancedBoVW &bovw, string locationCSVPath, string dataDirPath, string fileExt, string outfile, const map<string,vector<int> >& locations, const vector<string>& wordsToDraw, const Mat& toCompare)
{
    
    //int count=0;
    vector<double> maxs;
    maxs.resize(bovw.featureSize(),0);
    vector<double> mins;
    mins.resize(bovw.featureSize(),numeric_limits<double>::max());
    vector< vector<float>* > images;
    
    vector<float>* exemplar_c = bovw.featurizeImage(toCompare);
    images.push_back(NULL);
    images.push_back(exemplar_c);
    images.push_back(NULL);
    for (int i=0; i<exemplar_c->size(); i++)
    {
        if (exemplar_c->at(i) > maxs[i])
            maxs[i] = exemplar_c->at(i);
        if (exemplar_c->at(i) < mins[i])
            mins[i] = exemplar_c->at(i);
    }
    
    for (string word : wordsToDraw)
    {
        //count += locations[get<0>(t)].size();
        vector<float>* avg = new vector<float>(bovw.featureSize());
        for (int loc : locations.at(word))
        {
            std::string imagePath = dataDirPath + "wordimg_" + to_string(loc) + fileExt;
            Mat exemplar = imread(imagePath,CV_LOAD_IMAGE_GRAYSCALE);
            assert(exemplar.rows>0);
            vector<float>* exemplar_b = bovw.featurizeImage(exemplar);
            
            double score = bovw.compareImage(exemplar,*exemplar_c);
            cout << score << endl;
            assert(exemplar_b->size() > 0);
            images.push_back(exemplar_b);
            
            for (int i=0; i<exemplar_b->size(); i++)
            {
                if (exemplar_b->at(i) > maxs[i])
                    maxs[i] = exemplar_b->at(i);
                if (exemplar_b->at(i) < mins[i])
                    mins[i] = exemplar_b->at(i);
                avg->at(i)+=exemplar_b->at(i);
            }
        }
        for (int i=0; i<avg->size(); i++)
            avg->at(i)/=locations.at(word).size();
        images.push_back(NULL);
        images.push_back(avg);
        images.push_back(NULL);
    }
    
    int row=0;
    
    int numBins=bovw.featureSize()/(3*bovw.codebook->size());
    Mat hists[numBins];
    for (int i=0; i<numBins; i++)
    {
        hists[i] = (Mat_<Vec3b>(images.size(),3*bovw.codebook->size()));
    }
    for (vector<float>* exemplar_b : images)
    {
        //assert(exemplar_b->size() == bovw.featureSize());
        //assert(row<images.size());
        //_colorCylcleIndex=0;
        for (int i=0; i<bovw.featureSize(); i++)
        {
            int bin = i/(3*bovw.codebook->size());
            int feature = i%(3*bovw.codebook->size());
            double weight;
            if (exemplar_b!=NULL)
            {
                weight = ((exemplar_b->at(i)-mins[i])/(maxs[i]-mins[i]!=0?maxs[i]-mins[i]:1));
                int r = weight>.5?255*(weight-.5)/.5:0;
                int g = weight>.5?255*(1.0-weight)/.5:255*(weight)/.5;
                int b = weight<.5?((weight+.2)/.7)*255*(.5-weight)/.5:0;
                Vec3b color=Vec3b(b,g,r);
                hists[bin].at<Vec3b>(row,feature) = color;
            }
            else
            {
                Vec3b color=Vec3b(255,255,255);
                hists[bin].at<Vec3b>(row,feature) = color;
            }
            
            //hists[bin].at<Vec3b>(row,3*feature) = color;
            //hists[bin].at<Vec3b>(1+row,1+3*feature) = color;
            //hists[bin].at<Vec3b>(row,1+3*feature) = color;
            //hists[bin].at<Vec3b>(1+row,3*feature) = color;
        }
        
        row+=1;
        //delete exemplar_b;
    }
    for (int i=0; i<numBins; i++)
    {
        imwrite(to_string(i)+"_"+outfile,hists[i]);
    }
    cout << "done" << endl;
}
