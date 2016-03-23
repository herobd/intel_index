#include "embattspotter.h"
///extern "C" {
///#include "matio.h"
///}
#include "gwdataset.h"

void EmbAttSpotter::test()
{
    testImages = {"test/testImages/small0.png","test/testImages/small1.png","test/testImages/small2.png","test/testImages/small3.png","test/testImages/small4.png"};
    vector<Mat>* temp_features_corpus = _features_corpus;
    _features_corpus=NULL;
    vector<Mat>* temp_feats_training = _feats_training;
    _feats_training=NULL;
    vector<Mat>* temp_batches_cca_att = _batches_cca_att;
    _batches_cca_att=NULL;
    vector<string>* tempcorpus_imgfiles = corpus_imgfiles;
    corpus_imgfiles = &testImages;
    vector<string>* temptraining_imgfiles=training_imgfiles;
    vector<string>* temptraining_labels=training_labels;
    training_imgfiles=&testImages;
    int tempnumBatches=numBatches;
    numBatches=-1;
    vector<int> tempbatches_index=batches_index;
    vector<int> tempbatches_indexEnd=batches_indexEnd;
    string tempsaveName=saveName;
    saveName="test/tmp_saveTest";
    Mat* temp_phocsTr = _phocsTr;
    
    
    // delete files
    system(("rm "+saveName+"*").c_str());
    
    int tempgenericBatchSize=genericBatchSize;
    genericBatchSize=2;
    int tempnumWordsTrain=numWordsTrain;
    numWordsTrain=testImages.size();
    int tempnum_samples_PCA = num_samples_PCA;
    num_samples_PCA = 5000;
    
    //TODO, tests go here  
    sinMat_cosMat_test(); 
    normalizeL2Columns_test();
    otsuBinarization_test();
    embed_labels_PHOC_test();
    DoBB_test();
    loadCorpus_test();
    spot_test();
    extract_feats_test();
    extract_FV_feats_fast_and_batch_test();
    features_corpus_test();
    feats_training_test();
    phow_test();
    getImageDescriptorFV_test();
    batches_cca_att_test();
    
    //Compare to mat files
    training_imgfiles=NULL;
    training_labels=NULL;
    training_dataset = new GWDataset("/home/brian/intel_index/brian_handwriting/EmbeddedAtt_Almazan/datasets/GW/queries/queries.gtp","/home/brian/intel_index/brian_handwriting/EmbeddedAtt_Almazan/datasets/GW/images/");
    
    phocsTr_test();
    
    delete _features_corpus;
    _features_corpus=temp_features_corpus;
    delete _feats_training;
    _feats_training=temp_feats_training;
    delete _batches_cca_att;
    _batches_cca_att=temp_batches_cca_att;
    corpus_imgfiles=tempcorpus_imgfiles;
    training_imgfiles=temptraining_imgfiles;
    training_labels=temptraining_labels;
    numBatches=tempnumBatches;
    batches_index=tempbatches_index;
    batches_indexEnd=tempbatches_indexEnd;
    saveName=tempsaveName;
    genericBatchSize=tempgenericBatchSize;
    numWordsTrain=tempnumWordsTrain;
    num_samples_PCA=tempnum_samples_PCA;
    delete _phocsTr;
    _phocsTr = temp_phocsTr;
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
    
    vector<Mat>* res = extract_FV_feats_fast_and_batch(testImages,NULL,NULL,5);
    assert(res->size()==1);
    assert(res->at(0).rows==5);
    delete res;
    vector<int> start, end;
    res = extract_FV_feats_fast_and_batch(testImages,&start,&end,2);
    assert(res->size()==3 && start.size()==3 && end.size()==3);
    assert(res->at(0).rows==2);
    assert(res->at(1).rows==2);
    assert(res->at(2).rows==1);
    assert(start[0]==0 && end[0]==2);
    assert(start[1]==2 && end[1]==4);
    assert(start[2]==4 && end[2]==5);
    delete res;
    res = extract_FV_feats_fast_and_batch(testImages,&start,&end,1);
    assert(res->size()==5 && start.size()==5 && end.size()==5);
    assert(res->at(0).rows==1);
    assert(res->at(1).rows==1);
    assert(res->at(3).rows==1);
    assert(res->at(4).rows==1);
    assert(start[0]==0 && end[0]==1);
    assert(start[1]==1 && end[1]==2);
    assert(start[3]==3 && end[3]==4);
    delete res;
    res = extract_FV_feats_fast_and_batch(testImages,&start,&end,3);
    assert(res->size()==2 && start.size()==2 && end.size()==2);
    assert(res->at(0).rows==3);
    assert(res->at(1).rows==2);
    assert(start[0]==0 && end[0]==3);
    assert(start[1]==3 && end[1]==5);
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
    Mat res1 = feats_training();//create
    delete _feats_training;
    _feats_training=NULL;//clear
    Mat res2 = feats_training();//load
    //assert(res1.size()==1);
    //assert(res1.size() == res2.size());
    //for (int i=0; i<res1.size(); i++)
    //{
        assert(res1.rows==res2.rows);
        assert(res1.cols==res2.cols);
        for (int r=0; r<res1.rows; r++)
            for (int c=0; c<res1.cols; c++)
                assert(res1.at<float>(r,c)==res2.at<float>(r,c));
    //}
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
    Mat v = (Mat_<float>(2,2)<< 0.0, 0.123, 
                                1.89, -0.33);
    
    Mat t=sinMat(v);
    assert(abs(t.at<float>(0,0)-(0))<.00001);
    assert(abs(t.at<float>(0,1)-(.1226900900))<.00001);
    assert(abs(t.at<float>(1,0)-(0.9494856148))<.00001);
    assert(abs(t.at<float>(1,1)-(-0.324043028))<.00001);
    
    t=cosMat(v);
    assert(abs(t.at<float>(0,0)-(1))<.00001);
    assert(abs(t.at<float>(0,1)-(.9924450321))<.00001);
    assert(abs(t.at<float>(1,0)-(-0.313810559))<.00001);
    assert(abs(t.at<float>(1,1)-(0.9460423435))<.00001);
}

void EmbAttSpotter::normalizeL2Columns_test()
{
    Mat v = (Mat_<float>(1,2)<<  2.0, 1.0);
    normalizeL2Columns(v);
    assert(v.at<float>(0,0)==1.0 && v.at<float>(0,1)==1.0);
    
    v = (Mat_<float>(2,2)<<4.0, 1.0,
                           3.0, 0.0 );
    normalizeL2Columns(v);
    assert(v.at<float>(0,1)==1.0);
    assert(v.at<float>(1,1)==0.0);
    assert(abs(v.at<float>(0,0)-0.8)<.00001);
    assert(abs(v.at<float>(1,0)-0.6)<.00001);
    
}

void EmbAttSpotter::embed_labels_PHOC_test()
{
    vector<string> t = {"aaaa","bbbb","aabb","abab","abcdefgh"};
    Mat* res = embed_labels_PHOC(t);
    
    assert(res->cols==t.size());
    int sizeShould=0;
    for (int c : phoc_levels)
        sizeShould+=c*unigrams.size();
    for (int c : phoc_levels_bi)
        sizeShould+=c*bigrams.size();
    assert(res->rows==sizeShould);
    
    //aaaa
    assert(res->at<float>(0*unigrams.size()+0,0)==1);
    assert(res->at<float>(1*unigrams.size()+0,0)==1);
    for (int i=1; i<unigrams.size(); i++)
    {
        assert(res->at<float>(0*unigrams.size()+i,0)==0);
        assert(res->at<float>(1*unigrams.size()+i,0)==0);
    } 
    assert(res->at<float>(2*unigrams.size()+0,0)==1);
    assert(abs(res->at<float>(3*unigrams.size()+0,0)-.66666666)<.00001);
    assert(res->at<float>(4*unigrams.size()+0,0)==1);
    for (int i=1; i<unigrams.size(); i++)
    {
        assert(res->at<float>(2*unigrams.size()+i,0)==0);
        assert(res->at<float>(3*unigrams.size()+i,0)==0);
        assert(res->at<float>(4*unigrams.size()+i,0)==0);
    }  
    assert(res->at<float>(5*unigrams.size()+0,0)==1);
    assert(res->at<float>(6*unigrams.size()+0,0)==1);
    assert(res->at<float>(7*unigrams.size()+0,0)==1);
    assert(res->at<float>(8*unigrams.size()+0,0)==1);
    for (int i=1; i<unigrams.size(); i++)
    {
        assert(res->at<float>(5*unigrams.size()+i,0)==0);
        assert(res->at<float>(6*unigrams.size()+i,0)==0);
        assert(res->at<float>(7*unigrams.size()+i,0)==0);
        assert(res->at<float>(8*unigrams.size()+i,0)==0);
    }
    
    //bbbb
    assert(res->at<float>(0*unigrams.size()+1,1)==1);
    assert(res->at<float>(1*unigrams.size()+1,1)==1);
    for (int i=0; i<unigrams.size(); i++)
    {
        assert(i==1 || res->at<float>(0*unigrams.size()+i,1)==0);
        assert(i==1 || res->at<float>(1*unigrams.size()+i,1)==0);
    } 
    assert(res->at<float>(2*unigrams.size()+1,1)==1);
    assert(abs(res->at<float>(3*unigrams.size()+1,1)-.66666666)<.00001);
    assert(res->at<float>(4*unigrams.size()+1,1)==1);
    for (int i=0; i<unigrams.size(); i++)
    {
        assert(i==1 || res->at<float>(2*unigrams.size()+i,1)==0);
        assert(i==1 || res->at<float>(3*unigrams.size()+i,1)==0);
        assert(i==1 || res->at<float>(4*unigrams.size()+i,1)==0);
    }  
    assert(res->at<float>(5*unigrams.size()+1,1)==1);
    assert(res->at<float>(6*unigrams.size()+1,1)==1);
    assert(res->at<float>(7*unigrams.size()+1,1)==1);
    assert(res->at<float>(8*unigrams.size()+1,1)==1);
    for (int i=0; i<unigrams.size(); i++)
    {
        assert(i==1 || res->at<float>(5*unigrams.size()+i,1)==0);
        assert(i==1 || res->at<float>(6*unigrams.size()+i,1)==0);
        assert(i==1 || res->at<float>(7*unigrams.size()+i,1)==0);
        assert(i==1 || res->at<float>(8*unigrams.size()+i,1)==0);
    } 
    
    //aabb
    assert(res->at<float>(0*unigrams.size()+0,2)==1);
    assert(res->at<float>(1*unigrams.size()+1,2)==1);
    for (int i=0; i<unigrams.size(); i++)
    {
        assert(i==0 || res->at<float>(0*unigrams.size()+i,2)==0);
        assert(i==1 || res->at<float>(1*unigrams.size()+i,2)==0);
    } 
    assert(res->at<float>(2*unigrams.size()+0,2)==1);
    assert(abs(res->at<float>(3*unigrams.size()+0,2)-.66666666)<.00001);
    assert(abs(res->at<float>(3*unigrams.size()+1,2)-.66666666)<.00001);
    assert(res->at<float>(4*unigrams.size()+1,2)==1);
    for (int i=0; i<unigrams.size(); i++)
    {
        assert(i==0 || res->at<float>(2*unigrams.size()+i,2)==0);
        assert(i==1 || i==0 || res->at<float>(3*unigrams.size()+i,2)==0);
        assert(i==1 || res->at<float>(4*unigrams.size()+i,2)==0);
    }  
    assert(res->at<float>(5*unigrams.size()+0,2)==1);
    assert(res->at<float>(6*unigrams.size()+0,2)==1);
    assert(res->at<float>(7*unigrams.size()+1,2)==1);
    assert(res->at<float>(8*unigrams.size()+1,2)==1);
    for (int i=0; i<unigrams.size(); i++)
    {
        assert(i==0 || res->at<float>(5*unigrams.size()+i,2)==0);
        assert(i==0 || res->at<float>(6*unigrams.size()+i,2)==0);
        assert(i==1 || res->at<float>(7*unigrams.size()+i,2)==0);
        assert(i==1 || res->at<float>(8*unigrams.size()+i,2)==0);
    }  
    
    //abab
    assert(res->at<float>(0*unigrams.size()+0,3)==1);
    assert(res->at<float>(0*unigrams.size()+1,3)==1);
    assert(res->at<float>(1*unigrams.size()+0,3)==1);
    assert(res->at<float>(1*unigrams.size()+1,3)==1);
    for (int i=0; i<unigrams.size(); i++)
    {
        assert(i==0||i==1 || res->at<float>(0*unigrams.size()+i,3)==0);
        assert(i==0||i==1 || res->at<float>(1*unigrams.size()+i,3)==0);
    } 
    assert(res->at<float>(2*unigrams.size()+0,3)==1);
    assert(abs(res->at<float>(3*unigrams.size()+0,3)-.66666666)<.00001);
    assert(abs(res->at<float>(3*unigrams.size()+1,3)-.66666666)<.00001);
    assert(res->at<float>(4*unigrams.size()+1,3)==1);
    for (int i=0; i<unigrams.size(); i++)
    {
        assert(i==0 || i==1 || res->at<float>(2*unigrams.size()+i,3)==0);
        assert(i==1 || i==0 || res->at<float>(3*unigrams.size()+i,3)==0);
        assert(i==1 || i==0 || res->at<float>(4*unigrams.size()+i,3)==0);
    }  
    assert(res->at<float>(5*unigrams.size()+0,3)==1);
    assert(res->at<float>(6*unigrams.size()+1,3)==1);
    assert(res->at<float>(7*unigrams.size()+0,3)==1);
    assert(res->at<float>(8*unigrams.size()+1,3)==1);
    for (int i=0; i<unigrams.size(); i++)
    {
        assert(i==0 || res->at<float>(5*unigrams.size()+i,3)==0);
        assert(i==1 || res->at<float>(6*unigrams.size()+i,3)==0);
        assert(i==0 || res->at<float>(7*unigrams.size()+i,3)==0);
        assert(i==1 || res->at<float>(8*unigrams.size()+i,3)==0);
    }  
    
    //abcdefgh
    for (int i=0; i<4; i++)
    {
        assert(res->at<float>(0*unigrams.size()+i,4)==1);
        assert(res->at<float>(1*unigrams.size()+i,4)==0);
    } 
    for (int i=4; i<8; i++)
    {
        assert(res->at<float>(0*unigrams.size()+i,4)==0);
        assert(res->at<float>(1*unigrams.size()+i,4)==1);
    } 
    for (int i=8; i<unigrams.size(); i++)
    {
        assert(res->at<float>(0*unigrams.size()+i,4)==0);
        assert(res->at<float>(1*unigrams.size()+i,4)==0);
    } 
    
     
    assert(res->at<float>(5*unigrams.size()+0,4)==1);
    assert(res->at<float>(5*unigrams.size()+1,4)==1);
    assert(res->at<float>(6*unigrams.size()+2,4)==1);
    assert(res->at<float>(6*unigrams.size()+3,4)==1);
    assert(res->at<float>(7*unigrams.size()+4,4)==1);
    assert(res->at<float>(7*unigrams.size()+5,4)==1);
    assert(res->at<float>(8*unigrams.size()+6,4)==1);
    assert(res->at<float>(8*unigrams.size()+7,4)==1);
    
    for (int i=0; i<unigrams.size(); i++)
    {
        assert(i==0||i==1 || res->at<float>(5*unigrams.size()+i,4)==0);
        assert(i==2||i==3 || res->at<float>(6*unigrams.size()+i,4)==0);
        assert(i==4||i==5 || res->at<float>(7*unigrams.size()+i,4)==0);
        assert(i==6||i==7 || res->at<float>(8*unigrams.size()+i,4)==0);
    }   
    
    
    
    
    
    
    delete res;
}

void EmbAttSpotter::otsuBinarization_test()
{
    Mat testimg = imread("test/testImages/gray.png",CV_LOAD_IMAGE_GRAYSCALE);
    Mat res=otsuBinarization(testimg);
    /*imshow("test",testimg);
    //waitKey();
    
    Mat dst = res.clone();
    for (int r=0; r<dst.rows; r++)
        for (int c=0; c<dst.cols; c++)
        {
            if (dst.at<unsigned char>(r,c)==1)
                dst.at<unsigned char>(r,c)=0;
            else
                dst.at<unsigned char>(r,c)=255;
        }
    imshow("test",dst);
    //imshowB("res",res,255,0);
    waitKey();*/
    assert(res.at<unsigned char>(3,4)==0);
    assert(res.at<unsigned char>(4,10)==1);
    assert(res.at<unsigned char>(4,15)==0);
    assert(res.at<unsigned char>(4,79)==0);
    assert(res.at<unsigned char>(3,84)==1);
    assert(res.at<unsigned char>(4,89)==0);
    assert(res.at<unsigned char>(20,72)==0);
    assert(res.at<unsigned char>(12,72)==0);
    assert(res.at<unsigned char>(9,38)==1);
    assert(res.at<unsigned char>(24,53)==1);
    assert(res.at<unsigned char>(15,129)==0);
}

void EmbAttSpotter::DoBB_test()
{
    Mat img = imread("test/testImages/BrianDavis.png",CV_LOAD_IMAGE_GRAYSCALE);
    int x1,x2,y1,y2;
    DoBB(img,&x1,&x2,&y1,&y2);
    assert(38<=x1 && x1<=40);
    assert(300<=x2 && x2<=306);
    assert(29<=y1 && y1<=44);
    assert(60<=y2 && y2<=70);
    
    img = imread("test/testImages/hellogoodbye.png",CV_LOAD_IMAGE_GRAYSCALE);
    DoBB(img,&x1,&x2,&y1,&y2);
    assert(40<=x1 && x1<=48);
    assert(310<=x2 && x2<=324);
    assert(35<=y1 && y1<=48);
    assert(60<=y2 && y2<=70);
}

/*void EmbAttSpotter::phocsTr_test()
{
    mat_t    *matfp;
    matvar_t *matvar;

    matfp = Mat_Open("test/phocs.mat",MAT_ACC_RDONLY);
    assert( NULL != matfp );

    matvar = Mat_VarReadInfo(matfp,"phocs");
    if ( NULL == matvar ) {
        fprintf(stderr,"Variable 'phocs' not found, or error "
                       "reading MAT file\n");
    } else {
        assert(matvar->data_type==MAT_T_SINGLE);
        assert(matvar->rank==2);
        assert(phocsTr().rows==matvar->dims[0] && phocsTr().cols==matvar->dims[1]);
        for (int r=0; r<phocsTr().rows; r++)
            for (int c=0; c<phocsTr().cols; c++)
            {//I'm assuming column major ordering for the mat
                assert(abs(phocsTr().at<float>(r,c)-((float*)matvar->data)[c+r*phocsTr().cols])<0.0001);
            }
        
        Mat_VarFree(matvar);
    }
    Mat_Close(matfp);
}*/
