#include "mog.h"

MOG::MOG(int demX, int demY, int spectrumSize)
{
    int dimensions[2]={demX,demY};
    som = new SOM(2, dimensions, spectrumSize, Node::EUCL); 
}

void MOG::train(const map<char, list<Grapheme*> >& graphemes)
{
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
    
    train_vectors.resize(trnrec.entries.size() - tstrec.entries.size());
    for (int i = 0; i < (int)trnrec.entries.size(); i++) {
            if (trnrec.entries[i] == 0)
                    continue;
            train_vectors[trn_index++] = trnrec.entries[i]->vec;
    }

    //get radius of half the SOM size////////////////////////////////////
    float R, R0 = som->R0();  
    float nrule, nrule0 = 0.9f;

    int epochs = _wtoi(argv[4]);
    float N = (float)epochs;
    int x = 0;  //0...N-1
    //training////////////////////////////////////////////////////////////////////////////////////
    while (epochs) {
            //exponential shrinking
            R = R0 * exp(-10.0f * (x * x) / (N * N));          //radius shrinks over time
            nrule = nrule0 * exp(-10.0f * (x * x) / (N * N));  //learning rule shrinks over time
            x++;

            som->train(&train_vectors, R, nrule);

            //wprintf(L"  epoch: %d    R: %.2f nrule: %g \n", (epochs-- - 1), R, nrule);

            if (kbhit() && _getwch() == 'q') //quit program ?
                    epochs = 0;
    }
    //////////////////////////////////////////////////////////////////////////////////////////////
}

void MOG::extractFeatures(Grapheme* g, float* features)
{
    features=new float(spectrumSize);
    
    features[0]=(g->maxX()-g->minX())/(g->maxY()-g->minY());
    
    int runLength=1;
    int directionHist[8]={0,0,0,0,0,0,0,0};
    int directionChanges=0;
    int angleHist[8]={0,0,0,0,0,0,0,0};
    int maxRun=0;
    int curRUn=1;
    vector<int> dcc;
    Point prev;
    
    Point current(0,g->maxY());
    for(int x=0; x<g->img()->cols; x++)
    {
        if (g->img()->at<unsigned char>(g->maxY(),x)==g->imgId())
        {
            current.x=x;
            break;
        }        
    }
    
    while()
    {
        // 3 2 1 neighbor id table
        // 4 x 0
        // 5 6 7
        int tableIndex=0;
        for (int tableIndex=0; tableIndex<8; tableIndex++)
        {
            
            int xDelta=1;
            if (tableIndex>2 && tableIndex<6) xDelta=-1;
            else if (tableIndex==2 || tableIndex==6) xDelta=0;
            int yDelta=0;
            if (tableIndex>0 && tableIndex<4) yDelta=-1;
            else if (tableIndex>4 && tableIndex<8) xDelta=1;
            
            int x = current.x+xDelta;
            int y = current.y+yDelta;
            
            if (!(x==prev.x&&y==prev.y) && g->img()->at<unsigned char>(y,x)==g->imgId())
            {
                
                break;
            }
        }
    }
    
    
}
