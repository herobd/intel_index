#ifndef MOG_H
#define MOG_H

#include "opencv2/core/core.hpp"

#include "grapheme.h"

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
    void train(const map<char, list<Grapheme*> >& graphemes, int epochs, int demX=9, int demY=9);
    void extractFeatures( const Grapheme* g, float* features);
    int getWinningNode(const Grapheme* g);
    double boxdist(int node1, int node2);
    void getNumClasses(){return som->get_nodes_number();}
    
private:
    SOM* som;
    bool trained;
};

#endif // MOG_H
