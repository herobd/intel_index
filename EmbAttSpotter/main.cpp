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
	TCLAP::ValueArg<string> modelArg("l","location","load/save prefix", false,"model/evalGW","string");
	cmd.add( modelArg );
	TCLAP::SwitchArg testF("t","test","Run unit tests", cmd, false);
	TCLAP::ValueArg<int> imageArg("i","image","show visual result", false,-1,"int");
	cmd.add( imageArg );
	TCLAP::SwitchArg retrainAttReprTrF("1","attReprTr","Retrain attReprTr", cmd, false);
	TCLAP::SwitchArg retrainEmbeddingF("2","embedding","Retrain embedding", cmd, false);
	cmd.parse( argc, argv );

	 
	if ( testF.getValue() )
	{
	    EmbAttSpotter spotter("testing",true,1);
	    
		spotter.test();
	}
	
	if ( evalF.getValue() )
	{
	    EmbAttSpotter spotter(modelArg.getValue());
	    GWDataset train("test/queries_train.gtp","/home/brian/intel_index/brian_handwriting/EmbeddedAtt_Almazan/datasets/GW/images/");
	    GWDataset test("test/queries_test.gtp","/home/brian/intel_index/brian_handwriting/EmbeddedAtt_Almazan/datasets/GW/images/");
	    spotter.setTraining_dataset(&train);
	    
	    if ( retrainAttReprTrF.getValue() )
	        spotter.attReprTr(true);
        if ( retrainEmbeddingF.getValue() )
	        spotter.embedding(true);
	    
		spotter.eval(&test);
	}
	
	if ( imageArg.getValue()>=0 )
	{
	    EmbAttSpotter spotter("model/evalGW");
	    GWDataset train("test/queries_train.gtp","/home/brian/intel_index/brian_handwriting/EmbeddedAtt_Almazan/datasets/GW/images/");
	    GWDataset test("test/queries_test.gtp","/home/brian/intel_index/brian_handwriting/EmbeddedAtt_Almazan/datasets/GW/images/");
	    spotter.setTraining_dataset(&train);
	    spotter.setCorpus_dataset(&test);
	    
	    int ex=imageArg.getValue();
	    
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
		    cout<<"I rank "<<i<<" with score "<<iter->first<<endl;
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
		    cout<<"T rank "<<i<<" with score "<<iter->first<<endl;
		    imshow("T "+to_string(i),test.image(iter->second));
		    
		}
		waitKey();
	}


	} catch (TCLAP::ArgException &e)  // catch any exceptions
	{ std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; }
}
