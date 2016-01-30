#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>

#include <assert.h>
#include <math.h>
#include <sys/time.h>

#include <mex.h>
#include <matrix.h>

#include <mcheck.h>
#include <assert.h>

int sort(const void *x, const void *y) {
    if (*(int*)x > *(int*)y) return 1;
    else if (*(int*)x < *(int*)y) return -1;
    else return 0;
}

struct rank_ele {
	int rank;
	float score;
};

int sort_with_scores(const void *x, const void *y) {
    if (((struct rank_ele*)x)->rank > ((struct rank_ele*)y)->rank) return 1;
    else if (((struct rank_ele*)x)->rank < ((struct rank_ele*)y)->rank) return -1;
    else return 0;
}

struct classList {
	int* classes;
	int length;
};



bool contains (struct classList* l, int class){
    //mexPrintf("entered contains\n");
	for (int i=0; i<l->length; i++)
		if (l->classes[i]==class) return true;
	return false;
}

void mexFunction (int nlhs, mxArray *plhs[],
        int nrhs, const mxArray*prhs[]) {
    
    mtrace ();
    
    //mexPrintf("\tEntered computeStats_c_subword\n");
    
    /* Input parameters */
    /* [0]: Similarity matrix
     * [1]: queriesClasses
     * [2]: datasetClasses
     * [3]: NRelevantsPerQuery, number of relevants for each query.
     * [4]: queriesIdx (can be -1s)
     */
    /* Output parameters */
    /* [0]: p@1 array
     * [1]: map array
     * [2]: idx of the best match for each query 
     */
            
    int Nqueries, Ndataset;
    
    int *queriesCls;
    //int *datasetCls;
    struct classList* datasetCls;
    int *NRelevantsPerQuery;
    int *queriesIdx;        
    float *S;    
          
    
    /* Read Data */
    S =  (float*)mxGetData(prhs[0]);
    Nqueries = (int) mxGetN(prhs[0]);
    Ndataset = (int) mxGetM(prhs[0]);
    queriesCls =  (int*)mxGetData(prhs[1]);    
    //datasetCls =  (int*)mxGetData(prhs[2]);
    NRelevantsPerQuery = (int*)mxGetData(prhs[3]);
    queriesIdx =  (int*)mxGetData(prhs[4]);
    float thresholdAtPrec =  (float)mxGetScalar(prhs[5]);
    
	mwSize total_num_of_cells;
	mwIndex index;
	const mxArray *cell_element_ptr;
  
    //mexPrintf("\tStarting cell analysis\n");
	total_num_of_cells = mxGetNumberOfElements(prhs[2]);
	datasetCls = malloc(total_num_of_cells*sizeof(struct classList));

	/* Each cell mxArray contains m-by-n cells; Each of these cells
	is an mxArray. */ 
	for (index=0; index<total_num_of_cells; index++)  {
		
		cell_element_ptr = mxGetCell(prhs[2], index);
		if (cell_element_ptr == NULL) {
			mexPrintf("\tEmpty Cell\n");
		} else {
			
			//get_characteristics(cell_element_ptr);
			//analyze_class(cell_element_ptr);
			const mwSize  *dims;
			//mwSize number_of_dimensions;
			//number_of_dimensions = mxGetNumberOfDimensions(cell_element_ptr);
  			dims = mxGetDimensions(cell_element_ptr);
  			datasetCls[index].length = (int) dims[1];
			datasetCls[index].classes = (int*)mxGetData(cell_element_ptr); 
		}
	}
    //mexPrintf("\tFinished cell analysis\n");
    
    /* Prepare output */
    mwSize dims[1];
    dims[0]= Nqueries;
    plhs[0] = mxCreateNumericArray(1, dims, mxSINGLE_CLASS, mxREAL);
    plhs[1] = mxCreateNumericArray(1, dims, mxSINGLE_CLASS, mxREAL);
    plhs[2] = mxCreateNumericArray(1, dims, mxINT32_CLASS, mxREAL);
    float *pP1 = (float*)mxGetData(plhs[0]);
    float *pMap = (float*)mxGetData(plhs[1]);
    int *bestIdx = (int*)mxGetData(plhs[2]);
    
    
    
    /* one query per row, scores in each column */
    /* for each query */
    //pragma omp parallel  for
    double sumThresh=0;
    for (int i=0; i < Nqueries; i++)
    {
    	//mexPrintf("on query %d\n",i);
        pMap[i]=0;
        pP1[i]=0;
        /* Create a private list of relevants. */
        int *rank = (int*)malloc(NRelevantsPerQuery[i]*sizeof(int));
        struct rank_ele *rank_with_scores = (struct rank_ele *)malloc(NRelevantsPerQuery[i]*sizeof(struct rank_ele));
        int Nrelevants = 0;
        /* Get its class */
        int qclass = queriesCls[i];
        /* For each element in the dataset */
        float bestS=-99999;
        int p1=0;
        
        //if (i==9) mexPrintf("start loop\n");
        
        for (int j=0; j < Ndataset; j++)
        {            
        	
        	//if (i==9) mexPrintf(" in loop %d\n",j);
        	
            float s = S[i*Ndataset + j];
            /* Precision at 1 part */
            if (queriesIdx[i]!=j && s > bestS)
            {
                bestS = s;
                p1 = contains(&datasetCls[j],qclass);
                bestIdx[i] = j+1; /* Matlab style */
            }
            /* If it is from the same class and it is not the query idx, it is a relevant one. */
            /* Compute how many on the dataset get a better score and how many get an equal one, excluding itself and the query.*/
            if (contains(&datasetCls[j],qclass) && queriesIdx[i]!=j)
            {
                int better=0;
                int equal = 0;
                
                //if (i==9) mexPrintf(" other loop start\n",j);
                for (int k=0; k < Ndataset; k++)
                {
                    if (k!=j && queriesIdx[i]!=k)
                    {
                        float s2 = S[i*Ndataset + k];
                        if (s2> s) better++;
                        else if (s2==s) equal++;
                    }
                }
                //if (i==9) mexPrintf(" other loop done\n",j);
                
                rank[Nrelevants]=better+floor(equal/2.0);
                rank_with_scores[Nrelevants].rank=better+floor(equal/2.0);
                rank_with_scores[Nrelevants].score=s;
                Nrelevants++;
            }
            
            //always
            
            int better=0;
            int equal = 0;
            
            for (int k=0; k < Ndataset; k++)
            {
                if (k!=j && queriesIdx[i]!=k)
                {
                    float s2 = S[i*Ndataset + k];
                    if (s2> s) better++;
                    else if (s2==s) equal++;
                }
            }
            //rank_with_scores[j].rank=better+floor(equal/2.0);
            //rank_with_scores[j].score=s;
            
            
        }
        
        /* Sort the ranked positions) */
        qsort(rank, Nrelevants, sizeof(int), sort);
        qsort(rank_with_scores, Nrelevants, sizeof(struct rank_ele), sort_with_scores);
        
        pP1[i] = p1;
        
        double scoreAtPrec;
        double distFromPrec=5;
        
        /* Get mAP and store it */
        for(int j=0;j<Nrelevants;j++){
            /* if rank[i] >=k it was not on the topk. Since they are sorted, that means bail out already */
            
            float prec_at_k =  ((float)(j+1))/(rank[j]+1);
            
            //mexPrintf("prec_at_k: %f\n", prec_at_k);
            pMap[i] += prec_at_k;            
            
            //float curMap = pMap[i]/(1+j);
            //mexPrintf("query[%d] prec_at_k=%f curMap=%f rank=%d score=%f\n",i,prec_at_k,curMap,rank[j],rank_with_scores[j].score);
            assert(rank_with_scores[j].rank == rank[j]);
            if (fabs(thresholdAtPrec-prec_at_k)<distFromPrec)
            {
                distFromPrec=fabs(thresholdAtPrec-prec_at_k);
                scoreAtPrec=rank_with_scores[j].score;
            }
        }
        if (distFromPrec==5)
            mexPrintf("error, dist isn't right\n");
        if (scoreAtPrec<0)
            mexPrintf("error, scoreAtPrec<0,  %f\n",scoreAtPrec);
        sumThresh += scoreAtPrec;
        
        pMap[i]/=Nrelevants;
        
        free(rank);
        free(rank_with_scores);
    }
    
    mexPrintf("before free\n");
    free(datasetCls);
    mexPrintf("computin' done!\n");
    if (sumThresh<0)
        mexPrintf("error, sumThresh<0\n");
    double thresh = sumThresh/Nqueries;
    if (thresh<0)
        mexPrintf("error, thresh<0\n");
    mexPrintf("avg threshold for prec %f = %f\n",thresholdAtPrec,thresh);
    plhs[3]=mxCreateDoubleScalar(thresh);
    
    muntrace ();
    return;
    
}


