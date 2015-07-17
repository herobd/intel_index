#include "liang.h"

Liang::Liang()
{
    trained=false;
}

//We assume img is a binary image
double Liang::score(string query, const Mat &img)
{
    float minWidth=0;
    float maxWidth =0;
    for (char c : query)
    {
        minWidth+=charWidthAvgs[c] - 2*charWidthStdDevs[2];
        maxWidth+=charWidthAvgs[c] + 2*charWidthStdDevs[2];
    }
    if (img.cols < minWidth || img.cols > maxWidth)
        return BAD_SCORE;
    
    vector<Grapheme*> graphemes = extractGraphemes(img);
    map<const Grapheme*,int> winningNodes(graphemes.size());
    for (const Grapheme* g: graphemes)
    {
        winningNodes[g] = MOG.getWinningNode(g);
    }
    vector<list<const Grapheme*> > segmentation = learnCharacterSegmentation(query,img,graphemes);
    
    double ret=0;
    for (int k=0; k<query.size(); k++)
    {
        for (const Grapheme* g : segmentation[k])
        {
            double maxsubscore=0;
            for (int i=0; i<MOG.getNumClasses(); i++)
            {
                double subscore = graphemeSpectrums[query[k]][i]/MOG.boxdist(i,winningNodes[g]);
                if (subscore>maxsubscore) maxsubscore=subscore;
            }
            ret+=maxsubscore;
        }
    }
    return ret;
}

void Liang::trainCharacterModels(string imgDirPath, string imgNamePattern, string imgExt, string annotationsPath)
{
    ifstream annotationsFile;
    annotationsFile.open (annotationsPath, ios::in);
    string word;
    regex clean("[^a-zA-Z]");
    
    
    map<char, list<int> > charWidths;
    map<char, list<Grapheme*> > charGraphemes;
    map<char, int> charCounts;
    
    int wordCount=1;
    while (getline (annotationsFile,word))
    {
        Mat img(imgDirPath + imgNamePattern + to_string(wordCount++) + imgExt);
        string query = regex_replace(word,clean,"");
        vector<Grapheme*> graphemes = extractGraphemes(img);
        learnCharacterSegmentation(query,graphemes,img,&charGraphemes,&charWidths, &charCounts);
    }
    
    MOG.train(charGraphemes);
    
    for (char c='A'; c<='z'; c++)
    {
        if (c > 'Z' && c < 'a') c = 'a';
        
        if (charGraphemes[c].size()==0)
        {
            cout << "No example found for" << c <<endl;
            continue;
        }
        
        graphemeSpectrums[c].resize(getNumClasses());
        for (const Grapheme* g : charGraphemes[c])
        {
            
            graphemeSpectrums[c][MOG.getWinningNode(g)]+=1;
            delete g;
        }
        
        for (float &count : graphemeSpectrums[c])
            count/=charCount[c];
        
        int sum=0;
        for (int width : charWidths[c])
        {
            sum+=width;
        }
        charWidthAvgs[c] = sum/(float)charWidths[c].size();
        int variance=0;
        for (int width : charWidths[c])
        {
            variance+=(charWidthAvgs[c]-width)*(charWidthAvgs[c]-width);
        }
        variance /= charWidths[c].size();
        charWidthStdDevs[c] = sqrt(variance);
    }
    trained=true;
}


vector<Grapheme*> Liang::extractGraphemes(const Mat &img)
{
    
    Mat skel;
    thinning(img,skel);
    Mat graphemes = breakSegments(skel);
    int count = repairLoops(graphemes);
    vector<Grapheme*> ret; //= makeGraphemes(graphemes,count);
    for (int tag=1; tag<=count; tag++)
    {
        ret.push_back(new Grapheme(graphemes,tag));
    }
    return ret;
}



vector<list<const Grapheme*> > Liang::learnCharacterSegmentation(string query, vector<Grapheme*> graphemes, const Mat &img, map<char, list<Grapheme*> >* charGraphemes, map<char, list<int> >* accumWidths, map<char, int>* charCounts)
{
    vector<int> charWidths(graphemes.size());
    double inc = img.cols/(double)query.size();
    charWidths[0]=inc;
    for (int c=1; c<graphemes.size(); c++)
    {
        inc += img.cols/(double)query.size();
        charWidths[c] = inc - charWidths[c-1];
        
    }
        
    int upperBaseline, lowerBaseline;
    upperBaseline = findUpperBaseline(img,hasAscender(query));
    lowerBaseline = findLowerBaseline(img,hasDescender(query));
    bool change=true;
    vector<list<const Grapheme*> > characters(query.size()); 
    
    
    auto findAndAddToNearest = [&] (bool ascender){
        int currentX=0;
        int bestC;
        int bestDist=9999;
        for (int c=0; c<query.size(); c++)
        {
            int prevX=currentX;
            currentX += charWidths[c];
            if ((ascender&&isAscender(query[c])) || (!ascender&&isDescender(query[c])))
            {
                if (centriod.x>=prevX && centriod.x<=currentX)
                {
                    bestC=c;
                    break;
                }
                else
                {
                    int dist = min(abs(centriod.x-currentX), abs(centriod.x-prevX));
                    if (dist<bestDist)
                    {
                        bestDist=dist;
                        bestC=c;
                    }
                }
            }
        }
        if (find(characters[bestC].begin(),characters[bestC].end(),graphemes[g])==characters[bestC].end())
        {
            for (int c2=0; c2<query.size(); c2++)
            {
                characters[c2].remove(graphemes[g]);
            }
            characters[bestC].push_back(graphemes[g]);
            change=true;
        }
    };
    
    for (int i=0; change && i< MAX_ITER_CHAR_SEG; i++)
    {
        change=false;
        for (int g=0; g<graphemes.size(); g++)
        {
            
            Point centriod = findCentriod(graphemes[g]);
            if (graphemes[g]->maxY() < upperBaseline)
            {
                findAndAddToNearest(true);
            }
            else if (graphemes[g]->minY() > lowerBaseline)
            {
                findAndAddToNearest(false);
            }
            else
            {
                int currentX=0;
                for (int c=0; c<query.size(); c++)
                {
                    currentX += charWidths[c];
                    if (centriod.x < currentX &&
                            find(characters[c].begin(),characters[c].end(),graphemes[g])==characters[c].end())
                    {
                        for (int c2=0; c2<query.size(); c2++)
                        {
                            characters[c2].remove(graphemes[g]);
                        }
                        characters[c].push_back(graphemes[g]);
                        change=true;
                        
                        break;   
                    }
                    
                }
            }
        }
        
        for (int c=0; c<query.size(); c++)
        {
            int minX=9999;
            int maxX=0;
            for (const Grapheme* g : characters[c])
            {
                int gMinX=g->minXBetween(upperBaseline,lowerBaseline);
                int gMaxX=g->maxXBetween(upperBaseline,lowerBaseline);
                if (gMinX < minX) minX=gMinX;
                if (gMaxX < maxX) maxX=gMaxX;
            }
            charWidth[c]=maxX-minX;
        }
    }
    
    for (int c=0; c<query.size(); c++)
    {
        if (accumWidths!=NULL) (*accumWidths)[query[c]].push_back(charWidth[c]);
        if (charGraphemes!=NULL) (*charGraphemes)[query[c]].insert((*charGraphemes)[query[c]].end(),characters[c].begin(),characters[c].end());
        if (charCounts!=NULL) (*charCounts)[query[c]]++;
    }
    
    return characters;
}

void Liang::saveCharacterModels(string filePath)
{
    assert(trained);
    ofstream file;
    file.open (filePath, ios::out);
    file << graphemeSpectrums['a'].size();
    for (int c='A'; c<='z'; c++)
    {
        if (c > 'Z' && c < 'a') c = 'a';
        file << c << "," << charWidthAvgs[c] << "," << charWidthStdDevs[c];
        
        for (float v : graphemeSpectrums[c])
            file << "," << v;
        
        file << endl;
    }
    
    file.close();
    MOG.save("MOG_"+filePath);
}

void Liang::loadCharacterModels(string filePath)
{
    regex charRGX("[a-zA-Z]");
    regex numberRGX("\\d+(\\.\\d+)?");
    
    ifstream file;
    file.open (filePath, ios::in);
    string line;
    getline (file,line);
    int spectrumSize = stoi(line);
    
    
    while (getline(file,line))
    {
        
        smatch sm;
        regex_search(line,sm,charRGX);   
        char c = sm[0][0];
        line = sm.suffix().str();
        
        regex_search(line,sm,numberRGX);   
        charWidthAvgs[c] = stof(sm[0]);
        line = sm.suffix().str();
        
        regex_search(line,sm,numberRGX);   
        charWidthStdDevs[c] = stof(sm[0]);
        line = sm.suffix().str();
        
        
        while(regex_search(line,sm,numberRGX))
        {
            int v = stof(sm[0]);
            graphemeSpectrums[c].push_back(v);
            
//            for (auto x:sm) std::cout << x << " ";
//            std::cout << std::endl;
            line = sm.suffix().str();
        }
        assert(graphemeSpectrums[c].size() == spectrumSize);
    }
    
    MOG.load("MOG_"+filePath);
    trained=true;
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
 * 		src  The source image, binary with range = [0,255]
 * 		dst  The destination image
 */
void Liang::thinning(const cv::Mat& src, cv::Mat& dst)
{
    dst = src.clone();
    dst /= 255;         // convert to binary image

    cv::Mat prev = cv::Mat::zeros(dst.size(), CV_8UC1);
    cv::Mat diff;

    do {
        thinningIteration(dst, 0);
        thinningIteration(dst, 1);
        cv::absdiff(dst, prev, diff);
        dst.copyTo(prev);
    } 
    while (cv::countNonZero(diff) > 0);

    dst *= 255;
}
/////////////////////////////////////////////////////////


Mat Liang::breakSegments(const Mat& skel)
{
    Mat ret = skel.clone();
    
    list<Point> startingPointQueue;
    startingPointQueue.push_back(??);
    int curLabel=1;
    
    while(!startingPointQueue.empty())
    {
        Point cur=startingPointQueue.front();
        startingPointQueue.pop_front();
        int curRunLen=0;
        if (ret.at<unsigned char>(cur)==254)
        while(1)
        {
            Point next(-1,-1);
            char count =0;
            ret.at<unsigned char>(cur)=curLabel;
            curRunLen++;
            for (int direction=0; direction<8; direction++)
            {
                
                
                int xDelta=1;
                if (direction>2 && direction<6) xDelta=-1;
                else if (direction==2 || direction==6) xDelta=0;
                int yDelta=0;
                if (direction>0 && direction<4) yDelta=-1;
                else if (direction>4 && direction<8) xDelta=1;
                
                int x = cur.x+xDelta;
                int y = cur.y+yDelta;
                
                if (ret.at<unsigned char>(y,x)==255)
                {
                    count++;
                    if (next.x==-1)
                    {
                        next.x=x;
                        next.y=y;
                    }
                    else
                        startingPointQueue.push_back(Point(x,y));
                    ret.at<unsigned char>(y,x)=254;
                }
                else if (ret.at<unsigned char>(y,x)>0)
                {
                    count++;
                }
            }
            if (count>1 && curRunLen>1)//runLen prevents single pixel graphemes in diabolic cases
            {
                startingPointQueue.push_back(next);
                ret.at<unsigned char>(y,x)=254;
                break;
            }
            else if (count==0)
                break;
            
            cur=next;
        }
    }
}
