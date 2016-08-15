#include "embattspotter.h"
#include "gwdataset.h"

#include <tclap/CmdLine.h>
#include <map>


int main(int argc, char** argv)
{
    //VL_PRINT ("Hello world!") ;
    
    
    
    try {  

	TCLAP::CmdLine cmd("EmbAttSpotter tester", ' ', "0.1");
	TCLAP::SwitchArg evalF("e","eval","Evaluate against dataset", cmd, false);
	TCLAP::SwitchArg evalSpotF("s","evalSpotting","Evaluate spotting results", cmd, false);
	TCLAP::ValueArg<string> modelArg("l","location","load/save prefix", false,"model/evalGW","string");
	cmd.add( modelArg );
	TCLAP::SwitchArg testF("t","test","Run unit tests", cmd, false);
	TCLAP::ValueArg<int> imageArg("i","image","show visual result", false,-1,"int");
	cmd.add( imageArg );
	TCLAP::ValueArg<int> image2Arg("c","image2","compare images", false,-1,"int");
	cmd.add( image2Arg );
	TCLAP::SwitchArg retrainAttReprTrF("1","attReprTr","Retrain attReprTr", cmd, false);
	TCLAP::SwitchArg retrainEmbeddingF("2","embedding","Retrain embedding", cmd, false);
	TCLAP::ValueArg<string> trainFileArg("9","trainfile","training file: *.gtp for 'docimagefile lx ty rx by gt', *.txt for 'imagefile gt'", false,"test/queries_train.gtp","string");
	cmd.add( trainFileArg );
	TCLAP::ValueArg<string> testFileArg("8","testfile","testing file: *.gtp for 'docimagefile lx ty rx by gt', *.txt for 'imagefile gt'", false,"test/queries_test.gtp","string");
	cmd.add( testFileArg );
	TCLAP::ValueArg<string> imageDirArg("d","images","directory containing images", false,"/home/brian/intel_index/brian_handwriting/EmbeddedAtt_Almazan/datasets/GW/images/","string");
	cmd.add( imageDirArg );
	cmd.parse( argc, argv );

	 
	if ( testF.getValue() )
	{
	    EmbAttSpotter spotter("testing",true,1);
	    
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
	if ( evalSpotF.getValue() )
	{
	    EmbAttSpotter spotter(modelArg.getValue());
	    GWDataset train(trainFileArg.getValue(),imageDirArg.getValue());
	    GWDataset test(testFileArg.getValue(),imageDirArg.getValue());
            spotter.setTraining_dataset(&train);
	    
	    if ( retrainAttReprTrF.getValue() )
	        spotter.attReprTr(true);
            if ( retrainEmbeddingF.getValue() )
	        spotter.embedding(true);
	    
		spotter.evalSpotting(&test);
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


	} catch (TCLAP::ArgException &e)  // catch any exceptions
	{ std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; }
}
