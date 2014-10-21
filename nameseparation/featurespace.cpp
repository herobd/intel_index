#include "featurespace.h"

FeatureSpace::FeatureSpace()
{
}

float myDist (const std::shared_ptr<NGramInstance> &o1, const std::shared_ptr<NGramInstance> &o2)
{
    return o1->getFeatureVector().distance(o2->getFeatureVector());//TODO likely needs refined
}

void FeatureSpace::init(QVector<std::shared_ptr<NGramTrainingInstance> > trainingData)
{
    rawdata = trainingData;
    FeatureSpace::MyDataset castData;//(trainingData.begin(),trainingData.end());
    for (std::shared_ptr<NGramTrainingInstance> p : trainingData)
    {
        castData.append(std::dynamic_pointer_cast<NGramInstance>(p));
    }
    
    oracle = new VectorOracle<MyDataset, std::shared_ptr<NGramInstance> >(castData, myDist);
    index = KGraph::create();
    
    KGraph::IndexParams params;  // safe to use default value, see document for details.
    index->build(*oracle, params, NULL);
}

void FeatureSpace::save(QString path)
{
    if (index != NULL)
        index->save(path.toAscii());
}

QMap<double,std::pair<std::shared_ptr<NGramPossibleInstance>,std::shared_ptr<NGramTrainingInstance> > > FeatureSpace::findKNearestNeighbors(std::shared_ptr<NGramPossibleInstance> candidate, int k)
{
    std::shared_ptr<NGramInstance> query = std::dynamic_pointer_cast<NGramInstance>(candidate);
    
    KGraph::SearchParams params;
    params.K = k;
    std::vector<unsigned> knn(k);   // to save K-NN ids.
    
    index->search(oracle->query(query), params, &knn[0], NULL);
    
    QMap<double,std::pair<std::shared_ptr<NGramPossibleInstance>,std::shared_ptr<NGramTrainingInstance> > > ret;
    for (unsigned i : knn)
    {
        double dist = candidate->getFeatureVector().distance(rawdata[i]->getFeatureVector());
        ret[dist] = std::make_pair(candidate,rawdata[i]);
    }
    return ret;
}

//Flann can be paralellized, with MPI or other options
//QMap<double,std::pair<std::shared_ptr<NGramPossibleInstance>,std::shared_ptr<NGramTrainingInstance> > > findKNearestNeighborsForAll(QVector< std::shared_ptr<NGramPossibleInstance> > candidates, int k)
//{
    
    
    
//    flann::Matrix<float> query;
//    //enter candidates into query
    
    
//    flann::Matrix<int> indices(new int[query.rows*k], query.rows, k);
//    flann::Matrix<float> dists(new float[query.rows*k], query.rows, k);
    
//    // construct an randomized kd-tree index using 4 kd-trees
//    flann::Index<flann::L2<float> > index(dataset, flann::KDTreeIndexParams(4));
//    index.buildIndex();
    
//    // do a knn search, using 128 checks
//    index.knnSearch(query, indices, dists, k, flann::SearchParams(128));
    
//    QMap<double,std::pair<std::shared_ptr<NGramPossibleInstance>,std::shared_ptr<NGramTrainingInstance> > > ret;
//    for (int i=0; i<indices.rows; i++)
//    {
        
//    }
    
//    delete[] query.ptr();
//    delete[] indices.ptr();
//    delete[] dists.ptr();
    
    
//}
