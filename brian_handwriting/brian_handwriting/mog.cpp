#include "mog.h"

MOG::MOG()
{
    trained==false;
}

void MOG::save(string filePath)
{
    assert(trained);
    som->save(filePath.c_str());
}

void MOG::load(string filePath)
{
    som = new SOM(filePath.c_str());
    trained=true;
}

int MOG::getWinningNode(const Grapheme* g)
{
    assert(trained);
    float* features = new float[SPECTURM_SIZE];
    extractFeatures(g,features);
    const Node* bmu = som->classify(features);
    //cout << "MOG classifying as " << (bmu->get_class()-1);
    return (bmu->get_class()-1);
}

double MOG::boxdist(int node1, int node2)
{
    return max(fabs(som->get_node(node1)->get_coords()[0]-som->get_node(node2)->get_coords()[0]),
            fabs(som->get_node(node1)->get_coords()[1]-som->get_node(node2)->get_coords()[1]));
}

void MOG::train(const map<char, list<const Grapheme*> >& graphemes, int epochs, int demX, int demY)
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
            train_vectors.push_back(new float[SPECTURM_SIZE]);
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
    
    
    features[WH_RATIO]=(1.0+g->maxX()-g->minX())/(1.0+g->maxY()-g->minY());
    
    int runLength=1;
    int directionHist[8]={0,0,0,0,0,0,0,0};
    int directionChanges=0;
    int angleHist[8]={0,0,0,0,0,0,0,0};
    int maxRun=0;
    int maxRunDir=0;
    int curRun=1;
    bool loop=false;
    vector<int> dcc;
    
    Mat graphemeMap = (*(g->img()))(Range(g->minY(),g->maxY()+1), Range(g->minX(),g->maxX()+1)).clone();
    
    Point current(0,graphemeMap.rows-1);
    for(int x=0; x<graphemeMap.cols; x++)
    {
        if (graphemeMap.at<unsigned char>(graphemeMap.rows-1,x)==g->imgId())
        {
            current.x=x;
            graphemeMap.at<unsigned char>(current) += 100;
            break;
        }        
    }
    
    int prevDirection=-1;
    int firstDirection;
    bool done = false;
    while(!done)
    {
        done=true;
        // 3 2 1 neighbor id table
        // 4 x 0
        // 5 6 7
        
        for (int direction=0; direction<8; direction = direction!=6?direction+2:1)
        {
            
            
//            int xDelta=1;
//            if (direction>2 && direction<6) xDelta=-1;
//            else if (direction==2 || direction==6) xDelta=0;
//            int yDelta=0;
//            if (direction>0 && direction<4) yDelta=-1;
//            else if (direction>4 && direction<8) xDelta=1;
            
            int x = current.x+xDelta(direction);
            int y = current.y+yDelta(direction);
            if (x<0 || y<0 || x>=graphemeMap.cols || y>=graphemeMap.rows)
                continue;
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
                            maxRunDir=prevDirection;
                        }
                        curRun=1;
                    }
                    else
                        curRun++;
                    
                    angleHist[mod(direction-prevDirection,8)]++;
                }
                else
                    firstDirection=direction;
                
                
                dcc.push_back(direction); // or? // dcc.push_back(mod(direction-prevDirection,8))
                graphemeMap.at<unsigned char>(y,x) += 100;
                prevDirection=direction;
                current.x=x;
                current.y=y;
                break;
            }
            else if (mod(abs(((prevDirection+4)%8)-direction),8)>1 && graphemeMap.at<unsigned char>(y,x)==g->imgId()+100)
            {
                assert(!loop && done);
                loop=true;
                
                //Record the closing move
                directionHist[direction]++;
                if (prevDirection!=direction)
                {
                    directionChanges++;
                    if (curRun > maxRun) 
                    {
                        maxRun=curRun;
                        maxRunDir=prevDirection;
                    }
                }
                angleHist[mod(direction-prevDirection,8)]++;
                angleHist[mod(firstDirection-direction,8)]++;
                dcc.push_back(direction);
                break;
            }
            
            
        }
//        assert(!(loop&&!done));
    }
    if (curRun > maxRun) 
    {
        maxRunDir=prevDirection;
    }
    
    //Enclosed shape indicator
    features[ENCLOSED]=loop?7:0;
    //Run length
    features[RUN_LENGTH]=runLength;
    //Main directions
    vector<int> directionRank={0,1,2,3,4,5,6,7};
    sort(directionRank.begin(),directionRank.end(),[&directionHist](const int& l, const int& r){return directionHist[l]>directionHist[r];});
    for (int i=0; i<NUM_MAIN_DIRECTIONS; i++)
        features[MAIN_DIRECTIONS+i]=directionRank[i];
    //Direction counts
    for (int i=0; i<NUM_DIRECTIONS; i++)
        features[DIRECTION_COUNTS+i]=directionHist[i];
    //Change of directions
    features[DIRECTION_CHANGES]=directionChanges;
    //Main angles
    vector<int> angleRank={0,1,2,3,4,5,6,7};
    sort(angleRank.begin(),angleRank.end(),[&angleHist](const int& l, const int& r){return angleHist[l]>angleHist[r];});
    for (int i=0; i<NUM_MAIN_ANGLES; i++)
        features[MAIN_ANGLES+i]=angleRank[i];
    //Angle counts
    for (int i=0; i<NUM_ANGLES; i++)
        features[ANGLE_COUNTS+i]=angleHist[i];
    //Re-sampled DCC
    if (dcc.size()>0)
    {
        for (int i=0; i<DCC_RESAMPLE_SIZE; i++)
        {
            float in_orig = i*((dcc.size()-1.0)/(DCC_RESAMPLE_SIZE-1.0)); //Using a linear interpolation over the values (in folded space).
            if (!(dcc[floor(in_orig)]==0&&dcc[ceil(in_orig)]==7) && !(dcc[floor(in_orig)]==7&&dcc[ceil(in_orig)]==0))
                features[RESAMPLED_DCC+i]=(1.0-(in_orig-floor(in_orig)))*dcc[floor(in_orig)] + (in_orig-floor(in_orig))*dcc[ceil(in_orig)];
            else if (dcc[floor(in_orig)]==0)
                features[RESAMPLED_DCC+i]=(1.0-(in_orig-floor(in_orig)))*8.0 + (in_orig-floor(in_orig))*dcc[ceil(in_orig)];
            else
                features[RESAMPLED_DCC+i]=(1.0-(in_orig-floor(in_orig)))*dcc[floor(in_orig)] + (in_orig-floor(in_orig))*8.0;
        }
    }
    else
    {
        for (int i=0; i<DCC_RESAMPLE_SIZE; i++)
            features[RESAMPLED_DCC+i]=0;
        
    }
    //Max run direction
    features[MAX_RUN_DIRECTION]=maxRunDir;
    
}





void MOG::unittest()
{
    Mat graphemes = (Mat_<unsigned char>(6,6) <<   0,  0,  0, 5, 5, 0,
                                                   0,  1,  1, 0, 0, 5,
                                                   1,  0,  1, 0, 5, 0,
                                                   0,  1,  1, 0, 0, 0,
                                                   0,  0,  0, 0, 0, 0,
                                                   0,  0,  0, 0, 0, 0);
    Grapheme g0(graphemes,5);
    float* features=new float[SPECTURM_SIZE];
    extractFeatures(&g0,features);
    assert(features[WH_RATIO]==1);
    assert(features[ENCLOSED]==0);
    assert(features[RUN_LENGTH]==4);
    assert(features[DIRECTION_COUNTS+0]==0);
    assert(features[DIRECTION_COUNTS+1]==1);
    assert(features[DIRECTION_COUNTS+2]==0);
    assert(features[DIRECTION_COUNTS+3]==1);
    assert(features[DIRECTION_COUNTS+4]==1);
    assert(features[DIRECTION_COUNTS+5]==0);
    assert(features[DIRECTION_COUNTS+6]==0);
    assert(features[DIRECTION_COUNTS+7]==0);
    assert(features[DIRECTION_CHANGES]==2);
    assert(features[ANGLE_COUNTS+0]==0);
    assert(features[ANGLE_COUNTS+1]==1);
    assert(features[ANGLE_COUNTS+2]==1);
    assert(features[ANGLE_COUNTS+3]==0);
    assert(features[ANGLE_COUNTS+4]==0);
    assert(features[ANGLE_COUNTS+5]==0);
    assert(features[ANGLE_COUNTS+6]==0);
    assert(features[ANGLE_COUNTS+7]==0);
    assert(1<=features[RESAMPLED_DCC+0] && features[RESAMPLED_DCC+0]<3);
    assert(1<=features[RESAMPLED_DCC+1] && features[RESAMPLED_DCC+1]<3);
    assert(1<=features[RESAMPLED_DCC+2] && features[RESAMPLED_DCC+2]<3);
    assert(1<features[RESAMPLED_DCC+3] && features[RESAMPLED_DCC+3]<4);
    assert(1<features[RESAMPLED_DCC+4] && features[RESAMPLED_DCC+4]<4);
    assert(1<features[RESAMPLED_DCC+5] && features[RESAMPLED_DCC+5]<4);
    assert(3<features[RESAMPLED_DCC+6] && features[RESAMPLED_DCC+6]<=4);
    assert(3<features[RESAMPLED_DCC+7] && features[RESAMPLED_DCC+7]<=4);
    assert(3<features[RESAMPLED_DCC+8] && features[RESAMPLED_DCC+8]<=4);
    assert(features[MAX_RUN_DIRECTION]==1);
    
    Grapheme g1(graphemes,1);
    features;
    extractFeatures(&g1,features);
    assert(features[WH_RATIO]==1);
    assert(features[ENCLOSED]==7);
    assert(features[RUN_LENGTH]==6);
    assert(features[DIRECTION_COUNTS+0]==1);
    assert(features[DIRECTION_COUNTS+1]==0);
    assert(features[DIRECTION_COUNTS+2]==2);
    assert(features[DIRECTION_COUNTS+3]==0);
    assert(features[DIRECTION_COUNTS+4]==1);
    assert(features[DIRECTION_COUNTS+5]==1);
    assert(features[DIRECTION_COUNTS+6]==0);
    assert(features[DIRECTION_COUNTS+7]==1);
    assert(features[DIRECTION_CHANGES]==4);
    assert(features[ANGLE_COUNTS+0]==1);
    assert(features[ANGLE_COUNTS+1]==2);
    assert(features[ANGLE_COUNTS+2]==3);
    assert(features[ANGLE_COUNTS+3]==0);
    assert(features[ANGLE_COUNTS+4]==0);
    assert(features[ANGLE_COUNTS+5]==0);
    assert(features[ANGLE_COUNTS+6]==0);
    assert(features[ANGLE_COUNTS+7]==0);
    assert(0<=features[RESAMPLED_DCC+0] && features[RESAMPLED_DCC+0]<2);
    assert(0<=features[RESAMPLED_DCC+1] && features[RESAMPLED_DCC+1]<2);
    assert(0<features[RESAMPLED_DCC+2] && features[RESAMPLED_DCC+2]<=2);
    assert(0<features[RESAMPLED_DCC+3] && features[RESAMPLED_DCC+3]<=2);
    assert(2<=features[RESAMPLED_DCC+4] && features[RESAMPLED_DCC+4]<4);
    assert(2<=features[RESAMPLED_DCC+5] && features[RESAMPLED_DCC+5]<4);
    assert(2<features[RESAMPLED_DCC+6] && features[RESAMPLED_DCC+6]<5);
    assert(2<features[RESAMPLED_DCC+7] && features[RESAMPLED_DCC+6]<5);
    assert(4<features[RESAMPLED_DCC+8] && features[RESAMPLED_DCC+8]<7);
    assert(4<features[RESAMPLED_DCC+9] && features[RESAMPLED_DCC+9]<7);
    assert(5<features[RESAMPLED_DCC+10] && features[RESAMPLED_DCC+10]<=7);
    assert(5<features[RESAMPLED_DCC+11] && features[RESAMPLED_DCC+11]<=7);
    assert(features[MAX_RUN_DIRECTION]==2);
    
    Mat graphemes2 = (Mat_<unsigned char>(6,6) <<   0,  0,  0, 0, 5, 0,
                                                    0,  1,  1, 0, 0, 5,
                                                    1,  0,  1, 0, 5, 0,
                                                    1,  1,  1, 5, 0, 0,
                                                    0,  0,  0, 0, 0, 0,
                                                    0,  0,  0, 0, 0, 0);
    Grapheme g2(graphemes2,1);
    Grapheme g3(graphemes2,5);
    
    Mat graphemes3 = (Mat_<unsigned char>(6,6) <<   0,  0,  5, 5, 5, 0,
                                                    0,  0,  0, 0, 0, 5,
                                                    1,  1,  1, 0, 5, 0,
                                                    1,  0,  1, 0, 0, 0,
                                                    1,  1,  0, 0, 0, 0,
                                                    0,  0,  0, 0, 0, 0);
    Grapheme g4(graphemes3,1);
    Grapheme g5(graphemes3,5);
    
    Mat graphemes4 = (Mat_<unsigned char>(6,6) <<   1,  0,  0, 0, 0, 0,
                                                    1,  0,  0, 5, 5, 0,
                                                    1,  0,  5, 0, 0, 0,
                                                    1,  0,  5, 0, 0, 0,
                                                    1,  0,  0, 5, 5, 0,
                                                    0,  0,  0, 0, 0, 0);
    Grapheme g6(graphemes4,1);
    Grapheme g7(graphemes4,5);
    
    Mat graphemes5 = (Mat_<unsigned char>(6,6) <<   1,  0,  0, 0, 5, 0,
                                                    1,  0,  0, 5, 0, 0,
                                                    1,  0,  5, 0, 0, 0,
                                                    0,  1,  5, 0, 0, 0,
                                                    0,  1,  0, 5, 0, 0,
                                                    0,  0,  0, 0, 0, 0);
    Grapheme g8(graphemes5,1);
    Grapheme g9(graphemes5,5);
    
    Mat graphemes6 = (Mat_<unsigned char>(6,6) <<   0,  1,  0, 0, 0, 0,
                                                    0,  1,  5, 5, 0, 0,
                                                    1,  0,  5, 0, 0, 0,
                                                    1,  0,  5, 0, 0, 0,
                                                    1,  0,  0, 5, 5, 5,
                                                    1,  0,  0, 0, 0, 0);
    Grapheme g10(graphemes6,1);
    Grapheme g11(graphemes6,5);
    
    map<char, list<const Grapheme*> > examples;
    list<const Grapheme*> gs = {&g0,&g1,&g2,&g3,&g4,&g5,&g6,&g7,&g8,&g9,&g10,&g11};
    examples['a']=gs;
    train(examples,50,2,2);
    assert(getWinningNode(&g0)==getWinningNode(&g3));
    assert(getWinningNode(&g1)==getWinningNode(&g2));
    assert(getWinningNode(&g6)==getWinningNode(&g10));
    assert(getWinningNode(&g7)==getWinningNode(&g11));
    
    Mat graphemes7 = (Mat_<unsigned char>(6,6) <<   1,  0,  0, 0, 0, 0,
                                                    1,  0,  0, 5, 5, 0,
                                                    1,  0,  5, 0, 0, 0,
                                                    0,  1,  5, 0, 0, 0,
                                                    1,  0,  5, 5, 5, 0,
                                                    0,  0,  0, 0, 0, 0);
    Grapheme g12(graphemes7,1);
    Grapheme g13(graphemes7,5);
    assert(getWinningNode(&g6)==getWinningNode(&g12));
    assert(getWinningNode(&g7)==getWinningNode(&g13));
    
    Mat graphemes8 = (Mat_<unsigned char>(6,6) <<   0,  0,  0, 0, 0, 0,
                                                    0,  0,  0, 5, 5, 0,
                                                    1,  1,  0, 0, 0, 5,
                                                    1,  0,  1, 0, 0, 5,
                                                    0,  1,  0, 0, 5, 0,
                                                    0,  0,  0, 0, 0, 0);
    Grapheme g14(graphemes8,1);
    Grapheme g15(graphemes8,5);
    assert(getWinningNode(&g0)==getWinningNode(&g15));
    assert(getWinningNode(&g1)==getWinningNode(&g14));
    
    cout << "passed MOG tests" << endl;
}
void MOG::writeGraphemes(const Mat& img)
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
