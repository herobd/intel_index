#include "MorphSpotter.h"
#include <string.h>

MorphSpotter::MorphSpotter()
{

}
void MorphSpotter::cvToD(const Mat& cvI, DImage& dI)
{
    dI.setLogicalSize(cvI.cols,cvI.rows);
    unsigned char* data1 = dI.dataPointer_u8();
    unsigned char* dataO = cvI.data;
    /*for (int i=0; i< cvI.cols * cvI.rows; i++)
    {
        data1[i]=dataO[i];
    }*/
    memcpy(data1,dataO,cvI.cols * cvI.rows);
}

double MorphSpotter::score(Mat im1, Mat im2)
{
    DImage dim1, dim2;
    Mat bim1=binarize(im1);
    Mat bim2=binarize(im2);
    cvToD(bim1,dim1);
    cvToD(bim2,dim2);
    /*int tval = getThreshold(im1);
    DThresholder::threshImage_(dim1,dim1, tval);
    tval = getThreshold(im2);
    DThresholder::threshImage_(dim2,dim2, tval);*/
    return mobj.getWordMorphCost(dim1,dim2);
}
double MorphSpotter::scoreFast(Mat im1, Mat im2)
{
    DImage dim1, dim2;
    Mat bim1=binarize(im1);
    Mat bim2=binarize(im2);
    cvToD(bim1,dim1);
    cvToD(bim2,dim2);
    /*int tval = getThreshold(im1);
    DThresholder::threshImage_(dim1,dim1, tval);
    tval = getThreshold(im2);
    DThresholder::threshImage_(dim2,dim2, tval);*/
    return mobj.getWordMorphCostFast(dim1,dim2);
}
double MorphSpotter::score_preThreshed(Mat im1, Mat im2)
{
    DImage dim1, dim2;
    cvToD(im1,dim1);
    cvToD(im2,dim2);
    return mobj.getWordMorphCost(dim1,dim2);
}
double MorphSpotter::scoreFast_preThreshed(Mat im1, Mat im2)
{
    DImage dim1, dim2;
    cvToD(im1,dim1);
    cvToD(im2,dim2);
    return mobj.getWordMorphCostFast(dim1,dim2);
}

inline int xDelta(int direction)
{
    int xDelta=1;
    if (direction>2 && direction<6) xDelta=-1;
    else if (direction==2 || direction==6) xDelta=0;
    return xDelta;
}

inline int yDelta(int direction)
{
    int yDelta=0;
    if (direction>0 && direction<4) yDelta=-1;
    else if (direction>4 && direction<8) yDelta=1;
    return yDelta;
}

void MorphSpotter::scrubCCs(Mat& im)
{
    vector<bool> visited(im.cols*im.rows);
    visited.assign(im.cols*im.rows,false);

    for (int r=0; r<im.rows; r++)
        for (int c=0; c<im.cols; c++)
        {
            if (!visited[c+r*im.cols] && im.at<unsigned char>(r,c)==0)
            {
                list<Point> toVisit;
                list<Point> toScrub;
                toVisit.push_back(Point(c,r));
                toScrub.push_back(Point(c,r));
                visited[c+r*im.cols]=true;
                int count = 1;
                while (!toVisit.empty())
                {
                    Point cur = toVisit.back();
                    toVisit.pop_back();

                    for (int direction=0; direction<8; direction++)
                    {
                        int x = cur.x+xDelta(direction);
                        int y = cur.y+yDelta(direction);
                        if (x<0 || y<0 || x>=im.cols || y>=im.rows)
                            continue;
                        if (im.at<unsigned char>(y,x)==0 && !visited[x+y*im.cols])
                        {
                            ++count;
                            visited[x+y*im.cols]=true;
                            toVisit.push_back(Point(x,y));
                            toScrub.push_back(Point(x,y));
                        }
                    }
                }

                if (count <= SCRUB_THRESH)
                {
                    /*toVisit.clear();
                    toVisit.push_back(Point(c,r));
                    while (!toVisit.empty())
                    {
                        Point cur = toVisit.back();
                        toVisit.pop_back();
                        im.at<unsigned char>(cur)=255;

                        for (int direction=0; direction<8; direction++)
                        {
                            int x = cur.x+xDelta(direction);
                            int y = cur.y+yDelta(direction);
                            if (x<0 || y<0 || x>=im.cols || y>=im.rows)
                                continue;
                            if (im.at<unsigned char>(y,x)==0)
                            {
                                im.at<unsigned char>(y,x)=255;
                                toVisit.push_back(Point(x,y));
                            }
                        }
                    }*/
                    for (Point p : toScrub)
                        im.at<unsigned char>(p)=255;
                }
            }
        }
}

//Both binarizes and does some cleaning
Mat MorphSpotter::binarize(const Mat& orig)
{
    Mat ret;
    adaptiveThreshold(orig,ret,255,ADAPTIVE_THRESH_GAUSSIAN_C,THRESH_BINARY,11,2);
    /*Mat element = getStructuringElement(MORPH_ELLIPSE,
                                       Size( 3, 3 ) );

    /// Apply the erosion operation
    Mat scrubbed;
    dilate( ret, scrubbed, element );
    element = getStructuringElement(MORPH_ELLIPSE,
                                   Size( 7, 7 ) );
    erode( scrubbed, scrubbed, element );
    
    for (int r=0; r<ret.rows; r++)
        for (int c=0; c<ret.cols; c++)
            if (scrubbed.at<unsigned char>(r,c)==255)
                ret.at<unsigned char>(r,c)=255;
*/
    //imshow("before scrub",ret);
    scrubCCs(ret);
    imshow("bin",ret);
    waitKey();
    return ret;
}

int sort_xxx(const void *x, const void *y) {
    if (*(int*)x > *(int*)y) return 1;
    else if (*(int*)x < *(int*)y) return -1;
    else return 0;
}

void MorphSpotter::eval(const Dataset* data)
{
    float map=0;
    int queryCount=0;
    vector<Mat> binarized(data->size());
    #pragma omp parallel  for
    for (int inst=0; inst<data->size(); inst++)
    {
        binarized[inst]=binarize(data->image(inst));
    }
    
    #pragma omp parallel  for
    for (int inst=0; inst<data->size(); inst++)
    {
        int other=0;
        string text = data->labels()[inst];
        for (int inst2=0; inst2<data->size(); inst2++)
        {
            if (inst!=inst2 && text.compare(data->labels()[inst2])==0)
            {
                other++;
            }
        }
        if (other==0)
            continue;

        int *rank = new int[other];//(int*)malloc(NRelevantsPerQuery[i]*sizeof(int));
        int Nrelevants = 0;
        float ap=0;

        float bestS=-99999;
        vector<float> scores(data->size());// = spot(data->image(inst),text,hy); //scores
        for (int j=0; j < data->size(); j++)
        {
            scores[j] = score_preThreshed(binarized[inst],binarized[j]);
        }

        for (int j=0; j < data->size(); j++)
        {
            float s = scores[j];
            //cout <<"score for "<<j<<" is "<<s<<". It is ["<<data->labels()[j]<<"], we are looking for ["<<text<<"]"<<endl;
            /* Precision at 1 part */
            /*if (inst!=j && s > bestS)
            {
                bestS = s;
                p1 = text==data->labels()[j];
                //bestIdx[inst] = j;
            }*/
            /* If it is from the same class and it is not the query idx, it is a relevant one. */
            /* Compute how many on the dataset get a better score and how many get an equal one, excluding itself and the query.*/
            if (text.compare(data->labels()[j])==0 && inst!=j)
            {
                int better=0;
                int equal = 0;

                for (int k=0; k < data->size(); k++)
                {
                    if (k!=j && inst!=k)
                    {
                        float s2 = scores[k];
                        if (s2> s) better++;
                        else if (s2==s) equal++;
                    }
                }


                rank[Nrelevants]=better+floor(equal/2.0);
                Nrelevants++;
            }

        }
        qsort(rank, Nrelevants, sizeof(int), sort_xxx);

        //pP1[i] = p1;

        /* Get mAP and store it */
        for(int j=0;j<Nrelevants;j++){
            /* if rank[i] >=k it was not on the topk. Since they are sorted, that means bail out already */

            float prec_at_k =  ((float)(j+1))/(rank[j]+1);
            //mexPrintf("prec_at_k: %f\n", prec_at_k);
            ap += prec_at_k;
        }
        ap/=Nrelevants;

        #pragma omp critical (storeMAP)
        {
            queryCount++;
            map+=ap;
        }

        delete[] rank;
    }

    cout<<"map: "<<(map/queryCount)<<endl; 
}
