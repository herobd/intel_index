double MorphSpotter::score(Mat im1, Mat im2)
{
    DImage dim1, dim2;
    cvToD(im1,dim1);
    cvToD(im2,dim2);
    int tval = getThreshold(im1);
    DThresholder::threshImage_(dim1,dim1, tval);
    tval = getThreshold(im2);
    DThresholder::threshImage_(dim2,dim2, tval);
    return mobj.getWordMorphCost(dim1,dim2);
}
double MorphSpotter::scoreFast(Mat im1, Mat im2)
{
    DImage dim1, dim2;
    cvToD(im1,dim1);
    cvToD(im2,dim2);
    int tval = getThreshold(im1);
    DThresholder::threshImage_(dim1,dim1, tval);
    tval = getThreshold(im2);
    DThresholder::threshImage_(dim2,dim2, tval);
    return mobj.getWordMorphCostFast(dim1,dim2);
}
double MorphSpotter::score_preThreshed(Mat im1, Mat im2)
{
    DImage dim1, dim2;
    cvToD(im1,dim1);
    cvToD(im2,dim2);
    return mobj.getWordMorphCost(dim1,dim2);
}
double MorphSpotter::scoreFast_preThreshed(Mat im1, Mat im2)
{
    DImage dim1, dim2;
    cvToD(im1,dim1);
    cvToD(im2,dim2);
    return mobj.getWordMorphCostFast(dim1,dim2);
}

Mat MorphSpotter::binarize(const Mat& orig)
{


void MorphSpotter::eval(const Dataset* data)
{
    float map=0;
    int queryCount=0;
    vector<Mat> binarized(data->size());
    #pragma omp parallel  for
    for (int inst=0; inst<data->size(); inst++)
    {
        binarized[inst]=binarize(data.image(inst));
    }
    
    #pragma omp parallel  for
    for (int inst=0; inst<data->size(); inst++)
    {
        int other=0;
        string text = data->labels()[inst];
        for (int inst2=0; inst2<data->size(); inst2++)
        {
            if (inst!=inst2 && text.compare(data->labels()[inst2])==0)
            {
                other++;
            }
        }
        if (other==0)
            continue;

        int *rank = new int[other];//(int*)malloc(NRelevantsPerQuery[i]*sizeof(int));
        int Nrelevants = 0;
        float ap=0;

        float bestS=-99999;
        vector<float> scores(data->size());// = spot(data->image(inst),text,hy); //scores
        for (int j=0; j < data->size(); j++)
        {
            scores[j] = score_preThreshed(binarized[inst],binarized[j]);
        }

        for (int j=0; j < data->size(); j++)
        {
            float s = scores[j];
            //cout <<"score for "<<j<<" is "<<s<<". It is ["<<data->labels()[j]<<"], we are looking for ["<<text<<"]"<<endl;
            /* Precision at 1 part */
            /*if (inst!=j && s > bestS)
            {
                bestS = s;
                p1 = text==data->labels()[j];
                //bestIdx[inst] = j;
            }*/
            /* If it is from the same class and it is not the query idx, it is a relevant one. */
            /* Compute how many on the dataset get a better score and how many get an equal one, excluding itself and the query.*/
            if (text.compare(data->labels()[j])==0 && inst!=j)
            {
                int better=0;
                int equal = 0;

                for (int k=0; k < data->size(); k++)
                {
                    if (k!=j && inst!=k)
                    {
                        float s2 = scores[k];
                        if (s2> s) better++;
                        else if (s2==s) equal++;
                    }
                }


                rank[Nrelevants]=better+floor(equal/2.0);
                Nrelevants++;
            }

        }
        qsort(rank, Nrelevants, sizeof(int), sort_xxx);

        //pP1[i] = p1;

        /* Get mAP and store it */
        for(int j=0;j<Nrelevants;j++){
            /* if rank[i] >=k it was not on the topk. Since they are sorted, that means bail out already */

            float prec_at_k =  ((float)(j+1))/(rank[j]+1);
            //mexPrintf("prec_at_k: %f\n", prec_at_k);
            ap += prec_at_k;
        }
        ap/=Nrelevants;

        #pragma omp critical (storeMAP)
        {
            queryCount++;
            map+=ap;
        }

        delete[] rank;
    }

    cout<<"map: "<<(map/queryCount)<<endl; 
}

//DMorphInk mobj;
void* word_morphing_thread_func(void *params){
  WORDWARP_THREAD_PARMS *pparms;
  int numTrain;
  DMorphInk mobj;
  int testWordIdx;

  pparms = (WORDWARP_THREAD_PARMS*)params;
  numTrain = pparms->numTrain;
  testWordIdx = pparms->testWordIdx;

  //now compare to all training values
  int numLexReductionWordsSkipped;
  numLexReductionWordsSkipped = 0;
  for(int tr=(pparms->threadNum); tr < numTrain; tr+=(pparms->numThreads)){
    double morphCost=0.;
    double DPcost=0.;

#if DO_FAST_PASS_FIRST
    if(pparms->fFastPass)
      morphCost =
        mobj.getWordMorphCostFast(*(pparms->pimgTest),
                                  pparms->rgTrainingImages[tr],
                                  pparms->bandWidthDP,/*15 */
                                  0./*nonDiagonalCostDP*/,
                                  pparms->meshSpacingStatic,
                                  pparms->numRefinesStatic,
                                  pparms->meshDiv,
                                  pparms->lengthPenalty);
    else
#else
      morphCost = mobj.getWordMorphCost(*(pparms->pimgTest),
                                        pparms->rgTrainingImages[tr],
                                        pparms->bandWidthDP,/*15 bandWidthDP*/
                                        0./*nonDiagonalCostDP*/,
                                        pparms->meshSpacingStatic,
                                        pparms->numRefinesStatic,
                                        pparms->meshDiv,
                                        pparms->lengthPenalty);
#endif
    DPcost = mobj.warpCostDP;
    pparms->rgCostsMorph[testWordIdx*(long)numTrain+tr] = morphCost;
    pparms->rgCostsDP[testWordIdx*(long)numTrain+tr] = DPcost;
    pparms->rgNumLexReductionWordsSkipped[pparms->threadNum] =
      numLexReductionWordsSkipped;
  }
  return NULL;
}

