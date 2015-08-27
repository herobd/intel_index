#include "liangtests.h"



void LiangTests::keyword_duplicationTest(int keywordNum, int testNum, string annFilePath, string imgDirPath, string imgNamePattern, string imgExt)
{
    vector<string> keywords = {"account","Alexandria","allowance","arms","Bacon","Bell","Bread","Breeches","Captain","colonel","commissary","commission","companies","Company","Coopers","Cumberland","December","Dinwiddie","discipline","Fleming","Force","Fort","Hoggs","Instructions","Lieutenant","medicines","October","Officer","Orders","Pattersons","Rangers","Recruit","Regiment","Salt","Stephen","Stockades","Surgeon","Troops"};
    
    string keyword=keywords[keywordNum];
    cout << "Testing keyword: " << keyword << endl;
    
    if (imgDirPath[imgDirPath.size()-1]!='/')
        imgDirPath += '/';
    if (imgExt[0]!='.')
        imgExt = '.'+imgExt;
    
    ifstream file;
    file.open (annFilePath, ios::in);
    assert(file.is_open());
    
    string word;
    regex clean("[^a-zA-Z0-9]");
    
    vector< pair<int, string> > samples;
    int num=0;
    while (getline(file,word))
    {
        word = regex_replace(word,clean,"");
        samples.push_back(pair<int,string>(++num,word));
    }
    random_shuffle(samples.begin(),samples.end());
    
    vector< pair<int, string> > trainingSet;
    vector< pair<int, string> > testingSet;
    vector< pair<int, string> > waitingSet;
    
    map<char, int> charCounts;
    for (char c : keyword) charCounts[c]=0;
    vector<int> keywordLocations;
    for (const pair<int, string>& sample : samples)
    {
        if (sample.second.compare(keyword)==0)
        {
            testingSet.push_back(sample);
            keywordLocations.push_back(sample.first);
        }
        else if (sharedLetterNeeded(sample.second,charCounts))
        {
            trainingSet.push_back(sample);
        }
        else
        {
            waitingSet.push_back(sample);
        }
    }
    for (int i=.3 * waitingSet.size(); i>=0; i--)
    {
        testingSet.push_back(waitingSet.back());
        waitingSet.pop_back();
    }
    trainingSet.insert(trainingSet.end(),waitingSet.begin(),waitingSet.end());
    
    cout <<"training..." << endl;
    Liang spotter;
    spotter.trainCharacterModels(imgDirPath,imgNamePattern,imgExt,trainingSet);
    
    string savefilename = "liang_"+to_string(keywordNum)+"_"+to_string(testNum);
    cout << "Training complete. (saving character models as " << savefilename+".models" << ")" << endl; 
    spotter.saveCharacterModels(savefilename+".models");
    
    vector<pair<int,double> >scores;
    #pragma omp parallel
    {
        vector<pair<int,double> >myScores;
    
        #pragma omp for nowait schedule(dynamic,100)
        for (int i=0; i<testingSet.size(); i++)
        {
            string imgPath=imgDirPath + imgNamePattern + to_string(testingSet[i].first) + imgExt;
            Mat testImg= imread(imgPath,0);
            threshold(testImg,testImg,IMAGE_THRESH,255,1);
            double score = spotter.score(keyword,testImg);
            myScores.push_back(pair<int,double>(testingSet[i].first,score));
            if (testingSet[i].second.compare(keyword)==0)
            {
                cout << "score on "<<testingSet[i].first<<" is " << score << endl;
            }
        }
        
        #pragma omp critical
        scores.insert(scores.end(),myScores.begin(),myScores.end());
    }
    
    sort(scores.begin(),scores.end(),[](const pair<int,double> &l, const pair<int,double> &r)->bool {return l.second > r.second;});
    
    int foundRelevent = 0;
    double avgPrecisionAt10 = 0.0;
    double avgPrecision = 0.0;
    int totalRelevent=keywordLocations.size();
    double precision;
    double precisionAt10;
    double avg=0;
    int avgCount=0;
    for (int top=0; top<scores.size(); top++)
    {
        int ii = scores[top].first;
        if (scores[top].second>=0)
        {
            avg += scores[top].second;
            avgCount++;
        }
        if (find(keywordLocations.begin(),keywordLocations.end(),ii)!=keywordLocations.end())
        {
            foundRelevent++;
            precision = foundRelevent/(double)(top+1);
            avgPrecision += precision;
        }
        if (top==9)
        {
            avgPrecisionAt10=avgPrecision/min(10,totalRelevent);
            precisionAt10=precision;
        }
    }
    avg/=avgCount;
    avgPrecision = avgPrecision/totalRelevent;
    cout << "mAP = " << avgPrecision << endl;
    cout << "mAP at rank 10 = " << avgPrecisionAt10 << endl;
    cout << "precision at rank 10 = " << precisionAt10 << endl;
    cout << "average score is "<<avg<<endl;

    string fullResults = keyword+"_"+to_string(testNum) + "\n" + to_string(scores[0].first);
    for (int i=1; i<scores.size(); i++)
    {
        fullResults += "\n" + to_string(scores[i].first);
    }
//    fullResults += "};
    
    ofstream out;
    out.open (savefilename+".results", ios::out);
    out << fullResults;
    out.close();
    
}


bool LiangTests::sharedLetterNeeded(string word, map<char, int>& charCounts)
{
    bool yes=false;
    for (auto &cc : charCounts)
    {
        if (cc.second<10 && word.find(cc.first)!=string::npos)
        {
            yes=true;
            cc.second++;
        }
    }
    
    return yes;
}
