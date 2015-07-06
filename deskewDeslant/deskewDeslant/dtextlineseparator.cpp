#include "dtextlineseparator.h"
#include "dprofile.h"
#include "dtcm.h"
#include "dthresholder.h"
#include "drect.h"
#include "dslantangle.h"
#include <string.h>



///assumes that the image is BINARY with bg=255
int DTextlineSeparator::estimateAvgHeight(DImage &imgBinary,
					  int ROIx0, int ROIy0,
					  int ROIx1, int ROIy1,
					  char *stDebugBaseName){
  DImage imgROI;
  int w, h;
  D_uint8 *pu8;
  DProfile prof;

  if(-1 == ROIx1)
    ROIx1 = imgBinary.width()-1;
  if(-1 == ROIy1)
    ROIy1 = imgBinary.height()-1;

  imgBinary.copy_(imgROI,ROIx0,ROIy0,ROIx1-ROIx0+1,ROIy1-ROIy0+1);

  char stTmp[1024];
  sprintf(stTmp, "%s_roi.pgm",stDebugBaseName);
  imgROI.save(stTmp);
  
  w = imgROI.width();
  h = imgROI.height();
  pu8 = imgROI.dataPointer_u8();
  for(int y=0, idx=0; y < h; ++y){
    for(int x=0; x < w; ++x, ++idx){
      if((pu8[idx] > 0) && (pu8[idx] < 255)){
	fprintf(stderr, "DTextlineSeparator::estimateAvgHeight() requires "
		"BINARY image!\n");
	exit(1);
      }
    }
  }
  prof.getImageVerticalProfile(imgROI,true);
  DImage imgTmp;
  imgTmp = prof.toDImage(100,true);
  sprintf(stTmp,"%s_prof.pgm",stDebugBaseName);
  imgTmp.save(stTmp);
  prof.smoothAvg(2);
  imgTmp = prof.toDImage(100,true);
  sprintf(stTmp,"%s_prof_smooth.pgm",stDebugBaseName);
  imgTmp.save(stTmp);



  prof.getVertAvgRunlengthProfile(imgROI,0x00,false);
  imgTmp = prof.toDImage(100,true);
  sprintf(stTmp,"%s_prof_rle.pgm",stDebugBaseName);
  imgTmp.save(stTmp);
  prof.smoothAvg(2);
  imgTmp = prof.toDImage(100,true);
  sprintf(stTmp,"%s_prof_rle_smooth.pgm",stDebugBaseName);
  imgTmp.save(stTmp);


  // find a radiusX that gives a good histogram from the TCM
  // (we want the TCM to give responses of about
  printf("  *creating TCM histograms...\n");fflush(stdout);

  int rgHists[40][256];
  memset(rgHists,0,sizeof(int)*40*256);

  for(int rx = 10; rx < 400; rx +=10){
    DImage imgTCM;
    D_uint8 *p8;
    int max = 0;
    int ry;
    ry = rx/6;
    if(ry < 1)
      ry = 1;
    DTCM::getImageTCM_(imgTCM, imgROI, rx,ry, false,NULL);
    p8 = imgTCM.dataPointer_u8();
    for(int y = 0, idx=0; y < h; ++y){
      for(int x = 0; x < w; ++x,++idx){
	rgHists[rx/10][p8[idx]] += 1;
      }
    }
    rgHists[rx/10][0] = 0;
    max = 0;
    for(int i=0;i<256;++i)
      if(rgHists[rx/10][i] > max)
	max =rgHists[rx/10][i];
    for(int i=0;i<256;++i){//scale from 0 to 255
      if (max!=0)
        rgHists[rx/10][i] = rgHists[rx/10][i] * 255 / max;
    }
  }
  //now save the histograms as an image
  DImage imgTCMhists;
  imgTCMhists.create(256,40,DImage::DImage_u8);
  D_uint8 *p8;
  p8 = imgTCMhists.dataPointer_u8();
  for(int y=0, idx=0; y < 40; ++y){
    for(int x=0; x < 256; ++x, ++idx){
      p8[idx] = (D_uint8)(rgHists[y][x]);
    }
  }
  sprintf(stTmp, "%s_tcmhist.pgm",stDebugBaseName);
  imgTCMhists.save(stTmp);
  printf("  *done creating TCM histograms...\n");
  

  int radiusX, radiusY;
  radiusX = imgROI.width() / 20;
  if(radiusX < 10)
    radiusX = 10;
  if(radiusX > 200)
    radiusX = 200;
  radiusY = radiusX / 5;
  //  if(radiusY < 2)
    radiusY = 2;
  printf("  TCM radiusX=%d radiusY=%d\n", radiusX,radiusY);
  DTCM::getImageTCM_(imgTmp, imgROI, radiusX,radiusY, false,stDebugBaseName);
  //  DTCM::getImageTCM_(imgTmp, imgROI, 1,1, false);

  // double *rgProf;
  // rgProf = prof.dataPointer();
  // for(int i=100; i < 500; ++i){
  //   if(rgProf[i] > 0.)
  //     printf("[%d]=%f ",i, rgProf[i]);
  // }
  // printf("\n");
  return 0;
}




///assumes that the image is BINARY with bg=255
/** Takes profiles of numStrips vertical strips (plus numStrips-1
    overlapping strips) and uses them to estimate the avg textline
    height **/
int DTextlineSeparator::estimateAvgHeight2(DImage &imgBinary,
					   int numStrips,
					   char *stDebugBaseName){
  int w, h;
  D_uint8 *pu8;
  DProfile prof;
  DProfile *rgProfs;// profiles of overlapping strips of image
  DProfile *rgProfsRL;//avg white RL profile
  DProfile *rgProfsSmear;// profiles of overlapping strips of image after smear
  char stTmp[1024];
  int *rgPeakThresh;
  int *rgPeakThreshRL;
  double *rgPeakLineOffs;
  

  rgProfs = new DProfile[numStrips*2-1];
  D_CHECKPTR(rgProfs);
  rgProfsRL = new DProfile[numStrips*2-1];
  D_CHECKPTR(rgProfsRL);
  rgProfsSmear = new DProfile[numStrips*2-1];
  D_CHECKPTR(rgProfsSmear);
  rgPeakThresh = new int[numStrips*2-1];
  D_CHECKPTR(rgPeakThresh);
  rgPeakThreshRL = new int[numStrips*2-1];
  D_CHECKPTR(rgPeakThreshRL);
  rgPeakLineOffs = new double[numStrips*2-1];
  D_CHECKPTR(rgPeakLineOffs);

  w = imgBinary.width();
  h = imgBinary.height();
  pu8 = imgBinary.dataPointer_u8();
  for(int y=0, idx=0; y < h; ++y){
    for(int x=0; x < w; ++x, ++idx){
      if((pu8[idx] > 0) && (pu8[idx] < 255)){
  	fprintf(stderr, "DTextlineSeparator::estimateAvgHeight() requires "
  		"BINARY image with values of 0 or 255!\n");
  	exit(1);
      }
    }
  }

  DImage imgStrip;
  int stripW, stripLeft;
  DProfile profWeightedStrokeDist;
  int **rgBlackSpacingHist;

  rgBlackSpacingHist = new int*[numStrips*2-1];
  D_CHECKPTR(rgBlackSpacingHist);
  rgBlackSpacingHist[0]=new int[200*(numStrips*2-1)];
  D_CHECKPTR(rgBlackSpacingHist[0]);
  memset(rgBlackSpacingHist[0],0,sizeof(int)*200*(numStrips*2-1));
  for(int i=1; i < (numStrips*2-1); ++i){
    rgBlackSpacingHist[i] = &(rgBlackSpacingHist[i-1][200]);//only 2-199 are valid spacings
  }
  
  int **rgPeakYs;
  int *rgNumPeaks;
  int **rgValleyYs;
  int *rgNumValleys;

  rgPeakYs = new int*[numStrips*2-1];
  D_CHECKPTR(rgPeakYs);
  rgPeakYs[0] = new int[(numStrips*2-1)*h];
  D_CHECKPTR(rgPeakYs[0]);
  rgValleyYs = new int*[numStrips*2-1];
  D_CHECKPTR(rgValleyYs);
  rgValleyYs[0] = new int[(numStrips*2-1)*h];
  D_CHECKPTR(rgValleyYs);
  for(int i = 1; i < (numStrips*2-1); ++i){
    rgPeakYs[i] = &(rgPeakYs[i-1][h]);
    rgValleyYs[i] = &(rgValleyYs[i-1][h]);
  }

  rgNumPeaks = new int[numStrips*2-1];
  D_CHECKPTR(rgNumPeaks);
  rgNumValleys = new int[numStrips*2-1];
  D_CHECKPTR(rgNumValleys);
  for(int i=0; i < (numStrips*2-1); ++i){
    rgNumPeaks[i] = 0;
    rgNumValleys[i] = 0;
  }

  stripW = (w + numStrips-1) / numStrips;
  printf("w=%d h=%d stripW=%d\n",w,h,stripW);
  for(int i=0; i < numStrips*2-1; ++i){
    stripLeft = i * stripW/2;
    if(i == numStrips*2-2){//last strip may have slightly different width
      stripW = w - stripLeft - 1;
    }
    imgBinary.copy_(imgStrip, stripLeft, 0, stripW, h);
    rgProfs[i].getImageVerticalProfile(imgStrip,false);
    rgProfs[i].smoothAvg(2);
    rgProfsRL[i].getVertAvgRunlengthProfile(imgStrip,0xff,true);
    rgProfsRL[i].smoothAvg(2);

    double *pdbl;
    pdbl = rgProfs[i].dataPointer();
    for(int j=0; j < h; ++j)
      pdbl[j] /= 255; // now the profile is number of white pixels (was GS prof)

    unsigned int profMax;
    profMax = (unsigned int)rgProfs[i].max();

    //use original image to create histogram of horizontal foreground spacing
    //(distance from black pixel to next black pixel) weighted by profile value
    //inverse (number of black pixels instead of white pixels)
    for(int y=2; y < (h-2); ++y){//ignore 2 on each end (smoothing boundaries)
      int lastBlackX;
      int runlength;
      int x;
      int weight;

      x = stripLeft-199;
      if(x < 0)
	x=0;
      lastBlackX = x;
      runlength = 0;
      for( ; (x<stripLeft+stripW+199) && (x < w); ++x){
	if(pu8[y*w+x] == 0){//black
	  runlength = x - lastBlackX;
	  if((runlength >= 2) && (runlength < 200)){
	    weight = (int)profMax - (int)pdbl[y];//inverse of profile value at y
	    rgBlackSpacingHist[0/*i*/][runlength] += weight;
	  }
	  lastBlackX=x;
	}
      }
    }

    //now multiply the values by the avg runlength
    double *pdblRL;
    pdblRL = rgProfsRL[i].dataPointer();
    // for(int j=0; j < h; ++j)
    //   pdbl[j] *= pdblRL[j];


    //now get a histogram of the profile values and use otsu to determine
    //a threshold between peaks and valleys
    unsigned int *rgProfHist;
    double peakThresh;
    rgProfHist = (unsigned int*)calloc(profMax+1,sizeof(unsigned int));
    D_CHECKPTR(rgProfHist);
    for(int j=0; j < h; ++j)
      ++(rgProfHist[(int)(pdbl[j])]);
    peakThresh = DThresholder::getOtsuThreshVal(rgProfHist, profMax+1);
    rgPeakLineOffs[i] = peakThresh / (double)stripW;//now a fraction of stripW


    //choose a threshold between peaks and valleys as the thresh that maximizes
    //how many peaks there are that are between 2 and 200 pixels high
    

    // unsigned int max,min;
    // max = 0;
    // min = rgProfHist[0];
    // for(int j=0; j < stripW; ++j){
    //   if(rgProfHist[j] > max)
    // 	max = rgProfHist[j];
    //   if(rgProfHist[j] < min)
    // 	min = rgProfHist[j];
    // }
    // rgPeakLineOffs[i] = peakThresh / (double)max;
    // printf("peakThresh=%lf  rgPeakLineOffs=%f\n",
    // 	   peakThresh,rgPeakLineOffs[i]);
    free(rgProfHist);
    rgPeakThresh[i] = (int)peakThresh;
  }

  //to get the spacing estimate, get the max, then find the next position
  //that is less than 1/3 of the max.  Use that as the estimate to determine
  //scale
  int spacingMax;
  int spacingEstimate;
  
  spacingMax = 2;
  for(int j=3; j<200; ++j){
    if(rgBlackSpacingHist[0][j] > rgBlackSpacingHist[0][spacingMax])
      spacingMax = j;
  }
  spacingEstimate = spacingMax;
  for(int j=spacingMax+1; j < 200; ++j){
    if(rgBlackSpacingHist[0][j] < (rgBlackSpacingHist[0][spacingMax] / 3)){
      spacingEstimate = j;
      break;
    }
  }
  printf(" spacing estimate =        *** %d pixels\n",spacingEstimate);


  // now smear the image based on the spacing estimate, then take new profiles
  DImage imgSmear;
  D_uint8 *psmear;
  imgSmear = imgBinary;
  psmear = imgSmear.dataPointer_u8();
  
  for(int y=0; y < h; ++y){
    int lastBlackX;
    int runlength;
    
    lastBlackX = w;
    for(int x=0; x < w; ++x){
      if(pu8[y*w+x] == 0){//black
	runlength = x - lastBlackX;
	if((runlength < 2*spacingEstimate) && (runlength >0)){
	  // fill in the white since last black pixel with black
	  for(int xp=lastBlackX+1; xp < x; ++xp){
	    psmear[(y*w+xp)] = 128;
	  }
	}
	lastBlackX = x;
      }
    }
  }
  sprintf(stTmp,"%s_smear.ppm",stDebugBaseName);
  imgSmear.save(stTmp);

  // now recalculate all of the profiles
  stripW = (w + numStrips-1) / numStrips;
  int *rgSmearThresh;
  rgSmearThresh = new int[numStrips*2-1];
  D_CHECKPTR(rgSmearThresh);
  for(int i=0; i < numStrips*2-1; ++i){
    double *pdbl;
    unsigned int profMax;
    stripLeft = i * stripW/2;
    if(i == numStrips*2-2){//last strip may have slightly different width
      stripW = w - stripLeft - 1;
    }
    // imgSmear.copy_(imgStrip, stripLeft, 0, stripW, h);
    imgBinary.copy_(imgStrip, stripLeft, 0, stripW, h);
    rgProfsSmear[i].getImageVerticalProfile(imgStrip,false);


    // invert the profile so black is 255 and white is zero before smoothing
    pdbl = rgProfsSmear[i].dataPointer();
    profMax = (unsigned int)rgProfsSmear[i].max();
    for(int y=0; y < h; ++y)
      pdbl[y] = profMax - pdbl[y];

    rgProfsSmear[i].smoothAvg(spacingEstimate*2/3);
    profMax = (unsigned int)rgProfsSmear[i].max();//new max after smoothing





    // decide where peak/valleys in profile are
    {
      int prevSign = 0;
      double deriv;
      double *pdbl;
      int numZeros = 0;

      pdbl = rgProfsSmear[i].dataPointer();
      //use profile derivative and dist from last peak/valley
      //to decide where peaks and valleys are
      for(int y=1; y < (h-1); ++y){
	deriv = pdbl[y+1] - pdbl[y-1];
	if(deriv > 0.){//rising
	  if(prevSign <= 0){//valley
	    rgValleyYs[i][rgNumValleys[i]] = y-numZeros/2;//(middle of plateaus)
	    ++(rgNumValleys[i]);
	  }
	  prevSign = 1;
	  numZeros = 0;
	}
	else if(deriv < 0.){//falling
	  if(prevSign >= 0){//peak
	    rgPeakYs[i][rgNumPeaks[i]] = y-numZeros/2;//(middle of plateaus)
	    ++(rgNumPeaks[i]);
	  }
	  prevSign = -1;
	  numZeros = 0;
	}
	else{ // zero slope
	  ++numZeros;
	}
      }//end for(y=...
    }

    // combine peaks that are too close to each other
    {
      int numPeaksRemoved = 0;
      bool fRemoved;
      fRemoved = true;
      while(fRemoved && (rgNumPeaks[i]>1)){
	fRemoved = false;
	int deletePeak=0;
	int deleteValley=0;
	for(int j=1; j < rgNumPeaks[i]; ++j){
	  if(abs(rgPeakYs[i][j-1]-rgPeakYs[i][j])<spacingEstimate*2/3){//too close
	    if(pdbl[rgPeakYs[i][j]] > pdbl[rgPeakYs[i][j-1]]){
	      printf("  A remove peak %d at y=%d\n",j-1,(int)pdbl[rgPeakYs[i][j-1]]);
	      deletePeak = j-1;
	    }
	    else{
	      printf("  B remove peak %d at y=%d\n",j,(int)pdbl[rgPeakYs[i][j]]);
	      deletePeak = j;
	    }
	    deleteValley = -1;

	    if(rgNumValleys[i] > 0){
	      if(rgPeakYs[i][0] < rgValleyYs[i][0]){//peak was first
		deleteValley = deletePeak;
	      }
	      else{//valley was first
		deleteValley = deletePeak+1;
	      }
	    }

	    //delete the peak
	    for(int k=deletePeak+1; k < rgNumPeaks[i]; ++k){
	      rgPeakYs[i][k-1] = rgPeakYs[i][k];
	    }
	    --(rgNumPeaks[i]);
	    //delete the valley (if in range)
	    if((deleteValley>=0) && (deleteValley < rgNumValleys[i])){
	      for(int k=deleteValley+1; k < rgNumValleys[i]; ++k){
		rgValleyYs[i][k-1] = rgValleyYs[i][k];
	      }
	    }
	    fRemoved = true;
	    ++numPeaksRemoved;
	    break;
	  }//if(abs(...
	}//for(int j=...
      }//while(fRemoved)
    }



#if 0
    rgSmearThresh[i] = 0;
    //choose threshold that maximizes the number of peaks
    int bestNumPeaks;
    int bestNumPeaksThresh;
    int peaksThresh;
    int numPeaks;
    //    double peaksProfMax = 0.;
    // for(int y=spacingEstimate/2-1; y < h-(spacingEstimate/2-1); ++y)
    //   if(pdbl[y] > peaksProfMax)
    // 	peaksProfMax = pdbl[y];
    bestNumPeaksThresh = 0;
    bestNumPeaks = 0;
    numPeaks = 1;
    for(int peaksThresh = 0; peaksThresh <= profMax; ++peaksThresh){
      numPeaks = 0;
      int fLead = -1;
      for(int y=0; y < h;++y){
	if(fLead>=0){
	  if((pdbl[y] <= peaksThresh)){
	    if((y-fLead) >= spacingEstimate/2)
	      ++numPeaks;
	    fLead = -1;
	  }
	}
	else{
	  if(pdbl[y] > peaksThresh)
	    fLead = y;
	}
      }
      if(numPeaks >= bestNumPeaks){
	bestNumPeaks = numPeaks;
	bestNumPeaksThresh = peaksThresh;
      }
    }
    rgSmearThresh[i] = bestNumPeaksThresh;
#endif


  }

  // now get x-height estimate using profiles (or black runlengths of smears)


  //debug: save an image with all of the profiles
  {
    DImage imgProfsAll;
    DImage imgProfsRLAll;
    imgProfsAll.create(w,h,DImage::DImage_u8);
    stripW = (w + numStrips-1) / numStrips;
    for(int i=0; i < numStrips*2-1; ++i){
      DImage imgTmp;
      imgTmp = rgProfs[i].toDImage(stripW/2,true);
      imgProfsAll.pasteFromImage(i*stripW/2,0,imgTmp,0,0,stripW/2,h);
    }
    imgProfsAll = imgProfsAll.convertedImgType(DImage::DImage_RGB);
    for(int i=0; i < numStrips*2-1; ++i){
      int peakLineOffs;
      // peakLineOffs = stripW/2 * rgPeakThresh[i] / rgProfs[i].max();
      peakLineOffs = (int)(rgPeakLineOffs[i]*stripW/2);
      // printf(" rgPeakLineOffs[%d]=%lf peakLineOffs=%d\n",i,rgPeakLineOffs[i],
      // 	     peakLineOffs);
      imgProfsAll.drawLine(i*stripW/2 + peakLineOffs+1, 0,
			   i*stripW/2 + peakLineOffs+1, h-1, 255-i,0,0);
      imgProfsAll.drawLine(i*stripW/2 + peakLineOffs, 0,
			   i*stripW/2 + peakLineOffs, h-1, 255-i,0,0);
      imgProfsAll.drawLine(i*stripW/2, 0,
			   i*stripW/2, h-1, 0, 255-i,0);
    }

    sprintf(stTmp,"%s_allprofs.pgm",stDebugBaseName);
    imgProfsAll.save(stTmp);
  }

  //debug: save an image with all of the smeared profiles
  {
    DImage imgProfsAll;
    DImage imgProfsRLAll;
    imgProfsAll.create(w,h,DImage::DImage_u8);
    stripW = (w + numStrips-1) / numStrips;
    for(int i=0; i < numStrips*2-1; ++i){
      DImage imgTmp;
      imgTmp = rgProfsSmear[i].toDImage(stripW/2,true);
      imgProfsAll.pasteFromImage(i*stripW/2,0,imgTmp,0,0,stripW/2,h);
    }
    imgProfsAll = imgProfsAll.convertedImgType(DImage::DImage_RGB);
    for(int i=0; i < numStrips*2-1; ++i){

      for(int j=0; j < rgNumPeaks[i]; ++j){
	int ypos;
	ypos = rgPeakYs[i][j];
	imgProfsAll.drawLine(i*stripW/2,ypos,(i+1)*stripW/2-1,ypos,255,0,0);
      }

      for(int j=0; j < rgNumValleys[i]; ++j){
	int ypos;
	ypos = rgValleyYs[i][j];
	imgProfsAll.drawLine(i*stripW/2,ypos,(i+1)*stripW/2-1,ypos,0,255,0);
      }



#if 0
      int prevSign = 0;
      int lastPeakY, lastValleyY, lastTurnY;
      double deriv;
      double *pdbl;

      pdbl = rgProfsSmear[i].dataPointer();
      lastPeakY = lastValleyY = lastTurnY = 0-spacingEstimate;
      //use profile derivative and dist from last peak/valley
      //to decide where peaks and valleys are
      for(int y=1; y < (h-1); ++y){
	deriv = pdbl[y+1] - pdbl[y-1];
	if(deriv > 0.){//rising
	  // imgProfsAll.setPixel(i*stripW/2,y,0,255,0);
	  if((prevSign <= 0) && ((y-lastTurnY)>spacingEstimate/2)){//valley
	    imgProfsAll.drawLine(i*stripW/2,y,(i+1)*stripW/2-1,y,0,255,0);
	    lastValleyY = lastTurnY = y;
	    prevSign = 1;
	  }
	}
	else if(deriv < 0.){//falling
	  // imgProfsAll.setPixel(i*stripW/2,y,255,0,0);
	  if(prevSign >= 0){
	    if(((y-lastTurnY)>spacingEstimate/2) ||
	       ((lastPeakY>=0)&&(pdbl[y] > pdbl[lastPeakY]))){//peak
	      if((lastPeakY>=0)&&(pdbl[y] > pdbl[lastPeakY])){//correct previous
		imgProfsAll.drawLine(i*stripW/2,lastPeakY,
				     (i+1)*stripW/2-1,lastPeakY,0,0,0);
	      }
	      imgProfsAll.drawLine(i*stripW/2,y,(i+1)*stripW/2-1,y,255,0,0);
	      lastPeakY = lastTurnY = y;
	      prevSign = -1;
	    }
	  }
	}
	else{ // zero slope (do nothing)
	  // imgProfsAll.setPixel(i*stripW/2,y,0,0,0);
	  // do nothing
	}

      }

      // int peakLineOffs;
      // peakLineOffs = rgSmearThresh[i] * stripW/2 / rgProfsSmear[i].max();
      // imgProfsAll.drawLine(i*stripW/2 + peakLineOffs+1, 0,
      // 			   i*stripW/2 + peakLineOffs+1, h-1, 255-i,0,0);
      // imgProfsAll.drawLine(i*stripW/2 + peakLineOffs, 0,
      // 			   i*stripW/2 + peakLineOffs, h-1, 255-i,0,0);
      // imgProfsAll.drawLine(i*stripW/2, 0,
      // 			   i*stripW/2, h-1, 0, 255-i,0);
#endif
    }
    sprintf(stTmp,"%s_allsmearprofs.pgm",stDebugBaseName);
    imgProfsAll.save(stTmp);
  }



  //debug: save an image with all of the RL profiles
  {
    DImage imgProfsAll;
    imgProfsAll.create(w,h,DImage::DImage_u8);
    stripW = (w + numStrips-1) / numStrips;
    for(int i=0; i < numStrips*2-1; ++i){
      DImage imgTmp;
      imgTmp = rgProfsRL[i].toDImage(stripW/2,true);
      imgProfsAll.pasteFromImage(i*stripW/2,0,imgTmp,0,0,stripW/2,h);
    }
    imgProfsAll = imgProfsAll.convertedImgType(DImage::DImage_RGB);

    sprintf(stTmp,"%s_allprofsRL.pgm",stDebugBaseName);
    imgProfsAll.save(stTmp);
  }

  //debug: save a gnuplot of the histograms of black spacing weighted by profile
  // the image has the histogram for each strip followed by the sum histogram
  // a value of -10 is placed at positions 0,1 of each histogram as a separator
  {
    DImage imgSpacingHists;
    FILE *fout;

    sprintf(stTmp,"%s_spacing_profs.dat",stDebugBaseName);
    fout = fopen(stTmp,"wb");
    if(!fout){
      fprintf(stderr, "couldn't open debug file '%s' for output\n",stTmp);
      exit(1);
    }
    for(int i=0; i < 1/*numStrips*2-1*/; ++i){
      for(int j=0; j < 200; ++j){
	int val;
	val = rgBlackSpacingHist[i][j];
	if(j<2)
	  val = -10;
	fprintf(fout,"%d\t%d\n",i*200+j, val);
      }
    }
    fclose(fout);
  }


  // now at the otsu x-position in the profile, get avg black runlength to
  // guess at peak (textline) height.
  // Do the same for white to guess at valley (spacing) height.
  // After getting it for each strip's profile, take the avg for the whole
  // page.  Use that to determine a smoothing value and a window size for the
  // transition count map (TCM). (maybe use median instead of avg?)

  delete [] rgPeakYs[0];
  delete [] rgPeakYs;
  delete [] rgNumPeaks;
  delete [] rgValleyYs[0];
  delete [] rgValleyYs;
  delete [] rgNumValleys;


  delete [] rgProfs;
  delete [] rgProfsRL;
  delete [] rgPeakThresh;
  delete [] rgPeakThreshRL;
  delete [] rgPeakLineOffs;
  delete rgBlackSpacingHist[0];
  delete [] rgBlackSpacingHist;
  //  exit(1);


  // prof.getImageVerticalProfile(imgROI,true);
  // DImage imgTmp;
  // imgTmp = prof.toDImage(100,true);
  // sprintf(stTmp,"%s_prof.pgm",stDebugBaseName);
  // imgTmp.save(stTmp);
  // prof.smoothAvg(2);
  // imgTmp = prof.toDImage(100,true);
  // sprintf(stTmp,"%s_prof_smooth.pgm",stDebugBaseName);
  // imgTmp.save(stTmp);



  // prof.getVertAvgRunlengthProfile(imgROI,0x00,false);
  // imgTmp = prof.toDImage(100,true);
  // sprintf(stTmp,"%s_prof_rle.pgm",stDebugBaseName);
  // imgTmp.save(stTmp);
  // prof.smoothAvg(2);
  // imgTmp = prof.toDImage(100,true);
  // sprintf(stTmp,"%s_prof_rle_smooth.pgm",stDebugBaseName);
  // imgTmp.save(stTmp);


  // // find a radiusX that gives a good histogram from the TCM
  // // (we want the TCM to give responses of about
  // printf("  *creating TCM histograms...\n");fflush(stdout);

  // int rgHists[40][256];
  // memset(rgHists,0,sizeof(int)*40*256);

  // for(int rx = 10; rx < 400; rx +=10){
  //   DImage imgTCM;
  //   D_uint8 *p8;
  //   int max = 0;
  //   int ry;
  //   ry = rx/6;
  //   if(ry < 1)
  //     ry = 1;
  //   DTCM::getImageTCM_(imgTCM, imgROI, rx,ry, false,NULL);
  //   p8 = imgTCM.dataPointer_u8();
  //   for(int y = 0, idx=0; y < h; ++y){
  //     for(int x = 0; x < w; ++x,++idx){
  // 	rgHists[rx/10][p8[idx]] += 1;
  //     }
  //   }
  //   rgHists[rx/10][0] = 0;
  //   max = 0;
  //   for(int i=0;i<256;++i)
  //     if(rgHists[rx/10][i] > max)
  // 	max =rgHists[rx/10][i];
  //   for(int i=0;i<256;++i){//scale from 0 to 255
  //     rgHists[rx/10][i] = rgHists[rx/10][i] * 255 / max;
  //   }
  // }
  // //now save the histograms as an image
  // DImage imgTCMhists;
  // imgTCMhists.create(256,40,DImage::DImage_u8);
  // D_uint8 *p8;
  // p8 = imgTCMhists.dataPointer_u8();
  // for(int y=0, idx=0; y < 40; ++y){
  //   for(int x=0; x < 256; ++x, ++idx){
  //     p8[idx] = (D_uint8)(rgHists[y][x]);
  //   }
  // }
  // sprintf(stTmp, "%s_tcmhist.pgm",stDebugBaseName);
  // imgTCMhists.save(stTmp);
  // printf("  *done creating TCM histograms...\n");
  

  // int radiusX, radiusY;
  // radiusX = imgROI.width() / 20;
  // if(radiusX < 10)
  //   radiusX = 10;
  // if(radiusX > 200)
  //   radiusX = 200;
  // radiusY = radiusX / 5;
  // //  if(radiusY < 2)
  //   radiusY = 2;
  // printf("  TCM radiusX=%d radiusY=%d\n", radiusX,radiusY);
  // DTCM::getImageTCM_(imgTmp, imgROI, radiusX,radiusY, false,stDebugBaseName);
  // //  DTCM::getImageTCM_(imgTmp, imgROI, 1,1, false);

  // // double *rgProf;
  // // rgProf = prof.dataPointer();
  // // for(int i=100; i < 500; ++i){
  // //   if(rgProf[i] > 0.)
  // //     printf("[%d]=%f ",i, rgProf[i]);
  // // }
  // // printf("\n");
  return 0;
}




///assumes that the image is BINARY with bg=255
/** Takes profiles of numStrips vertical strips (plus numStrips-1
    overlapping strips) and uses them to estimate the avg textline
    height **/
void DTextlineSeparator::getTextlineRects(DImage &img, int *numTextlines,
					  DRect **rgTextlineRects,
					  int *spacingEst,
					  char *stDebugBaseName){
  int w, h;
  D_uint8 *pu8;
  DProfile prof;
  DProfile profSmear;// profile of smeared image
  char stTmp[1024];
  
  w = img.width();
  h = img.height();
  pu8 = img.dataPointer_u8();
  for(int y=0, idx=0; y < h; ++y){
    for(int x=0; x < w; ++x, ++idx){
      if((pu8[idx] > 0) && (pu8[idx] < 255)){
  	fprintf(stderr, "DTextlineSeparator::getTextlineRects() requires "
  		"BINARY image with values of 0 or 255!\n");
  	exit(1);
      }
    }
  }

  DProfile profWeightedStrokeDist;
  int *rgBlackSpacingHist;

  rgBlackSpacingHist=new int[200];
  D_CHECKPTR(rgBlackSpacingHist);
  memset(rgBlackSpacingHist,0,sizeof(int)*200);
  
  int *rgPeakYs;
  int numPeaks;
  int *rgValleyYs;
  int numValleys;

  rgPeakYs = new int[h];
  D_CHECKPTR(rgPeakYs);
  rgValleyYs = new int[h];
  D_CHECKPTR(rgValleyYs);

  numPeaks = 0;
  numValleys = 0;

  {
    prof.getImageVerticalProfile(img,false);
    prof.smoothAvg(2);

    double *pdbl;
    pdbl = prof.dataPointer();
    for(int j=0; j < h; ++j)
      pdbl[j] /= 255; // now the profile is number of white pixels (was GS prof)

    unsigned int profMax;
    profMax = (unsigned int)prof.max();

    //use original image to create histogram of horizontal foreground spacing
    //(distance from black pixel to next black pixel) weighted by profile value
    //inverse (number of black pixels instead of white pixels)
    for(int y=2; y < (h-2); ++y){//ignore 2 on each end (smoothing boundaries)
      int lastBlackX;
      int runlength;
      int x;
      int weight;

      x=0;
      lastBlackX = -1;
      runlength = 0;
      for(x=0 ; x < w; ++x){
	if(pu8[y*w+x] == 0){//black
	  runlength = x - lastBlackX;
	  if((runlength >= 2) && (runlength < 200)){
	    weight = (int)profMax - (int)pdbl[y];//inverse of profile value at y
	    rgBlackSpacingHist[runlength] += weight;
	  }
	  lastBlackX=x;
	}
      }
    }
  }

  //to get the spacing estimate, get the max, then find the next position
  //that is less than 1/3 of the max.  Use that as the estimate to determine
  //scale
  int spacingMax;
  int spacingEstimate;
  
  spacingMax = 2;
  for(int j=3; j<200; ++j){
    if(rgBlackSpacingHist[j] > rgBlackSpacingHist[spacingMax])
      spacingMax = j;
  }
  spacingEstimate = spacingMax;
  for(int j=spacingMax+1; j < 200; ++j){
    if(rgBlackSpacingHist[j] < (rgBlackSpacingHist[spacingMax] / 3)){
      spacingEstimate = j;
      break;
    }
  }
  printf(" spacing estimate =        *** %d pixels\n",spacingEstimate);
  if(NULL != spacingEst){
    (*spacingEst) = spacingEstimate;
  }

  // now smear the image based on the spacing estimate, then take new profiles
  DImage imgSmear;
  D_uint8 *psmear;
  imgSmear = img;
  psmear = imgSmear.dataPointer_u8();
  
  for(int y=0; y < h; ++y){
    int lastBlackX;
    int runlength;
    
    lastBlackX = w;
    for(int x=0; x < w; ++x){
      if(pu8[y*w+x] == 0){//black
	runlength = x - lastBlackX;
	if((runlength < 2*spacingEstimate) && (runlength >0)){
	  // fill in the white since last black pixel with black
	  for(int xp=lastBlackX+1; xp < x; ++xp){
	    psmear[(y*w+xp)] = 128;
	  }
	}
	lastBlackX = x;
      }
    }
  }
  sprintf(stTmp,"%s_smear.ppm",stDebugBaseName);
  imgSmear.save(stTmp);

  // now recalculate all of the profiles
  int rgSmearThresh;

  {
    double *pdbl;
    unsigned int profMax;
    // imgSmear.copy_(imgStrip, stripLeft, 0, stripW, h);
    profSmear.getImageVerticalProfile(img,false);


    // invert the profile so black is 255 and white is zero before smoothing
    pdbl = profSmear.dataPointer();
    profMax = (unsigned int)profSmear.max();
    for(int y=0; y < h; ++y)
      pdbl[y] = profMax - pdbl[y];

    profSmear.smoothAvg(spacingEstimate*2/3);
    profMax = (unsigned int)profSmear.max();//new max after smoothing





    // decide where peak/valleys in profile are
    {
      int prevSign = 0;
      double deriv;
      double *pdbl;
      int numZeros = 0;

      pdbl = profSmear.dataPointer();
      //use profile derivative and dist from last peak/valley
      //to decide where peaks and valleys are
      for(int y=1; y < (h-1); ++y){

	int left, right;
	right = y + spacingEstimate/2;
	if(right > h)
	  right = h;
	left = y - spacingEstimate/2;
	if(left < 0)
	  left = 0;
	// deriv = pdbl[y+1] - pdbl[y-1];
	deriv = pdbl[right] - pdbl[left];
	if(deriv > 0.){//rising
	  if(prevSign <= 0){//valley
	    rgValleyYs[numValleys] = y-numZeros/2;//(middle of plateaus)
	    ++numValleys;
	  }
	  prevSign = 1;
	  numZeros = 0;
	}
	else if(deriv < 0.){//falling
	  if(prevSign >= 0){//peak
	    rgPeakYs[numPeaks] = y-numZeros/2;//(middle of plateaus)
	    ++numPeaks;
	  }
	  prevSign = -1;
	  numZeros = 0;
	}
	else{ // zero slope
	  ++numZeros;
	}
      }//end for(y=...
    }

#if 0
    // refine valleys so they are at true minima
    for(int v=0; v < numValleys; ++v){
      bool fRefined = false;
      int origY;
      origY = rgValleyYs[v];
      for(int offs=1; offs < spacingEstimate/2; ++offs){
	int checkY;
	checkY = rgValleyYs[v]-offs;
	if(checkY>=0){
	  if(pdbl[checkY] < pdbl[rgValleyYs[v]]){
	    rgValleyYs[v] = checkY;
	    fRefined = true;
	  }
	}
	checkY = rgValleyYs[v]+offs;
	if(checkY<h){
	  if(pdbl[checkY] < pdbl[rgValleyYs[v]]){
	    rgValleyYs[v] = checkY;
	    fRefined = true;
	  }
	}
      }
      if(fRefined)
	printf("    >>refined valley%d from y=%d to y=%d\n",
	       v,origY,rgValleyYs[v]);
    }
#endif



// #if 0
//     // get rid of false peaks (those that have very low prominence)
//     {
//       // figure out weighted avg prominence (weight by prominence of each peak)
//       double sumProm = 0.;
//       int numProm = 0;
//       for(int p=0; p < numPeaks; ++p){
// 	int numSides; // will be 1, 2, or 0
	
	
// 	//	double prom = pdbl[rgPeakYs[p]
//       }
//     }

//     // combine peaks that are too close to each other
//     {
//       int numPeaksRemoved = 0;
//       bool fRemoved;
//       fRemoved = true;
//       while(fRemoved && (numPeaks>1)){
// 	fRemoved = false;
// 	int deletePeak=0;
// 	int deleteValley=0;
// 	for(int j=1; j < numPeaks; ++j){
// 	  if(abs(rgPeakYs[j-1]-rgPeakYs[j])<spacingEstimate*2/3){//too close
// 	    if(pdbl[rgPeakYs[j]] > pdbl[rgPeakYs[j-1]]){
// 	      printf("  A remove peak %d at y=%d\n",j-1,(int)pdbl[rgPeakYs[j-1]]);
// 	      deletePeak = j-1;
// 	    }
// 	    else{
// 	      printf("  B remove peak %d at y=%d\n",j,(int)pdbl[rgPeakYs[j]]);
// 	      deletePeak = j;
// 	    }
// 	    deleteValley = -1;

// 	    if(numValleys > 0){
// 	      if(rgPeakYs[0] < rgValleyYs[0]){//peak was first
// 		deleteValley = deletePeak;
// 	      }
// 	      else{//valley was first
// 		deleteValley = deletePeak+1;
// 	      }
// 	    }

// 	    //delete the peak
// 	    for(int k=deletePeak+1; k < numPeaks; ++k){
// 	      rgPeakYs[k-1] = rgPeakYs[k];
// 	    }
// 	    --numPeaks;
// 	    //delete the valley (if in range)
// 	    if((deleteValley>=0) && (deleteValley < numValleys)){
// 	      for(int k=deleteValley+1; k < numValleys; ++k){
// 		rgValleyYs[k-1] = rgValleyYs[k];
// 	      }
// 	    }
// 	    fRemoved = true;
// 	    ++numPeaksRemoved;
// 	    break;
// 	  }//if(abs(...
// 	}//for(int j=...
//       }//while(fRemoved)
//     }

//     {//figure out peak-to-valley topographic prominence threshold
      
//     }
// #endif



  }

  printf("fPeakFirst = %d\n",(int)(rgPeakYs[0] < rgValleyYs[0]));
  printf("numPeaks=%d numValleys=%d\n", numPeaks, numValleys);
  for(int p=0; (p < numPeaks) || (p<numValleys); ++p){
    printf("\t%d:\t",p);
    if(p< numPeaks)
      printf("p%4d\t",rgPeakYs[p]);
    else
      printf("p----\t");
    if(p< numValleys)
      printf("v%4d\t",rgValleyYs[p]);
    else
      printf("v----\t");
    printf("\n");
  }


  (*numTextlines) = numPeaks;
  (*rgTextlineRects) = new DRect[numPeaks];
  D_CHECKPTR((*rgTextlineRects));
  bool fPeakFirst;
  fPeakFirst = rgPeakYs[0] < rgValleyYs[0];
  for(int p = 0; p < numPeaks; ++p){
    int topIdx, botIdx;
    if(fPeakFirst){
      topIdx = p-1;
      botIdx = p;
    }
    else{
      topIdx = p;
      botIdx = p+1;
    }
    (*rgTextlineRects)[p].x = 0;
    (*rgTextlineRects)[p].w = w-1;
    if(topIdx < 0)
      (*rgTextlineRects)[p].y = 0;
    else if(topIdx >= numValleys){
      fprintf(stderr, "This shouldn't happen!(%s:%d)\n", __FILE__, __LINE__);
      (*rgTextlineRects)[p].y = 0;
    }
    else{
      (*rgTextlineRects)[p].y = rgValleyYs[topIdx];
    }
    if(botIdx < 0){
      fprintf(stderr, "This shouldn't happen!(%s:%d)\n", __FILE__, __LINE__);
      (*rgTextlineRects)[p].h = h-((*rgTextlineRects)[p].y)-1;
    }
    else if(botIdx >= numValleys){
      (*rgTextlineRects)[p].h = h-((*rgTextlineRects)[p].y)-1;
    }
    else{
      (*rgTextlineRects)[p].h = rgValleyYs[botIdx]-((*rgTextlineRects)[p].y)-1;
    }
  }


  // now remove any textlines that seem empty
  {
    //avg # of pixels within a textline (weighted by # pxls in that textline)
    double sumPxls;
    double sumWeights;
    sumPxls = 0;
    sumWeights = 0;
    long *rgNumPixels;
    long pxlThresh;

    rgNumPixels = (long*)calloc(*numTextlines, sizeof(long));
    D_CHECKPTR(rgNumPixels);
    sumPxls = 0;
    sumWeights = 0;
    for(int p=0; p < (*numTextlines); ++p){
      long numPxls;
      numPxls = 0;
      for(int y=(*rgTextlineRects)[p].y; y <
	    ((*rgTextlineRects)[p].y+(*rgTextlineRects)[p].h); ++y){
	for(int x=(*rgTextlineRects)[p].x; x <
	      ((*rgTextlineRects)[p].x+(*rgTextlineRects)[p].w); ++x){
	  if(pu8[y*w+x]==0){//black pixel
	    ++numPxls;
	  }
	}
      }
      printf("    line%d numPxls=%ld\n",p,numPxls);
      rgNumPixels[p]=numPxls;
      sumPxls += numPxls * numPxls;
      sumWeights += numPxls;
    }
    printf("    sumPxls=%f sumWeights=%f\n",sumPxls, sumWeights);
    if(sumWeights > 0)
      sumPxls /= sumWeights;
    else
      sumPxls = 0;

    printf("    weighted avg number of pixels per line:%f\n",sumPxls);
    pxlThresh = sumPxls/10;
    printf("    pixel threshold=%ld\n",pxlThresh);
    
    //now get rid of lines with few pixels
    for(int p=(*numTextlines)-1; p >=0; --p){
      if(rgNumPixels[p] < pxlThresh){// one-twentieth of weighted avg
	printf("    remove textline %d (y=%d to y=%d) with %ld pixels\n",p,
	       (*rgTextlineRects)[p].y,(*rgTextlineRects)[p].y+
	       (*rgTextlineRects)[p].h, rgNumPixels[p]);
	for(int r=p; r < ((*numTextlines)-1); ++r){
	  (*rgTextlineRects)[r] = (*rgTextlineRects)[r+1];
	}
	--(*numTextlines);
      }
    }

    free(rgNumPixels);
  }

  printf("   There are now %d textlines\n", (*numTextlines));

  //debug: save an image with the textline rectangles drawn
  {
    DImage imgTextlines;
    imgTextlines = img.convertedImgType(DImage::DImage_RGB);
    for(int p = 0; p < (*numTextlines); ++p){
      int colorR, colorG, colorB;
      printf("\trect%d: x,y wxh=%d,%d %dx%d\n",p,(*rgTextlineRects)[p].x,
	     (*rgTextlineRects)[p].y,
	     (*rgTextlineRects)[p].w,(*rgTextlineRects)[p].h);
      colorR = ((p+1)*127) % 255;
      colorG = (p*127) % 255;
      colorB = (p) % 255;
      imgTextlines.drawRect((*rgTextlineRects)[p].x,(*rgTextlineRects)[p].y,
			    (*rgTextlineRects)[p].x+(*rgTextlineRects)[p].w-1,
			    (*rgTextlineRects)[p].y+(*rgTextlineRects)[p].h,
			    colorR, colorG, colorB);
      imgTextlines.drawRect((*rgTextlineRects)[p].x+1,(*rgTextlineRects)[p].y+1,
			    (*rgTextlineRects)[p].x+(*rgTextlineRects)[p].w-1-1,
			    (*rgTextlineRects)[p].y+(*rgTextlineRects)[p].h-1,
			    colorR, colorG, colorB);
      imgTextlines.drawRect((*rgTextlineRects)[p].x+2,(*rgTextlineRects)[p].y+2,
			    (*rgTextlineRects)[p].x+(*rgTextlineRects)[p].w-1-2,
			    (*rgTextlineRects)[p].y+(*rgTextlineRects)[p].h-2,
			    colorR, colorG, colorB);
    }
    sprintf(stTmp,"%s_tl_rects.pgm",stDebugBaseName);
    imgTextlines.save(stTmp);

  }





//   // now get x-height estimate using profiles (or black runlengths of smears)
// #if 0

//   //debug: save an image with all of the profiles
//   {
//     DImage imgProfsAll;
//     imgProfsAll = prof.toDImage(500,true);
//     imgProfsAll = imgProfsAll.convertedImgType(DImage::DImage_RGB);

//     sprintf(stTmp,"%s_allprofs.pgm",stDebugBaseName);
//     imgProfsAll.save(stTmp);
//   }

//   //debug: save an image with all of the smeared profiles
//   {
//     DImage imgProfsAll;
//     imgProfsAll = profSmear.toDImage(500,true);
//     imgProfsAll = imgProfsAll.convertedImgType(DImage::DImage_RGB);
    
//     for(int j=0; j < numPeaks; ++j){
//       int ypos;
//       ypos = rgPeakYs[j];
//       imgProfsAll.drawLine(0,ypos,499,ypos,255,0,0);
//     }
    
//     for(int j=0; j < numValleys; ++j){
//       int ypos;
//       ypos = rgValleyYs[j];
//       imgProfsAll.drawLine(0,ypos,499,ypos,0,255,0);
//     }
    
    
    
//     sprintf(stTmp,"%s_allsmearprofs.pgm",stDebugBaseName);
//     imgProfsAll.save(stTmp);
//   }

//   //debug: save a gnuplot of the histograms of black spacing weighted by profile
//   // the image has the histogram for each strip followed by the sum histogram
//   // a value of -10 is placed at positions 0,1 of each histogram as a separator
//   {
//     DImage imgSpacingHists;
//     FILE *fout;

//     sprintf(stTmp,"%s_spacing_profs.dat",stDebugBaseName);
//     fout = fopen(stTmp,"wb");
//     if(!fout){
//       fprintf(stderr, "couldn't open debug file '%s' for output\n",stTmp);
//       exit(1);
//     }
//     for(int j=0; j < 200; ++j){
//       int val;
//       val = rgBlackSpacingHist[j];
//       if(j<2)
// 	val = -10;
//       fprintf(fout,"%d\t%d\n",j, val);
//     }
//     fclose(fout);
//   }
// #endif

  // now at the otsu x-position in the profile, get avg black runlength to
  // guess at peak (textline) height.
  // Do the same for white to guess at valley (spacing) height.
  // After getting it for each strip's profile, take the avg for the whole
  // page.  Use that to determine a smoothing value and a window size for the
  // transition count map (TCM). (maybe use median instead of avg?)

  delete [] rgPeakYs;
  delete [] rgValleyYs;
  delete [] rgBlackSpacingHist;


  return;
}


