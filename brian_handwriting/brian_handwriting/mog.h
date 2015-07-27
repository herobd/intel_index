#ifndef MOG_H
#define MOG_H

#include <list>
#include <map>
#include "opencv2/core/core.hpp"

#include "grapheme.h"
#include "som/src/stdafx.h"
#include "som/src/LibSOM/som.h"

#define SPECTURM_SIZE 41

using namespace cv;
using namespace std;

class MOG
{
public:
    MOG();
    ~MOG(){delete som;}
    void save(string filePath);
    void load(string filePath);
    void train(const map<char, list<const Grapheme*> >& graphemes, int epochs=500, int demX=9, int demY=9);
    void extractFeatures( const Grapheme* g, float* features);
    int getWinningNode(const Grapheme* g);
    double boxdist(int node1, int node2);
    int getNumClasses(){return som->get_nodes_number();}
    
private:
    SOM* som;//http://www.codeproject.com/Articles/21385/Kohonen-s-Self-Organizing-Maps-in-C-with-Applicati#Using_the_code
    bool trained;
    int mod(int a, int b)
    {
        while (a<0) a+=b;
        return a%b;
    }
};

#endif // MOG_H
