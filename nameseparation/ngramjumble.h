#ifndef NGRAMJUMBLE_H
#define NGRAMJUMBLE_H

/**
  A datastructure
  This class is responsible for storing the possible n-grams, their
  morphscores and chunk windows in a manner that they can be statistically
  aligned by a name-lexicon model.
  
  This class is responsible for giving NORMALIZED morph scores, perhaps by
  using the standard deviation, either as a whole or by n-gram level.
  
  If a normalized score is too bad, we may choose to return an <UNK> token
**/

#include "QString"
#include <utility>

class NGramJumble
{
public:
    NGramJumble();
    
    
    void addNGram(int ngram, QString token, double disimilarityScore, std::pair<int,int> chunkWindow){}
};

#endif // NGRAMJUMBLE_H
