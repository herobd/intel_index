#ifndef FEATURESPACE_H
#define FEATURESPACE_H

#include <QVector>
#include <ngramtrainingexample.h>
#include <QMap>
#include <memory>
#include <flann/flann.hpp>
#include <kgraph.h>
#include <vector>

using namespace kgraph;

class FeatureSpace
{
//    class std::shared_ptr<NGramInstance>;
    

    typedef  QVector< std::shared_ptr<NGramInstance> > MyDataset;
    
    
    
    
public:
    FeatureSpace();
    
    //This uses the feature vector of the instance to place it in the space
    void init(QVector< std::shared_ptr<NGramTrainingInstance> >);
    
    QMap<double,std::pair<std::shared_ptr<NGramPossibleInstance>,std::shared_ptr<NGramTrainingInstance> > > findKNearestNeighbors(std::shared_ptr<NGramPossibleInstance> candidate, int k);
    
//    QMap<double,std::pair<std::shared_ptr<NGramPossibleInstance>,std::shared_ptr<NGramTrainingInstance> > > findKNearestNeighborsForAll(QList< std::shared_ptr<NGramPossibleInstance> > candidates, int k);
    
    void save(QString path);
    
private:
    QVector< std::shared_ptr<NGramTrainingInstance> > rawdata;
//    flann::Matrix<float> dataset;
    MyDataset data;
    KGraph *index;
    VectorOracle<MyDataset, std::shared_ptr<NGramInstance> >* oracle;
};

#endif // FEATURESPACE_H
