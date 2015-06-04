#include "codebook.h"

#include <assert.h>
#include <sstream>
#include <iostream>

bool Codebook::twentythree;

Codebook::Codebook()
{
}

void Codebook::test()
{
    codebook.clear();
    for (double y=0.1; y<6; y+=2+.05*(int)y)
        for (double x=0.1; x<5; x+=2+1.33*(int)y)
            for (double z=0.1; z<5; z+=2)
        {
            vector<double> v= {x,y,z};
            push_back(v);
        }
    vector<double> t1= {0.1,.4,.2};
    int t1q=quantize(t1);
//    assert(t1q==0);
    
    vector<double> t2= {1,2,1};
    vector< tuple<int,float> > t2q = quantizeSoft(t2,2);  
//    assert(fabs(get<1>(t2q[0])-.5)<.001 && fabs(get<1>(t2q[1])-.5)<.001);
    
    vector<double> t3= {1,3,4};
    vector< tuple<int,float> > t3q = quantizeSoft(t3,4);  
//    assert(fabs(get<1>(t3q[0])-.333)<.001 && fabs(get<1>(t3q[1])-.333)<.001 && fabs(get<1>(t3q[2])-.333)<.001);
}

void Codebook::push_back(vector<double> & exe)
{
    codebook.push_back(exe);
}

int Codebook::quantize(const vector<double> &term) const
{
    assert(term.size() == depth());
    int best=-1;
    double min=9999999999;
    for (int i=0; i<codebook.size(); i++)
    {
        double dist=0;
        for (int f=0; f<term.size(); f++)
        {
            dist += pow(term[f]-codebook[i][f],2);
        }
        
        if (dist<min)
        {
            min=dist;
            best=i;
        }
//        cout << "dist " << dist << endl;
    }
//    cout << "best " << best << ", min " << min << endl;
    return best;
}

int Codebook::quantize(const vector<float> &term) const
{
    assert(term.size() == depth());
    int best=-1;
    double min=9999999999;
    for (int i=0; i<codebook.size(); i++)
    {
        double dist=0;
        for (int f=0; f<term.size(); f++)
        {
            dist += pow(term[f]-codebook[i][f],2);
        }
        
        if (dist<min)
        {
            min=dist;
            best=i;
        }
//        cout << "dist " << dist << endl;
    }
//    cout << "best " << best << ", min " << min << endl;
    return best;
}

vector< tuple<int,float> > Codebook::quantizeSoft(const vector<double> &term, int t) const
{
    vector<float> temp;
    for (double n : term) temp.push_back((float)n);
    return quantizeSoft(temp,t);
}

vector< tuple<int,float> > Codebook::quantizeSoft(const vector<float> &term, int t) const
{
    assert(term.size() == depth());
//    int best[t];
//    double min[t];
    vector< tuple<int,float> > q;
    for (int i=0; i<codebook.size(); i++)
    {
        double dist=0;
        for (int f=0; f<term.size(); f++)
        {
            dist += pow(term[f]-codebook[i][f],2);
        }
//        cout << i<<": "<<dist<<endl;
        auto it = q.rbegin();
        while(1)
        {
            if (q.size() == 0)
            {
//                cout << "add first"<<endl;
                q.push_back(make_tuple(i,(float)dist));
                break;
            }
            else if (it==q.rend())
            {
//                cout << "add insert begining"<<endl;
                q.insert(it.base(),make_tuple(i,(float)dist));
                break;
            }
            else if (dist>=get<1>(*it))
            {
                if (it == q.rbegin() || q.size()<t)
                {
//                    cout << "add end, 'cause "<<dist<<">="<<get<1>(*it)<<endl;
                    q.push_back(make_tuple(i,(float)dist));
                }
                else
                {
//                    cout << "add insert mid, 'cause "<<dist<<">="<<get<1>(*it)<<endl;
                    q.insert(it.base(),make_tuple(i,(float)dist));
                }
                break;
            }
            
            
            it++;
        }
        if (q.size() > t)
            q.pop_back();
//        cout << "dist " << dist << endl;
    }
    
    vector< tuple<int,float> > ret; 
    //Set up and perform a least squares solve
//    cv::Mat X(term.size(),t,CV_32F);
//    for (int i=0; i<term.size(); i++)
//        for (int j=0; j<t; j++)
//        {
//            X.at<float>(i,j)=codebook[get<0>(q[j])][i];
//        }
//    cv::Mat y(term.size(),1, CV_32F);
//    for (int i=0; i<term.size(); i++)
//    {
//        y.at<float>(i,0) = term[i];
//    }
    
//    cv::Mat inter = (X.t()*X).inv(cv::DECOMP_CHOLESKY);
//    cv::Mat B = inter * X.t() * y;
//    cout <<"X:\n "<<X <<"\ninter:\n " << inter <<"\nB:\n " << B << endl;
    
//    for (int j=0; j<t; j++)
//    {
//        ret.push_back(make_tuple(get<0>(q[j]),B.at<float>(j,0)));
//    }
    
    //From "The Devil is in the Details"
    cv::Mat delta_i(term.size(),t,CV_32F);
    for (int i=0; i<term.size(); i++)
        for (int j=0; j<t; j++)
        {
            delta_i.at<float>(i,j) = term[i]-codebook[get<0>(q[j])][i];
        }
    
    float reg = 0.1;
    cv::Mat inter = (delta_i.t()*delta_i + reg*cv::Mat::eye(t,t,CV_32F)).inv();
//    cout << "sqr:\n "<<delta_i.t()*delta_i<<"\ninter:\n "<<inter<<endl;
    cv::Mat a = inter * delta_i.t()* cv::Mat::ones(term.size(),1,CV_32F);
    float norm = cv::norm(cv::Mat::ones(1,t,CV_32F)*a);
    for (int j=0; j<t; j++)
    {
        ret.push_back(make_tuple(get<0>(q[j]),a.at<float>(j,0)/norm));
    }
    
//    cout << "best " << best << ", min " << min << endl;
    return ret;
}

int Codebook::quantize(const cv::Mat &term) const
{
    assert(term.cols == depth());
    int best=-1;
    double min=9999999999;
    for (int i=0; i<codebook.size(); i++)
    {
        double dist=0;
        for (int f=0; f<term.cols; f++)
        {
            dist += pow(term.at<float>(0,f)-codebook[i][f],2);
        }
        
        if (dist<min)
        {
            min=dist;
            best=i;
        }
//        cout << "dist " << dist << endl;
    }
//    cout << "best " << best << ", min " << min << endl;
    return best;
}




void Codebook::save(string filePath)
{
    ofstream file;
    file.open (filePath, ios::out);
    
    file << codebook.size() << endl;
    file << codebook[0].size() << endl;
    for (vector<double> exemplar : codebook)
    {
        for (double val : exemplar)
        {
            file << val << endl;
        }
    }
    file.close();
}

void Codebook::readIn(string filePath)
{
    ifstream file;
    file.open (filePath, ios::in);
    string line;
    getline (file,line);
    int numOfEntries = stoi(line);
//    assert(numOfEntries==CODEBOOK_SIZE);
    getline (file,line);
    int numOfFeatures = stoi(line);
    for (int n=0; n < numOfEntries; n++)
    {
        vector<double> exemplar;
        for (int f=0; f < numOfFeatures; f++)
        {
            getline (file,line);
            int val = stoi(line);
            exemplar.push_back(val);
        }
        push_back(exemplar);
    }
    file.close();
}
void Codebook::readInCSV(string filePath)
{
    ifstream file;
    file.open (filePath, ios::in);
    string line;
//    getline (file,line);
//    int numOfEntries = stoi(line);
//    assert(numOfEntries==CODEBOOK_SIZE);
//    getline (file,line);
//    int numOfFeatures = stoi(line);
//    for (int n=0; n < numOfEntries; n++)
//    {
//        vector<double> exemplar;
//        for (int f=0; f < numOfFeatures; f++)
//        {
//            getline (file,line);
//            int val = stoi(line);
//            exemplar.push_back(val);
//        }
//        push_back(exemplar);
//    }
    
    while (getline(file,line))
    {
        vector<double> exemplar;
        
        istringstream ss(line);
        string token;
        
        while(getline(ss, token, ',')) {
            exemplar.push_back( stod(token) );
        }
        double freq = exemplar.back();
        exemplar.pop_back();
        inverseDocFreq.push_back(freq);
        codebook.push_back(exemplar);
    }
    
    
    
    file.close();
}
