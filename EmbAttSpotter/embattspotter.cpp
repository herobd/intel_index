
#include "embattspotter_eval.cpp"
#if TEST_MODE
    #include "embattspotter_test.cpp"
#endif

#include <armadillo>

EmbAttSpotter::EmbAttSpotter(string saveName, bool useNumbers, int test_mode)
{
    
    _GMM.means=NULL;
    _GMM.covariances=NULL;
    _GMM.priors=NULL;
    
    _attModels=NULL;
    _embedding=NULL;
    _batches_cca_att=NULL;
    _features_corpus=NULL;
    
    SIFT_sizes={2,4,6,8,10,12};
    stride=3;
    magnif=6.0;
    windowsize=1.5;   
    contrastthreshold=0.005; 
    
    numWordsTrainGMM=500;
    minH = -1;//?
    PCA_dim = 62;
    #if TEST_MODE
    this->test_mode=test_mode;
    #else
    test_mode=0;
    #endif
    
    if (test_mode==1)
        num_samples_PCA = 5000;
    else
        num_samples_PCA = 2000000;
    
    numGMMClusters = 16;
    numSpatialX = 6;//num of bins for spatail pyr
    numSpatialY = 2;
    
    sgdparams_lbds = {1e-3,1e-4,1e-5};
    
    phoc_levels = {2, 3, 4, 5};
    unigrams = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};
    if (useNumbers)
    {
        for (char n : "0123456789") {
            //cout << "added numeral "<<n<<endl;
            if (n!=0x00)
                unigrams.push_back(n);
        }
    }
    phoc_levels_bi = {2};
    ifstream bFile("test/bigrams.txt");
    string bigram;
    while (getline(bFile,bigram) && bigrams.size()<50)
    {
        bigrams.push_back(bigram);
    }
    bFile.close();
    
    /* Prepare dict */
    
    for (int i=0; i<unigrams.size(); i++)
    {
        vocUni2pos[unigrams[i]] = i;
    }
    
    for (int i=0; i<bigrams.size(); i++)
    {
        vocBi2pos[bigrams[i]] = i;
    }
    
    
    int totalLevels = 0;
    for (int level : phoc_levels)
    {
        totalLevels+=level;
    }
    
    phocSize = totalLevels*unigrams.size();
    
    phocSize_bi=0;
    for (int level : phoc_levels_bi)
    {
        phocSize_bi+=level*bigrams.size();
    }

    
    
    this->saveName = saveName;//+"_sp"+to_string(numSpatialX)+"x"+to_string(numSpatialY);
    
    
    numBatches=-1;
    corpusSize=-1;
    corpus_imgfiles=NULL;
    training_imgfiles=NULL;
    training_labels=NULL;
    training_dataset=NULL;
    corpus_dataset=NULL;
    genericBatchSize=5000;
    
    
    
    
}

EmbAttSpotter::~EmbAttSpotter()
{
    if (_GMM.means!=NULL)
    {
        delete[] _GMM.means;
        delete[] _GMM.covariances;
        delete[] _GMM.priors;
    }
    if (_attModels!=NULL)
    {
        //_attModels->W.release();
        //_attModels->numPosSamples.release();
        delete _attModels;
    }
    if (_embedding!=NULL)
    {
        //_embedding->rndmatx.release();
       // _embedding->rndmaty.release();
        //_embedding->M.release();
        //_embedding->matt.release();
        //_embedding->mphoc.release();
        //_embedding->Wx.release();
        //_embedding->Wy.release();
        delete _embedding;
    }
    if (_features_corpus!=NULL)
    {
        //_features_corpus->release();
        for (Mat& m : *_features_corpus)
            m.release();
        delete _features_corpus;
    }
    if (_batches_cca_att!=NULL)
    {
        delete _batches_cca_att;
    }
}

void EmbAttSpotter::loadCorpus(string dir)
{
    //TODO perform line and word segmentation and then all extract features and if we are trained, embed the word images
    
    //For now assume segmented word images
    //
}

void checkNaN(const Mat& m)
{
    for (int c=0; c<m.cols; c++)
        for (int r=0; r<m.rows; r++)
            assert(m.at<float>(r,c)==m.at<float>(r,c));
    
    for (int c=0; c<m.cols; c++)
        for (int r=0; r<m.rows; r++)
            assert(m.at<float>(r,c)>=0);
}


vector<float> EmbAttSpotter::spot(const Mat& exemplar, string word, float alpha)
{
    assert(alpha>=0 && alpha<=1);
    assert(word.length()>0 || alpha==1);
    assert(exemplar.rows*exemplar.cols>1 || alpha==0);
    
    
    
    Mat im;
    exemplar.convertTo(im, CV_32FC1);
    Mat query_feats = extract_feats(im);
    
    Mat query_att = attModels().W.t()*query_feats.t();
    Mat query_phoc = Mat::zeros(phocSize+phocSize_bi,1,CV_32F);
    if (alpha < 1)
    {
        computePhoc(word, vocUni2pos, map<string,int>(),unigrams.size(), phoc_levels, phocSize, query_phoc,0);
        computePhoc(word, map<char,int>(), vocBi2pos,bigrams.size(), phoc_levels_bi, phocSize_bi, query_phoc,0);
    }
    
#ifdef SAYATT
    {
        cout <<"attb: ";
        for (int r=0; r<query_att.rows; r++)
            cout << query_att.at<float>(r,0)<<", ";
        cout<<endl;
        cout <<"phoc: ";
        for (int r=0; r<query_phoc.rows; r++)
            cout << query_phoc.at<float>(r,0)<<", ";
        cout<<endl;
    }
#endif

#ifdef TEST_MODE
    if (test_mode)
    {
        //Read in from MATLAB's results
        vector<vector<float> > csv;
        readCSV("test/attReprTe_test2.csv",csv);
        assert(csv.size()==query_att.cols && csv[0].size()==query_att.rows);
        for (int r=0; r<csv.at(0).size(); r++)
            for (int c=0; c<csv.size(); c++)
            {
                query_att.at<float>(r,c) = csv[c][r];
            }
        csv.clear();
        readCSV("test/phocTe_test2.csv",csv);
        assert(csv.size()==query_phoc.cols && csv[0].size()==query_phoc.rows);
        for (int r=0; r<csv.at(0).size(); r++)
            for (int c=0; c<csv.size(); c++)
            {
                query_phoc.at<float>(r,c) = csv[c][r];
            }
    }
#endif

    Mat matx = embedding().rndmatx;//(Rect(0,0,embedding().M,embedding().rndmatx.cols));
    Mat maty = embedding().rndmaty;//(Rect(0,0,embedding().M,embedding().rndmatx.cols));
    assert(matx.rows==embedding().M);
    assert(maty.rows==embedding().M);
    
    //checkNaN(query_phoc);
    //checkNaN(maty);
    
    Mat tmp = matx*query_att;
#ifdef TEST_MODE
    if (test_mode)
        compareToCSV(tmp,"test/attReprTe_tmp_test2.csv");
#endif
    vconcat(cosMat(tmp),sinMat(tmp),tmp);
    assert(embedding().matt.size() == tmp.size());
    Mat query_emb_att = (1/sqrt(embedding().M)) * tmp - embedding().matt;
#ifdef TEST_MODE
    if (test_mode)
        compareToCSV(query_emb_att,"test/attReprTe_emb_mean_test2.csv");
#endif
    tmp = maty*query_phoc;
#ifdef TEST_MODE
    if (test_mode)
        compareToCSV(tmp,"test/phocTe_tmp_test2.csv");
#endif
    //checkNaN(tmp);
    vconcat(cosMat(tmp),sinMat(tmp),tmp);
    //checkNaN(embedding().mphoc);
    //checkNaN(tmp);
    assert(embedding().mphoc.size() == tmp.size());
    Mat query_emb_phoc = (1/sqrt(embedding().M)) * tmp - embedding().mphoc;
#ifdef TEST_MODE
    if (test_mode)
        compareToCSV(query_emb_phoc,"test/phocTe_emb_mean_test2.csv");
#endif
    
    //checkNaN(embedding().Wy);
    //checkNaN(query_emb_phoc);
    
    Mat query_cca_att = embedding().Wx.t()*query_emb_att;
    Mat query_cca_phoc = embedding().Wy.t()*query_emb_phoc;
    
#ifdef TEST_MODE
    if (test_mode)
    {
        compareToCSV(query_cca_att,"test/attReprTe_cca_test2.csv");
        compareToCSV(query_cca_phoc,"test/phocTe_cca_test2.csv");
    }
#endif
    //checkNaN(query_cca_att);
    //checkNaN(query_cca_phoc);
    
    normalizeL2Columns(query_cca_att);
    normalizeL2Columns(query_cca_phoc);
#ifdef TEST_MODE
    if (test_mode)
    {
        compareToCSV(query_cca_att,"test/attReprTe_cca_norm_test2.csv");
        compareToCSV(query_cca_phoc,"test/phocTe_cca_norm_test2.csv");
    }
    return vector<float>();
#endif
    Mat query_cca_hy = query_cca_att*alpha + query_cca_phoc*(1-alpha);
    
    /*cout <<"Query: ";
    for (int r=0; r<query_cca_hy.rows; r++)
        for (int c=0; c<query_cca_hy.cols; c++)
            cout <<query_cca_hy.at<float>(r,c)<<", ";
    cout<<endl;*/
    //checkNaN(query_cca_hy);
    
    
    //Mat s;//scores
    int numberOfInstances=-1;
    if (corpus_imgfiles!=NULL)
        numberOfInstances=corpus_imgfiles->size();
    else if (corpus_dataset!=NULL)
        numberOfInstances=corpus_dataset->size();
        
    vector<float> scores(numberOfInstances);
    
    batches_cca_att();
    //cout <<"numBatches "<<numBatches<<endl;
    
    #pragma omp parallel for
    for (int i=0; i<numBatches; i++)
    {
        //checkNaN(batches_cca_att()[i]);
        Mat s_batch = batches_cca_att()[i].t()*query_cca_hy;
        //??Mat s_batch = query_cca_hy*batches_cca_att()[i].t();
        //cout << "batch "<<i<<": ";
        //for (int r=0; r<s_batch.rows; r++)
        //    assert(s_batch.at<float>(r,0)>0);
        //    cout << s_batch.at<float>(r,0)<<", ";
        //cout <<endl;
        //s.push_back(s_batch);
        assert(s_batch.cols==1);
        //copy(s_batch.data,s_batch.data+s_batch.rows,scores.begin()+batches_index[i]);
        for (int r=0; r<s_batch.rows; r++)
            scores[r+batches_index[i]] = s_batch.at<float>(r,0);
    }
    //return s.toVector();
    return scores;
}

Mat EmbAttSpotter::extract_feats(const Mat& im)
{
    
    Mat feats_m=phow(im,&PCA_());
    if (test_mode)
    {
        compareToCSV(attModels().W,"test/attModels_W_test.csv",false,0.0005);
        compareToCSV(attReprTr(),"test/attReprTr_test.csv",false,0.0005);
    }
        /*vector<vector<float> > attModels_Wt;
    readCSV("test/attModels_W_test.csv",attModels_Wt);
    assert(attModels().W.rows==attModels_Wt.size() && attModels().W.cols==attModels_Wt[0].size());
    for (int r=0; r<attModels().W.rows; r++)
        for (int c=0; c<attModels().W.cols; c++)
            assert(abs(attModels().W.at<float>(r,c)-attModels_Wt[r][c])<0.0001);*/
   

    Mat feats_FV = getImageDescriptorFV(feats_m);
    
    /*cout<<"image feats: ";
    for (int c=0; c<min(feats_FV.cols,30); c++)
        cout<<feats_FV.at<float>(0,c)<<", ";
    cout<<endl;*/
    
    return feats_FV;
}

/*vector<Mat>* EmbAttSpotter::extract_FV_feats_fast_corpus(const vector<string>& imageLocations)
{
    
    //vector<Mat>* ret = new vector<Mat>(numBatches);
    vector<Mat>* ret = new vector<Mat>(numBatches);
    #pragma parallel for
    for (int i=0; i<numBatches; i++)
    {
        //ret->at(i) = Mat_<float>(batches_indexEnd[i]-batches_index[i],FV_DIM);
        Mat tmp = Mat_<float>(batches_indexEnd[i]-batches_index[i],FV_DIM);
        for (int j=batches_index[i]; j<batches_indexEnd[i]; j++)
        {
            Mat im = imread(imageLocations[j];
            Mat feats=phow(im,*PCA_());
            //ret->at(i).row(j-batches_index[i]) = getImageDescriptorFV(feats.t());
            tmp.row(j-batches_index[i]) = getImageDescriptorFV(feats.t());
        }
        //#pragma omp critical
        {
            //Mat aux = ret->rowRange(batches_index[i],batches_indexEnd[i]);
            //tmp.copyTo(aux);
            ret->at(i)=tmp;
        }
    }
    return ret;
}*/

vector<Mat>* EmbAttSpotter::extract_FV_feats_fast_and_batch(const vector<string>& imageLocations,vector<int>* batches_index,vector<int>* batches_indexEnd, int batchSize)
{
    if (test_mode)
        cout<<"extract_FV_feats_fast_and_batch1()"<<endl;
    bool del=false;
    if (batches_index==NULL & batches_indexEnd==NULL)
    {
        batches_index=new vector<int>();
        batches_indexEnd=new vector<int>();
        del=true;
    }
    
    
    int numBatches = imageLocations.size()/batchSize;
    if (imageLocations.size()%batchSize > batchSize/3)
        numBatches+=1;
    vector<Mat>* ret = new vector<Mat>(numBatches);
    
    //int batchSize = imageLocations.size()/numBatches;
    batches_index->clear();
    batches_indexEnd->clear();
    
    batches_index->push_back(0);
    for (int i=1; i<numBatches; i++)
    {
        batches_indexEnd->push_back(i*batchSize);
        batches_index->push_back(i*batchSize);
    }
    batches_indexEnd->push_back(imageLocations.size());
    
    
    for (int i=0; i<numBatches; i++)
    {
        //ret->at(i) = Mat_<float>(batches_indexEnd[i]-batches_index[i],FV_DIM);
        Mat tmp = Mat_<float>(batches_indexEnd->at(i)-batches_index->at(i),FV_DIM);
        int start=batches_index->at(i);
        int end=batches_indexEnd->at(i);
        #pragma omp parallel for
        for (int j=start; j<end; j++)
        {
            Mat im = imread(imageLocations[j],CV_LOAD_IMAGE_GRAYSCALE);
            Mat feats=phow(im,&PCA_());
            //ret->at(i).row(j-batches_index[i]) = getImageDescriptorFV(feats.t());
            Mat r = getImageDescriptorFV(feats);
            assert(r.cols==FV_DIM);
            #pragma omp critical (copyToBatch)//do we need this?
            r.copyTo(tmp.row(j-start));
        }
        
        for (int r=0; r<tmp.rows; r++)
            for (int c=0; c<tmp.cols; c++)
                assert(tmp.at<float>(r,c)==tmp.at<float>(r,c));
        
        ret->at(i)=tmp;
        
    }
    
    if (del)
    {
        delete batches_index;
        delete batches_indexEnd;
    }
    return ret;
}

vector<Mat>* EmbAttSpotter::extract_FV_feats_fast_and_batch(const Dataset* dataset,vector<int>* batches_index,vector<int>* batches_indexEnd, int batchSize)
{
    if (test_mode)
        cout<<"extract_FV_feats_fast_and_batch2()"<<endl;
    bool del=false;
    if (batches_index==NULL & batches_indexEnd==NULL)
    {
        batches_index=new vector<int>();
        batches_indexEnd=new vector<int>();
        del=true;
    }
    
    
    int numBatches = dataset->size()/batchSize;
    if (dataset->size()%batchSize > batchSize/3)
        numBatches+=1;
    if (numBatches==0)
        numBatches=1;
    //cout<<"in extract_FV_feats_fast_and_batch, I'm going to make "<<numBatches<<" batches, as the dataset is size "<<dataset->size()<<endl;
    vector<Mat>* ret = new vector<Mat>(numBatches);
    
    //int batchSize = imageLocations.size()/numBatches;
    batches_index->clear();
    batches_indexEnd->clear();
    
    batches_index->push_back(0);
    for (int i=1; i<numBatches; i++)
    {
        batches_indexEnd->push_back(i*batchSize);
        batches_index->push_back(i*batchSize);
    }
    batches_indexEnd->push_back(dataset->size());
    
    
    for (int i=0; i<numBatches; i++)
    {
        //ret->at(i) = Mat_<float>(batches_indexEnd[i]-batches_index[i],FV_DIM);
        Mat tmp = Mat_<float>(batches_indexEnd->at(i)-batches_index->at(i),FV_DIM);
        int start=batches_index->at(i);
        int end=batches_indexEnd->at(i);
        #pragma omp parallel for
        for (int j=start; j<end; j++)
        {
            Mat im = dataset->image(j);
            Mat feats=phow(im,&PCA_());
            //ret->at(i).row(j-batches_index[i]) = getImageDescriptorFV(feats.t());
            Mat r = getImageDescriptorFV(feats);
            assert(r.cols==FV_DIM);
            #pragma omp critical (copyToBatch)//do we need this?
            r.copyTo(tmp.row(j-start));
        }
        
        for (int r=0; r<tmp.rows; r++)
            for (int c=0; c<tmp.cols; c++)
                assert(tmp.at<float>(r,c)==tmp.at<float>(r,c));
        
        ret->at(i)=tmp;
        
    }
    
    if (del)
    {
        delete batches_index;
        delete batches_indexEnd;
    }
    return ret;
}

Mat EmbAttSpotter::get_FV_feats(const Dataset* dataset)
{
    if (test_mode)
        cout<<"get_FV_feats()"<<endl;
    int size;
    if (test_mode==1)
        size=456;
    else
        size=dataset->size();
    
    Mat ret = Mat_<float>(size,FV_DIM);
    #pragma omp parallel for
    for (int j=0; j<size; j++)
    {
        const Mat im = dataset->image(j);
        Mat feats=phow(im,&PCA_());
        //ret->at(i).row(j-batches_index[i]) = getImageDescriptorFV(feats.t());
        Mat r = getImageDescriptorFV(feats);
        assert(r.cols==FV_DIM);
        #pragma omp critical (copyToBatch)//do we need this?
        r.copyTo(ret.row(j));
    }
    
    for (int r=0; r<ret.rows; r++)
        for (int c=0; c<ret.cols; c++)
            assert(ret.at<float>(r,c)==ret.at<float>(r,c));
    
    return ret;
}

const vector<Mat>& EmbAttSpotter::features_corpus(bool retrain)
{
    string name = saveName+"_features_corpus.dat";
    if (_features_corpus==NULL)
    {
        ifstream in(name);
        if (in && !retrain)
        {
            //load
            int num;
            in >> num;
            if (numBatches==-1)
            {
                numBatches=num;
            }
            else
                assert(num==numBatches);
            _features_corpus = new vector<Mat>(numBatches);
            int trackCorpusSize=0;
            batches_index.clear();
            batches_indexEnd.clear();
            
            for (int i=0; i<numBatches; i++) {
                batches_index.push_back(trackCorpusSize);
                _features_corpus->at(i)=readFloatMat(in);
                trackCorpusSize += _features_corpus->at(i).rows;
                batches_indexEnd.push_back(trackCorpusSize);
            }
            in.close();
            
            if (corpusSize==-1)
                corpusSize=trackCorpusSize;
            else
                assert(corpusSize==trackCorpusSize);
        }
        else
        {
            in.close();
            if(corpus_imgfiles!=NULL)
                _features_corpus = extract_FV_feats_fast_and_batch(*corpus_imgfiles,&batches_index,&batches_indexEnd,genericBatchSize);
            else if (corpus_dataset!=NULL)
                _features_corpus = extract_FV_feats_fast_and_batch(corpus_dataset,&batches_index,&batches_indexEnd,genericBatchSize);
            else
                cout<<"ERROR: No corpus specified"<<endl;
            numBatches = _features_corpus->size();
            //save
            ofstream out(name);
            out << numBatches << " ";
            for (int i=0; i<numBatches; i++)
            {      
                writeFloatMat(out,_features_corpus->at(i));
            }
            out.close();
        }
        
    }
    return *_features_corpus;
}

/*void initBatches()
{
    int batchSize = corpusSize/numBatches;
    batches_index.clear();
    batches_indexEnd.clear();
    
    batches_index.push_back(0);
    for (int i=1; i<numBatches; i++)
    {
        batches_indexEnd.push_back(i*batchSize);
        batches_index.push_back(i*batchSize);
    }
    batches_indexEnd.push_back(corpusSize);
}*/


const Mat& EmbAttSpotter::feats_training(bool retrain)
{
    string name = saveName+"_feats_training.dat";
    if (_feats_training.rows==0)
    {
        #pragma omp  critical (feats_training)
        {
            if (_feats_training.rows==0)
            {
                ifstream in(name);
                if (in && !retrain)
                {
                    //load
                    int num;
                    in >> num;
                    assert(num==1);
                    _feats_training=readFloatMat(in);
                    in.close();
                }
                else
                {
                    in.close();
                    if (training_imgfiles!=NULL)
                    {
                        vector<Mat>* oo = extract_FV_feats_fast_and_batch(*training_imgfiles,NULL,NULL,training_imgfiles->size());
                        _feats_training = oo->at(0);
                        delete oo;
                    }
                    else
                    {
                        _feats_training=get_FV_feats(training_dataset);
                    }
                    
                    //save
                    ofstream out(name);
                    out << 1 << " ";
                    writeFloatMat(out,_feats_training);
                    
                    out.close();
                }
            }
        }
    }
    return _feats_training;
}

/*const Mat& feats_validation()
{
    string name = saveName+"_feats_validation.dat";
    if (_feats_validation==NULL)
    {
        ifstream in(name)
        if (in && !retrain)
        {
            //load
            int num;
            in >> num;
            assert(num==1);
            _feats_validation = new vector<Mat>(1);
            _feats_validation->at(0)=readFloatMat(in);
            in.close();
        }
        else
        {
            in.close();
            assert(validation_imgfiles!=NULL);
            _feats_validation = extract_FV_feats_fast_and_batch(*validation_imgfiles,NULL,NULL,validation_imgfiles->size());
            assert(_feats_validation->size()==1);
            //save
            ofstream out(name);
            out << 1;
            writeFloatMat(out,_feats_validation->at(0));
            
            out.close();
        }
        
    }
    return _feats_validation->at(0);
}*/

void showImage(float * im, int h, int w)
{
    Mat m(h,w,CV_32F);
    for (int r=0; r<h; r++)
        for (int c=0; c<w; c++) 
        {
            m.at<float>(r,c)=256*im[(r)*w+(c)];
            assert(im[(r)*w+(c)]>=0 && im[(r)*w+(c)]<=1);
        }
    m.convertTo(m,CV_8U);
    imshow("image",m);
    waitKey(1);
}

Mat createColorMat(float * im, int h, int w)
{
    Mat m(h,w,CV_32F);
    for (int r=0; r<h; r++)
        for (int c=0; c<w; c++) 
        {
            m.at<float>(r,c)=256*im[(r)*w+(c)];
            assert(im[(r)*w+(c)]>=0 && im[(r)*w+(c)]<=1);
        }
    m.convertTo(m,CV_8U);
    cvtColor(m,m,CV_GRAY2RGB);
    return m;
}


Mat EmbAttSpotter::phow(const Mat& im, const struct PCA_struct* PCA_pt)
{
    int bb_x1, bb_x2, bb_y1, bb_y2;
    DoBB(im,&bb_x1,&bb_x2,&bb_y1,&bb_y2);
    int bb_w=bb_x2-bb_x1 + 1;
    int bb_h=bb_y2-bb_y1 + 1;
    int cx = bb_x1+bb_w/2;
    int cy = bb_y1+bb_h/2;

    Mat feats_m;
    //for all sizes, extract SIFT features
    int maxSize=-1;
    for (int size : SIFT_sizes)
        if (size>maxSize)
            maxSize=size;
    
    //convert im to vl style
    
    #if USE_VL
    /**VL*/
    //imshow("image Origin",im);
    //waitKey(1);
    Mat imf;
    im.convertTo(imf,CV_32F);
    
    imf/=255; //??
    
    //cout <<"canary "<<imf.at<float>(0,0)<<endl;
    
    assert(im.isContinuous());
    assert(im.rows>0 && im.cols>0);
    //imf=imf.t(); 
    //assert(imf.isContinuous());
    
    /*vector<float> im_v;
    for (int i = 0; i < imf.rows; ++i)
      for (int j = 0; j < imf.cols; ++j)
        im_v.push_back(imf.at<float>(i, j));*/
    float* im_vl = (float*)imf.data;
    /*showImage(im_vl,imf.rows,imf.cols);*/
    /*VL**/
    #endif
    for (int size : SIFT_sizes)
    {
        
        double sigma = size/magnif;  //sqrt(pow(size/magnif,2) - .25);
        int gSize = 5*sigma;
        if (gSize%2==0)
            gSize++;
        
        #if USE_VL
        /**VL*/
        int off = floor(1 + (3.0/2.0) * (maxSize - size)); //MATALB, for DSIFT
        float* ims = new float[imf.rows*imf.cols];
        
        
        
        int newW, newH;
        
        vl_matlab_smooth_f(im_vl,imf.cols,imf.rows,sigma,ims,&newH,&newW);
        //cout << "new size ["<<newH<<", "<<newW<<"]"<<endl;
        
        //Mat imsMat; 
        //GaussianBlur( imf, imsMat, Size( gSize, gSize ), sigma, sigma );
        //assert(imsMat.isContinuous());
        //ims = (float*)imsMat.data;/**/
        //
        #if DRAW
        showImage(ims,imf.rows,imf.cols);
        #endif
        
        
        VlDsiftFilter* dsift;
        dsift = vl_dsift_new (imf.cols,imf.rows) ;
        
        VlDsiftDescriptorGeometry geom ;
        geom.numBinX = 4 ;
        geom.numBinY = 4 ;
        geom.numBinT = 8 ;
        geom.binSizeX = size ;
        geom.binSizeY = size ;
        vl_dsift_set_geometry(dsift, &geom) ;
        
        int step [2] = {stride,stride} ;
        vl_dsift_set_steps(dsift, step[0], step[1]) ;
        vl_dsift_set_bounds (dsift,off,off,imf.cols,imf.rows );
        vl_dsift_set_flat_window(dsift, 1) ;
        vl_dsift_set_window_size(dsift, 1.5) ;
        
        //cout <<"Data into dsift: "<<ims[0]<<" "<<ims[1]<<" "<<ims[2]<<" "<<ims[3]<<" "<<ims[4]<<endl;
        vl_dsift_process(dsift,ims);
        
        int num = vl_dsift_get_keypoint_num(dsift);
        //cout << "Total :"<<num<<endl;
        
        VlDsiftKeypoint const * kps = vl_dsift_get_keypoints(dsift);
        float const * descArr = vl_dsift_get_descriptors(dsift);
        Mat desc(num,SIFT_DIM,CV_32F);
        //vector<KeyPoint> keyPoints;
        for (int i=0; i<num; i++)
        {
            for (int f=0; f<SIFT_DIM; f++)
                desc.at<float>(i,f)=descArr[i*SIFT_DIM+f];
            
            //if (i==0 || i==num-1)
                //cout<<kps[i].y<<", "<<kps[i].x<<" : "<<size<<" = "<<kps[i].norm<<endl;
        }
        /*VL**/
        #else
        /**CV**/
        int off = floor((3.0/2.0) * maxSize);
        Mat ims; 
        GaussianBlur( im, ims, Size( gSize, gSize ), sigma, sigma );
        ims.convertTo(ims,CV_8U);
        
        
        
        //describe dense points
        vector<KeyPoint> keyPoints;
        for (int x=off; x<ims.cols-off; x+=stride)
            for (int y=off; y<ims.rows-off; y+=stride)
            {
                keyPoints.push_back(KeyPoint(x,y,size));
                //cout<<x<<", "<<y<<" : "<<size<<endl;
                
            }
        
        int nfeaturePoints=keyPoints.size();
        int nOctivesPerLayer=3;
        SIFT detector(0,nOctivesPerLayer,contrastthreshold);
        Mat desc;
        detector(ims,noArray(),keyPoints,desc,true);
        desc.convertTo(desc, CV_32FC1);
        
        //cout << "there are "<<keyPoints.size()<<" key points. we got "<<desc.rows<<" descriptors"<<endl;
        
        /*CV**/
        #endif
        
        
        vector<int> toKeep;
        Mat summed;
        
        reduce(desc,summed,1,CV_REDUCE_SUM);
        #if USE_VL
        /**VL*/
        for (int r=0; r<num; r++)
        {
            //cout << kps[r].x<<", "<<kps[r].y<<" = "<<kps[r].norm<<endl;
            if (summed.at<float>(r,0)>0 && kps[r].norm>=contrastthreshold)
                toKeep.push_back(r);
        }
        /*VL**/
        //cout <<"Start "<<num<< ", Removed "<<num-toKeep.size()<<endl;
        if (toKeep.size()<num) {
        #else
        /**CV*/
        for (int r=0; r<keyPoints.size(); r++)
        {
            if (summed.at<float>(r,0)>0)
            {
                //except the mass is calculated before the normalizations....
                //Do I not need to normaliz this sum?
                //int frameSizeX = self->geom.binSizeX * (self->geom.numBinX - 1) + 1 ;
                //int frameSizeY = self->geom.binSizeY * (self->geom.numBinY - 1) + 1 ;
                //float normConstant = frameSizeX * frameSizeY ;
                float mass = 0 ;
                for (int bint = 0 ; bint < desc.cols ; ++ bint)
                    mass += desc.at<float>(r,bint) ;
                //mass /= normConstant ;
                float norm = mass ;
                //cout << keyPoints[r].pt.x<<", "<<keyPoints[r].pt.y<<" = "<<norm<< "\t"<<keyPoints[r].response<<endl;
                if (norm>=contrastthreshold)
                    toKeep.push_back(r);
            }
        }
        /*CV**/
        //cout <<"Start "<<keyPoints.size()<< ", Removed "<<keyPoints.size()-toKeep.size()<<endl;
        if (toKeep.size()<keyPoints.size()) {
        #endif
            desc = select_rows(desc,toKeep);
        }
        
        desc /= 255.0;
        
        assert(desc.cols==SIFT_DIM);
        
        //normalize, subtract mean 
        for (unsigned int i=0; i<desc.rows; i++)
        {
            double sum=0;
            for (unsigned int j=0; j<desc.cols; j++)
            {
                sum += desc.at<float>(i,j);
                //sum += pow(desc.at<float>(i,j),4); //TODO, why isn't this used?
            }
            assert (sum!=0);
            double X = pow(sum,-0.25);
            for (unsigned int j=0; j<desc.cols; j++)
            {
                desc.at<float>(i,j) = desc.at<float>(i,j)*X;
            }
            if (PCA_pt!=NULL)
                desc.row(i) = desc.row(i) - PCA_pt->mean.t();
        }
        //trans with eigen vectors (desc is tranposed in relation to ALmazan's code, flip back at end)
        if (PCA_pt!=NULL)
            desc = (PCA_pt->eigvec.t()*desc.t()).t();
        #if DRAW
        #if USE_VL
        /**VL*/
        Mat draw = createColorMat(ims,imf.rows,imf.cols);
        //draw=draw.t();
        //delete ims;
        /*VL**/
        #else
        /**CV*/
        Mat draw;
        cvtColor(ims,draw,CV_GRAY2RGB);
        /*CV**/
        #endif
        
        #if USE_VL
        for (unsigned int i=0; i<num; i++)
        {
            
            int kpx=kps[i].x;
            int kpy=kps[i].y;
            assert(kpx<im.cols);
            assert(kpy<im.rows);
            
            
            
            
            
            size=1;
            Vec3b c(0,0,255);
            for (int xx=kpx-size; xx<kpx+size; xx++)
                for (int yy=kpy-size; yy<kpy+size; yy++)
                    if (xx>=0 && xx<draw.cols && yy>=0 && yy<draw.rows)
                    {
                        draw.at<Vec3b>(yy,xx)=c;
                    }
        }
        #endif
        #endif
        
        //append x,y information
        Mat augmented(desc.rows, desc.cols+2, desc.type());
        desc.copyTo(augmented(Rect(0, 0, desc.cols, desc.rows)));
        for (unsigned int j=0; j<desc.rows; j++)
        {
            #if USE_VL
            /**VL*/
            
            int kpx=kps[toKeep.at(j)].x;
            int kpy=kps[toKeep.at(j)].y;
            assert(kpx<im.cols);
            assert(kpy<im.rows);
            /*VL**/
            #else
            /**CV*/
            int kpx=keyPoints.at(toKeep.at(j)).pt.x;
            int kpy=keyPoints.at(toKeep.at(j)).pt.y;
            /*CV**/
            #endif
            
            
            augmented.at<float>(j,desc.cols) = (kpx-cx)/(float)bb_w;
            augmented.at<float>(j,desc.cols+1) = (kpy-cy)/(float)bb_h;
            //TODO augmented.at<float>(j,desc.cols+2) = (size);
            
            #if DRAW
            size=1;
            Vec3b c(100+rand()%156,100+rand()%156,100+rand()%156);
            for (int xx=kpx-size; xx<kpx+size; xx++)
                for (int yy=kpy-size; yy<kpy+size; yy++)
                    if (xx>=0 && xx<draw.cols && yy>=0 && yy<draw.rows)
                    {
                        draw.at<Vec3b>(yy,xx)=c;
                    }
            #endif
        }
        #if DRAW
        imshow("SIFT",draw);
        waitKey();
        #endif
        
        #if USE_VL
        vl_dsift_delete(dsift);
        #endif
        
        //vconcat(feats,augmented,feats);
        feats_m.push_back(augmented);
        
        if (PCA_pt!=NULL)
        {
            if (feats_m.cols!=AUG_PCA_DIM)
            {
                cout << "feats_m.cols: "<<feats_m.cols<<endl;
                cout << "AUG_PCA_DIM: "<<AUG_PCA_DIM<<endl;
            }
            assert(feats_m.cols==AUG_PCA_DIM);
        }
    }
    //for (int r=0; r<feats_m.rows; r++)
    //    for (int c=0; c<feats_m.cols; c++)
    //        assert(feats_m.at<float>(r,c)==feats_m.at<float>(r,c));
    return feats_m;
}

void EmbAttSpotter::DoBB(const Mat& im, int* bb_x1, int* bb_x2, int* bb_y1, int* bb_y2)
{
    float px = 0.975; //% Goal is to center. Be conservative
    float py = 0.8; //% Goal is to remove ascenders/descenders. Free for all.
    
    //does this mean on (1) for stroke pixels?
    //imbw = 1-im2bw(im,graythresh(im));
    Mat imbw= otsuBinarization(im);
    /*Mat dst = imbw.clone();
    for (int r=0; r<dst.rows; r++)
        for (int c=0; c<dst.cols; c++)
        {
            if (dst.at<unsigned char>(r,c)==1)
                dst.at<unsigned char>(r,c)=0;
            else
                dst.at<unsigned char>(r,c)=255;
        }
    imshow("test",dst);
    waitKey();*/
    
    Mat horzProf;
    reduce(imbw,horzProf,0,CV_REDUCE_SUM,CV_32F);
    Mat vertProf;
    reduce(imbw,vertProf,1,CV_REDUCE_SUM,CV_32F);
    Mat total;
    reduce(horzProf,total,1,CV_REDUCE_SUM,CV_32F);
    //sh = (sum(imbw)/sum(sum(imbw)));
    //sv = (sum(imbw,2)/sum(sum(imbw)));
    horzProf = horzProf/total.at<float>(0,0);
    //divide(horzProf,total,horzProf);
    vertProf = vertProf/total.at<float>(0,0);
    //divide(vertProf,total,vertProf);
    //shc = [0 cumsum(sh)];
    //svc = [0 cumsum(sv)'];
    vector<float> shc(horzProf.cols+1);
    for (int c=0; c<horzProf.cols; c++)
    {
        shc[c+1]=horzProf.at<float>(0,c)+shc[c];
    }
    vector<float> svc(vertProf.rows+1);
    for (int r=0; r<vertProf.rows; r++)
    {
        svc[r+1]=vertProf.at<float>(r,0)+svc[r];
    }
    assert(abs(shc[horzProf.cols]-1)<.0001);
    assert(abs(svc[vertProf.rows]-1)<.0001);
    //dh = bsxfun(@minus,shc,shc') > px;
    //dv = bsxfun(@minus,svc,svc') > py;
    //[p1h,p2h] = find(dh);
    
    //vector<int> p1h, p2h, pdh;
    int p1h, p2h;
    int min_pdv=1000;
    for (int i=0; i< shc.size(); i++)
    {
        for (int j=0; j< shc.size(); j++)
        {
            if (shc[i]-shc[j]>px)
            {
                //p1h.push_back(j);
                //p2h.push_back(i);
                //pdh.push_back(abs(i-j+1));
                int pdv = abs(i-j+1);
                if (pdv<min_pdv)
                {
                    min_pdv=pdv;
                    p1h=j;
                    p2h=i;
                }
            }
        }
    }
    //vector<int> p1v, p2v;//, pdv;
    int p1v, p2v;
    min_pdv=1000;
    //int idx_min_pdv=-1;
    for (int i=0; i< svc.size(); i++)
    {
        for (int j=0; j< svc.size(); j++)
        {
            if (svc[i]-svc[j]>py)
            {
                //p1v.push_back(j);
                //p2v.push_back(i);
                int pdv = abs(i-j+1);
                if (pdv<min_pdv)
                {
                    min_pdv=pdv;
                    p1v=j;
                    p2v=i;
                }
            }
        }
    }
   
    //why the random perm?
    //a= randperm(length(p1h)); p1h = p1h(a);p2h = p2h(a);
    //[p1v,p2v] = find(dv);
    //a= randperm(length(p1v)); p1v = p1v(a);p2v = p2v(a);

    //[vh,idxh] = sort(abs(p2h-p1h+1));
    //[vv,idxv] = sort(abs(p2v-p1v+1));

    //ph=[p1h(idxh(1)),p2h(idxh(1))];
    //pv=[p1v(idxv(1)),p2v(idxv(1))];

    //bbox = [min(ph),max(ph),min(pv),max(pv)];
    *bb_x1 = min(p2h, p1h);
    *bb_x2 = max(p2h, p1h);
    *bb_y1 = min(p2v, p1v);
    *bb_y2 = max(p2v, p1v);
}

Mat EmbAttSpotter::getImageDescriptorFV(const Mat& feats_m)
{
    assert(feats_m.cols==AUG_PCA_DIM);
    int dimension = AUG_PCA_DIM;
    int numClusters = numGMMClusters*numSpatialX*numSpatialY;
    //Mat flipped_feats = feats_m.t();//we have our featvecs on the row and OpenCV is row major
    assert(feats_m.isContinuous());
    float* dataToEncode = (float*)feats_m.data;
    int numDataToEncode = feats_m.rows;
    //float* enc = vl_malloc(sizeof(float) * 2 * dimension * numClusters);
    Mat ret(1,FV_DIM,CV_32F);
    assert(ret.isContinuous());
    float* enc = (float*)ret.data;
    //float f2 = enc[rowIdx*ret.step1() + colIdx];
    // run fisher encoding
    
    vl_fisher_encode
        (enc, VL_TYPE_FLOAT,
         GMM().means, dimension, numClusters,
         GMM().covariances,
         GMM().priors,
         dataToEncode, numDataToEncode,
         VL_FISHER_FLAG_IMPROVED
         ) ;
         
     //assert(ret.cols
     for (int r=0; r<ret.rows; r++)
        for (int c=0; c<ret.cols; c++)
            assert(ret.at<float>(r,c)==ret.at<float>(r,c));
     return ret;
}

const vector<Mat>& EmbAttSpotter::batches_cca_att()
{
    string name = saveName+"_batches_cca_att.dat";
    if (_batches_cca_att == NULL)
    {
        //cout<<"batches_cca_att is NULL"<<endl;
        #pragma omp critical (batches_cca_att)
        {
            if (_batches_cca_att == NULL)
            {
                //init
                
                
                //load
                ifstream in(name);
                if (in)
                {
                    int numBatchesRead;
                    in >> numBatchesRead;
                    if (numBatches==-1)
                        numBatches=numBatchesRead;
                    if (numBatchesRead==numBatches)
                    {
                        _batches_cca_att = new vector<Mat>(numBatches);
                        batches_index.clear();
                        batches_indexEnd.clear();
                        int trackCorpusSize=0;
                        for (int i=0; i<numBatches; i++)
                        {
                            batches_index.push_back(trackCorpusSize);
                            _batches_cca_att->at(i) = readFloatMat(in);
                            trackCorpusSize += _batches_cca_att->at(i).rows;
                            batches_indexEnd.push_back(trackCorpusSize);
                        }
                    }
                    else if (_batches_cca_att!=NULL)
                    {
                        delete _batches_cca_att;
                        _batches_cca_att=NULL;
                    }
                    in.close();
                    
                    
                }
            }
        }
    }
    if (_batches_cca_att==NULL)
    {
        #pragma omp critical (batches_cca_att)
        {
            if (_batches_cca_att==NULL)
            {
                //cout<<"doing batches_cca_att, numBatches="<<numBatches<<endl;
                
                features_corpus();
                //cout<<"after features_corpus, numBatches="<<numBatches<<endl;
                _batches_cca_att = new vector<Mat>(numBatches);
                //const Embedding& embedding = get_embedding();
                Mat matx = embedding().rndmatx;//(Rect(0,0,embedding().rndmatx.cols,embedding().M));
                assert(matx.rows==embedding().M);
                for (int i=0; i<numBatches; i++)
                {
                    // load batch_att
                    Mat tmp = matx*batch_att(i);
                    vconcat(cosMat(tmp),sinMat(tmp),tmp);
                    Mat batch_emb_att = (1/sqrt(embedding().M)) * tmp;
                    for (int c=0; c<batch_emb_att.cols; c++)
                        batch_emb_att.col(c) -= embedding().matt;
                    Mat batch_cca_att = embedding().Wx.t()*batch_emb_att;
                    _batches_cca_att->at(i)=batch_cca_att;
                }
                
                //save
                ofstream out(name);
                out << numBatches << " ";
                for (int i=0; i<numBatches; i++)
                    writeFloatMat(out,_batches_cca_att->at(i));
                out.close();
            }
        }
    }
    return *_batches_cca_att;
}

Mat EmbAttSpotter::batch_att(int batchNum)
{
    Mat m = features_corpus()[batchNum];
    return attModels().W.t()*(m.t());
}

void EmbAttSpotter::learn_attributes_bagging()
{
    if (test_mode)
        cout<<"learn_attributes_baggings()"<<endl;
    int dimFeats=feats_training().cols;
    int numAtt = phocsTr().rows;
    if (test_mode)//==1)
        numAtt=200;
    
    int numSamples = phocsTr().cols;
    _attModels = new AttributesModels;
    _attModels->W=Mat::zeros(dimFeats,numAtt,CV_32F);
    
    //_attModels->B=zeros(1,numAtt,CV_32F);
    _attModels->numPosSamples=Mat::zeros(1,numAtt,CV_32F);
    _attReprTr = Mat::zeros(numAtt,numSamples,CV_32F); //attFeatsTr, attFeatsBag
    
    Mat threshed;
    threshold(phocsTr(),threshed, 0.47999, 1, THRESH_BINARY);
    assert(threshed.type()==CV_32F);
    
    #pragma omp parallel for  num_threads(4)//I'm aumming I can read-only from Mats without worrying about thread stuff. If wrong, use data ptr
    for (int idxAtt=0; idxAtt<numAtt; idxAtt++)
    {
        //learn_att(...)
        
        deque<int> idxPos;
        deque<int> idxNeg;
        for (int r=0; r<threshed.cols; r++)
        {
            if (threshed.at<float>(idxAtt,r)==1)
                idxPos.push_back(r);
            else
                idxNeg.push_back(r);
        }
        if (idxPos.size()<2 || idxNeg.size()<2)
        {
            cout << "No model for attribute "<<idxAtt<<". Not enough data."<<endl;
            //continue
        }
        else
        {
            //...
            Mat Np = Mat::zeros(1,numSamples,CV_32F);
            int N=0;
            int numPosSamples=0;
            
            int numPass=2;
            int numIters=5;
            if (test_mode==1)
            {
                numPass=1;
                numIters=1;
            }
            for (int cPass=0; cPass<numPass; cPass++)
            {
                if (test_mode==0)
                {
                    random_shuffle ( idxPos.begin(), idxPos.end() );
                    random_shuffle ( idxNeg.begin(), idxNeg.end() );
                }
                int nTrainPos = .8*idxPos.size();
                int nValPos = idxPos.size()-nTrainPos;
                int nTrainNeg = .8*idxNeg.size();
                int nValNeg = idxNeg.size()-nTrainNeg;
                
                for (int it=0; it<numIters; it++)
                {
                    vector<int> idxTrain;
                    idxTrain.insert(idxTrain.end(), idxPos.begin(), idxPos.begin()+nTrainPos);
                    idxTrain.insert(idxTrain.end(), idxNeg.begin(), idxNeg.begin()+nTrainNeg);
                    vector<int> idxVal;
                    idxVal.insert(idxVal.end(), idxPos.begin()+nTrainPos,idxPos.end());
                    idxVal.insert(idxVal.end(), idxNeg.begin()+nTrainNeg,idxNeg.end());
                    assert(idxTrain.size()==nTrainPos+nTrainNeg);
                    assert(idxVal.size()==nValPos+nValNeg);
                    
                    Mat featsTrain = select_rows(feats_training(),idxTrain);
                    //Mat phocsTrain = select_rows(phocs_training(),idxTrain);
                    Mat featsVal = select_rows(feats_training(),idxVal);
                    //Mat phocsVal = select_rows(phocs_training(),idxVal);
                    
                    numPosSamples = numPosSamples + nTrainPos;
                    
                    double labelsTrain[featsTrain.rows];//binary vector, 1 where idxAtt is not zero, -1 where it is zero
                    for (int r=0; r<featsTrain.rows; r++)
                    {
                        labelsTrain[r] = phocsTr().at<float>(idxAtt,idxTrain[r])!=0?1:-1;
                    }
                    float labelsVal[featsVal.rows];//binary vector, 1 where idxAtt is not zero, -1 where it is zero
                    for (int r=0; r<featsVal.rows; r++)
                    {
                        labelsVal[r] = phocsTr().at<float>(idxAtt,idxVal[r])!=0?1:-1;
                    }
                    
                    VlSvm * svm=NULL;
                    Mat modelAtt = cvSVM(featsTrain,labelsTrain,featsVal,labelsVal,svm);
                    
                    
                    
                    N++;
                    for (int idx : idxVal)
                        Np.at<float>(0,idx)+=1;
                    
                    #pragma omp critical (learn_attributes_bagging_inside)
                    {
                        _attModels->W.col(idxAtt) += modelAtt;
                        for (int r=0; r<featsVal.rows; r++)
                        {
                            float s=0;
                            for (int c=0; c<featsVal.cols; c++)
                                s += featsVal.at<float>(r,c)*vl_svm_get_model(svm)[c];
                            _attReprTr.at<float>(idxAtt,r)+=s;
                        }
                    }
                    delete svm;
                    
                    
                    for (int circshift=0; circshift<nValPos; circshift++)
                    {
                        idxPos.push_back(idxPos.front());
                        idxPos.pop_front();
                    }
                    
                    for (int circshift=0; circshift<nValNeg; circshift++)
                    {
                        idxNeg.push_back(idxNeg.front());
                        idxNeg.pop_front();
                    }
                    
                    
                    /*int iiic=0;
                    for (int c=0; c<numSamples; c++)
                        if (Np.at<float>(0,c)==0)
                            iiic++;
                        //assert(Np.at<float>(0,c)!=0);
                    cout << "there are "<<iiic<<" zeros in Np. "<<cPass<<" "<<it<<endl;*/
                }
            }
            
            if (N!=0)
            {
                //for (int c=0; c<numSamples; c++)
                    //assert(Np.at<float>(0,c)!=0);
                #pragma omp critical (learn_attributes_bagging_inside)
                {
                    _attModels->W.col(idxAtt) /= (float)N;
                    divide(_attReprTr(Rect(0,idxAtt,numSamples,1)),Np,_attReprTr(Rect(0,idxAtt,numSamples,1)));
                    _attModels->numPosSamples.at<float>(0,idxAtt) = ceil(numPosSamples/(double)N);
                }
            }
        }
    }
}

Mat EmbAttSpotter::cvSVM(const Mat& featsTrain, const double* labelsTrain, const Mat& featsVal, const float* labelsVal, VlSvm * &bestsvm)
{
    Mat double_featsTrain;
    featsTrain.convertTo(double_featsTrain, CV_64F);
    assert(double_featsTrain.at<double>(0,0)==((double*)double_featsTrain.data)[0]);
    assert(double_featsTrain.at<double>(0,1)==((double*)double_featsTrain.data)[1]);
    assert(double_featsTrain.at<double>(1,0)==((double*)double_featsTrain.data)[double_featsTrain.cols]);
    assert(double_featsTrain.isContinuous());
    double bestmap=0;
    double bestlambda=0;
    for (double lambda : sgdparams_lbds)
    {
        if (test_mode!=0)
            vl_rand_seed (vl_get_rand(), 0) ;
       
        VlSvm * svm = vl_svm_new(VlSvmSolverSdca,//stochastic dual cord ascent
                               (double*)double_featsTrain.data, featsTrain.cols, featsTrain.rows,
                               labelsTrain,
                               lambda) ;
        //VlSvm * svm = vl_svm_new(VlSvmSolverSdca,//stochastic dual cord ascent
        //                       (double*)double_featsTrain.data, featsTrain.cols, featsTrain.rows,
        //                       labelsTrain,
        //                       lambda) ;
        assert(svm!=NULL);
        vl_svm_set_bias_multiplier (svm, 0.1);
        vl_svm_train(svm) ;
        
        double cmap = modelMap(svm,featsVal,labelsVal);
        if (cmap > bestmap || bestsvm==NULL)
        {
            bestmap=cmap;
            bestlambda=lambda;
            if (bestsvm!=NULL)
                delete bestsvm;
            bestsvm=svm;
        }
        else
        {
            delete svm;
        }
    }
    Mat ret(featsVal.cols,1,CV_32F);
    for (int c=0; c<ret.rows; c++)
    {
        ret.at<float>(c,0)=vl_svm_get_model(bestsvm)[c];
    }
    assert(bestsvm!=NULL);
    return ret;
}

double EmbAttSpotter::modelMap(VlSvm * svm, const Mat& featsVal, const float* labelsVal)
{
    vector< pair<int,float> > scores(featsVal.rows);
    for (int r=0; r<featsVal.rows; r++)
    {
        float s=0;
        for (int c=0; c<featsVal.cols; c++)
            s += featsVal.at<float>(r,c)*vl_svm_get_model(svm)[c];
        scores[r]=make_pair(labelsVal[r],s);
    }
    sort(scores.begin(), scores.end(),[](const pair<int,float>& lh, const pair<int,float>& rh) {return lh.second>rh.second;}); 
    vector<float> acc(scores.size());
    int accum=0;
    int N=0;
    int place=0;
    double map=0;
    for (const pair<int,float>& score : scores)
    {
        accum += score.first;
        map += (accum*score.first)/(double)(++place);
    }
    return map/N;
}

Mat EmbAttSpotter::select_rows(const Mat& m, vector<int> idx)
{
    Mat ret(idx.size(), m.cols, m.type());
    for (int i=0; i<idx.size(); i++)
    {
        m.row(idx[i]).copyTo(ret.row(i));
    }
    return ret;
}

const EmbAttSpotter::AttributesModels& EmbAttSpotter::attModels(bool retrain)
{
    if (_attModels==NULL)
    {
        #pragma omp  critical (learn_attributes_bagging)
        {
            if (_attModels==NULL)
            {
                string name = saveName+"_attModels.dat";
                ifstream in(name);
                if (!retrain && in)
                {
                    //load
                    _attModels = new AttributesModels();
                    _attModels->W = readFloatMat(in);
                    //_attModels->B = readFloatMat(in);
                    _attModels->numPosSamples = readFloatMat(in);
                    //_attReprTr = new Mat();
                    _attReprTr = readFloatMat(in);
                    in.close();
                }
                else
                {
                    in.close();
                    learn_attributes_bagging();
                    //save
                    ofstream out(name);
                    writeFloatMat(out,_attModels->W);
                    //writeFloatMat(out,_attModels->B);
                    writeFloatMat(out,_attModels->numPosSamples);
                    writeFloatMat(out,_attReprTr);
                    out.close();
                }
            }
        }
    }
    return *_attModels;
}

const EmbAttSpotter::Embedding& EmbAttSpotter::embedding(bool retrain)
{
    if (_embedding==NULL)
    {
        #pragma omp  critical (embedding)
        {
            if (_embedding==NULL)
            {
                string name = saveName+"_embedding.dat";
                ifstream in(name);
                if (!retrain && in)
                {
                    //load
                    _embedding = new Embedding();
                    _embedding->rndmatx = readFloatMat(in);
                    _embedding->rndmaty = readFloatMat(in);
                    in >> _embedding->M;
                    _embedding->matt = readFloatMat(in);
                    _embedding->mphoc = readFloatMat(in);
                    _embedding->Wx = readFloatMat(in);
                    _embedding->Wy = readFloatMat(in);
                    in.close();
                }
                else
                {
                    in.close();
                    learn_common_subspace();
                    //save
                    ofstream out(name);
                    writeFloatMat(out,_embedding->rndmatx);
                    writeFloatMat(out,_embedding->rndmaty);
                    out << _embedding->M << " ";
                    writeFloatMat(out,_embedding->matt);
                    writeFloatMat(out,_embedding->mphoc);
                    writeFloatMat(out,_embedding->Wx);
                    writeFloatMat(out,_embedding->Wy);
                    out.close();
                }
            }
        }
    }
    return *_embedding;
}

void EmbAttSpotter::learn_common_subspace()
{
    if (test_mode)
        cout<<"learn_common_subspace()"<<endl;
    assert(_embedding==NULL);
    _embedding = new Embedding();
    
    /*Mat attReprTr_lcs, attReprVa_lcs, phocsTr_lcs, phocsVa_lcs;
    vector<string> wordClsTr, wordClsVa;
    //...
    
    assert(attReprTr().rows == phocsTr().rows && 
            attReprTr().rows == training_labels.size());
    
    vector<int> seeds;
    for (int cont = 0; cont < attReprTr().rows; cont++)
        seeds.push_back(cont);

    randShuffle(seeds);

    Mat output;
    for (int cont = 0; cont < (int)(0.66*seeds.size()); cont++)
    {
        attReprTr_lcs.push_back(attReprTr().row(seeds[cont]));
        phocsTr_lcs.push_back(phocsTr().row(seeds[cont]));
        wordClsTr.push_back(training_labels[seeds[cont]]);
    }
    for (int cont = 0.66*seeds.size(); cont < seeds.size(); cont++)
    {
        attReprVal_lcs.push_back(attReprTr().row(seeds[cont]));
        phocsVal_lcs.push_back(phocsTr().row(seeds[cont]));
        wordClsVal.push_back(training_labels[seeds[cont]]);
    }*/
    
    //learnKCCA
    int M = 2500;
    float G = 40;
    int Dims = 160;//or K
    float reg = 1e-5;
    /*if (test_mode)
    {
        M=200;
        Dims=50;
    }*/
    int Dx = attReprTr().rows;
    int Dy = phocsTr().rows;
    Mat rndmatx(M,Dx,CV_32F);
    Mat rndmaty(M,Dy,CV_32F);
    if (test_mode)//==2)
    {
        //rndmatx = Mat::ones(M,Dx,CV_32F)*(1.0/(2.0*G));
        //rndmaty = Mat::ones(M,Dy,CV_32F)*(1.0/(2.0*G));
        #if TEST_MODE
        vector< vector<float> > f;///embedding_rndmatx_test2.csv
        readCSV("test/embedding_rndmatx_test2.csv", f);
        for (int r=0; r<f.size(); r++)
            for (int c=0; c<f[0].size(); c++)
                rndmatx.at<float>(r,c)=f[r][c];
        
        vector< vector<float> > f2;
        readCSV("test/embedding_rndmaty_test2.csv", f2);
        for (int r=0; r<f2.size(); r++)
            for (int c=0; c<f2[0].size(); c++)
                rndmaty.at<float>(r,c)=f2[r][c];
        
        #endif
        
    }
    else
    {
        RNG rng(12345);
        rng.fill(rndmatx,RNG::NORMAL,Scalar(0),Scalar(1/G));
        rng.fill(rndmaty,RNG::NORMAL,Scalar(0),Scalar(1/G));
    }
    
    Mat tmp = rndmatx*attReprTr();
    vconcat(cosMat(tmp),sinMat(tmp),tmp);
    Mat attReprTr_emb = (1/sqrt(M)) * tmp;
    tmp = rndmaty*phocsTr();
    vconcat(cosMat(tmp),sinMat(tmp),tmp);
    Mat phocsTr_emb = (1/sqrt(M)) * tmp;
#if TEST_MODE
    if (test_mode)
    {
        vector< vector<float> > attReprTr_cossin;
        readCSV("test/i_attRepTr_cossin_test2.csv", attReprTr_cossin);
        assert(attReprTr_cossin.size()==attReprTr_emb.rows && attReprTr_cossin[0].size()==attReprTr_emb.cols);
        for (int r=0; r<attReprTr_cossin.size(); r++)
            for (int c=0; c<attReprTr_cossin[0].size(); c++)
                assert(fabs(attReprTr_emb.at<float>(r,c)-attReprTr_cossin[r][c])<0.001);
        vector< vector<float> > phocsTr_cossin;
        readCSV("test/i_phocsTr_cossin_test2.csv", phocsTr_cossin);
        assert(phocsTr_cossin.size()==phocsTr_emb.rows && phocsTr_cossin[0].size()==phocsTr_emb.cols);
        for (int r=0; r<phocsTr_cossin.size(); r++)
            for (int c=0; c<phocsTr_cossin[0].size(); c++)
                assert(fabs(phocsTr_emb.at<float>(r,c)-phocsTr_cossin[r][c])<0.001);
    }
#endif
    // Mean center
    Mat ma;// = mean(attReprTr_emb,2);
    reduce(attReprTr_emb, ma, 1, CV_REDUCE_AVG);
    for (int r=0; r<ma.rows; r++)
        for (int c=0; c<ma.cols; c++)
            assert(ma.at<float>(r,c)==ma.at<float>(r,c));
    
    for (int c = 0; c < attReprTr_emb.cols; ++c) {
        attReprTr_emb.col(c) = attReprTr_emb.col(c) - ma;
    }

    Mat mh;// = mean(attReprTr_emb,2);
    reduce(phocsTr_emb, mh, 1, CV_REDUCE_AVG);
    for (int c = 0; c < phocsTr_emb.cols; ++c) {
        phocsTr_emb.col(c) = phocsTr_emb.col(c) - mh;
    }
#if TEST_MODE
    if (test_mode)
    {
        vector< vector<float> > attReprTr_ma;
        readCSV("test/i_attRepTr_ma_test2.csv", attReprTr_ma);
        assert(attReprTr_ma.size()==attReprTr_emb.rows && attReprTr_ma[0].size()==attReprTr_emb.cols);
        for (int r=0; r<attReprTr_ma.size(); r++)
            for (int c=0; c<attReprTr_ma[0].size(); c++)
                assert(fabs(attReprTr_emb.at<float>(r,c)-attReprTr_ma[r][c])<0.001);
        vector< vector<float> > phocsTr_mh;
        readCSV("test/i_phocsTr_mh_test2.csv", phocsTr_mh);
        assert(phocsTr_mh.size()==phocsTr_emb.rows && phocsTr_mh[0].size()==phocsTr_emb.cols);
        for (int r=0; r<phocsTr_mh.size(); r++)
            for (int c=0; c<phocsTr_mh[0].size(); c++)
                assert(fabs(phocsTr_emb.at<float>(r,c)-phocsTr_mh[r][c])<0.001);
    }
#endif

    // Learn CCA
    Mat Wx, Wy;
    cca2(attReprTr_emb.t(), phocsTr_emb.t(),reg,Dims,Wx,Wy);
    
    _embedding->rndmatx=rndmatx;
    _embedding->rndmaty=rndmaty;
    _embedding->M=M;
    _embedding->matt=ma;
    _embedding->mphoc=mh;
    _embedding->Wx=Wx;
    _embedding->Wy=Wy;
}

void EmbAttSpotter::cca2(Mat X, Mat Y, float reg, int d, Mat& Wx, Mat& Wy)
{
    // CCA calculate canonical correlations
    
    /* [Wx Wy r] = cca(X,Y) where Wx and Wy contains the canonical correlation
    % vectors as columns and r is a vector with corresponding canonical
    % correlations. The correlations are sorted in descending order. X and Y
    % are matrices where each column is a sample. Hence, X and Y must have
    % the same number of columns.
    %
    % Example: If X is M*K and Y is N*K there are L=MIN(M,N) solutions. Wx is
    % then M*L, Wy is N*L and r is L*1.
    %
    %
    % ? 2000 Magnus Borga, Link?pings universitet*/

    // --- Calculate covariance matrices ---
    int N = X.rows;
    int Dx = X.cols;
    int Dy = Y.cols;    


    Mat Cxx = X.t()*X / N + reg*Mat::eye(Dx,Dx,CV_32F);
    Mat Cyy = Y.t()*Y/N + reg*Mat::eye(Dy,Dy,CV_32F);
    Mat Cxy = X.t()*Y / N;
    Mat Cyx = Cxy.t();
#if TEST_MODE
    if (test_mode)
    {
        vector< vector<float> > loadCxx;
        readCSV("test/i_Cxx_test2.csv", loadCxx);
        assert(loadCxx.size()==Cxx.rows && loadCxx[0].size()==Cxx.cols);
        for (int r=0; r<loadCxx.size(); r++)
            for (int c=0; c<loadCxx[0].size(); c++)
                assert(fabs(Cxx.at<float>(r,c)-loadCxx[r][c])<0.001);
        vector< vector<float> > loadCyy;
        readCSV("test/i_Cyy_test2.csv", loadCyy);
        assert(loadCyy.size()==Cyy.rows && loadCyy[0].size()==Cyy.cols);
        for (int r=0; r<loadCyy.size(); r++)
            for (int c=0; c<loadCyy[0].size(); c++)
                assert(fabs(Cyy.at<float>(r,c)-loadCyy[r][c])<0.001);
        vector< vector<float> > loadCxy;
        readCSV("test/i_Cxy_test2.csv", loadCxy);
        assert(loadCxy.size()==Cxy.rows && loadCxy[0].size()==Cxy.cols);
        for (int r=0; r<loadCxy.size(); r++)
            for (int c=0; c<loadCxy[0].size(); c++)
                assert(fabs(Cxy.at<float>(r,c)-loadCxy[r][c])<0.001);
    }
#endif

    // --- Calcualte Wx and r ---
    //This is a heirachical test for which method will work, I don't really know if Cholesky is better...
    Mat tmp;
    if (!solve(Cxx,Cxy,tmp,cv::DECOMP_CHOLESKY))//mldivide
        if (!solve(Cxx,Cxy,tmp,cv::DECOMP_EIG))
            solve(Cxx,Cxy,tmp);
    if (!solve(Cyy.t(),tmp.t(),tmp,cv::DECOMP_CHOLESKY))//mrdivide
        if (!solve(Cyy.t(),tmp.t(),tmp,cv::DECOMP_EIG))
            solve(Cyy.t(),tmp.t(),tmp);
    Mat M =  (tmp.t())*Cyx;
#if TEST_MODE
    if (test_mode)
    {
        vector< vector<float> > loadM;
        readCSV("test/i_M_test2.csv", loadM);
        assert(loadM.size()==M.rows && loadM[0].size()==M.cols);
        for (int r=0; r<loadM.size(); r++)
            for (int c=0; c<loadM[0].size(); c++)
                assert(fabs(M.at<float>(r,c)-loadM[r][c])<0.001);
    }
#endif
    //[Wx,r] = eigs(double(M),d); // Basis in X
    //Mat r;
    //eigen(M.t(), r, Wx); 
    //assert(r.rows >= d);
    //Wx = Wx(Rect(0,0,Wx.cols,d)).t(); try and copy MATALB
    
    /*sqrt(r,r);      // Canonical correlations
    Mat V;
    flip(Wx,V,0);
    Mat rcc;
    flip(r,rcc,0);
    // already sorted, but for recreating MATLAB's sake ...
    Mat inds;
    sortIdx(rcc, inds, CV_SORT_EVERY_ROW + CV_SORT_DESCENDING);
    //for (int row=0; row<r.rows; row++)
    //    r.at<float>(inds.at<int>(row,0),0)=
    for (int row=0; row<inds.rows; row++)
    {
        r.row(row) = rcc.row(inds.at<int>(row,0));
        if (row>0 && row<d)
            assert(r.at<float>(row-1,0) > r.at<float>(row,0));
        Wx.row(row) = V.row(inds.at<int>(row,0));
    }
    Wx = Wx(Rect(0,0,Wx.cols,d)).t();*/
    
    arma::mat aM(M.rows,M.cols);
    for (int r=0; r<M.rows; r++)
        for(int c=0; c<M.cols; c++)
        {
            //not symetric --assert(M.at<float>(r,c)==M.at<float>(c,r));
            aM(r,c)=M.at<float>(r,c);
        }
    arma::cx_vec cxr;
    arma::cx_mat aWx;

    arma::eig_gen( cxr, aWx, aM );
    arma::vec r = arma::real(r);//      % Canonical correlations

    // --- Sort correlations ---

    arma::cx_mat V = aWx; //arma::fliplr(aWx);//         % reverse order of eigenvectors
    //r = arma::flipud(r);//    % extract eigenvalues anr reverse their orrer
    arma::uvec I = sort_index(r,"descend");// % sort reversed eigenvalues in ascending order
    //arma::vec rN=r;
    for (int j = 0; j<I.n_elem; j++)
    {
        //r(j) = rN(I(j));
        aWx.col(j) = V.col(I(j));
        //Wx(:,j) = V(:,I(j));  % sort reversed eigenvectors in ascending order
    }
    //r = arma::flipud(r);//          % restore sorted eigenvalues into descending order
    //aWx = arma::fliplr(aWx); 

    Wx=Mat(aWx.n_rows,d,CV_32F);
    for (int r=0; r<aWx.n_rows; r++)
        for (int c=0; c<d; c++)
            Wx.at<float>(r,c) = real(aWx(r,c)); //hmm, I'm assuming I dont want imaginary
    //Wx = Wx.t();

    // --- Calcualte Wy  ---
    solve(Cyy,Cyx,tmp);
    Wy = (tmp)*Wx;     // Basis in Y
    normalizeL2Columns(Wy);
}

const Mat& EmbAttSpotter::attReprTr(bool retrain)//correct orientation
{
    if (_attReprTr.rows==0)
    {
        #pragma omp  critical (learn_attributes_bagging)
        {
            if (_attReprTr.rows==0)
            {
                string name = saveName+"_attModels.dat";
                ifstream in(name);
                if (!retrain && in)
                {
                    //load
                    _attModels = new AttributesModels();
                    _attModels->W = readFloatMat(in);
                    //_attModels->B = readFloatMat(in);
                    _attModels->numPosSamples = readFloatMat(in);
                    //_attReprTr = new Mat();
                    _attReprTr = readFloatMat(in);
                    in.close();
                }
                else
                {
                    in.close();
                    learn_attributes_bagging();
                    //save
                    ofstream out(name);
                    writeFloatMat(out,_attModels->W);
                    //writeFloatMat(out,_attModels->B);
                    writeFloatMat(out,_attModels->numPosSamples);
                    writeFloatMat(out,_attReprTr);
                    out.close();
                }
            }
        }
    }
    return _attReprTr;
}

/*const Mat& EmbAttSpotter::attReprVa()
{
    if (_attReprVa==NULL)
    {
        string name = saveName+"_attReprVa.dat";
        ifstream in(name)
        if (!retrain && in)
        {
            //load
            _attReprVa = new Mat();
            *_attReprVa = readFloatMat(in);;
            in.close();
        }
        else
        {
            in.close();
            _attReprVa = new Mat();
            *_attReprVa = attModels().W*feats_validation().t();
            //save
            ofstream out(name);
            writeFloatMat(out,_attReprVa);
            out.close();
        }
        
    }
    return *_attReprVa;
}*/

vector<struct spotting_sw> EmbAttSpotter::spot_sw(const Mat& exemplar, string ngram, float alpha)
{
    
}

/*Mat EmbAttSpotter::scores(const Mat& query, const Mat& corpus_batch)
{
    return query*dataset.t();
}

Mat EmbAttSpotter::scores_sw(const Mat& query, const vector<Mat>& corpus_batch)
{
    for (const Mat& inst : corpus_batch)
    {
        //TODO
    }
}*/

void EmbAttSpotter::setTrainData(string gtFile, string imageDir, string saveAs)
{
    if (saveAs.size()>0)
        saveName=saveAs;
    /*if (in)
        in.close();
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (imageDir.c_str())) != NULL)
    {
        //cout << "reading images and obtaining descriptors" << endl;
        
        //get all filenames
        vector<string> fileNames;
        while ((ent = readdir (dir)) != NULL) {
            string fileName(ent->d_name);
            if (fileName[0] == '.' || (fileName[fileName.size()-1]!='G' && fileName[fileName.size()-1]!='g' &&  fileName[fileName.size()-1]!='f'))
                continue;
            fileNames.push_back(fileName);
        }*/
}


//We compute the GMM and PCA together as they are relient on the same data.
void EmbAttSpotter::get_GMM_PCA(int numWordsTrainGMM, string saveAs, bool retrain)
{   
    
    
    string name = saveAs+"_GMM_PCA_"+to_string(minH)+"_"+to_string(numGMMClusters)+"_"+to_string(PCA_dim)+".dat";
    ifstream in(name);
    if (!retrain && in)
    {
         //cout << name << " file already exists. Not retraining GMM and PCA." << endl;
         //load
        assert(_GMM.means==NULL);
        
        _PCA.mean= readFloatMat(in);
        _PCA.eigvec = readFloatMat(in);
        int size, sizeFull;
        _GMM.means = readFloatArray(in,&sizeFull);
        numGMMClusters = sizeFull/(AUG_PCA_DIM*numSpatialX*numSpatialY);
        _GMM.covariances = readFloatArray(in);
        _GMM.priors = readFloatArray(in,&size);
        assert(numGMMClusters== size/(numSpatialX*numSpatialY));
        
        PCA_dim = _PCA.eigvec.cols;
        
        in.close();
    }
    else
    {
        assert(training_imgfiles!=NULL || training_dataset!=NULL);
        /*if (in)
            in.close();
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir (imageDir.c_str())) != NULL)
        {
            //cout << "reading images and obtaining descriptors" << endl;
            
            //get all filenames
            vector<string> fileNames;
            while ((ent = readdir (dir)) != NULL) {
                string fileName(ent->d_name);
                if (fileName[0] == '.' || (fileName[fileName.size()-1]!='G' && fileName[fileName.size()-1]!='g' &&  fileName[fileName.size()-1]!='f'))
                    continue;
                fileNames.push_back(fileName);
            }*/
        int training_size;
        if (training_dataset!=NULL)
            training_size=training_dataset->size();
        else
            training_size=training_imgfiles->size();
        assert(training_size>=numWordsTrainGMM);
        
        Mat for_PCA(num_samples_PCA,SIFT_DIM,CV_32F);
        int sample_per_for_PCA = num_samples_PCA/numWordsTrainGMM;
        int on_sample=0;
        
        vector<Mat> bins(numSpatialX*numSpatialY);
        /*for (int i=0; i<numSpatialX*numSpatialY; i++)
        {
            bins[i] = Mat::zeros(0,SIFT_DIM+2,CV_32F);
        }*/
        
        #if TEST_MODE
        Mat newFor_PCA;
        #endif
        if (test_mode)
        {
            //Read in for_PCA from MATLAB's results
            vector<vector<float> > csv;
            readCSV("test/GMM_PCA_descs_all_test.csv",csv);
            newFor_PCA = Mat(csv[0].size(),csv.size(),CV_32F);
            for (int r=0; r<csv.at(0).size(); r++)
                for (int c=0; c<csv.size(); c++)
                {
                    newFor_PCA.at<float>(r,c) = csv[c][r];
                }
        }
        
        
        
        vector<bool> used(training_size);
        for (int i=0; i<numWordsTrainGMM; i++)
        {
            
            
            int imageIndex = rand()%training_size;
            
            if (test_mode!=0)
                imageIndex = i;
            else
            {
                int initIndex=imageIndex;
                while (used[imageIndex])
                {
                    imageIndex = (imageIndex+1)%training_size;
                    assert(imageIndex!=initIndex);
                }
            }
            
            Mat im;
            if (training_dataset!=NULL)
                im = training_dataset->image(imageIndex);
            else
                im = imread(training_imgfiles->at(imageIndex),CV_LOAD_IMAGE_GRAYSCALE);
            used[imageIndex]=true;
            //resize to minimum height
            /*if (im.rows<minH)
            {
                double ar = im.rows/(double)im.cols;
                int newWidth=minH/ar;
                resize(im, im, Size(minH, newWidth), 0, 0, INTER_CUBIC);
            }*/
            
            if (test_mode)
            {
            //cout <<"image "<<imageIndex<<" size ["<<im.rows<<" "<<im.cols<<"]"<<endl;
            //cout << training_dataset->labels()[imageIndex] << endl;
            //cout <<"image "<<imageIndex+1<<" size ["<<training_dataset->image(imageIndex+1).rows<<" "<<training_dataset->image(imageIndex+1).cols<<"]"<<endl;
            //cout << training_dataset->labels()[imageIndex+1] << endl;
            //cout << "canary "<<im.at<unsigned char>(0,0)<<endl;
            }
            
            
            
            Mat desc = phow(im);//includes xy's, normalization //TODO A possible improvment, include more meta-data like scale
            assert(desc.type() == CV_32F);
            assert(desc.cols == DESC_DIM);
            if (test_mode!=0)
            {   
                vector<vector<float> > csv;
                readCSV("test/GMM_PCA_descs/GMM_PCA_desc_"+to_string(i)+"_test.csv",csv);
                assert(csv.size() == desc.cols);
                cout <<"size dif "<<((int)desc.rows-(int)csv.at(0).size())<<endl;
                assert((int)desc.rows-(int)csv.at(0).size()<500);
                /*for (int ii=0; ii<desc.cols; ii++)
                    cout << desc.at<float>(0,ii) << endl;*/
                //exit(1);
                
                Mat newDesc(csv[0].size(),csv.size(),CV_32F);
                for (int r=0; r<csv[0].size(); r++)
                    for (int c=0; c<csv.size(); c++)
                        newDesc.at<float>(r,c)=csv[c][r];
                desc=newDesc;
                
                ///int numSamp = min(num_samples_PCA,desc.rows);
                for (int sample=0; sample<desc.rows && on_sample<num_samples_PCA; sample++)
                    desc(Rect(0,sample,SIFT_DIM,1)).copyTo(for_PCA.row(on_sample++));
                
            }
            else
                for (int sample=0; sample<sample_per_for_PCA; sample++)
                {
                    int randIndex = rand()%desc.rows;
                    desc(Rect(0,randIndex,SIFT_DIM,1)).copyTo(for_PCA.row(on_sample++));
                }
            
            
            //place in bins
            for (int r=0; r<desc.rows; r++)
            {
                int xBin = ((desc.at<float>(r,desc.cols-2)+0.5)/1.0)*numSpatialX;
                if (xBin<0) xBin=0;
                if (xBin>=numSpatialX) xBin=numSpatialX-1;
                int yBin = ((desc.at<float>(r,desc.cols-1)+0.5)/1.0)*numSpatialY;
                if (yBin<0) yBin=0;
                if (yBin>=numSpatialY) yBin=numSpatialY-1;
                //cout << "added desc to bin ["<<xBin<<","<<yBin<<"]"<<endl;
                if (bins[xBin*numSpatialY+yBin].rows!=0)
                    bins[xBin*numSpatialY+yBin].push_back(desc.row(r));
                else if (r!=62)
                    bins[xBin*numSpatialY+yBin]=desc.row(r).clone();
            }
        }
        
        //compute PCA
        if (num_samples_PCA>on_sample)
        {
            cout<<"Resizing for_PCA "<<on_sample<<endl;
            for_PCA = for_PCA(Rect(0,0,SIFT_DIM,on_sample));
        }
        else
            cout <<"for_PCA is fullsized. "<<num_samples_PCA<<endl;
        if (test_mode)
        {
            if (newFor_PCA.rows != for_PCA.rows || newFor_PCA.cols != for_PCA.cols)
            {
                cout << "for_PCA size mismatch, matlab: "<<newFor_PCA.rows<<","<< newFor_PCA.cols<<" mine: "<<for_PCA.rows<<","<<for_PCA.cols<<endl;
            }
            /*for (int r=0; r<newFor_PCA.rows; r++)
                for (int c=0; c<newFor_PCA.cols; c++)
                {
                    if (fabs(for_PCA.at<float>(r,c) - newFor_PCA.at<float>(r,c) > 0.01))
                        cout <<"sig dif at ["<<r<<","<<c<<"]: "<<for_PCA.at<float>(r,c) - newFor_PCA.at<float>(r,c)<<endl;
                }*/
            for_PCA=newFor_PCA;
        }
        compute_PCA(for_PCA.t(),PCA_dim);
            
        
        //GMM
        if (test_mode)
        {
            vector<Mat> newBins(numSpatialX*numSpatialY);
            for (int i=0; i<numSpatialX*numSpatialY; i++)
            {
                vector<vector<float> > csv;
                readCSV("test/GMM_vecs/GMM_descs_"+to_string(i)+".csv",csv);
                newBins[i] = Mat(csv[0].size(),csv.size(),CV_32F);
                for (int r=0; r<csv.at(0).size(); r++)
                    for (int c=0; c<csv.size(); c++)
                    {
                        newBins[i].at<float>(r,c) = csv[c][r];
                    }
                if (newBins[i].rows!=bins[i].rows || newBins[i].cols!=bins[i].cols)
                    cout <<"Desc Size dif for bin "<<i<<", mine: "<<bins[i].rows<<", "<<bins[i].cols<<",  MATLAB: "<<newBins[i].rows<<", "<<newBins[i].cols<<endl;
                else
                    for (int r=0; r<csv.at(0).size(); r++)
                        for (int c=0; c<csv.size(); c++)
                        {
                            if (fabs(newBins[i].at<float>(r,c)-bins[i].at<float>(r,c))>0.001)
                            {
                                cout <<"Desc Large dif bin "<<i<<" ["<<r<<","<<c<<"] "<<newBins[i].at<float>(r,c)-bins[i].at<float>(r,c)<<endl;
                            }
                        }
            }
            
            bins=newBins;
        }
        compute_GMM(bins,numSpatialX,numSpatialY,numGMMClusters);
        
        
        //save
        ofstream out;
        out.open(name);
        writeFloatMat(out,_PCA.mean);
        writeFloatMat(out,_PCA.eigvec);
        writeFloatArray(out,_GMM.means,numGMMClusters*AUG_PCA_DIM*numSpatialX*numSpatialY);
        writeFloatArray(out,_GMM.covariances,numGMMClusters*AUG_PCA_DIM*numSpatialX*numSpatialY);
        writeFloatArray(out,_GMM.priors,numGMMClusters*numSpatialX*numSpatialY);
        out.close();
        /*}
        else
        {
            cout << "Error, could not open imageDir: "<<imageDir << endl;
            assert(false);
        }*/
     }
    
}

void EmbAttSpotter::writeFloatMat(ofstream& dst, const Mat& m)
{
    assert(m.type()==CV_32F);
    dst << "[ "<< m.rows<<" "<<m.cols<<" ] ";
    dst << setprecision(9);
    for (int r=0; r<m.rows; r++)
        for (int c=0; c<m.cols; c++)
        {
            assert(m.at<float>(r,c)==m.at<float>(r,c));
            dst << m.at<float>(r,c) << " ";
        }
}

Mat EmbAttSpotter::readFloatMat(ifstream& src)
{
    int rows, cols;
    string rS ="";
    string cS ="";
    //src >> rows;
    //src >> cols;
    char c=' ';
    while (c!='[')
    {
        c=src.get();
    }
    src.get();
    
    while (c!=' ')
    {
        c=src.get();
        rS+=c;
    }
    c='.';
    while (c!=' ')
    {
        c=src.get();
        cS+=c;
    }
    while (c!=']')
        c=src.get();
    c=src.get();
    rows = stoi(rS);
    cols = stoi(cS);
    Mat ret(rows,cols,CV_32F);
    for (int r=0; r<rows; r++)
        for (int c=0; c<cols; c++)
            src >> ret.at<float>(r,c);
    return ret;
}

void EmbAttSpotter::writeFloatArray(ofstream& dst, const float* a, int size)
{
    dst << "[ "<<size<<" ] ";
    dst << setprecision(9);
    for (int i=0; i<size; i++)
           dst << a[i] << " ";
}

float* EmbAttSpotter::readFloatArray(ifstream& src, int* sizeO)
{
    int size;
    //src >> size;
    string sS ="";
    char c=' ';
    while (c!='[')
    {
        c=src.get();
    }
    src.get();
    
    while (c!=' ')
    {
        c=src.get();
        sS+=c;
    }
    while (c!=']')
        c=src.get();
    c=src.get();
    size=stoi(sS);
    float* ret = new float[size];
    double tmp;
    for (int i=0; i<size; i++)
    {
        src >> tmp;
        ret[i]=tmp;
    }
    if (sizeO!=NULL)
        *sizeO = size;
    return ret;
}

const EmbAttSpotter::PCA_struct & EmbAttSpotter::PCA_(bool retrain)
{
    if (_PCA.eigvec.rows==0)
    {
        #pragma omp  critical (get_GMM_PCA)
        {
            if (_PCA.eigvec.rows==0)
                get_GMM_PCA(numWordsTrainGMM, saveName, retrain);
        }
    }
    
    return _PCA;
}

const EmbAttSpotter::GMM_struct & EmbAttSpotter::GMM(bool retrain)
{
    if (_GMM.means==NULL)
    {
        #pragma omp  critical (get_GMM_PCA)
        {
            if (_GMM.means==NULL)
                get_GMM_PCA(numWordsTrainGMM, saveName, retrain);
        }
    }
    
    return _GMM;
}

void EmbAttSpotter::compute_PCA(const Mat& data, int PCA_dim)
{
    if (test_mode)
        cout<<"compute_PCA()"<<endl;
    assert(_PCA.eigvec.rows==0);
    /*PCA pt_pca(data, cv::Mat(), CV_PCA_DATA_AS_ROW, PCA_dim);
    assert(pt_pca.mean.type()==CV_32F);
    _PCA.mean = pt_pca.mean;
    
    if (test_mode)
    {
        //test
        Mat testMean(1,128,CV_32F);
        for (int r=0; r<data.rows; r++)
            testMean += data.row(r);
        testMean /= data.rows;
        
        for (int c=0; c<data.cols; c++)
            assert(fabs( testMean.at<float>(0,c) - _PCA.mean.at<float>(0,c) )<0.0001);
        //test
    }
    
    _PCA.eigvec = pt_pca.eigenvectors;//(Rect(0,0,eig_vecs.cols,PCA_dim));
    */

    //armadillo version
    arma::mat X(data.rows,data.cols);
    for (int r=0; r<data.rows; r++)
        for(int c=0; c<data.cols; c++)
        {
            //not symetric --assert(M.at<float>(r,c)==M.at<float>(c,r));
            X(r,c)=data.at<float>(r,c);
        }

    arma::mat m = arma::mean(X,1);

    arma::cx_vec cxEigval;
    arma::cx_mat aEigvec;

    arma::eig_gen( cxEigval, aEigvec, arma::cov(X.t()));


    arma::cx_mat V = aEigvec; //arma::fliplr(aWx);//         % reverse order of eigenvectors
    arma::uvec I = sort_index(real(cxEigval),"descend");// % sort reversed eigenvalues in ascending order
    for (int j = 0; j<I.n_elem; j++)
    {
        aEigvec.col(j) = V.col(I(j));
    }

    _PCA.eigvec=Mat(aEigvec.n_rows,PCA_dim,CV_32F);
    for (int r=0; r<aEigvec.n_rows; r++)
        for (int c=0; c<PCA_dim; c++)
            _PCA.eigvec.at<float>(r,c) = real(aEigvec(r,c)); //hmm, I'm assuming I dont want imaginary
    _PCA.mean=Mat(m.n_rows,m.n_cols,CV_32F);
    for (int r=0; r<m.n_rows; r++)
        for (int c=0; c<m.n_cols; c++)
             _PCA.mean.at<float>(r,c) = m(r,c);
}

void EmbAttSpotter::compute_GMM(const vector<Mat>& bins, int numSpatialX, int numSpatialY, int numGMMClusters)
{
    if (test_mode)
        cout<<"compute_GMM()"<<endl;
    assert(_GMM.means==NULL);
    _GMM.means = new float[numGMMClusters*AUG_PCA_DIM*numSpatialX*numSpatialY];
    _GMM.covariances = new float[numGMMClusters*AUG_PCA_DIM*numSpatialX*numSpatialY];
    _GMM.priors = new float[numGMMClusters*numSpatialX*numSpatialY];
    //for (int i=0; i<numSpatialX*numSpatialY; i++)
    for (int xBin=0; xBin<numSpatialX; xBin++)
        for (int yBin=0; yBin<numSpatialY; yBin++)
    {
        //int xBin = i%numSpatialX;
        //int yBin = i/numSpatialX;
        int i = xBin*numSpatialY+yBin;
        Mat d = bins[i](Rect(0,0,SIFT_DIM,bins[i].rows));
        Mat xy = bins[i](Rect(SIFT_DIM,0,2,bins[i].rows));
        for (int r = 0; r < d.rows; ++r)
            d.row(r) = d.row(r) - PCA_().mean.t();
        //subtract(d,PCA_().mean,d);
        d = (PCA_().eigvec.t()*d.t()).t();//transposed to fit VL format  
        //hconcat(d,xy,d);
        hconcat(d,xy,d);
        assert(d.type() == CV_32F);
        assert(d.cols==AUG_PCA_DIM);
        assert(d.isContinuous());
        /*for (int r=0; r<d.rows; r++)
            for (int c=0; c<d.cols; c++)
                assert(d.at<float>(r,c)==d.at<float>(r,c));
        */
        if (test_mode)
        {
            vector<vector<float> > csv;
            readCSV("test/GMM_vecs/GMM_vec_"+to_string(i)+".csv",csv);
            Mat newD = Mat(csv[0].size(),csv.size(),CV_32F);
            for (int r=0; r<csv[0].size(); r++)
                for (int c=0; c<csv.size(); c++)
                {
                    newD.at<float>(r,c) = csv[c][r];
                }
            if (newD.rows!=d.rows || newD.cols!=d.cols)
                cout <<"Vector Size dif for bin "<<i<<", mine: "<<d.rows<<", "<<d.cols<<",  MATLAB: "<<newD.rows<<", "<<newD.cols<<endl;
            else
                for (int r=0; r<d.rows; r++)
                    for (int c=0; c<d.cols; c++)
                    {
                        if (fabs(fabs(newD.at<float>(r,c))-fabs(d.at<float>(r,c)))>0.001)
                        {
                            cout <<"Vector Large dif bin "<<i<<" ["<<r<<","<<c<<"] "<<newD.at<float>(r,c)<<"  "<<d.at<float>(r,c)<<endl;
                        }
                    }
            
            d=newD;
            
            
            vl_rand_seed (vl_get_rand(), 0) ;
        }
        
        VlGMM* gmm = vl_gmm_new (VL_TYPE_FLOAT, AUG_PCA_DIM, numGMMClusters) ;
        vl_gmm_set_max_num_iterations (gmm, 30);
        vl_gmm_set_num_repetitions (gmm, 2);
        vl_gmm_set_initialization (gmm,VlGMMRand);
        
        assert(((float*) d.data)[1] == d.at<float>(0,1));
        assert(((float*) d.data)[d.cols] == d.at<float>(1,0));
        
        
        vl_gmm_cluster (gmm, d.data, d.rows);
        
        float* means = (float*) vl_gmm_get_means(gmm);
        copy(means,means+numGMMClusters*AUG_PCA_DIM,_GMM.means+numGMMClusters*AUG_PCA_DIM*i);
        
        float* covariances = (float*) vl_gmm_get_covariances(gmm);
        /*for (int ttt=0; ttt<numGMMClusters*AUG_PCA_DIM; ttt++)
        {
            assert(covariances[ttt]==covariances[ttt]);
        }*/
        copy(covariances,covariances+numGMMClusters*AUG_PCA_DIM,_GMM.covariances+numGMMClusters*AUG_PCA_DIM*i);

        float* priors = (float*) vl_gmm_get_priors(gmm);
        copy(priors,priors+numGMMClusters,_GMM.priors+numGMMClusters*i);
        
        
        
        vl_gmm_delete(gmm);
         
         
    }
    
    //GMM.we = GMM.we/sum(GMM.we);
    float sum=0;
    for (int i=0; i<numGMMClusters*numSpatialX*numSpatialY; i++)
    {
        assert(_GMM.priors[i]==_GMM.priors[i]);
        sum += _GMM.priors[i];
    }
    for (int i=0; i<numGMMClusters*numSpatialX*numSpatialY; i++)
        _GMM.priors[i] /= sum;
    
    
    if (test_mode)
     {
        vector<vector<float> > GMM_mean;
        readCSV("test/GMM_mean_test.csv",GMM_mean);
        vector<vector<float> > GMM_covariances;
        readCSV("test/GMM_covariances_test.csv",GMM_covariances);
        vector<vector<float> > GMM_priors;
        readCSV("test/GMM_priors_test.csv",GMM_priors);
        assert(GMM_mean[0].size()==numGMMClusters*numSpatialX*numSpatialY);
        assert(GMM_mean.size()==AUG_PCA_DIM);
        for (int r=0; r<GMM_mean.size(); r++)
            for (int c=0; c<GMM_mean[0].size(); c++)
                _GMM.means[r+GMM_mean.size()*c]=GMM_mean[r][c];
        for (int r=0; r<GMM_covariances.size(); r++)
            for (int c=0; c<GMM_covariances[0].size(); c++)
                _GMM.covariances[r+GMM_covariances.size()*c]=GMM_covariances[r][c];
        for (int r=0; r<GMM_priors.size(); r++)
            for (int c=0; c<GMM_priors[0].size(); c++)
                _GMM.priors[r+GMM_priors.size()*c]=GMM_priors[r][c];
     }
    
    /*for (int ttt=0; ttt<numGMMClusters*AUG_PCA_DIM*numSpatialX*numSpatialY; ttt++)
    {
        assert(_GMM.means[ttt]==_GMM.means[ttt]);
        assert(_GMM.covariances[ttt]==_GMM.covariances[ttt]);
    }
    for (int ttt=0; ttt<numGMMClusters*numSpatialX*numSpatialY; ttt++)
    {
        assert(_GMM.priors[ttt]==_GMM.priors[ttt]);
    }*/
}




Mat EmbAttSpotter::embed_labels_PHOC(const vector<string>& labels)
{
    
    
    /* Prepare output */
    //float *phocs = new float[phocSize*corpusSize+phocSize_bi*corpusSize];
    Mat phocs = Mat::zeros(phocSize+phocSize_bi,labels.size(),CV_32F);
    /* Compute */
    for (int i=0; i < labels.size();i++)
    {
        computePhoc(labels[i], vocUni2pos, map<string,int>(),unigrams.size(), phoc_levels, phocSize, phocs,i);
    }
    
    for (int i=0; i < labels.size();i++)
    {
        computePhoc(labels[i], map<char,int>(), vocBi2pos,bigrams.size(), phoc_levels_bi, phocSize_bi, phocs,i);
    }
    
    
    return phocs;
}

#define HARD 0

void EmbAttSpotter::computePhoc(string str, map<char,int> vocUni2pos, map<string,int> vocBi2pos, int Nvoc, vector<int> levels, int descSize, Mat& out, int instance)
{
    int strl = str.length();
    
    int doUnigrams = vocUni2pos.size()!=0;
    int doBigrams = vocBi2pos.size()!=0;
    
    /* For each block */
    //float *p = out;
    int p=0;
    for (int level : levels)
    {
        /* For each split in that level */
        for (int ns=0; ns < level; ns++)
        {
            float starts = ns/(float)level;
            float ends = (ns+1)/(float)level;
            
            /* For each character */
            if (doUnigrams)
            {
                for (int c=0; c < strl; c++)
                {
                    if (vocUni2pos.count(str[c])==0)
                    {
                        /* Character not included in dictionary. Skipping.*/
                        continue;
                    }
                    int posOff = vocUni2pos[str[c]]+p;
                    float startc = c/(float)strl;
                    float endc = (c+1)/(float)strl;
                    
                    /* Compute overlap over character size (1/strl)*/
                    if (endc < starts || ends < startc) continue;
                    float start = (starts > startc)?starts:startc;
                    float end = (ends < endc)?ends:endc;
                    float ov = (end-start)*strl;
                    #if HARD
                    if (ov >=0.48)
                    {
                        //p[posOff]+=1;
                        out.at<float>(posOff,instance)+=1;
                    }
                    #else
                    //p[posOff] = max(ov, p[posOff]);
                    out.at<float>(posOff,instance)=max(ov, out.at<float>(posOff,instance));
                    #endif
                }
            }
            if (doBigrams)
            {
                for (int c=0; c < strl-1; c++)
                {
                    string sstr=str.substr(c,2);
                    if (vocBi2pos.count(sstr)==0)
                    {
                        /* Character not included in dictionary. Skipping.*/
                        continue;
                    }
                    int posOff = vocBi2pos[sstr]+p;
                    float startc = c/(float)strl;
                    float endc = (c+2)/(float)strl;
                    
                    /* Compute overlap over bigram size (2/strl)*/
                    if (endc < starts || ends < startc){ continue;}
                    float start = (starts > startc)?starts:startc;
                    float end = (ends < endc)?ends:endc;
                    float ov = (end-start)*strl/2.0;
                    if (ov >=0.48)
                    {
                        //p[posOff]+=1;
                        out.at<float>((out.rows-descSize)+posOff,instance)+=1;
                    }
                }
            }
            p+=Nvoc;
        }
    }
    return;
}

const Mat& EmbAttSpotter::phocsTr(bool retrain)//correct orientation
{
    if (_phocsTr.rows==0 || retrain)
    {
        #pragma omp  critical (phocsTr)
        {
            if (_phocsTr.rows==0 || retrain)
            {
                if (training_labels!=NULL)
                    _phocsTr = embed_labels_PHOC(*training_labels);
                else if (training_dataset!=NULL)
                    _phocsTr = embed_labels_PHOC(training_dataset->labels());
                else
                    assert(false && "No training data specified");
            }
        }
    }
    return _phocsTr;
}

/*const Mat& phocs_training()
{
    if (_phocs_training==NULL)
    {
        _phocs_training = new Mat(idxTrain.size()+idxValidation.size(),phocs().cols,phocs().type());
        int row=0;
        for (int idx : idxTrain)
        {
            phocs().row(idx).copyTo(_phocs_training->row(row++));
        }
        for (int idx : idxValidation)
        {
            phocs().row(idx).copyTo(_phocs_training->row(row++));
        }
    }
    return *_phocs_training;
}*/

Mat EmbAttSpotter::sinMat(const Mat& x)
{
    //x-(x^3/3!)+(x^5/5!)-(x^7/7!) +.... 
    /*Mat p3;
    pow(x,3,p3);
    Mat p5;
    pow(x,5,p5);
    Mat p7;
    pow(x,7,p7);
    Mat p9;
    pow(x,9,p9);
    Mat p11;
    pow(x,11,p11);
    Mat p13;
    pow(x,13,p13);
    Mat p15;
    pow(x,15,p15);
    Mat p17;
    pow(x,17,p17);
    
    return x-(p3/(2*3))+(p5/(2*3*4*5))-(p7/(2*3*4*5*6*7))-(p9/(2*3*4*5*6*7*8*9))-(p11/(2*3*4*5*6*7*8*9*10*11))-(p13/(2*3*4*5*6*7*8*9*10*11*12*13))-(p15/(2*3*4*5*6*7*8*9*10*11*12*13*14*15))-(p17/(2*3*4*5*6*7*8*9*10*11*12*13*14*15*16*17));*/
    assert(x.type()==CV_32F);
    Mat ret(x.rows,x.cols,CV_32F);
    for (int r=0; r<x.rows; r++)
        for (int c=0; c<x.cols; c++)
            ret.at<float>(r,c) = sin(x.at<float>(r,c));
    return ret;
}

Mat EmbAttSpotter::cosMat(const Mat& x)
{
    //return sinMat((-1*x)+(CV_PI/2));
    assert(x.type()==CV_32F);
    Mat ret(x.rows,x.cols,CV_32F);
    for (int r=0; r<x.rows; r++)
        for (int c=0; c<x.cols; c++)
            ret.at<float>(r,c) = cos(x.at<float>(r,c));
    return ret;
}

Mat& EmbAttSpotter::normalizeL2Columns(Mat& m)
{
    assert(m.type()==CV_32F);
    Mat tmp;
    multiply(m,m,tmp);
    reduce(tmp,tmp,0,CV_REDUCE_SUM);
    sqrt(tmp,tmp);
    for (int c=0; c<m.cols; c++)
    {
        if (tmp.at<float>(0,c)!=0)
            m.col(c) /=tmp.at<float>(0,c);
    }
    
    return m;
}

Mat EmbAttSpotter::otsuBinarization(const Mat& src)
{
    //make histogram
    vector<int> histogram(256);
    for (int x=0; x<src.cols; x++)
    {
        for (int y=0; y<src.rows; y++)
        {
            int bin = src.at<unsigned char>(y,x);
            histogram[bin]++;
        }
    }
    
    //otsu
    int total = src.rows*src.cols;
    double sum =0;
    for (int i = 1; i < 256; ++i)
            sum += i * histogram[i];
    double sumB = 0;
    double wB = 0;
    double wF = 0;
    double mB;
    double mF;
    double max = 0.0;
    double between = 0.0;
    double threshold1 = 0.0;
    double threshold2 = 0.0;
    for (int i = 0; i < 256; ++i)
    {
        wB += histogram[i];
        if (wB == 0)
            continue;
        wF = total - wB;
        if (wF == 0)
            break;
        sumB += i * histogram[i];
        mB = sumB / (wB*1.0);
        mF = (sum - sumB) / (wF*1.0);
        between = wB * wF * pow(mB - mF, 2);
        if ( between >= max )
        {
            threshold1 = i;
            if ( between > max )
            {
                threshold2 = i;
            }
            max = between; 
//            printf("1:%f\t 2:%f\tmax:%f\twB:%d\twF:%d\tmB:%f\tmf:%f\n",threshold1,threshold2,max,wB,wF,mB,mF);
        }
    }
//    printf("1:%f\t 2:%f\tmax:%f\n",threshold1,threshold2,max);
    
    double thresh = ( threshold1 + threshold2 ) / 2.0;
    //cout <<"thresh: "<<thresh<<" th1: "<<threshold1<<" th2: "<<threshold2<<" max: "<<max<<endl;
    
    //thresh
    Mat ret(src.rows,src.cols,CV_8U);
    for (int x=0; x<ret.cols; x++)
    {
        for (int y=0; y<ret.rows; y++)
        {
            if (src.at<unsigned char>(y,x) <= thresh)
                ret.at<unsigned char>(y,x)=1;
            else
                ret.at<unsigned char>(y,x)=0;
        }
    }
    return ret;
}

void EmbAttSpotter::setTraining_dataset(const Dataset* d)
{
    training_dataset=d;
}
void EmbAttSpotter::setCorpus_dataset(const Dataset* d)
{
    corpus_dataset=d;
}

//}}}//?
