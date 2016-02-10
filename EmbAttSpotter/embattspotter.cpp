#include "embattspotter.h"


EmbAttSpotter::EmbAttSpotter(string saveName)
{
    
    _GMM.means=NULL;
    _GMM.covariances=NULL;
    _GMM.priors=NULL;
    
    SIFT_sizes=[2,4,6,8,10,12];
    stride=3;
    magnif=6.0;
    windowsize=1.5;   
    contrastthreshold=0.005; 
    
    minH = -1;//?
    PCA_dim = 62;
    num_samples_PCA = 200000;
    numGMMClusters = 16;
    numSpatialX = 6;//num pf bins for spatail pyr
    numSpatialY = 2;
    
    phoc_levels = [2 3 4 5];
    unigrams = ['a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'];
    if (useNumbers)
    {
        for (char n : "0123456789")
            unigrams.push_back(n);
    }
    bigrams = ...;//TODO
    
    this->saveName = saveName;//+"_sp"+to_string(numSpatialX)+"x"+to_string(numSpatialY);
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
    if (_features!=NULL)
    {
        _features->release();
        delete _features;
    }
    if (_feats_training!=NULL)
    {
        _feats_training.release();
        delete _feats_training;
    }
    if (_phocs_training!=NULL)
    {
        _phocs_training.release();
        delete _phocs_training;
    }
    if (_phocs!=NULL)
    {
        _phocs.release();
        delete _phocs;
    }
}

void EmbAttSpotter::loadCorpus(string dir)
{
    //TODO perform line and word segmentation and then all extract features and if we are trained, embed the word images
    
    //For now assume segmented word images
    //divide out batches
}

vector<float> EmbAttSpotter::spot(const Mat& exemplar, string word, float alpha=0.5)
{
    assert(alpha>=0 && alpha<=1);
    assert(word.length()>0 || alpha==1);
    assert(exemplar.rows*exemplar.cols>1 || alpha==0);
    
    const Embedding& embedding = get_embedding();
    
    Mat im;
    exemplar.convertTo(im, CV_32FC1);
    query_feats = extract_feats(im);
    
    query_att = attModels().W.t()*query_feats;
    
    Mat matx = embedding.rndmatx(Rect(0,0,embedding.M,embedding.rndmatx.cols));
    Mat maty = embedding.rndmaty(Rect(0,0,embedding.M,embedding.rndmatx.cols));
    
    Mat tmp = matx*query_att;
    vconcat(cos(tmp),sin(tmp),tmp);
    Mat query_emb_att = (1/sqrt(embedding.M)) * tmp - embedding.matt;
    tmp = maty*query_phoc;
    vconcat(cos(tmp),sin(tmp),tmp);
    Mat query_emb_phoc = (1/sqrt(embedding.M)) * tmp - embedding.mphoc;
    
    
    Mat query_cca_att = embedding.Wx.t()*query_emb_att;
    Mat query_cca_phoc = embedding.Wy.t()*query_emb_phoc;
    
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
        assert(s_batch.cols==1)
        copy(s_batch.data(),s_batch.data()+s_batch.rows,scores.begin()+batches_index[i]);
    }
    //return s.toVector();
    return scores;
}

Mat EmbAttSpotter::extract_feats(const Mat& im)
{
    
    Mat feats_m=phow(im,&PCA());
    
    Mat feats_FV = getImageDescriptorFV(feats_m.t());
    
    
    
    return feats_FV;
}

Mat* EmbAttSpotter::extract_FV_feats_fast(const vector<string>& imageLocations)
{
    
    //vector<Mat>* ret = new vector<Mat>(numBatches);
    Mat* ret = new Mat(corpusSize,FV_DIM);
    #pragma parallel for
    for (int i=0; i<numBatches; i++)
    {
        //ret->at(i) = Mat_<float>(batches_indexEnd[i]-batches_index[i],FV_DIM);
        Mat tmp = Mat_<float>(batches_indexEnd[i]-batches_index[i],FV_DIM);
        for (int j=batches_index[i]; j<batches_indexEnd[i]; j++)
        {
            Mat im = imread(imageLocations[j];
            Mat feats=phow(im,*PCA());
            //ret->at(i).row(j-batches_index[i]) = getImageDescriptorFV(feats.t());
            tmp.row(j-batches_index[i]) = getImageDescriptorFV(feats.t());
        }
        #pragma omp critical
        {
            Mat aux = ret->rowRange(batches_index[i],batches_indexEnd[i]);
            tmp.copyTo(aux);
        }
    }
    return ret;
}

const Mat& features()
{
    string name = saveName+"_features.dat";
    if (_features==NULL)
    {
        ifstream in(name)
        if (in && !retrain)
        {
            //load
            _features = new Mat(readFloatMat(in));
            in.close();
        }
        else
        {
            in.close();
            _features = extract_FV_feats_fast(//TODO);
            //save
            ofstream out(name);
            writeFloatMat(out,*_features);
            out.close();
        }
        
    }
    return *_features;
}

const Mat& feats_training()
{
    if (_feats_training==NULL)
    {
        _feats_training = new Mat(idxTrain.size()+idxValidation.size(),features().cols,features().type());
        int row=0;
        for (int idx : idxTrain)
        {
            features().row(idx).copyTo(_feats_training->row(row++));
        }
        for (int idx : idxValidation)
        {
            features().row(idx).copyTo(_feats_training->row(row++));
        }
    }
    return *_feats_training;
}

Mat EmbAttSPotter::phow(const Mat& im, const PCA_Object* PCA_pt)
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
        vector<KeyPoint>* keyPoints = new vector<KeyPoint>();
        for (int x=off; x<img.cols; x+=stride)
            for (int y=off; y<img.rows; y+=stride)
                keyPoints->push_back(KeyPoint(x,y,size));
        
        int nfeaturePoints=keyPoints->size();
        int nOctivesPerLayer=3;
        SIFT detector(nfeaturePoints,nOctivesPerLayer,contrastthreshold);//TODO remove by contrastthreshold and if all zero (or a neg)
        Mat desc;
        detector(ims,noArray(),*keyPoints,desc,true);
        desc.convertTo(desc, CV_32FC1)
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
        
        const Embedding& embedding = get_embedding();
        Mat matx = embedding.rndmatx(Rect(0,0,embedding.M,embedding.rndmatx.cols));
        for (int i=0; i<numBatches; i++)
        {
            //TODO load batch_att
            Mat tmp = matx*batch_att;
            vconcat(cos(tmp),sin(tmp),tmp);
            Mat batch_emb_att = (1/sqrt(embedding.M)) * tmp - embedding.matt;
            Mat batch_cca_att = embedding.Wx.t()*batch_emb_att;
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

AttributeModels* learn_attributes_bagging()
{
    int dimFeats=feats_training().cols;
    int numAtt = phoc_training().cols;
    AttributeModels* ret = new AttributeModels();
    ret->W=zeros(dimFeats,numAtt,CV_32F);
    ret->B=zeros(1,numAtt,CV_32F);
    ret->numPosSamples=zeros(1,numAtt,CV_32I);
    
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
            cout << "Model for attribute "<<idxAtt<<". Note enough data."<<endl;
            //continue
        }
        else
        {
            //TODO...
            Mat phoc_att = phoc_training()(Rect(0,idxAtt,phoc_training().rows,1));
        }
    }
}

const struct AttributesModels& EmbAttSpotter::attModels()
{
    if (_attModels==NULL)
    {
        string name = saveAs+"_attModes.dat";
        ifstream in(name)
        if (!retrain && in)
        {
            //load
            _attModels = new AttributeModels();
            _attModels->W = readFloatMat(in);
            _attModels->B = readFloatMat(in);
            _attModels->numPosSamples = readFloatMat(in);
            in.close();
        }
        else
        {
            in.close();
            _attModels = learn_attributes_bagging();
            //save
            ofstream out(name);
            writeFloatMat(out,_attModels->W);
            writeFloatMat(out,_attModels->B);
            writeFloatMat(out,_attModels->numPosSamples);
            out.close();
        }
        
    }
    return *_attModels;
}

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
        
    }
}

void train(string gtFile, string imageDir, string saveAs)
{
    
    
    int numWordsTrainGMM = ?;//TODO
    compute_GMM_PCA(numWordsTrainGMM,imageDir,saveAs);
}


//We compute the GMM and PCA together as they are relient on the same data.
void get_GMM_PCA(int numWordsTrain, string imageDir, string saveAs, bool retrain)
{   
    
    
    string name = saveAs+"_GMM_PCA_"+to_string(minH)+"_"+to_string(numGMMClusters)+"_"+to_string(PCA_dim)+".dat";
    ifstream in(name)
    if (!retrain && in)
    {
         //cout << name << " file already exists. Not retraining GMM and PCA." << endl;
         //load
        assert(_GMM.means==NULL);
        
        _PCA.mean = readFloatMat(in);
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
            writeFloatMat(out,PCA().mean);
            writeFloatMat(out,PCA().eigvec);
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

void writeFloatMat(ofstream& dst, const Mat& m)
{
    assert(m.type==CV_32F);
    dst << m.rows<<" "<<m.cols<<" ";
    for (int r=0; r<m.rows; r++)
        for (int c=0; c<m.cols; c++)
            dst << m.at<float>(r,c) << " ";
}

Mat readFloatMat(ifstream& src)
{
    int rows, cols;
    src >> rows >> cols;
    Mat ret(rows,cols,CV_32F);
    for (int r=0; r<rows; r++)
        for (int c=0; c<cols; c++)
            src >> ret.at<float>(r,c);
    return ret;
}

void writeFloatArray(ofstream& dst, const float* a, int size)
{
    dst << size<<" ";
    for (int i=0; i<size; i++)=
           dst << a[i] << " ";
}

float* readFloatArray(ifstream& src, int* sizeO)
{
    int size;
    src >> size;
    float* ret = new float[size];
    for (int i=0; r<size; i++)
        src >> ret[i]);
    if (size)!=NULL)
        *sizeO = size;
    return ret;
}

const struct PCA& PCA()
{
    if (_PCA.eigvec.rows==0)
        get_GMM_PCA(numWordsTrain, imageDir, saveAs, retrainGMM_PCA);
    
    return _PCA;
}

const struct GMM& GMM()
{
    if (_GMM.means==NULL)
        get_GMM_PCA(numWordsTrain, imageDir, saveAs, retrainGMM_PCA);
    
    return _GMM;
}

void compute_PCA(const Mat& data, int PCA_dim)
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

void compute_GMM(const Mat& bins, int numSpatialX, int numSpatialY, int numGMMClusters)
{
    assert(_GMM.means==NULL);
    _GMM.means = new float[numGMMClusters*DESC_DIM*numSpatialX*numSpatialY];
    _GMM.covariances = new float[numGMMClusters*DESC_DIM*numSpatialX*numSpatialY];
    _GMM.priors = new float[numGMMClusters*numSpatialX*numSpatialY];
    for (int i=0; i<numSpatialX*numSpatialY; i++)
    {
        Mat d = bins[xBin+yBin*numSpatialX](Rect(0,0,bins[xBin+yBin*numSpatialX].rows,SIFT_DIM));
        Mat xy = bins[xBin+yBin*numSpatialX](Rect(0,SIFdim,bins[xBin+yBin*numSpatialX].rows,2));
        Mat d -= PCA().mean;
        d = (PCA().eigvec.t()*d.t()).t();
        hconcat(d,xy,d);
        assert(d.type() == CV_32F);
        VlGMM* gmm = vl_gmm_new (VL_TYPE_FLOAT, SIFTdim+2, numGMMClusters) ;
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




Mat* embed_labels_PHOC()
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
    Mat* phocs = new Mat(corpusSize,phocSize+phocSize_bi,CV_32F);
    /* Compute */
    for (int i=0; i < corpusSize;i++)
    {
        computePhoc(corpusLabels[i], vocUni2pos, map<string,int>(),unigrams.size(), phoc_levels, phocSize, phocs,i);
    }
    
    for (int i=0; i < corpusSize;i++)
    {
        computePhoc(corpusLabels[i], map<char,int>(), vocBi2pos,bigrams.size(), phoc_levels_bi, phocSize_bi, phocs,i);
    }
    
    
    return phocs;
}

#define HARD 0

void computePhoc(string str, map<char,int> vocUni2pos, map<string,int> vocBi2pos, int Nvoc, vector<int> levels, int descSize, Mat *out, int row;)
{
    int strl = str.length;
    
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
                        out->at<float>(row,posOff)+=1;
                    }
                    #else
                    p[posOff] = max(ov, p[posOff]);
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
                        out->at<float>(row,(out->cols-descSize)+posOff)+=1;
                    }
                }
            }
            p+=Nvoc;
        }
    }
    return;
}

const Mat* phocs()
{
    if (_phocs==NULL)
    {
        _phocs = embed_labels_PHOC();
    }
    return *_phocs;
}

const Mat& phocs_training()
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
}
