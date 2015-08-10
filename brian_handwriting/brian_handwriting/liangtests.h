#ifndef LIANGTESTS_H
#define LIANGTESTS_H

#include "liang.h"
#include "opencv2/core/core.hpp"
#include <fstream>
#include <omp.h>

using namespace std;
using namespace cv;

class LiangTests
{
public:
    static void keyword_duplicationTest(int keywordNum, int testNum, string annFilePath, string imgDirPath, string imgNamePattern, string imgExt);
    
private:
    static bool sharedLetterNeeded(string word, map<char, int>& charCounts);
};

#endif // LIANGTESTS_H
