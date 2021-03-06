#include "embattspotter.h"
#include <set>
#include <map>

#define PAD_EXE 9
#define END_PAD_EXE 3

int sort_xxx(const void *x, const void *y) {
    if (*(int*)x > *(int*)y) return 1;
    else if (*(int*)x < *(int*)y) return -1;
    else return 0;
}

void EmbAttSpotter::eval(const Dataset* data)
{
    setCorpus_dataset(data);
    for (double hy=0.0; hy<=1.0; hy+=0.1)
    {
        
        float map=0;
        int queryCount=0;
        #pragma omp parallel  for
        for (int inst=0; inst<data->size(); inst++)
        {
            int other=0;
            string text = data->labels()[inst];
            for (int inst2=0; inst2<data->size(); inst2++)
            {
                if (inst!=inst2 && text.compare(data->labels()[inst2])==0)
                {
                    other++;
                }
            }
            if (other==0)
                continue;
            
            int *rank = new int[other];//(int*)malloc(NRelevantsPerQuery[i]*sizeof(int));
            int Nrelevants = 0;
            float ap=0;
            
            float bestS=-99999;
            vector<float> scores = spot(data->image(inst),text,hy); //scores
            for (int j=0; j < data->size(); j++)
            {            
                float s = scores[j];
                //cout <<"score for "<<j<<" is "<<s<<". It is ["<<data->labels()[j]<<"], we are looking for ["<<text<<"]"<<endl;
                /* Precision at 1 part */
                /*if (inst!=j && s > bestS)
                {
                    bestS = s;
                    p1 = text==data->labels()[j];
                    //bestIdx[inst] = j;
                }*/
                /* If it is from the same class and it is not the query idx, it is a relevant one. */
                /* Compute how many on the dataset get a better score and how many get an equal one, excluding itself and the query.*/
                if (text.compare(data->labels()[j])==0 && inst!=j)
                {
                    int better=0;
                    int equal = 0;
                    
                    for (int k=0; k < data->size(); k++)
                    {
                        if (k!=j && inst!=k)
                        {
                            float s2 = scores[k];
                            if (s2> s) better++;
                            else if (s2==s) equal++;
                        }
                    }
                    
                    
                    rank[Nrelevants]=better+floor(equal/2.0);
                    Nrelevants++;
                }
                
            }
            qsort(rank, Nrelevants, sizeof(int), sort_xxx);
            
            //pP1[i] = p1;
            
            /* Get mAP and store it */
            for(int j=0;j<Nrelevants;j++){
                /* if rank[i] >=k it was not on the topk. Since they are sorted, that means bail out already */
                
                float prec_at_k =  ((float)(j+1))/(rank[j]+1);
                //mexPrintf("prec_at_k: %f\n", prec_at_k);
                ap += prec_at_k;            
            }
            ap/=Nrelevants;
            
            #pragma omp critical (storeMAP)
            {
                queryCount++;
                map+=ap;
            }
            
            delete[] rank;
        }
        
        cout<<"map: "<<(map/queryCount)<<" for "<<hy<<endl;
    }
}

//#define LIVE_SCORE_OVERLAP_THRESH 0.55
//This is a testing function for the simulator
float EmbAttSpotter::evalSubwordSpotting_singleScore(string ngram, const vector<SubwordSpottingResult>& res, const vector< vector<int> >* corpusXLetterStartBounds, const vector< vector<int> >* corpusXLetterEndBounds, int skip) const
{
    //string ngram = exemplars->labels()[inst];
    int Nrelevants = 0;
    float ap=0;
    
    float bestS=-99999;
    //vector<SubwordSpottingResult> res = subwordSpot(exemplars->image(inst),ngram,hy); //scores
    float maxScore=-9999;
    for (auto r : res)
        if (r.score>maxScore)
            maxScore=r.score;
    vector<float> scores;
    vector<bool> rel;
    int l=ngram.length()-1;
    vector<int> checked(corpus_dataset->size());
    for (int j=0; j<res.size(); j++)
    {
        SubwordSpottingResult r = res[j];
        //checked[r.imIdx]=true;
        if (skip == r.imIdx)
            continue;

        //the indices of the words and the seg file can be different, so
        int letterBoundsIndex = corpus_dataset->wordId(r.imIdx);

        size_t loc = corpus_dataset->labels()[r.imIdx].find(ngram);
        if (loc==string::npos)
        {
            scores.push_back(r.score);
            rel.push_back(false);
            checked[r.imIdx]++;
        }
        else
        {
            int loc2 = corpus_dataset->labels()[r.imIdx].find(ngram,loc+1);
            vector<int> matching;
            for (int jj=0; jj < res.size(); jj++)
            {
                if (res[jj].imIdx == r.imIdx && j!=jj && res[jj].imIdx!=skip)
                    matching.push_back(jj);
            }
            assert(corpusXLetterEndBounds->size() > letterBoundsIndex);
            assert(corpusXLetterEndBounds->at(letterBoundsIndex).size()>loc+l);
            float myOverlap = ( min(corpusXLetterEndBounds->at(letterBoundsIndex).at(loc+l), r.endX) 
                                - max(corpusXLetterStartBounds->at(letterBoundsIndex).at(loc), r.startX) ) 
                              /
                              ( max(corpusXLetterEndBounds->at(letterBoundsIndex).at(loc+l), r.endX) 
                                - min(corpusXLetterStartBounds->at(letterBoundsIndex).at(loc), r.startX) +0.0);
            if (loc2 != string::npos)
            {
                assert(corpusXLetterEndBounds->at(letterBoundsIndex).size()>loc2+l);
                float myOverlap2 = ( min(corpusXLetterEndBounds->at(letterBoundsIndex).at(loc2+l), r.endX) 
                                    - max(corpusXLetterStartBounds->at(letterBoundsIndex).at(loc2), r.startX) ) 
                                  /
                                  ( max(corpusXLetterEndBounds->at(letterBoundsIndex).at(loc2+l), r.endX) 
                                    - min(corpusXLetterStartBounds->at(letterBoundsIndex).at(loc2), r.startX) +0.0);
                if (myOverlap2>myOverlap)
                {
                    myOverlap=myOverlap2;
                    loc=loc2;
                }
            }
            
            if (matching.size()>0)
            {
                //float relPos = (loc+(ngram.length()/2.0))/corpus_dataset->labels()[r.imIdx].length();
                //float myDif = fabs(relPos - (r.startX + (r.endX-r.startX)/2.0)/corpus_dataset->image(r.imIdx).cols);
                bool other=false;
                for (int oi : matching)
                {
                    float otherOverlap = ( min(corpusXLetterEndBounds->at(letterBoundsIndex).at(loc+l), res[oi].endX) 
                                            - max(corpusXLetterStartBounds->at(letterBoundsIndex).at(loc), res[oi].startX) ) 
                                          /
                                          ( max(corpusXLetterEndBounds->at(letterBoundsIndex).at(loc+l), res[oi].endX) 
                                            - min(corpusXLetterStartBounds->at(letterBoundsIndex).at(loc), res[oi].startX) +0.0);
                    //if (otherOverlap > myOverlap) {
                    if ((otherOverlap > myOverlap && checked[r.imIdx]==0) || otherOverlap >= myOverlap) {
                        other=true;
                        break;
                    }
                }
                if (other)
                {
                    scores.push_back(r.score);
                    rel.push_back(false);
                }
                else if (myOverlap > LIVE_SCORE_OVERLAP_THRESH)
                {
                    scores.push_back(r.score);
                    rel.push_back(true);
                    checked[r.imIdx]++;
                }
            }
            else
            {
                /*bool ngram1H = loc+(ngram.length()/2.0) < 0.8*corpus_dataset->labels()[r.imIdx].length()/2.0;
                bool ngram2H = loc+(ngram.length()/2.0) > 1.2*corpus_dataset->labels()[r.imIdx].length()/2.0;
                bool ngramM = loc+(ngram.length()/2.0) > corpus_dataset->labels()[r.imIdx].length()/3.0 &&
                    loc+(ngram.length()/2.0) < 2.0*corpus_dataset->labels()[r.imIdx].length()/3.0;
                float sLoc = r.startX + (r.endX-r.startX)/2.0;
                bool spot1H = sLoc < 0.8*corpus_dataset->image(r.imIdx).cols/2.0;
                bool spot2H = sLoc > 1.2*corpus_dataset->image(r.imIdx).cols/2.0;
                bool spotM = sLoc > corpus_dataset->image(r.imIdx).cols/3.0 &&
                    sLoc < 2.0*corpus_dataset->image(r.imIdx).cols/3.0;
                    */

                if (myOverlap > LIVE_SCORE_OVERLAP_THRESH)
                //if ( (ngram1H&&spot1H) || (ngram2H&&spot2H) || (ngramM&&spotM) )
                {
                    scores.push_back(r.score);
                    rel.push_back(true);
                }
                else
                {
                    //cout<<"bad overlap["<<j<<"]: "<<myOverlap<<"  spot["<<r.startX<<","<<r.endX<<"]  bounds["<<corpusXLetterStartBounds->at(letterBoundsIndex)[loc]<<","<<corpusXLetterEndBounds->at(letterBoundsIndex)[loc+l]<<"]"<<endl;
                    scores.push_back(r.score);
                    rel.push_back(false);
                    //Insert a dummy result for the correct spotting to keep MAP accurate
                    scores.push_back(maxScore);
                    rel.push_back(true);
                }
                checked[r.imIdx]++;

            }
        }
    }
    
    for (int j=0; j<corpus_dataset->size(); j++)
    {
        int loc = corpus_dataset->labels()[j].find(ngram);
        if (!checked[j] &&  loc!=string::npos)
        {
            scores.push_back(maxScore);
            rel.push_back(true); 
            checked[j]++;
        }
        if (loc !=string::npos && checked[j]<2 && corpus_dataset->labels()[j].find(ngram,loc+1) != string::npos)
        {
            scores.push_back(maxScore);
            rel.push_back(true);
            checked[j]++;
        }
    }
    vector<int> rank;
    for (int j=0; j < scores.size(); j++)
    {            
        float s = scores[j];
        //cout <<"score for "<<j<<" is "<<s<<". It is ["<<data->labels()[j]<<"], we are looking for ["<<text<<"]"<<endl;
        
        if (rel[j])
        {
            int better=0;
            int equal = 0;
            
            for (int k=0; k < scores.size(); k++)
            {
                if (k!=j)
                {
                    float s2 = scores[k];
                    if (s2< s) better++;
                    else if (s2==s) equal++;
                }
            }
            
            
            rank.push_back(better+floor(equal/2.0));
            Nrelevants++;
        }
        
    }
    qsort(rank.data(), Nrelevants, sizeof(int), sort_xxx);
    
    //pP1[i] = p1;
    
    /* Get mAP and store it */
    for(int j=0;j<Nrelevants;j++){
        /* if rank[i] >=k it was not on the topk. Since they are sorted, that means bail out already */
        
        float prec_at_k =  ((float)(j+1))/(rank[j]+1);
        //mexPrintf("prec_at_k: %f\n", prec_at_k);
        ap += prec_at_k;            
    }
    ap/=Nrelevants;
    
   return ap;
}


void EmbAttSpotter::evalSubwordSpottingWithCharBounds(const Dataset* data, const vector< vector<int> >* corpusXLetterStartBounds, const vector< vector<int> >* corpusXLetterEndBounds, double hyV)
{
    setCorpus_dataset(data);


    double hyS=0.0;
    double hyE=1.0;
    if (hyV>=0)
    {
        hyS=hyE=hyV;
    }
    for (double hy=hyS; hy<=hyE; hy+=0.2)
    {
        set<string> done;
        float mAP=0;
        int queryCount=0;
        map<string,int> ngramCounter;

        #pragma omp parallel for
        for (int inst=0; inst<data->size(); inst++)
        {
            string label = data->labels()[inst];
            if (label.length()<2)
                continue;
            int bigram= inst%(label.length()-1);
            string ngram="";
            do {
                ngram = label.substr(bigram,2);
                int nRel=0;
                for (int inst2=0; inst2<data->size(); inst2++)
                {
                    if (data->labels()[inst2].find(ngram) != string::npos)
                        nRel++;
                }
                if (nRel<10)
                {
                    bigram= (bigram+1)%(label.length()-1);
                    ngram="";
                }
            } while(ngram.length()==0 && bigram!=inst%(label.length()-1));
            if (ngram.length()==0)
                continue;

            Mat exemplar;
            if (hy==0)
            {
                bool cc=false;
#pragma omp critical (ddd)
                {
                if (done.find(ngram)!=done.end())
                    cc=true;
                else
                    done.insert(ngram);
                }
                if (cc)
                    continue;
            }
            else
            {
                Mat wordIm = data->image(inst);
                int x1 = max(0,corpusXLetterStartBounds->at(inst)[bigram] - (bigram==0?END_PAD_EXE:PAD_EXE));
                int x2 = min(wordIm.cols-1,corpusXLetterEndBounds->at(inst)[bigram+1] + (bigram==label.length()-2?END_PAD_EXE:PAD_EXE));
                exemplar = wordIm(Rect(x1,0,x2-x1+1,wordIm.rows));
            }
            float ap=0;
            
            //imshow("exe", exemplar);
            //waitKey();
            vector<SubwordSpottingResult> res = subwordSpot(exemplar,ngram,hy); //scores

            ap = evalSubwordSpotting_singleScore(ngram, res, corpusXLetterStartBounds, corpusXLetterEndBounds, inst);
            #pragma omp critical (storeMAP)
            {
                queryCount++;
                mAP+=ap;
                cout<<"on spotting inst:"<<inst<<", "<<ngram<<"   ap: "<<ap<<endl;
                ngramCounter[ngram]++;
            }
            
        }
        cout<<"ngram counts= ";
        for (auto p : ngramCounter)
            cout<<p.first<<":"<<p.second<<"  ";
        cout<<endl;
        cout<<"FULL map: "<<(mAP/queryCount)<<" for hy:"<<hy<<endl;
    }
}

void EmbAttSpotter::evalSubwordSpotting(const Dataset* exemplars, /*string exemplars_locations,*/ const Dataset* data, double hyV)
{
    setCorpus_dataset(data);

    map<string,set<int> > widths;
    for (int i=0; i<exemplars->size(); i++)
    {
        widths[exemplars->labels()[i]].insert(exemplars->image(i).cols);
    }

    cout<<"Average exemplar widths"<<endl;
    for (auto p : widths)
    {
        double avg=0;
        for (int w : p.second)
            avg+=w;
        avg/=p.second.size();
        cout <<p.first<<": "<<avg<<endl;
    }

    double hyS=0.0;
    double hyE=1.0;
    if (hyV>=0)
    {
        hyS=hyE=hyV;
    }
    for (double hy=hyS; hy<=hyE; hy+=0.2)
    {
        set<string> done;
        float map=0;
        int queryCount=0;
        float gramMap=0;
        string gram="";
        int gramCount=0;
        #pragma omp parallel for
        for (int inst=0; inst<exemplars->size(); inst++)
        {
            string ngram = exemplars->labels()[inst];
            if (hy==0)
            {
                bool cc=false;
#pragma omp critical (ddd)
                {
                if (done.find(ngram)!=done.end())
                    cc=true;
                else
                    done.insert(ngram);
                }
                if (cc)
                    continue;
            }
            //cout <<"on spotting inst:"<<inst<<", "<<ngram;
            //cout << flush;
            //int *rank = new int[other];//(int*)malloc(NRelevantsPerQuery[i]*sizeof(int));
            int Nrelevants = 0;
            float ap=0;
            
            float bestS=-99999;
            //imshow("exe", exemplars->image(inst));
            //waitKey();
            vector<SubwordSpottingResult> res = subwordSpot(exemplars->image(inst),ngram,hy); //scores
            float maxScore=-9999;
            for (auto r : res)
                if (r.score>maxScore)
                    maxScore=r.score;
            vector<float> scores;
            vector<bool> rel;
            vector<bool> checked(corpus_dataset->size());
            for (int j=0; j<res.size(); j++)
            {
                SubwordSpottingResult r = res[j];
                checked[r.imIdx]=true;
                size_t loc = data->labels()[r.imIdx].find(ngram);
                if (loc==string::npos)
                {
                    scores.push_back(r.score);
                    rel.push_back(false);
                }
                else
                {
                    vector<int> matching;
                    for (int jj=0; jj < res.size(); jj++)
                    {
                        if (res[jj].imIdx == r.imIdx && j!=jj)
                            matching.push_back(jj);
                    }
                    if (matching.size()>0)
                    {
                        float relPos = (loc+(ngram.length()/2.0))/data->labels()[r.imIdx].length();
                        float myDif = fabs(relPos - (r.startX + (r.endX-r.startX)/2.0)/data->image(r.imIdx).cols);
                        bool other=false;
                        for (int oi : matching)
                        {
                            float oDif = fabs(relPos - (res[oi].startX + (res[oi].endX-res[oi].startX)/2.0)/data->image(res[oi].imIdx).cols);
                            if (oDif < myDif) {
                                other=true;
                                break;
                            }
                        }
                        if (other)
                        {
                            scores.push_back(r.score);
                            rel.push_back(false);
                        }
                        else
                        {
                            scores.push_back(r.score);
                            rel.push_back(true);
                        }
                    }
                    else
                    {
                        bool ngram1H = loc+(ngram.length()/2.0) < 0.8*data->labels()[r.imIdx].length()/2.0;
                        bool ngram2H = loc+(ngram.length()/2.0) > 1.2*data->labels()[r.imIdx].length()/2.0;
                        bool ngramM = loc+(ngram.length()/2.0) > data->labels()[r.imIdx].length()/3.0 &&
                            loc+(ngram.length()/2.0) < 2.0*data->labels()[r.imIdx].length()/3.0;
                        float sLoc = r.startX + (r.endX-r.startX)/2.0;
                        bool spot1H = sLoc < 0.8*data->image(r.imIdx).cols/2.0;
                        bool spot2H = sLoc > 1.2*data->image(r.imIdx).cols/2.0;
                        bool spotM = sLoc > data->image(r.imIdx).cols/3.0 &&
                            sLoc < 2.0*data->image(r.imIdx).cols/3.0;

                        if ( (ngram1H&&spot1H) || (ngram2H&&spot2H) || (ngramM&&spotM) )
                        {
                            scores.push_back(r.score);
                            rel.push_back(true);
                        }
                        else
                        {
                            scores.push_back(r.score);
                            rel.push_back(false);
                            //Insert a dummy result for the correct spotting to keep MAP accurate
                            scores.push_back(maxScore);
                            rel.push_back(true);
                        }

                    }
                }
            }
            for (int j=0; j<corpus_dataset->size(); j++)
            {
                if (!checked[j] &&  corpus_dataset->labels()[j].find(ngram)!=string::npos)
                {
                    scores.push_back(maxScore);
                    rel.push_back(true); 
                }
            }
            vector<int> rank;
            for (int j=0; j < scores.size(); j++)
            {            
                float s = scores[j];
                //cout <<"score for "<<j<<" is "<<s<<". It is ["<<data->labels()[j]<<"], we are looking for ["<<text<<"]"<<endl;
                
                if (rel[j])
                {
                    int better=0;
                    int equal = 0;
                    
                    for (int k=0; k < scores.size(); k++)
                    {
                        if (k!=j)
                        {
                            float s2 = scores[k];
                            if (s2< s) better++;
                            else if (s2==s) equal++;
                        }
                    }
                    
                    
                    rank.push_back(better+floor(equal/2.0));
                    Nrelevants++;
                }
                
            }
            qsort(rank.data(), Nrelevants, sizeof(int), sort_xxx);
            
            //pP1[i] = p1;
            
            /* Get mAP and store it */
            for(int j=0;j<Nrelevants;j++){
                /* if rank[i] >=k it was not on the topk. Since they are sorted, that means bail out already */
                
                float prec_at_k =  ((float)(j+1))/(rank[j]+1);
                //mexPrintf("prec_at_k: %f\n", prec_at_k);
                ap += prec_at_k;            
            }
            ap/=Nrelevants;
            
            #pragma omp critical (storeMAP)
            {
                queryCount++;
                map+=ap;
                cout<<"on spotting inst:"<<inst<<", "<<ngram<<"[# "<<Nrelevants<<"]   ap: "<<ap<<endl;
                /*if (gram.compare(ngram)!=0)
                {
                    if (gramCount>0)
                    {
                        cout <<"ap for ["<<gram<<"]: "<<(gramMap/gramCount)<<endl;
                        gramCount=0;
                        gramMap=0;
                    }
                    gram=ngram;
                }
                gramMap+=ap;
                gramCount++;*/
            }
            
        }
        //cout <<"ap for ["<<gram<<"]: "<<(gramMap/gramCount)<<endl;
        
        cout<<"FULL map: "<<(map/queryCount)<<" for hy:"<<hy<<endl;
    }
}
void EmbAttSpotter::evalSubwordSpottingCombine(const Dataset* exemplars, /*string exemplars_locations,*/ const Dataset* data, double hyV)
{
    setCorpus_dataset(data);

    map<string,set<int> > widths;
    for (int i=0; i<exemplars->size(); i++)
    {
        widths[exemplars->labels()[i]].insert(exemplars->image(i).cols);
    }

    cout<<"Average exemplar widths"<<endl;
    for (auto p : widths)
    {
        double avg=0;
        for (int w : p.second)
            avg+=w;
        avg/=p.second.size();
        cout <<p.first<<": "<<avg<<endl;
    }
    map<string,vector<Mat> > combExemplars;
    for (int inst=0; inst<exemplars->size(); inst++)
    {
        string ngram = exemplars->labels()[inst];
        combExemplars[ngram].push_back(exemplars->image(inst));
    }


    double hyS=0.0;
    double hyE=1.0;
    if (hyV>=0)
    {
        hyS=hyE=hyV;
    }
    for (double hy=hyS; hy<=hyE; hy+=0.2)
    {
        set<string> done;
        float map=0;
        int queryCount=0;
        float gramMap=0;
        string gram="";
        int gramCount=0;
        #pragma omp parallel  for
        for (int inst=0; inst<combExemplars.size(); inst++)
        {
            auto iter = combExemplars.begin();
            for (int i=0; i<inst; i++)
                iter++;
            string ngram = iter->first;
            if (hy==0)
            {
                bool cc=false;
#pragma omp critical (ddd)
                {
                if (done.find(ngram)!=done.end())
                    cc=true;
                else
                    done.insert(ngram);
                }
                if (cc)
                    continue;
            }
            //cout <<"on spotting inst:"<<inst<<", "<<ngram;
            //cout << flush;
            //int *rank = new int[other];//(int*)malloc(NRelevantsPerQuery[i]*sizeof(int));
            int Nrelevants = 0;
            float ap=0;
            
            float bestS=-99999;
            //imshow("exe", exemplars->image(inst));
            //waitKey();
            vector<SubwordSpottingResult> res = subwordSpot(iter->second,ngram,hy); //scores
            float maxScore=-9999;
            for (auto r : res)
                if (r.score>maxScore)
                    maxScore=r.score;
            vector<float> scores;
            vector<bool> rel;
            for (int j=0; j<res.size(); j++)
            {
                SubwordSpottingResult r = res[j];
                size_t loc = data->labels()[r.imIdx].find(ngram);
                if (loc==string::npos)
                {
                    scores.push_back(r.score);
                    rel.push_back(false);
                }
                else
                {
                    vector<int> matching;
                    for (int jj=0; jj < res.size(); jj++)
                    {
                        if (res[jj].imIdx == r.imIdx && j!=jj)
                            matching.push_back(jj);
                    }
                    if (matching.size()>0)
                    {
                        float relPos = (loc+(ngram.length()/2.0))/data->labels()[r.imIdx].length();
                        float myDif = fabs(relPos - (r.startX + (r.endX-r.startX)/2.0)/data->image(r.imIdx).cols);
                        bool other=false;
                        for (int oi : matching)
                        {
                            float oDif = fabs(relPos - (res[oi].startX + (res[oi].endX-res[oi].startX)/2.0)/data->image(res[oi].imIdx).cols);
                            if (oDif < myDif) {
                                other=true;
                                break;
                            }
                        }
                        if (other)
                        {
                            scores.push_back(r.score);
                            rel.push_back(false);
                        }
                        else
                        {
                            scores.push_back(r.score);
                            rel.push_back(true);
                        }
                    }
                    else
                    {
                        bool ngram1H = loc+(ngram.length()/2.0) < 0.8*data->labels()[r.imIdx].length()/2.0;
                        bool ngram2H = loc+(ngram.length()/2.0) > 1.2*data->labels()[r.imIdx].length()/2.0;
                        bool ngramM = loc+(ngram.length()/2.0) > data->labels()[r.imIdx].length()/3.0 &&
                            loc+(ngram.length()/2.0) < 2.0*data->labels()[r.imIdx].length()/3.0;
                        float sLoc = r.startX + (r.endX-r.startX)/2.0;
                        bool spot1H = sLoc < 0.8*data->image(r.imIdx).cols/2.0;
                        bool spot2H = sLoc > 1.2*data->image(r.imIdx).cols/2.0;
                        bool spotM = sLoc > data->image(r.imIdx).cols/3.0 &&
                            sLoc < 2.0*data->image(r.imIdx).cols/3.0;

                        if ( (ngram1H&&spot1H) || (ngram2H&&spot2H) || (ngramM&&spotM) )
                        {
                            scores.push_back(r.score);
                            rel.push_back(true);
                        }
                        else
                        {
                            scores.push_back(r.score);
                            rel.push_back(false);
                            //Insert a dummy result for the correct spotting to keep MAP accurate
                            scores.push_back(maxScore);
                            rel.push_back(true);
                        }

                    }
                }
            }
            vector<int> rank;
            for (int j=0; j < scores.size(); j++)
            {            
                float s = scores[j];
                //cout <<"score for "<<j<<" is "<<s<<". It is ["<<data->labels()[j]<<"], we are looking for ["<<text<<"]"<<endl;
                
                if (rel[j])
                {
                    int better=0;
                    int equal = 0;
                    
                    for (int k=0; k < scores.size(); k++)
                    {
                        if (k!=j)
                        {
                            float s2 = scores[k];
                            if (s2> s) better++;
                            else if (s2==s) equal++;
                        }
                    }
                    
                    
                    rank.push_back(better+floor(equal/2.0));
                    Nrelevants++;
                }
                
            }
            qsort(rank.data(), Nrelevants, sizeof(int), sort_xxx);
            
            //pP1[i] = p1;
            
            /* Get mAP and store it */
            for(int j=0;j<Nrelevants;j++){
                /* if rank[i] >=k it was not on the topk. Since they are sorted, that means bail out already */
                
                float prec_at_k =  ((float)(j+1))/(rank[j]+1);
                //mexPrintf("prec_at_k: %f\n", prec_at_k);
                ap += prec_at_k;            
            }
            ap/=Nrelevants;
            
            #pragma omp critical (storeMAP)
            {
                queryCount++;
                map+=ap;
                cout<<"on spotting inst:"<<inst<<", "<<ngram<<"   ap: "<<ap<<endl;
                /*if (gram.compare(ngram)!=0)
                {
                    if (gramCount>0)
                    {
                        cout <<"ap for ["<<gram<<"]: "<<(gramMap/gramCount)<<endl;
                        gramCount=0;
                        gramMap=0;
                    }
                    gram=ngram;
                }
                gramMap+=ap;
                gramCount++;*/
            }
            
        }
        //cout <<"ap for ["<<gram<<"]: "<<(gramMap/gramCount)<<endl;
        
        cout<<"FULL map: "<<(map/queryCount)<<" for hy:"<<hy<<endl;
    }
}
