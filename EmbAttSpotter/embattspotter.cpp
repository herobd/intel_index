#include "embattspotter.h"


EmbAttSpotter::EmbAttSpotter()
{
}

void EmbAttSpotter::loadCorpus(string dir)
{
    //TODO perform line and word segmentation and then all extract features and if we are trained, embed the word images
}

vector<float> EmbAttSpotter::spot(const Mat& exemplar, string word, float alpha=0.5)
{
    assert(alpha>=0 && alpha<=1);
    assert(word.length()>0 || alpha==1);
    assert(exemplar.rows*exemplar.cols>1 || alpha==0);
    
    const Embedding& embedding = get_embedding();
    const vector<Mat>& batches_cca_att = get_batches_cca_att();
    const AttributesModels& attModels = get_attModels();
    
    Mat im;
    exemplar.convertTo(im, CV_32FC1);
    query_feats = extractFeats();
    
    query_att = attModels.W.t()*query_feats;
    
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
    
    Mat s;//scores
    
    //TODO parallelize
    for (int i=0; i<numBatches; i++)
    {
        Mat batch_cca_att = batches_cca_att[i];
        Mat s_batch = query_cca_hy*batch_cca_att.t();
        s.push_back(s_batch);
    }
    return s.toVector();
}

Mat EmbAttSpotter::extractFeats(const Mat& im)
{
    vector<int> SIFT_sizes=[2,4,6,8,10,12];
    int stride=3;
    double magnif=6.0;
    double windowsize=1.5;   
    double contrastthreshold=0.005; 
    
    int maxSize=0;
    for (int size : SIFT_sizes)
    {
        if (size>maxSize)
            maxSize=size;
    }
    
    
    
    Mat feats=phow(im,*PCA());
    
    Mat feats_FV = getImageDescriptorFV(feats.t());
    
    
    
    return feats_FV;
}

Mat EmbAttSPotter::phow(const Mat& im, const PCA_Object* PCA_pt)
{
    int bb_x1, bb_x2, bb_y1, bb_y2;
    DoBB(im,&bb_x1,&bb_x2,&bb_y1,&bb_y2);
    int bb_w=bb_x2-bb_x1 + 1;
    int bb_h=bb_y2-bb_y1 + 1;
    int cx = bb_x1+w/2;
    int cy = bb_y1+h/2;

    Mat feats;
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
        feats.push_back(augmented);
    }
    return feats;
}

Mat EmbAttSpotter::getImageDescriptorFV(const Mat& feats)
{
    int dimension = feasts.cols;
    int numClusters = 
    assert(feats.isContinuous());
    float* dataToEncode = (float*)feats.data();//VL expects column major order, but we have our featvecs on the rows
    int numDataToEncode = feats.rows;
    //float* enc = vl_malloc(sizeof(float) * 2 * dimension * numClusters);
    Mat ret(dimension*2*numClusters,1);
    assert(ret.isContinuous());
    float* enc = (float*)ret.data;
    //float f2 = enc[rowIdx*ret.step1() + colIdx];
    // run fisher encoding
    vl_fisher_encode
        (enc, VL_TYPE_FLOAT,
         vl_gmm_get_means(GMM()), dimension, numClusters,
         vl_gmm_get_covariances(GMM()),
         vl_gmm_get_priors(GMM()),
         dataToEncode, numDataToEncode,
         VL_FISHER_FLAG_IMPROVED
         ) ;
     return ret;
}

const vector<Mat>& EmbAttSpotter::get_batches_cca_att()
{
    if (_batches_cca_att == NULL)
    {
        //TODO load or init
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
    }
    return *_batches_cca_att;
}

const AttributesModels& EmbAttSpotter::get_attModels()
{
    if (_attModels==NULL)
    {
        //TODO learn_attributes_bagging
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

void compute_GMM_PCA(int numWordsTrain, string imageDir, string saveAs, bool retrain)
{   
    int minH = ?;
    int PCA_dim = ?;
    int num_samples_PCA = 200000;
    string name = saveAs+"_GMM_PCA_"+minH.dat+"_"+PCA_dim+".dat";
    if (!retrain && ifstream(name))
    {
         cout << name << " file already exists. Not retraining GMM and PCA." << endl;
    }
    else
    {
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir (directory.c_str())) != NULL)
        {
            cout << "reading images and obtaining descriptors" << endl;

            vector<string> fileNames;
            while ((ent = readdir (dir)) != NULL) {
                string fileName(ent->d_name);
                if (fileName[0] == '.' || (fileName[fileName.size()-1]!='G' && fileName[fileName.size()-1]!='g' &&  fileName[fileName.size()-1]!='f'))
                    continue;
                fileNames.push_back(fileName);
            }
            
            Mat for_PCA(SIFT_dim,num_samples_PCA,?);
            int sample_per_for_PCA = num_samples_PCA/numWordsTrain;
            int on_sample=0;
            
            vector<Mat> bins(numSpatialX*numSpatialY);
            
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
                 
                Mat desc = phow(im);
                //TODO divide by max (255?)
                
                //sample for PCA, discard x,y
                for (int sample=0; sample<sample_per_for_PCA; sample++)
                {
                    int randIndex = rand()%desc.rows;
                    for_PCA.row(num_samples_PCA++).copyTo(desc(Rect(0,randIndex,SIFT_dim,1)));
                }
                
                //place in bins
                vector<Point> bins(desc.rows);
                for (int r=0; r<desc.rows; r++)
                {
                    int xBin = (desc.at<?>(r,desc.cols-2)/(double)im.cols)*numSpatialX;
                    int yBin = (desc.at<?>(r,desc.cols-1)/(double)im.rows)*numSpatialY;
                    bins[xBin+yBin*numSpatialX].push_back(desc.row(r));
                }
            }
            
            //compute PCA
            compute_PCA(for_PCA,PCA_dim);
                
            
            //GMM
            
        }
        else
        {
            cout << "Error, could not open imageDir: "<<imageDir << endl;
            assert(false);
        }
    }
}
