#include "liang.h"

Liang::Liang()
{
    trained=false;
}


double Liang::score(string query, const Mat &img)
{
    SynModel queryModel = getModel(query);
    if (img.cols < queryModel.minWidth() || img.cols > queryModel.maxWidth())
        return BAD_SCORE;
    
    vector<Grapheme> graphemes = extractGraphemes(img);
    vector<int> winningNodes(graphemes.size());
    for (int i=0; i<graphemes.size(); i++)
    {
        winningNodes[i] = MOG.getWinningNode(graphemes[i]);
    }
}

SynModel Liang::getModel(string query)
{
    
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
        
        for (const Grapheme* g : charGraphemes[c])
        {
            
            graphemeSpectrums[c][g->getMOG_class()]+=1;
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


vector<Grapheme> Liang::extractGraphemes(const Mat &img)
{
    Mat skel = extractSkeleton(img);
    Mat graphemes = breakSegments(skel);
    int count = repairLoops(graphemes);
    vector<Grapheme> ret = makeGraphemes(graphemes,count);
    return ret;
}

void Liang::learnCharacterSegmentation(string query, vector<Grapheme*> graphemes, const Mat &img, map<char, list<Grapheme*> >* charGraphemes, map<char, list<int> >* accumWidths, map<char, int>* charCounts)
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
    vector<list<Grapheme*> > characters(query.size()); 
    
    
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
        (*accumWidths)[query[c]].push_back(charWidth[c]);
        (*charGraphemes)[query[c]].somethin
        (*charCounts)[query[c]]++;
    }
    
    
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




int SynModel::minWidth()
{
    return avgWidth-2*stdDevWidth;
}

int SynModel::maxWidth()
{
    return avgWidth+2*stdDevWidth;
}
