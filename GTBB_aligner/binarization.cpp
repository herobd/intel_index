//A combined approach for the binarization of handwritten document images
//Ntirogiannis, Gatos, Pratikakis
//2014
Mat ntirogiannisBinarization(const Mat& src, bool visualize)
{
    Mat bg_estimation;
    bg_estimation = niblackBinarization(src,60,-0.2);
    if (visualize)
        imshow("niblack",bg_estimation);
        
    bg_estimation = dilate(bg_estimation,3);//dilate the fg, shrinking the bg
    if (visualize)
        imshow("niblack dilated",bg_estimation);
    
    Mat BG_prime;
    bg_estimation = inpainting(src,bg_estimation, &BG_prime);
    if (visualize)
        imshow("bg estimation (inpainted)",bg_estimation);
        
    Mat normalized = img_normalize(src,bg_estimation);
    if (visualize)
        imshow("normalized",normalized);
        
    Mat otsued = otsuBinarization(normalized);
    Mat first_binarization = postProcessing(otsued);
    if (visualize)
        imshow("first_binarization",first_binarization);
    
    Mat skel = skeletonization(first_binarization);
    double FG_average, FG_std;
    extract_feat(src,&skel, &FG_average, &FG_std);
    double BG_average, BG_std;
    extract_feat(BG_prime,NULL, &BG_average, &BG_std);
    //Mat fg_feat = extract_fg_feat(src)
    //Mat bg_feat = extract_bg_feat(bg_estimation?);
    double C = -50.0*log(10*(FG_average+FG_std)/(BG_average-BG_std));
    double SW = strokeWidth(first_binarization,skel);
    Mat second_binarization = niblackBinarization(normalized,2*SW,-.2-.1*floor(C/10));
    Mat dst = combine(first_binarization,second_binarization);
    
    return dst;
}

void test()
{
    Mat testImg = imread("testImages/gray.png",CV_GRAYSCALE);
    Mat res;
    niblackBinarization(testImg,res,30,-0.2);
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
    
    Mat testImg2 = (Mat_<unsigned char>(5,5) << 0,0,0,0,0,
                                                0,0,0,0,0,
                                                0,0,1,0,0,
                                                0,0,0,0,0,
                                                0,0,0,0,0);
    testImg2 = dilate(testImg2,3);
    for (int x=0; x<5; x++)
    {
        assert(testImg2.at<unsigned char>(0,x)==0);
        assert(testImg2.at<unsigned char>(4,x)==0);
        assert(testImg2.at<unsigned char>(x,0)==0);
        assert(testImg2.at<unsigned char>(x,4)==0);
    }
    for (int x=1; x<4; x++)
        for (int y=1; y<4; y++)
            assert(testImg2.at<unsigned char>(y,x)==1);
}

Mat niblackBinarization(const Mat& src, int size, double k)//inefficent
{
    Mat dst = (Mat_<unsigned char>(src.rows,src.cols));
    for (int x=0; x<src.cols; x++)
        for (int y=0; y<src.rows; y++)
        {
            double sum=0;
            for (int xOff=-floor(size/2); xOff<ceil(size/2); xOff++)
                for (int yOff=-floor(size/2); yOff<ceil(size/2); yOff++)
                {
                    int ty = y+yOff;
                    if (ty<0 || ty>=src.rows)
                        ty = y-yOff;
                    
                    int tx = x+xOff;
                    if (tx<0 || tx>=src.cols)
                        tx = x-xOff;
                        
                    sum += src.at<unsigned char>(ty,tx);
                }
            double mean = sum/(src.cols*src.rows);
            double stdDev=0;
            for (int xOff=-floor(size/2); xOff<ceil(size/2); xOff++)
                for (int yOff=-floor(size/2); yOff<ceil(size/2); yOff++)
                {
                    int ty = y+yOff;
                    if (ty<0 || ty>=src.rows)
                        ty = y-yOff;
                    
                    int tx = x+xOff;
                    if (tx<0 || tx>=src.cols)
                        tx = x-xOff;
                        
                    stdDev += pow(src.at<unsigned char>(ty,tx)-mean,2);
                }
            stdDev = sqrt(stdDev/(src.cols*src.rows));
            double thresh = mean + k*stdDev;
            if (src.at<unsigned char>(y,x) <= thresh)
                dst.at<unsigned char>(y,x) = 1;//fg
            else
                dst.at<unsigned char>(y,x) = 0;//bg
        }
    return dst;
}

Mat dilate(const Mat& src, int size)
{
    Mat dst = src.clone();
    for (int r=0; r<src.rows; r++)
        for (int c=0; c<src.cols; c++)
        {
            if (src.at<unsigned char>(r,c)==1)
            {
            
                for (int ri=-size/2; ri<=size/2; ri++)
                {
                    for (int rj=-size/2; rj<=size/2; rj++)
                    {
                        if (ri+r>=0 && ri+r<src.rows &&
                                rj+c>=0 && rj+c<src.cols)
                        {
                            src.at<unsigned char>(ri+r,rj+c)=1;
                        }
                        
                    }
                }
            }
        }
    return dst;
}

Mat inpainting(const Mat& src, const Mat& mask, Mat* prime)
{
    int x_start[4] = {0,0,mask.cols,mask.cols};
    int x_end[4] = {mask.cols,mask.cols,0,0};
    int y_start[4] = {0,mask.rows,0,mask.rows};
    int y_end[4] = {mask.rows,0,mask.rows,0};
    Mat dst = (Mat_<unsigned char>(src.rows,src.cols));
    *prime = (Mat_<unsigned char>(src.rows,src.cols));
    Mat P[4];
    Mat I = src.clone();
    for (int i=0; i<4; i++)
    {
        P[i] = (Mat_<unsigned char>(mask.rows,mask.cols));;
        Mat M = mask.clone();
        for (int y=y_start[i]; y<y_end[i]; y+=y_end[i]>y_start[i]?1:-1)
            for (int x=x_start[i]; x<x_end[i]; x+=x_end[i]>x_start[i]?1:-1)
                if (M.at<unsigned char>(y,x) == 0)
                {
                    P[i].at<unsigned char>(y,x) = (
                                                    (x-1>=0?I.at<unsigned char>(y,x-1)*M.at<unsigned char>(y,x-1):0) +
                                                    (y-1>=0?I.at<unsigned char>(y-1,x)*M.at<unsigned char>(y-1,x):0) + 
                                                    (x+1<I.cols?I.at<unsigned char>(y,x+1)*M.at<unsigned char>(y,x+1):0) +
                                                    (y+1<I.rows?I.at<unsigned char>(y+1,x)*M.at<unsigned char>(y+1,x):0)
                                                  )/4;
                    I.at<unsigned char>(y,x) = P[i].at<unsigned char>(y,x);
                    M.at<unsigned char>(y,x) = 1;
                }
    }
    for (int y=y_start[0]; y<y_end[0]; y+=y_end[0]>y_start[0]?1:-1)
            for (int x=x_start[0]; x<x_end[0]; x+=x_end[0]>x_start[i]?1:-1)
            {
                dst.at<unsigned char>(y,x) = std::min( std::min(P[0].at<unsigned char>(y,x),
                                                                P[1].at<unsigned char>(y,x))
                                                       ,
                                                       std::min(P[2].at<unsigned char>(y,x),
                                                                P[3].at<unsigned char>(y,x))
                                                     );
                prime->.at<unsigned char>(y,x) = (P[0].at<unsigned char>(y,x) + 
                                                  P[1].at<unsigned char>(y,x) + 
                                                  P[2].at<unsigned char>(y,x) +
                                                  P[3].at<unsigned char>(y,x))/4.0;
                                                     
            }
    return dst;
}

Mat img_normalize(const Mat& src, const Mat& bg)
{
    float Fmin=numeric_limits<float>::max();
    float Fmax=0;
    int Imin=numeric_limits<float>::max();
    int Imax=0;
    Mat F(src.rows,src.cols,CV_32F);
    for (int r=0; r<src.rows; r++)
        for (int c=0; c<src.cols; c++)
        {
            F.at<float>(r,c) = (src.at<unsigned char>(r,c) + 1.0) / (bg.at<unsigned char>(r,c) + 1.0);
            if (F.at<float>(r,c) > Fmax)
                Fmax = F.at<float>(r,c);
            if (F.at<float>(r,c) < Fmin)
                Fmin = F.at<float>(r,c);
            if (src.at<unsigned char>(r,c) > Imax)
                Imax = src.at<unsigned char>(r,c);
            if (src.at<unsigned char>(r,c) < Imin)
                Imin = src.at<unsigned char>(r,c);
            
        }
    Mat dst = (Mat_<unsigned char>(src.rows,src.cols));
    for (int r=0; r<src.rows; r++)
        for (int c=0; c<src.cols; c++)
        {
            dst.at<unsigned char>(r,c) = ceil( (Imax-Imin)*(F.at<float>(r,c)-Fmin)/(Fmax-Fmin) + Imin );
            
        }
    return dst
}

Mat otsuBinarization(const Mat& src)
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
    
    
    //thresh
    Mat ret(src.size,src.type);
    for (int x=0; x<ret.col; x++)
    {
        for (int y=0; y<ret.rows; y++)
        {
            if (src.at<unsigned char>(y,x) <= threshold)
                ret.at<unsigned char>(y,x)=1;
            else
                ret.at<unsigned char>(y,x)=0;
        }
    }
    return ret;
}



Mat postProcessing(const Mat& bin)
{
    //find connected components
    Mat cc = bin.clone();
    map<int,int> ccMap;
    map<int,int> ccCounts;
    map<int,int> ccMinY;
    map<int,int> ccMaxY;
    int ccIndex=1;
    int countPixels=0;
    for (int r=0; r<bin.rows; r++)
        for (int c=0; c<bin.cols; c++)
        {
            if (bin.at<unsigned char>(r,c)==1)
            {
                countPixels++:
                if ((r==0||bin.at<unsigned char>(r-1,c)==0) && (c==0||bin.at<unsigned char>(r,c-1)==0))
                {
                    cc.at<unsigned char>(r,c)=++ccIndex;
                    ccCounts[ccIndex]=1;
                    ccMap[ccIndex]=0;
                    ccMinY[ccIndex]=r;
                    ccMaxY[ccIndex]=r;
                }
                else if ((r==0||bin.at<unsigned char>(r-1,c)==0||bin.at<unsigned char>(r-1,c)==bin.at<unsigned char>(r,c-1)) && (c!=0&&bin.at<unsigned char>(r,c-1)!=0))
                {
                    int ccI=bin.at<unsigned char>(r,c-1);
                    cc.at<unsigned char>(r,c)=ccI;
                    ccCounts[ccI]++;
                    ccMaxY[ccI]=r;
                }
                else if ((r!=0&&bin.at<unsigned char>(r-1,c)!=0) && (c==0||bin.at<unsigned char>(r,c-1)==0))
                {
                    int ccI=bin.at<unsigned char>(r-1,c);
                    cc.at<unsigned char>(r,c)=ccI;
                    ccCounts[ccI]++;
                    ccMaxY[ccI]=r;
                }
                else //merge
                {
                    int toMerge = bin.at<unsigned char>(r,c-1);
                    while (ccMap[toMerge]!=0)
                        toMerge = ccMap[toMerge];
                    int mergeTo = bin.at<unsigned char>(r-1,c);
                    ccCounts[mergeTo] += ccCounts[toMerge];
                    ccMap[toMerge]=mergeTo;
                    ccMinY[mergeTo]=std::min(ccMinY[mergeTo],ccMinY[toMerge]);
                    ccMaxY[mergeTo]=std::max(ccMaxY[mergeTo],ccMaxY[toMerge]);
                    cc.at<unsigned char>(r,c)=mergeTo;
                    ccCounts[mergeTo]++;
                }
            }
        }
    int ccCount=0;
    int maxHeight=0;
    map<int,vector<int>> ccByHeight;
    for (auto ccPointer : ccMap)
    {
        if (ccPointer.second==0)
        {
            ccCount++;
            int height = ccMaxY[ccPointer.first]-ccMinY[ccPointer.first];
            if (height > maxHeight)
                maxHeight = height;
            ccByHeight[height].push_back(ccPointer.first);
        }
    }
    
    
    double sum=0;
    int h=-1;
    for (int j=0; j<max_h; j++)
    {
        double RP = 0;
        for (ccIndex : ccByHeight[j])
        {
            RP += ccCounts[ccIndex];
        }
        RP /= countPixels;
        
        double RC = ccByHeight[j].size()/(double) ccCount;
        
        sum += RP/RC;
        if (sum > 1) {
            h=j;
            break;
        }
    }
    assert(h!=-1);
    
    for (int r=0; r<cc.rows; r++)
        for (int c=0; c<cc.cols; c++)
        {
            int ccIndex=cc.at<unsigned char>(r,c);
            while (ccMap[ccIndex]!=0)
                        ccIndex = ccMap[ccIndex];
            if (ccMaxY[ccIndex]-ccMinY[ccIndex] >= h)
            {
                cc.at<unsigned char>(r,c)=1;
            }
            else
            {
                cc.at<unsigned char>(r,c)=0;
            }
        }
    return cc;
}
#define FF 3

unsigned char ToByte(bool b[8])
{
    unsigned char c = 0;
    for (int i=0; i < 8; ++i)
        if (b[i])
            c |= 1 << i;
    return c;
}
Mat LeeChenSkel(const Mat& bin)
{
    vector<char> lookupTable(16*16) << FF, FF, FF, 01, FF, 00, 01, 01, FF, FF, FF, FF, 01, 00, 01, 01,
                                       FF, FF, FF, FF, 00, FF, 00, 00, 01, FF, FF, FF, 01, 00, 01, 01,
                                       FF, FF, FF, FF, FF, FF, FF, FF, FF, FF, FF, FF, FF, FF, FF, FF,
                                       01, FF, FF, FF, 00, 00, 00, 00, 01, FF, FF, FF, 01, 00, 01, 01,
                                       FF, 00, FF, 00, FF, FF, FF, 00, FF, FF, FF, FF, FF, 00, FF, 00,
                                       00, FF, FF, 00, FF, FF, 00, FF, 00, 00, FF, 00, 00, FF, 00, FF,
                                       01, 00, FF, 00, FF, 00, FF, 00, FF, FF, FF, FF, FF, 00, FF, 00,
                                       01, 00, FF, 00, 00, FF, 00, FF, 01, 00, FF, 00, 01, FF, 01, FF,
                                       FF, 01, FF, 01, FF, 00, FF, 01, FF, FF, FF, FF, FF, 00, FF, 01,
                                       FF, FF, FF, FF, FF, 00, FF, 00, FF, FF, FF, FF, FF, 00, FF, 01,
                                       FF, FF, FF, FF, FF, FF, FF, FF, FF, FF, FF, FF, FF, FF, FF, FF,
                                       FF, FF, FF, FF, FF, 00, FF, 00, FF, FF, FF, FF, FF, 00, FF, 00,
                                       01, 01, FF, 01, FF, 00, FF, 01, FF, FF, FF, FF, FF, 00, FF, 01,
                                       00, 00, FF, 00, 00, FF, 00, FF, 00, 00, FF, 00, 00, FF, 00, FF,
                                       01, 01, FF, 01, FF, 00, FF, 01, FF, FF, FF, FF, FF, 00, FF, 00,
                                       01, 01, FF, 01, 00, FF, 00, FF, 01, 01, FF, 00, 01, FF, 00, FF;
    Mat skel = bin.clone();
    for (int r=0; r<skel.rows; r++)
        for (int c=0; c<skel.cols; c++)
        {
            bool p1 = bin.at<unsigned char>(r,c)==1;
            if (p1)
            {
                bool p2 = r>0 && bin.at<unsigned char>(r-1,c)==1;
                bool p3 = r>0 && c<bin.cols-1 && bin.at<unsigned char>(r-1,c+1)==1;
                bool p4 = c<bin.cols-1 && bin.at<unsigned char>(r,c+1)==1;
                bool p5 = r<bin.rows-1 && c<bin.cols-1 && bin.at<unsigned char>(r+1,c+1)==1;
                bool p6 = r<bin.rows-1  && bin.at<unsigned char>(r+1,c)==1;
                bool p7 = r<bin.rows-1 && c>0 && bin.at<unsigned char>(r+1,c-1)==1;
                bool p8 = c>0 && bin.at<unsigned char>(r,c-1)==1;
                bool p9 = r>0 && c>0 && bin.at<unsigned char>(r-1,c-1)==1;
                bool b[p9,p8,p7,p6,p5,p4,p3,p2];
                if (FF!=lookupTable[ToByte(b)])
                    skel.at<unsigned char>(r,c)=0;
            }
        }
}


double computeStrokeWidth(int row, int col, const Mat& bin)
{
    double minSqrDist=9999;
    Point minPoint(-1,-1);
    bool term = false;
    vector<Point> toEval;
    vector<Point> toEvalNext;
    toEval.push(col,row);
    Mat mark = bin.clone();
    do
    {
        if (minPoint.x!=-1)
            term=true;
        for (Point p : toEval)
        {
            Point u(p.x,p,y-1);
            Point d(p.x,p,y+1);
            Point l(p.x-1,p,y);
            Point r(p.x-2,p,y);
            if (bin.at<unsigned char(u)==0||bin.at<unsigned char(d)==0||bin.at<unsigned char(l)==0||bin.at<unsigned char(r)==0)
            {
                double dist = (col-p.x)*(col-p.x) + (row-p.y)*(row-p.y);
                if (minSqrDist>dist)
                {
                    minSqrDist=dist;
                    minPoint=p;
                }
            }
            else
            {
                toEvalNext.push_back(u);
                toEvalNext.push_back(d);
                toEvalNext.push_back(l);
                toEvalNext.push_back(r);
            }
        }
        toEval.clear();
        toEval.swap(toEvalNext);
    } while (!term);
    return sqrt(minSqrDist);
}
double strokeWidth(const Mat& bin, const Mat& skel)
{
    Mat cc = skel.clone();
    map<int,int> ccMap;
    map<int,int> ccCounts;
    map<int,int> ccMinY;
    map<int,int> ccMaxY;
    int ccIndex=1;
    int countPixels=0;
    for (int r=0; r<skel.rows; r++)
        for (int c=0; c<skel.cols; c++)
        {
            if (bin.at<unsigned char>(r,c)==1)
            {
                countPixels++:
                if ((r==0||bin.at<unsigned char>(r-1,c)==0) && (c==0||bin.at<unsigned char>(r,c-1)==0))
                {
                    cc.at<unsigned char>(r,c)=++ccIndex;
                    ccMap[ccIndex]=0;
                    ccMax[ccIndex]=std::max(computeStrokeWidth(r,c,bin),ccMax[ccIndex]);
                }
                else if ((r==0||bin.at<unsigned char>(r-1,c)==0||bin.at<unsigned char>(r-1,c)==bin.at<unsigned char>(r,c-1)) && (c!=0&&bin.at<unsigned char>(r,c-1)!=0))
                {
                    int ccI=bin.at<unsigned char>(r,c-1);
                    cc.at<unsigned char>(r,c)=ccI;
                    ccMax[ccI]=std::max(computeStrokeWidth(r,c,bin),ccMax[ccI]);
                }
                else if ((r!=0&&bin.at<unsigned char>(r-1,c)!=0) && (c==0||bin.at<unsigned char>(r,c-1)==0))
                {
                    int ccI=bin.at<unsigned char>(r-1,c);
                    cc.at<unsigned char>(r,c)=ccI;
                    ccMaY[ccI]=std::max(computeStrokeWidth(r,c,bin),ccMax[ccI]);
                }
                else //merge
                {
                    int toMerge = bin.at<unsigned char>(r,c-1);
                    while (ccMap[toMerge]!=0)
                        toMerge = ccMap[toMerge];
                    int mergeTo = bin.at<unsigned char>(r-1,c);
                    ccMap[toMerge]=mergeTo;
                    ccMax[mergeTo]=std::max(computeStrokeWidth(r,c,bin),std::max(ccMax[mergeTo],ccMax[toMerge]));
                    cc.at<unsigned char>(r,c)=mergeTo;
                }
            }
        }
    double sum=0;
    int ccCount=0;
    for (auto ccPointer : ccMap)
    {
        if (ccPointer.second==0)
        {
            ccCount++;
            sum += ccMax[ccPointer.first];
        }
    }
    return sum/ccCount;
}

void extract_feat(const Mat& src, const Mat* skel, double* average, double* std)
    for (int r=0; r< src.row
{
}
