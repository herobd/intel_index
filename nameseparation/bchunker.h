#ifndef BCHUNKER_H
#define BCHUNKER_H

#include "BPixelCollection.h"
#include <memory>
#include "ngramtrainingexample.h"

class BChunker
{
public:
    BChunker(const BPixelCollection* img);
    
    //The number of chunks
    int number() const{}
    
    //range is inclusive
    std::shared_ptr<NGramPossibleInstance> getChunks(int windowStart, int windowEnd){}
    
//    BImage/DImage getChunks(int start=0, int end=-1);
};

#endif // BCHUNKER_H
