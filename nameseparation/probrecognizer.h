#ifndef PROBRECOGNIZER_H
#define PROBRECOGNIZER_H

#include <QVector>
#include "BPixelCollection.h"
#include "bchunker.h"
#include "ngramtrainingexample.h"
#include "ngramjumble.h"
#include <memory>
#include "featurespace.h"
#include "ngrammodel.h"

class ProbRecognizer
{
public:
    //presumes the word is cleaned of noise and slant is removed
    ProbRecognizer(const QMap<int,QVector< std::shared_ptr<NGramTrainingInstance> > > *ngramTraining);
    QVector<QString> topN(unsigned int n);
    void recog(const BPixelCollection *word);
    
private:
    double getMorphScore(std::shared_ptr<NGramInstance> first, std::shared_ptr<NGramInstance> second);
    QMap<double,QList<QString> > align(const NGramJumble &jumble, const NGramModel &model);
    
    QMap<int,FeatureSpace> trainingFeatureSpace;
};

#endif // PROBRECOGNIZER_H
