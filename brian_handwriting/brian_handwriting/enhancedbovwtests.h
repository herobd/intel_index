#ifndef ENHANCEDBOVWTESTS_H
#define ENHANCEDBOVWTESTS_H
#include "enhancedbovw.h"
//#include <mpi.h>
#include <omp.h>
class EnhancedBoVWTests
{
public:
    static void experiment(EnhancedBoVW &bovw, string locationCSVPath, string exemplarDirPath, string dataDirPath, int dataSize, int numExemplarsPer, string fileExt, bool skip);
//    static void experiment_dist(EnhancedBoVW &bovw, string locationCSVPath, string exemplarDirPath, string dataDirPath, int dataSize, int numExemplarsPer, string fileExt, bool skip);
    static void experiment_Aldavert(EnhancedBoVW &bovw, string locationCSVPath, string dataDirPath, int dataSize, string fileExt, string outfile);
//    static void experiment_Aldavert_dist(EnhancedBoVW &bovw, string locationCSVPath, string dataDirPath, int dataSize, string fileExt);
    static void experiment_Aldavert_dist_batched(EnhancedBoVW &bovw, string locationCSVPath, string dataDirPath, int dataSize, string fileExt, int batchNum, int numOfBatches, string outfile);
    
    static void experiment_Aldavert_dist_batched_test(int scenario);
};


#endif // ENHANCEDBOVWTESTS_H
