
#include "embattspotter_eval.cpp"
#if TEST_MODE
    #include "embattspotter_test.cpp"
#endif

#include <armadillo>

#define CHAR_ASPECT_RATIO 2.45 //TODO, is this robust?
//#define SUBWORD_WINDOW_PAD 0.5 //[119]     //0.2 [86]
#define SUBWORD_WINDOW_EXTEND  1.3   //2.3//[15?] //1.8//[125] //2.0//[138] //1.7 //[119]  //1.3 [86]
#define COARSE_STRIDE 10
#define STRIDE_PORTION 0.28

EmbAttSpotter::EmbAttSpotter(string saveName, bool load, bool useNumbers, int test_mode)
{
    
    _GMM.means=NULL;
    _GMM.covariances=NULL;
    _GMM.priors=NULL;
    
    _attModels=NULL;
    _embedding=NULL;
    _batches_cca_att=NULL;
    _subwordWindows_cca_att_saved=NULL;
    _features_corpus=NULL;
    
    SIFT_sizes={2,4,6,8,10,12};
    stride=3;
    magnif=6.0;
    windowsize=1.5;   
    contrastthreshold=0.005; 

    maxSiftSize=-1;
    for (int size : SIFT_sizes)
        if (size>maxSiftSize)
            maxSiftSize=size;
    subwordWindowPad = (maxSiftSize/magnif)*5;
    
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
        num_samples_PCA = 1000000;//2000000;
    
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
    ifstream bFile("model/bigrams.txt");
    assert(bFile.is_open());
    string bigram;
    while (getline(bFile,bigram) && bigrams.size()<50)
    {
        bigrams.push_back(bigram);
    }
    bFile.close();
    assert(bigrams.size()>0);
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

    _averageCharWidth=-1;    
    
    this->saveName = saveName;//+"_sp"+to_string(numSpatialX)+"x"+to_string(numSpatialY);
    
    
    numBatches=-1;
    corpusSize=-1;
    corpus_imgfiles=NULL;
    training_imgfiles=NULL;
    training_labels=NULL;
    training_dataset=NULL;
    corpus_dataset=NULL;
    genericBatchSize=5000;

    s_windowWidth=-1;
    s_stride=-1;

    makeBig=false;


    /*int minH=80;
    int maxH=80;
    pp = [](Mat patch) {    
        patch.convertTo(patch,CV_32F);
        patch/=255;
        
        double m;
        minMaxIdx(patch,NULL,&m);
        if (m<0.2)
            patch*=0.2/m;
        
        if (patch.rows>maxH)
        {
            double ratio = (maxH+0.0)/patch.rows;
            resize(patch,patch,Size(),ratio,ratio,INTER_CUBIC);
        }
        else if (patch.rows<minH)
        {
            double ratio = (maxH+0.0)/patch.rows;
            resize(patch,patch,Size(),ratio,ratio,INTER_CUBIC);
        }
        
        
        patch*=255;
        patch.convertTo(patch,CV_8U);
        return patch;
    };*/
    spottingNgramLengths={2};
    if (load)
    {
        GMM();
        PCA_();
        attModels();
        embedding();
//        subwordWindows_cca_att_saved()
    }
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
    if (_subwordWindows_cca_att_saved!=NULL)
    {
        delete _subwordWindows_cca_att_saved;
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
    
    //for (int c=0; c<m.cols; c++)
    //    for (int r=0; r<m.rows; r++)
    //        assert(m.at<float>(r,c)>=0);
}


vector<float> EmbAttSpotter::spot(const Mat& exemplar, string word, float alpha)
{
    assert(alpha>=0 && alpha<=1);
    assert(word.length()>0 || alpha==1);
    assert(exemplar.rows*exemplar.cols>1 || alpha==0);
    assert (exemplar.channels()==1);


    
    
    Mat query_feats;
    if (makeBig)
    {
        Mat big;
        resize(exemplar,big,Size(),2,2);
        query_feats = extract_feats(big);
    }
    else
        query_feats = extract_feats(exemplar);
    
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
    if (test_mode==1)
    {
        //Read in from MATLAB's results
        vector<vector<float> > csv;
        readCSV("test/attReprTe_test2.csv",csv);
        assert(csv[0].size()==query_att.cols && csv.size()==query_att.rows);
        for (int c=0; c<csv.at(0).size(); c++)
            for (int r=0; r<csv.size(); r++)
            {
                query_att.at<float>(r,c) = csv[r][c];
            }
        csv.clear();
        readCSV("test/phocsTe_test2.csv",csv);
        assert(csv.size()==query_phoc.rows && csv[0].size()==query_phoc.cols);
        for (int r=0; r<csv.size(); r++)
            for (int c=0; c<csv.at(0).size(); c++)
            {
                query_phoc.at<float>(r,c) = csv[r][c];
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
    if (test_mode==1)
        compareToCSV(tmp,"test/attReprTe_tmp_test2.csv");
#endif
    vconcat(cosMat(tmp),sinMat(tmp),tmp);
    assert(embedding().matt.size() == tmp.size());
    Mat query_emb_att = (1/sqrt(embedding().M)) * tmp - embedding().matt;
#ifdef TEST_MODE
    if (test_mode==1)
        compareToCSV(query_emb_att,"test/attReprTe_emb_mean_test2.csv");
#endif
    tmp = maty*query_phoc;
#ifdef TEST_MODE
    if (test_mode==1)
        compareToCSV(tmp,"test/phocsTe_tmp_test2.csv");
#endif
    //checkNaN(tmp);
    vconcat(cosMat(tmp),sinMat(tmp),tmp);
    //checkNaN(embedding().mphoc);
    //checkNaN(tmp);
    assert(embedding().mphoc.size() == tmp.size());
    Mat query_emb_phoc = (1/sqrt(embedding().M)) * tmp - embedding().mphoc;
#ifdef TEST_MODE
    if (test_mode==1)
        compareToCSV(query_emb_phoc,"test/phocsTe_emb_mean_test2.csv");
#endif
    
    //checkNaN(embedding().Wy);
    //checkNaN(query_emb_phoc);
    
    Mat query_cca_att = embedding().Wx.t()*query_emb_att;
    Mat query_cca_phoc = embedding().Wy.t()*query_emb_phoc;
    
#ifdef TEST_MODE
    if (test_mode==1)
    {
        compareToCSV(query_cca_att,"test/attReprTe_cca_test2.csv");
        compareToCSV(query_cca_phoc,"test/phocsTe_cca_test2.csv");
    }
#endif
    //checkNaN(query_cca_att);
    //checkNaN(query_cca_phoc);
    
    normalizeL2Columns(query_cca_att);
    normalizeL2Columns(query_cca_phoc);
#ifdef TEST_MODE
    if (test_mode==1)
    {
        compareToCSV(query_cca_att,"test/attReprTe_cca_norm_test2.csv");
        compareToCSV(query_cca_phoc,"test/phocsTe_cca_norm_test2.csv");
        return vector<float>();
    }
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


float EmbAttSpotter::compare(const Mat& im1, const Mat& im2)
{
    assert(im1.rows*im1.cols>1);
    assert (im1.channels()==1);
    assert(im2.rows*im2.cols>1);
    assert (im2.channels()==1);
    
    
    Mat matx = embedding().rndmatx;//(Rect(0,0,embedding().M,embedding().rndmatx.cols));
    Mat maty = embedding().rndmaty;//(Rect(0,0,embedding().M,embedding().rndmatx.cols));
    assert(matx.rows==embedding().M);
    assert(maty.rows==embedding().M);
    
    /////////////////////
    Mat im1_feats;
    if (makeBig)
    {
        Mat big;
        resize(im1,big,Size(),2,2);
        im1_feats = extract_feats(big);
    }
    else
        im1_feats = extract_feats(im1);
    
    Mat im1_att = attModels().W.t()*im1_feats.t();

    
    Mat tmp;
    Mat im1_cca_att;
    tmp = matx*im1_att;
    vconcat(cosMat(tmp),sinMat(tmp),tmp);
    assert(embedding().matt.size() == tmp.size());
    Mat im1_emb_att = (1/sqrt(embedding().M)) * tmp - embedding().matt;
    
    
    im1_cca_att = embedding().Wx.t()*im1_emb_att;
    
    normalizeL2Columns(im1_cca_att);

    /////////////////////
    Mat im2_feats;
    if (makeBig)
    {
        Mat big;
        resize(im2,big,Size(),2,2);
        im2_feats = extract_feats(big);
    }
    else
        im2_feats = extract_feats(im2);
   
    Mat im2_att = attModels().W.t()*im2_feats.t();
    

    
    Mat im2_cca_att;
    tmp = matx*im2_att;
    vconcat(cosMat(tmp),sinMat(tmp),tmp);
    assert(embedding().matt.size() == tmp.size());
    Mat im2_emb_att = (1/sqrt(embedding().M)) * tmp - embedding().matt;
    
    
    im2_cca_att = embedding().Wx.t()*im2_emb_att;
    
    normalizeL2Columns(im2_cca_att);
    //////////////////////////

    Mat score = im1_cca_att.t() * im2_cca_att;
    return -1*score.at<float>(0,0);
}

/*float EmbAttSpotter::compare(string text, const Mat& im)
{
    assert(im.rows*im.cols>1);
    assert (im.channels()==1);
    assert (text.length()>0);
    
    
    Mat matx = embedding().rndmatx;//(Rect(0,0,embedding().M,embedding().rndmatx.cols));
    Mat maty = embedding().rndmaty;//(Rect(0,0,embedding().M,embedding().rndmatx.cols));
    assert(matx.rows==embedding().M);
    assert(maty.rows==embedding().M);
    
    /////////////////////
    Mat im_feats = extract_feats(im);
    
    Mat im_att = attModels().W.t()*im_feats.t();

    
    Mat tmp;
    Mat im_cca_att;
    tmp = matx*im_att;
    vconcat(cosMat(tmp),sinMat(tmp),tmp);
    assert(embedding().matt.size() == tmp.size());
    Mat im_emb_att = (1/sqrt(embedding().M)) * tmp - embedding().matt;
    
    
    im_cca_att = embedding().Wx.t()*im_emb_att;
    
    normalizeL2Columns(im_cca_att);

    /////////////////////
    Mat query_phoc = Mat::zeros(phocSize+phocSize_bi,1,CV_32F);
    computePhoc(text, vocUni2pos, map<string,int>(),unigrams.size(), phoc_levels, phocSize, query_phoc,0);
    computePhoc(text, map<char,int>(), vocBi2pos,bigrams.size(), phoc_levels_bi, phocSize_bi, query_phoc,0);
    tmp = maty*query_phoc;
    //checkNaN(tmp);
    vconcat(cosMat(tmp),sinMat(tmp),tmp);
    //checkNaN(embedding().mphoc);
    //checkNaN(tmp);
    assert(embedding().mphoc.size() == tmp.size());
    Mat query_emb_phoc = (1/sqrt(embedding().M)) * tmp - embedding().mphoc;
    Mat query_cca_phoc = embedding().Wy.t()*query_emb_phoc;
    //checkNaN(query_cca_phoc);
    
    
    normalizeL2Columns(query_cca_phoc);
    //////////////////////////

    Mat score = im_cca_att.t() * query_cca_phoc;
    return -1*score.at<float>(0,0);
}*/

//A non-lazy version of the function
float EmbAttSpotter::compare(string text, const Mat& im) const
{
    assert (_embedding!=NULL);
    assert (_attModels!=NULL);
    assert(im.rows*im.cols>1);
    assert (im.channels()==1);
    assert (text.length()>0);
    
    
    Mat matx = _embedding->rndmatx;//(Rect(0,0,embedding().M,embedding().rndmatx.cols));
    Mat maty = _embedding->rndmaty;//(Rect(0,0,embedding().M,embedding().rndmatx.cols));
    assert(matx.rows==_embedding->M);
    assert(maty.rows==_embedding->M);
    
    /////////////////////
    Mat im_feats;
    if (makeBig)
    {
        Mat big;
        resize(im,big,Size(),2,2);
        im_feats = extract_feats(big);
    }
    else
        im_feats = extract_feats(im);
    
    
    Mat im_att = _attModels->W.t()*im_feats.t();

    
    Mat tmp;
    Mat im_cca_att;
    tmp = matx*im_att;
    vconcat(cosMat(tmp),sinMat(tmp),tmp);
    assert(_embedding->matt.size() == tmp.size());
    Mat im_emb_att = (1/sqrt(_embedding->M)) * tmp - _embedding->matt;
    
    
    im_cca_att = _embedding->Wx.t()*im_emb_att;
    
    normalizeL2Columns(im_cca_att);

    /////////////////////
    Mat query_phoc = Mat::zeros(phocSize+phocSize_bi,1,CV_32F);
    computePhoc(text, vocUni2pos, map<string,int>(),unigrams.size(), phoc_levels, phocSize, query_phoc,0);
    computePhoc(text, map<char,int>(), vocBi2pos,bigrams.size(), phoc_levels_bi, phocSize_bi, query_phoc,0);
    tmp = maty*query_phoc;
    //checkNaN(tmp);
    vconcat(cosMat(tmp),sinMat(tmp),tmp);
    //checkNaN(embedding().mphoc);
    //checkNaN(tmp);
    assert(_embedding->mphoc.size() == tmp.size());
    Mat query_emb_phoc = (1/sqrt(_embedding->M)) * tmp - _embedding->mphoc;
    Mat query_cca_phoc = _embedding->Wy.t()*query_emb_phoc;
    //checkNaN(query_cca_phoc);
    
    
    normalizeL2Columns(query_cca_phoc);
    //////////////////////////

    Mat score = im_cca_att.t() * query_cca_phoc;
    return -1*score.at<float>(0,0);
}
float EmbAttSpotter::compare(string text, int imIdx) const
{
    assert (_embedding!=NULL);
    assert (_attModels!=NULL);
    assert (_batches_cca_att!=NULL);
    assert (text.length()>0);
    
    
    Mat maty = _embedding->rndmaty;//(Rect(0,0,embedding().M,embedding().rndmatx.cols));
    assert(maty.rows==_embedding->M);
    
    int batch=0;
    int si=0;
    for (;batch<_batches_cca_att->size(); batch++)
    {
        if (si+_batches_cca_att->at(batch).cols>imIdx)
            break;
        si+=_batches_cca_att->at(batch).cols;
    }
    Mat im_cca_att = _batches_cca_att->at(batch).col(imIdx-si);
    

    /////////////////////
    Mat query_phoc = Mat::zeros(phocSize+phocSize_bi,1,CV_32F);
    computePhoc(text, vocUni2pos, map<string,int>(),unigrams.size(), phoc_levels, phocSize, query_phoc,0);
    computePhoc(text, map<char,int>(), vocBi2pos,bigrams.size(), phoc_levels_bi, phocSize_bi, query_phoc,0);
    Mat tmp = maty*query_phoc;
    //checkNaN(tmp);
    vconcat(cosMat(tmp),sinMat(tmp),tmp);
    //checkNaN(embedding().mphoc);
    //checkNaN(tmp);
    assert(_embedding->mphoc.size() == tmp.size());
    Mat query_emb_phoc = (1/sqrt(_embedding->M)) * tmp - _embedding->mphoc;
    Mat query_cca_phoc = _embedding->Wy.t()*query_emb_phoc;
    //checkNaN(query_cca_phoc);
    
    
    normalizeL2Columns(query_cca_phoc);
    //////////////////////////

    Mat score = im_cca_att.t() * query_cca_phoc;
    return -1*score.at<float>(0,0);
}

void EmbAttSpotter::primeSubwordSpotting(int len)
{
    s_windowWidth =  averageCharWidth()*(len*SUBWORD_WINDOW_EXTEND) + 2*subwordWindowPad;
    s_stride=averageCharWidth()*STRIDE_PORTION;
    if (makeBig)
    {
        s_windowWidth*=2;
        s_stride*=2;
    }
    cout <<"subword["<<len<<"] window width: "<<s_windowWidth<<", stride: "<<s_stride<<endl;
    subwordWindows_cca_att_saved(0,s_windowWidth,s_stride);
}
void EmbAttSpotter::primeSubwordSpotting_set(int windowWidth, int stride)
{
    s_windowWidth =  windowWidth;
    s_stride=stride;
    if (makeBig)
    {
        s_windowWidth*=2;
        s_stride*=2;
    }
    cout <<"subword[] window width: "<<s_windowWidth<<", stride: "<<s_stride<<endl;
    subwordWindows_cca_att_saved(0,s_windowWidth,s_stride);
}

/*vector< SubwordSpottingResult > EmbAttSpotter::subwordSpot(const Mat& exemplar, string word, float alpha, float refinePortion)
{
    //int s_windowWidth=exemplar.cols;
    cout <<"Old windowWidth would have been: "<<exemplar.cols<<endl;
    //if (s_windowWidth==0)
    //{
    int  s_windowWidth =  averageCharWidth()*(word.length()*SUBWORD_WINDOW_EXTEND+SUBWORD_WINDOW_PAD);
    //}
    int s_stride=averageCharWidth()*STRIDE_PORTION;
    assert(alpha>=0 && alpha<=1);
    assert(word.length()>0 || alpha==1);
    assert(exemplar.rows*exemplar.cols>1 || alpha==0);
    assert (exemplar.channels()==1 || alpha==0);
    
    

    Mat matx = embedding().rndmatx;//(Rect(0,0,embedding().M,embedding().rndmatx.cols));
    Mat maty = embedding().rndmaty;//(Rect(0,0,embedding().M,embedding().rndmatx.cols));
    assert(matx.rows==embedding().M);
    assert(maty.rows==embedding().M);
    
    Mat tmp;
    Mat query_cca_att;
    Mat query_cca_phoc;
    if (alpha!=0)
    {
        Mat query_feats = extract_feats(exemplar);
        
        Mat query_att = attModels().W.t()*query_feats.t();
        tmp = matx*query_att;
        vconcat(cosMat(tmp),sinMat(tmp),tmp);
        assert(embedding().matt.size() == tmp.size());
        Mat query_emb_att = (1/sqrt(embedding().M)) * tmp - embedding().matt;
        
        
        query_cca_att = embedding().Wx.t()*query_emb_att;
        
        normalizeL2Columns(query_cca_att);
    } 
    if (alpha!=1)
    { 
        Mat query_phoc = Mat::zeros(phocSize+phocSize_bi,1,CV_32F);
        computePhoc(word, vocUni2pos, map<string,int>(),unigrams.size(), phoc_levels, phocSize, query_phoc,0);
        computePhoc(word, map<char,int>(), vocBi2pos,bigrams.size(), phoc_levels_bi, phocSize_bi, query_phoc,0);
        tmp = maty*query_phoc;
        //checkNaN(tmp);
        vconcat(cosMat(tmp),sinMat(tmp),tmp);
        //checkNaN(embedding().mphoc);
        //checkNaN(tmp);
        assert(embedding().mphoc.size() == tmp.size());
        Mat query_emb_phoc = (1/sqrt(embedding().M)) * tmp - embedding().mphoc;
        query_cca_phoc = embedding().Wy.t()*query_emb_phoc;
        //checkNaN(query_cca_phoc);
        
        
        normalizeL2Columns(query_cca_phoc);
    }
    Mat query_cca_hy;
    if (alpha!=0 && alpha!=1)
       query_cca_hy = query_cca_att*alpha + query_cca_phoc*(1-alpha);
    else if (alpha==1)
        query_cca_hy = query_cca_att;
    else if (alpha==0)
        query_cca_hy = query_cca_phoc;
    
    //checkNaN(query_cca_hy);
    
    
    //Mat s;//scores
    int numberOfInstances=-1;
    if (corpus_imgfiles!=NULL)
        numberOfInstances=corpus_imgfiles->size();
    else if (corpus_dataset!=NULL)
        numberOfInstances=corpus_dataset->size();
        
    //map <scores, pair<image,window> >
    multimap<float,pair<int,int> > scores;
    
    

    //ttt#pragma omp parallel for
    for (int i=0; i<corpus_dataset->size(); i++)
    {
        Mat s_batch = subwordWindows_cca_att_saved(i,s_windowWidth,s_stride).t()*query_cca_hy;
        assert(s_batch.rows<=corpus_dataset->image(i).cols);
        float topScoreInd=-1;
        float topScore=-999999;
        float top2ScoreInd=-1;
        float top2Score=-999999;
        for (int r=0; r<s_batch.rows; r++) {
            assert((r)*s_stride<corpus_dataset->image(i).cols);
            float s = s_batch.at<float>(r,0);
            //cout <<"im "<<i<<" x: "<<r*stride<<" score: "<<s<<endl;
            if (s>topScore)
            {
                topScore=s;
                topScoreInd=r;
            }
        }
        int diff = (s_windowWidth*.8)/s_stride;
        for (int r=0; r<s_batch.rows; r++) {
            float s = s_batch.at<float>(r,0);
            if (s>top2Score && abs(r-topScoreInd)>diff)
            {
                top2Score=s;
                top2ScoreInd=r;
            }
        }

        //ttt#pragma omp critical (subword_spot)
        {
        assert(topScoreInd!=-1);
        scores.emplace(-1*topScore, make_pair(i,topScoreInd));
        if (top2ScoreInd!=-1)
            scores.emplace(-1*top2Score, make_pair(i,top2ScoreInd));
        }
    }
    
    //Now, we will refine only the top X% of the results
    auto iter = scores.begin();
    vector< SubwordSpottingResult > finalScores;
    for (int i=0; i<scores.size()*refinePortion; i++, iter++)
    {
        //if (refineThresh!=0 && iter->first > -1*fabs(refineThresh))
        //    break;
        finalScores.push_back(refine(iter->first,iter->second.first,iter->second.second,s_windowWidth,s_stride,query_cca_hy));
    }
    

    return finalScores;
}*/
Mat EmbAttSpotter::cca(const Mat& im) const
{
    Mat tmp;
    Mat matx = _embedding->rndmatx;//(Rect(0,0,_embedding->M,_embedding->rndmatx.cols));
    Mat query_feats;
    if (makeBig)
    {
        Mat big;
        resize(im,big,Size(),2,2);
        query_feats = extract_feats(big);
    }
    else
        query_feats = extract_feats(im);
    
    Mat query_att = _attModels->W.t()*query_feats.t();
    tmp = matx*query_att;
    vconcat(cosMat(tmp),sinMat(tmp),tmp);
    assert(_embedding->matt.size() == tmp.size());
    Mat query_emb_att = (1/sqrt(_embedding->M)) * tmp - _embedding->matt;
    
    
    Mat query_cca_att = _embedding->Wx.t()*query_emb_att;
    
    normalizeL2Columns(query_cca_att);
    return query_cca_att;
}
Mat EmbAttSpotter::cca(string text) const
{
    Mat maty = _embedding->rndmaty;//(Rect(0,0,_embedding->M,_embedding->rndmatx.cols));
    assert(maty.rows==_embedding->M);

    Mat query_phoc = Mat::zeros(phocSize+phocSize_bi,1,CV_32F);
    computePhoc(text, vocUni2pos, map<string,int>(),unigrams.size(), phoc_levels, phocSize, query_phoc,0);
    computePhoc(text, map<char,int>(), vocBi2pos,bigrams.size(), phoc_levels_bi, phocSize_bi, query_phoc,0);
    Mat tmp = maty*query_phoc;
    //checkNaN(tmp);
    vconcat(cosMat(tmp),sinMat(tmp),tmp);
    //checkNaN(_embedding->mphoc);
    //checkNaN(tmp);
    assert(_embedding->mphoc.size() == tmp.size());
    Mat query_emb_phoc = (1/sqrt(_embedding->M)) * tmp - _embedding->mphoc;
    Mat query_cca_phoc = _embedding->Wy.t()*query_emb_phoc;
    //checkNaN(query_cca_phoc);
    
    
    normalizeL2Columns(query_cca_phoc);

    return query_cca_phoc;
}
vector< SubwordSpottingResult > EmbAttSpotter::subwordSpot(const vector<Mat>& exemplars, string word, float alpha, float refinePortion) const
{
    assert(alpha>=0 && alpha<=1);
    Mat query_cca_att;
    Mat query_cca_phoc;
    if (alpha!=0)
    {
        for (const Mat ex : exemplars)
        {
            assert(ex.rows*ex.cols>1);
            assert (ex.channels()==1);
            if (query_cca_att.rows>0)
                query_cca_att += cca(ex);
            else
                query_cca_att = cca(ex);
        }
        query_cca_att /=  exemplars.size();
        
    } 
    if (alpha!=1)
    { 
        assert(word.length()>0);
        query_cca_phoc = cca(word);
    }
    return subwordSpotCCA(query_cca_att,query_cca_phoc,word.length(),alpha, refinePortion);
}
vector< SubwordSpottingResult > EmbAttSpotter::subwordSpot_full(const Mat& exemplar, string word, float alpha) const
{
    assert(alpha>=0 && alpha<=1);
    assert(word.length()>0 || alpha==1);
    assert(exemplar.rows*exemplar.cols>1 || alpha==0);
    assert (exemplar.channels()==1 || alpha==0);
    
    Mat query_cca_att;
    Mat query_cca_phoc;
    if (alpha!=0)
    {
        query_cca_att = cca(exemplar);
        
    } 
    if (alpha!=1)
    { 
        query_cca_phoc = cca(word);
    }
    return subwordSpotCCA_full(query_cca_att,query_cca_phoc,alpha);
}
vector< SubwordSpottingResult > EmbAttSpotter::subwordSpot(const Mat& exemplar, string word, float alpha, float refinePortion) const
{
    assert(alpha>=0 && alpha<=1);
    assert(word.length()>0 || alpha==1);
    assert(exemplar.rows*exemplar.cols>1 || alpha==0);
    assert (exemplar.channels()==1 || alpha==0);
    
    Mat query_cca_att;
    Mat query_cca_phoc;
    if (alpha!=0)
    {
        query_cca_att = cca(exemplar);
        
    } 
    if (alpha!=1)
    { 
        query_cca_phoc = cca(word);
    }
    return subwordSpotCCA(query_cca_att,query_cca_phoc,word.length(),alpha, refinePortion);
}
vector< SubwordSpottingResult > EmbAttSpotter::subwordSpotCCA(const Mat& query_cca_att, const Mat& query_cca_phoc, int text_length, float alpha, float refinePortion) const
{
    assert (_embedding!=NULL);
    assert (_attModels!=NULL);
    assert (_averageCharWidth>0);
    //int s_windowWidth=exemplar.cols;
    //if (s_windowWidth==0)
    //{
    //int  s_windowWidth =  _averageCharWidth*(text_length*SUBWORD_WINDOW_EXTEND) + 2*subwordWindowPad;
    //}
    //int s_stride=_averageCharWidth*STRIDE_PORTION;
    
    

    
    Mat query_cca_hy;
    if (alpha!=0 && alpha!=1)
       query_cca_hy = query_cca_att*alpha + query_cca_phoc*(1-alpha);
    else if (alpha==1)
        query_cca_hy = query_cca_att;
    else if (alpha==0)
        query_cca_hy = query_cca_phoc;
    
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
        
    //map <scores, pair<image,window> >
    multimap<float,pair<int,int> > scores;
    
    

    //ttt#pragma omp parallel for
    for (int i=0; i<corpus_dataset->size(); i++)
    {
        Mat s_batch = subwordWindows_cca_att_saved(i,s_windowWidth,s_stride).t()*query_cca_hy;
        int im_width = corpus_dataset->image(i).cols;
        if (makeBig)
            im_width*=2;
        assert(s_batch.rows<=im_width);
        float topScoreInd=-1;
        float topScore=-999999;
        float top2ScoreInd=-1;
        float top2Score=-999999;
        for (int r=0; r<s_batch.rows; r++) {
            if ((r)*s_stride >= im_width) {
                cout<<"ERROR: sliding window moving out of bounds for iamge "<<i<<". Window starts at "<<(r)*s_stride<<", but image is only "<<im_width<<" wide"<<endl;
            }
            assert((r)*s_stride<im_width);
            float s = s_batch.at<float>(r,0);
            //cout <<"im "<<i<<" x: "<<r*stride<<" score: "<<s<<endl;
            if (s>topScore)
            {
                topScore=s;
                topScoreInd=r;
            }
        }
        int diff = (s_windowWidth*.8)/s_stride;
        for (int r=0; r<s_batch.rows; r++) {
            float s = s_batch.at<float>(r,0);
            if (s>top2Score && abs(r-topScoreInd)>diff)
            {
                top2Score=s;
                top2ScoreInd=r;
            }
        }

        //ttt#pragma omp critical (subword_spot)
        {
        assert(topScoreInd!=-1);
        scores.emplace(-1*topScore, make_pair(i,topScoreInd));
        if (top2ScoreInd!=-1)
            scores.emplace(-1*top2Score, make_pair(i,top2ScoreInd));
        }
    }
    
    //Now, we will refine only the top X% of the results
    auto iter = scores.begin();
    int finalSize = scores.size()*refinePortion;
    vector< SubwordSpottingResult > finalScores(finalSize);
    for (int i=0; i<finalSize; i++, iter++)
    {
        //if (refineThresh!=0 && iter->first > -1*fabs(refineThresh))
        //    break;
        finalScores.at(i) = refine(iter->first,iter->second.first,iter->second.second,s_windowWidth,s_stride,query_cca_hy);
    }
    
    if (makeBig)
    {
        for (auto& s : finalScores)
        {
            s.startX/=2;
            s.endX/=2;
        }
    }
    return finalScores;
}

vector< SubwordSpottingResult > EmbAttSpotter::subwordSpotCCA_full(const Mat& query_cca_att, const Mat& query_cca_phoc, float alpha) const
{
    assert (_embedding!=NULL);
    assert (_attModels!=NULL);
    assert (_averageCharWidth>0);
    //int s_windowWidth=exemplar.cols;
    //if (s_windowWidth==0)
    //{
    //int  s_windowWidth =  _averageCharWidth*(text_length*SUBWORD_WINDOW_EXTEND) + 2*subwordWindowPad;
    //}
    //int s_stride=_averageCharWidth*STRIDE_PORTION;
    
    

    
    Mat query_cca_hy;
    if (alpha!=0 && alpha!=1)
       query_cca_hy = query_cca_att*alpha + query_cca_phoc*(1-alpha);
    else if (alpha==1)
        query_cca_hy = query_cca_att;
    else if (alpha==0)
        query_cca_hy = query_cca_phoc;
    
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
        
    //map <scores, pair<image,window> >
    multimap<float,pair<int,int> > scores;
    
    

    //ttt#pragma omp parallel for
    for (int i=0; i<corpus_dataset->size(); i++)
    {
        Mat s_batch = subwordWindows_cca_att_saved(i,s_windowWidth,s_stride).t()*query_cca_hy;
        int im_width = corpus_dataset->image(i).cols;
        if (makeBig)
            im_width*=2;
        assert(s_batch.rows<=im_width);
        float topScoreInd=-1;
        float topScore=-999999;
        float top2ScoreInd=-1;
        float top2Score=-999999;
        for (int r=0; r<s_batch.rows; r++) {
            if ((r)*s_stride >= im_width) {
                cout<<"ERROR: sliding window moving out of bounds for iamge "<<i<<". Window starts at "<<(r)*s_stride<<", but image is only "<<im_width<<" wide"<<endl;
            }
            assert((r)*s_stride<im_width);
            float s = s_batch.at<float>(r,0);
            scores.emplace(-1*s, make_pair(i,r));
        }
    }
    
    auto iter = scores.begin();
    int finalSize = scores.size();
    vector< SubwordSpottingResult > finalScores(finalSize);
    for (int i=0; i<finalSize; i++, iter++)
    {
        int x0=iter->second.second*s_stride;
        int im_width = corpus_dataset->image(iter->second.first).cols;
        if (makeBig)
            im_width*=2;
        assert(im_width>x0);
        int x1=iter->second.second*s_stride+s_windowWidth-1;
        if (x1>=im_width)
            x1=im_width-1;
        finalScores.at(i) = SubwordSpottingResult(iter->second.first,iter->first,x0,x1);
    }
    
    if (makeBig)
    {
        for (auto& s : finalScores)
        {
            s.startX/=2;
            s.endX/=2;
        }
    }
    return finalScores;
}


SubwordSpottingResult EmbAttSpotter::refine(float score, int imIdx, int windIdx, int s_windowWidth, int s_stride, const Mat& query_cca) const
{
    //
    //Each boundary is scored moving independently at 1/2 stride, then a either the best of these or both is used.
    //The reasoning is that we want to be efficent but adapt.
    //only 5 scorings are done at maximum, 4 at a minimum.
    //The window can shift or resize.
    int x0=windIdx*s_stride;
    int im_width = corpus_dataset->image(imIdx).cols;
    if (makeBig)
        im_width*=2;
    assert(im_width>x0);
    int x1=windIdx*s_stride+s_windowWidth-1;
    if (x1>=im_width)
        x1=im_width-1;
    float bestScore = score;
    /*Shift, not used
    int bestShift=0;

    float scale=0.5;
    if (x0>(s_stride*scale))
    {
        s_mat = subword_cca_att(imIdx,x0-(s_stride*scale),x1-(s_stride*scale)).t()*query_cca;
        if (-1*s_mat.at<float>(0,0) < bestScore)
        {
            bestScore = -1*s_mat.at<float>(0,0);
            bestShift=-(s_stride*scale);
        }
    }
    if (x1<im_width-(1+ s_stride*scale) )
    {
        s_mat = subword_cca_att(imIdx,x0+(s_stride*scale),x1+(s_stride*scale)).t()*query_cca;
        if (-1*s_mat.at<float>(0,0) < bestScore)
        {
            bestScore = -1*s_mat.at<float>(0,0);
            bestShift=(s_stride*scale);
        }
    }

    x0+=bestShift;
    x1+=bestShift;*/

    float scale=0.5;
    int bestX0=x0;
    int bestX1=x1;
    float bestScore0=bestScore;
    float bestScore1=bestScore;
    Mat s_mat;
    if (x0>(s_stride*scale))
    {
        s_mat = subword_cca_att(imIdx,x0-(s_stride*scale),x1).t()*query_cca;
        if (-1*s_mat.at<float>(0,0) < bestScore0)
        {
            bestScore0 = -1*s_mat.at<float>(0,0);
            bestX0=x0-(s_stride*scale);
        }
    }
    s_mat = subword_cca_att(imIdx,x0+(s_stride*scale),x1).t()*query_cca;
    if (-1*s_mat.at<float>(0,0) < bestScore0)
    {
        bestScore0 = -1*s_mat.at<float>(0,0);
        bestX0=x0+(s_stride*scale);
    }

    if (x1<im_width-(1+ s_stride*scale) )
    {
        s_mat = subword_cca_att(imIdx,x0,x1+(s_stride*scale)).t()*query_cca;
        if (-1*s_mat.at<float>(0,0) < bestScore1)
        {
            bestScore1 = -1*s_mat.at<float>(0,0);
            bestX1=x1+(s_stride*scale);
        }
    }
    s_mat = subword_cca_att(imIdx,x0,x1-(s_stride*scale)).t()*query_cca;
    if (-1*s_mat.at<float>(0,0) < bestScore1)
    {
        bestScore1 = -1*s_mat.at<float>(0,0);
        bestX1=x1-(s_stride*scale);
    }
    int newX0, newX1;
    if (bestScore0<bestScore1)
    {
        newX0=bestX0;
        newX1=x1;
        bestScore=bestScore0;
    }
    else
    {
        newX0=x0;
        newX1=bestX1;
        bestScore=bestScore1;
    }

    if (bestX1!=x1 && bestX0!=x0)
    {
        s_mat = subword_cca_att(imIdx,bestX0,bestX1).t()*query_cca;
        if (-1*s_mat.at<float>(0,0) < bestScore)
        {
            bestScore = -1*s_mat.at<float>(0,0);
            newX0=bestX0;
            newX1=bestX1;
        }
    }

    //Mat s_mat = subword_cca_att(imIdx,x0,x1).t()*query_cca;
    //assert(s_mat.rows==1 && s_mat.cols==1);

    //This is to tighten as we favor spotting with wide windows
    newX0+=subwordWindowPad;
    newX1-=subwordWindowPad;
    return SubwordSpottingResult(imIdx,bestScore,newX0,newX1);
}

double EmbAttSpotter::getAverageCharWidth() const
{
    assert(_averageCharWidth>0);
    return _averageCharWidth;
}

double EmbAttSpotter::averageCharWidth()
{
#pragma omp critical (compute_acw)
    if (_averageCharWidth<=0)
    {
        string name = saveName+"_charWidth.dat";
        ifstream in(name);
        if (in)
        {
            //load
            in>>_averageCharWidth;
            in.close();
            cout <<"Loaded avg char width="<<_averageCharWidth<<endl;
        }
        else
        {
            if (training_dataset->size()>0);
            //double heightAvg=0;
            for (int i=0; i<training_dataset->size(); i++)
            {
                Mat gray=training_dataset->image(i);
                double labelLen = training_dataset->labels()[i].length();
                if (gray.channels()>1)
                    cvtColor(gray,gray,CV_RGB2GRAY);
                Mat bin;
                int blockSize = (1+gray.rows)/2;
                if (blockSize%2==0)
                    blockSize++;
                //if (gray.type()==CV_8UC3)
                //    cv::cvtColor(gray,gray,CV_RGB2GRAY);
                adaptiveThreshold(gray, bin, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, blockSize, 10);

                //Find the baselines
                int topBaseline;
                int botBaseline;
                {
                    int avgWhite=0;
                    int countWhite=0;
                    cv::Mat hist = cv::Mat::zeros(gray.rows,1,CV_32F);
                    map<int,int> topPixCounts, botPixCounts;
                    for (int c=0; c<gray.cols; c++)
                    {
                        int topPix=-1;
                        int lastSeen=-1;
                        for (int r=0; r<gray.rows; r++)
                        {
                            if (bin.at<unsigned char>(r,c))
                            {
                                if (topPix==-1)
                                {
                                    topPix=r;
                            }
                                lastSeen=r;
                            }
                            else
                            {
                                avgWhite+=gray.at<unsigned char>(r,c);
                                countWhite++;
                            }
                            hist.at<float>(r,0)+=gray.at<unsigned char>(r,c);
                        }
                        if (topPix!=-1)
                            topPixCounts[topPix]++;
                        if (lastSeen!=-1)
                            botPixCounts[lastSeen]++;
                    }
                    avgWhite /= countWhite;

                    int maxTop=-1;
                    int maxTopCount=-1;
                    for (auto c : topPixCounts)
                    {
                        if (c.second > maxTopCount)
                        {
                            maxTopCount=c.second;
                            maxTop=c.first;
                        }
                    }
                    int maxBot=-1;
                    int maxBotCount=-1;
                    for (auto c : botPixCounts)
                    {
                        if (c.second > maxBotCount)
                        {
                            maxBotCount=c.second;
                            maxBot=c.first;
                        }
                    }

                    //cv::Mat kernel = cv::Mat::ones( 5, 1, CV_32F )/ (float)(5);
                    //cv::filter2D(hist, hist, -1 , kernel );
                    cv::Mat edges;
                    int pad=5;
                    cv::Mat paddedHist = cv::Mat::ones(hist.rows+2*pad,1,hist.type());
                    double avg=0;
                    double maxHist=-99999;
                    double minHist=99999;
                     for (int r=0; r<hist.rows; r++)
                    {
                        avg+=hist.at<float>(r,0);
                        if (hist.at<float>(r,0)>maxHist)
                            maxHist=hist.at<float>(r,0);
                        if (hist.at<float>(r,0)<minHist)
                            minHist=hist.at<float>(r,0);
                    }
                    avg/=hist.rows;
                    paddedHist *= avg;
                    hist.copyTo(paddedHist(cv::Rect(0,pad,1,hist.rows)));
                    float kernelData[11] = {1,1,1,1,.5,0,-.5,-1,-1,-1,-1};
                    cv::Mat kernel = cv::Mat(11,1,CV_32F,kernelData);
                    cv::filter2D(paddedHist, edges, -1 , kernel );//, Point(-1,-1), 0 ,BORDER_AVERAGE);
                    float kernelData2[11] = {.1,.1,.1,.1,.1,.1,.1,.1,.1,.1,.1};
                    cv::Mat kernel2 = cv::Mat(11,1,CV_32F,kernelData2);
                    cv::Mat blurred;
                    cv::filter2D(hist, blurred, -1 , kernel2 );//, Point(-1,-1), 0 ,BORDER_AVERAGE);
                    topBaseline=-1;
                    float maxEdge=-9999999;
                    botBaseline=-1;
                    float minEdge=9999999;
                    float minPeak=9999999;
                    float center=-1;
                    for (int r=pad; r<gray.rows+pad; r++)
                    {
                        float v = edges.at<float>(r,0);
                        if (v>maxEdge)
                        {
                            maxEdge=v;
                            topBaseline=r-pad;
                        }
                        if (v<minEdge)
                        {
                            minEdge=v;
                            botBaseline=r-pad;
                        }

                        if (blurred.at<float>(r-pad,0) < minPeak) {
                            center=r-pad;
                            minPeak=blurred.at<float>(r-pad,0);
                        }
                    }
                    if (topBaseline>center)
                    {
                        if (maxTop < center)
                            topBaseline=maxTop;
                        else
                        {
                            maxEdge=-999999;
                            for (int r=pad; r<center+pad; r++)
                            {
                                float v = edges.at<float>(r,0);
                                if (v>maxEdge)
                                {
                                    maxEdge=v;
                                    topBaseline=r-pad;
                                }
                            }
                        }
                    }
                    if (botBaseline<center)
                    {
                        if (maxBot > center)
                            botBaseline=maxBot;
                        else
                        {
                            minEdge=999999;
                            for (int r=center+1; r<gray.rows+pad; r++)
                            {
                                float v = edges.at<float>(r,0);
                                if (v<minEdge)
                                {
                                    minEdge=v;
                                    botBaseline=r-pad;
                                }
                            }
                        }
                    }
                    if (botBaseline < topBaseline)//If they fail this drastically, the others won't be much better.
                    {
                        topBaseline=maxTop;
                        botBaseline=maxBot;
                    }
                }
                /*heightAvg+=botBaseline-topBaseline;
                */

                //create profile
                int avgWhite=0;
                int countWhite=0;
                cv::Mat hist = cv::Mat::zeros(1,gray.cols,CV_32F);
                map<int,int> leftPixCounts, rightPixCounts;
                for (int r=topBaseline; r<botBaseline; r++)
                {
                    int leftPix=-1;
                    int lastSeen=-1;
                    for (int c=0; c<gray.cols; c++)
                    {
                        if (bin.at<unsigned char>(r,c))
                        {
                            if (leftPix==-1)
                            {
                                leftPix=c;
                            }
                            lastSeen=c;
                        }
                        else
                        {
                            avgWhite+=gray.at<unsigned char>(r,c);
                            countWhite++;
                        }
                        hist.at<float>(0,c)+=gray.at<unsigned char>(r,c);
                    }
                    if (leftPix!=-1)
                        leftPixCounts[leftPix]++;
                    if (lastSeen!=-1)
                        rightPixCounts[lastSeen]++;
                }
                avgWhite /= countWhite;

                int maxLeft=-1;
                int maxLeftCount=-1;
                for (auto r : leftPixCounts)
                {
                    if (r.second > maxLeftCount)
                    {
                        maxLeftCount=r.second;
                        maxLeft=r.first;
                    }
                }
                int maxRight=-1;
                int maxRightCount=-1;
                for (auto r : rightPixCounts)
                {
                    if (r.second > maxRightCount)
                    {
                        maxRightCount=r.second;
                        maxRight=r.first;
                    }
                }

                //cv::Mat kernel = cv::Mat::ones( 5, 1, CV_32F )/ (float)(5);
                //cv::filter2D(hist, hist, -1 , kernel );
                cv::Mat edges;
                int pad=19;
                cv::Mat paddedHist = cv::Mat::ones(1,hist.cols+2*pad,hist.type());
                double avg=0;
                double maxHist=-99999;
                double minHist=99999;
                 for (int c=0; c<hist.cols; c++)
                {
                    avg+=hist.at<float>(0,c);
                    if (hist.at<float>(0,c)>maxHist)
                        maxHist=hist.at<float>(0,c);
                    if (hist.at<float>(0,c)<minHist)
                        minHist=hist.at<float>(0,c);
                }
                avg/=hist.cols;
                /*double stddev=0;
                for (int c=0; c<hist.cols; c++)
                {
                    stddev+=pow(hist.at<float>(0,c)-avg,2);
                }
                stddev/=sqrt(hist.cols);
                for (int c=0; c<hist.cols; c++)
                {
                    hist.at<float>(0,c) = min((float)hist.at<float>(0,c),(float)(avg+stddev));
                }*/
                //We want the padding to not effect the landscape, but we want to get the ends if the word takes up the whole BB.
                //So, we use the average. Hopefully this will only effect the landscape when
                //it is relatively uniform, which hopfully occurs in situations the words takes up the whole bounding box.
                paddedHist *= (avg);//-sqrt(var));
                hist.copyTo(paddedHist(cv::Rect(pad,0,hist.cols,1)));
                float kernelData[39] = {0.5,0.75,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,.5,0,-.5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-0.75,-0.5};
                cv::Mat kernel = cv::Mat(1,21,CV_32F,kernelData);
                cv::filter2D(paddedHist, edges, -1 , kernel );//, Point(-1,-1), 0 ,BORDER_AVERAGE);
                //float kernelData2[19] = {.1,.1,.1,.1,.1,.1,.1,.1,.1,.1,.1,.1,.1,.1,.1,.1,.1,.1,.1};
                //cv::Mat kernel2 = cv::Mat(1,19,CV_32F,kernelData2);
                //cv::Mat blurred;
                //cv::filter2D(hist, blurred, -1 , kernel2 );//, Point(-1,-1), 0 ,BORDER_AVERAGE);
                float maxEdge=-9999999;
                float minEdge=9999999;
                //float minPeak=9999999;
                //float center=-1; //This won't be the real center, but it atleast shouldn't be on the end.

                int leftBound, rightBound;
                for (int c=pad; c<gray.cols+pad; c++)
                {
                    float v = edges.at<float>(0,c);
                    if (v>maxEdge)
                    {
                        maxEdge=v;
                        leftBound=c-pad;
                    }
                    if (v<minEdge)
                    {
                        minEdge=v;
                        rightBound=c-pad;
                    }
                    //cout<<v<<"\t";

                    /*if (blurred.at<float>(0,c-pad) < minPeak) {
                        center=c-pad;
                        minPeak=blurred.at<float>(0,c-pad);
                    }*/
                }
                //cout<<endl;
                if (/*leftBound>center ||*/ leftBound>gray.cols/2 || leftBound<0)
                {
                    //cout<<"left: "<<leftBound<<endl;
                    leftBound=maxLeft;
                }
                if (/*rightBound<center ||*/ rightBound<gray.cols/2 || rightBound>=gray.cols)
                {
                    //cout<<"right: "<<rightBound<<endl;
                    rightBound=maxRight;
                }

                _averageCharWidth += (rightBound-leftBound)/labelLen;
               
                /*cout<<"min: "<<minEdge<<", max: "<<maxEdge<<endl; 
                Mat draw;
                cvtColor(gray,draw,CV_GRAY2RGB);
                for (int r=0; r<draw.rows; r++)
                {
                    draw.at<Vec3b>(r,leftBound) = Vec3b(0,0,255);
                    draw.at<Vec3b>(r,rightBound) = Vec3b(0,0,255);
                }
                for (int c=0; c<draw.cols; c++)
                {
                    draw.at<Vec3b>(topBaseline,c) = Vec3b(255,0,255);
                    draw.at<Vec3b>(botBaseline,c) = Vec3b(255,0,255);
                }

                int drawH=20;
                edges = 255*(edges-minEdge)/(maxEdge-minEdge);
                edges.convertTo(edges,CV_8U);
                for (int i=0; i<edges.cols; i++)
                    cout<<(int)edges.at<unsigned char>(0,i)<<endl;
                Mat drawHist(drawH,edges.cols,CV_8U);
                for (int i=0; i<drawH; i++)
                {
                    //drawHist.row(i)=edges.row(0);
                    edges.row(0).copyTo(drawHist.row(i));
                }
                cvtColor(drawHist,drawHist,CV_GRAY2RGB);
                for (int i=0; i<drawH; i++)
                {
                    drawHist.at<Vec3b>(i,rightBound+pad) = Vec3b(0,0,255);
                    drawHist.at<Vec3b>(i,leftBound+pad) = Vec3b(0,0,255);
                }

                hist = 255*(hist-minHist)/(maxHist-minHist);
                hist.convertTo(hist,CV_8U);
                for (int i=0; i<hist.cols; i++)
                    cout<<(int)hist.at<unsigned char>(0,i)<<endl;
                Mat drawHistO(drawH,hist.cols,CV_8U);
                for (int i=0; i<drawH; i++)
                {
                    //drawHistO.row(i)=hist.row(0);
                    hist.row(0).copyTo(drawHistO.row(i));
                }
                cvtColor(drawHistO,drawHistO,CV_GRAY2RGB);
                for (int i=0; i<drawH; i++)
                {
                    drawHistO.at<Vec3b>(i,rightBound) = Vec3b(0,0,255);
                    drawHistO.at<Vec3b>(i,leftBound) = Vec3b(0,0,255);
                }

                //if ((rightBound-leftBound)/labelLen > 90)
                //{
                cout <<"["<<i<<"] char width: "<<(rightBound-leftBound)/labelLen<<endl;
                imshow("baselines",draw);
                imshow("hist",drawHist);
                imshow("histO",drawHistO);
                waitKey();
                //}
                */
            }
            //heightAvg/=(corpus_dataset->size()+0.0);
            //_averageCharWidth=CHAR_ASPECT_RATIO*heightAvg;
            _averageCharWidth /=(training_dataset->size()+0.0);
            cout <<"Estimated avg char width="<<_averageCharWidth<<endl;

            ofstream out(name);
            out<<_averageCharWidth;
            out.close();
        }
    }
    return _averageCharWidth;
}

/*double EmbAttSpotter::compare(const Mat& im1, const Mat& im2)
{
    assert(im1.rows*im1.cols>1);
    assert (im1.channels()==1);
    assert(im2.rows*im2.cols>1);
    assert (im2.channels()==1);
    
    
    Mat query_feats1 = extract_feats(im1);
    Mat query_feats2 = extract_feats(im2);
    
    
    Mat query_att1 = attModels().W.t()*query_feats1.t();
    Mat query_att2 = attModels().W.t()*query_feats2.t();

    Mat matx = embedding().rndmatx;//(Rect(0,0,embedding().M,embedding().rndmatx.cols));
    assert(matx.rows==embedding().M);
    
    //checkNaN(query_phoc);
    //checkNaN(maty);
    
    Mat tmp = matx*query_att1;
    vconcat(cosMat(tmp),sinMat(tmp),tmp);
    assert(embedding().matt.size() == tmp.size());
    Mat query_emb_att1 = (1/sqrt(embedding().M)) * tmp - embedding().matt;
    
    Mat query_cca_att1 = embedding().Wx.t()*query_emb_att1;
    
    
    normalizeL2Columns(query_cca_att1);
    
    tmp = matx*query_att2;
    vconcat(cosMat(tmp),sinMat(tmp),tmp);
    assert(embedding().matt.size() == tmp.size());
    Mat query_emb_att2 = (1/sqrt(embedding().M)) * tmp - embedding().matt;
    
    Mat query_cca_att2 = embedding().Wx.t()*query_emb_att2;
    
    
    normalizeL2Columns(query_cca_att2);
    /*cout <<"Query: ";
    for (int r=0; r<query_cca_hy.rows; r++)
        for (int c=0; c<query_cca_hy.cols; c++)
            cout <<query_cca_hy.at<float>(r,c)<<", ";
    cout<<endl;/
    //checkNaN(query_cca_hy);
    
    
    return query_cca_att1.dot(query_cca_att2);
}*/

Mat EmbAttSpotter::extract_feats(const Mat& im)
{
    
    Mat feats_m=phow(im,&PCA_());
    if (test_mode==1)
    {
        compareToCSV(attModels().W,"test/attModels_W_test.csv",false,0.0005);
        compareToCSV(attReprTr(),"test/attReprTr_test2.csv",false,0.0005);
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
Mat EmbAttSpotter::extract_feats(const Mat& im) const
{
    assert(_PCA.eigvec.rows!=0);
    Mat feats_m=phow(im,&_PCA);
    Mat feats_FV = getImageDescriptorFV(feats_m);
    
    
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
    if (test_mode==1)
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
    if (test_mode==1)
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
            if (makeBig)
                resize(im,im,Size(),2,2);
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
    if (test_mode==1)
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
        Mat im = dataset->image(j);
        if (makeBig)
            resize(im,im,Size(),2,2);
        Mat feats=phow(im,&PCA_());
        if (feats.cols==0)
        {
            cout<<"no phow: "<<j<<endl;
            imshow("image"+to_string(j),im);
            waitKey();
        }
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
    #pragma omp  critical (feats_training)
    if (_feats_training.rows==0)
    {
        {
            //if (_feats_training.rows==0)
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


Mat EmbAttSpotter::phow(const Mat& im, const struct PCA_struct* PCA_pt, vector<int>* xs) const
{
    assert(im.channels()==1);
    int bb_x1, bb_x2, bb_y1, bb_y2;
    DoBB(im,&bb_x1,&bb_x2,&bb_y1,&bb_y2);
    int bb_w=bb_x2-bb_x1 + 1;
    int bb_h=bb_y2-bb_y1 + 1;
    int cx = bb_x1+bb_w/2;
    int cy = bb_y1+bb_h/2;

    Mat feats_m;
    //for all sizes, extract SIFT features
    
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
        int off = floor(1 + (3.0/2.0) * (maxSiftSize - size)); //MATALB, for DSIFT
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
        vl_dsift_set_bounds (dsift,off,off,imf.cols-1,imf.rows-1 );
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
        int off = floor((3.0/2.0) * maxSiftSize);
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
            double minVal;
            int test[2];
            minMaxIdx(desc.row(r),&minVal,NULL,test);
            if (summed.at<float>(r,0)>0 && kps[r].norm>=contrastthreshold && minVal>=0)
                toKeep.push_back(r);
            //if (minVal<0)
            //    cout<<"Discluded descriptor for negative value, at size:"<<size<<" row:"<<r<<"index:"<<test[1]<<endl;
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
        
        
        assert(desc.cols==SIFT_DIM);
        
        //normalize, subtract mean 
        for (unsigned int i=0; i<desc.rows; i++)
        {
            double sum=0;
            for (unsigned int j=0; j<desc.cols; j++)
            {
#if USE_VL
                desc.at<float>(i,j) =  min(desc.at<float>(i,j)*512.0,255.0)/255.0;
#else
                desc.at<float>(i,j)/=255;
#endif
                assert(desc.at<float>(i,j)>=0);
                desc.at<float>(i,j) = sqrt(desc.at<float>(i,j));
                sum += desc.at<float>(i,j)*desc.at<float>(i,j);
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
        {
            desc = (PCA_pt->eigvec.t()*desc.t());
            desc=desc.t();
        }
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
        int prevXS;
        if (xs)
        {
            prevXS=xs->size();
            xs->resize(prevXS+desc.rows);
        }
        if (desc.rows>0)
        {
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
                if (xs)
                    xs->at(prevXS+j)=kpx;   
                
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
            delete[] ims;
            #endif
            checkNaN(augmented); 
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
    }
    //for (int r=0; r<feats_m.rows; r++)
    //    for (int c=0; c<feats_m.cols; c++)
    //        assert(feats_m.at<float>(r,c)==feats_m.at<float>(r,c));
    return feats_m;
}

void EmbAttSpotter::DoBB(const Mat& im, int* bb_x1, int* bb_x2, int* bb_y1, int* bb_y2) const
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
     //for (int r=0; r<ret.rows; r++)
     //   for (int c=0; c<ret.cols; c++)
     //       assert(ret.at<float>(r,c)==ret.at<float>(r,c));
     return ret;
}
Mat EmbAttSpotter::getImageDescriptorFV(const Mat& feats_m) const
{
    if (feats_m.rows==0)
        return Mat::zeros(1,FV_DIM,CV_32F);
    assert (_GMM.means!=NULL);
    assert(feats_m.cols==AUG_PCA_DIM);
    int dimension = AUG_PCA_DIM;
    int numClusters = numGMMClusters*numSpatialX*numSpatialY;
    assert(feats_m.isContinuous());
    float* dataToEncode = (float*)feats_m.data;
    int numDataToEncode = feats_m.rows;
    Mat ret(1,FV_DIM,CV_32F);
    assert(ret.isContinuous());
    float* enc = (float*)ret.data;
    // run fisher encoding
    
    vl_fisher_encode
        (enc, VL_TYPE_FLOAT,
         _GMM.means, dimension, numClusters,
         _GMM.covariances,
         _GMM.priors,
         dataToEncode, numDataToEncode,
         VL_FISHER_FLAG_IMPROVED
         ) ;
         
     return ret;
}

const vector<Mat>& EmbAttSpotter::batches_cca_att()
{
    string name = saveName+"_batches_cca_att.dat";
    #pragma omp critical (batches_cca_att)
    if (_batches_cca_att == NULL)
    {
        //cout<<"batches_cca_att is NULL"<<endl;
        {
            //if (_batches_cca_att == NULL)
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
    #pragma omp critical (batches_cca_att)
    if (_batches_cca_att==NULL)
    {
        {
            //if (_batches_cca_att==NULL)
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
                    normalizeL2Columns(batch_cca_att);
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

Mat EmbAttSpotter::subwordWindows_cca_att_saved(int imIdx, int s_windowWidth, int s_stride)
{
    string name = saveName+"_subwordWindows_cca_att_w"+to_string(s_windowWidth)+"_s"+to_string(s_stride)+".dat";
#pragma omp critical (subwordWindows_cca_att)
    if (_subwordWindows_cca_att_saved==NULL)
    {
        
        ifstream in(name);
        if (in)
        {
            int numWordsRead;
            in >> numWordsRead;
            _subwordWindows_cca_att_saved = new vector<Mat>(numWordsRead);
            for (int i=0; i<numWordsRead; i++)
            {
                _subwordWindows_cca_att_saved->at(i) = readFloatMat(in);
            }
            in.close();
            assert(corpus_dataset->size() == numWordsRead);   
            
        }
        else
        {
            cout<<"Creating subwordWindows_cca_att_saved (w:"<<s_windowWidth<<" s:"<<s_stride<<")"<<endl;

            _subwordWindows_cca_att_saved = new vector<Mat>(corpus_dataset->size());
            for (int i=0; i<corpus_dataset->size(); i++)
            {
                _subwordWindows_cca_att_saved->at(i) = subwordWindows_cca_att(i,s_windowWidth,s_stride);
            }
            ofstream out(name);
            out << corpus_dataset->size() << " ";
            for (int i=0; i<corpus_dataset->size(); i++)
            {
                writeFloatMat(out,_subwordWindows_cca_att_saved->at(i));
            }
            out.close();
        }
    }

    return _subwordWindows_cca_att_saved->at(imIdx);
}
const Mat EmbAttSpotter::subwordWindows_cca_att_saved(int imIdx, int s_windowWidth, int s_stride) const 
{
    assert (_subwordWindows_cca_att_saved!=NULL);
    return _subwordWindows_cca_att_saved->at(imIdx);
}
/*Mat EmbAttSpotter::subwordWindows_cca_att(int imIdx, int s_windowWidth, int s_stride)
{
    Mat matx = embedding().rndmatx;
    int numWindows = max(1,(int)ceil((corpus_dataset->image(imIdx).cols-(s_windowWidth-1)+0.0)/s_stride));
    Mat window_feats=Mat_<float>(numWindows,FV_DIM);
    int windIdx=0;
    int windS=0;
    int windE=min(s_windowWidth-1,corpus_dataset->image(imIdx).cols-1);
    for (; windE<corpus_dataset->image(imIdx).cols; windS+=s_stride, windE+=s_stride, windIdx++)
    {
        
        Mat feats=phowsByX(imIdx,windS,windE);
        checkNaN(feats);
        //cout <<"w["<<windS<<", "<<windE<<"]: "<<feats.rows<<endl;
        Mat tmp= getImageDescriptorFV(feats);
        checkNaN(tmp);

        tmp.copyTo(window_feats.row(windIdx));
        
    }
    assert(windIdx==numWindows);
    //assert(sum(window_feats.row(24))[0]!=0);
    checkNaN(window_feats);
    Mat windows_att= attModels().W.t()*(window_feats.t());
    checkNaN(windows_att);
    Mat tmp = matx*windows_att;
    vconcat(cosMat(tmp),sinMat(tmp),tmp);
    Mat batch_emb_att = (1/sqrt(embedding().M)) * tmp;
    for (int c=0; c<batch_emb_att.cols; c++)
        batch_emb_att.col(c) -= embedding().matt;
    Mat ret_cca_att = embedding().Wx.t()*batch_emb_att;
    normalizeL2Columns(ret_cca_att);

    checkNaN(ret_cca_att);

    return ret_cca_att;
}*/
Mat EmbAttSpotter::subwordWindows_cca_att(int imIdx, int s_windowWidth, int s_stride) const
{
    assert(_embedding!=NULL);
    Mat matx = _embedding->rndmatx;
    int im_width = corpus_dataset->image(imIdx).cols;
    if (makeBig)
        im_width*=2;
    int numWindows = max(1,(int)ceil((im_width-(s_windowWidth-1)+0.0)/s_stride));
    Mat window_feats=Mat_<float>(numWindows,FV_DIM);
    int windIdx=0;
    int windS=0;
    int windE=min(s_windowWidth-1,im_width-1);
    for (; windE<im_width; windS+=s_stride, windE+=s_stride, windIdx++)
    {
        
        Mat feats=phowsByX(imIdx,windS,windE);
        checkNaN(feats);
        //cout <<"w["<<windS<<", "<<windE<<"]: "<<feats.rows<<endl;
        Mat tmp= getImageDescriptorFV(feats);
        checkNaN(tmp);

        tmp.copyTo(window_feats.row(windIdx));
        
    }
    assert(windIdx==numWindows);
    //assert(sum(window_feats.row(24))[0]!=0);
    /*if(numWindows!=1)
    {
        bool dif=false;
        for (int c=0; c<FV_DIM; c++)
            if (fabs(window_feats.at<float>(0,c) - window_feats.at<float>(1,c))>0.0001)
            {
                dif=true;
                break;
            }
        assert(dif);
    }*/
    checkNaN(window_feats);
    Mat windows_att= _attModels->W.t()*(window_feats.t());
    checkNaN(windows_att);
    Mat tmp = matx*windows_att;
    vconcat(cosMat(tmp),sinMat(tmp),tmp);
    Mat batch_emb_att = (1/sqrt(_embedding->M)) * tmp;
    for (int c=0; c<batch_emb_att.cols; c++)
        batch_emb_att.col(c) -= _embedding->matt;
    Mat ret_cca_att = _embedding->Wx.t()*batch_emb_att;
    normalizeL2Columns(ret_cca_att);

    checkNaN(ret_cca_att);

    return ret_cca_att;
}
/*Mat EmbAttSpotter::subword_cca_att(int imIdx, int windS, int windE)
{
    Mat matx = embedding().rndmatx;
        
    Mat feats=phowsByX(imIdx,windS,windE);
    Mat subword_feats = getImageDescriptorFV(feats);
    
    Mat windows_att= attModels().W.t()*(subword_feats.t());
    Mat tmp = matx*windows_att;
    vconcat(cosMat(tmp),sinMat(tmp),tmp);
    Mat batch_emb_att = (1/sqrt(embedding().M)) * tmp;
    for (int c=0; c<batch_emb_att.cols; c++)
        batch_emb_att.col(c) -= embedding().matt;
    Mat ret_cca_att = embedding().Wx.t()*batch_emb_att;
    normalizeL2Columns(ret_cca_att);
    return ret_cca_att;
}*/
Mat EmbAttSpotter::subword_cca_att(int imIdx, int windS, int windE) const
{
    assert(_embedding!=NULL);
    Mat matx = _embedding->rndmatx;
    Mat feats=phowsByX(imIdx,windS,windE);
    Mat subword_feats = getImageDescriptorFV(feats);
     
    Mat windows_att= _attModels->W.t()*(subword_feats.t());
    Mat tmp = matx*windows_att;
    vconcat(cosMat(tmp),sinMat(tmp),tmp);
    Mat batch_emb_att = (1/sqrt(_embedding->M)) * tmp;
    for (int c=0; c<batch_emb_att.cols; c++)
        batch_emb_att.col(c) -= _embedding->matt;
    Mat ret_cca_att = _embedding->Wx.t()*batch_emb_att;
    normalizeL2Columns(ret_cca_att);

    return ret_cca_att;
}

/*Mat EmbAttSpotter::phowsByX(int i, int xS, int xE)
{
    Mat im = corpus_dataset->image(i)(Rect(xS,0,xE-xS+1,corpus_dataset->image(i).rows)).clone();
    //imshow("window",im);
    //waitKey();
    Mat ret = phow(im,&PCA_());
    return ret;
}*/
Mat EmbAttSpotter::phowsByX(int i, int xS, int xE) const
{
    assert(_PCA.eigvec.rows!=0);
    Mat fullIm = corpus_dataset->image(i);
    if (makeBig)
        resize(fullIm,fullIm,Size(),2,2);
    assert(xS>=0 && xE<fullIm.cols);
    Mat im = fullIm(Rect(xS,0,xE-xS+1,fullIm.rows)).clone();
    Mat ret = phow(im,&_PCA);
    return ret;
}


void EmbAttSpotter::learn_attributes_bagging()
{
    if (test_mode==1)
        cout<<"learn_attributes_bagging()"<<endl;
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
    
    #pragma omp parallel for // num_threads(4)//I'm aumming I can read-only from Mats without worrying about thread stuff. If wrong, use data ptr
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
                        labelsVal[r] = phocsTr().at<float>(idxAtt,idxVal[r])!=0?1:0; //instrad of -1 as it isn't used in SVM
                    }
                    
                    VlSvm * svm=NULL;
                    Mat modelAtt = cvSVM(featsTrain,labelsTrain,featsVal,labelsVal,&svm);
                    
                    
                    
                    N++;
                    for (int idx : idxVal)
                        Np.at<float>(0,idx)+=1;
                    
                    #pragma omp critical (learn_attributes_bagging_inside)
                    {
                        _attModels->W.col(idxAtt) += modelAtt;
                        //assert(featsVal.rows==numSamples);
                        assert(vl_svm_get_dimension(svm)==featsVal.cols);
                        /*for (int r=0; r<featsVal.rows; r++)
                        {
                            float s=0;
                            for (int c=0; c<featsVal.cols; c++)
                                s += featsVal.at<float>(r,c)*((double const*)vl_svm_get_model(svm))[c];
                            _attReprTr.at<float>(idxAtt,idxVal[r])+=s;
                        }*/
                        Mat sc = modelAtt.t() * featsVal.t();
                        assert(sc.cols==featsVal.rows);
                        for (int r=0; r<featsVal.rows; r++)
                            _attReprTr.at<float>(idxAtt,idxVal[r])+=sc.at<float>(0,r);
                    }
                    vl_svm_delete(svm);
                    
                    
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
                    //divide(_attReprTr(Rect(0,idxAtt,numSamples,1)),Np,_attReprTr(Rect(0,idxAtt,numSamples,1)));
                    divide(_attReprTr.row(idxAtt),Np,_attReprTr.row(idxAtt));
                    _attModels->numPosSamples.at<float>(0,idxAtt) = ceil(numPosSamples/(double)N);
                }
            }
        }
    }
}

Mat EmbAttSpotter::cvSVM(const Mat& featsTrain, const double* labelsTrain, const Mat& featsVal, const float* labelsVal, VlSvm ** bestsvm)
{
    //Mat double_featsTrain;
    //featsTrain.convertTo(double_featsTrain, CV_64F);
    //assert(double_featsTrain.at<double>(0,0)==((double*)double_featsTrain.data)[0]);
    //assert(double_featsTrain.at<double>(0,1)==((double*)double_featsTrain.data)[1]);
    //assert(double_featsTrain.at<double>(1,0)==((double*)double_featsTrain.data)[double_featsTrain.cols]);
    //assert(double_featsTrain.isContinuous());
    assert(featsTrain.cols == featsVal.cols);
    assert(*bestsvm==NULL);
    double bestmap=0;
    double bestlambda=0;

    VlSvmDataset* dataset = vl_svmdataset_new(VL_TYPE_FLOAT,(float*)featsTrain.data, featsTrain.cols, featsTrain.rows) ;

    for (double lambda : sgdparams_lbds)
    {
        if (test_mode!=0)
            vl_rand_seed (vl_get_rand(), 0) ;
       
        VlSvm * svm = vl_svm_new_with_dataset(VlSvmSolverSdca,//stochastic dual cord ascent
                               dataset,
                               labelsTrain,
                               lambda) ;
        //VlSvm * svm = vl_svm_new(VlSvmSolverSdca,//stochastic dual cord ascent
        //                       (double*)double_featsTrain.data, featsTrain.cols, featsTrain.rows,
        //                       labelsTrain,
        //                       lambda) ;
        //VlSvm * svm = vl_svm_new(VlSvmSolverSdca,//stochastic dual cord ascent
        //                       (double*)double_featsTrain.data, featsTrain.cols, featsTrain.rows,
        //                       labelsTrain,
        //                       lambda) ;
        assert(svm!=NULL);
        vl_svm_set_loss(svm,VlSvmLossHinge);
        vl_svm_set_bias_multiplier (svm, 0.1);
        if (test_mode!=0)
            vl_rand_seed (vl_get_rand(), 0) ;
        vl_svm_train(svm) ;
        
        double cmap = modelMap(vl_svm_get_dimension(svm),vl_svm_get_model(svm),featsVal,labelsVal);
        if (cmap > bestmap || *bestsvm==NULL)
        {
            bestmap=cmap;
            bestlambda=lambda;
            if (*bestsvm!=NULL)
                vl_svm_delete(*bestsvm);
            *bestsvm=svm;
        }
        else
        {
            vl_svm_delete(svm);
        }
    }
    Mat ret(vl_svm_get_dimension(*bestsvm),1,CV_32F);
    double const* model = vl_svm_get_model(*bestsvm);
    for (int r=0; r<vl_svm_get_dimension(*bestsvm); r++)
    {
        double ddd = model[r];
        bool ttt = isfinite(ddd);
        assert(ttt);
        ret.at<float>(r,0)=model[r];
    }
    assert(*bestsvm!=NULL);
    vl_svmdataset_delete(dataset);
    return ret;
}

double EmbAttSpotter::modelMap(int model_size, double const* svm_model, const Mat& featsVal, const float* labelsVal)
{       
    assert(model_size==featsVal.cols); 
    vector< pair<int,float> > scores(featsVal.rows);
    int N=0;
    for (int r=0; r<featsVal.rows; r++)
    {
        float s=0;
        for (int c=0; c<model_size; c++) {
            s += featsVal.at<float>(r,c)*svm_model[c];
            //assert(isfinite(svm_model[r]));
            if (!isfinite(svm_model[c]))
                return -1;
        }
        scores.at(r)=make_pair(labelsVal[r],s);
        N+=labelsVal[r];
    }
    sort(scores.begin(), scores.end(),[](const pair<int,float>& lh, const pair<int,float>& rh) {return lh.second>rh.second;}); 
    vector<float> acc(scores.size());
    int accum=0;
    int place=0;
    double map=0;
    for (const pair<int,float>& score : scores)
    {
        accum += score.first;
        map += (accum*score.first)/(double)(++place);
    }
    return map/N;
}

Mat EmbAttSpotter::select_rows(const Mat& m, vector<int> idx) const
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
    #pragma omp  critical (learn_attributes_bagging)
    if (_attModels==NULL)
    {
        {
            //if (_attModels==NULL)
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
    #pragma omp  critical (embedding)
    if (_embedding==NULL)
    {
        {
            //if (_embedding==NULL)
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
    if (test_mode==1)
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
    /*if (test_mode==1)
    {
        M=200;
        Dims=50;
    }*/
    int Dx = attReprTr().rows;
    int Dy = phocsTr().rows;
    Mat rndmatx(M,Dx,CV_32F);
    Mat rndmaty(M,Dy,CV_32F);
    if (test_mode==1)//==2)
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
    if (test_mode==1)
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
    if (test_mode==1)
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
    if (test_mode==1)
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
    if (test_mode==1)
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
    cout<<"arma1: "<<M.rows<<", "<<M.cols<<endl;
    arma::mat aM(M.rows,M.cols);
    for (int r=0; r<M.rows; r++)
        for(int c=0; c<M.cols; c++)
        {
            //not symetric --assert(M.at<float>(r,c)==M.at<float>(c,r));
            aM(r,c)=M.at<float>(r,c);
        }
    arma::cx_vec cxr;
    arma::cx_mat aWx;

    cout<<"arma2: eig"<<endl;
    arma::eig_gen( cxr, aWx, aM );
    cout<<"arma3:real "<<endl;
    arma::vec r = arma::real(cxr);//      % Canonical correlations

    // --- Sort correlations ---

    cout<<"arma4: assign"<<endl;
    arma::cx_mat V = aWx; //arma::fliplr(aWx);//         % reverse order of eigenvectors
    //r = arma::flipud(r);//    % extract eigenvalues anr reverse their orrer
    cout<<"arma5: sort"<<endl;
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
    #pragma omp  critical (learn_attributes_bagging)
    if (_attReprTr.rows==0)
    {
        {
            //if (_attReprTr.rows==0)
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
        for (int i=0; i<numSpatialX*numSpatialY; i++)
        {
            bins[i] = Mat(0,SIFT_DIM+2,CV_32F);
        }
        
        #if TEST_MODE
        Mat newFor_PCA;
        if (test_mode==1)
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
        #endif
        
        
        
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
            if (makeBig)
                resize(im,im,Size(),2,2);
            used[imageIndex]=true;
            //resize to minimum height
            /*if (im.rows<minH)
            {
                double ar = im.rows/(double)im.cols;
                int newWidth=minH/ar;
                resize(im, im, Size(minH, newWidth), 0, 0, INTER_CUBIC);
            }*/
            
            if (test_mode==1)
            {
            //cout <<"image "<<imageIndex<<" size ["<<im.rows<<" "<<im.cols<<"]"<<endl;
            //cout << training_dataset->labels()[imageIndex] << endl;
            //cout <<"image "<<imageIndex+1<<" size ["<<training_dataset->image(imageIndex+1).rows<<" "<<training_dataset->image(imageIndex+1).cols<<"]"<<endl;
            //cout << training_dataset->labels()[imageIndex+1] << endl;
            //cout << "canary "<<im.at<unsigned char>(0,0)<<endl;
            }
            
            
            
            Mat desc = phow(im);//includes xy's, normalization //TODO A possible improvment, include more meta-data like scale
            if (desc.cols==0)
            {
                cout<<"Blank phow, image "<<imageIndex<<endl;
                imshow("image",im);
                waitKey();
                continue;
            }
            assert(desc.type() == CV_32F);
            assert(desc.cols == DESC_DIM);
            if (test_mode!=0)
            {   
                vector<vector<float> > csv;
                readCSV("test/GMM_PCA_descs/GMM_PCA_desc_"+to_string(i)+"_test.csv",csv);
                assert(csv.size() == desc.cols);
                cout <<"size dif "<<((int)desc.rows-(int)csv.at(0).size())<<endl;
                //assert((int)desc.rows-(int)csv.at(0).size()<500);
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
                for (int sample=0; sample<sample_per_for_PCA && on_sample<num_samples_PCA; sample++)
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
                //if (bins[xBin*numSpatialY+yBin].rows!=0)
                    bins[xBin*numSpatialY+yBin].push_back(desc.row(r));
                //else if (r!=62)
                //    bins[xBin*numSpatialY+yBin]=desc.row(r).clone();
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
        #if TEST_MODE
        if (test_mode==1)
        {
            //canot compare to matlab becuase it orders by bin
            /*if (newFor_PCA.rows != for_PCA.rows || newFor_PCA.cols != for_PCA.cols)
            {
                cout << "for_PCA size mismatch, matlab: "<<newFor_PCA.rows<<","<< newFor_PCA.cols<<" mine: "<<for_PCA.rows<<","<<for_PCA.cols<<endl;
                assert(false);
            }
            for (int r=0; r<newFor_PCA.rows; r++)
                for (int c=0; c<newFor_PCA.cols; c++)
                {
                    if (fabs(for_PCA.at<float>(r,c) - newFor_PCA.at<float>(r,c) > 0.01))
                        cout <<"sig dif at ["<<r<<","<<c<<"]: "<<for_PCA.at<float>(r,c) - newFor_PCA.at<float>(r,c)<<endl;
                    assert(for_PCA.at<float>(r,c) - newFor_PCA.at<float>(r,c) < 0.01);
                }*/
            for_PCA=newFor_PCA;
        }
        #endif
        compute_PCA(for_PCA.t(),PCA_dim);
            
        
        //GMM
        if (test_mode==1)
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
                {
                    cout <<"Desc Size dif for bin "<<i<<", mine: "<<bins[i].rows<<", "<<bins[i].cols<<",  MATLAB: "<<newBins[i].rows<<", "<<newBins[i].cols<<endl;
                    assert(false);
                }
                else
                    for (int r=0; r<csv.at(0).size(); r++)
                        for (int c=0; c<csv.size(); c++)
                        {
                            if (fabs(newBins[i].at<float>(r,c)-bins[i].at<float>(r,c))>0.001)
                            {
                                cout <<"Desc Large dif bin "<<i<<" ["<<r<<","<<c<<"] "<<newBins[i].at<float>(r,c)-bins[i].at<float>(r,c)<<endl;
                                assert(false);
                            }
                        }
            }
            
            //bins=newBins;
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
    #pragma omp  critical (get_PCA)
    if (_PCA.eigvec.rows==0)
    {
        //#pragma omp  critical (get_PCA_GMM)
        {
            if (_PCA.eigvec.rows==0)
                get_GMM_PCA(numWordsTrainGMM, saveName, retrain);
        }
    }
    
    return _PCA;
}

const EmbAttSpotter::GMM_struct & EmbAttSpotter::GMM(bool retrain)
{
    #pragma omp  critical (get_GMM)
    if (_GMM.means==NULL)
    {
        //#pragma omp  critical (get_PCA_GMM)
        {
            if (_GMM.means==NULL)
                get_GMM_PCA(numWordsTrainGMM, saveName, retrain);
        }
    }
    
    return _GMM;
}

void EmbAttSpotter::compute_PCA(const Mat& data, int PCA_dim)
{
    if (test_mode==1)
        cout<<"compute_PCA()"<<endl;
    assert(_PCA.eigvec.rows==0);
    /*PCA pt_pca(data, cv::Mat(), CV_PCA_DATA_AS_ROW, PCA_dim);
    assert(pt_pca.mean.type()==CV_32F);
    _PCA.mean = pt_pca.mean;
    
    if (test_mode==1)
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
    cout<<"arma6: "<<data.rows<<",  "<<data.cols<<endl;
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
    if (test_mode==1)
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
            d.row(r) = d.row(r) - _PCA.mean.t();
        //subtract(d,PCA_().mean,d);
        d = (_PCA.eigvec.t()*d.t()).t();//transposed to fit VL format  
        //hconcat(d,xy,d);
        hconcat(d,xy,d);
        assert(d.type() == CV_32F);
        assert(d.cols==AUG_PCA_DIM);
        assert(d.rows==bins[i].rows);
        assert(d.isContinuous());
        /*for (int r=0; r<d.rows; r++)
            for (int c=0; c<d.cols; c++)
                assert(d.at<float>(r,c)==d.at<float>(r,c));
        */
        if (test_mode==1)
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
        else if (test_mode==2)
        {
            compareToCSV(d,"test/GMM_vecs/GMM_vec_"+to_string(i)+".csv",true,0.00001);
            vl_rand_seed (vl_get_rand(), 0) ;
        }

        VlGMM* gmm = vl_gmm_new (VL_TYPE_FLOAT, AUG_PCA_DIM, numGMMClusters) ;
        vl_gmm_set_max_num_iterations (gmm, 30);
        vl_gmm_set_num_repetitions (gmm, 2);
        vl_gmm_set_initialization (gmm,VlGMMRand);
        if (test_mode==2 && (i==7 || i==6))
            vl_gmm_set_verbosity(gmm,1);
        
        assert(((float*) d.data)[1] == d.at<float>(0,1));
        assert(((float*) d.data)[d.cols] == d.at<float>(1,0));
        
        
        vl_gmm_cluster (gmm, d.data, d.rows);
        
        float* means = (float*) vl_gmm_get_means(gmm);
        copy(means,means+numGMMClusters*AUG_PCA_DIM,_GMM.means+numGMMClusters*AUG_PCA_DIM*i);
        
        float* covariances = (float*) vl_gmm_get_covariances(gmm);
        bool dif = false;
        for (int ttt=0; ttt<numGMMClusters*AUG_PCA_DIM; ttt++)
        {
            if(covariances[0]!=covariances[ttt])
            {
                dif=true;
                break;
            }
        }
        assert(dif);
        copy(covariances,covariances+numGMMClusters*AUG_PCA_DIM,_GMM.covariances+numGMMClusters*AUG_PCA_DIM*i);

        float* priors = (float*) vl_gmm_get_priors(gmm);
        copy(priors,priors+numGMMClusters,_GMM.priors+numGMMClusters*i);
        
        
        
        vl_gmm_delete(gmm);
         
         
    }


    bool dif = false;
    for (int ttt=0; ttt<AUG_PCA_DIM*numGMMClusters*numSpatialX*numSpatialY; ttt++)
    {
        if(_GMM.covariances[0]!=_GMM.covariances[ttt])
        {
            dif=true;
            break;
        }
    }
    assert(dif);

    
    //GMM.we = GMM.we/sum(GMM.we);
    float sum=0;
    for (int i=0; i<numGMMClusters*numSpatialX*numSpatialY; i++)
    {
        assert(_GMM.priors[i]==_GMM.priors[i]);
        sum += _GMM.priors[i];
    }
    for (int i=0; i<numGMMClusters*numSpatialX*numSpatialY; i++)
        _GMM.priors[i] /= sum;
    
    cout<<"finished GMM computation"<<endl;
    if (test_mode==1)
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

void EmbAttSpotter::computePhoc(string str, map<char,int> vocUni2pos, map<string,int> vocBi2pos, int Nvoc, vector<int> levels, int descSize, Mat& out, int instance) const
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

Mat EmbAttSpotter::sinMat(const Mat& x) const
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

Mat EmbAttSpotter::cosMat(const Mat& x) const
{
    //return sinMat((-1*x)+(CV_PI/2));
    assert(x.type()==CV_32F);
    Mat ret(x.rows,x.cols,CV_32F);
    for (int r=0; r<x.rows; r++)
        for (int c=0; c<x.cols; c++)
            ret.at<float>(r,c) = cos(x.at<float>(r,c));
    return ret;
}

Mat& EmbAttSpotter::normalizeL2Columns(Mat& m) const
{
    assert(m.type()==CV_32F);
    Mat tmp;
    multiply(m,m,tmp);
    reduce(tmp,tmp,0,CV_REDUCE_SUM);
    assert(tmp.rows==1);
    sqrt(tmp,tmp);
    for (int c=0; c<m.cols; c++)
    {
        if (tmp.at<float>(0,c)!=0)
            m.col(c) /=tmp.at<float>(0,c);
    }
    
    return m;
}

Mat EmbAttSpotter::otsuBinarization(const Mat& src) const
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
    //training_dataset->preprocess(pp);
    /*float sumH=0;
    for (int i=0; i<training_dataset->size(); i++)
    {
        sumH += training_dataset->image(i).rows;
    }
    if (sumH/training_dataset->size() < 50)
    {
        cout <<"average training image H: "<<sumH/training_dataset->size()<<", adding SIFT size 1"<<endl;
        SIFT_sizes.push_back(1);
    }*/
    if (averageCharWidth()<30)
    {
        //training_dataset->makeBig();
        makeBig=true;
        cout<<"Make big."<<endl;
    }
}
void EmbAttSpotter::setCorpus_dataset(const Dataset* d, bool load)
{
    corpus_dataset=d;
    //corpus_dataset->preprocess(pp);
    if (averageCharWidth()<30)
    {
        //corpus_dataset->makeBig();
        makeBig=true;
        cout<<"Make big."<<endl;
    }
    if (load) {
        GMM();
        PCA_();
        attModels();
        embedding();
        batches_cca_att();
        for (int len : spottingNgramLengths)
            primeSubwordSpotting(len);
    }
}

void EmbAttSpotter::setCorpus_dataset_fullSub(const Dataset* d, int charWidth, int windowWidth, int stride, bool load)
{
    corpus_dataset=d;
    //corpus_dataset->preprocess(pp);
    _averageCharWidth=charWidth;
    if (averageCharWidth()<30)
    {
        //corpus_dataset->makeBig();
        makeBig=true;
        cout<<"Make big."<<endl;
    }
    if (load) {
        GMM();
        PCA_();
        attModels();
        embedding();
        batches_cca_att();
        primeSubwordSpotting_set(windowWidth,stride);
    }
}
//}}}//?
