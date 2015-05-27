#include "hmmquery.h"

HMMQuery::HMMQuery(Mat &img, const Codebook* codebook, /**/vector<Vec3b> &colorTable)
{
    this->codebook=codebook;
    fHeight = 1+(img.size[0]-SIFT_EFFECTIVE_WINDOW_SIZE)/WINDOW_SIZE;
    fWidth = 1+(img.size[1]-SIFT_EFFECTIVE_WINDOW_SIZE)/WINDOW_SIZE;
    SIFTize(img);
    numStates = timeSpan*SCALE;
    
    a.resize(numStates);
    c.resize(numStates);
    mu.resize(numStates);
    U.resize(numStates);
    initProb.assign(numStates,0);
    initProb[0]=.8;
    initProb[1]=.2;
//    vector< vector< vector<int> > > associatedTermVectors(numStates);
    int i=0;
//    double to = timeSpan/(1.0*numStates);
    for (int state=0; state<numStates; state++)
    {
        c[state].assign(NUM_MIXTURES,0);
        mu[state].resize(NUM_MIXTURES);
        U[state].resize(NUM_MIXTURES);
        
        
        
        
        double Ni=0;
        for (; i < (int) ((state+1)*timeSpan/(1.0*numStates)); i++)
        {
//            associatedTermVectors[state].push_back(termVectors[i]);
            Ni+=1;
            
            if (NUM_MIXTURES == codebook->size())
            for (int feature : termVectors[i])
            {
                c[state][feature]++;
            }
        }
        
//        cout << Ni << endl;
        
//        double Ni=associatedtimeSpan;
        //a[state].assign(numStates,0);
        a[state][state]=(Ni-1.0)/Ni;
        a[state][state+1]=(1.0)/Ni;
        default_random_engine generator(38838);
        normal_distribution<double> distribution(0.0,1.0);
        //Mat example(NUM_MIXTURES,NUM_MIXTURES, CV_32F);
        for (int feature=0; feature<NUM_MIXTURES; feature++)
        {
            if (NUM_MIXTURES == codebook->size())
                c[state][feature]/=Ni;
            else
                c[state][feature]=distribution(generator);
            
            mu[state][feature]=Mat(NUM_MIXTURES,1, CV_32F,Scalar::all(0));
            mu[state][feature].at<double>(feature,0)=1;
            U[state][feature]=Mat(NUM_MIXTURES,NUM_MIXTURES, CV_32F,Scalar::all(0));
            for (int i=0; i<NUM_MIXTURES; i++)
            {
                U[state][feature].at<double>(i,i)=distribution(generator);
                for (int j=i+1; j<NUM_MIXTURES; j++)
                {
                    U[state][feature].at<double>(i,j)=distribution(generator);
                    U[state][feature].at<double>(j,i)=U[state][feature].at<double>(i,j);
                }
            }
        }
        
        
    }
    
    
    
    
    //Baum-Welch///////////////////
    
    baumwelch();
    
    
    ///////////////////////
    
    
    
    
    
    //save
    {
        
        Mat output = img.clone();
        cvtColor( output, output, CV_GRAY2RGB );
//        cout << timeSpan << " x " << termVectors[0].size() << endl;
        for (int i=0; i<timeSpan; i++)
        {
            for (int j=0; j<termVectors[0].size(); j++)
            {
//                cout << "val " << termVectors[i][j] << endl;
                output.at<Vec3b>(SIFT_EFFECTIVE_WINDOW_SIZE/2+j*WINDOW_SIZE,SIFT_EFFECTIVE_WINDOW_SIZE/2+i*SPACING)=colorTable[termVectors[i][j]];
                
                output.at<Vec3b>(SIFT_EFFECTIVE_WINDOW_SIZE/2+j*WINDOW_SIZE+1,SIFT_EFFECTIVE_WINDOW_SIZE/2+i*SPACING )=colorTable[termVectors[i][j]];
                output.at<Vec3b>(SIFT_EFFECTIVE_WINDOW_SIZE/2+j*WINDOW_SIZE ,SIFT_EFFECTIVE_WINDOW_SIZE/2+i*SPACING+1)=colorTable[termVectors[i][j]];
                output.at<Vec3b>(SIFT_EFFECTIVE_WINDOW_SIZE/2+j*WINDOW_SIZE+1,SIFT_EFFECTIVE_WINDOW_SIZE/2+i*SPACING+1)=colorTable[termVectors[i][j]];
            }
        }
        imwrite( "./query.png", output );
    }
}

void HMMQuery::baumwelch()
{
    while(1)
    {
        vector< vector<double> > alpha(timeSpan);//[t][i]
        
        
        alpha[0].resize(numStates);
        for (int state=0; state<numStates; state++)
        {
            alpha[0][state]=initProb[state]*prob(termVectors[0],state);
        }
        for (int t=1; t<timeSpan; t++)
        {
            alpha[t].resize(numStates);
            for (int state=0; state<numStates; state++)
            {
                double sum=0;
                for (int statePrev=0; statePrev<numStates; statePrev++)
                {
                    sum += alpha[t-1][statePrev]*a[statePrev][state];
                }
                alpha[t][state]=prob(termVectors[t],state) * sum;
            }
        }
        
        vector< vector<double> > beta(timeSpan);//[t][i]
        beta[timeSpan-1].resize(numStates);
        for (int state=0; state<numStates; state++)
        {
            beta[timeSpan-1][state]=1;
        }
        for (int t=timeSpan-2; t>=0; t--)
        {
            beta[t].resize(numStates);
            for (int state=0; state<numStates; state++)
            {
                double sum=0;
                for (int stateNext=0; stateNext<numStates; stateNext++)
                {
                    sum += beta[t+1][stateNext]*a[state][stateNext]*prob(termVectors[t+1],stateNext);
                }
                beta[t][state]= sum;
            }
        }
        
        vector< vector<double> > temp1(timeSpan);//[t][i]
        for (int t=0; t<timeSpan; t++)
        {
            temp1[t].resize(numStates);
            for (int state=0; state<numStates; state++)
            {
                double sum=0;
                for (int stateNext=0; stateNext<numStates; stateNext++)
                {
                    sum += alpha[t][stateNext] * beta[t][stateNext];
                }
                temp1[t][state] = (alpha[t][state] * beta[t][state]) / sum;
//                if (sum==0)
//                    cout << "1. " << alpha[t][state]<<"*"<<beta[t][state]<<"/"<<sum<<endl;
                
            }
        }
        
        vector< vector< vector<double> > > temp2(timeSpan);//[t][i][j]
        for (int t=0; t<timeSpan-1; t++)
        {
            temp2[t].resize(numStates);
            for (int i=0; i<numStates; i++)
            {
                temp2[t][i].resize(numStates);
                for (int j=0; j<numStates; j++)
                {
                    double sum=0;
                    for (int k=0; k<numStates; k++)
                    {
                        sum += alpha[t][k] * beta[t][k];
                    }
                    temp2[t][i][j]= (alpha[t][i]*a[i][j]*beta[t+1][j]*prob(termVectors[t+1],j))/sum;
//                    if (t==0 && i==0)
//                        cout <<"temp2[0][" <<j<<"]="<< temp2[t][i][j]<<endl;
//                    if (sum==0)
//                        cout << "2. " << alpha[t][i]<<"*"<<a[i][j]<<"*"<<beta[t+1][j]<<"*"<<prob(termVectors[t+1],j)<<"/"<<sum<<endl;
                }
            }
        }
        
        //update
        double delta=0;
        for (int state=0; state<numStates; state++)
        {
            delta += fabs(temp1[0][state]-initProb[state]);
//            cout << "d1:"<<temp1[0][state]<<" "<<initProb[state]<<endl;
            initProb[state]=temp1[0][state];
            
            double sumBot=0;
            for (int t=0; t<timeSpan-1; t++)
                sumBot += temp1[t][state];
//            if (sumBot==0)
//                cout <<"3. " <<sumBot<<endl;
            //ignore b updates
            for (int stateNext=0; stateNext<numStates; stateNext++)
            {
                double sumTop=0;
                
                for (int t=0; t<timeSpan-1; t++)
                {
                    sumTop += temp2[t][state][stateNext];
                    
                }
                delta += fabs((sumTop/sumBot)-a[state][stateNext]);
//                cout << "d2:"<<(sumTop/sumBot)<<" "<<a[state][stateNext]<<endl;
                a[state][stateNext] = sumTop/sumBot;
            }
            
            
        }
        
        //mixture stuff
        vector< vector< vector<double> > > gamma(timeSpan);
        for (int t=0; t<timeSpan-1; t++)
        {
            gamma[t].resize(numStates);
            for (int j=0; j<numStates; j++)
            {
                gamma[t][j].resize(NUM_MIXTURES);
                for (int k=0; k<NUM_MIXTURES; k++)
                {
                    double sum1=0;
                    for (int n=0; n<numStates; n++)
                    {
                        sum1 += alpha[t][n] * beta[t][n];
                    }
                    double sum2=0;
                    for (int m=0; m<NUM_MIXTURES; m++)
                    {
                        sum2 += c[j][m] * gauss(termVectors[t],mu[j][m],U[j][m]);
                    }
                    
                    gamma[t][j][k] = (alpha[t][j]*beta[t][j]/sum1) *
                                     (c[j][k]*gauss(termVectors[t],mu[j][k],U[j][k])/sum2);
                }
            }
        }
        
        //update mixture stuff
        for (int j=0; j<numStates; j++)
        {
            for (int k=0; k<NUM_MIXTURES; k++)
            {
                Mat sumTopMu(NUM_MIXTURES,1,CV_32F,Scalar::all(0));
                Mat sumTopU(NUM_MIXTURES,NUM_MIXTURES,CV_32F,Scalar::all(0));
                double sumBottom=0;
                double sumBottomC=0;
                for (int t=0; t<timeSpan; t++)
                {
                    sumTopMu += gamma[t][j][k]*makeO(termVectors[t]);
                    sumTopU += gamma[t][j][k] * (makeO(termVectors[t])-mu[j][k])*(makeO(termVectors[t])-mu[j][k]).t();
                    sumBottom += gamma[t][j][k];
                    for (int m=0; m<NUM_MIXTURES; m++)
                        sumBottomC+=gamma[t][j][m];
                }
                mu[j][k]=sumTopMu/sumBottom;
                U[j][k]=sumTopU/sumBottom;
                c[j][k]=sumBottom/sumBottomC;
            }
        }
        
//        cout << "delta=" << delta <<endl;
        if (delta<.3)break;
    }
}

//For current use, only prbabilty is needed
double HMMQuery::eval(const vector< vector<int> > &observation)//observations are feature nums
{
    //Viterbi algorithm
    map<int, map<int,double> > V;//[up to][state]
    
    //No speed ups
//    int furthestState=0;
    for (int state=0; state<numStates; state++)
    {
        if (initProb[state]>0)
        {
            V[0][state] = prob(observation[0],state)*initProb[state];
//            furthestState=state;
        }
//        else
//            break;//speedup
    }
    
//    cout << "V[0][0]="<<V[0][0]<<" "<<prob(observation[0],0)<<"*"<<initProb[0]<<endl;
    for (int pos=1; pos<observation.size(); pos++)
    {
//        cout << "pos " << pos << " ";
        for (int state=0; state<numStates; state++)
        {
//            if ()
//            {
                double Pyk=prob(observation[pos],state);
                double max=0;
                for (int prevState=0; prevState<numStates; prevState++)
                {
                    double val=a[prevState][state]*V[pos-1][prevState];
                    if (val>max)
                        max=val;
                    
//                    if (state==0 && prevState==0) cout<<"("<<a[prevState][state]<<","<<V[pos-1][prevState]<<") ";
                }
                V[pos][state]=max*Pyk;
//            }
//                cout<< state << ":" << max << " ";
        }
//        cout <<endl;
    }
    double ret=0;
    for (int state=0; state<numStates; state++)
    {
        if (V[observation.size()-1][state] > ret)
            ret=V[observation.size()-1][state];
    }
    return ret;
}


double HMMQuery::logEval(const vector< vector<int> > &observation)//observations are feature nums
{
    //Viterbi algorithm
    map<int, map<int,double> > V;//[up to][state]
    
    //No speed ups
//    int furthestState=0;
    for (int state=0; state<numStates; state++)
    {
        if (initProb[state]>0)
        {
            V[0][state] = log(prob(observation[0],state)) + log(initProb[state]);
//            furthestState=state;
        }
//        else
//            break;//speedup
    }
    
//    cout << "V[0][0]="<<V[0][0]<<" "<<prob(observation[0],0)<<"*"<<initProb[0]<<endl;
    for (int pos=1; pos<observation.size(); pos++)
    {
//        cout << "pos " << pos << " ";
        for (int state=0; state<numStates; state++)
        {
//            if ()
//            {
                double Pyk=prob(observation[pos],state);
                double max=0;
                for (int prevState=0; prevState<numStates; prevState++)
                {
                    double val=log(a[prevState][state])+V[pos-1][prevState];
                    if (val>max)
                        max=val;
                    
//                    if (state==0 && prevState==0) cout<<"("<<a[prevState][state]<<","<<V[pos-1][prevState]<<") ";
                }
                V[pos][state]=max+log(Pyk);
//            }
//                cout<< state << ":" << max << " ";
        }
//        cout <<endl;
    }
    double ret=0;
    for (int state=0; state<numStates; state++)
    {
        if (V[observation.size()-1][state] > ret)
            ret=V[observation.size()-1][state];
    }
    return exp(ret);
}


double HMMQuery::prob(const vector<int> &obs, int state)
{
//    double sum=0;
    
//    for(const int feature : obs)
//    {
//        sum += c[state][feature];
//    }
    
//    return sum/CODEBOOK_SIZE;
//    return sum/obs.size();
    
    double sum=0;
    for (int m=0; m<codebook->size(); m++)
    {
        sum+=c[state][m] * gauss(obs,mu[state][m],U[state][m]);
    }
    return sum;
}

Mat HMMQuery::makeO(const vector<int> &oVec)
{
    Mat o(NUM_MIXTURES,1,CV_32F,Scalar::all(0));
    for (int feature : oVec)
        o.at<double>(feature,0)=1;
    return o;
}

double HMMQuery::gauss(const vector<int> &oVec, const Mat &mu, const Mat &U)
{
    Mat o = makeO(oVec);
    //something
    Mat temp=(-.5*((o-mu).t()*U.inv()*(o-mu)));
    return ( 1/sqrt(pow(2*CV_PI,NUM_MIXTURES)*determinant(U)) ) * exp(temp.at<double>(0,0));
}

void HMMQuery::SIFTize(Mat &img)
{
#if USE_CUST_SIFT
    vector< vector<double> > descriptors;
#else
    Mat descriptors;
#endif
    
    
    
    for (int i =SIFT_EFFECTIVE_WINDOW_SIZE/2; i < img.size[1]-SIFT_EFFECTIVE_WINDOW_SIZE/2; i+=SPACING)
    {
        vector<KeyPoint> keyPoints;
        for (int j =SIFT_EFFECTIVE_WINDOW_SIZE/2; j < img.size[0]-SIFT_EFFECTIVE_WINDOW_SIZE/2; j+=WINDOW_SIZE)
        {
            //does angle need to be set? or is -1 proper?
            KeyPoint p(i,j,SIFT_WINDOW_SIZE);
            //keyPoints[(i/5)+(j/5)*fWidth] = (p);
            //assert((i/5)+(j/5)*fWidth < fWidth*fHeight);
            keyPoints.push_back(p);
        }
#if USE_CUST_SIFT
        CustomSIFT::extract(img,keyPoints,descriptors);
#else
        SIFT detector;// = new SIFT();
        detector(img,Mat(),keyPoints,descriptors,true);
#endif
        vector<int> window;
        //cout << "fHeight="<<fHeight<<" desc="<<descriptors.size()<<endl;
        assert(fHeight == descriptors.size());
        for (int r=0; r<fHeight; r++)
        {
#if USE_CUST_SIFT
            if(i==49 & r==1)
            {
                cout << "qy-dif ";
                for (int iii=0; iii<descriptors[r].size(); iii++)
                    cout<<descriptors[r][iii]<<",";
                cout<<endl;
            }
            int code = codebook->quantize(descriptors[r]);
#else
            vector<double> term;
            descriptors.row(r).copyTo(term);
            
            if(i==49 & r==1)
            {
                cout << "qy-dif ";
                for (int iii=0; iii<term.size(); iii++)
                    cout<<term[iii]<<",";
                cout<<endl;
            }

            else if(i==49 & r==2)
            {
                cout << "qy-nex ";
                for (int iii=0; iii<term.size(); iii++)
                    cout<<term[iii]<<",";
                cout<<endl;
            }
            
            int code = codebook->quantize(term);
            #endif
            
            window.push_back(code);
        }
        termVectors.push_back(window);
    }
    
    timeSpan=termVectors.size();
//    return termVectors;
    
    
}


