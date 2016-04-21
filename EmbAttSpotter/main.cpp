#include "embattspotter.h"
#include "gwdataset.h"

#include <tclap/CmdLine.h>


int main(int argc, char** argv)
{
    //VL_PRINT ("Hello world!") ;
    
    
    
    try {  

	TCLAP::CmdLine cmd("EmbAttSpotter tester", ' ', "0.1");
	TCLAP::SwitchArg evalF("e","eval","Evaluate against dataset", cmd, false);
	TCLAP::SwitchArg testF("t","test","Run unit tests", cmd, false);
	TCLAP::SwitchArg retrainAttReprTrF("1","attReprTr","Retrain attReprTr", cmd, false);
	TCLAP::SwitchArg retrainEmbeddingF("2","embedding","Retrain embedding", cmd, false);
	cmd.parse( argc, argv );

	 
	if ( testF.getValue() )
	{
	    EmbAttSpotter spotter("testing",true,true);
	    
		spotter.test();
	}
	
	if ( evalF.getValue() )
	{
	    EmbAttSpotter spotter("model/evalGW");
	    GWDataset train("test/queries_train.gtp","/home/brian/intel_index/brian_handwriting/EmbeddedAtt_Almazan/datasets/GW/images/");
	    GWDataset test("test/queries_test.gtp","/home/brian/intel_index/brian_handwriting/EmbeddedAtt_Almazan/datasets/GW/images/");
	    spotter.setTraining_dataset(&train);
	    
	    if ( retrainAttReprTrF.getValue() )
	        spotter.attReprTr(true);
        if ( retrainEmbeddingF.getValue() )
	        spotter.embedding(true);
	    
		spotter.eval(&test);
	}


	} catch (TCLAP::ArgException &e)  // catch any exceptions
	{ std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; }
}
