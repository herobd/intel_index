#include "codebook.h"

#include <assert.h>
#include <sstream>

bool Codebook::twentythree;

Codebook::Codebook()
{
}

void Codebook::push_back(vector<double> & exe)
{
    codebook.push_back(exe);
}

int Codebook::quantize(const vector<double> &term) const
{
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

int Codebook::quantize(const cv::Mat &term) const
{
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
    assert(numOfEntries==CODEBOOK_SIZE);
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
