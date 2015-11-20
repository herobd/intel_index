#ifndef ENHANCEDBOVWTESTS_H
#define ENHANCEDBOVWTESTS_H
#include "enhancedbovw.h"
//#include <mpi.h>
#include <omp.h>
#include <limits>
class EnhancedBoVWTests
{
public:
    static void experiment(EnhancedBoVW &bovw, string locationCSVPath, string exemplarDirPath, string dataDirPath, int dataSize, int numExemplarsPer, string fileExt, bool skip);
//    static void experiment_dist(EnhancedBoVW &bovw, string locationCSVPath, string exemplarDirPath, string dataDirPath, int dataSize, int numExemplarsPer, string fileExt, bool skip);
    static void experiment_Aldavert(EnhancedBoVW &bovw, string locationCSVPath, string dataDirPath, int dataSize, string fileExt, string outfile);
//    static void experiment_Aldavert_dist(EnhancedBoVW &bovw, string locationCSVPath, string dataDirPath, int dataSize, string fileExt);
    static void experiment_Aldavert_dist_batched(EnhancedBoVW &bovw, string locationCSVPath, string dataDirPath, int dataSize, string fileExt, int batchNum, int numOfBatches, string outfile);
    
    static void experiment_Aldavert_dist_batched_test(int scenario);
    static void test(EnhancedBoVW& bovw);
    static void createGGobiFile(EnhancedBoVW &bovw, string locationCSVPath, string dataDirPath, string fileExt, int numWords, string outfile);
    static void drawData(EnhancedBoVW &bovw, string locationCSVPath, string dataDirPath, string fileExt, int numWords, string outfile);
};


#endif // ENHANCEDBOVWTESTS_H
