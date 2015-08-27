#include "liang.h"

Liang::Liang()
{
    trained=false;
}

//We assume img is a binary image (values of 0=background or 255=foreground)
double Liang::score(string query, const Mat &img)
{
    assert(trained);
    
    int wordWidth=-1;
    int firstPixel=-1;
    for (int x=0; x<img.cols && firstPixel==-1; x++)
    {
        for (int y=0; y<img.rows; y++)
        {
            if (img.at<unsigned char>(y,x)>0)
            {
                firstPixel=x;
                break;
            }
        }
    }
    for (int x=img.cols-1; x>=0 && wordWidth==-1; x--)
    {
        for (int y=0; y<img.rows; y++)
        {
            if (img.at<unsigned char>(y,x)>0)
            {
                wordWidth=x-firstPixel;
                break;
            }
        }
    }
    float minWidth=0;
    float maxWidth =0;
    double unitWidth = wordWidth/(0.0+query.size());
    for (char c : query)
    {
        minWidth+=charWidthAvgs[c] - 2*unitWidth*unitCharWidthStdDevs[c];
        maxWidth+=charWidthAvgs[c] + 2*unitWidth*unitCharWidthStdDevs[c];
        if (graphemeSpectrums[c].size()==0)
        {
            cout << "No model for letter: " << c << endl;
            return BAD_SCORE;
        }
    }
    
    
    if (wordWidth < minWidth || wordWidth > maxWidth)
    {
        //cout << "img length [" << wordWidth << "] ouside of range [" << minWidth << " - " << maxWidth << "]"<<endl;
        return BAD_SCORE;
    }
    
    list<Point> localMaxs, localMins;
    int centerLine;
    Mat skel;
    thinning(img,skel);
    vector<Grapheme*> graphemes = extractGraphemes(skel,&localMaxs,&localMins,&centerLine);
    map<const Grapheme*,int> winningNodes;
    for (const Grapheme* g: graphemes)
    {
        winningNodes[g] = mog.getWinningNode(g);
    }
    vector<list<const Grapheme*> > segmentation = learnCharacterSegmentation(query,graphemes,localMaxs,localMins,centerLine, img);
    
    double ret=0;
    for (int k=0; k<query.size(); k++)
    {
        for (const Grapheme* g : segmentation[k])
        {
            double maxsubscore=0;
            for (int i=0; i<mog.getNumClasses(); i++)
            {
                double rxp = graphemeSpectrums[query[k]][i];
                assert(rxp<1000);
                double subscore = rxp/(mog.boxdist(i,winningNodes[g])+1);
                if (subscore>maxsubscore) maxsubscore=subscore;
            }
            ret+=maxsubscore;
            delete g;
        }//seems to bias towards longer images
//        ret/=segmentation[k].size();
    }
    return ret;
}

void Liang::trainCharacterModels(string imgDirPath, string imgNamePattern, string imgExt, string annotationsPath)
{
    
    ifstream annotationsFile;
    annotationsFile.open (annotationsPath, ios::in);
    string word;
    regex clean("[^a-zA-Z0-9]");
    
    
    int wordCount=1;
    
    vector< pair<int,string> > examples;
    while (getline (annotationsFile,word))
    {
        string query = regex_replace(word,clean,"");
        examples.push_back(pair<int,string>(wordCount++,query));
    }
    trainCharacterModels(imgDirPath,imgNamePattern,imgExt,examples);
}

void Liang::trainCharacterModels(string imgDirPath, string imgNamePattern, string imgExt, const vector< pair<int,string> >& examples)
{
    if (imgDirPath[imgDirPath.size()-1] != '/') imgDirPath += "/";
    if (imgExt[0] != '.') imgExt = "."+imgExt;
    
    
    map<char, list<int> > charWidths;
    map<char, list<double> > unitCharWidths;
    map<char, list<const Grapheme*> > charGraphemes;
    map<char, int> charCounts;
    
    omp_lock_t writelock;
    omp_init_lock(&writelock);
    
#pragma omp parallel
    #pragma omp single
    for (pair<int,string> ex : examples)
    {
        #pragma omp task firstprivate(ex)
        {
            string imgPath=imgDirPath + imgNamePattern + to_string(ex.first) + imgExt;
            Mat img= imread(imgPath,0);
            threshold(img,img,IMAGE_THRESH,255,1);
            Mat skel;
            thinning(img,skel);
            list<Point> localMaxs, localMins;
            int centerLine;
            vector<Grapheme*> graphemes = extractGraphemes(skel, &localMaxs, &localMins, &centerLine);
            
            
            learnCharacterSegmentation(ex.second,graphemes,localMaxs,localMins,centerLine,img,&charGraphemes,&charWidths, &unitCharWidths, &charCounts, &writelock);
        }
    }
    cout << "Beginning MOG training" << endl;
    mog.train(charGraphemes,400);//500
    showMOG(charGraphemes);
    
    
    for (char c='0'; c<='z'; c++)
    {
        if (c > '9' && c < 'A') c = 'A';
        if (c > 'Z' && c < 'a') c = 'a';
        
        if (charGraphemes[c].size()==0)
        {
            cout << "No example found for " << c <<endl;
            continue;
        }
        
        graphemeSpectrums[c].resize(mog.getNumClasses());
        for (const Grapheme* g : charGraphemes[c])
        {
            
            graphemeSpectrums[c][mog.getWinningNode(g)]+=1;
            delete g;
        }
        
        for (float &count : graphemeSpectrums[c])
            count/=charCounts[c];
        
        int sum=0;
        for (int width : charWidths[c])
        {
            sum+=width;
        }
        double sumUnit=0;
        for (int width : unitCharWidths[c])
        {
            sumUnit+=width;
        }
        charWidthAvgs[c] = sum/(float)charWidths[c].size();
        double unitCharWidthAvg = sumUnit/unitCharWidths[c].size();
        double variance=0;
        for (int width : unitCharWidths[c])
        {
            variance+=(unitCharWidthAvg-width)*(unitCharWidthAvg-width);
        }
        variance /= unitCharWidths[c].size();
        unitCharWidthStdDevs[c] = sqrt(variance);
    }
    trained=true;
}

void Liang::showCharacterModels(string imgDirPath, string imgNamePattern, string imgExt, const vector< pair<int,string> >& examples, string modelMOG)
{
    cout << "show mog" << endl;
    
    if (imgDirPath[imgDirPath.size()-1] != '/') imgDirPath += "/";
    if (imgExt[0] != '.') imgExt = "."+imgExt;
    
    
    map<char, list<int> > charWidths;
    map<char, list<double> > unitCharWidths;
    map<char, list<const Grapheme*> > charGraphemes;
    map<char, int> charCounts;
    
    omp_lock_t writelock;
    omp_init_lock(&writelock);
    
#pragma omp parallel
    #pragma omp single
    for (pair<int,string> ex : examples)
    {
        #pragma omp task firstprivate(ex)
        {
            string imgPath=imgDirPath + imgNamePattern + to_string(ex.first) + imgExt;
            Mat img= imread(imgPath,0);
            threshold(img,img,IMAGE_THRESH,255,1);
            Mat skel;
            thinning(img,skel);
            list<Point> localMaxs, localMins;
            int centerLine;
            vector<Grapheme*> graphemes = extractGraphemes(skel, &localMaxs, &localMins, &centerLine);
            
            
            learnCharacterSegmentation(ex.second,graphemes,localMaxs,localMins,centerLine,img,&charGraphemes,&charWidths, &unitCharWidths, &charCounts, &writelock);
        }
    }
    
    mog.load(modelMOG);
    showMOG(charGraphemes);
}

void Liang::showMOG(const map<char, list<const Grapheme*> >& charGraphemes)
{
    
    //take samples from graphemes and display where they end up in an image 
    int boxSize=400;
    int numEx=4;
    Mat img(9*boxSize,9*boxSize,CV_8UC3);
    for (int i=boxSize-1; i<9*boxSize; i+=boxSize)
    {
        line(img,Point(0,i),Point(9*boxSize-1,i),Scalar(255));
        line(img,Point(i,0),Point(i,9*boxSize-1),Scalar(255));
    }
    
    vector<int> counts(9*9);
    bool cont=true;
    map<char, map<int,int> > countUse;
    for(int ii=0; ii<81*300 && cont; ii++)
    {
        char c = 'a' + rand()%26;
        int g = rand()%charGraphemes.at(c).size();
        if (countUse[c][g]>0)
            continue;
        countUse[c][g]++;
        
        auto iter = charGraphemes.at(c).begin();
        for (int i=0; i<g; i++)
            iter++;
        int node = mog.getWinningNode(*iter);
        if (counts[node]<numEx*numEx)
        {
            int col = mog.nodeCol(node);
            int row = mog.nodeRow(node);
            int xOff = (counts[node]/numEx)*(boxSize/numEx) + col*boxSize;
            int yOff = (counts[node]%numEx)*(boxSize/numEx) + row*boxSize;
            
            Vec3b color(rand()%256,rand()%256,rand()%256);
            
            const Mat* graphemes  = (*iter)->img();
            for (int i=(*iter)->minX(); i<=(*iter)->maxX(); i++)
                for (int j=(*iter)->minY(); j<=(*iter)->maxY(); j++)
                {
                    if (graphemes->at<unsigned char>(j,i)==(*iter)->imgId())
                    {
                        int x = xOff + i-(*iter)->minX();
                        int y = yOff + j-(*iter)->minY();
                        if (x<img.cols && y<img.rows)
                            img.at<Vec3b>(y, x) = color;
                    }
                }
            
            counts[node]++;
        }
        
        cont = false;
        for (int count : counts)
        {
            if (count<numEx*numEx)
            {
                cont=true;
                break;
            }
        }
        
    }
    
    
    imshow("MOG",img);
    waitKey();
    imwrite("MOG.png",img);
}

vector<Grapheme*> Liang::extractGraphemes(const Mat &skel, list<Point>* localMaxs, list<Point>* localMins, int* centerLine)
{
    
    
    Mat graphemes = breakSegments(skel,localMaxs,localMins,centerLine);
    list<int> tags = repairLoops(graphemes);
    vector<Grapheme*> ret; //= makeGraphemes(graphemes,count);
    for (int tag : tags)
    {
        ret.push_back(new Grapheme(graphemes,tag));
    }
    return ret;
}



vector<list<const Grapheme*> > Liang::learnCharacterSegmentation(string query, const vector<Grapheme*>& graphemes, const list<Point>& localMaxs, const list<Point>& localMins, int centerLine, const Mat& img, map<char, list<const Grapheme*> >* charGraphemes, map<char, list<int> >* accumWidths, map<char, list<double> >* unitCharWidths, map<char, int>* charCounts, omp_lock_t *writelock)
{
    
        
    int upperBaseline, lowerBaseline;
    findBaselines(hasAscender(query),hasDescender(query),&upperBaseline,&lowerBaseline,localMins,localMaxs,centerLine,img);
    
    int leftX=9999;
    int rightX=0;
    for (Grapheme* g : graphemes)
    {
//        int gMax = g->maxX();
//        int gMin = g->minX();
        int gMax = g->maxXBetween(upperBaseline,lowerBaseline);
        int gMin = g->minXBetween(upperBaseline,lowerBaseline);
        if (gMax>rightX) rightX=gMax;
        if (gMin<leftX) leftX=gMin;
    }
    assert(leftX!=9999 && rightX!=0);
    int width = 1+rightX-leftX;
    double unitWidth = width/(0.0+query.size());
    
    vector<int> charWidths(query.size());
    double inc = width/(double)query.size();
    charWidths[0]=inc;
    int soFar=inc;
    for (int c=1; c<query.size(); c++)
    {
        inc += width/(double)query.size();
        charWidths[c] = inc - soFar;
        soFar+=charWidths[c];
    }
    
    bool change=true;
    vector<list<const Grapheme*> > characters(query.size()); 
    
    int g;
    Point2f centriod;//&leftX,&query,&charWidths,&centriod,&characters,&graphemes,&change,&g
    auto findAndAddToNearest = [&] (bool ascender){
        int currentX=leftX;
        int bestC=-1;
        int bestDist=9999;
        for (int c=0; c<query.size(); c++)
        {
            int prevX=currentX;
            currentX += charWidths[c];
            if ((ascender&&isAscender(query[c])) || (!ascender&&isDescender(query[c])))
            {
                if ((centriod.x>=prevX && centriod.x<=currentX) || (c==0 && centriod.x<=currentX))
                {
                    bestC=c;
                    break;
                }
                else
                {
                    int dist = std::min(abs(centriod.x-currentX), abs(centriod.x-prevX));
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
        for (g=0; g<graphemes.size(); g++)
        {
            
            centriod = graphemes[g]->centriod();
            if (graphemes[g]->maxY() < upperBaseline && hasAscender(query))
            {
                findAndAddToNearest(true);
            }
            else if (graphemes[g]->minY() > lowerBaseline && hasDescender(query))
            {
                findAndAddToNearest(false);
            }
            else
            {
                int currentX=leftX;
                for (int c=0; c<query.size(); c++)
                {
                    currentX += charWidths[c];
                    if ((centriod.x <= currentX) || (c==query.size()-1))
                    {
//                        if (centriod.x > currentX)
//                            cout << "[" << query << "] grapheme " << g << " before the end on iter " << i << endl;
                        
                        if (find(characters[c].begin(),characters[c].end(),graphemes[g])==characters[c].end())
                        {
                            for (int c2=0; c2<query.size(); c2++)
                            {
                                characters[c2].remove(graphemes[g]);
                            }
                            characters[c].push_back(graphemes[g]);
                            change=true;
                        }
                        
                        break;   
                    }
                    
                }
            }
        }
        
        
        
        adjustCharWidths(characters, query, upperBaseline, lowerBaseline, leftX, charWidths);
    }
    
    if (writelock!=NULL)
        omp_set_lock(writelock);
    for (int c=0; c<query.size(); c++)
    {
        if (accumWidths!=NULL) (*accumWidths)[query[c]].push_back(charWidths[c]);
        if (unitCharWidths!=NULL) (*unitCharWidths)[query[c]].push_back(charWidths[c]/unitWidth);
        if (charGraphemes!=NULL) (*charGraphemes)[query[c]].insert((*charGraphemes)[query[c]].end(),characters[c].begin(),characters[c].end());
        if (charCounts!=NULL) (*charCounts)[query[c]]++;
    }
    if (writelock!=NULL)
        omp_unset_lock(writelock);
    
    return characters;
}

void Liang::findMinMaxInBaselines(const list<const Grapheme*>& graphemes, int upperBaseline, int lowerBaseline, int& min, int& max)
{
    if (graphemes.size() > 0)
    {
        min=9999;
        max=0;   
        for (const Grapheme* g : graphemes)
        {
            int gMinX=g->minXBetween(upperBaseline,lowerBaseline);
            int gMaxX=g->maxXBetween(upperBaseline,lowerBaseline);
            if (gMinX < min) min=gMinX;
            if (gMaxX > max) max=gMaxX;
        }
        if (min==9999)
        {
            for (const Grapheme* g : graphemes)
            {
                int gMinX=g->minX();
                int gMaxX=g->maxX();
                if (gMinX < min) min=gMinX;
                if (gMaxX > max) max=gMaxX;
            }
        }
    }
    else
    {
//        min=maxX;
//        max=maxX;
        assert(false);
    }
}

void Liang::adjustCharWidths(const vector<list<const Grapheme*> >& characters, const string& query, int upperBaseline, int lowerBaseline, int leftX, vector<int>& charWidths)
{
    int minX;
    int maxX;
    if (characters[0].size()!=0)
        findMinMaxInBaselines(characters[0],upperBaseline,lowerBaseline,minX,maxX);
    else
    {
        minX=leftX;
        maxX=-1;
    }
    int prevMinX=-1;
    int prevMaxX=minX;
    
    int nextMinX;
    int nextMaxX;   
    
    
    for (int c=1; c<query.size(); c++)
    {
        if (characters[c].size()!=0)
            findMinMaxInBaselines(characters[c],upperBaseline,lowerBaseline,nextMinX,nextMaxX);
        else
        {
            if(maxX==-1)//&&(charWidths[c]>0|| query.compare("28th")==0));
                maxX=minX;
            
            nextMinX=maxX;
            nextMaxX=-1;
            
        }
        if (maxX==-1)
        {
            maxX=nextMinX;
        }
        
        charWidths[c-1]=max(((maxX+nextMinX)/2)-((minX+prevMaxX)/2),0);
//            assert(charWidths[c-1]<100 || query.size()==1);
//            assert(charWidths[c-1]>0 || query.compare("28th")==0);
        
        prevMaxX=maxX;
        prevMinX=minX;
        maxX=nextMaxX;
        minX=nextMinX;
    }
    charWidths[query.size()-1]=max((maxX)-((minX+prevMaxX)/2),0);
//        assert(charWidths[query.size()-1]<100 || query.size()==1);
//        assert(charWidths[query.size()-1]>0 || query.compare("28th")==0);
}

void Liang::saveCharacterModels(string filePath)
{
    assert(trained);
    ofstream file;
    file.open (filePath, ios::out);
    file << graphemeSpectrums['a'].size() << endl;
    for (char c='0'; c<='z'; c++)
    {
        if (c > '9' && c < 'A') c = 'A';
        if (c > 'Z' && c < 'a') c = 'a';
        
        file << c << "," << charWidthAvgs[c] << "," << unitCharWidthStdDevs[c];
        
        for (float v : graphemeSpectrums[c])
            file << "," << v;
        
        file << endl;
    }
    
    file.close();
    mog.save(filePath+"_mog");
}



void Liang::loadCharacterModels(string filePath)
{
    regex charRGX("[a-zA-Z0-9]");
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
        string f = sm[0];
        char c = f[0];
        line = sm.suffix().str();
        
        regex_search(line,sm,numberRGX);   
        charWidthAvgs[c] = stof(sm[0]);
        line = sm.suffix().str();
        
        regex_search(line,sm,numberRGX);   
        unitCharWidthStdDevs[c] = stof(sm[0]);
        line = sm.suffix().str();
        
        
        while(regex_search(line,sm,numberRGX))
        {
            double v = stof(sm[0].str());
            graphemeSpectrums[c].push_back(v);
            
//            for (auto x:sm) std::cout << x << " ";
//            std::cout << std::endl;
            line = sm.suffix().str();
        }
        assert(graphemeSpectrums[c].size() == spectrumSize || graphemeSpectrums[c].size() == 0);
    }
    
    mog.load(filePath+"_mog");
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
    
    cleanNoiseFromEdges(dst);
}
/////////////////////////////////////////////////////////
void Liang::cleanNoiseFromEdges(Mat &skel)
{
    for (int x=0; x<5; x++)
        for (int y=0; y<skel.rows; y++)
            if (skel.at<unsigned char>(y,x)>0)
                scrubCC(skel,x,y);
    
    for (int y=0; y<5; y++)
        for (int x=0; x<skel.cols; x++)
            if (skel.at<unsigned char>(y,x)>0)
                scrubCC(skel,x,y);
    
    for (int x=skel.cols-5; x<skel.cols; x++)
        for (int y=0; y<skel.rows; y++)
            if (skel.at<unsigned char>(y,x)>0)
                scrubCC(skel,x,y);
    
    for (int y=skel.rows-5; y<skel.rows; y++)
        for (int x=0; x<skel.cols; x++)
            if (skel.at<unsigned char>(y,x)>0)
                scrubCC(skel,x,y);
}

void Liang::scrubCC(Mat& skel, int xStart, int yStart)
{
    vector<bool> visited(skel.cols*skel.rows);
    visited.assign(skel.cols*skel.rows,false);
    list<Point> toVisit;
    toVisit.push_back(Point(xStart,yStart));
    visited[xStart+yStart*skel.cols]=true;
    int count = 0;
    while (!toVisit.empty() && count<=SCRUB_THRESH)
    {
        Point cur = toVisit.back();
        toVisit.pop_back();
        
        for (int direction=0; direction<8; direction++)
        {
            int x = cur.x+xDelta(direction);
            int y = cur.y+yDelta(direction);
            if (x<0 || y<0 || x>=skel.cols || y>=skel.rows)
                continue;
            if (skel.at<unsigned char>(y,x)>0 && !visited[x+y*skel.cols])
            {
                if (++count > SCRUB_THRESH) break;
                visited[x+y*skel.cols]=true;
                toVisit.push_back(Point(x,y));
            }
        }
    }
    
    if (count <= SCRUB_THRESH)
    {
        toVisit.clear();
        toVisit.push_back(Point(xStart,yStart));
        while (!toVisit.empty())
        {
            Point cur = toVisit.back();
            toVisit.pop_back();
            
            for (int direction=0; direction<8; direction++)
            {
                int x = cur.x+xDelta(direction);
                int y = cur.y+yDelta(direction);
                if (x<0 || y<0 || x>=skel.cols || y>=skel.rows)
                    continue;
                if (skel.at<unsigned char>(y,x)>0)
                {
                    skel.at<unsigned char>(y,x)=0;
                    toVisit.push_back(Point(x,y));
                }
            }
        }
    }
}


Mat Liang::breakSegments(const Mat& skel, list<Point>* maxima, list<Point>* minima, int* centerLine)
{
    bool delMin=false;
    bool delMax=false;
    if (minima==NULL)
    {
	    delMin=true;
	    minima = new list<Point>();
    }
    if (maxima==NULL)
    {
           delMax=true;
           maxima = new list<Point>();
    }
    
    int sumY=0;
    int countPixels=0;
    
    Mat ret = skel.clone();
    int curLabel=0;
    int prevCCSize=0;
//    list<Point> breakPoints;
    for (int scanX=0; scanX<ret.cols; scanX++)
       for (int scanY=0; scanY<ret.rows; scanY++)
        {
           
           
            if (ret.at<unsigned char>(scanY,scanX)==UNMARKED_POINT)
            {
                int ccSize=0;
                list<Point> startingPointQueue;
                
                if (!branchPoint(scanX,scanY,ret,startingPointQueue,curLabel))
                { 
                        startingPointQueue.push_back(Point(scanX,scanY));
                        ret.at<unsigned char>(Point(scanX,scanY))=++curLabel;
                }
                else
                    ccSize++;
                
                int ccMax=9999;
                Point ccMaxPoint;
                int ccMin=0;
                Point ccMinPoint;
                
                while(!startingPointQueue.empty())
                {
                    Point cur=startingPointQueue.front();
                    startingPointQueue.pop_front();
                    
                    int thisLabel=ret.at<unsigned char>(cur);
                    
                    MinMaxTracker tracker(minima,maxima,curLabel,ret,SMALL_LOCAL_NEIGHBORHOOD);
                    MinMaxTracker tracker2(minima,maxima,curLabel,ret,MED_LOCAL_NEIGHBORHOOD);
                    MinMaxTracker tracker3(minima,maxima,curLabel,ret,BIG_LOCAL_NEIGHBORHOOD);
                    
                    //        if (ret.at<unsigned char>(cur)==254)
                    while(1)
                    {
                        ccSize++;
                        if (ret.at<unsigned char>(cur.y,cur.x)!=0)
                        {
                            if (cur.y<ccMax)
                            {
                                ccMax=cur.y;
                                ccMaxPoint = Point(cur.x,cur.y);
                            }
                            if (cur.y>ccMin)
                            {
                                ccMin=cur.y;
                                ccMinPoint = Point(cur.x,cur.y);
                            }
                        }
                        Point next(-1,-1);
                        char count =0;
                        //            ret.at<unsigned char>(cur)=START_POINT;
                        
                        //Check for branch points first
                        bool end=false;
                        for (int direction=0; direction<8; direction++)
                        {
                            int x = cur.x+xDelta(direction);
                            int y = cur.y+yDelta(direction);
                            if (x<0 || y<0 || x>=skel.cols || y>=skel.rows)
                                continue;
                            
                           if (ret.at<unsigned char>(y,x)==UNMARKED_POINT && branchPoint(x,y,ret,startingPointQueue,curLabel)){
                               end=true;
                               ccSize++;//as the branchPoint functiona handels all events of being on the branch point
                               if (direction%2==1)
                               {
                                   int sidex = cur.x+xDelta(mod(direction+1,8));
                                   int sidey = cur.y+yDelta(mod(direction+1,8));
                                   if (!(sidex<0 || sidey<0 || sidex>=ret.cols || sidey>=ret.rows) &&
                                           ret.at<unsigned char>(sidey,sidex)==UNMARKED_POINT)
                                       ret.at<unsigned char>(sidey,sidex)=thisLabel;
                                   
                                   sidex = cur.x+xDelta(mod(direction-1,8));
                                   sidey = cur.y+yDelta(mod(direction-1,8));
                                   if (!(sidex<0 || sidey<0 || sidex>=ret.cols || sidey>=ret.rows) &&
                                           ret.at<unsigned char>(sidey,sidex)==UNMARKED_POINT)
                                       ret.at<unsigned char>(sidey,sidex)=thisLabel;
                               }
                           }
                           
                        }
                        if (end)
                        {
                            tracker.track(cur,thisLabel);
                            tracker2.track(cur,thisLabel);
                            tracker3.track(cur,thisLabel);
                            break;
                        }
                        //if (end) 
			//{
			//	startingPointQueue.push_back(cur);
			//		
                        //	ret.at<unsigned char>(next)=thisLabel;
			//	break;
			//}
                        //for (int direction=0; direction<8; direction=direction!=6?direction+2:1)
                        for (int direction=1; direction<8; direction=direction!=7?direction+2:0)
                        {
                            int x = cur.x+xDelta(direction);
                            int y = cur.y+yDelta(direction);
                            if (x<0 || y<0 || x>=skel.cols || y>=skel.rows)
                                continue;
//                            if (end && (direction%2==1))
//                                continue;
                            if (ret.at<unsigned char>(y,x)==JUNCTION_POINT) 
                                count++;
                            if (!end && ret.at<unsigned char>(y,x)==UNMARKED_POINT)
                            {
                                if (next.x==-1)
                                {
                                    next.x=x;
                                    next.y=y;
                                    
                                }
                                else
                                {
                                    ret.at<unsigned char>(y,x) = thisLabel;
                                    
                                    if (cur.x==scanX && cur.y==scanY)
                                        startingPointQueue.push_back(Point(x,y));
                                }
                                
                            }
                            else if (cur.x==scanX && cur.y==scanY && 
                                     ret.at<unsigned char>(y,x)>0 && 
                                     ret.at<unsigned char>(y,x)!=JUNCTION_POINT)
                            {
                                ccSize+=prevCCSize;
                            }
//                            else if (end && ret.at<unsigned char>(y,x)>0 && ret.at<unsigned char>(y,x)!=JUNCTION_POINT)
//                            {//relabel our point that branchPoint() stole
//                                ret.at<unsigned char>(y,x) = thisLabel;
//                                startingPointQueue.push_back(Point(x,y));
//                            }
                                
                        }
                        
                       
                        tracker.track(cur,thisLabel);
                        tracker2.track(cur,thisLabel);
                        tracker3.track(cur,thisLabel);
                        
                        if (next.x==-1)
                        {
                            if (count==0)//relabel this chain to merge it
                            {
                                if (!relabelStrech(thisLabel,cur,ret,tracker,tracker2,tracker3))
                                {
                                    tracker.checkEnd();
                                    tracker2.checkEnd();
                                    tracker3.checkEnd();
                                }
                                
                            }
                            break;
                        }
                        
                        
                        
                        
                        
                        
//                        if (!findingStart)
                        ret.at<unsigned char>(next)=thisLabel;
                        cur=next;
                        
                    }
                }
                
                if (ccSize>5)
                {
                    if (!delMin)
                    {
                        for (const Point& p : *minima)
                        {
                            if (abs(p.y - ccMin)<5 && abs(p.x - ccMinPoint.x)<10)
                            {
                                ccMin=-1;
                                break;
                            }
                        }
                        if (ccMin!=-1) minima->push_back(ccMinPoint);
                    }
                    if (!delMax)
                    {
                        for (const Point& p : *maxima)
                        {
                            if (abs(p.y-ccMax)<5 && abs(p.x-ccMaxPoint.x)<10)
                            {
                                ccMax=-1;
                                break;
                            }
                        }
                        if (ccMax!=-1) maxima->push_back(ccMaxPoint);
                    }
                    
                }
                else
                {
                    scrubCC(ret,scanX,scanY);
                }
                prevCCSize=ccSize;
            }
            
            if (ret.at<unsigned char>(scanY,scanX)!=0 && ret.at<unsigned char>(scanY,scanX)!=JUNCTION_POINT)
            {
                int thisLabel=ret.at<unsigned char>(scanY,scanX);
//                if (thisLabel==UNMARKED_POINT || thisLabel==MARKED_POINT)
//                    thisLabel==-1;
                int otherLabel=-1;
                int relabelSingle=true;
                bool junctions[8]={false,false,false,false,false,false,false,false};
                for (int direction=0; direction<8; direction++)
                {
                    int x = scanX+xDelta(direction);
                    int y = scanY+yDelta(direction);
                    if (x<0 || y<0 || x>=ret.cols || y>=ret.rows)
                        continue;
                    
                    if (ret.at<unsigned char>(y,x)==JUNCTION_POINT)
                        junctions[direction]=true;
                }
                for (int direction=0; direction<8; direction++)
                {
                    int x = scanX+xDelta(direction);
                    int y = scanY+yDelta(direction);
                    if (x<0 || y<0 || x>=ret.cols || y>=ret.rows)
                        continue;
                    
                    if (ret.at<unsigned char>(y,x)==thisLabel)
                    {
                        relabelSingle=false;
                    }
                    else if (ret.at<unsigned char>(y,x)!=0 && 
                             !junctions[direction] && 
                             !junctions[mod(direction-1,8)] &&
                             !junctions[mod(direction+1,8)] &&
                             ret.at<unsigned char>(y,x)!=UNMARKED_POINT &&
                             ret.at<unsigned char>(y,x)!=MARKED_POINT)
                    {
//                        if(!(ret.at<unsigned char>(y,x)!=UNMARKED_POINT && ret.at<unsigned char>(y,x)!=MARKED_POINT))
//                        {
//                            writeGraphemes(ret);
//                            assert(ret.at<unsigned char>(y,x)!=UNMARKED_POINT && ret.at<unsigned char>(y,x)!=MARKED_POINT);
//                        }
                        otherLabel=ret.at<unsigned char>(y,x);
                    }
                }
                if (relabelSingle && otherLabel!=-1)
                    ret.at<unsigned char>(scanY,scanX)=otherLabel;
//                else if (thisLabel==-1)
//                {
//                    writeGraphemes(ret); assert(false && "unlabeled pixel had no assignment");
//                }
            }
            
            if (ret.at<unsigned char>(scanY,scanX)>0)
            {
                sumY+=scanY;
                countPixels++;
            }
    }
    
    assert(delMin || minima->size()>0);
    if (delMin) 
        delete minima;
    else
        prune(minima,true,ret);
    assert(delMax || maxima->size()>0);
    if (delMax) 
        delete maxima;
    else
        prune(maxima,false,ret);
    
    
    
    
    if (centerLine!=NULL)
        *centerLine=sumY/countPixels;
    return ret;
}

void Liang::prune(list<Point>* points, bool checkBelow, const Mat& skel)
{
    int avgY=0;
    for (Point p : *points)
        avgY+=p.y;
    avgY /= points->size();
    
    auto iter=points->begin();
    while(iter!=points->end())
    {
        bool erase=false;
        if (abs(iter->y-avgY)>10)
            for (int y=iter->y+(checkBelow?1:-1); (checkBelow&&y<skel.rows)||(!checkBelow&&y>=0); y+=(checkBelow?1:-1))
            {
                if (skel.at<unsigned char>(y,iter->x)>0)
                {
                    erase=true;
                    break;
                }
            }
        
        if (erase)
            iter = points->erase(iter);
        else 
            iter++;
    }
}

bool Liang::branchPoint(int cur_x, int cur_y, Mat& ret, list<Point>& startingPointQueue, int& curLabel)
{
    bool prevDirHad=ret.at<unsigned char>(cur_y+yDelta(7),cur_x+xDelta(7))>0;
	int count=0;
    for (int direction=0; direction<8; direction++) 
	{
        int x = cur_x+xDelta(direction);
        int y = cur_y+yDelta(direction);
        if (x<0 || y<0 || x>=ret.cols || y>=ret.rows)
        {
            prevDirHad=false;
            continue;
        }
        
        if (!prevDirHad && ret.at<unsigned char>(y,x)>0)
            count++;
        
        prevDirHad=ret.at<unsigned char>(y,x)>0;
	}
	
	if (count<3)
		return false;
	else
	{
		ret.at<unsigned char>(cur_y,cur_x)=JUNCTION_POINT;
//		prevDirHad=ret.at<unsigned char>(cur_y+yDelta(7),cur_x+xDelta(7))>0;
		
		
        bool filled[8]={false,false,false,false,false,false,false,false};
        for (int direction=0; direction<8; direction++)
        {
            int x = cur_x+xDelta(direction);
			int y = cur_y+yDelta(direction);
			if (x<0 || y<0 || x>=ret.cols || y>=ret.rows)
            {
				continue;
            }
            if (ret.at<unsigned char>(y,x)>0)
                filled[direction]=true;
        }
//        for (int direction=1; direction<8; direction=direction!=7?direction+2:0)
        for (int direction=0; direction<8; direction=direction!=6?direction+2:1)
		{
			int x = cur_x+xDelta(direction);
			int y = cur_y+yDelta(direction);
			if (x<0 || y<0 || x>=ret.cols || y>=ret.rows)
            {
//                prevDirHad=false;
				continue;
            }
            
            bool side1 = filled[mod(direction-1,8)];
            bool side2 = filled[mod(direction+1,8)];
            if (((!side1 && !side2) || direction%2==0) && ret.at<unsigned char>(y,x)==UNMARKED_POINT)
            {
                if (!branchPoint(x,y,ret,startingPointQueue,curLabel))
                {
                    startingPointQueue.push_back(Point(x,y));
                    ret.at<unsigned char>(y,x)=++curLabel;
                }
            }
            
//			if (!prevDirHad && ret.at<unsigned char>(y,x)==UNMARKED_POINT)
//			{
//			    //We'll check if the next is part of the same brachch
//			    Point next(cur_x+xDelta((direction+1)%8),cur_y+yDelta((direction+1)%8));
//				if (ret.at<unsigned char>(next)>0 &&
//				        !checkCorner(x,y,direction,ret))
//				{
//				    if (!branchPoint(next.x,next.y,ret,startingPointQueue,curLabel))
//				    {
//				    	startingPointQueue.push_back(next);
//				    	ret.at<unsigned char>(next)=++curLabel;
//				    }
//				}
//				else
//				{
//				    if (!branchPoint(x,y,ret,startingPointQueue,curLabel))
//				    {
//				    	startingPointQueue.push_back(Point(x,y));
//				    	ret.at<unsigned char>(y,x)=++curLabel;
//				    }
//				}
//			}

//			prevDirHad=ret.at<unsigned char>(y,x)>0;
		}
		return true;
	}
    assert(false);
}

bool Liang::relabelStrech(int label, Point from, Mat& ret, MinMaxTracker& tracker, MinMaxTracker& tracker2, MinMaxTracker& tracker3)
{
    int replace=-1;
    Point cur;
    for (int direction=1; direction<8; direction=direction!=7?direction+2:0)
    {
        int x = from.x+xDelta(direction);
        int y = from.y+yDelta(direction);
        if (x<0 || y<0 || x>=ret.cols || y>=ret.rows)
            continue;
        
        if (ret.at<unsigned char>(y,x)>0 && 
                ret.at<unsigned char>(y,x)!=label && 
                ret.at<unsigned char>(y,x)!=JUNCTION_POINT)
        {
            if (replace==-1)
            {
                replace=ret.at<unsigned char>(y,x);
                cur=Point(x,y);
                ret.at<unsigned char>(y,x)=label;
            }
            else
            {
                ret.at<unsigned char>(y,x)=label;
            }
        }
    }
    
    if(replace!=-1 && replace!=JUNCTION_POINT)
    {
        int count;
        while(1)
        {
            if (++count <tracker.neighborhoodSize()*2+1)
                tracker.track(cur,label);
            if (count <tracker2.neighborhoodSize()*2+1)
                tracker2.track(cur,label);
            if (count <tracker3.neighborhoodSize()*2+1)
                tracker3.track(cur,label);
            
            Point next(-1,-1);
            //            ret.at<unsigned char>(cur)=START_POINT;
            
            //for (int direction=0; direction<8; direction++)
            for (int direction=1; direction<8; direction=direction!=7?direction+2:0)
            {
                
                
                
                
                int x = cur.x+xDelta(direction);
                int y = cur.y+yDelta(direction);
                if (x<0 || y<0 || x>=ret.cols || y>=ret.rows)
                    continue;
                
                if (ret.at<unsigned char>(y,x)==replace)
                {
                    if (next.x==-1)
                    {
                        next.x=x;
                        next.y=y;
                        ret.at<unsigned char>(y,x)=label;
                    }
                    else
                    {
                        ret.at<unsigned char>(y,x)=label;
                    }
                }
            }
            
            cur=next;
            if (cur.x==-1) break;
        }
        
        return true;
    }
    else
        return false;
}

//void Liang::onLocals(Point& toAdd, list<Point>& after, list<Point>& before, Point& inQuestion, int neighborhoodSize, list<Point>& minima, list<Point>& maxima)
//{
//    if (after.size()==neighborhoodSize)
//    {
//        after.pop_front();
//    }
//    if (inQuestion.x!=-1)
//    {
//        after.push_back(inQuestion);
//    }
//    if (before.size()==neighborhoodSize)
//    {
//        inQuestion=before.front();
//        before.pop_front();
//    }
//    before.push_back(toAdd);
    
//    if (after.size()==neighborhoodSize)
//    {
//        bool candidateMax=true;
//        bool candidateMin=true;
        
        
        
//        auto prev = after.begin();
//        auto cur = after.begin(); cur++;
//        for (; cur!=after.end(); prev++,cur++)
//        {
//            if (prev->y > cur->y) candidateMin=false;
//            if (prev->y < cur->y) candidateMax=false;
//        }
//        if (after.back().y > inQuestion.y) candidateMin=false;
//        if (after.back().y < inQuestion.y) candidateMax=false;
        
//        prev = before.begin();
//        cur = before.begin(); cur++;
//        for (; cur!=before.end(); prev++,cur++)
//        {
//            if (prev->y < cur->y) candidateMin=false;
//            if (prev->y > cur->y) candidateMax=false;
//        }
//        if (inQuestion.y < before.front().y) candidateMin=false;
//        if (inQuestion.y > before.front().y) candidateMax=false;
        

//        auto check = after.rbegin();
//        while (candidateMin && (*check).y==inQuestion.y && check!=after.rend())
//        {
//            candidateMin = find(minima.rbegin(),minima.rend(),(*check)) == minima.rend();
//            check++;
//        }
//        check = after.rbegin();
//        while (candidateMax && (*check).y==inQuestion.y && check!=after.rend())
//        {
//            candidateMax = find(maxima.rbegin(),maxima.rend(),(*check)) == maxima.rend();
//            check++;
//        }
        
//        if (candidateMin && 
//                (std::max(inQuestion.y-after.front().y,inQuestion.y-before.back().y)>neighborhoodSize/4.0) && 
//                (std::min(inQuestion.y-after.front().y,inQuestion.y-before.back().y)>0))
//        {
//            minima.push_back(inQuestion);
//        }
        
//        if (candidateMax && 
//                (std::max(after.front().y-inQuestion.y,before.back().y-inQuestion.y)>neighborhoodSize/4.0) && 
//                (std::min(after.front().y-inQuestion.y,before.back().y-inQuestion.y)>0))
//        {
//            maxima.push_back(inQuestion);
//        }
//    }
//}

list<int> Liang::repairLoops(Mat& graphemes)
{
    Mat visited = graphemes.clone();
    map<int, int> mergeTable;
    map<Point, int,PointComp> juncMap;
    int juncLabel=JUNCTION_POINT;
    for (int scanX=0; scanX<graphemes.cols; scanX++)
        for (int scanY=0; scanY<graphemes.rows; scanY++)
        {
            if (visited.at<unsigned char>(scanY,scanX)>0)
            {
                Point cur(scanX,scanY);
                int curLabel=graphemes.at<unsigned char>(cur);
                list<int> chain;
//                chain.push_back(curLabel);
                exploreChain(cur,curLabel,chain, graphemes, visited, mergeTable,juncLabel,juncMap);
            }
        }
    
    
    list<int> finalLabels;
    for (int scanX=0; scanX<graphemes.cols; scanX++)
        for (int scanY=0; scanY<graphemes.rows; scanY++)
        {
            if (graphemes.at<unsigned char>(scanY,scanX)>0)
            {
                if (graphemes.at<unsigned char>(scanY,scanX)!=JUNCTION_POINT)
                {
                    int label = endMerge(graphemes.at<unsigned char>(scanY,scanX),mergeTable);
                    graphemes.at<unsigned char>(scanY,scanX)=label;
                    finalLabels.push_back(label);
                    finalLabels.sort();
                    finalLabels.unique();
                }
//                else if (juncMap.count(Point(scanX,scanY))>0)
//                {
//                    int label = endMerge(juncMap.at(Point(scanX,scanY)),mergeTable);
//                    graphemes.at<unsigned char>(scanY,scanX)=label;
//                    finalLabels.push_back(label);
//                    finalLabels.sort();
//                    finalLabels.unique();
//                }
                else
                {
                    map<int, int> counts;
                    int totalCounts=0;
                    int firstLabel=-1;
                    int prevLabel=-1;
                    int label = endMerge(juncMap.at(Point(scanX,scanY)),mergeTable);
                    bool done=false;
                    Mat mark = graphemes.clone();
                    for (int direction=0; direction<8; direction++)
                    {
                        int x = scanX+xDelta(direction);
                        int y = scanY+yDelta(direction);
                        if (x<0 || y<0 || x>=graphemes.cols || y>=graphemes.rows || mark.at<unsigned char>(y,x)==0)
                            continue;
                        int thisLabel=endMerge(graphemes.at<unsigned char>(y,x),mergeTable);
                        
                        if (thisLabel == label)
                        {
                            done=true;
                            break;
                        }
                        else if (thisLabel!=JUNCTION_POINT)
                        {
                            if (direction==0) firstLabel=thisLabel;
                            
                            if ((direction==0 || (direction==7&&firstLabel!=thisLabel) || (direction!=7&&prevLabel!=thisLabel)) &&
                                    thisLabel!=0)
                                counts[thisLabel]++;
                            
                            prevLabel=thisLabel;
                            mark.at<unsigned char>(y,x)=0;
                        }
                        else if (label == endMerge(juncMap.at(Point(x,y)),mergeTable))
                        {
                            done=true;
                            break;
                        }
                        else
                        {
                            
                            countJunction(x,y,direction,direction==0,direction==7,firstLabel,graphemes,mergeTable, counts, prevLabel, mark);
                            mark.at<unsigned char>(y,x)=0;
                        }
                    }
                    mark.release();
                    
                    if (done)
                    {
                        graphemes.at<unsigned char>(scanY,scanX)=label;
                        continue;
                    }
                    
                    list<int> passThrough;
                    for (auto cp : counts)
                    {
                        if (cp.second > 1)
                            passThrough.push_back(cp.first);
                    }
                    
                    if (passThrough.size()==1)
                    {
                        graphemes.at<unsigned char>(scanY,scanX)=passThrough.front();
                    }
                    else if(passThrough.size()==0)
                    {
                        if (counts.size()>0)
                        {
                            for (auto p : counts)
                            {
                                if (p.first!=JUNCTION_POINT)
                                {
                                    graphemes.at<unsigned char>(scanY,scanX)=p.first;
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        assert(false && "undefined intersection");
                    }
//                    if (counts==3)
//                    {
                        
//                    }
//                    else
//                    {
//                        assert(false && "four intersection not handled");
//                    }
                }
            }
        }
    
    return finalLabels;
}

void Liang::countJunction(int fromX, int fromY, int prevDir, bool first, bool last, int& firstLabel, const Mat& graphemes, const map<int, int>& mergeTable, map<int,int>& counts, int& prevLabel, Mat& mark)
{
    int dirStart, dirEnd;
    switch(prevDir)
    {
    case 0:
        dirStart=7;
        dirEnd=1;
        break;
    case 1:
        dirStart=7;
        dirEnd=3;
        break;
    case 2:
        dirStart=1;
        dirEnd=3;
        break;
    case 3:
        dirStart=1;
        dirEnd=5;
        break;
    case 4:
        dirStart=3;
        dirEnd=5;
        break;
    case 5:
        dirStart=3;
        dirEnd=7;
        break;
    case 6:
        dirStart=5;
        dirEnd=7;
        break;
    case 7:
        dirStart=5;
        dirEnd=1;
        break;
    }
    
    for (int direction=dirStart; direction!=(dirEnd+1)%8; direction=(direction+1)%8)
    {
        int x = fromX+xDelta(direction);
        int y = fromY+yDelta(direction);
        if (x<0 || y<0 || x>=graphemes.cols || y>=graphemes.rows)
            continue;
        int thisLabel=endMerge(graphemes.at<unsigned char>(y,x),mergeTable);
        
        if (thisLabel!=JUNCTION_POINT && mark.at<unsigned char>(y,x)!=0)
        {
            if (first&&direction==dirStart) firstLabel=thisLabel;
            
            if (thisLabel!=0 && 
			    ( (first&&direction==dirStart) || 
			      (last&&direction==dirEnd&&firstLabel!=thisLabel) || 
			      ((!last||direction!=dirEnd)&&(prevLabel!=thisLabel)) ))
                counts[thisLabel]++;
            
            prevLabel=thisLabel;
            
        }
        else if (mark.at<unsigned char>(y,x)!=0)
        {
            countJunction(x,y,direction,first&&direction==dirStart,last&&direction==dirEnd,firstLabel,graphemes,mergeTable, counts, prevLabel, mark);
        }
        mark.at<unsigned char>(y,x)=0;
    }
}

bool Liang::checkCorner(int fromX, int fromY, int prevDir, const Mat& skel)
{
    if (skel.at<unsigned char>(fromY,fromX)==0) return false;
    
    int dirStart, dirEnd;
    switch(prevDir)
    {
    case 0:
        dirStart=7;
        dirEnd=1;
        break;
    case 1:
        dirStart=7;
        dirEnd=3;
        break;
    case 2:
        dirStart=1;
        dirEnd=3;
        break;
    case 3:
        dirStart=1;
        dirEnd=5;
        break;
    case 4:
        dirStart=3;
        dirEnd=5;
        break;
    case 5:
        dirStart=3;
        dirEnd=7;
        break;
    case 6:
        dirStart=5;
        dirEnd=7;
        break;
    case 7:
        dirStart=5;
        dirEnd=1;
        break;
    }
    
    for (int direction=dirStart; direction!=(dirEnd+1)%8; direction=(direction+1)%8)
    {
        int x = fromX+xDelta(direction);
        int y = fromY+yDelta(direction);
        if (x<0 || y<0 || x>=skel.cols || y>=skel.rows)
            continue;
        
        if(skel.at<unsigned char>(y,x)>0)
            return true;
    }
    return false;
}

//This creates the mergeTable by relabeling
void Liang::  exploreChain(Point cur, int curLabel, list<int> chain, Mat& graphemes, Mat& visited, map<int, int>& mergeTable, int& juncLabel, map<Point, int,PointComp> &juncMap, int prevDir)
{
//    cout << "  0( [" << cur.x<<","<<cur.y<< "], " << curLabel << ", ...)"<<endl;
    assert(curLabel!=0);
    assert(curLabel==JUNCTION_POINT || juncLabel>curLabel);
    
    if (curLabel != JUNCTION_POINT)
        chain.push_back(curLabel);
    else
    {
        if (juncMap.count(cur)==0)
        {
            juncMap[cur] = --juncLabel;
        }
        
        chain.push_back(juncMap.at(cur));
    }
    
    
    while(cur.x!=-1)
    {
        visited.at<unsigned char>(cur)=0;
        Point next(-1,-1);
        
        bool juncFound=false;

        
        bool junctions[8] = {false,false,false,false,false,false,false,false};
        bool onEven=false;
        bool onOdd=false;
        for (int direction=0; direction<8; direction++)
        {
            int x = cur.x+xDelta(direction);
            int y = cur.y+yDelta(direction);
            if (x<0 || y<0 || x>=graphemes.cols || y>=graphemes.rows)
                continue;
            
            if (graphemes.at<unsigned char>(y,x)==JUNCTION_POINT)
            {
                junctions[direction]=true;
                if (direction%2==0)
                    onEven=true;
                else 
                    onOdd=true;
            }
        }
        int nextPrevDir=-1;
        for (int direction=0; direction<8; direction=direction!=6?direction+2:1)
        {
            if ((onEven && direction==0) || (direction==1 && onOdd))
            {
                for (int direction2=direction; direction2<8; direction2+=2)
                {
                    if (junctions[direction2] && !directionForbidden(direction2,prevDir))
                    {
                        int x = cur.x+xDelta(direction2);
                        int y = cur.y+yDelta(direction2);
                        if (visited.at<unsigned char>(y,x)>0)
                        {
                              exploreChain(Point(x,y),JUNCTION_POINT,chain,graphemes,visited,mergeTable,juncLabel,juncMap,direction2);
                            juncFound=true;
                        }
                        else if (juncMap.count(Point(x,y))>0 && find(chain.begin(),chain.end(),juncMap.at(Point(x,y))) != chain.end())
                        {
                            relabel(chain.back(),juncMap.at(Point(x,y)),chain,mergeTable);
                            juncFound=true;
                        }
//                        else
//                        {//if we've passed by, we still can finish a loop, so look ahead
//                            //I think all branches have already been visited in this scenario
//                            int doRelabel=-1;
//                            int bestLoc=-1;
//                            lookAheadJunctions(x,y,direction2,cur,chain,graphemes,doRelabel,bestLoc);
                            
//                            if (doRelabel!=-1)
//                            {
//                                relabel(chain.back(),doRelabel,chain,mergeTable);
//                                juncFound=true;
//                            }
//                        }
                    }
                }
                
                if (juncFound)
                {
                    direction= direction==0?6:8;
                    continue;
                }
            }
            else if (juncFound)
            {
                break;
            }
            int x = cur.x+xDelta(direction);
            int y = cur.y+yDelta(direction);
            if (x<0 || y<0 || x>=graphemes.cols || y>=graphemes.rows  || directionForbidden(direction,prevDir))
                continue;
            
            
            int newLabel=graphemes.at<unsigned char>(y,x);
            
            //                if (    !junctions[direction] &&
            //                        !junctions[mod(direction-1,8)] &&
            //                        !junctions[mod(direction+1,8)])
            {
                if (visited.at<unsigned char>(y,x)>0)
                {
                    if (newLabel!=curLabel)
                    {
                        if (find(chain.begin(),chain.end(),newLabel) != chain.end())
                        {
                            relabel(chain.back(),newLabel,chain,mergeTable);
                        }
                          exploreChain(Point(x,y),newLabel,chain,graphemes,visited,mergeTable,juncLabel,juncMap,direction);
                        
                        if (curLabel != JUNCTION_POINT)
                            break;
                    }
                    else
                    {
                        next=Point(x,y);
                        nextPrevDir=direction;
                        break;
                    }
                }
                else if (newLabel>0 && newLabel!=curLabel && find(chain.begin(),chain.end(),newLabel) != chain.end())
                {
                    relabel(chain.back(),newLabel,chain,mergeTable);
                    break;
                }
            }
        }
        prevDir=nextPrevDir;
        
        cur=next;
        
    }
}

/*void Liang::  0(Point cur, int curLabel,list<int> chain, Mat& graphemes, Mat& visited, map<int, int>& mergeTable, int prevDir)
{
//    cout << "  0( [" << cur.x<<","<<cur.y<< "], " << curLabel << ", ...)"<<endl;
    
    if (curLabel != JUNCTION_POINT)
        chain.push_back(curLabel);
    
    
    
//    int runLen=0;
    while(cur.x!=-1)
    {
        visited.at<unsigned char>(cur)=0;
        Point next(-1,-1);
//        runLen++;
        
        //For odd cases, we scan first for a junction_point neighbor
        bool juncFound=false;
        for (int direction=0; direction<8; direction++)
        {
            int x = cur.x+xDelta(direction);
            int y = cur.y+yDelta(direction);
            if (x<0 || y<0 || x>=graphemes.cols || y>=graphemes.rows || directionForbidden(direction,prevDir))
                continue;
            
            
            
            if (graphemes.at<unsigned char>(y,x)==JUNCTION_POINT)
            {
                
                if (visited.at<unsigned char>(y,x)>0)
                {
                      0(Point(x,y),JUNCTION_POINT,chain,graphemes,visited,mergeTable,direction);
                    juncFound=true;
                }
                else
                {//if we've passed by, we still can finish a loop, so look ahead
                 //I think all branches have already been visited in this scenario
                    int doRelabel=-1;
                    int bestLoc=-1;
                    lookAheadJunctions(x,y,direction,cur,chain,graphemes,doRelabel,bestLoc);
                    
                    if (doRelabel!=-1)
                        relabel(chain.back(),doRelabel,chain,mergeTable);
                }
            }
        }
        
        if (!juncFound)
        {
            bool junctions[8] = {false,false,false,false,false,false,false,false};
            for (int direction=0; direction<8; direction++)
            {
                int x = cur.x+xDelta(direction);
                int y = cur.y+yDelta(direction);
                if (x<0 || y<0 || x>=graphemes.cols || y>=graphemes.rows)
                    continue;
                
                if (graphemes.at<unsigned char>(y,x)==JUNCTION_POINT)
                    junctions[direction]=true;
            }
            int nextPrevDir=-1;
            for (int direction=0; direction<8; direction=direction!=6?direction+2:1)
            {
                int x = cur.x+xDelta(direction);
                int y = cur.y+yDelta(direction);
                if (x<0 || y<0 || x>=graphemes.cols || y>=graphemes.rows  || directionForbidden(direction,prevDir))
                    continue;
                
                
                int newLabel=graphemes.at<unsigned char>(y,x);
                
                if (    !junctions[direction] &&
                        !junctions[mod(direction-1,8)] &&
                        !junctions[mod(direction+1,8)])
                {
                    if (visited.at<unsigned char>(y,x)>0)
                    {
                        if (newLabel!=curLabel)
                        {
                            if (find(chain.begin(),chain.end(),newLabel) != chain.end())
                            {
                                relabel(chain.back(),newLabel,chain,mergeTable);
                            }
                              0(Point(x,y),newLabel,chain,graphemes,visited,mergeTable,direction);
                            
                            if (curLabel != JUNCTION_POINT)
                                break;
                        }
                        else
                        {
                            next=Point(x,y);
                            nextPrevDir=direction;
                            break;
                        }
                    }
                    else if (newLabel>0 && newLabel!=curLabel && find(chain.begin(),chain.end(),newLabel) != chain.end())
                    {
                        relabel(chain.back(),newLabel,chain,mergeTable);
                        break;
                    }
                }
            }
            prevDir=nextPrevDir;
        }
        cur=next;
        
    }
}*/



void Liang::lookAheadJunctions(int x, int y, int prevDirection, Point cur, const list<int>& chain, const Mat& graphemes, int &doRelabel, int &bestLoc)
{
    for (int direction2=0; direction2<8; direction2++)
    {
        
        int x2 = x+xDelta(direction2);
        int y2 = y+yDelta(direction2);
        if (x<0 || y<0 || x>=graphemes.cols || y>=graphemes.rows || (x2==cur.x && y2==cur.y) || directionForbidden(direction2,prevDirection))
            continue;
        int lookahead = graphemes.at<unsigned char>(y2,x2);
        if (lookahead==JUNCTION_POINT)
        {
            lookAheadJunctions(x2,y2,direction2, Point(x,y), chain,graphemes,doRelabel,bestLoc);
        }
        else
        {
            int loc=0;
            for (auto iter =chain.begin(); iter != chain.end(); iter++, loc++)
            {
                if (*iter == lookahead && loc>bestLoc)
                {
                    doRelabel=lookahead;
                    bestLoc=loc;
                    break;
                }
            }
        }
    }
}

bool Liang::directionForbidden(int direction, int prevDir)
{   
    return (prevDir!=-1) && min(mod(mod(prevDir-4,8)-direction,8),mod(direction-mod(prevDir-4,8),8))<2;
}

void Liang::relabel(int from,int to,list<int>& chain,map<int, int>& mergeTable)
{
    
    int newLabel = endMerge(to,mergeTable);
    //assert(newLabel!=0);
    auto iter=chain.rbegin();
    while (*iter!=from && iter!=chain.rend()) iter++;
    for (; iter!=chain.rend() && *iter!=to; iter++)
    {
        if (endMerge(*iter,mergeTable) != newLabel)
            mergeTable[endMerge(*iter,mergeTable)]=newLabel;
    }
}

int Liang::endMerge(int start, const map<int, int>& mergeTable)
{
    if (mergeTable.find(start) != mergeTable.end())
        return endMerge(mergeTable.at(start),mergeTable);
    else 
        return start;
}


void Liang::findBaselines(bool hasAscender, bool hasDescender, int* upperBaseline, int* lowerBaseline, const list<Point>& localMins, const list<Point>& localMaxs, int centerLine, const Mat& img)
{
    //list<Point> localMins, localMaxs;
    
    //findAllMaxMin(skel,localMins, localMaxs);
    int threshold;
    vector<int> counts;
    if (hasAscender||hasDescender)
    {
        //Use Otsu threshold to find where to start baselines.
        //Ignores all lines with zero pixels as those aren't text
        //code copied from http://www.dandiggins.co.uk/arlib-9.html
        counts.resize(img.rows);
        vector<double> hist(img.cols);
        int rows=0;
        for (int y=0; y<img.rows; y++)
        {
            int count=0;
            for (int x=0; x<img.cols; x++)
                if (img.at<unsigned char>(y,x)>0)
                    count++;
            hist[count]++;
            counts[y]=count;
            if (count>0)
                rows++;
        }
        double mean=0;
        
        for (int i=1; i<img.cols; i++)
        {
            
            hist[i]/=rows;
            mean += i*hist[i];
        }
        
        double w=0;
        double u=0;
        double work3=0;
        for (int i=1; i<img.cols-1; i++) {
            w+=hist[i];
            u+=(i*hist[i]);
            double work1 = (mean * w - u);
            double work2 = (work1 * work1) / ( w * (1.0f-w) );
            if (work2>work3) work3=work2;
        }
        threshold = (int)sqrt(work3);
    }
    
    
    *upperBaseline = 9999;
    if (hasAscender)
    {
        int candidate1=-1;
        if (counts[centerLine]>=threshold)
        {
            for (int y=centerLine-1; y>=0; y--)
            {
                if (counts[y]<threshold)
                {
                    candidate1=y;
                    break;
                }
            }
        }
        else
        {
            for (int y=0; y<img.rows; y++)
            {
                if (counts[y]>=threshold)
                {
                    candidate1=y-1;
                    break;
                }
            }
        }
        
        int candidate2=9999;
        int lowMaxima=0;
        int highMaxima=9999;
        list<Point> newMaxs;
        for (Point p : localMaxs)
        {
            if (p.y <= centerLine+15)
            {
                newMaxs.push_back(p);
                if (p.y > lowMaxima) lowMaxima=p.y;
                if (p.y < highMaxima) highMaxima=p.y;
            }
        }
        
        for (Point p : newMaxs)
        {
            if (p.y-highMaxima >= lowMaxima-p.y)
            {
                if (p.y < candidate2) candidate2 = p.y;
            }
        }
        assert(candidate2!=9999);
        
        if (candidate1!=-1)
            *upperBaseline = (candidate1+candidate2)/2;
        else
            *upperBaseline = candidate2;
        assert(*upperBaseline>=0);
    }
    else
    {
        double averageMaxima=0;
        for (Point p : localMaxs)
        {
            
            averageMaxima += p.y;
        }
        averageMaxima/=localMaxs.size();
        
        double dev=0;
        for (Point p : localMaxs)
        {
            dev+=(p.y-averageMaxima)*(p.y-averageMaxima);
        }
        dev=sqrt(dev/localMaxs.size());
        
        double dev2=0;
        int count=0;
        for (Point p : localMaxs)
        {
            if (abs(p.y-averageMaxima)<5 || abs(p.y-averageMaxima)<=dev)
            {
                dev2+=(p.y-averageMaxima)*(p.y-averageMaxima);
                count++;
            }
        }//not real std dev as using old avg
        dev2=sqrt(dev2/count);
        
        *upperBaseline = averageMaxima - 2*ceil(dev2);
    }
    
    *lowerBaseline = 0;
    if (hasDescender)
    {
        int candidate1=-1;
        if (counts[centerLine]>threshold)
        {
            for (int y=centerLine+1; y<img.rows; y++)
            {
                if (counts[y]<threshold)
                {
                    candidate1=y;
                    break;
                }
            }
        }
        else
        {
            for (int y=img.rows-1; y>=0; y--)
            {
                if (counts[y]>=threshold)
                {
                    candidate1=y+1;
                    break;
                }
            }
        }
        
        int candidate2=-1;
        int lowMinima=0;
        int highMinima=9999;
        list<Point> newMins;
        for (Point p : localMins)
        {
            if (p.y >= centerLine-15)
            {
                newMins.push_back(p);
                if (p.y > lowMinima) lowMinima=p.y;
                if (p.y < highMinima) highMinima=p.y;
            }
        }
        
        for (Point p : newMins)
        {
            if (p.y-highMinima <= lowMinima-p.y)
            {
                if (p.y > candidate2) candidate2 = p.y;
            }
        }
        assert(candidate2!=-1);
        if (candidate1!=-1)
            *lowerBaseline = (candidate1+candidate2)/2;
        else
            *lowerBaseline = candidate2;
        assert(*lowerBaseline < img.rows);
    }
    else
    {
        double averageMinima=0;
        for (Point p : localMins)
        {
            
            averageMinima+=p.y;
        }
        averageMinima/=localMins.size();
        
        double dev=0;
        
        for (Point p : localMins)
        {
            dev+=(p.y-averageMinima)*(p.y-averageMinima);
        }
        dev=sqrt(dev/localMins.size());
        
        double dev2=0;
        int count=0;
        for (Point p : localMins)
        {
            if (abs(p.y-averageMinima)<5 || abs(p.y-averageMinima)<=dev)
            {
                dev2+=(p.y-averageMinima)*(p.y-averageMinima);
                count++;
            }
        }
        dev2=sqrt(dev2/count);
        
        *lowerBaseline = averageMinima + 2*ceil(dev2);
    }
    
    if(*upperBaseline >= *lowerBaseline)//something strange is going on
    {
        double averageMaxima=0;
        for (Point p : localMaxs)
        {
            
            averageMaxima += p.y;
        }
        averageMaxima/=localMaxs.size();
        
        double dev=0;
        for (Point p : localMaxs)
        {
            dev+=(p.y-averageMaxima)*(p.y-averageMaxima);
        }
        dev=sqrt(dev/localMaxs.size());
        
        double dev2=0;
        int count=0;
        for (Point p : localMaxs)
        {
            if (abs(p.y-averageMaxima)<5 || abs(p.y-averageMaxima)<=dev)
            {
                dev2+=(p.y-averageMaxima)*(p.y-averageMaxima);
                count++;
            }
        }//not real std dev as using old avg
        dev2=sqrt(dev2/count);
        
        *upperBaseline = averageMaxima - 2*ceil(dev2);
        
        double averageMinima=0;
        for (Point p : localMins)
        {
            
            averageMinima+=p.y;
        }
        averageMinima/=localMins.size();
        
        dev=0;
        for (Point p : localMins)
        {
            dev+=(p.y-averageMinima)*(p.y-averageMinima);
        }
        dev=sqrt(dev/localMins.size());
        
        dev2=0;
        count=0;
        for (Point p : localMins)
        {
            if (abs(p.y-averageMinima)<5 || abs(p.y-averageMinima)<=dev)
            {
                dev2+=(p.y-averageMinima)*(p.y-averageMinima);
                count++;
            }
        }
        dev2=sqrt(dev2/count);
        
        *lowerBaseline = averageMinima + 2*ceil(dev2);
    }
}

//void Liang::findBaselines(bool hasAscender, bool hasDescender, int* upperBaseline, int* lowerBaseline, const list<Point>& localMins, const list<Point>& localMaxs, int centerLine)
//{
//    //list<Point> localMins, localMaxs;
    
//    //findAllMaxMin(skel,localMins, localMaxs);
    
    
    
//    *upperBaseline = 9999;
//    if (hasAscender)
//    {
        
//        //We will filter out outlier maxs and mins as our baseline finding method is sensitive to them
        
        
//        int lowMaxima=0;
//        int highMaxima=9999;
//        list<Point> newMaxs;
//        for (Point p : localMaxs)
//        {
//            if (p.y <= centerLine+15)
//            {
//                newMaxs.push_back(p);
//                if (p.y > lowMaxima) lowMaxima=p.y;
//                if (p.y < highMaxima) highMaxima=p.y;
//            }
//        }
        
//        for (Point p : newMaxs)
//        {
//            if (p.y-highMaxima >= lowMaxima-p.y)
//            {
//                if (p.y < *upperBaseline) *upperBaseline = p.y;
//            }
//        }
//        assert(*upperBaseline != 9999);
//    }
//    else
//    {
//        double averageMaxima=0;
//        for (Point p : localMaxs)
//        {
            
//            averageMaxima += p.y;
//        }
//        averageMaxima/=localMaxs.size();
        
//        double dev=0;
//        for (Point p : localMaxs)
//        {
//            dev+=(p.y-averageMaxima)*(p.y-averageMaxima);
//        }
//        dev=sqrt(dev/localMaxs.size());
        
//        double dev2=0;
//        int count=0;
//        for (Point p : localMaxs)
//        {
//            if (abs(p.y-averageMaxima)<5 || abs(p.y-averageMaxima)<=dev)
//            {
//                dev2+=(p.y-averageMaxima)*(p.y-averageMaxima);
//                count++;
//            }
//        }//not real std dev as using old avg
//        dev2=sqrt(dev2/count);
        
//        *upperBaseline = averageMaxima - 2*ceil(dev2);
//    }
    
//    *lowerBaseline = 0;
//    if (hasDescender)
//    {
//        //We will filter out outlier maxs and mins as our baseline finding method is sensitive to them
//        int lowMinima=0;
//        int highMinima=9999;
//        list<Point> newMins;
//        for (Point p : localMins)
//        {
//            if (p.y >= centerLine-15)
//            {
//                newMins.push_back(p);
//                if (p.y > lowMinima) lowMinima=p.y;
//                if (p.y < highMinima) highMinima=p.y;
//            }
//        }
        
//        for (Point p : newMins)
//        {
//            if (p.y-highMinima <= lowMinima-p.y)
//            {
//                if (p.y > *lowerBaseline) *lowerBaseline = p.y;
//            }
//        }
//        assert(*lowerBaseline != 9999);
//    }
//    else
//    {
//        double averageMinima=0;
//        for (Point p : localMins)
//        {
            
//            averageMinima+=p.y;
//        }
//        averageMinima/=localMins.size();
        
//        double dev=0;
        
//        for (Point p : localMins)
//        {
//            dev+=(p.y-averageMinima)*(p.y-averageMinima);
//        }
//        dev=sqrt(dev/localMins.size());
        
//        double dev2=0;
//        int count=0;
//        for (Point p : localMins)
//        {
//            if (abs(p.y-averageMinima)<5 || abs(p.y-averageMinima)<=dev)
//            {
//                dev2+=(p.y-averageMinima)*(p.y-averageMinima);
//                count++;
//            }
//        }
//        dev2=sqrt(dev2/count);
        
//        *lowerBaseline = averageMinima + 2*ceil(dev2);
//    }
    
//    if(*upperBaseline >= *lowerBaseline)//something strange is going on
//    {
//        double averageMaxima=0;
//        for (Point p : localMaxs)
//        {
            
//            averageMaxima += p.y;
//        }
//        averageMaxima/=localMaxs.size();
        
//        double dev=0;
//        for (Point p : localMaxs)
//        {
//            dev+=(p.y-averageMaxima)*(p.y-averageMaxima);
//        }
//        dev=sqrt(dev/localMaxs.size());
        
//        double dev2=0;
//        int count=0;
//        for (Point p : localMaxs)
//        {
//            if (abs(p.y-averageMaxima)<5 || abs(p.y-averageMaxima)<=dev)
//            {
//                dev2+=(p.y-averageMaxima)*(p.y-averageMaxima);
//                count++;
//            }
//        }//not real std dev as using old avg
//        dev2=sqrt(dev2/count);
        
//        *upperBaseline = averageMaxima - 2*ceil(dev2);
        
//        double averageMinima=0;
//        for (Point p : localMins)
//        {
            
//            averageMinima+=p.y;
//        }
//        averageMinima/=localMins.size();
        
//        dev=0;
//        for (Point p : localMins)
//        {
//            dev+=(p.y-averageMinima)*(p.y-averageMinima);
//        }
//        dev=sqrt(dev/localMins.size());
        
//        dev2=0;
//        count=0;
//        for (Point p : localMins)
//        {
//            if (abs(p.y-averageMinima)<5 || abs(p.y-averageMinima)<=dev)
//            {
//                dev2+=(p.y-averageMinima)*(p.y-averageMinima);
//                count++;
//            }
//        }
//        dev2=sqrt(dev2/count);
        
//        *lowerBaseline = averageMinima + 2*ceil(dev2);
//    }
//}

//void Liang::findAllMaxMin(const Mat& skel, list<Point>& localMins, list<Point>& localMaxs)
//{
//    Mat visited = skel.clone();
    
    
//    list<Point> after, before;
//    Point inQuestion;
    
    
//    list<Point> startPoints;
    
//    for (int scanX=0; scanX<skel.cols; scanX++)
//        for (int scanY=0; scanY<skel.rows; scanY++)
//        {
//            if (visited.at<unsigned char>(scanY,scanX)==UNMARKED_POINT)
//            {
//                startPoints.push_back(Point(scanX,scanY));
//                visited.at<unsigned char>(scanY,scanX)=MARKED_POINT;
            
//                while(!startPoints.empty())
//                {
//                    Point cur = startPoints.front();
//                    startPoints.pop_front();
                    
//                    while (cur.x!=-1)
//                    {
//                        Point next(-1,-1);
//                        for (int direction=0; direction<8; direction++)
//                        {
//                            int x = cur.x+xDelta(direction);
//                            int y = cur.y+yDelta(direction);
//                            if (x<0 || y<0 || x>=skel.cols || y>=skel.rows)
//                                continue;
                            
//                            if (visited.at<unsigned char>(y,x)==UNMARKED_POINT)
//                            {
                                
//                                if (next.x==-1)
//                                {
//                                    next.x=x;
//                                    next.y=y;
//                                }
//                                else
//                                {
//                                    startPoints.push_back(Point(x,y));
//                                    visited.at<unsigned char>(y,x)=MARKED_POINT;
//                                }
//                            }
//                        }
                        
//                        onLocals(cur,after,before,inQuestion,BASELINE_NEIGHBORHOOD_SIZE,localMins,localMaxs);
                        
//                        cur=next;
//                        visited.at<unsigned char>(cur)=MARKED_POINT;
//                    }
                    
                    
                    
//                }
//            }
//        }
//}

bool Liang::unittest()
{
    assert(xDelta(0)==1);
    assert(xDelta(1)==1);
    assert(xDelta(2)==0);
    assert(xDelta(3)==-1);
    assert(xDelta(4)==-1);
    assert(xDelta(5)==-1);
    assert(xDelta(6)==0);
    assert(xDelta(7)==1);
    assert(yDelta(0)==0);
    assert(yDelta(1)==-1);
    assert(yDelta(2)==-1);
    assert(yDelta(3)==-1);
    assert(yDelta(4)==0);
    assert(yDelta(5)==1);
    assert(yDelta(6)==1);
    assert(yDelta(7)==1);
    
    map<int, int> mergeTable;
    assert(endMerge(10,mergeTable)==10);
    mergeTable[10]=1;
    assert(endMerge(2,mergeTable)==2);
    assert(endMerge(10,mergeTable)==1);
    mergeTable[2]=10;
    assert(endMerge(2,mergeTable)==1);
    
    map<int, int> mergeTable2;
    list<int> chain1 = {10,1};
    relabel(1,10,chain1,mergeTable2);
    assert(endMerge(10,mergeTable2)  ==  endMerge(1,mergeTable2));
    list<int> chain2 = {10,1,2,3,4,5,6,7};
    relabel(6,3,chain2,mergeTable2);
    assert(endMerge(10,mergeTable2)  ==  endMerge(1,mergeTable2));
    assert(endMerge(3,mergeTable2)  ==  endMerge(4,mergeTable2));
    assert(endMerge(3,mergeTable2)  ==  endMerge(5,mergeTable2));
    assert(endMerge(3,mergeTable2)  ==  endMerge(6,mergeTable2));
    assert(endMerge(2,mergeTable2)  ==  2);
    assert(endMerge(7,mergeTable2)  ==  7);
    
    cout << "relabel tests passed" << endl;
    
    Mat graphemes1 = (Mat_<unsigned char>(4,4) <<  0,  1,  1, 0,
                                                   2,  0,  0, 1,
                                                   2,  0,  0, 1,
                                                   0,  2,  2, 0);
    Mat visited1 = graphemes1.clone();
    list<int> chain3;
    map<int, int> mergeTable3;
    map<Point, int,PointComp> juncMap;
    int juncLabel=JUNCTION_POINT;
    exploreChain(Point(1,0), 1,chain3, graphemes1, visited1, mergeTable3,juncLabel,juncMap);
    assert(endMerge(1,mergeTable3)  ==  endMerge(2,mergeTable3));
    
    list<int> labels1 = repairLoops(graphemes1);
    assert(graphemes1.at<unsigned char>(0,1)==labels1.front());
    assert(graphemes1.at<unsigned char>(0,2)==labels1.front());
    assert(graphemes1.at<unsigned char>(1,0)==labels1.front());
    assert(graphemes1.at<unsigned char>(2,0)==labels1.front());
    assert(graphemes1.at<unsigned char>(1,3)==labels1.front());
    assert(graphemes1.at<unsigned char>(2,3)==labels1.front());
    assert(graphemes1.at<unsigned char>(3,1)==labels1.front());
    assert(graphemes1.at<unsigned char>(3,2)==labels1.front());
    
    Mat graphemes2 = (Mat_<unsigned char>(4,4) <<  0,  1,  1, 0,
                                                   4,  0,  0, 3,
                                                   4,  0,  0, 3,
                                                   0,  2,  2, 0);
    Mat visited2 = graphemes2.clone();
    list<int> chain4;
    map<int, int> mergeTable4;
    juncMap.clear();
    juncLabel=JUNCTION_POINT;
    exploreChain(Point(1,0), 1,chain4, graphemes2, visited2, mergeTable4,juncLabel,juncMap);
    assert(endMerge(1,mergeTable4)  ==  endMerge(2,mergeTable4));
    assert(endMerge(1,mergeTable4)  ==  endMerge(3,mergeTable4));
    assert(endMerge(1,mergeTable4)  ==  endMerge(4,mergeTable4));
    
    list<int> labels2 = repairLoops(graphemes2);
    assert(graphemes2.at<unsigned char>(0,1)==labels2.front());
    assert(graphemes2.at<unsigned char>(0,2)==labels2.front());
    assert(graphemes2.at<unsigned char>(1,0)==labels2.front());
    assert(graphemes2.at<unsigned char>(2,0)==labels2.front());
    assert(graphemes2.at<unsigned char>(1,3)==labels2.front());
    assert(graphemes2.at<unsigned char>(2,3)==labels2.front());
    assert(graphemes2.at<unsigned char>(3,1)==labels2.front());
    assert(graphemes2.at<unsigned char>(3,2)==labels2.front());
    
    Mat graphemes3 = (Mat_<unsigned char>(4,4) <<  0,  1,  1, 1,
                                                   0,  1,  0, 0,
                                                   0,  2,  0, 2,
                                                   0,  2,  2, 0);
    Mat visited3 = graphemes3.clone();
    list<int> chain5;
    map<int, int> mergeTable5;
    juncMap.clear();
    juncLabel=JUNCTION_POINT;
    exploreChain(Point(1,0), 1,chain5, graphemes3, visited3, mergeTable5,juncLabel,juncMap);
    assert(endMerge(1,mergeTable5)  !=  endMerge(2,mergeTable5));
    
    list<int> labels3 = repairLoops(graphemes3);
    assert(labels3.size()==2);
    assert(graphemes3.at<unsigned char>(0,2)==graphemes3.at<unsigned char>(0,3));
    assert(graphemes3.at<unsigned char>(1,1)!=graphemes3.at<unsigned char>(2,1));
    
    int j=JUNCTION_POINT;
    Mat graphemes4 = (Mat_<unsigned char>(6,6) <<  0,  1,  1, 1, 5, 0,
                                                   0,  1,  0, 0, 0, 5,
                                                   3,  j,  0, 2, 0, 5,
                                                   3,  0,  2, 0, j, 0,
                                                   3,  0,  0, 0, 4, 0,
                                                   0,  0,  0, 0, 4, 0);
    Mat visited4 = graphemes4.clone();
    list<int> chain6;
    map<int, int> mergeTable6;
    juncMap.clear();
    juncLabel=JUNCTION_POINT;
    
    exploreChain(Point(1,0), 1,chain6, graphemes4, visited4, mergeTable6,juncLabel,juncMap);
    assert(endMerge(3,mergeTable6)  !=  endMerge(1,mergeTable6));
    assert(endMerge(4,mergeTable6)  !=  endMerge(1,mergeTable6));
    assert(endMerge(1,mergeTable6)  ==  endMerge(5,mergeTable6));
    assert(endMerge(1,mergeTable6)  ==  endMerge(2,mergeTable6));
    
    visited4 = graphemes4.clone();
    chain6.clear();
    mergeTable6.clear();
    juncMap.clear();
    juncLabel=JUNCTION_POINT;
    exploreChain(Point(0,2), 3,chain6, graphemes4, visited4, mergeTable6,juncLabel,juncMap);
    assert(endMerge(3,mergeTable6)  !=  endMerge(1,mergeTable6));
    assert(endMerge(4,mergeTable6)  !=  endMerge(1,mergeTable6));
    assert(endMerge(1,mergeTable6)  ==  endMerge(5,mergeTable6));
    assert(endMerge(1,mergeTable6)  ==  endMerge(2,mergeTable6));
//    assert(graphemes4.at<unsigned char>(2,1) == endMerge(1,mergeTable6));
//    assert(graphemes4.at<unsigned char>(3,4) == endMerge(1,mergeTable6));
    
    list<int> labels4 = repairLoops(graphemes4);
    assert(labels4.size()==3);
    assert(graphemes4.at<unsigned char>(2,3)==graphemes4.at<unsigned char>(0,3));
    assert(graphemes4.at<unsigned char>(2,0)!=graphemes4.at<unsigned char>(2,1));
    assert(graphemes4.at<unsigned char>(4,4)!=graphemes4.at<unsigned char>(2,1));
    
    
    
    Mat graphemes5 = (Mat_<unsigned char>(6,6) <<  0,  1,  1, 1, 5, 0,
                                                   0,  1,  0, 0, 0, 5,
                                                   0,  1,  0, 0, 0, 5,
                                                   3,  j,  2, 0, 5, 0,
                                                   3,  0,  0, 2, j, 0,
                                                   3,  0,  0, 0, 4, 0);
    Mat visited5 = graphemes5.clone();
    list<int> chain7;
    map<int, int> mergeTable7;
    juncMap.clear();
    juncLabel=JUNCTION_POINT;
    exploreChain(Point(1,0), 1,chain7, graphemes5, visited5, mergeTable7,juncLabel,juncMap);
    assert(endMerge(3,mergeTable7)  !=  endMerge(1,mergeTable7));
    assert(endMerge(4,mergeTable7)  !=  endMerge(1,mergeTable7));
    assert(endMerge(1,mergeTable7)  ==  endMerge(5,mergeTable7));
    assert(endMerge(1,mergeTable7)  ==  endMerge(2,mergeTable7));
    
    visited5 = graphemes5.clone();
    chain7.clear();
    mergeTable7.clear();
    juncMap.clear();
    juncLabel=JUNCTION_POINT;
    exploreChain(Point(0,4), 3,chain7, graphemes5, visited5, mergeTable7,juncLabel,juncMap);
    assert(endMerge(3,mergeTable7)  !=  endMerge(1,mergeTable7));
    assert(endMerge(4,mergeTable7)  !=  endMerge(1,mergeTable7));
    assert(endMerge(1,mergeTable7)  ==  endMerge(5,mergeTable7));
    assert(endMerge(1,mergeTable7)  ==  endMerge(2,mergeTable7));
    
    list<int> labels5 = repairLoops(graphemes5);
    assert(labels5.size()==3);
    assert(graphemes5.at<unsigned char>(0,4)==graphemes5.at<unsigned char>(0,3));
    assert(graphemes5.at<unsigned char>(3,0)!=graphemes5.at<unsigned char>(3,1));
    assert(graphemes5.at<unsigned char>(4,4)==graphemes5.at<unsigned char>(4,3));
    assert(graphemes5.at<unsigned char>(4,4)!=graphemes5.at<unsigned char>(5,4));
    
    Mat graphemes6 = (Mat_<unsigned char>(6,6) <<  0,  1,  0, 0, 0, 0,
                                                   0,  1,  0, 0, 0, 5,
                                                   0,  1,  0, 0, 5, 0,
                                                   0,  0,  j, j, 0, 0,
                                                   0,  3,  0, 0, 4, 0,
                                                   3,  0,  0, 0, 4, 0);
    Mat visited6 = graphemes6.clone();
    list<int> chain8;
    map<int, int> mergeTable8;
    juncMap.clear();
    juncLabel=JUNCTION_POINT;
    exploreChain(Point(1,0), 1,chain8, graphemes6, visited6, mergeTable8,juncLabel,juncMap);
    assert(endMerge(3,mergeTable8)  !=  endMerge(1,mergeTable8));
    assert(endMerge(3,mergeTable8)  !=  endMerge(5,mergeTable8));
    assert(endMerge(1,mergeTable8)  !=  endMerge(4,mergeTable8));
    assert(endMerge(1,mergeTable8)  !=  endMerge(5,mergeTable8));
    
    visited6 = graphemes6.clone();
    chain8.clear();
    mergeTable8.clear();
    juncMap.clear();
    juncLabel=JUNCTION_POINT;
    exploreChain(Point(0,5), 1,chain8, graphemes6, visited6, mergeTable8,juncLabel,juncMap);
    assert(endMerge(3,mergeTable8)  !=  endMerge(1,mergeTable8));
    assert(endMerge(3,mergeTable8)  !=  endMerge(5,mergeTable8));
    assert(endMerge(1,mergeTable8)  !=  endMerge(4,mergeTable8));
    assert(endMerge(1,mergeTable8)  !=  endMerge(5,mergeTable8));
    
    list<int> labels6 = repairLoops(graphemes6);
    assert(labels6.size()==4);
    assert(graphemes6.at<unsigned char>(0,1)!=graphemes6.at<unsigned char>(5,0));
    assert(graphemes6.at<unsigned char>(0,1)!=graphemes6.at<unsigned char>(1,5));
    assert(graphemes6.at<unsigned char>(0,1)!=graphemes6.at<unsigned char>(5,4));
    assert(graphemes6.at<unsigned char>(4,4)!=graphemes6.at<unsigned char>(1,5));
    
    
    Mat graphemes7 = (Mat_<unsigned char>(6,6) <<  0,  1,  1, 5, 5, 0,
                                                   0,  1,  0, 0, 0, 5,
                                                   0,  1,  0, 0, 5, 0,
                                                   0,  0,  j, j, 0, 0,
                                                   0,  3,  0, 0, 4, 0,
                                                   3,  0,  0, 0, 4, 0);
    Mat visited7 = graphemes7.clone();
    list<int> chain9;
    map<int, int> mergeTable9;
    juncMap.clear();
    juncLabel=JUNCTION_POINT;
    exploreChain(Point(1,0), 1,chain9, graphemes7, visited7, mergeTable9,juncLabel,juncMap);
    assert(endMerge(3,mergeTable9)  !=  endMerge(1,mergeTable9));
    assert(endMerge(3,mergeTable9)  !=  endMerge(5,mergeTable9));
    assert(endMerge(1,mergeTable9)  !=  endMerge(4,mergeTable9));
    assert(endMerge(1,mergeTable9)  ==  endMerge(5,mergeTable9));
    
    visited7 = graphemes7.clone();
    chain9.clear();
    mergeTable9.clear();
    juncMap.clear();
    juncLabel=JUNCTION_POINT;
    exploreChain(Point(0,5), 3,chain9, graphemes7, visited7, mergeTable9,juncLabel,juncMap);
    assert(endMerge(3,mergeTable9)  !=  endMerge(1,mergeTable9));
    assert(endMerge(3,mergeTable9)  !=  endMerge(5,mergeTable9));
    assert(endMerge(1,mergeTable9)  !=  endMerge(4,mergeTable9));
    assert(endMerge(1,mergeTable9)  ==  endMerge(5,mergeTable9));
    
    list<int> labels7 = repairLoops(graphemes7);
    assert(labels7.size()==3);
    assert(graphemes7.at<unsigned char>(0,2)!=graphemes7.at<unsigned char>(5,0));
    assert(graphemes7.at<unsigned char>(0,2)==graphemes7.at<unsigned char>(1,5));
    assert(graphemes7.at<unsigned char>(0,2)==graphemes7.at<unsigned char>(3,3));
    assert(graphemes7.at<unsigned char>(0,2)==graphemes7.at<unsigned char>(3,2));
    assert(graphemes7.at<unsigned char>(4,4)!=graphemes7.at<unsigned char>(3,2));
    
    
    Mat graphemes8 = (Mat_<unsigned char>(6,6) <<  0,  0,  1, 5, 5, 0,
                                                   0,  1,  0, 0, 0, 5,
                                                   1,  0,  0, 0, 5, 0,
                                                   0,  j,  6, j, 0, 0,
                                                   3,  0,  0, 0, 4, 0,
                                                   3,  0,  0, 0, 4, 0);
    Mat visited8 = graphemes8.clone();
    list<int> chain10;
    map<int, int> mergeTable10;
    juncMap.clear();
    juncLabel=JUNCTION_POINT;
    exploreChain(Point(1,0), 1,chain10, graphemes8, visited8, mergeTable10,juncLabel,juncMap);
    assert(endMerge(3,mergeTable10)  !=  endMerge(1,mergeTable10));
    assert(endMerge(3,mergeTable10)  !=  endMerge(5,mergeTable10));
    assert(endMerge(1,mergeTable10)  !=  endMerge(4,mergeTable10));
    assert(endMerge(1,mergeTable10)  ==  endMerge(5,mergeTable10));
    assert(endMerge(1,mergeTable10)  ==  endMerge(6,mergeTable10));
    
    list<int> labels8 = repairLoops(graphemes8);
    assert(labels8.size()==3);
    assert(graphemes8.at<unsigned char>(0,2)!=graphemes8.at<unsigned char>(5,0));
    assert(graphemes8.at<unsigned char>(0,2)==graphemes8.at<unsigned char>(1,5));
    assert(graphemes8.at<unsigned char>(0,2)==graphemes8.at<unsigned char>(3,3));
    assert(graphemes8.at<unsigned char>(0,2)==graphemes8.at<unsigned char>(3,2));
    assert(graphemes8.at<unsigned char>(0,2)==graphemes8.at<unsigned char>(3,1));
    assert(graphemes8.at<unsigned char>(4,4)!=graphemes8.at<unsigned char>(3,1));
    assert(graphemes8.at<unsigned char>(4,4)!=graphemes8.at<unsigned char>(3,3));
    
    Mat graphemesX1 = (Mat_<unsigned char>(6,6) << 0,  1,  1, 0, 0, 0,
                                                   1,  0,  2, 2, 2, 0,
                                                   1,  0,  0, 0, 2, 0,
                                                   1,  1,  j, 2, 0, 0,
                                                   0,  0,  3, 0, 0, 0,
                                                   0,  0,  3, 0, 0, 0);
    Mat visitedX1 = graphemesX1.clone();
    list<int> chainX1;
    map<int, int> mergeTableX1;
    juncMap.clear();
    juncLabel=JUNCTION_POINT;
    exploreChain(Point(1,0), 1,chainX1, graphemesX1, visitedX1, mergeTableX1,juncLabel,juncMap);
    assert(endMerge(1,mergeTableX1)  ==  endMerge(2,mergeTableX1));
    
    visitedX1 = graphemesX1.clone();
    mergeTableX1.clear();
    juncMap.clear();
    juncLabel=JUNCTION_POINT;
    exploreChain(Point(1,4), 1,chainX1, graphemesX1, visitedX1, mergeTableX1,juncLabel,juncMap);
    assert(endMerge(1,mergeTableX1)  ==  endMerge(2,mergeTableX1));
    
    visitedX1 = graphemesX1.clone();
    mergeTableX1.clear();
    juncMap.clear();
    juncLabel=JUNCTION_POINT;
    exploreChain(Point(5,2), 1,chainX1, graphemesX1, visitedX1, mergeTableX1,juncLabel,juncMap);
    assert(endMerge(1,mergeTableX1)  ==  endMerge(2,mergeTableX1));
    
    
    list<int> labelsX1 = repairLoops(graphemesX1);
    assert(labelsX1.size()==2);
    assert(graphemesX1.at<unsigned char>(0,1)==graphemesX1.at<unsigned char>(1,2));
    
    
    Mat graphemesX2 = (Mat_<unsigned char>(6,6) << 0,  0,  3, 7, 7, 0,
                                                   0,  3,  0, 0, 0, 7,
                                                   0,  j,  6, j,10, j,
                                                   0,  2,  0, 5, 0, 8,
                                                   0,  2,  0, 5, 0, 9,
                                                   1,  j,  4, j, 9, 0);
    Mat visitedX2 = graphemesX2.clone();
    list<int> chainX2;
    map<int, int> mergeTableX2;
    juncMap.clear();
    juncLabel=JUNCTION_POINT;
    exploreChain(Point(0,5), 1,chainX2, graphemesX2, visitedX2, mergeTableX2,juncLabel,juncMap);
    assert(endMerge(1,mergeTableX2)  !=  endMerge(2,mergeTableX2));
    assert(endMerge(3,mergeTableX2)  ==  endMerge(2,mergeTableX2));
    assert(endMerge(3,mergeTableX2)  ==  endMerge(4,mergeTableX2));
    assert(endMerge(3,mergeTableX2)  ==  endMerge(5,mergeTableX2));
    assert(endMerge(3,mergeTableX2)  ==  endMerge(6,mergeTableX2));
    assert(endMerge(3,mergeTableX2)  ==  endMerge(7,mergeTableX2));
    assert(endMerge(3,mergeTableX2)  ==  endMerge(8,mergeTableX2));
    assert(endMerge(3,mergeTableX2)  ==  endMerge(9,mergeTableX2));
    assert(endMerge(3,mergeTableX2)  ==  endMerge(10,mergeTableX2));
    
    
    list<int> labelsX2 = repairLoops(graphemesX2);
    assert(labelsX2.size()==2);
    assert(graphemesX2.at<unsigned char>(5,0)!=graphemesX2.at<unsigned char>(5,1));
    assert(graphemesX2.at<unsigned char>(0,4)==graphemesX2.at<unsigned char>(5,1));
    assert(graphemesX2.at<unsigned char>(0,4)==graphemesX2.at<unsigned char>(3,3));
    
    
    Mat graphemesX3 = (Mat_<unsigned char>(6,6) << 3,  3,  3, 3, 0, 0,
                                                   0,  0,  0, 3, 0, 0,
                                                   0,  2,  2, j, 0, 0,
                                                   0,  2,  0, j, 1, 0,
                                                   0,  2,  j, 0, 1, 0,
                                                   0,  0,  1, 1, 1, 0);
    Mat visitedX3 = graphemesX3.clone();
    list<int> chainX3;
    map<int, int> mergeTableX3;
    juncMap.clear();
    juncLabel=JUNCTION_POINT;
    exploreChain(Point(0,0), 3,chainX3, graphemesX3, visitedX3, mergeTableX3,juncLabel,juncMap);
    assert(endMerge(3,mergeTableX3)  !=  endMerge(2,mergeTableX3));
    assert(endMerge(1,mergeTableX3)  ==  endMerge(2,mergeTableX3));
    
    
    list<int> labelsX3 = repairLoops(graphemesX3);
    assert(labelsX3.size()==2);
    assert(graphemesX3.at<unsigned char>(2,3)==graphemesX3.at<unsigned char>(2,1));
    assert(graphemesX3.at<unsigned char>(3,3)==graphemesX3.at<unsigned char>(2,1));
    assert(graphemesX3.at<unsigned char>(4,2)==graphemesX3.at<unsigned char>(2,1));
    
    cout << "repairLoops tests passed" << endl;
    
    Mat skel1 = (Mat_<unsigned char>(6,6) <<       0, 255, 255,   0, 255, 255,
                                                   0,   0,   0, 255,   0,   0,
                                                   0,   0,   0, 255,   0,   0,
                                                   0,   0,   0, 255,   0,   0,
                                                   0,   0,   0, 255,   0,   0,
                                                   0,   0,   0,   0,   0,   0);
    Mat seg1 = breakSegments(skel1);
    assert(seg1.at<unsigned char>(0,1) != seg1.at<unsigned char>(0,5));
    assert(seg1.at<unsigned char>(0,1) != seg1.at<unsigned char>(4,3));
    assert(seg1.at<unsigned char>(4,3) != seg1.at<unsigned char>(0,5));
    assert(seg1.at<unsigned char>(1,3) == j);
    
    Mat skel2 = (Mat_<unsigned char>(6,6) <<       0, 255, 255, 255, 255,   0,
                                                   0, 255,   0,   0,   0, 255,
                                                   0, 255,   0, 255,   0, 255,
                                                 255, 255, 255,   0, 255,   0,
                                                 255,   0,   0,   0, 255,   0,
                                                 255,   0,   0,   0,   0, 255);
    Mat seg2 = breakSegments(skel2);
    assert(seg2.at<unsigned char>(5,0) != seg2.at<unsigned char>(1,1));
    assert(seg2.at<unsigned char>(1,1) != seg2.at<unsigned char>(5,5));
    assert(seg2.at<unsigned char>(1,1) != seg2.at<unsigned char>(2,3));
//    assert(seg2.at<unsigned char>(1,1) != seg2.at<unsigned char>(2,5));??
    assert(seg2.at<unsigned char>(3,4) == j);
    assert(seg2.at<unsigned char>(5,0) != seg2.at<unsigned char>(2,5));
    assert(seg2.at<unsigned char>(2,5) != seg2.at<unsigned char>(5,5));
    assert(seg2.at<unsigned char>(2,5) != seg2.at<unsigned char>(2,3));
    
    
    Mat skel5 = (Mat_<unsigned char>(6,6) <<       0, 255,   0,   0,   0, 255,
                                                   0, 255,   0,   0, 255,   0,
                                                   0, 255,   0,   0, 255,   0,
                                                   0,   0, 255, 255,   0,   0,
                                                   0, 255,   0,   0, 255,   0,
                                                 255,   0,   0,   0, 255,   0);
    Mat seg5 = breakSegments(skel5);
    assert(seg5.at<unsigned char>(5,0) != seg5.at<unsigned char>(0,1));
    assert(seg5.at<unsigned char>(0,5) != seg5.at<unsigned char>(0,1));
    assert(seg5.at<unsigned char>(5,0) != seg5.at<unsigned char>(5,4));
    assert(seg5.at<unsigned char>(3,2) ==j);
    assert(seg5.at<unsigned char>(3,3) ==j);
    
    Mat skel6 = (Mat_<unsigned char>(6,6) <<     255,   0,   0,   0,   0, 255,
                                                 255,   0,   0,   0, 255,   0,
                                                 255,   0,   0,   0, 255,   0,
                                                   0, 255, 255, 255,   0,   0,
                                                 255,   0,   0,   0, 255,   0,
                                                 255,   0,   0,   0, 255,   0);
    Mat seg6 = breakSegments(skel6);
    assert(seg6.at<unsigned char>(5,0) != seg6.at<unsigned char>(0,0));
    assert(seg6.at<unsigned char>(0,5) != seg6.at<unsigned char>(0,0));
    assert(seg6.at<unsigned char>(5,0) != seg6.at<unsigned char>(5,4));
    assert(seg6.at<unsigned char>(3,1) ==j);
    assert(seg6.at<unsigned char>(3,3) ==j);
    assert(seg6.at<unsigned char>(0,0) != seg6.at<unsigned char>(3,2));
    assert(seg6.at<unsigned char>(0,5) != seg6.at<unsigned char>(3,2));
    assert(seg6.at<unsigned char>(5,0) != seg6.at<unsigned char>(3,2));
    assert(seg6.at<unsigned char>(5,4) != seg6.at<unsigned char>(3,2));
    
    Mat skel7 = (Mat_<unsigned char>(6,6) <<       0,   0,   0,   0, 255,   0,
                                                   0,   0,   0, 255, 255,   0,
                                                   0,   0, 255, 255,   0,   0,
                                                   0, 255, 255,   0, 255,   0,
                                                   0, 255,   0,   0,   0, 255,
                                                 255,   0,   0,   0,   0,   0);
    Mat seg7 = breakSegments(skel7);
    assert(seg7.at<unsigned char>(5,0) != seg7.at<unsigned char>(0,4));
    assert(seg7.at<unsigned char>(5,0) != seg7.at<unsigned char>(0,0));
    assert(seg7.at<unsigned char>(5,0) != seg7.at<unsigned char>(4,5));
    assert(seg7.at<unsigned char>(5,0) == seg7.at<unsigned char>(4,1));
    assert(seg7.at<unsigned char>(5,0) == seg7.at<unsigned char>(3,1));
    assert(seg7.at<unsigned char>(5,0) == seg7.at<unsigned char>(3,2));
    assert(seg7.at<unsigned char>(0,4) == seg7.at<unsigned char>(1,4));
    assert(seg7.at<unsigned char>(3,4) == seg7.at<unsigned char>(4,5));
    
    Mat skel8 = (Mat_<unsigned char>(7,7) <<       0,   0,   0,   0,   0, 255,255,
	                                	           0,   0,   0, 255, 255,   0,  0,
                                                   0,   0, 255, 255,   0,   0,  0,
                                                   0, 255, 255,   0,   0,   0,  0,
                                                   0, 255,   0,   0,   0,   0,  0,
                                                 255,   0,   0,   0,   0,   0,  0,
                                                 255,   0,   0,   0,   0,   0,  0);
    Mat seg8 = breakSegments(skel8);
    assert(seg8.at<unsigned char>(6,0) == seg8.at<unsigned char>(0,6));
    assert(seg8.at<unsigned char>(6,0) == seg8.at<unsigned char>(2,2));
    assert(seg8.at<unsigned char>(6,0) == seg8.at<unsigned char>(3,2));
    
    Mat skel9 = (Mat_<unsigned char>(7,7) <<       0,   0,   0,   0,   0, 255,255,
	                                	           0,   0,   0, 255, 255,   0,255,
                                                   0,   0, 255, 255,   0,   0,255,
                                                   0, 255, 255,   0,   0,   0,255,
                                                   0, 255,   0,   0,   0,   0,255,
                                                 255,   0,   0,   0,   0,   0,255,
                                                 255, 255, 255, 255, 255, 255,255);
    Mat seg9 = breakSegments(skel9);
//    repairLoops(seg9);
    assert(seg9.at<unsigned char>(6,0) == seg9.at<unsigned char>(1,4));
    assert(seg9.at<unsigned char>(6,0) == seg9.at<unsigned char>(2,2));
    assert(seg9.at<unsigned char>(6,0) == seg9.at<unsigned char>(3,2));
//    assert(seg9.at<unsigned char>(6,0) == seg9.at<unsigned char>(6,6));

    
    Mat skel10 = (Mat_<unsigned char>(7,7) <<    255, 255, 255, 255,   0,   0,  0,
	                                	           0, 255,   0, 255,   0,   0,  0,
                                                   0,   0, 255, 255, 255, 255,255,
                                                   0,   0,   0,   0,   0, 255,  0,
                                                   0,   0,   0,   0,   0, 255,  0,
                                                   0,   0,   0,   0,   0,   0,  0,
                                                   0,   0,   0,   0,   0,   0,  0);
    Mat seg10 = breakSegments(skel10);
    assert(seg10.at<unsigned char>(0,0) != seg10.at<unsigned char>(0,2));
    assert(seg10.at<unsigned char>(0,0) != seg10.at<unsigned char>(1,1));
    assert(seg10.at<unsigned char>(1,1) != seg10.at<unsigned char>(0,2));
    assert(seg10.at<unsigned char>(1,1) != seg10.at<unsigned char>(2,4));
    assert(seg10.at<unsigned char>(1,1) != seg10.at<unsigned char>(4,5));
    assert(seg10.at<unsigned char>(2,4) != seg10.at<unsigned char>(4,5));
    
    Mat skel11 = (Mat_<unsigned char>(8,8) <<      0,   0,   0,   0,   0,   0,   0,  0, 
                                                   0, 255, 255, 255, 255,   0,   0,  0,
	                                	           0,   0, 255,   0, 255, 255, 255,  0,
                                                   0,   0,   0, 255, 255,   0, 255,255,
                                                   0,   0,   0,   0,   0,   0, 255,  0,
                                                   0,   0,   0,   0,   0,   0, 255,  0,
                                                 255,   0,   0,   0,   0, 255,   0,  0,
                                                   0, 255, 255, 255, 255,   0,   0,  0);
    Mat seg11 = breakSegments(skel11);
    assert(seg11.at<unsigned char>(1,1) != seg11.at<unsigned char>(1,3));
    assert(seg11.at<unsigned char>(1,1) != seg11.at<unsigned char>(2,2));
    assert(seg11.at<unsigned char>(2,2) != seg11.at<unsigned char>(1,3));
    assert(seg11.at<unsigned char>(2,2) != seg11.at<unsigned char>(2,5));
    assert(seg11.at<unsigned char>(2,2) != seg11.at<unsigned char>(5,6));
    assert(seg11.at<unsigned char>(2,5) != seg11.at<unsigned char>(5,6));

    cout << "min/max tests"<<endl;
    Mat skel3 = (Mat_<unsigned char>(10,10) << 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                                               0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                                               0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                                               0,   0,   0,   0,   0, 255,   0,   0,   0,   0,
                                               0,   0,   0,   0, 255,   0, 255,   0,   0,   0,
                                               0,   0,   0, 255,   0,   0,   0, 255,   0,   0,
                                               0,   0, 255,   0,   0,   0,   0,   0, 255,   0,
                                               0, 255,   0,   0,   0,   0,   0,   0,   0, 255,
                                             255,   0,   0,   0,   0,   0,   0,   0,   0, 255,
                                               0,   0,   0,   0,   0,   0,   0,   0,   0, 255);
    Mat seg3 = breakSegments(skel3);
    assert(seg3.at<unsigned char>(4,4) != seg3.at<unsigned char>(4,6));
    assert(seg3.at<unsigned char>(4,4) == seg3.at<unsigned char>(8,0));
    assert(seg3.at<unsigned char>(4,6) == seg3.at<unsigned char>(8,9));
    
    
    Mat skel4 = (Mat_<unsigned char>(10,10) <<     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                                                   0,   0,   0,   0, 255, 255,   0,   0,   0,   0,
                                                   0,   0, 255, 255,   0,   0, 255, 255,   0,   0,
                                                   0, 255,   0,   0,   0,   0,   0,   0, 255,   0,
                                                 255,   0,   0,   0,   0,   0,   0,   0,   0, 255,
                                                 255,   0,   0,   0,   0,   0,   0, 255, 255,   0,
                                                 255,   0,   0,   0,   0,   0, 255,   0,   0,   0,
                                                   0, 255, 255, 255,   0,   0, 255,   0,   0,   0,
                                                   0,   0,   0,   0, 255,   0, 255,   0,   0,   0,
                                                   0,   0,   0,   0,   0, 255,   0,   0,   0,   0);
    Mat seg4 = breakSegments(skel4);
    assert(seg4.at<unsigned char>(2,3) != seg4.at<unsigned char>(2,6));
    assert(seg4.at<unsigned char>(8,4) != seg4.at<unsigned char>(8,6));
    assert(seg4.at<unsigned char>(2,3) == seg4.at<unsigned char>(8,4));
    assert(seg4.at<unsigned char>(8,6) == seg4.at<unsigned char>(2,6));
    
    Mat skel12 = (Mat_<unsigned char>(10,10) <<    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                                                   0,   0,   0,   0,   0,   0, 255,   0,   0,   0,
                                                   0,   0,   0,   0,   0, 255,   0, 255,   0,   0,
                                                   0,   0,   0,   0, 255,   0,   0, 255,   0,   0,
                                                 255,   0,   0,   0, 255,   0,   0, 255,   0,   0,
                                                   0, 255,   0, 255,   0,   0,   0,   0, 255, 255,
                                                   0,   0, 255, 255,   0,   0,   0,   0, 255,   0,
                                                   0,   0, 255,   0,   0,   0,   0, 255,   0,   0,
                                                   0,   0, 255,   0,   0,   0, 255,   0,   0,   0,
                                                   0,   0,   0, 255, 255, 255,   0,   0,   0,   0);
    Mat seg12 = breakSegments(skel12);
    assert(seg12.at<unsigned char>(4,0) != seg12.at<unsigned char>(5,3));
    assert(seg12.at<unsigned char>(7,2) != seg12.at<unsigned char>(5,3));
    
    Mat skel13 = (Mat_<unsigned char>(14,14) <<  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                                                 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                                                 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                                                 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                                                 0,   0,   0,   0, 255, 255, 255, 255, 255, 255, 255, 255, 255,   0,
                                                 0,   0,   0,   0, 255,   0,   0,   0,   0,   0,   0,   0,   0, 255,
                                                 0,   0,   0, 255,   0,   0,   0,   0,   0,   0,   0,   0,   0, 255,
                                                 0,   0, 255,   0,   0,   0,   0,   0,   0,   0,   0, 255, 255, 255,
                                                 0, 255,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 255,
                                               255,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 255,
                                                 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 255, 255,   0,
                                                 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                                                 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                                                 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0);
    Mat seg13 = breakSegments(skel13);
    assert(seg13.at<unsigned char>(9,0) != seg13.at<unsigned char>(6,13));
    
    Mat skel14 = (Mat_<unsigned char>(10,10) <<    0,   0,   0,   0, 255,   0,   0,   0,   0,   0,
                                                   0,   0,   0, 255,   0,   0,   0,   0,   0,   0,
                                                   0, 255, 255,   0,   0,   0,   0,   0,   0,   0,
                                                   0, 255,   0,   0,   0,   0,   0,   0,   0,   0,
                                                 255,   0,   0,   0,   0,   0,   0,   0,   0, 255,
                                                   0, 255,   0,   0,   0,   0,   0,   0,   0, 255,
                                                   0, 255,   0,   0,   0,   0,   0,   0,   0, 255,
                                                   0,   0, 255,   0,   0,   0,   0,   0, 255,   0,
                                                   0,   0, 255,   0,   0,   0,   0,   0, 255,   0,
                                                   0,   0,   0, 255, 255, 255, 255, 255,   0,   0);
    Mat seg14 = breakSegments(skel14);
    assert(seg14.at<unsigned char>(0,4) == seg14.at<unsigned char>(8,2));
    assert(seg14.at<unsigned char>(4,9) != seg14.at<unsigned char>(8,2));
    
    Mat skel16 = (Mat_<unsigned char>(10,10) <<    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                                                   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                                                   0,   0,   0, 255,   0,   0,   0,   0,   0,   0,
                                                   0,   0,   0, 255,   0,   0,   0,   0,   0,   0,
                                                   0,   0,   0, 255,   0,   0,   0,   0,   0,   0,
                                                   0,   0, 255, 255, 255,   0,   0,   0,   0,   0,
                                                   0, 255, 255,   0,   0, 255,   0,   0,   0,   0,
                                                 255, 255,   0,   0,   0,   0, 255,   0,   0,   0,
                                                   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                                                   0,   0,   0,   0,   0,   0,   0,   0,   0,   0);
    Mat seg16 = breakSegments(skel16);
    assert(seg16.at<unsigned char>(4,3) != seg16.at<unsigned char>(5,2));
    assert(seg16.at<unsigned char>(4,3) != seg16.at<unsigned char>(5,4));
    
    Mat skel17 = (Mat_<unsigned char>(10,10) <<  255,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                                                   0, 255, 255,   0,   0,   0,   0,   0,   0,   0,
                                                   0,   0,   0, 255,   0,   0,   0,   0,   0,   0,
                                                   0,   0,   0, 255,   0,   0,   0,   0,   0,   0,
                                                   0,   0,   0, 255,   0,   0,   0,   0,   0,   0,
                                                   0,   0, 255, 255, 255,   0,   0,   0,   0,   0,
                                                   0, 255, 255,   0,   0, 255,   0,   0,   0,   0,
                                                 255, 255,   0,   0,   0,   0, 255,   0,   0,   0,
                                                   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                                                   0,   0,   0,   0,   0,   0,   0,   0,   0,   0);
    Mat seg17 = breakSegments(skel17);
    assert(seg17.at<unsigned char>(4,3) != seg17.at<unsigned char>(5,2));
    assert(seg17.at<unsigned char>(4,3) != seg17.at<unsigned char>(5,4));
    
    Mat skel15 = (Mat_<unsigned char>(10,10) <<  255, 255, 255, 255,   0,   0,   0,   0,   0,   0,
                                                   0,   0,   0, 255,   0,   0,   0,   0,   0,   0,
                                                   0,   0, 255, 255, 255,   0,   0,   0,   0,   0,
                                                 255,   0, 255,   0,   0, 255,   0, 255, 255, 255,
                                                 255,   0,   0,   0,   0, 255, 255, 255,   0,   0,
                                                   0, 255,   0,   0,   0,   0, 255,   0,   0,   0,
                                                   0,   0, 255,   0,   0,   0, 255,   0,   0,   0,
                                                   0,   0,   0, 255, 255, 255,   0,   0,   0,   0,
                                                   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                                                   0,   0,   0,   0,   0,   0,   0,   0,   0,   0);
    Mat seg15 = breakSegments(skel15);
    assert(seg15.at<unsigned char>(5,6) != seg15.at<unsigned char>(4,5));
    assert(seg15.at<unsigned char>(5,6) != seg15.at<unsigned char>(4,7));
    assert(seg15.at<unsigned char>(4,5) != seg15.at<unsigned char>(3,2));
    assert(seg15.at<unsigned char>(4,5) != seg15.at<unsigned char>(0,3));
    assert(seg15.at<unsigned char>(4,6) == j);
    assert(seg15.at<unsigned char>(2,3) == j);
    
    Mat skel18 = (Mat_<unsigned char>(10,10) <<  255,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                                                   0, 255,   0,   0, 255, 255, 255,   0,   0,   0,
                                                 255, 255, 255, 255,   0,   0, 255,   0,   0,   0,
                                                 255,   0,   0,   0,   0,   0, 255,   0,   0,   0,
                                                 255,   0,   0,   0,   0, 255, 255, 255, 255,   0,
                                                 255,   0,   0,   0, 255, 255,   0,   0,   0,   0,
                                                   0, 255, 255, 255,   0, 255,   0,   0,   0,   0,
                                                   0,   0,   0, 255, 255,   0,   0,   0,   0,   0,
                                                   0,   0,   0,   0, 255,   0,   0,   0,   0,   0,
                                                   0,   0,   0,   0,   0, 255,   0,   0,   0,   0);
    Mat seg18 = breakSegments(skel18);
    assert(seg18.at<unsigned char>(0,0) != seg18.at<unsigned char>(2,0));
    assert(seg18.at<unsigned char>(0,0) != seg18.at<unsigned char>(2,2));
    assert(seg18.at<unsigned char>(0,0) != seg18.at<unsigned char>(7,3));
    assert(seg18.at<unsigned char>(9,5) != seg18.at<unsigned char>(7,3));
    assert(seg18.at<unsigned char>(5,4) != seg18.at<unsigned char>(7,3));
    assert(seg18.at<unsigned char>(6,5) != seg18.at<unsigned char>(7,3));
    assert(seg18.at<unsigned char>(4,5) != seg18.at<unsigned char>(5,4));
    assert(seg18.at<unsigned char>(6,5) != seg18.at<unsigned char>(5,4));
    
    visited1 = seg18.clone();
    chain1.clear();
    mergeTable.clear();
    juncMap.clear();
    juncLabel=JUNCTION_POINT;
    exploreChain(Point(0,0), seg18.at<unsigned char>(0,0),chain1, seg18, visited1, mergeTable,juncLabel,juncMap);
    assert(endMerge(seg18.at<unsigned char>(0,0),mergeTable)  !=  endMerge(seg18.at<unsigned char>(2,0),mergeTable));
    assert(endMerge(seg18.at<unsigned char>(2,2),mergeTable)  ==  endMerge(seg18.at<unsigned char>(2,0),mergeTable));
    assert(endMerge(seg18.at<unsigned char>(2,2),mergeTable)  ==  endMerge(seg18.at<unsigned char>(7,3),mergeTable));
    assert(endMerge(seg18.at<unsigned char>(2,2),mergeTable)  ==  endMerge(seg18.at<unsigned char>(5,4),mergeTable));
    assert(endMerge(seg18.at<unsigned char>(2,2),mergeTable)  ==  endMerge(seg18.at<unsigned char>(6,5),mergeTable));
    assert(endMerge(seg18.at<unsigned char>(2,2),mergeTable)  ==  endMerge(seg18.at<unsigned char>(4,5),mergeTable));
    
    
    
    cout << "breakSegments tests passed" << endl;
    
    int thresh=150;
    
    Mat testWord = imread("/home/brian/intel_index/data/gw_20p_wannot/deslant/wordimg_21.png");
    cvtColor( testWord, testWord, CV_RGB2GRAY );
    threshold(testWord,testWord,thresh,255,1);
//    imshow ("test word",testWord);
//    waitKey(5);
    int top,bot;
    thinning(testWord,skel1);
//    imshow ("test skel",skel1);
//    waitKey(5);
    
    list<Point> localMins, localMaxs;
    int centerLine;
    Mat graphemes = breakSegments(skel1,&localMaxs,&localMins, &centerLine);
    findBaselines(false,false,&top,&bot,localMins,localMaxs,centerLine,testWord);
    Mat out; cvtColor(skel1,out,CV_GRAY2RGB);
    for (Point p : localMins)
        out.at<Vec3b>(p) = Vec3b(0,255,0);
    for (Point p : localMaxs)
        out.at<Vec3b>(p) = Vec3b(0,0,255);
    imwrite("minmax.png",out);
    assert(localMins.size()>=2);
    assert(localMaxs.size()>=3);
    
    
    
    assert(top>=2 && top<=12);
    assert(bot>=18 && bot<=27);
    
    
    
    localMins.clear();
    localMaxs.clear();
    
    testWord = imread("/home/brian/intel_index/data/gw_20p_wannot/deslant/wordimg_10.png");
    cvtColor( testWord, testWord, CV_RGB2GRAY );
    threshold(testWord,testWord,thresh,255,1);
    thinning(testWord,skel1);
    
    graphemes = breakSegments(skel1,&localMaxs,&localMins ,&centerLine);
    findBaselines(hasAscender("the"),hasDescender("the"),&top,&bot,localMins,localMaxs,centerLine,testWord);
    cvtColor(skel1,out,CV_GRAY2RGB);
    for (Point p : localMins)
        out.at<Vec3b>(p) = Vec3b(0,255,0);
    for (Point p : localMaxs)
        out.at<Vec3b>(p) = Vec3b(0,0,255);
    imwrite("minmax.png",out);
    assert(localMins.size()==4);
    assert(localMaxs.size()>=4);
    
    assert(top>=42 && top<=59);
    assert(bot>=70 && bot<=80);
   
    localMins.clear();
    localMaxs.clear();
    
    testWord = imread("/home/brian/intel_index/data/gw_20p_wannot/deslant/wordimg_95.png");
    cvtColor( testWord, testWord, CV_RGB2GRAY );
    threshold(testWord,testWord,thresh,255,1);
    thinning(testWord,skel1);
    
    graphemes = breakSegments(skel1,&localMaxs,&localMins,&centerLine);
    findBaselines(hasAscender("immediately"),hasDescender("immediately"),&top,&bot,localMins,localMaxs,centerLine,testWord);
    cvtColor(skel1,out,CV_GRAY2RGB);
    for (Point p : localMins)
        out.at<Vec3b>(p) = Vec3b(0,255,0);
    for (Point p : localMaxs)
        out.at<Vec3b>(p) = Vec3b(0,0,255);
    imwrite("minmax.png",out);
    assert(localMins.size()>4);
    assert(localMaxs.size()>5);
    
    assert(top>=15 && top<=33);
    assert(bot>=46 && bot<=60);
   
    localMins.clear();
    localMaxs.clear();
    

    testWord = imread("/home/brian/intel_index/data/gw_20p_wannot/deslant/wordimg_18.png");
    cvtColor( testWord, testWord, CV_RGB2GRAY );
    threshold(testWord,testWord,thresh,255,1);
    thinning(testWord,skel1);
    
    //imwrite("threshed.png",testWord);
    
    graphemes = breakSegments(skel1,&localMaxs,&localMins,&centerLine);
    findBaselines(true,true,&top,&bot,localMins,localMaxs,centerLine,testWord);
    cvtColor(skel1,out,CV_GRAY2RGB);
    for (Point p : localMins)
        out.at<Vec3b>(p) = Vec3b(0,255,0);
    for (Point p : localMaxs)
        out.at<Vec3b>(p) = Vec3b(0,0,255);
    imwrite("minmax.png",out);
    assert(localMins.size()>=6);
    assert(localMaxs.size()>=6);
    
    assert(top>=40 && top<=56);
    assert(bot>=60 && bot<=73);
   
    localMins.clear();
    localMaxs.clear();
    
    
    
    vector<Grapheme*> gs = extractGraphemes(skel1,&localMaxs,&localMins,&centerLine);
    vector<list<const Grapheme*> > segmentation = learnCharacterSegmentation("from",gs,localMaxs,localMins,centerLine,testWord);
    out = segmentation[0].front()->img()->clone();
    cvtColor(out,out,CV_GRAY2RGB);
    for (const list<const Grapheme*> &gs : segmentation)
    {
        Vec3b color(rand()%256,rand()%256,rand()%256);
        for (const Grapheme* g : gs)
        {
            for (int x=0; x<segmentation[0].front()->img()->cols; x++ )
                for (int y=0; y<segmentation[0].front()->img()->rows; y++ )
                {
                    if (segmentation[0].front()->img()->at<unsigned char>(y,x)==g->imgId())
                        out.at<Vec3b>(y,x)=color;
                }
        }
    }
    imwrite("segmentation.png",out);
    assert(segmentation.size()==4);
    assert(segmentation[0].size()>7);
    assert(segmentation[1].size()>1);
    assert(segmentation[2].size()>1);
    assert(segmentation[3].size()>1);
    for (Grapheme* g : gs) delete g;
    
    localMins.clear();
    localMaxs.clear();

    testWord = imread("/home/brian/intel_index/data/gw_20p_wannot/deslant/wordimg_36.png");
    cvtColor( testWord, testWord, CV_RGB2GRAY );
    threshold(testWord,testWord,thresh,255,1);
    thinning(testWord,skel1);
    
    graphemes = breakSegments(skel1,&localMaxs,&localMins,&centerLine);
    findBaselines(true,false,&top,&bot,localMins,localMaxs,centerLine,testWord);
    cvtColor(skel1,out,CV_GRAY2RGB);
    for (Point p : localMins)
        out.at<Vec3b>(p) = Vec3b(0,255,0);
    for (Point p : localMaxs)
        out.at<Vec3b>(p) = Vec3b(0,0,255);
    imwrite("minmax.png",out);
    assert(localMins.size()>=4);
    assert(localMaxs.size()>=3);
    
    assert(top>=27 && top<=36);
    assert(bot>=41 && bot<=50);
    
    localMins.clear();
    localMaxs.clear();
    gs = extractGraphemes(skel1,&localMaxs,&localMins,&centerLine);
    segmentation = learnCharacterSegmentation("two",gs,localMaxs,localMins,centerLine,testWord);
    out = segmentation[0].front()->img()->clone();
    cvtColor(out,out,CV_GRAY2RGB);
    for (const list<const Grapheme*> &gs : segmentation)
    {
        Vec3b color(rand()%256,rand()%256,rand()%256);
        for (const Grapheme* g : gs)
        {
            for (int x=0; x<segmentation[0].front()->img()->cols; x++ )
                for (int y=0; y<segmentation[0].front()->img()->rows; y++ )
                {
                    if (segmentation[0].front()->img()->at<unsigned char>(y,x)==g->imgId())
                        out.at<Vec3b>(y,x)=color;
                }
        }
    }
    imwrite("segmentation.png",out);
    assert(segmentation.size()==3);
    assert(segmentation[0].size()<=3 || segmentation[0].size()>=1);
    assert(segmentation[1].size()<=8 && segmentation[1].size()>=5);
    assert(segmentation[2].size()==1 || segmentation[2].size()==2);
    for (Grapheme* g : gs) delete g;
    
    
    localMins.clear();
    localMaxs.clear();

    testWord = imread("/home/brian/intel_index/data/gw_20p_wannot/deslant/wordimg_256.png");
    cvtColor( testWord, testWord, CV_RGB2GRAY );
    threshold(testWord,testWord,thresh,255,1);
    thinning(testWord,skel1);
    
    graphemes = breakSegments(skel1,&localMaxs,&localMins,&centerLine);
    findBaselines(hasAscender("you"),hasDescender("you"),&top,&bot,localMins,localMaxs,centerLine,testWord);
    cvtColor(skel1,out,CV_GRAY2RGB);
    for (Point p : localMins)
        out.at<Vec3b>(p) = Vec3b(0,255,0);
    for (Point p : localMaxs)
        out.at<Vec3b>(p) = Vec3b(0,0,255);
    imwrite("minmax.png",out);
    assert(localMins.size()>=4);
    assert(localMaxs.size()>=1);
    
    assert(top>=0 && top<=13);
    assert(bot>=17 && bot<=26);
    
    localMins.clear();
    localMaxs.clear();
    gs = extractGraphemes(skel1,&localMaxs,&localMins,&centerLine);
    segmentation = learnCharacterSegmentation("you",gs,localMaxs,localMins,centerLine,testWord);
    
    out = segmentation[0].front()->img()->clone();
    cvtColor(out,out,CV_GRAY2RGB);
    for (const list<const Grapheme*> &gs : segmentation)
    {
        Vec3b color(rand()%256,rand()%256,rand()%256);
        for (const Grapheme* g : gs)
        {
            for (int x=0; x<segmentation[0].front()->img()->cols; x++ )
                for (int y=0; y<segmentation[0].front()->img()->rows; y++ )
                {
                    if (segmentation[0].front()->img()->at<unsigned char>(y,x)==g->imgId())
                        out.at<Vec3b>(y,x)=color;
                }
        }
    }
    imwrite("segmentation.png",out);
    
    assert(segmentation.size()==3);
    assert(segmentation[0].size()==6 || segmentation[0].size()==7);
    assert(segmentation[1].size()<=4 && segmentation[1].size()>=1);
    assert(segmentation[2].size()<=5 && segmentation[2].size()>=3);
    for (Grapheme* g : gs) delete g;
    
    localMins.clear();
    localMaxs.clear();

    testWord = imread("/home/brian/intel_index/data/gw_20p_wannot/deslant/wordimg_39.png");
    cvtColor( testWord, testWord, CV_RGB2GRAY );
    threshold(testWord,testWord,thresh,255,1);
    thinning(testWord,skel1);
    
    graphemes = breakSegments(skel1,&localMaxs,&localMins,&centerLine);
    cvtColor(skel1,out,CV_GRAY2RGB);
    for (Point p : localMins)
        out.at<Vec3b>(p) = Vec3b(0,255,0);
    for (Point p : localMaxs)
        out.at<Vec3b>(p) = Vec3b(0,0,255);
    imwrite("minmax.png",out);
    findBaselines(hasAscender("of"),hasDescender("of"),&top,&bot,localMins,localMaxs,centerLine,testWord);
    
    
    assert(localMins.size()>=2);
    assert(localMaxs.size()>=2);
    
    assert(top<=60);
    assert(bot>=68);
    
    localMins.clear();
    localMaxs.clear();
    
    testWord = imread("/home/brian/intel_index/data/gw_20p_wannot/deslant/wordimg_84.png");
    cvtColor( testWord, testWord, CV_RGB2GRAY );
    threshold(testWord,testWord,thresh,255,1);
    thinning(testWord,skel1);
    
    graphemes = breakSegments(skel1,&localMaxs,&localMins,&centerLine);
    cvtColor(skel1,out,CV_GRAY2RGB);
    for (Point p : localMins)
        out.at<Vec3b>(p) = Vec3b(0,255,0);
    for (Point p : localMaxs)
        out.at<Vec3b>(p) = Vec3b(0,0,255);
    imwrite("minmax.png",out);
    findBaselines(hasAscender("Officers"),hasDescender("Officers"),&top,&bot,localMins,localMaxs,centerLine,testWord);
    
    
    assert(localMins.size()>=2);
    assert(localMaxs.size()>=2);
    
    assert(21<=top&&top<=33);
    assert(37<=bot&&bot<=47);
    
    localMins.clear();
    localMaxs.clear();
    
    testWord = imread("/home/brian/intel_index/data/gw_20p_wannot/deslant/wordimg_113.png");
    cvtColor( testWord, testWord, CV_RGB2GRAY );
    threshold(testWord,testWord,thresh,255,1);
    thinning(testWord,skel1);
    
    graphemes = breakSegments(skel1,&localMaxs,&localMins,&centerLine);
    cvtColor(skel1,out,CV_GRAY2RGB);
    for (Point p : localMins)
        out.at<Vec3b>(p) = Vec3b(0,255,0);
    for (Point p : localMaxs)
        out.at<Vec3b>(p) = Vec3b(0,0,255);
    imwrite("minmax.png",out);
    findBaselines(hasAscender("if"),hasDescender("if"),&top,&bot,localMins,localMaxs,centerLine,testWord);
    
    
    assert(localMins.size()>=2);
    assert(localMaxs.size()>=1);
    
    assert(17<=top&&top<=49);
    assert(51<=bot&&bot<=65);
    
    localMins.clear();
    localMaxs.clear();
    
    testWord = imread("/home/brian/intel_index/data/gw_20p_wannot/deslant/wordimg_216.png");
    cvtColor( testWord, testWord, CV_RGB2GRAY );
    threshold(testWord,testWord,thresh,255,1);
    thinning(testWord,skel1);
    
    graphemes = breakSegments(skel1,&localMaxs,&localMins,&centerLine);
    cvtColor(skel1,out,CV_GRAY2RGB);
    for (Point p : localMins)
        out.at<Vec3b>(p) = Vec3b(0,255,0);
    for (Point p : localMaxs)
        out.at<Vec3b>(p) = Vec3b(0,0,255);
    imwrite("minmax.png",out);
    findBaselines(hasAscender("Letters"),hasDescender("Letters"),&top,&bot,localMins,localMaxs,centerLine,testWord);
    
    
    assert(localMins.size()>=2);
    assert(localMaxs.size()>=2);
    
//    assert(35<=top&&top<=55);
    assert(65<=bot&&bot<=85);

    localMins.clear();
    localMaxs.clear();
    testWord = imread("/home/brian/intel_index/data/gw_20p_wannot/deslanted_threshed/wordimg_1542.png");
    cvtColor( testWord, testWord, CV_RGB2GRAY );
    threshold(testWord,testWord,thresh,255,1);
    thinning(testWord,skel1);
    graphemes = breakSegments(skel1,&localMaxs,&localMins,&centerLine);
    cvtColor(skel1,out,CV_GRAY2RGB);
    for (Point p : localMins)
        out.at<Vec3b>(p) = Vec3b(0,255,0);
    for (Point p : localMaxs)
        out.at<Vec3b>(p) = Vec3b(0,0,255);
    imwrite("minmax.png",out);
    writeGraphemes(graphemes);
    findBaselines(hasAscender("account"),hasDescender("account"),&top,&bot,localMins,localMaxs,centerLine,testWord);
    cout << "account top: " << top << " bot: " << bot << ", center line: " << centerLine << endl;
    localMins.clear();
    localMaxs.clear();
    gs = extractGraphemes(skel1,&localMaxs,&localMins,&centerLine);
    segmentation = learnCharacterSegmentation("account",gs,localMaxs,localMins,centerLine,testWord);
    writeSegmentation(segmentation);
    
//    localMins.clear();
//    localMaxs.clear();
//    testWord = imread("/home/brian/intel_index/brian_handwriting/Fryers.png");
//    cvtColor( testWord, testWord, CV_RGB2GRAY );
//    threshold(testWord,testWord,thresh,255,1);
//    thinning(testWord,skel1);
//    graphemes = breakSegments(skel1,&localMaxs,&localMins,&centerLine);
//    cvtColor(skel1,out,CV_GRAY2RGB);
//    for (Point p : localMins)
//        out.at<Vec3b>(p) = Vec3b(0,255,0);
//    for (Point p : localMaxs)
//        out.at<Vec3b>(p) = Vec3b(0,0,255);
//    imwrite("minmax.png",out);
//    writeGraphemes(graphemes);
//    findBaselines(hasAscender("Fryers"),hasDescender("Fryers"),&top,&bot,localMins,localMaxs,centerLine,testWord);
//    cout << "Fryers top: " << top << " bot: " << bot << ", center line: " << centerLine << endl;
//    localMins.clear();
//    localMaxs.clear();
//    gs = extractGraphemes(skel1,&localMaxs,&localMins,&centerLine);
//    segmentation = learnCharacterSegmentation("Fryers",gs,localMaxs,localMins,centerLine,testWord);
//    writeSegmentation(segmentation);
    
//    localMins.clear();
//    localMaxs.clear();
//    testWord = imread("/home/brian/intel_index/brian_handwriting/abroad.png");
//    cvtColor( testWord, testWord, CV_RGB2GRAY );
//    threshold(testWord,testWord,thresh,255,1);
//    thinning(testWord,skel1);
//    graphemes = breakSegments(skel1,&localMaxs,&localMins,&centerLine);
//    cvtColor(skel1,out,CV_GRAY2RGB);
//    for (Point p : localMins)
//        out.at<Vec3b>(p) = Vec3b(0,255,0);
//    for (Point p : localMaxs)
//        out.at<Vec3b>(p) = Vec3b(0,0,255);
//    imwrite("minmax.png",out);
//    writeGraphemes(graphemes);
//    findBaselines(hasAscender("abroad"),hasDescender("abroad"),&top,&bot,localMins,localMaxs,centerLine,testWord);
//    cout << "Fryers top: " << top << " bot: " << bot << ", center line: " << centerLine << endl;
//    localMins.clear();
//    localMaxs.clear();
//    gs = extractGraphemes(skel1,&localMaxs,&localMins,&centerLine);
//    segmentation = learnCharacterSegmentation("abroad",gs,localMaxs,localMins,centerLine,testWord);
//    writeSegmentation(segmentation);
    
    
    cout << "Liang tests passed" << endl;
    return true;
}

bool Liang::hasAscender(const string& word)
{
    for (char c : word)
    {
        if (isAscender(c))
            return true;
    }
    return false;
}
bool Liang::hasDescender(const string& word)
{
    for (char c : word)
    {
        if (isDescender(c))
            return true;
    }
    return false;
}

bool Liang::isAscender(char c)
{
    if ('A'<=c && c<='Z') return true;
    if (c=='t') return true;
    if (c=='l') return true;
    if (c=='d') return true;
    if (c=='f') return true;
    if (c=='h') return true;
    if (c=='k') return true;
    if (c=='l') return true;
    if (c=='b') return true;
    return false;
}

bool Liang::isDescender(char c)
{
    if ('A'<=c && c<='Z') return false;
    if (c=='q') return true;
    if (c=='y') return true;
    if (c=='p') return true;
    if (c=='f') return true;//maybe? if cursive
    if (c=='g') return true;
    if (c=='j') return true;
    return false;
}

void Liang::writeGraphemes(const Mat& img)
{
	map<int,Vec3b> colors;
	Mat out;
	cvtColor(img,out,CV_GRAY2RGB);
	for (int x=0; x< img.cols; x++)
	{
		for (int y=0; y<img.rows; y++)
		{
			int v = img.at<unsigned char>(y,x);
			if (v>0)
			{
				if (colors.find(v) == colors.end())
				{
					colors[v] = Vec3b(rand()%256,rand()%256,rand()%256);
				}
				out.at<Vec3b>(y,x)=colors[v];
			}
		}
	}
	imwrite("debug.png",out);
}

void Liang::writeSegmentation(const vector<list<const Grapheme*> >& segmentation)
{
    Mat out = segmentation[0].front()->img()->clone();
    cvtColor(out,out,CV_GRAY2RGB);
    for (const list<const Grapheme*> &gs : segmentation)
    {
        Vec3b color(rand()%256,rand()%256,rand()%256);
        for (const Grapheme* g : gs)
        {
            for (int x=0; x<segmentation[0].front()->img()->cols; x++ )
                for (int y=0; y<segmentation[0].front()->img()->rows; y++ )
                {
                    if (segmentation[0].front()->img()->at<unsigned char>(y,x)==g->imgId())
                        out.at<Vec3b>(y,x)=color;
                }
        }
    }
    imwrite("segmentation.png",out);
}
