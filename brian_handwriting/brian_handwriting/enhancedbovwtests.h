#ifndef ENHANCEDBOVWTESTS_H
#define ENHANCEDBOVWTESTS_H
#include "enhancedbovw.h"
//#include <mpi.h>
#include <omp.h>
#include <limits>
class EnhancedBoVWTests
{
public:
    static void experiment(EnhancedBoVW &bovw, string locationCSVPath, string exemplarDirPath, string dataDirPath, int dataSize, int numExemplarsPer, string fileExt, string outfile, bool skip);
//    static void experiment_dist(EnhancedBoVW &bovw, string locationCSVPath, string exemplarDirPath, string dataDirPath, int dataSize, int numExemplarsPer, string fileExt, bool skip);
    static void experiment_Aldavert(EnhancedBoVW &bovw, string locationCSVPath, string dataDirPath, int dataSize, string fileExt, string outfile);
//    static void experiment_Aldavert_dist(EnhancedBoVW &bovw, string locationCSVPath, string dataDirPath, int dataSize, string fileExt);
    static void experiment_dist_batched(EnhancedBoVW &bovw, string locationCSVPath, string exemplarDirPath, string dataDirPath, int dataSize, int numExemplarsPer, string fileExt, int batchNum, int numOfBatches, string outfile, bool process=true);
    static void experiment_Aldavert_dist_batched(EnhancedBoVW &bovw, string locationCSVPath, string dataDirPath, int dataSize, string fileExt, int batchNum, int numOfBatches, string outfile);
    
    static multimap<double,int> experiment_Aldavert_single(EnhancedBoVW &bovw, string locationCSVPath, string dataDirPath, int dataSize, string fileExt, Mat query, string gt);
    static void experiment_Aldavert_dist_batched_test(int scenario);
    static void test(EnhancedBoVW& bovw);
    static void createGGobiFile(EnhancedBoVW &bovw, string locationCSVPath, string dataDirPath, string fileExt, int numWords, string outfile);
    static void drawData(EnhancedBoVW &bovw, string locationCSVPath, string dataDirPath, string fileExt, int numWords, string outfile);
    static void drawDataForWords(EnhancedBoVW &bovw, string locationCSVPath, string dataDirPath, string fileExt, const vector<string>& wordsToDraw, string outfile);
    static void compareDataForWords(EnhancedBoVW &bovw, string locationCSVPath, string dataDirPath, string fileExt, const vector<string>& wordsToDraw, string ex_file, string outfile);
    
private:
    static void constructHistograms(EnhancedBoVW &bovw, string locationCSVPath, string dataDirPath, string fileExt, string outfile, const map<string,vector<int> >& locations, const vector<string>& wordsToDraw);
    static void compareHistograms(EnhancedBoVW &bovw, string locationCSVPath, string dataDirPath, string fileExt, string outfile, const map<string,vector<int> >& locations, const vector<string>& wordsToDraw, const Mat& toCompare);
};


#endif // ENHANCEDBOVWTESTS_H
