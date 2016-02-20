#include "embattspotter.h"

#if TEST_MODE
    #include "embattspotter_test.cpp"
#endif

EmbAttSpotter::EmbAttSpotter(string saveName)
{
    
    _GMM.means=NULL;
    _GMM.covariances=NULL;
    _GMM.priors=NULL;
    
    _embedding=NULL;
    
    SIFT_sizes=[2,4,6,8,10,12];
    stride=3;
    magnif=6.0;
    windowsize=1.5;   
    contrastthreshold=0.005; 
    
    minH = -1;//?
    PCA_dim = 62;
    num_samples_PCA = 200000;
    numGMMClusters = 16;
    numSpatialX = 6;//num of bins for spatail pyr
    numSpatialY = 2;
    
    phoc_levels = [2, 3, 4, 5];
    unigrams = ['a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'];
    if (useNumbers)
    {
        for (char n : "0123456789")
            unigrams.push_back(n);
    }
    phoc_levels_bi = [2];
    ifstream bFile("bigrams.txt");
    string bigram;
    while (getline(bFile,bigram))
    {
        bigrams.push_back(bigram);
    }
    
    this->saveName = saveName;//+"_sp"+to_string(numSpatialX)+"x"+to_string(numSpatialY);
    
    
    numBatches=-1;
    corpusSize=-1;
    corpus_imgfiles=NULL;
    training_imgfiles=NULL;
    
    
    
    
    #if TEST_MODE
        testImages = ["test/testImages/small0.png","test/testImages/small1.png","test/testImages/small2.png","test/testImages/small3.png","test/testImages/small4.png"];
    #endif
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
        _attModels->W.release();
        _attModels->M.release();
        delete _attModels;
    }
    if (_embedding!=NULL)
    {
        _embedding->rndmatx.release();
        _embedding->rndmaty.release();
        _embedding->M.release();
        _embedding->matt.release();
        _embedding->mphoc.release();
        _embedding->Wx.release();
        _embedding->Wy.release();
        delete _embedding;
    }
    if (_features_corpus!=NULL)
    {
        //_features_corpus->release();
        delete _features_corpus;
    }
    if (_feats_training!=NULL)
    {
        //_feats_training.release();
        delete _feats_training;
    }
    /*if (_phocs_training!=NULL)
    {
        _phocs_training->release();
        delete _phocs_training;
    }
    if (_phocs!=NULL)
    {
        _phocs->release();
        delete _phocs;
    }*/
    if (_phocsrT!=NULL)
    {
        _phocsTr->release();
        delete _phocsTr;
    }
}

void EmbAttSpotter::loadCorpus(string dir)
{
    //TODO perform line and word segmentation and then all extract features and if we are trained, embed the word images
    
    //For now assume segmented word images
    //
}

vector<float> EmbAttSpotter::spot(const Mat& exemplar, string word, float alpha=0.5)
{
    assert(alpha>=0 && alpha<=1);
    assert(word.length()>0 || alpha==1);
    assert(exemplar.rows*exemplar.cols>1 || alpha==0);
    
    
    
    Mat im;
    exemplar.convertTo(im, CV_32FC1);
    query_feats = extract_feats(im);
    
    query_att = attModels().W*query_feats.t();
    
    Mat matx = embedding().rndmatx(Rect(0,0,embedding().M,embedding().rndmatx.cols));
    Mat maty = embedding().rndmaty(Rect(0,0,embedding().M,embedding().rndmatx.cols));
    
    Mat tmp = matx*query_att;
    vconcat(cos(tmp),sin(tmp),tmp);
    Mat query_emb_att = (1/sqrt(embedding().M)) * tmp - embedding().matt;
    tmp = maty*query_phoc;
    vconcat(cos(tmp),sin(tmp),tmp);
    Mat query_emb_phoc = (1/sqrt(embedding().M)) * tmp - embedding().mphoc;
    
    
    Mat query_cca_att = embedding().Wx.t()*query_emb_att;
    Mat query_cca_phoc = embedding().Wy.t()*query_emb_phoc;
    
    normalizeColumns(query_cca_att);
    normalizeColumns(query_cca_phoc);
    Mat query_cca_hy = query_cca_att*alpha + query_cca_phoc*(1-alpha);
    
    //Mat s;//scores
    vector<float> scores(corpusSize);
    
    #pragma parallel for
    for (int i=0; i<numBatches; i++)
    {
        Mat s_batch = query_cca_hy*batches_cca_att()[i].t();
        //s.push_back(s_batch);
        assert(s_batch.cols==1);
        copy(s_batch.data(),s_batch.data()+s_batch.rows,scores.begin()+batches_index[i]);
    }
    //return s.toVector();
    return scores;
}

Mat EmbAttSpotter::extract_feats(const Mat& im)
{
    
    Mat feats_m=phow(im,&PCA_());
    
    Mat feats_FV = getImageDescriptorFV(feats_m.t());
    
    
    
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
    bool del=false;
    if (batches_index==NULL & batches_indexEnd==NULL)
    {
        batches_index=new vector<int>();
        batches_indexEnd=new vector<int>();
        del=true;
    }
    
    
    int numBatches = imageLocations.size()/batchSize;
    vector<Mat>* ret = new vector<Mat>(numBatches);
    
    int batchSize = imageLocations.size()/numBatches;
    batches_index->clear();
    batches_indexEnd->clear();
    
    batches_index->push_back(0);
    for (int i=1; i<numBatches; i++)
    {
        batches_indexEnd->push_back(i*batchSize);
        batches_index->push_back(i*batchSize);
    }
    batches_indexEnd->push_back(corpusSize);
    
    
    for (int i=0; i<numBatches; i++)
    {
        //ret->at(i) = Mat_<float>(batches_indexEnd[i]-batches_index[i],FV_DIM);
        Mat tmp = Mat_<float>(batches_indexEnd->at(i)-batches_index->at(i),FV_DIM);
        int start=batches_index->at(i);
        int end=batches_indexEnd->at(i);
        #pragma omp parallel for
        for (int j=start; j<end; j++)
        {
            Mat im = imread(imageLocations[j],CV_GRAYSCALE);
            Mat feats=phow(im,&PCA_());
            //ret->at(i).row(j-batches_index[i]) = getImageDescriptorFV(feats.t());
            Mat r = getImageDescriptorFV(feats.t());
            #pragma omp critical //?
            tmp.row(j-start) = r;
        }
        
        ret->at(i)=tmp;
        
    }
    
    if (del)
    {
        delete batches_index;
        delete batches_indexEnd;
    }
    return ret;
}

const vector<Mat>& EmbAttSpotter::features_corpus()
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
            assert(corpus_imgfiles!=NULL);
            _features_corpus = extract_FV_feats_fast_and_batch(*corpus_imgfiles,&batches_index,&batches_indexEnd,genericBatchSize);
            numBatches = _features_corpus->size();
            //save
            ofstream out(name);
            out << numBatches;
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


const Mat& EmbAttSpotter::feats_training()
{
    string name = saveName+"_feats_training.dat";
    if (_feats_training==NULL)
    {
        ifstream in(name);
        if (in && !retrain)
        {
            //load
            int num;
            in >> num;
            assert(num==1);
            _feats_training = new vector<Mat>(1);
            _feats_training->at(0)=readFloatMat(in);
            in.close();
        }
        else
        {
            in.close();
            assert(training_imgfiles!=NULL);
            _feats_training = extract_FV_feats_fast_and_batch(*training_imgfiles,NULL,NULL,training_imgfiles->size());
            assert(_feats_training->size()==1);
            //save
            ofstream out(name);
            out << 1;
            writeFloatMat(out,_feats_training->at(0));
            
            out.close();
        }
        
    }
    return _feats_training->at(0);
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


Mat EmbAttSpotter::phow(const Mat& im, const struct PCA* PCA_pt)
{
    int bb_x1, bb_x2, bb_y1, bb_y2;
    DoBB(im,&bb_x1,&bb_x2,&bb_y1,&bb_y2);
    int bb_w=bb_x2-bb_x1 + 1;
    int bb_h=bb_y2-bb_y1 + 1;
    int cx = bb_x1+w/2;
    int cy = bb_y1+h/2;

    Mat feats_m;
    //for all sizes, extract SIFT features
    for (int size : SIFT_sizes)
    {
        int off = floor(1 + 3/2 * (maxSize - size));
        double sigma = size/magnif;
        Mat ims; 
        GaussianBlur( im, ims, Size( 5*sigma, 5*sigma ), sigma, sigma );
        
        //describe dense points
        vector<KeyPoint> keyPoints;
        for (int x=off; x<img.cols; x+=stride)
            for (int y=off; y<img.rows; y+=stride)
                keyPoints.push_back(KeyPoint(x,y,size));
        
        int nfeaturePoints=keyPoints->size();
        int nOctivesPerLayer=3;
        SIFT detector(nfeaturePoints,nOctivesPerLayer,contrastthreshold);
        Mat desc;
        detector(ims,noArray(),keyPoints,desc,true);
        vector<int> toKeep;
        Mat summed;
        desc.convertTo(desc, CV_32FC1)
        reduce(desc,summed,1,CV_REDUCE_SUM);
        for (int r=0; r<keyPoint.size(); r++)
        {
            if (summed.at<float>(r,0)>0 && norm(desc.row(r))>=contrastthreshold)
                toKeep.push_back(r);
        }
        if (toKeep.size<keyPoint.size())
            desc = select_rows(desc,toKeep);
        desc /= 255.0;
        
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
            if (PCA_pt!=null)
                desc.row(i) -= PCA_pt->mean;
        }
        //trans with eigen vectors (desc is tranposed in relation to ALmazan's code, flip back at end)
        if (PCA_pt!=null)
            desc = (PCA_pt->eigvec.t()*desc.t()).t();
        
        
        //append x,y information
        augmented.create(desc.rows+2, desc.cols, desc.type());
        desc.copyTo(augmented(Rect(0, 0, desc.cols, desc.rows)));
        for (unsigned int j=0; j<desc.cols; j++)
        {
            augmented.at<float>(desc.rows,j) = ((*keyPoints)[j].x-cx)/w;
            augmented.at<float>(desc.rows+1,j) = ((*keyPoints)[j].y-cy)/h;
        }
        
        //vconcat(feats,augmented,feats);
        feats_m.push_back(augmented);
    }
    return feats_m;
}

Mat EmbAttSpotter::getImageDescriptorFV(const Mat& feats_m)
{
    assert(feats_m.cols==DESC_DIM);
    int dimension = DESC_DIM;
    int numClusters = numGMMClusters;
    assert(feats_m.isContinuous());
    float* dataToEncode = (float*)feats_m.data();//VL expects column major order, but we have our featvecs on the rows
    int numDataToEncode = feats_m.rows;
    //float* enc = vl_malloc(sizeof(float) * 2 * dimension * numClusters);
    Mat ret(FV_DIM,1);
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
     return ret;
}

const vector<Mat>& EmbAttSpotter::batches_cca_att()
{
    string name = saveName+"_batches_cca_att_"+to_string(numBatches)+".dat";
    if (_batches_cca_att == NULL)
    {
        //init
        _batches_cca_att = new vector<Mat>(numBatches);
        
        //load
        ifstream in(name);
        if (in)
        {
            int numBatchesRead;
            in >> numBatchesRead;
            if (numBatchesRead==numBatches)
            {
                for (int i=0; i<numBatches; i++)
                    _batches_cca_att->at(i) = readFloatMat(in);
            }
            in.close();
        }
    }
    if (_batches_cca_att->size() != numBatches)
    {
        _batches_cca_att->clear();
        
        //const Embedding& embedding = get_embedding();
        Mat matx = embedding().rndmatx(Rect(0,0,embedding().M,embedding().rndmatx.cols));
        for (int i=0; i<numBatches; i++)
        {
            // load batch_att
            Mat tmp = matx*batch_att(i);
            vconcat(cos(tmp),sin(tmp),tmp);
            Mat batch_emb_att = (1/sqrt(embedding().M)) * tmp - embedding().matt;
            Mat batch_cca_att = embedding().Wx.t()*batch_emb_att;
            _batches_cca_att->push_back(batch_cca_att);
        }
        
        //save
        ofstream out(name);
        out << numBatches << " ";
        for (int i=0; i<numBatches; i++)
            writeFloatMat(out,_batches_cca_att->at(i));
        out.close();
    }
    return *_batches_cca_att;
}

Mat EmbAttSpotter::batch_att(int batchNum)
{
    Mat m = features_corpus()[batchNum];
    return attModels().W*(m.t());
}

void EmbAttSpotter::learn_attributes_bagging()
{
    int dimFeats=feats_training().cols;
    int numAtt = phoc_training().cols;
    int numSamples = phoc_training().rows;
    _attModels =  = new AttributeModels();
    _attModels->W=zeros(dimFeats,numAtt,CV_32F);
    //_attModels->B=zeros(1,numAtt,CV_32F);
    _attModels->numPosSamples=zeros(1,numAtt,CV_32F);
    _attReprTr = new Mat(zeros(numAtt,numSamples,CV_32F)); //attFeatsTr, attFeatsBag
    
    Mat threshed;
    threshold(phoc_training(),threshed, 0.47999, 1, THRESH_BINARY);
    assert(threshed.type()==CV_8U);
    for (int idxAtt=0; idxAtt<numAtt; idxAtt++)
    {
        vector<int> idxPos;
        vector<int> idxNeg;
        for (int r=0; r<threshed.rows; r++)
        {
            if (threshed.at<unsigned char>(r,idxAtt)==1)
                idxPos.push_back(r);
            else
                idxNeg.push_back(r);
        }
        if (idxPos.size()<2 || idxNeg.size()<2)
        {
            cout << "No model for attribute "<<idxAtt<<". Note enough data."<<endl;
            //continue
        }
        else
        {
            //...
            Mat Np(numSamples,1,CV_32F);
            int N=0;
            int numPosSamples=0;
            
            for (int cPass=0; cPass<2; cPass++)
            {
                random_shuffle ( idxPos.begin(), idxPos.end() );
                random_shuffle ( idxNeg.begin(), idxNeg.end() );
                int nTrainPos = .8*idxPos.size();
                int nValPos = idxPos.size()-nTrainPos;
                int nTrainNeg = .8*idxNeg.size();
                int nValNeg = idxNeg.size()-nTrainNeg;
                
                for (int it=0; it<5; it++)
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
                    
                    float* labelsTrain[featsTrain.rows];//binary vector, 1 where idxAtt is not zero, -1 where it is zero
                    for (int r=0; r<featsTrain.rows; r++)
                    {
                        labelsTrain[r] = phocs_training().at<float>(idxTrain[r],idxAtt)!=0?1:-1;
                    }
                    float* labelsVal[featsVal.rows];//binary vector, 1 where idxAtt is not zero, -1 where it is zero
                    for (int r=0; r<featsVal.rows; r++)
                    {
                        labelsVal[r] = phocs_training().at<float>(idxVal[r],idxAtt)!=0?1:-1;
                    }
                    
                    _attModels->W += cvSVM(featsTrain,labelsTrain,featsVal,labelsVal);
                    
                    N++;
                    for (int idx : idxVal)
                        Np.at<float>(idx++,0)+=1;
                    for (int r=0; r<featsVal.rows; r++)
                    {
                        float s=0;
                        for (int c=0; c<featsVal.cols; c++)
                            s += featsVal.at<flaot>(r,c)*vl_svm_get_model(svm)[c];
                        _attReprTr->at<float>(idxAtt,r)+=s;
                    }
                }
            }
            
            if (N!=0)
            {
                _attModels->W /= (float)N;
                divide((*_attReprTr)(Rect(0,idxAtt,numSamples,1)),Np,(*_attReprTr)(Rect(0,idxAtt,numSamples,1)));
                _attModels->numPosSamples.at<float>(0,idxAtt) = ceil(numPosSamples/(double)N);
            }
        }
    }
}

Mat EmbAttSpotter::cvSVM(const Mat& featsTrain, const float* labelsTrain, const Mat& featsVal, const float* labelsVal)
{
    assert(featsTrain.isContinuous());
    assert(featsVal.isContinuous());
    double bestmap=0;
    double bestlambda=0;
    double best
    VlSvm * bestsvm=NULL;
    for (double lambda : sgdparams_lbds)
    {
        VlSvm * svm = vl_svm_new(VlSvmSolverSdca,
                               featsTrain.data, featsTrain.cols, featsTrain.rows,
                               labelsTrain,
                               lambda) ;
        vl_svm_set_bias_multiplier (svm, 0.1);
        vl_svm_train(svm) ;
        
        double cmap = modelMap(svm,featsVal,labelsVal);
        if (cmap > bestmap)
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
    Mat ret(1,featsVal.cols,CV_32F);
    for (int c=0; c<ret.cols; c++)
    {
        ret.at<float>(1,c)=vl_svm_get_model(bestsvm)[c];
    }
    delete bestsvm;
    return ret;
}

double EmbAttSpotter::modelMap(VlSvm * svm, const Mat& featsVal, const float* labelsVal)
{
    vector< pair<int,float> > scores(featsVal.rows);
    for (int r=0; r<featsVal.rows; r++)
    {
        float s=0;
        for (int c=0; c<featsVal.cols; c++)
            s += featsVal.at<flaot>(r,c)*vl_svm_get_model(svm)[c];
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
    Mat ret(idx.size(); m.cols, m.type());
    for (int i=0; i<idx.size(); i++)
    {
        m.row(idx[i]).copyTo(ret.row(i));
    }
    return ret;
}

const struct AttributesModels& EmbAttSpotter::EmbAttSpotter::attModels()
{
    if (_attModels==NULL)
    {
        string name = saveAs+"_attModels.dat";
        ifstream in(name);
        if (!retrain && in)
        {
            //load
            _attModels = new AttributeModels();
            _attModels->W = readFloatMat(in);
            _attModels->B = readFloatMat(in);
            _attModels->numPosSamples = readFloatMat(in);
            _attReprTr = new Mat();
            *_attReprTr = readFloatMat(in);
            in.close();
        }
        else
        {
            in.close();
            learn_attributes_bagging();
            //save
            ofstream out(name);
            writeFloatMat(out,_attModels->W);
            writeFloatMat(out,_attModels->B);
            writeFloatMat(out,_attModels->numPosSamples);
            writeFloatMat(out,_attReprTr);
            out.close();
        }
        
    }
    return *_attModels;
}

const struct Embedding& EmbAttSpotter::embedding()
{
    if (_embedding==NULL)
    {
        string name = saveAs+"_embedding.dat";
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
            out << _embedding->M;
            writeFloatMat(out,_embedding->matt);
            writeFloatMat(out,_embedding->mphoc);
            writeFloatMat(out,_embedding->Wx);
            writeFloatMat(out,_embedding->Wy);
            out.close();
        }
        
    }
    return *_embedding;
}

void EmbAttSpotter::learn_common_subspace()
{
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
    int Dims = 160;
    float reg = 1e-5;
    int Dx = attReprTr_lcs.rows;
    int Dy = phocsTr_lcs.rows;
    Mat rndmatx(M,Dx,CV_32F);
    Mat rndmaty(M,Dy,CV_32F);
    RNG rng(12345);
    rng.fill(rndmatx,RNG::NORMAL,Scalar(0),Scalar(1/G));
    rng.fill(rndmaty,RNG::NORMAL,Scalar(0),Scalar(1/G));
    
    Mat tmp = rndmatx*attReprTr();
    vconcat(cos(tmp),sin(tmp),tmp);
    Mat attReprTr_emb = 1/sqrt(M) * tmp;
    tmp = rndmaty*phocsTr();
    vconcat(cos(tmp),sin(tmp),tmp);
    Mat phocsTr_emb = 1/sqrt(bestM) * [ cos(tmp); sin(tmp)];

    // Mean center
    Mat ma;// = mean(attReprTr_emb,2);
    reduce(attReprTr_emb, ma, 1, CV_REDUCE_AVG);
    for (int c = 0; c < attReprTr_emb.cols; ++c) {
        attReprTr_emb.col(c) = attReprTr_emb.col(r) - ma;
    }

    Mat mh;// = mean(attReprTr_emb,2);
    reduce(phocsTr_emb, mh, 1, CV_REDUCE_AVG);
    for (int c = 0; c < phocsTr_emb.cols; ++c) {
        phocsTr_emb.col(c) = phocsTr_emb.col(r) - mh;
    }

    // Learn CCA
    Mat Wx, Wy;
    cca2(attReprTr_emb.t(), phocsTr_emb.t(),reg,K,Wx,Wy);
    
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


    Mat Cxx = X.t()*X / N + reg*eye(Dx);
    Mat Cyy = Y.t()*Y/N + reg*eye(Dy);
    Mat Cxy = X.t()*Y / N;
    Mat Cyx = Cxy.t();

    // --- Calcualte Wx and r ---
    Mat tmp;
    solve(Cxx,Cxy,tmp);
    Mat M =  (tmp/Cyy)*Cyx;
    //[Wx,r] = eigs(double(M),d); // Basis in X
    Mat r;
    eigen(M, r, Wx);
    assert(r.rows >= d);
    //r = r(Rect(0,0,d,1)).t();//only use top d
    Wx = Wx(Rect(0,0,d,Wx.cols)).t();
    //r = sqrt(r);      // Canonical correlations

    // already sorted

    // --- Calcualte Wy  ---
    solve(Cyy,Cyx,tmp);
    Wy = (tmp)*Wx;     // Basis in Y
    normalizeColumns(Wy);
}

const Mat& EmbAttSpotter::attReprTr()//correct orientation
{
    if (_attReprTr==NULL)
    {
        string name = saveAs+"_attModels.dat";
        ifstream in(name);
        if (!retrain && in)
        {
            //load
            _attModels = new AttributeModels();
            _attModels->W = readFloatMat(in);
            //_attModels->B = readFloatMat(in);
            _attModels->numPosSamples = readFloatMat(in);
            _attReprTr = new Mat();
            *_attReprTr = readFloatMat(in);;
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
    return *_attReprTr;
}

/*const Mat& EmbAttSpotter::attReprVa()
{
    if (_attReprVa==NULL)
    {
        string name = saveAs+"_attReprVa.dat";
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

vector<struct spotting_sw> EmbAttSpotter::spot_sw(const Mat& exemplar, string ngram, float alpha=0.5)
{
    
}

Mat EmbAttSpotter::scores(const Mat& query, const Mat& corpus_batch)
{
    return queries*dataset.t();
}

Mat EmbAttSpotter::scores_sw(const Mat& query, const vector<Mat>& corpus_batch)
{
    for (const Mat& inst : corpus_batch)
    {
        //TODO
    }
}

void EmbAttSpotter::train(string gtFile, string imageDir, string saveAs)
{
    
    
    int numWordsTrainGMM = 0;//TODO
    compute_GMM_PCA(numWordsTrainGMM,imageDir,saveAs);
}


//We compute the GMM and PCA together as they are relient on the same data.
void EmbAttSpotter::get_GMM_PCA(int numWordsTrain, string imageDir, string saveAs, bool retrain)
{   
    
    
    string name = saveAs+"_GMM_PCA_"+to_string(minH)+"_"+to_string(numGMMClusters)+"_"+to_string(PCA_dim)+".dat";
    ifstream in(name);
    if (!retrain && in)
    {
         //cout << name << " file already exists. Not retraining GMM and PCA." << endl;
         //load
        assert(_GMM.means==NULL);
        
        in >> _PCA.mean;// = readFloatMat(in);
        _PCA.eigvec = readFloatMat(in);
        int size;
        _GMM.means = readFloatArray(in,&size);
        numGMMClusters = size/DESC_DIM;
        _GMM.covariances = readFloatArray(in,&size);
        assert(numGMMClusters==size/DESC_DIM);
        _GMM.priors = readFloatArray(in,&size);
        assert(numGMMClusters==size);
        in.close();
    }
    else
    {
        if (in)
            in.close();
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir (directory.c_str())) != NULL)
        {
            //cout << "reading images and obtaining descriptors" << endl;
            
            //get all filenames
            vector<string> fileNames;
            while ((ent = readdir (dir)) != NULL) {
                string fileName(ent->d_name);
                if (fileName[0] == '.' || (fileName[fileName.size()-1]!='G' && fileName[fileName.size()-1]!='g' &&  fileName[fileName.size()-1]!='f'))
                    continue;
                fileNames.push_back(fileName);
            }
            
            Mat for_PCA(SIFT_DIM,num_samples_PCA,CV_32F);
            int sample_per_for_PCA = num_samples_PCA/numWordsTrain;
            int on_sample=0;
            
            vector<Mat> bins(numSpatialX*numSpatialY);
            for (int i=0; i<numSpatialX*numSpatialY; i++)
            {
                bins[xBin+yBin*numSpatialX] = zeros(0,SIFT_DIM+2,descType)
            }
            
            for (int i=0; i<numWordsTrain; i++)
            {
                int imageIndex = rand()%fileNames.size();
                Mat im = readim(imageIndex,CV_GRAYSCALE);
                //resize to minimum height
                if (im.rows<minH)
                {
                    double ar = im.rows/(double)im.cols;
                    int newWidth=minH/ar;
                    resize(im, im, Size(minH, newWidth), 0, 0, INTER_CUBIC);
                }
                 
                Mat desc = phow(im);//includes xy's, normalization
                
                
                //sample for PCA, discard x,y
                for (int sample=0; sample<sample_per_for_PCA; sample++)
                {
                    int randIndex = rand()%desc.rows;
                    for_PCA.row(num_samples_PCA++).copyTo(desc(Rect(0,randIndex,SIFT_DIM,1)));
                }
                
                //place in bins
                for (int r=0; r<desc.rows; r++)
                {
                    int xBin = (desc.at<descType>(r,desc.cols-2)/(double)im.cols)*numSpatialX;
                    int yBin = (desc.at<descType>(r,desc.cols-1)/(double)im.rows)*numSpatialY;
                    bins[xBin+yBin*numSpatialX].push_back(desc.row(r));
                }
            }
            
            //compute PCA
            compute_PCA(for_PCA,PCA_dim);
                
            
            //GMM
            compute_GMM(bins,numSpatialX,numSpatialY,numGMMClusters)
            
            
            //save
            ofstream out;
            out.open(name);
            out<<PCA_().mean;
            writeFloatMat(out,PCA_().eigvec);
            writeFloatArray(out,GMM().means,numGMMClusters*DESC_DIM*numSpatialX*numSpatialY);
            writeFloatArray(out,GMM().covariances,numGMMClusters*DESC_DIM*numSpatialX*numSpatialY);
            writeFloatArray(out,GMM().priors,numGMMClusters*numSpatialX*numSpatialY);
            out.close();
        }
        else
        {
            cout << "Error, could not open imageDir: "<<imageDir << endl;
            assert(false);
        }
    }
    
}

void EmbAttSpotter::writeFloatMat(ofstream& dst, const Mat& m)
{
    assert(m.type==CV_32F);
    dst << m.rows<<" "<<m.cols<<" ";
    for (int r=0; r<m.rows; r++)
        for (int c=0; c<m.cols; c++)
            dst << m.at<float>(r,c) << " ";
}

Mat EmbAttSpotter::readFloatMat(ifstream& src)
{
    int rows, cols;
    src >> rows >> cols;
    Mat ret(rows,cols,CV_32F);
    for (int r=0; r<rows; r++)
        for (int c=0; c<cols; c++)
            src >> ret.at<float>(r,c);
    return ret;
}

void EmbAttSpotter::writeFloatArray(ofstream& dst, const float* a, int size)
{
    dst << size<<" ";
    for (int i=0; i<size; i++)=
           dst << a[i] << " ";
}

float* EmbAttSpotter::readFloatArray(ifstream& src, int* sizeO)
{
    int size;
    src >> size;
    float* ret = new float[size];
    double tmp;
    for (int i=0; r<size; i++)
    {
        src >> tmp;
        ret[i]=tmp;
    }
    if (sizeO!=NULL)
        *sizeO = size;
    return ret;
}

const struct PCA_struct & EmbAttSpotter::PCA_()
{
    if (_PCA.eigvec.rows==0)
        get_GMM_PCA(numWordsTrain, imageDir, saveAs, retrainGMM_PCA);
    
    return _PCA;
}

const struct GMM_struct & EmbAttSpotter::GMM()
{
    if (_GMM.means==NULL)
        get_GMM_PCA(numWordsTrain, imageDir, saveAs, retrainGMM_PCA);
    
    return _GMM;
}

void EmbAttSpotter::compute_PCA(const Mat& data, int PCA_dim)
{
    assert(_PCA.eigvec.rows==0);
    //Mat mean;
    //reduce(data,mean, 1, CV_REDUCE_AVG);
    //assert(mean.cols==SIFT_DIM);
    //Mat covar;
    //calcCovarMatrix(data, covar, mean, cv::COVAR_ROWS | CV_COVAR_NORMAL );
    PCA pt_pca(data, cv::Mat(), CV_PCA_DATA_AS_ROW, 0);//? I'm unsure if this is the correct usage
    _PCA.mean = pt_pca.mean;
    assert(mean.rows==1);
    Mat eig_vals = pt_pca.eigenvalues;
    Mat eig_vecs = pt_pca.eigenvectors;
    
    Mat idxs;
    sortIdx(eig_vals, idxs, CV_SORT_EVERY_ROW + CV_SORT_DESCENDING);
    _PCA.eigvec = eig_vals(Rect(0,0,PCA_dim,eig_vecs.cols));//Mat(PCA_dim,eig_vecs.cols,eig_vals.type());
    
}

void EmbAttSpotter::compute_GMM(const Mat& bins, int numSpatialX, int numSpatialY, int numGMMClusters)
{
    assert(_GMM.means==NULL);
    _GMM.means = new float[numGMMClusters*DESC_DIM*numSpatialX*numSpatialY];
    _GMM.covariances = new float[numGMMClusters*DESC_DIM*numSpatialX*numSpatialY];
    _GMM.priors = new float[numGMMClusters*numSpatialX*numSpatialY];
    for (int i=0; i<numSpatialX*numSpatialY; i++)
    {
        int xBin = i%numSpatialX;
        int yBin = i/numSpatialX;
        Mat d = bins[xBin+yBin*numSpatialX](Rect(0,0,bins[xBin+yBin*numSpatialX].rows,SIFT_DIM));
        Mat xy = bins[xBin+yBin*numSpatialX](Rect(0,SIFdim,bins[xBin+yBin*numSpatialX].rows,2));
        Mat d = d.minus(PCA_().mean);
        d = (PCA_().eigvec.t()*d.t()).t();
        hconcat(d,xy,d);
        assert(d.type() == CV_32F);
        VlGMM* gmm = vl_gmm_new (VL_TYPE_FLOAT, DESC_DIM, numGMMClusters) ;
        vl_gmm_set_max_num_iterations (gmm, 30);
        vl_gmm_set_initialization (gmm,VlGMMRand);
        vl_gmm_cluster (gmm, d, d.rows);
        
        float* means = vl_gmm_get_means(gmm)
        copy(means,means+numGMMClusters*DESC_DIM,_GMM.means+numGMMClusters*i);
        
        float* covariances = vl_gmm_get_covariances(gmm);
        copy(covariances,covariances+numGMMClusters*DESC_DIM,_GMM.covariances+numGMMClusters*i);

        float* priors = vl_gmm_get_priors(gmm);
        copy(priors,priors+numGMMClusters,_GMM.priors+numGMMClusters*i);
    }
}




Mat* EmbAttSpotter::embed_labels_PHOC(vector<string> labels)
{
    /* Prepare dict */
    map<char,int> vocUni2pos;
    for (int i=0; i<unigrams.size(); i++)
    {
        vocUni2pos[unigrams[i]] = i;
    }
    map<std::string,int> vocBi2pos;
    for (int i=0; i<bigrams.size(); i++)
    {
        vocBi2pos[bigrams[i]] = i;
    }
    
    
    int totalLevels = 0;
    for (int level : phoc_levels)
    {
        totalLevels+=level;
    }
    
    int phocSize = totalLevels*unigrams.size();
    
    int phocSize_bi=0;
    for (int level : phoc_levels_bi)
    {
        phocSize_bi+=level*bigrams.size();
    }
    
    /* Prepare output */
    //float *phocs = new float[phocSize*corpusSize+phocSize_bi*corpusSize];
    Mat* phocs = new Mat(phocSize+phocSize_bi,,labels.size()CV_32F);
    /* Compute */
    for (int i=0; i < labels.size();i++)
    {
        computePhoc(labels[i], vocUni2pos, map<string,int>(),unigrams.size(), phoc_levels, phocSize, phocs,i);
    }
    
    for (int i=0; i < corpusSize;i++)
    {
        computePhoc(labels[i], map<char,int>(), vocBi2pos,bigrams.size(), phoc_levels_bi, phocSize_bi, phocs,i);
    }
    
    
    return phocs;
}

#define HARD 0

void EmbAttSpotter::computePhoc(string str, map<char,int> vocUni2pos, map<string,int> vocBi2pos, int Nvoc, vector<int> levels, int descSize, Mat *out, int instance)
{
    int strl = str.length();
    
    int doUnigrams = vocUni2pos.size()!=0;
    int doBigrams = vocBi2pos.size()!=0;
    
    /* For each block */
    //float *p = out;
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
                    int posOff = vocUni2pos[str[c]];
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
                        out->at<float>(posOff,instance)+=1;
                    }
                    #else
                    //p[posOff] = max(ov, p[posOff]);
                    out->at<float>(posOff,instance)=max(ov, out->at<float>(posOff,instance));
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
                    int posOff = vocBi2pos[sstr];
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
                        out->at<float>((out->rows-descSize)+posOff,instance)+=1;
                    }
                }
            }
            //p+=Nvoc;
        }
    }
    return;
}

const Mat* EmbAttSpotter::phocsTr()//correct orientation
{
    if (_phocsTr==NULL)
    {
        _phocsTr = embed_labels_PHOC(training_labels);
    }
    return *_phocsTr;
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
