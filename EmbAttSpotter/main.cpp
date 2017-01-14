#include "embattspotter.h"
#include "gwdataset.h"
#include "stridedataset.h"

#include <tclap/CmdLine.h>
#include <map>
#include <time.h>

int main(int argc, char** argv)
{
    //VL_PRINT ("Hello world!") ;
    
    
    
    try {  

	TCLAP::CmdLine cmd("EmbAttSpotter tester", ' ', "0.1");
	TCLAP::SwitchArg evalF("e","eval","Evaluate against dataset", cmd, false);
	TCLAP::SwitchArg evalSubF("s","evalSubwordSpotting","Evaluate subword spotting results", cmd, false);
	TCLAP::SwitchArg evalSubCombF("a","evalSubwordSpottingCombine","Evaluate subword spotting results when combining exemplars", cmd, false);
	TCLAP::ValueArg<int> primeSubwordArg("p","primeSubword","save the cca att for the given string length", false, -1,"int");
	cmd.add( primeSubwordArg );
	TCLAP::ValueArg<string> modelArg("l","location","load/save prefix", false,"model/evalGW","string");
	cmd.add( modelArg );
	TCLAP::SwitchArg testF("t","test","Run unit tests", cmd, false);
	TCLAP::ValueArg<int> imageArg("i","image","show visual result", false,-1,"int");
	cmd.add( imageArg );
	TCLAP::ValueArg<int> image2Arg("c","image2","compare images", false,-1,"int");
	cmd.add( image2Arg );
	TCLAP::ValueArg<string> compare1Arg("1","compare1","1st image for comparison", false,"","string");
	cmd.add( compare1Arg );
	TCLAP::ValueArg<string> compare2Arg("2","compare2","2nd image for comparison", false,"","string");
	cmd.add( compare2Arg );
	TCLAP::SwitchArg retrainAttReprTrF("6","attReprTr","Retrain attReprTr", cmd, false);
	TCLAP::SwitchArg retrainEmbeddingF("5","embedding","Retrain embedding", cmd, false);
	TCLAP::ValueArg<string> trainFileArg("9","trainfile","training file: *.gtp for 'docimagefile lx ty rx by gt', *.txt for 'imagefile gt'", false,"test/queries_train.gtp","string");
	cmd.add( trainFileArg );
	TCLAP::ValueArg<string> testFileArg("8","testfile","testing file: *.gtp for 'docimagefile lx ty rx by gt', *.txt for 'imagefile gt'", false,"test/queries_test.gtp","string");
	cmd.add( testFileArg );
	TCLAP::ValueArg<string> exemplarFileArg("7","exemplars","exemplar file: *.gtp for 'docimagefile lx ty rx by gt', *.txt for 'imagefile gt'", false,"test/exemplars.txt","string");
	cmd.add( exemplarFileArg );
	//TCLAP::ValueArg<string> exemplarLocArg("7","exemplar_locations","exemplar locations file", false,"test/exemplars.txt","string");
	//cmd.add( exemplarLocArg );
	TCLAP::ValueArg<string> imageDirArg("d","images","directory containing images", false,"/home/brian/intel_index/brian_handwriting/EmbeddedAtt_Almazan/datasets/GW/images/","string");
	cmd.add( imageDirArg );
	TCLAP::ValueArg<string> exemplarDirArg("x","exemplarsDir","directory containing exemplar images", false,"/home/brian/intel_index/data/gw_20p_wannot/bigrams_clean_deslant/","string");
	cmd.add( exemplarDirArg );
	TCLAP::ValueArg<string> fullFileArg("f","full","do a full sliding window subword spotting on this image", false,"","string");
	cmd.add( fullFileArg );
	TCLAP::ValueArg<float> hyarg("y","hybridalpha","hybrid alpha, 0 text only, 1 image only", false,-1,"float");
	cmd.add( hyarg );
	TCLAP::ValueArg<string> charSegCSVArg("3","charSeg","char seg csv file: 'word,pageId,x1,y1,x2,y2,char1start,char1end,char2start,...'", false,"","string");
        cmd.add( charSegCSVArg );
	cmd.parse( argc, argv );

	 
	if ( testF.getValue() )
	{
	    EmbAttSpotter spotter("testing",false,true,1);
	    
		spotter.test();
	}
	
	if ( evalF.getValue() )
	{
	    EmbAttSpotter spotter(modelArg.getValue());
	    GWDataset train(trainFileArg.getValue(),imageDirArg.getValue());
	    GWDataset test(testFileArg.getValue(),imageDirArg.getValue());
            spotter.setTraining_dataset(&train);
	    
	    if ( retrainAttReprTrF.getValue() )
	        spotter.attReprTr(true);
            if ( retrainEmbeddingF.getValue() )
	        spotter.embedding(true);
	    
		spotter.eval(&test);
	}

        if (primeSubwordArg.getValue()>=0)
        {
	    EmbAttSpotter spotter(modelArg.getValue());
	    GWDataset train(trainFileArg.getValue(),imageDirArg.getValue());
	    GWDataset test(testFileArg.getValue(),imageDirArg.getValue());
            spotter.setTraining_dataset(&train);
            spotter.setCorpus_dataset(&test);
            spotter.primeSubwordSpotting(primeSubwordArg.getValue());
        }

	if ( evalSubF.getValue() ||  evalSubCombF.getValue())
	{
	    EmbAttSpotter spotter(modelArg.getValue());
	    GWDataset train(trainFileArg.getValue(),imageDirArg.getValue());
            GWDataset test(testFileArg.getValue(),imageDirArg.getValue());

            spotter.setTraining_dataset(&train);
	    
	    if ( retrainAttReprTrF.getValue() )
	        spotter.attReprTr(true);
            if ( retrainEmbeddingF.getValue() )
	        spotter.embedding(true);
	    
            if (charSegCSVArg.getValue().length()>0)
            {
                vector< vector<int> > corpusXLetterStartBoundsRel;
                vector< vector<int> > corpusXLetterEndBoundsRel;
                ifstream in (charSegCSVArg.getValue());
                string line;
                //getline(in,line);//header
                while (getline(in,line))
                {
                    string s;
                    std::stringstream ss(line);
                    getline(ss,s,',');
                    if (s.compare(test.labels()[corpusXLetterStartBoundsRel.size()])!=0)
                        cout<<s<<" != "<<test.labels()[corpusXLetterStartBoundsRel.size()]<<endl;
                    assert(s.compare(test.labels()[corpusXLetterStartBoundsRel.size()])==0);
                    getline(ss,s,',');
                    getline(ss,s,',');//x1
                    int x1=stoi(s);
                    getline(ss,s,',');
                    getline(ss,s,',');//x2
                    getline(ss,s,',');
                    vector<int> lettersStartRel, lettersEndRel;

                    while (getline(ss,s,','))
                    {
                        lettersStartRel.push_back(stoi(s)-x1);
                        getline(ss,s,',');
                        lettersEndRel.push_back(stoi(s)-x1);
                        //getline(ss,s,',');//conf
                    }
                    corpusXLetterStartBoundsRel.push_back(lettersStartRel);
                    corpusXLetterEndBoundsRel.push_back(lettersEndRel);
                }
                in.close();
                
                spotter.evalSubwordSpottingWithCharBounds(&test, &corpusXLetterStartBoundsRel, &corpusXLetterEndBoundsRel, hyarg.getValue());

            }
            else
            {
                GWDataset exemplars(exemplarFileArg.getValue(),exemplarDirArg.getValue());
                if ( evalSubF.getValue() )
                    spotter.evalSubwordSpotting(&exemplars, &test, hyarg.getValue());
                else
                    spotter.evalSubwordSpottingCombine(&exemplars, &test, hyarg.getValue());
            }
	}

	
	if ( imageArg.getValue()>=0 )
	{
	    EmbAttSpotter spotter(modelArg.getValue());
	    GWDataset train(trainFileArg.getValue(),imageDirArg.getValue());
	    GWDataset test(testFileArg.getValue(),imageDirArg.getValue());
	    spotter.setTraining_dataset(&train);
	    spotter.setCorpus_dataset(&test);
	    
	    int ex=imageArg.getValue();
	    
	    if ( image2Arg.getValue()>=0 )
            {
                int ex2= image2Arg.getValue();
                double score = spotter.compare(test.image(ex),test.image(ex2));
                cout <<"score: "<<score<<endl;
            }
            else
            {
                cout <<"test word "<<test.labels()[ex]<<endl;
                imshow("test word",test.image(ex));
                
                vector<float> scores = spotter.spot(test.image(ex), "", 1);
                multimap<float, int> ranked;
                for (int i=0; i<scores.size(); i++)
                    ranked.emplace(scores[i],i);
                auto iter = ranked.end();
                for (int i=1; i<=5; i++)
                {
                    iter--;
                    cout<<"I rank "<<i<<" is "<<iter->second<<" with score "<<iter->first<<endl;
                    imshow("I "+to_string(i),test.image(iter->second));
                    
                }
                
                scores = spotter.spot(test.image(ex), test.labels()[ex], 0);
                ranked.clear();
                for (int i=0; i<scores.size(); i++)
                    ranked.emplace(scores[i],i);
                iter = ranked.end();
                for (int i=1; i<=5; i++)
                {
                    iter--;
                    cout<<"T rank "<<i<<" is "<<iter->second<<" with score "<<iter->first<<endl;
                    imshow("T "+to_string(i),test.image(iter->second));
                    
                }
                waitKey();
            }
	}

        if (compare1Arg.getValue().length()>0 && compare2Arg.getValue().length()>0)
        {
	    EmbAttSpotter spotter(modelArg.getValue());
            Mat im1 = imread(compare1Arg.getValue(),CV_LOAD_IMAGE_GRAYSCALE);
            Mat im2 = imread(compare2Arg.getValue(),CV_LOAD_IMAGE_GRAYSCALE);
            //im1 = GWDataset::preprocess(im1);
            //im2 = GWDataset::preprocess(im2);
            cout<<spotter.compare(im1,im2)<<endl;;
        }

        if (fullFileArg.getValue().length()>0)
        {
            assert(compare1Arg.getValue().length()>0);
	    EmbAttSpotter spotter(modelArg.getValue());
	    StrideDataset im(fullFileArg.getValue());
	    spotter.setCorpus_dataset_fullSub(&im);
	    Mat ex = imread(compare1Arg.getValue(),CV_LOAD_IMAGE_GRAYSCALE);
            clock_t start = clock();
            time_t start2 = time(0);
            vector< SubwordSpottingResult > res = spotter.subwordSpot_full(ex,"",1);
            clock_t end = clock();
            time_t end2 = time(0);
            double time = (double) (end-start) / CLOCKS_PER_SEC;
            double time2 = difftime(end2, start2) * 1000.0;
            cout<<"Took "<<time<<" secs."<<endl;
            cout<<"Took "<<time2<<" secs."<<endl;
            Mat img = imread(fullFileArg.getValue());
            Mat orig = img.clone();
            int top=100;
            float maxS = res[0].score;
            float minS = res[top].score;
            for (int i=0; i<top; i++)
            {
                float s = 1-((res[i].score-minS)/(maxS-minS));
                //cout <<res[i].score<<": "<<s<<endl;
                for (int x=res[i].startX; x<=res[i].endX; x++)
                    for (int y=res[i].imIdx*3; y<res[i].imIdx*3 +65; y++)
                    {
                        Vec3b& p = img.at<Vec3b>(y,x);
                        Vec3b o = orig.at<Vec3b>(y,x);
                        p[0] = min(p[0],(unsigned char)(o[0]*s));
                    }
            }
            imwrite("spotting.png",img);
            imshow("spotting",img);
            waitKey();


        }

	} catch (TCLAP::ArgException &e)  // catch any exceptions
	{ std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; }
}

