#include "mog.h"

MOG::MOG()
{
    trained==false;
}

void MOG::save(string filePath)
{
    assert(trained);
    som->save(wstring(filePath.begin(),filePath.end()).c_str());
}

void MOG::load(string filePath)
{
    som = new SOM(wstring(filePath.begin(),filePath.end()).c_str());
    trained=true;
}

int MOG::getWinningNode(const Grapheme* g)
{
    float* features;
    extractFeatures(g,features);
    const Node* bmu = som->classify(features);
    cout << "MOG classifying as " << (bmu->get_class()-1);
    return (bmu->get_class()-1);
}

double MOG::boxdist(int node1, int node2)
{
    return max(fabs(som->get_node(node1)->get_coords()[0]-som->get_node(node2)->get_coords()[0]),
            fabs(som->get_node(node1)->get_coords()[1]-som->get_node(node2)->get_coords()[1]));
}

void MOG::train(const map<char, list<Grapheme*> >& graphemes, int epochs, int demX, int demY)
{
    int dimensions[2]={demX,demY};
    som = new SOM(2, dimensions, SPECTURM_SIZE, Node::EUCL); 
    
    //array of vectors for training
    int trn_index = 0;
    vector<float *> train_vectors;
    
    for (const auto& pair : graphemes)
    {
        for (const Grapheme* g : pair.second)
        {
            extractFeatures(g,train_vectors[trn_index++]);
        }
    }
    

    //get radius of half the SOM size////////////////////////////////////
    float R, R0 = som->R0();  
    float nrule, nrule0 = 0.9f;

//    int epochs = _wtoi(argv[4]);
    float N = (float)epochs;
    int x = 0;  //0...N-1
    //training////////////////////////////////////////////////////////////////////////////////////
    while (epochs) {
            //exponential shrinking
            R = R0 * exp(-10.0f * (x * x) / (N * N));          //radius shrinks over time
            nrule = nrule0 * exp(-10.0f * (x * x) / (N * N));  //learning rule shrinks over time
            x++;

            som->train(&train_vectors, R, nrule);

            wprintf(L"  epoch: %d    R: %.2f nrule: %g \n", (epochs - 1), R, nrule);
            epochs--;
//            if (kbhit() && _getwch() == 'q') //quit program ?
//                    epochs = 0;
    }
    //////////////////////////////////////////////////////////////////////////////////////////////
    trained=true;
}

void MOG::extractFeatures( const Grapheme* g, float* features)
{
    features=new float(SPECTURM_SIZE);
    
    features[0]=(g->maxX()-g->minX())/(g->maxY()-g->minY());
    
    int runLength=1;
    int directionHist[8]={0,0,0,0,0,0,0,0};
    int directionChanges=0;
    int angleHist[8]={0,0,0,0,0,0,0,0};
    int maxRun=0;
    int maxRunDir;
    int curRun=1;
    bool loop=false;
    vector<int> dcc;
    
    Mat graphemeMap = *(g->img())(Range(g->minY(),g->maxY()+1), Range(g->minX(),g->maxX()+1)).clone();
    
    Point current(0,g->maxY());
    for(int x=0; x<graphemeMap.cols; x++)
    {
        if (graphemeMap.at<unsigned char>(graphemeMap.rows-1,x)==g->imgId())
        {
            current.x=x;
            break;
        }        
    }
    
    int prevDirection=-1;
    bool done = false;
    while(!done)
    {
        done=true;
        // 3 2 1 neighbor id table
        // 4 x 0
        // 5 6 7
        
        for (int direction=0; direction<8; direction++)
        {
            
            
            int xDelta=1;
            if (direction>2 && direction<6) xDelta=-1;
            else if (direction==2 || direction==6) xDelta=0;
            int yDelta=0;
            if (direction>0 && direction<4) yDelta=-1;
            else if (direction>4 && direction<8) xDelta=1;
            
            int x = current.x+xDelta;
            int y = current.y+yDelta;
            
            if (graphemeMap.at<unsigned char>(y,x)==g->imgId())
            {
                done = false;
                runLength++;
                directionHist[direction]++;
                if (prevDirection!=-1)
                {
                    if (prevDirection!=direction)
                    {
                        directionChanges++;
                        if (curRun > maxRun) 
                        {
                            maxRun=curRun;
                            maxRunDir=PrevDirection;
                        }
                        curRun=1;
                    }
                    else
                        curRun++;
                    
                    angleHist[mod(direction-prevDirection,8)]++;
                }
                
                dcc.push_back(direction);
                graphemeMap.at<unsigned char>(y,x) += 100;
                prevDirection=direction;
                break;
            }
            else if (graphemeMap.at<unsigned char>(y,x)==g->imgId()+100)
            {
                loop=true;
                
            }
            
            
        }
    }
    if (curRun > maxRun) 
    {
        maxRunDir=prevDirection;
    }
    
    //Enclosed shape indicator
    features[1]=loop?7:0;
    //Run length
    features[2]=runLength;
    //Main directions
    vector<int> directionRank={0,1,2,3,4,5,6,7};
    sort(directionRank.begin(),directionRank.end(),[&directionHist](const int& l, const int& r){return directionHist[l]>directionHist[r];});
    for (int i=0; i<4; i++)
        features[3+i]=directionRank[i];
    //Direction counts
    for (int i=0; i<8; i++)
        features[7+i]=directionHist[i];
    //Change of directions
    features[15]=directionChanges;
    //Main angles
    vector<int> angleRank={0,1,2,3,4,5,6,7};
    sort(angleRank.begin(),angleRank.end(),[&directionHist](const int& l, const int& r){return angleHist[l]>angleHist[r];});
    for (int i=0; i<4; i++)
        features[16+i]=angleRank[i];
    //Angle counts
    for (int i=0; i<8; i++)
        features[20+i]=angleHist[i];
    //Re-sampled DCC
    for (int i=0; i<12; i++)
    {
        float in_orig = i*((dcc.size()-1)/11.0); //Using a linear interpolation over the values (in folded space).
        if (!(dcc[floor(in_orig)]==0&&dcc[ceil(in_orig)]==7) && !(dcc[floor(in_orig)]==7&&dcc[ceil(in_orig)]==0))
            features[28+i]=(1.0-(in_orig-floor(in_orig)))*dcc[floor(in_orig)] + (in_orig-floor(in_orig))*dcc[ceil(in_orig)];
        else if (dcc[floor(in_orig)]==0)
            features[28+i]=(1.0-(in_orig-floor(in_orig)))*8.0 + (in_orig-floor(in_orig))*dcc[ceil(in_orig)];
        else
            features[28+i]=(1.0-(in_orig-floor(in_orig)))*dcc[floor(in_orig)] + (in_orig-floor(in_orig))*8.0;
    }
    //Max run direction
    features[40]=maxRunDir;
    
}
