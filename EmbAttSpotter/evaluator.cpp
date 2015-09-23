#include "evaluator.h"

Evaluator::Evaluator()
{
}

void evaluate(EmbAttSpotter &spotter)
{
    
    
    for (double alpha=0; alpha<=1.0; alpha+=.1)
    {
        for (int i=0; i<attReprTe_ccas.size(); i++)
        {
            Mat attReprTe_cca = attReprTe_ccas[i];
            Mat attRepr_hybrid = attReprTe_cca*alpha + phocsTe_cca*(1-alpha);
            ?? = spotter.eval_dp_asymm(attRepr_hybrid,attReprTe_ccas,DATA.wordClsTe,DATA.labelsTe);
        }
    }
    
}


