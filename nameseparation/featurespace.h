#ifndef FEATURESPACE_H
#define FEATURESPACE_H

#include <QVector>
#include <ngramtrainingexample.h>
#include <QMap>
#include <memory>


class FeatureSpace
{
public:
    FeatureSpace();
    
    //This uses the feature vector of the instance to place it in the space
    void init(QVector<NGramTrainingInstance*>){}
    
    
    QMap<double,std::pair<std::shared_ptr<NGramPossibleInstance>,std::shared_ptr<NGramTrainingInstance> > > findKNearestNeighbors(std::shared_ptr<NGramPossibleInstance> candidate, int k){}
};

#endif // FEATURESPACE_H
