#include "binarization.h"

#include <iostream>

//A combined approach for the binarization of handwritten document images
//Ntirogiannis, Gatos, Pratikakis
//2014
Mat Binarization::ntirogiannisBinarization(const Mat& src, int off, int on, bool visualize)
{
        
    Mat initBin = niblackBinarization(src,60,-0.2);
    if (visualize)
        {imshowB("niblack",initBin,off,on); waitKey();}
        
    Mat mask = dilate(initBin,3);//dilate the fg, shrinking the bg
    if (visualize)
        {imshowB("niblack dilated",mask,off,on); waitKey();}
    
    for (int r=0; r<mask.rows; r++)
        for (int c=0; c<mask.cols; c++)
        {
            if (mask.at<unsigned char>(r,c)==1)
                mask.at<unsigned char>(r,c)=0;
            else
                mask.at<unsigned char>(r,c)=1;
        }
    
    Mat BG_prime;
    Mat bg_estimation = inpainting(src,mask, &BG_prime);
    if (visualize)
        {imshow("bg estimation (inpainted)",bg_estimation); waitKey();}
        
    Mat normalized = img_normalize(src,bg_estimation);
    if (visualize)
        {imshow("normalized",normalized); waitKey();}//here
        
    Mat otsued = otsuBinarization(normalized);
    if (visualize)
        {imshowB("otsued",otsued,off,on); waitKey();}//here
    Mat first_binarization = postProcessing(otsued);
    if (visualize)
        {imshowB("first_binarization",first_binarization,off,on); waitKey();}
    
    Mat skel = LeeChenSkel(first_binarization);
    if (visualize)
        {imshowB("skel",skel,off,on); waitKey();}
    double FG_average, FG_std;
    extract_feat(src,&skel, &FG_average, &FG_std);
    double BG_average, BG_std;
    extract_feat(BG_prime,NULL, &BG_average, &BG_std);
    //Mat fg_feat = extract_fg_feat(src)
    //Mat bg_feat = extract_bg_feat(bg_estimation?);
    double C = -50.0*log10((FG_average+FG_std)/(BG_average-BG_std));
    double SW = strokeWidth(first_binarization,skel);
    
    cout << "FG_avg: "<<FG_average<<" FG_std: "<<FG_std<<" BG_avg: "<<BG_average<<" BG_std: "<<BG_std<<" C: "<<C<<" SW: "<<SW<<endl;
    
    Mat second_binarization = niblackBinarization(normalized,2*SW,-.2-.1*floor(C/10));
    if (visualize)
        {imshowB("second_binarization",second_binarization,off,on); waitKey();}
    Mat dst = combine(first_binarization,second_binarization,C);
    
    if (off!=0 || on!=1)
    {
        for (int r=0; r<dst.rows; r++)
            for (int c=0; c<dst.cols; c++)
            {
                if (dst.at<unsigned char>(r,c)==1)
                    dst.at<unsigned char>(r,c)=on;
                else
                    dst.at<unsigned char>(r,c)=off;
            }
    }
    if (visualize)
        waitKey();
    
    return dst;
}

void Binarization::imshowB(string window, const Mat& img, int off, int on)
{
    Mat dst = img.clone();
    if (off!=0 || on!=1)
    {
        for (int r=0; r<dst.rows; r++)
            for (int c=0; c<dst.cols; c++)
            {
                if (dst.at<unsigned char>(r,c)==1)
                    dst.at<unsigned char>(r,c)=on;
                else
                    dst.at<unsigned char>(r,c)=off;
            }
    }
    imshow(window,dst);
}

void Binarization::test()
{
    Mat testImg = imread("testImages/gray.png",CV_LOAD_IMAGE_GRAYSCALE);
    Mat res=niblackBinarization(testImg,30,-0.2);
    //imshowB("res",res,255,0);
    //waitKey();
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
    
    res=otsuBinarization(testImg);
    //imshowB("res",res,255,0);
    //waitKey();
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

/*Mat Binarization::niblackBinarization(const Mat& src, int size, double k)//inefficent
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
                    
                    double dif = ((double)src.at<unsigned char>(ty,tx))-mean;
                    stdDev += dif*dif;
                }
            stdDev = sqrt(stdDev/(src.cols*src.rows));
            double thresh = mean + k*stdDev;
            if (src.at<unsigned char>(y,x) <= thresh)
                dst.at<unsigned char>(y,x) = 1;//fg
            else
                dst.at<unsigned char>(y,x) = 0;//bg
        }
    return dst;
}*/


Mat Binarization::niblackBinarization(const Mat& src, int size, double k)//fast
{
    Mat dst = (Mat_<unsigned char>(src.rows,src.cols));
    double sum=0;
    double sqSum=0;
    list<int> sumCols;
    list<int> sqSumCols;
    double mean;
    double varSum;
    double stdDev;
    double thresh;
    for (int xOff=-floor(size/2); xOff<ceil(size/2); xOff++)
    {
        double sumCol=0;
        double sqSumCol=0;
        for (int yOff=-floor(size/2); yOff<ceil(size/2); yOff++)
        {
            int ty = 0+yOff;
            if (ty<0 || ty>=src.rows)
                ty = 0-yOff;
            
            int tx = 0+xOff;
            if (tx<0 || tx>=src.cols)
                tx = 0-xOff;
                
            sumCol +=src.at<unsigned char>(ty,tx);
            sqSumCol +=src.at<unsigned char>(ty,tx)*src.at<unsigned char>(ty,tx);
        }
        sumCols.push_back(sumCol);
        sum += sumCol;
        sqSumCols.push_back(sqSumCol);
        sqSum += sqSumCol;
    }
    mean=sum/(size*size);
    
    for (int y=0; y<src.rows; y++)
    {
        int x = y%2==0? 0 : src.cols-1;
        int xIter = y%2==0? 1 : -1;
        if (y!=0)
        {
            //move rows
            auto sumColsIter = sumCols.begin();
            auto sqSumColsIter = sqSumCols.begin();
            int yOffPrev = -floor(size/2)-1;
            int yOffNext = ceil(size/2)-1;
            sum=0;
            sqSum=0;
            for (int xOff=-floor(size/2); xOff<ceil(size/2); xOff++)
            {
                int tyPrev = y+yOffPrev;
                if (tyPrev<0 || tyPrev>=src.rows)
                    tyPrev = y-yOffPrev;
                int tyNext = y+yOffNext;
                if (tyNext<0 || tyNext>=src.rows)
                    tyNext = y-yOffNext;
                
                int tx = x+xOff;
                if (tx<0 || tx>=src.cols)
                    tx = x-xOff;
                    
                *sumColsIter -=src.at<unsigned char>(tyPrev,tx);
                *sumColsIter +=src.at<unsigned char>(tyNext,tx);
                sum += *sumColsIter;
                sumColsIter++;
                
                *sqSumColsIter -=src.at<unsigned char>(tyPrev,tx)*src.at<unsigned char>(tyPrev,tx);
                *sqSumColsIter +=src.at<unsigned char>(tyNext,tx)*src.at<unsigned char>(tyNext,tx);
                sqSum += *sqSumColsIter;
                sqSumColsIter++;
            }
            mean=sum/(size*size);
        }
        varSum = (size*size)*mean*mean + sqSum -2*mean*sum;
        stdDev = sqrt(varSum/(size*size));
        thresh = mean + k*stdDev;
        //cout << "thrsh V " << thresh << "  mean: " << mean << "  varSum: " << varSum << "  sum: "<<sum<<"  sqSum: "<<sqSum<<endl;
        //assert(varSum>=0);
        if (src.at<unsigned char>(y,x) < thresh)
            dst.at<unsigned char>(y,x) = 1;//fg
        else
            dst.at<unsigned char>(y,x) = 0;//bg
        
        
        do 
        {
            x+=xIter;
            
            int xOff;
            if (y%2==0)
            {
                sum -= sumCols.front();
                sumCols.pop_front();
                
                sqSum -= sqSumCols.front();
                sqSumCols.pop_front();
                
                xOff = ceil(size/2)-1;
            }
            else
            {
                sum -= sumCols.back();
                sumCols.pop_back();
                
                sqSum -= sqSumCols.back();
                sqSumCols.pop_back();
                
                xOff = -floor(size/2);
            }
            
            double sumCol=0;
            double sqSumCol=0;
            for (int yOff=-floor(size/2); yOff<ceil(size/2); yOff++)
            {
                int ty = y+yOff;
                if (ty<0 || ty>=src.rows)
                    ty = y-yOff;
                
                int tx = x+xOff;
                if (tx<0 || tx>=src.cols)
                    tx = x-xOff;
                    
                sumCol +=src.at<unsigned char>(ty,tx);
                sqSumCol +=src.at<unsigned char>(ty,tx)*src.at<unsigned char>(ty,tx);
            }
            if (y%2==0)
            {
                sumCols.push_back(sumCol);
                sqSumCols.push_back(sqSumCol);
            }
            else
            {
                sumCols.push_front(sumCol);
                sqSumCols.push_front(sqSumCol);
            }
            sum += sumCol;
            sqSum += sqSumCol;
            
            mean = sum/(size*size);
            varSum = (size*size)*mean*mean + sqSum -2*mean*sum;
            stdDev = sqrt(varSum/(size*size));
            thresh = mean + k*stdDev;
            //cout << "thrsh H " << thresh << "  mean: " << mean << "  varSum: " << varSum << "  sum: "<<sum<<"  sqSum: "<<sqSum<<endl;
            //assert(varSum>=0);
            if (src.at<unsigned char>(y,x) <= thresh)
                dst.at<unsigned char>(y,x) = 1;//fg
            else
                dst.at<unsigned char>(y,x) = 0;//bg
        } while (0<=x && x<src.cols);
    }
    return dst;
}

Mat Binarization::dilate(const Mat& src, int size)
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
                            dst.at<unsigned char>(ri+r,rj+c)=1;
                        }
                        
                    }
                }
            }
        }
    return dst;
}

Mat Binarization::inpainting(const Mat& src, const Mat& mask, Mat* prime, double* avg, double* std, bool show)
{
    int x_start[4] = {0,0,mask.cols-1,mask.cols-1};
    int x_end[4] = {mask.cols,mask.cols,-1,-1};
    int y_start[4] = {0,mask.rows-1,0,mask.rows-1};
    int y_end[4] = {mask.rows,-1,mask.rows,-1};
    Mat dst = src.clone();
    if (prime != NULL)
        *prime = src.clone();
    Mat P[4];
    Mat I = src.clone();
    for (int i=0; i<4; i++)
    {
        P[i] = (Mat_<unsigned char>(mask.rows,mask.cols));
        Mat M = mask.clone();
        int yStep = y_end[i]>y_start[i]?1:-1;
        int xStep = x_end[i]>x_start[i]?1:-1;
        for (int y=y_start[i]; y!=y_end[i]; y+=yStep)
            for (int x=x_start[i]; x!=x_end[i]; x+=xStep)
            {
                
                if (M.at<unsigned char>(y,x) == 0)
                {
                    
                    int denom = (x-1>=0?M.at<unsigned char>(y,x-1):0) + 
                                (y-1>=0?M.at<unsigned char>(y-1,x):0) +
                                (x+1<I.cols?M.at<unsigned char>(y,x+1):0) + 
                                (y+1<I.rows?M.at<unsigned char>(y+1,x):0);
                    if (denom !=0)
                    {
                        P[i].at<unsigned char>(y,x) = (
                                                        (x-1>=0?I.at<unsigned char>(y,x-1)*M.at<unsigned char>(y,x-1):0) +
                                                        (y-1>=0?I.at<unsigned char>(y-1,x)*M.at<unsigned char>(y-1,x):0) + 
                                                        (x+1<I.cols?I.at<unsigned char>(y,x+1)*M.at<unsigned char>(y,x+1):0) +
                                                        (y+1<I.rows?I.at<unsigned char>(y+1,x)*M.at<unsigned char>(y+1,x):0)
                                                      )/denom;
                        //if (P[i].at<unsigned char>(y,x)!=0)
                        //{
                            if (avg!=NULL && std!=NULL)
                            {
                                if (P[i].at<unsigned char>(y,x)>*avg)
                                {
                                    double dif = std::min(P[i].at<unsigned char>(y,x)-(*avg), 3*(*std));
                                    P[i].at<unsigned char>(y,x) -= 8.5*dif/(*std);
                                }
                                else if (P[i].at<unsigned char>(y,x)<*avg)
                                {
                                    double dif = std::min((*avg)-P[i].at<unsigned char>(y,x), 3*(*std));
                                    P[i].at<unsigned char>(y,x) += 8.5*dif/(*std);
                                }
                            }
                            I.at<unsigned char>(y,x) = P[i].at<unsigned char>(y,x);
                            M.at<unsigned char>(y,x) = 1;
                        //}
                        //else
                        //    P[i].at<unsigned char>(y,x) = 255;
                    }
                    else
                        P[i].at<unsigned char>(y,x) = 255;
                }
                else
                    P[i].at<unsigned char>(y,x) = I.at<unsigned char>(y,x);
            }
    }
    if (show)
    {
        imshow("P[0]",P[0]); waitKey();
        imshow("P[1]",P[1]); waitKey();
        imshow("P[2]",P[2]); waitKey();
        imshow("P[3]",P[3]); waitKey();
    }
    
    for (int y=0; y<mask.rows; y++)
        for (int x=0; x<mask.cols; x++)
        {
            if (mask.at<unsigned char>(y,x) == 0)
            {
                dst.at<unsigned char>(y,x) = std::min( std::min(P[0].at<unsigned char>(y,x),
                                                                P[1].at<unsigned char>(y,x))
                                                       ,
                                                       std::min(P[2].at<unsigned char>(y,x),
                                                                P[3].at<unsigned char>(y,x))
                                                     );
                assert(dst.at<unsigned char>(y,x)!=0);
                if (prime != NULL)
                    prime->at<unsigned char>(y,x) =  (P[0].at<unsigned char>(y,x) + 
                                                      P[1].at<unsigned char>(y,x) + 
                                                      P[2].at<unsigned char>(y,x) +
                                                      P[3].at<unsigned char>(y,x))/4.0;
            }
                                                 
        }
    return dst;
}

Mat Binarization::img_normalize(const Mat& src, const Mat& bg)
{
    float Fmin=numeric_limits<float>::max();
    float Fmax=0;
    int Imin=numeric_limits<int>::max();
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
    if (Fmax>2.0)
        Fmax=2.0;
    Mat dst = (Mat_<unsigned char>(src.rows,src.cols));
    for (int r=0; r<src.rows; r++)
        for (int c=0; c<src.cols; c++)
        {
            dst.at<unsigned char>(r,c) = ceil( (Imax-Imin)*std::min( (F.at<float>(r,c)-Fmin)/(Fmax-Fmin),1.0f) + Imin );
            //assert(dst.at<unsigned char>(r,c)!=0);
        }
    return dst;
}

Mat Binarization::otsuBinarization(const Mat& src)
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

tuple< Mat,map<int,int> > connComp(const Mat& bin, function<void (int,int,int)> newCC, function<void (int,int,int)> addToCC, function<void (int,int,int,int)> mergeCC)
{
    Mat cc(bin.rows,bin.cols,CV_32S);
    map<int,int> ccMap;
    int ccIndex=1;
    int countPixels=0;
    for (int r=0; r<bin.rows; r++)
        for (int c=0; c<bin.cols; c++)
        {
            int oldIndex=ccIndex;
            if (bin.at<unsigned char>(r,c)==1)
            {
                countPixels++;
                int upBin = bin.at<unsigned char>(r-1,c);
                int leftBin = bin.at<unsigned char>(r,c-1);
                int upLeftBin = bin.at<unsigned char>(r-1,c-1);
                int upRightBin = bin.at<unsigned char>(r-1,c+1);
                
                int ccI=0;
                if (r!=0&&upBin!=0)
                {
                    ccI = cc.at<int>(r-1,c);
                    while (ccMap[ccI]!=0)
                    {
                        ccI = ccMap[ccI];
                        //ccMap[cc.at<int>(r-1,c)]=ccI;
                    }
                    if (cc.at<int>(r-1,c) != ccI)
                        cc.at<int>(r-1,c)=ccI;
                }
                else if (c!=0&&leftBin!=0)
                {
                    ccI = cc.at<int>(r,c-1);
                    while (ccMap[ccI]!=0)
                    {
                        ccI = ccMap[ccI];
                        //ccMap[cc.at<int>(r,c-1)]=ccI;
                    }
                    if (cc.at<int>(r,c-1) != ccI)
                        cc.at<int>(r,c-1)=ccI;
                }
                else if (r!=0&&c!=0&&upLeftBin!=0)
                {
                    ccI = cc.at<int>(r-1,c-1);
                    while (ccMap[ccI]!=0)
                    {
                        ccI = ccMap[ccI];
                        //ccMap[cc.at<int>(r-1,c-1)]=ccI;
                    }
                    if (cc.at<int>(r-1,c-1) != ccI)
                        cc.at<int>(r-1,c-1)=ccI;
                }
                int ccIDag=0;
                if (r!=0&&c<bin.cols-1&&upRightBin!=0)
                {
                    ccIDag = cc.at<int>(r-1,c+1);
                    while (ccMap[ccIDag]!=0)
                    {
                        ccIDag = ccMap[ccIDag];
                        //ccMap[cc.at<int>(r-1,c+1)]=ccIDag;
                    }
                    if (cc.at<int>(r-1,c+1) != ccIDag)
                        cc.at<int>(r-1,c+1)=ccIDag;
                }
                if (ccI==0)
                    ccI=ccIDag;
                if (ccIDag!=0 && ccIDag!=ccI)
                {//merge
                    int mergeTo = ccI;
                    int toMerge = ccIDag;
                    if (mergeTo != toMerge)
                    {
                        //cout << "merging "<<toMerge<<" to "<<mergeTo<<endl;
                        ccMap[toMerge]=mergeTo;
                        mergeCC(mergeTo,toMerge,r,c);
                    }
                    addToCC(mergeTo,r,c);
                    cc.at<int>(r,c)=mergeTo;
                    
                    assert(mergeTo!=1);
                    assert(toMerge!=1);
                }
                else
                {
                    if (ccI==0)
                    {//new
                        cc.at<int>(r,c)=++ccIndex;
                        ccMap[ccIndex]=0;
                        newCC(ccIndex,r,c);
                    }
                    else
                    {//add
                        cc.at<int>(r,c)=ccI;
                        assert(ccI!=1);
                        addToCC(ccI,r,c);
                    }
                }
                /*if ((r==0||upBin==0) && (c==0||leftBin==0))
                {
                    cc.at<unsigned int>(r,c)=++ccIndex;
                    ccMap[ccIndex]=0;
                    newCC(ccIndex,r,c);
                }
                else if ((r==0||upBin==0) && (c!=0&&leftBin!=0))
                {
                    int leftCCIndex = cc.at<int>(r,c-1);
                    while (ccMap[leftCCIndex]!=0)
                        leftCCIndex = ccMap[leftCCIndex];
                    int ccI=leftCCIndex;
                    cc.at<unsigned int>(r,c)=ccI;
                    assert(ccI!=1);
                    addToCC(ccI,r,c);
                }
                else if ((r!=0&&upBin!=0) && (c==0||leftBin==0))
                {
                    int upCCIndex = cc.at<int>(r-1,c);
                    while (ccMap[upCCIndex]!=0)
                            upCCIndex = ccMap[upCCIndex];
                    int ccI=upCCIndex;
                    cc.at<unsigned int>(r,c)=ccI;
                    assert(ccI!=1);
                    addToCC(ccI,r,c);
                }
                else if ((r!=0&&c!=0&&upLeftBin!=0) && (c==0||leftBin==0))
                {
                    int upCCIndex = cc.at<int>(r-1,c);
                    while (ccMap[upCCIndex]!=0)
                            upCCIndex = ccMap[upCCIndex];
                    int ccI=upCCIndex;
                    cc.at<unsigned int>(r,c)=ccI;
                    assert(ccI!=1);
                    addToCC(ccI,r,c);
                }
                else //merge
                {
                    int upCCIndex = cc.at<int>(r-1,c);
                    while (ccMap[upCCIndex]!=0)
                            upCCIndex = ccMap[upCCIndex];
                    int leftCCIndex = cc.at<int>(r,c-1);
                    while (ccMap[leftCCIndex]!=0)
                        leftCCIndex = ccMap[leftCCIndex];
                    if (upCCIndex==leftCCIndex)
                    {
                        int ccI=upCCIndex;
                        cc.at<unsigned int>(r,c)=ccI;
                        assert(ccI!=1);
                        addToCC(ccI,r,c);
                    }
                    else
                    {
                        int mergeTo = upCCIndex;
                        int toMerge = leftCCIndex;
                        if (mergeTo != toMerge)
                        {
                            //cout << "merging "<<toMerge<<" to "<<mergeTo<<endl;
                            ccMap[toMerge]=mergeTo;
                            mergeCC(mergeTo,toMerge,r,c);
                        }
                        addToCC(mergeTo,r,c);
                        cc.at<unsigned int>(r,c)=mergeTo;
                        
                        assert(mergeTo!=1);
                        assert(toMerge!=1);
                    }
                }*/
            }
            assert(oldIndex==ccIndex || oldIndex+1==ccIndex);
            assert(ccIndex==1 || ccMap.find(ccIndex)!=ccMap.end());
            assert(ccIndex>0);
        }
    return make_tuple(cc,ccMap);
}

Mat Binarization::postProcessing(const Mat& bin)
{
    //find connected components
    /*Mat cc(bin.rows,bin.cols,CV_32S);
    map<int,int> ccMap;*/
    map<int,int> ccCounts;
    map<int,int> ccMinY;
    map<int,int> ccMaxY;
    int countPixels = 0;
    auto newCC = [&ccMaxY,&ccMinY,&ccCounts, &countPixels](int ccIndex, int r, int c) {
        ccCounts[ccIndex]=1;
        ccMinY[ccIndex]=r;
        ccMaxY[ccIndex]=r;
        countPixels++;
    };
    auto addToCC = [&ccMaxY, &ccCounts, &countPixels](int ccI, int r, int c) {
        ccCounts[ccI]++;
        ccMaxY[ccI]=r;
        countPixels++;
    };
    auto mergeCC = [&ccMaxY,&ccMinY,&ccCounts, &countPixels](int mergeTo, int toMerge, int r, int c) {
        ccCounts[mergeTo] += ccCounts[toMerge];
        ccMinY[mergeTo]=std::min(ccMinY[mergeTo],ccMinY[toMerge]);
        ccMaxY[mergeTo]=r;
        countPixels++;
    };
    tuple< Mat,map<int,int> > ccA = connComp(bin,newCC,addToCC,mergeCC);
    Mat cc = get<0>(ccA);
    map<int,int> ccMap = get<1>(ccA);
    
    /*int ccIndex=1;
    int countPixels=0;
    for (int r=0; r<bin.rows; r++)
        for (int c=0; c<bin.cols; c++)
        {
            if (bin.at<unsigned char>(r,c)==1)
            {
                countPixels++;
                int upBin = bin.at<unsigned char>(r-1,c);
                int upCCIndex = cc.at<int>(r-1,c);
                while (ccMap[upCCIndex]!=0)
                        upCCIndex = ccMap[upCCIndex];
                int leftBin = bin.at<unsigned char>(r,c-1);
                int leftCCIndex = cc.at<int>(r,c-1);
                while (ccMap[leftCCIndex]!=0)
                        leftCCIndex = ccMap[leftCCIndex];
                if ((r==0||upBin==0) && (c==0||leftBin==0))
                {
                    cc.at<unsigned int>(r,c)=++ccIndex;
                    
                    ccMap[ccIndex]=0;
                    
                    ccCounts[ccIndex]=1;
                    ccMinY[ccIndex]=r;
                    ccMaxY[ccIndex]=r;
                }
                else if ((r==0||upBin==0) && (c!=0&&leftBin!=0))
                {
                    int ccI=leftCCIndex;
                    cc.at<unsigned int>(r,c)=ccI;
                    ccCounts[ccI]++;
                    ccMaxY[ccI]=r;
                    assert(ccI!=1);
                }
                else if ((r!=0&&upBin!=0) && (c==0||leftBin==0||upCCIndex==leftCCIndex))
                {
                    int ccI=upCCIndex;
                    cc.at<unsigned int>(r,c)=ccI;
                    ccCounts[ccI]++;
                    ccMaxY[ccI]=r;
                    assert(ccI!=1);
                }
                else //merge
                {
                    int mergeTo = upCCIndex;
                    int toMerge = leftCCIndex;
                    if (mergeTo != toMerge)
                    {
                        cout << "merging "<<toMerge<<" to "<<mergeTo<<endl;
                        ccCounts[mergeTo] += ccCounts[toMerge];
                        ccMap[toMerge]=mergeTo;
                        ccMinY[mergeTo]=std::min(ccMinY[mergeTo],ccMinY[toMerge]);
                        ccCounts[mergeTo]++;
                    }
                    ccMaxY[mergeTo]=r;//std::max(ccMaxY[mergeTo],ccMaxY[toMerge]);
                    cc.at<unsigned int>(r,c)=mergeTo;
                    
                    assert(mergeTo!=1);
                    assert(toMerge!=1);
                    
                }
            }
        }*/
    int ccCount=0;
    int maxHeight=0;
    map< int, vector<int> > ccByHeight;
    for (auto ccPointer : ccMap)
    {
        if (ccPointer.second==0 && ccPointer.first!=0)
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
    for (int j=0; j<maxHeight; j++)
    {
        double RP = 0;
        for (int ccIndex : ccByHeight[j])
        {
            RP += ccCounts[ccIndex];
        }
        RP /= countPixels;
        
        double RC = ccByHeight[j].size()/(double) ccCount;
        //cout << "RP: "<<RP<<" RC: "<<RC<<" RP/RC: "<<RP/RC<<endl;
        if (RC!=0)
            sum += RP/RC;
        if (sum > 1) {
            h=j;
            break;
        }
    }
    
    Mat ret(cc.rows,cc.cols,CV_8U);
    //Mat test(cc.rows,cc.cols,CV_8UC3);
    //map<int,Vec3b> colors;
    for (int r=0; r<cc.rows; r++)
        for (int c=0; c<cc.cols; c++)
        {
            int ccIndex=cc.at<unsigned int>(r,c);
            while (ccMap[ccIndex]!=0)
                        ccIndex = ccMap[ccIndex];
            if (ccMaxY[ccIndex]-ccMinY[ccIndex] >= h)
            {
                ret.at<unsigned char>(r,c)=1;
            }
            else
            {
                ret.at<unsigned char>(r,c)=0;
            }
            
            /*if (colors.find(ccIndex)==colors.end())
            {
                colors[ccIndex]=Vec3b((rand()%156)+100,(rand()%156)+100,(rand()%156)+100);
            }
            test.at<Vec3b>(r,c)=colors[ccIndex];*/
        }
    //imshow("test",test);
    //waitKey();
    assert(h!=-1);
    return ret;
}


//////////ZHANG SKELTONIZATION///////////////////////////////////////
///////////from https://github.com/bsdnoobz/zhang-suen-thinning//////
void thinningIteration(cv::Mat& img, int iter)
{
    CV_Assert(img.channels() == 1);
    CV_Assert(img.depth() != sizeof(uchar));
    CV_Assert(img.rows > 3 && img.cols > 3);

    cv::Mat marker = cv::Mat::zeros(img.size(), CV_8UC1);

    int nRows = img.rows;
    int nCols = img.cols;

    if (img.isContinuous()) {
        nCols *= nRows;
        nRows = 1;
    }

    int x, y;
    uchar *pAbove;
    uchar *pCurr;
    uchar *pBelow;
    uchar *nw, *no, *ne;    // north (pAbove)
    uchar *we, *me, *ea;
    uchar *sw, *so, *se;    // south (pBelow)

    uchar *pDst;

    // initialize row pointers
    pAbove = NULL;
    pCurr  = img.ptr<uchar>(0);
    pBelow = img.ptr<uchar>(1);

    for (y = 1; y < img.rows-1; ++y) {
        // shift the rows up by one
        pAbove = pCurr;
        pCurr  = pBelow;
        pBelow = img.ptr<uchar>(y+1);

        pDst = marker.ptr<uchar>(y);

        // initialize col pointers
        no = &(pAbove[0]);
        ne = &(pAbove[1]);
        me = &(pCurr[0]);
        ea = &(pCurr[1]);
        so = &(pBelow[0]);
        se = &(pBelow[1]);

        for (x = 1; x < img.cols-1; ++x) {
            // shift col pointers left by one (scan left to right)
            nw = no;
            no = ne;
            ne = &(pAbove[x+1]);
            we = me;
            me = ea;
            ea = &(pCurr[x+1]);
            sw = so;
            so = se;
            se = &(pBelow[x+1]);

            int A  = (*no == 0 && *ne == 1) + (*ne == 0 && *ea == 1) +
                     (*ea == 0 && *se == 1) + (*se == 0 && *so == 1) +
                     (*so == 0 && *sw == 1) + (*sw == 0 && *we == 1) +
                     (*we == 0 && *nw == 1) + (*nw == 0 && *no == 1);
            int B  = *no + *ne + *ea + *se + *so + *sw + *we + *nw;
            int m1 = iter == 0 ? (*no * *ea * *so) : (*no * *ea * *we);
            int m2 = iter == 0 ? (*ea * *so * *we) : (*no * *so * *we);

            if (A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0)
                pDst[x] = 1;
        }
    }

    img &= ~marker;
}
/**
 * Function for thinning the given binary image
 *
 * Parameters:
 *              src  The source image, binary with range = [0,255]
 *              dst  The destination image
 */
void ZhangSkel(const cv::Mat& bin, cv::Mat& dst)
{
    dst = bin.clone();     // convert to binary image

    cv::Mat prev = cv::Mat::zeros(dst.size(), CV_8UC1);
    cv::Mat diff;

    do {
        thinningIteration(dst, 0);
        thinningIteration(dst, 1);
        cv::absdiff(dst, prev, diff);
        dst.copyTo(prev);
    }
    while (cv::countNonZero(diff) > 0);

    

    //cleanNoiseFromEdges(dst);
}


#define FF 3

unsigned char Binarization::ToByte(bool b[8])
{
    unsigned char c = 0;
    for (int i=0; i < 8; ++i)
        if (b[i])
            c |= 1 << i;
    return c;
}
//RECOGNITION OF HANDWRITTEN CHINESE CHARACTERS VIA SHORT LINE SEGMENTS
//Lee & Chen, Pattern Recognition 1992
Mat Binarization::LeeChenSkel(const Mat& bin)
{
    vector<char> lookupTable         { FF, FF, FF, 01, FF, 00, 01, 01, FF, FF, FF, FF, 01, 00, 01, 01,
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
                                       01, 01, FF, 01, 00, FF, 00, FF, 01, 01, FF, 00, 01, FF, 00, FF };
    Mat skel;// = bin.clone();
    ZhangSkel(bin,skel);
    bool cont=true;
    for  (int iter=0; cont && iter<bin.cols*bin.rows; iter++)
    {
        cont=false;
        for (int r=0; r<skel.rows; r++)
            for (int c=0; c<skel.cols; c++)
            {
                bool p1 = skel.at<unsigned char>(r,c)==1;
                if (p1)
                {
                    bool p2 = r>0 && skel.at<unsigned char>(r-1,c)==1;
                    bool p3 = r>0 && c<skel.cols-1 && skel.at<unsigned char>(r-1,c+1)==1;
                    bool p4 = c<skel.cols-1 && skel.at<unsigned char>(r,c+1)==1;
                    bool p5 = r<skel.rows-1 && c<skel.cols-1 && skel.at<unsigned char>(r+1,c+1)==1;
                    bool p6 = r<skel.rows-1  && skel.at<unsigned char>(r+1,c)==1;
                    bool p7 = r<skel.rows-1 && c>0 && skel.at<unsigned char>(r+1,c-1)==1;
                    bool p8 = c>0 && skel.at<unsigned char>(r,c-1)==1;
                    bool p9 = r>0 && c>0 && skel.at<unsigned char>(r-1,c-1)==1;
                    bool b[8] = {p9,p8,p7,p6,p5,p4,p3,p2};
                    if (FF!=lookupTable[ToByte(b)]) 
                    {
                        skel.at<unsigned char>(r,c)=0;
                        cont=true;
                        //if (lookupTable[ToByte(b)]==01)
                            //cout<<"Zhang pixel found"<<endl;
                    }
                    //else
                    //    cout << "keep pixel "<<r<<","<<c<<" b: "<<(int)ToByte(b)<<endl;
                }
            }
    }
    return skel;
}


double Binarization::computeStrokeWidth(int row, int col, const Mat& bin)
{
    double minSqrDist=9999;
    Point minPoint(-1,-1);
    bool term = false;
    vector<Point> toEval;
    vector<Point> toEvalNext;
    toEval.push_back(Point(col,row));
    Mat mark = bin.clone();
    mark.at<unsigned char>(row,col)=0;
    do
    {
        if (minPoint.x!=-1)
            term=true;
        for (Point p : toEval)
        {
            Point u(p.x,p.y-1);
            Point d(p.x,p.y+1);
            Point l(p.x-1,p.y);
            Point r(p.x-2,p.y);
            if (p.y==0||bin.at<unsigned char>(u)==0||
                p.y==bin.rows-1||bin.at<unsigned char>(d)==0||
                p.x==0||bin.at<unsigned char>(l)==0||
                p.x==bin.cols-1||bin.at<unsigned char>(r)==0
               )
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
                if (p.y>0&&mark.at<unsigned char>(u)!=0)
                {
                    toEvalNext.push_back(u);
                    mark.at<unsigned char>(u)=0;
                }
                if (p.y<bin.rows-1&&mark.at<unsigned char>(d)!=0)
                {
                    toEvalNext.push_back(d);
                    mark.at<unsigned char>(d)=0;
                }
                if (p.x>0&&mark.at<unsigned char>(l)!=0)
                {
                    toEvalNext.push_back(l);
                    mark.at<unsigned char>(l)=0;
                }
                if (p.x<bin.cols-1&&mark.at<unsigned char>(r)!=0)
                {
                    toEvalNext.push_back(r);
                    mark.at<unsigned char>(r)=0;
                }
            }
        }
        toEval.clear();
        toEval.swap(toEvalNext);
    } while (!term);
    mark.release();
    return sqrt(minSqrDist);
}
double Binarization::strokeWidth(const Mat& bin, const Mat& skel)
{
    map<int,double> ccMax;
    auto newCC = [&ccMax, &bin](int ccIndex, int r, int c) {
        ccMax[ccIndex]=std::max(computeStrokeWidth(r,c,bin),ccMax[ccIndex]);
    };
    auto addToCC = [&ccMax, &bin](int ccI, int r, int c) {
        ccMax[ccI]=std::max(computeStrokeWidth(r,c,bin),ccMax[ccI]);
    };
    auto mergeCC = [&ccMax, &bin](int mergeTo, int toMerge, int r, int c) {
        ccMax[mergeTo]=std::max(computeStrokeWidth(r,c,bin),std::max(ccMax[mergeTo],ccMax[toMerge]));
    };
    tuple< Mat,map<int,int> > ccA = connComp(bin,newCC,addToCC,mergeCC);
    Mat cc = get<0>(ccA);
    map<int,int> ccMap = get<1>(ccA);
    /*
    Mat cc = skel.clone();
    map<int,int> ccMap;
    map<int,int> ccCounts;
    map<int,double> ccMax;
    int ccIndex=1;
    for (int r=0; r<skel.rows; r++)
        for (int c=0; c<skel.cols; c++)
        {
            if (bin.at<unsigned char>(r,c)==1)
            {
                if ((r==0||bin.at<unsigned char>(r-1,c)==0) && (c==0||bin.at<unsigned char>(r,c-1)==0))
                {
                    cc.at<unsigned char>(r,c)=++ccIndex;
                    ccMap[ccIndex]=0;
                    ccMax[ccIndex]=std::max(computeStrokeWidth(r,c,bin),ccMax[ccIndex]);
                }
                else if ((r==0||bin.at<unsigned char>(r-1,c)==0||cc.at<unsigned char>(r-1,c)==cc.at<unsigned char>(r,c-1)) && (c!=0&&bin.at<unsigned char>(r,c-1)!=0))
                {
                    int ccI=bin.at<unsigned char>(r,c-1);
                    cc.at<unsigned char>(r,c)=ccI;
                    ccMax[ccI]=std::max(computeStrokeWidth(r,c,bin),ccMax[ccI]);
                }
                else if ((r!=0&&bin.at<unsigned char>(r-1,c)!=0) && (c==0||bin.at<unsigned char>(r,c-1)==0))
                {
                    int ccI=bin.at<unsigned char>(r-1,c);
                    cc.at<unsigned char>(r,c)=ccI;
                    ccMax[ccI]=std::max(computeStrokeWidth(r,c,bin),ccMax[ccI]);
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
        }*/
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

void Binarization::extract_feat(const Mat& src, const Mat* skel, double* average, double* std)
{
    double sum=0;
    int count=0;
    for (int r=0; r< src.rows; r++)
        for (int c=0; c<src.cols; c++)
        {
            if (skel==NULL || skel->at<unsigned char>(r,c)==1)
            {
                sum += src.at<unsigned char>(r,c);
                count++;
            }
        }
    *average = sum/count;
    sum=0;
    for (int r=0; r< src.rows; r++)
        for (int c=0; c<src.cols; c++)
        {
            if (skel==NULL || skel->at<unsigned char>(r,c)==1)
            {
                sum += pow(*average - src.at<unsigned char>(r,c), 2);
            }
        }
    *std = sqrt(sum/count);
}

Mat Binarization::combine(const Mat& o_bin, const Mat& n_bin, double C)
{
    map<int,int> ccCounts;
    map<int,int> ccCountsIntersectOtsu;
    auto newCC = [&ccCounts, &ccCountsIntersectOtsu, &o_bin](int ccIndex, int r, int c) {
        ccCounts[ccIndex]=1;
        ccCountsIntersectOtsu[ccIndex]= o_bin.at<unsigned char>(r,c)==1?1:0;
    };
    auto addToCC = [&ccCounts, &ccCountsIntersectOtsu, &o_bin](int ccI, int r, int c) {
        ccCounts[ccI]++;
        ccCountsIntersectOtsu[ccI] += o_bin.at<unsigned char>(r,c)==1?1:0;
    };
    auto mergeCC = [&ccCounts, &ccCountsIntersectOtsu, &o_bin](int mergeTo, int toMerge, int r, int c) {
        ccCounts[mergeTo] += ccCounts[toMerge];
        ccCountsIntersectOtsu[mergeTo] += ccCountsIntersectOtsu[toMerge];
    };
    tuple< Mat,map<int,int> > ccA = connComp(n_bin,newCC,addToCC,mergeCC);
    Mat cc = get<0>(ccA);
    map<int,int> ccMap = get<1>(ccA);
    /*
    Mat cc = n_bin.clone();
    map<int,int> ccMap;
    map<int,int> ccCounts;
    map<int,int> ccCountsIntersectOtsu;
    int ccIndex=1;
    for (int r=0; r<n_bin.rows; r++)
        for (int c=0; c<n_bin.cols; c++)
        {
            if (n_bin.at<unsigned char>(r,c)==1)
            {
                if ((r==0||n_bin.at<unsigned char>(r-1,c)==0) && (c==0||n_bin.at<unsigned char>(r,c-1)==0))
                {
                    cc.at<unsigned char>(r,c)=++ccIndex;
                    ccCounts[ccIndex]=1;
                    ccCountsIntersectOtsu[ccIndex]= o_bin.at<unsigned char>(r,c)==1?1:0;
                    ccMap[ccIndex]=0;
                }
                else if ((r==0||n_bin.at<unsigned char>(r-1,c)==0||cc.at<unsigned char>(r-1,c)==cc.at<unsigned char>(r,c-1)) && (c!=0&&n_bin.at<unsigned char>(r,c-1)!=0))
                {
                    int ccI=n_bin.at<unsigned char>(r,c-1);
                    cc.at<unsigned char>(r,c)=ccI;
                    ccCounts[ccI]++;
                    ccCountsIntersectOtsu[ccI] += o_bin.at<unsigned char>(r,c)==1?1:0;
                }
                else if ((r!=0&&n_bin.at<unsigned char>(r-1,c)!=0) && (c==0||n_bin.at<unsigned char>(r,c-1)==0))
                {
                    int ccI=n_bin.at<unsigned char>(r-1,c);
                    cc.at<unsigned char>(r,c)=ccI;
                    ccCounts[ccI]++;
                    ccCountsIntersectOtsu[ccI] += o_bin.at<unsigned char>(r,c)==1?1:0;
                }
                else //merge
                {
                    int toMerge = n_bin.at<unsigned char>(r,c-1);
                    while (ccMap[toMerge]!=0)
                        toMerge = ccMap[toMerge];
                    int mergeTo = n_bin.at<unsigned char>(r-1,c);
                    ccCounts[mergeTo] += ccCounts[toMerge];
                    ccCountsIntersectOtsu[mergeTo] += ccCountsIntersectOtsu[toMerge];
                    ccMap[toMerge]=mergeTo;
                    cc.at<unsigned char>(r,c)=mergeTo;
                    ccCounts[mergeTo]++;
                    ccCountsIntersectOtsu[mergeTo] += o_bin.at<unsigned char>(r,c)==1?1:0;
                }
            }
        }*/
    
    
    Mat res(n_bin.size(),n_bin.type());
    vector<int> ccKeep;
    for (auto ccPointer : ccMap)
    {
        if (ccPointer.second==0 && ccPointer.first!=0)
        {
            if ((100.0*ccCountsIntersectOtsu[ccPointer.first])/ccCounts[ccPointer.first] >= C)
                ccKeep.push_back(ccPointer.first);
        }
    }
    for (int r=0; r<cc.rows; r++)
        for (int c=0; c<cc.cols; c++)
        {
            if (cc.at<int>(r,c)!=0)
            {
                int ccI = cc.at<int>(r,c);
                while (ccMap[ccI]!=0)
                {
                    ccI = ccMap[ccI];
                    ccMap[cc.at<int>(r,c)]=ccI;
                }
                if (binary_search(ccKeep.begin(), ccKeep.end(),ccI))
                    res.at<unsigned char>(r,c)==1;
                
            }
        }
    for (int r=0; r<res.rows; r++)
        for (int c=0; c<res.cols; c++)
        {
            if (o_bin.at<unsigned char>(r,c)!=0 && res.at<unsigned char>(r,c)==0)
            {
                for (int rOff=-1; rOff<=1; rOff++)
                    for (int cOff=-1; cOff<=1; cOff++)
                    {
                        if (res.at<unsigned char>(r+rOff,c+cOff)!=0)
                        {
                            res.at<unsigned char>(r,c)=1;
                            rOff=2;
                            break;
                        }
                    }
            }
        }
    return res;
}
