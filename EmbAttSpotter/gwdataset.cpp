#include "gwdataset.h"

GWDataset::GWDataset(const string& queries, const string& imDir, int margin)
{
    string extension = queries.substr(queries.find_last_of('.')+1);
    bool gtp=extension.compare("gtp")==0;

    ifstream fileQueries(queries);
    assert(fileQueries.good());
    //2700270.tif 519 166 771 246 orders
    string line;
    regex qExtGtp("(\\S+\\.\\S+) (\\d+) (\\d+) (\\d+) (\\d+) (\\w+)");
    regex qExt("(.+\\.\\S+) (\\w+)");
    
    
    
    string curPathIm="";
    Mat curIm;
    
    while (getline(fileQueries,line))
    {
        if (line.length()==0)
            continue;
        smatch sm;
        Mat patch;
        string label;
        if (gtp)
        {
            regex_search(line,sm,qExtGtp);

            string pathIm=imDir+string(sm[1]);
            pathIms.push_back(pathIm);
            
            if (curPathIm.compare(pathIm)!=0)
            {
                curPathIm=pathIm;
                curIm = imread(curPathIm,CV_LOAD_IMAGE_GRAYSCALE);
            }
            int x1=max(1,stoi(sm[2])-margin)-1;
            int x2=min(curIm.cols,stoi(sm[4])+margin)-1;
            int y1=max(1,stoi(sm[3])-margin)-1;
            int y2=min(curIm.rows,stoi(sm[5])+margin)-1;
            Rect loc(x1,y1,x2-x1+1,y2-y1+1);
            locs.push_back(loc);
            patch = curIm(loc);
            assert(patch.rows*patch.cols>1);
            label=string(sm[6]);
        }
        else
        {
            regex_search(line,sm,qExt);
            patch=imread(imDir+string(sm[1]),CV_LOAD_IMAGE_GRAYSCALE);
            assert(patch.rows*patch.cols>1);
            label=string(sm[2]);
        }
        patch = preprocess(patch);        
        wordImages.push_back(patch);
        _labels.push_back(label);
    }

}

Mat GWDataset::preprocess(Mat im)
{
    int maxH=80;
    int minH=80;
    Mat patch;
    im.convertTo(patch,CV_32F);
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
}

const vector<string>& GWDataset::labels() const
{
    return _labels;
}
int GWDataset::size() const
{
    return _labels.size();
}
const Mat GWDataset::image(unsigned int i) const
{
    return wordImages.at(i);
}
/*
int xa=min(stoi(sm[2]),stoi(sm[3]));
        int xb=max(stoi(sm[2]),stoi(sm[3]));
        int ya=min(stoi(sm[4]),stoi(sm[5]));
        int yb=max(stoi(sm[4]),stoi(sm[5]));
        
        int x1=max(0,xa);
        int x2=min(curIm.cols-1,xb);
        int y1=max(0,ya);
        int y2=min(curIm.rows-1,yb);
*/
