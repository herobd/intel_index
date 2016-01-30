#include "segmentation.h"

void Segmentation::detectTextZones(const Mat& orig, bool VIZ)
{
    //line based
    Mat bin = Binarization::ntirogiannisBinarization(orig,0,1);
    double AH = getAverageCharHeight(bin);
    Mat smoothedV = RLSA_V(bin,10);
    if (VIZ)
        {Binarization::imshowB("smoothedV",smoothedV,255,0); waitKey();}
    Mat fuzzyV = fuzzyRLSA_V(smoothedV);
    if (VIZ)
        {Binarization::imshowB("fuzzyV",fuzzyV,255,0); waitKey();}
    Mat vertLines = Binarization::otsuBinarization(fuzzyV);
    if (VIZ)
        {Binarization::imshowB("vertLines",vertLines,255,0); waitKey();}
    vector<int> profile = squaredVertProfile(vertLines);
    double T = pow(10*AH,2);
    vector< pair<int,int> > candidatesL = findTextAreaCandidatesLines(profile,T);
    
    //whitespace based
    vector<int> whiteRunCounts = getWhiteRunCounts(bin,bin.rows/3);
    vector< pair<int,int> > candidatesW = findTextAreaCandidatesWhitespace(whiteRunCounts);
    
    
    //sort(cadidates.begin(),cadidates.end(),[](const pair<int,int>& a, const pair<int,int>& b) -> bool { return a.second-a.first < b.second-b.first; });
    
    vector< pair<int,int> > verticleTextAreas = processCandidates(candidatesL,candidatesW);
    //TODO check against previous images' text areas
    
    Mat vertLinesCleaned = removeLines_V(bin, vertLines);
    if (VIZ)
        {Binarization::imshowB("vertLinesCleaned",vertLinesCleaned,255,0); waitKey();}
    
    Mat rotated = vertLinesCleaned.t();
    
    Mat smoothedH = RLSA_V(rotated,10);
    Mat fuzzyH = fuzzyRLSA_V(smoothedH);
    Mat horzLines = Binarization::?(fuzzyH);
    Mat horzLinesCleaned = removeLines_V(rotated, horzLines);
    
    Mat linesCleaned = horzLinesCleaned.t();
    {Binarization::imshowB("linesCleaned",linesCleaned,255,0); waitKey();}
}

Mat Segmentation::RLSA_V(const Mat& bin, int C)
{
    //if two 'on' pixels in a column are seperated by C or less 'off' pixels, turn those pixels on
    Mat ret=bin.clone();
    int lastOn;
    for (int c=0; c<bin.cols; c++)
    {
        lastOn=-1;
        for (int r=0; r<bin.rows; r++)
        {
            if (bin.at<unsigned char>(r,c)==1)
            {
                if (r-lastOn<(C+1))
                {
                    for (int rf=r-1; rf>lastOn; rf--)
                        ret.at<unsigned char>(rf,c)=1;
                }
                lastOn=r;
            }
        }
        if (bin.rows-lastOn<(C+1))
        {
            for (int rf=bin.rows-1; rf>lastOn; rf--)
                ret.at<unsigned char>(rf,c)=1;
        }
    }
}

Mat Segmentation::fuzzyRLSA_V(const Mat& bin?, int C)
{
    //value of each pixel is fuzzy runlength in vert dir
    Mat conn(bin.rows,bin.cols,CV_32S);
    int maxRun=0;
    for (int c=0; c<bin.cols; c++)
    {
        int runLen=0;
        int runCount=0;
        int spaceLen=0;
        for (int r=0; r<bin.rows; r++)
        {
            if (bin.at<unsigned char>(r,c)==1)
            {
                runLen++;
                runCount++;
                spaceLen=0;
            }
            else
            {
                if (runLen>0)
                {   //cont run
                    runLen++;
                    if (++spaceLen>C)
                    {   //end run
                        for (int rf=r; rf>r-spaceLen; rf--)
                            conn.at<unsigned char>(rf,c)=0;
                        for (int rf=r-spaceLen; rf>r-runLen rf--)
                            conn.at<unsigned char>(rf,c)=runCount;
                        if (runCount>maxRun)
                            maxRun=runCount;
                        runLen=0;
                        runCount=0;
                    }
                }
                else
                {   //no run
                    conn.at<unsigned char>(rf,c)=0;
                }
            }
        }
        if (runLen>0)
        {   //end run
            for (int rf=bin.rows-1; rf>bin.rows-runLen rf--)
                conn.at<unsigned char>(rf,c)=runCount;
            if (runCount>maxRun)
                maxRun=runCount;
        }
    }
    Mat ret(bin.rows,bin.cols,CV_8U);
    for (int c=0; c<bin.cols; c++)
    {
        for (int r=0; r<bin.rows; r++)
            ret.at<unsigned char>(r,c)=255*(conn.at<unsigned char>(r,c)/(0.0+maxRun));
    }
    return ret;
}

vector<int> Segmentation::squaredVertProfile(const Mat& vertLines)
{
    vector<int> ret(vertLines.cols);
    for (int x=0; x<vertLines.cols; x++)
    {
        int runLen=0;
        for (int y=0; y<vertLines.rows; y++)
        {
            if (vertLines.at<unsigned char>(y,x)==1)
            {
                runLen++;
            }
            else if (runLen>0)
            {
                ret[x]+=runLen*runLen;
                runLen=0;
            }
        }
    }
    return ret;
}

vector< pair<int,int> > Segmentation::findTextAreaCandidatesLines(vector<int> profile, int T)
{
    vector< pair<int,int> >ret;
    cout <<"findTextAreaCandidatesLines() not implemented"<<endl;
    return ret;
}

vector<int> Segmentation::getWhiteRunCounts(const Mat& bin, int thresh)
{
    vector<int> ret;
    cout <<"getWhiteRunCounts() not implemented"<<endl;
    return ret;
}

vector< pair<int,int> > Segmentation::processCandidates(const vector< pair<int,int> >& candidatesL, const vector< pair<int,int> >& candidatesW)
{
    vector< pair<int,int> >ret;
    cout <<"processCandidates() not implemented"<<endl;
    return ret;
}



Mat Segmentation::removeLines_V(const Mat& bin, const Mat& vertLines)
{
    Mat skel = Binarization::LeeChenSkel(vertLines);
    map<int,double> ccWidthSum;
    map<int,int> ccWidthCount;
    auto newCC_skel = [&ccWidthSum, &ccWidthCount, &bin](int ccIndex, int r, int c) {
        ccWidthSum[ccIndex]=computeStrokeWidth(r,c,bin);
        ccWidthCount[ccIndex]=1;
    };
    auto addToCC_skel = [&ccWidthSum, &ccWidthCount, &bin](int ccI, int r, int c) {
        ccWidthSum[ccI]+=computeStrokeWidth(r,c,bin);
        ccWidthCount[ccI]++;
    };
    auto mergeCC_skel = [&ccWidthSum, &ccWidthCount, &bin](int mergeTo, int toMerge, int r, int c) {
        ccWidthSum[mergeTo]+=ccWidthSum[toMerge];
        ccWidthCount[mergeTo]+=ccWidthCount[toMerge];
    };
    tuple< Mat,map<int,int> > ccA_skel = Binarization::connComp(skel,newCC_skel,addToCC_skel,mergeCC_skel);
    Mat cc_skel = get<0>(ccA_skel);
    map<int,int> ccMap_skel = get<1>(ccA_skel);
    
    map<int,double> ccWidthAvg;
    for (ccP : cc_reg)
    {
        if (ccMap_reg[ccP.first]==0)
        {
        
        }
    }
    
    map<int, list<Point> > points;
    mat<int,int> regToSkel;
    auto newCC_reg = [&points, &regToSkel, &cc_skel, &ccMap_skel](int ccIndex, int r, int c) {
        points[ccIndex].push_back(Point(c,r));
        int ccI_skel = cc_skel.at<unsigned char>(r,c);
        if (ccI_skel != 0)
        {
            while (ccMap_skel[ccI_skel] !=0)
            {
                ccI_skel = ccMap_skel[ccI_skel];
            }
            regToSkel[ccIndex]=ccI_skel;
        }
    };
    auto addToCC_reg = [&points, &regToSkel, &cc_skel, &ccMap_skel](int ccI, int r, int c) {
        points[ccI].push_back(Point(c,r));
        if (regToSkel.find(ccI)==regToSkel.end())
        {
            int ccI_skel = cc_skel.at<unsigned char>(r,c);
            if (ccI_skel != 0)
            {
                while (ccMap_skel[ccI_skel] !=0)
                {
                    ccI_skel = ccMap_skel[ccI_skel];
                }
                regToSkel[ccI]=ccI_skel;
            }
        }
    };
    auto mergeCC_reg = [&points](int mergeTo, int toMerge, int r, int c) {
        points[mergeTo].insert(points[mergeTo].end(),points[toMerge].begin(),points[toMerge].end());
    };
    tuple< Mat,map<int,int> > ccA_reg = Binarization::connComp(skel,newCC_reg,addToCC_reg,mergeCC_reg);
    Mat cc_reg = get<0>(ccA_reg);
    map<int,int> ccMap_reg = get<1>(ccA_reg);
    
    
    
    for (ccP : cc_reg)
    {
        if (ccMap_reg[ccP.first]==0)
        {
            //cv fitline
            //scan horiz runs on line (only on cc)
            //remove those that are shorter to, equal, or slightly larger than to avg width
        }
    }
}
