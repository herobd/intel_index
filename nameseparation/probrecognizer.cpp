#include "probrecognizer.h"

ProbRecognizer::ProbRecognizer(const QMap<int,QVector<NGramTrainingInstance*> > *ngramTraining)
{
    foreach (int ngram, ngramTraining->keys())
    {
        trainingFeatureSpace[ngram].init(ngramTraining->value(ngram));
    }
}

/*This can potentailly be parallelized by dividing up the window partions
  Some problem spots are the filling of instancesToEvaluate, which  simply has to be none comparison based,
  and align().
  */
void ProbRecognizer::recog(const BPixelCollection *word)
{
    BChunker chunker(word);
    QVector<QPoint> ngramWindowSizes;
    
    NGramJumble jumble;//This could be done for each n-gram for speed, but for accuracy I'll do them all together.
    
    for (int ngram : trainingFeatureSpace.keys())
    {
        //We have a couple choices here
        //I'm choosing to pile all possible matches for n together, with th assumption being that certain window sizes will find better matches than others and we can weed the bad out
        //But one may choose to do each window size seperately
        
        QMap<double,std::pair<std::shared_ptr<NGramPossibleInstance>,std::shared_ptr<NGramTrainingInstance> > > instanceDistances;//distance,index
        for (int windowSize = ngramWindowSizes[ngram].y(); windowSize>=ngramWindowSizes[ngram].x(); windowSize--)
        {
            for (int windowOffset=0; windowOffset<chunker.number()-windowSize; windowOffset++)
            {
                std::shared_ptr<NGramPossibleInstance> candidate = chunker.getChunks(chunker.number()-windowSize-windowOffset,chunker.number()-windowOffset-1);
                int k = 5;//TODO this is an arbtrary assignment
                instanceDistances.unite(trainingFeatureSpace[ngram].findKNearestNeighbors(candidate,k));
            }
            
        }
        
        //do we take the top x? the top y%? all below threshold z?
        QList<std::pair<std::shared_ptr<NGramPossibleInstance>,std::shared_ptr<NGramTrainingInstance> >> instancesToEvaluate;
        //TODO: fill instancesToEvaluate
        
        
        
        for (std::pair<std::shared_ptr<NGramPossibleInstance>,std::shared_ptr<NGramTrainingInstance> > p : instancesToEvaluate)
        {
            double morphScore = getMorphScore(std::dynamic_pointer_cast<NGramInstance>(p.first),std::dynamic_pointer_cast<NGramInstance>(p.second));
            jumble.addNGram(ngram,p.second->getText(),morphScore,p.first->getChunkWindow());
        }
    }
    
    NGramModel nameNGramModel;//TODO init
    
    //<probabilty, list of ngram strings with <UNK> wild grams> i.e. <0.00123,["M","<UNK>","SON"]>
    QMap<double,QList<QString> > ngramAlignmentProbabilities = align(jumble,nameNGramModel);
    
    
    
}

double ProbRecognizer::getMorphScore(std::shared_ptr<NGramInstance> first, std::shared_ptr<NGramInstance> second)
{
    //TODO
    return 0;
}

QMap<double,QList<QString> > ProbRecognizer::align(const NGramJumble &jumble, const NGramModel &model)
{
    //TODO
}
