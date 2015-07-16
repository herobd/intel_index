#ifndef MOG_H
#define MOG_H

#include "opencv2/core/core.hpp"

#include "grapheme.h"

using namespace cv;
using namespace std;

class MOG
{
public:
    MOG();
    ~MOG(){delete som;}
    
    
private:
    SOM* som;
};

#endif // MOG_H
