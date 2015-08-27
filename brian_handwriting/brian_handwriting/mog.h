#ifndef MOG_H
#define MOG_H

#include <list>
#include <map>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "grapheme.h"
#include "som/src/stdafx.h"
#include "som/src/LibSOM/som.h"
#include "defines.h"



#define NUM_MAIN_DIRECTIONS 4
#define NUM_DIRECTIONS 8
#define NUM_MAIN_ANGLES 4
#define NUM_ANGLES 8
#define DCC_RESAMPLE_SIZE 12

#define WH_RATIO 0
#define ENCLOSED WH_RATIO+1
#define RUN_LENGTH ENCLOSED+1
#define MAIN_DIRECTIONS RUN_LENGTH+1 //3
#define DIRECTION_COUNTS MAIN_DIRECTIONS+NUM_MAIN_DIRECTIONS //7
#define DIRECTION_CHANGES DIRECTION_COUNTS+NUM_DIRECTIONS //15
#define MAIN_ANGLES DIRECTION_CHANGES+1 //16
#define ANGLE_COUNTS MAIN_ANGLES+NUM_MAIN_ANGLES //20
#define RESAMPLED_DCC ANGLE_COUNTS+NUM_ANGLES //28
#define MAX_RUN_DIRECTION RESAMPLED_DCC+DCC_RESAMPLE_SIZE //40

#define SPECTURM_SIZE MAX_RUN_DIRECTION+1

#define LOOP_CONST 14
#define DOUBLE_LOOP_CONST 21

using namespace cv;
using namespace std;

class MOG
{
public:
    MOG();
    ~MOG(){if (trained) delete som;}
    void save(string filePath);
    void load(string filePath);
    void train(const map<char, list<const Grapheme*> >& graphemes, int epochs=500, int demX=9, int demY=9);
    void extractFeatures( const Grapheme* g, float* features);
    int getWinningNode(const Grapheme* g);
    double boxdist(int node1, int node2);
    int getNumClasses(){return som->get_nodes_number();}
    
    int nodeCol(int i)
    {
        const Node* n =som->get_node(i);
        return n->get_coords()[0];
    }
    int nodeRow(int i)
    {
        const Node* n =som->get_node(i);
        return n->get_coords()[1];
    }
    
    void unittest();
    
    
private:
    SOM* som;//http://www.codeproject.com/Articles/21385/Kohonen-s-Self-Organizing-Maps-in-C-with-Applicati#Using_the_code
    bool trained;
    
    void writeGraphemes(const Mat& img);
};

#endif // MOG_H
