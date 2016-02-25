#include "embattspotter.h"

void EmbAttSpotter::test()
{
    vector<Mat>* temp_features_corpus = _features_corpus;
    _features_corpus=NULL;
    vector<Mat>* temp_feats_training = _feats_training;
    _feats_training=NULL;
    vector<Mat>* temp_batches_cca_att = _batches_cca_att;
    _batches_cca_att=NULL;
    vector<string>* tempcorpus_imgfiles = corpus_imgfiles;
    corpus_imgfiles = &testImages;
    vector<string>* temptraining_imgfiles=training_imgfiles;
    training_imgfiles=&testImages;
    int tempnumBatches=numBatches;
    numBatches=-1;
    vector<int> tempbatches_index=batches_index;
    vector<int> tempbatches_indexEnd=batches_indexEnd;
    string tempsaveName=saveName;
    saveName="test/tmp_saveTest";
    // delete files
    system(("rm "+saveName+"*").c_str());
    
    int tempgenericBatchSize=genericBatchSize;
    genericBatchSize=2;
    
    //TODO, tests go here  
    sinMat_cosMat_test(); 
    normalizeL2Columns_test();
    loadCorpus_test();
    spot_test();
    extract_feats_test();
    extract_FV_feats_fast_and_batch_test();
    features_corpus_test();
    feats_training_test();
    phow_test();
    getImageDescriptorFV_test();
    batches_cca_att_test();
    
    
    delete _features_corpus;
    _features_corpus=temp_features_corpus;
    delete _feats_training;
    _feats_training=temp_feats_training;
    delete _batches_cca_att;
    _batches_cca_att=temp_batches_cca_att;
    corpus_imgfiles=tempcorpus_imgfiles;
    training_imgfiles=temptraining_imgfiles;
    numBatches=tempnumBatches;
    batches_index=tempbatches_index;
    batches_indexEnd=tempbatches_indexEnd;
    saveName=tempsaveName;
    genericBatchSize=tempgenericBatchSize;
}


void EmbAttSpotter::loadCorpus_test()
{
    //TODO 
}

void EmbAttSpotter::spot_test()
{
    //TODO
}

void EmbAttSpotter::extract_feats_test()
{
    
    //TODO
}


void EmbAttSpotter::extract_FV_feats_fast_and_batch_test()
{
    
    vector<Mat>* res = extract_FV_feats_fast_and_batch(testImages,NULL,NULL,4);
    assert(res->size()==1);
    assert(res->at(0).rows==4);
    delete res;
    vector<int> start, end;
    res = extract_FV_feats_fast_and_batch(testImages,&start,&end,2);
    assert(res->size()==2 && start.size()==2 && end.size()==2);
    assert(res->at(0).rows==2);
    assert(res->at(1).rows==2);
    assert(start[0]==0 && end[0]==2);
    assert(start[1]==2 && end[1]==4);
    delete res;
    res = extract_FV_feats_fast_and_batch(testImages,&start,&end,1);
    assert(res->size()==4 && start.size()==4 && end.size()==4);
    assert(res->at(0).rows==1);
    assert(res->at(1).rows==1);
    assert(res->at(3).rows==1);
    assert(res->at(4).rows==1);
    assert(start[0]==0 && end[0]==1);
    assert(start[1]==1 && end[1]==2);
    assert(start[3]==3 && end[1]==4);
    delete res;
    res = extract_FV_feats_fast_and_batch(testImages,&start,&end,3);
    assert(res->size()==2 && start.size()==2 && end.size()==2);
    assert(res->at(0).rows==3);
    assert(res->at(1).rows==1);
    assert(start[0]==0 && end[0]==3);
    assert(start[1]==3 && end[1]==4);
    delete res;
}

void EmbAttSpotter::features_corpus_test()
{
    vector<Mat> res1 = features_corpus();//create
    delete _features_corpus;
    _features_corpus=NULL;//clear
    vector<Mat> res2 = features_corpus();//load
    
    assert(res1.size() == res2.size());
    for (int i=0; i<res1.size(); i++)
    {
        assert(res1[i].rows==res2[i].rows);
        assert(res1[i].cols==res2[i].cols);
        for (int r=0; r<res1[i].rows; r++)
            for (int c=0; c<res1[i].cols; c++)
                assert(res1[i].at<float>(r,c)==res2[i].at<float>(r,c));
    }
}



void EmbAttSpotter::feats_training_test()
{
    vector<Mat> res1 = feats_training();//create
    delete _feats_training;
    _feats_training=NULL;//clear
    vector<Mat> res2 = feats_training();//load
    assert(res1.size()==1);
    assert(res1.size() == res2.size());
    for (int i=0; i<res1.size(); i++)
    {
        assert(res1[i].rows==res2[i].rows);
        assert(res1[i].cols==res2[i].cols);
        for (int r=0; r<res1[i].rows; r++)
            for (int c=0; c<res1[i].cols; c++)
                assert(res1[i].at<float>(r,c)==res2[i].at<float>(r,c));
    }
}


void EmbAttSpotter::phow_test()
{//TODO
    
}

void EmbAttSpotter::getImageDescriptorFV_test()
{//TODO
    
}

void EmbAttSpotter::batches_cca_att_test()
{
    assert(numBatches!=-1);
    vector<Mat> res1 = batches_cca_att();//create
    delete _batches_cca_att;
    _batches_cca_att=NULL;//clear
    vector<Mat> res2 = batches_cca_att();//load
    
    assert(res1.size() == res2.size());
    for (int i=0; i<res1.size(); i++)
    {
        assert(res1[i].rows==res2[i].rows);
        assert(res1[i].cols==res2[i].cols);
        for (int r=0; r<res1[i].rows; r++)
            for (int c=0; c<res1[i].cols; c++)
                assert(res1[i].at<float>(r,c)==res2[i].at<float>(r,c));
    }
}


void EmbAttSpotter::sinMat_cosMat_test()
{
    Mat v = Mat_<float>(2,2, { 0.0, 0.123, 1.89, -0.33});
    
    t=sinMat(v);
    assert(abs(t.at<float>(0,0)-(0))<.00001);
    assert(abs(t.at<float>(0,1)-(-0.45990349))<.00001);
    assert(abs(t.at<float>(1,0)-(0.9494856148))<.00001);
    assert(abs(t.at<float>(1,1)-(-0.324043028))<.00001);
    
    t=cosMat(v);
    assert(abs(t.at<float>(0,0)-(1))<.00001);
    assert(abs(t.at<float>(0,1)-(.9924450321))<.00001);
    assert(abs(t.at<float>(1,0)-(-0.312810559))<.00001);
    assert(abs(t.at<float>(1,1)-(0.9460423435))<.00001);
}

void EmbAttSpotter::normalizeL2Columns_test()
{
    Mat v = Mat_<float>(1,2, { 2.0, 1.0});
    normalizeL2Columns(v);
    assert(v.at<float>(0,0)==1.0 && v.at<float>(0,1)==1.0);
    
    v = Mat_<float>(2,2, { 4.0, 1.0,
                           3.0, 0.0 });
    normalizeL2Columns(v);
    assert(v.at<float>(0,1)==1.0);
    assert(v.at<float>(1,1)==0.0);
    assert(v.at<float>(0,0)==0.8);
    assert(v.at<float>(1,0)==0.6);
    
}

void EmbAttSpotter::embed_labels_PHOC_test()
{
    vector<string> t = {"aaaa","bbbb","aabb","abab","abcdefgh"};
    Mat* res = embed_labels_PHOC(t);
    //TODO
    
    delete res;
}
