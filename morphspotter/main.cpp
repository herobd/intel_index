#include "MorphSpotter.h"
#include "gwdataset.h"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "dataset.h"

#include <tclap/CmdLine.h>
#include <map>

using namespace std;
using namespace cv;



int main(int argc, char** argv)
{
    //VL_PRINT ("Hello world!") ;
    
    
    
    try {  

	TCLAP::CmdLine cmd("MorphSpotter tester", ' ', "0.1");
	TCLAP::SwitchArg evalF("e","eval","Evaluate against dataset", cmd, false);
	TCLAP::SwitchArg evalSpotF("s","evalSubword","Evaluate subword spotting results", cmd, false);
	TCLAP::ValueArg<int> imageArg("i","image","show visual result", false,-1,"int");
	cmd.add( imageArg );
	TCLAP::ValueArg<int> image2Arg("c","image2","compare images", false,-1,"int");
	cmd.add( image2Arg );
	TCLAP::ValueArg<string> testFileArg("8","testfile","testing file: *.gtp for 'docimagefile lx ty rx by gt', *.txt for 'imagefile gt'", false,"test/queries_test.gtp","string");
	cmd.add( testFileArg );
	TCLAP::ValueArg<string> exemplarFileArg("7","exemplars","exemplar file: *.gtp for 'docimagefile lx ty rx by gt', *.txt for 'imagefile gt'", false,"test/exemplars.txt","string");
	cmd.add( exemplarFileArg );
	TCLAP::ValueArg<string> imageDirArg("d","images","directory containing images", false,"/home/brian/intel_index/brian_handwriting/EmbeddedAtt_Almazan/datasets/GW/images/","string");
	cmd.add( imageDirArg );
	TCLAP::ValueArg<string> exemplarDirArg("x","exemplarsDir","directory containing exemplar images", false,"/home/brian/intel_index/data/gw_20p_wannot/bigrams_clean_deslant/","string");
	cmd.add( exemplarDirArg );
	cmd.parse( argc, argv );

	 
	
	if ( evalF.getValue() )
	{
	    MorphSpotter spotter;
	    GWDataset test(testFileArg.getValue(),imageDirArg.getValue());
            //spotter.setTraining_dataset(&train);
	    
	    
            spotter.eval(&test);
	}
	if ( evalSpotF.getValue() )
	{
	    MorphSpotter spotter;
	    GWDataset test(testFileArg.getValue(),imageDirArg.getValue());
	    GWDataset exemplars(exemplarFileArg.getValue(),exemplarDirArg.getValue());
            //spotter.setTraining_dataset(&train);
	    
	    //spotter.evalSubword(&exemplars, &test);
	}
	
	if ( imageArg.getValue()>=0 )
	{
	    MorphSpotter spotter;
	    GWDataset test(testFileArg.getValue(),imageDirArg.getValue());
	    
	    int ex=imageArg.getValue();
	    
	    if ( image2Arg.getValue()>=0 )
            {
                int ex2= image2Arg.getValue();
                double score = spotter.score(test.image(ex),test.image(ex2));
                cout <<"score: "<<score<<endl;
            }
            else
            {
                /*cout <<"test word "<<test.labels()[ex]<<endl;
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
                waitKey();*/
            }
	}


	} catch (TCLAP::ArgException &e)  // catch any exceptions
	{ std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; }
}
