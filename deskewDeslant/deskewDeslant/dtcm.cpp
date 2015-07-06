#include "dtcm.h"

/// TCM (Transition Count Map) for black-to-white or white-to-black transitions
/**Computes a TCM for imgSrc and stores it in imgDst.  The TCM value
   at each location is the number of transition pixels within a
   rectangular area centered at that pixel defined by radiusX and
   radiusY. If fVertical is true, then the transitions are decided for
   vertical neighbors, otherwise horizontal transitions are found
   (default).
 */
void DTCM::getImageTCM_(DImage &imgDst, DImage &imgSrc,
			int radiusX, int radiusY, bool fVertical,
			char *stDebugBaseName){
  int w, h;
  D_uint64 *pIntegralImg64;//integral image of values
  D_uint8 *pu8Src, *pu8Dst;
  char stTmp[1024];
  D_uint8 transitionVal = 0x01;

  getTransitionImage_(imgDst, imgSrc, false, fVertical);

//   if(fScale)
//     transitionVal = 0xff;

  w=imgSrc.width();
  h=imgSrc.height();

//   imgDst.create(w,h,DImage::DImage_u8);
  pu8Src = imgSrc.dataPointer_u8();
  pu8Dst = imgDst.dataPointer_u8();

//   if(fVertical){
//     for(int y=1; y < h; ++y){
//       for(int x=0; x < w; ++x){
// 	int idx;
// 	idx = y*w+x;
// #if 1
// 	if((pu8Src[idx]!=255) && (pu8Src[idx]!=0)){
// 	  fprintf(stderr,"DTCM::getImageTCM_() only allows bitonal images!\n");
// 	  exit(1);
// 	}
// #endif       
// 	if(pu8Src[idx] != pu8Src[idx-w]){//transition from BtoW or WtoB
// 	  pu8Dst[idx] = transitionVal;
// 	}
// 	else{
// 	  pu8Dst[idx] = 0x00;
// 	}
//       }
//     }
//   }
//   else{
//     for(int y=0; y < h; ++y){
//       for(int x=1; x < w; ++x){
// 	int idx;
// 	idx = y*w+x;
// #if 1
// 	if((pu8Src[idx]!=255) && (pu8Src[idx]!=0)){
// 	  fprintf(stderr,"DTCM::getImageTCM_() only allows bitonal images!\n");
// 	  exit(1);
// 	}
// #endif       
// 	if(pu8Src[idx] != pu8Src[idx-1]){//transition from BtoW or WtoB
// 	  pu8Dst[idx] = transitionVal;
// 	}
// 	else{
// 	  pu8Dst[idx] = 0x00;
// 	}
//       }
//     }
//   }

  sprintf(stTmp,"%s_trans.pgm",stDebugBaseName);
  if(NULL != stDebugBaseName)
    imgDst.save(stTmp);

  // use integral images to keep track of how many transitions within kernel

  pIntegralImg64 = (D_uint64*)malloc(sizeof(D_uint64) * w * h);
  D_CHECKPTR(pIntegralImg64);

  // fill in the integral image values
  pIntegralImg64[0] = (D_uint64)(pu8Dst[0]);
  for(int x = 1; x < w; ++x){ // first row
    pIntegralImg64[x] = pIntegralImg64[x-1]+(D_uint64)(pu8Dst[x]);
  }
  for(int y = 1, idx=w; y < h; ++y){
    pIntegralImg64[idx] = pIntegralImg64[idx-w] + (D_uint64)(pu8Dst[idx]);
    ++idx;
    for(int x = 1; x < w; ++x, ++idx){
      pIntegralImg64[idx] = pIntegralImg64[idx-1]+
	pIntegralImg64[idx-w] + (D_uint64)(pu8Dst[idx]) -
	pIntegralImg64[idx-w-1];
    }
  }

  // now get the sum of values within the rectangular kernel and store in imgDst
  int ww;
  ww = 2*radiusX+1;
  for(int y=0, idx=0; y < h; ++y){
    for(int x=0; x < w; ++x, ++idx){
      int xleft, ytop, xright, ybot;
      int idxA, idxB, idxC, idxD;
      D_uint64 sum;
      xleft = (x-radiusX-1);
      if(xleft < 0)
	xleft = 0;
      xright = (x+radiusX);
      if(xright >= w)
	xright = w-1;
      ytop = (y-radiusY-1);
      if(ytop < 0)
	ytop = 0;
      ybot = (y+radiusY);
      if(ybot >= h)
	ybot = h-1;
      idxB = ytop * w + xleft;
      idxC = ytop * w + xright;
      idxD = ybot * w + xleft;
      idxA = ybot * w + xright;
      sum = pIntegralImg64[idxA] + pIntegralImg64[idxB] -
	pIntegralImg64[idxC] - pIntegralImg64[idxD];
      if(sum > 255)
	pu8Dst[idx] = 255;
      else
	pu8Dst[idx] = (D_uint8)sum;
    }
  }

  free(pIntegralImg64);

  sprintf(stTmp,"%s_transmap.pgm",stDebugBaseName);
  if(NULL != stDebugBaseName)
    imgDst.save(stTmp);
  imgDst.setDataRange(0,255);
  sprintf(stTmp,"%s_transmapstretch.pgm",stDebugBaseName);
  if(NULL != stDebugBaseName)
    imgDst.save(stTmp);
}

///Store an image that is black except where there are B-W or W-B transitions 
/**If fScale is true, transition pixels are 255, otherwise they are 1.
   All non-transition pixels are 0 (black).  If fVertical is true, then
   the transitions are decided for vertical neighbors, otherwise horizontal
   transitions are found (default).
 */
void DTCM::getTransitionImage_(DImage &imgDst, DImage &imgSrc, bool fScale,
			       bool fVertical){
  int w, h;
  D_uint64 *pIntegralImg64;//integral image of values
  D_uint8 *pu8Src, *pu8Dst;
  char stTmp[1024];
  D_uint8 transitionVal = 0x01;

  if(fScale)
    transitionVal = 0xff;

  w=imgSrc.width();
  h=imgSrc.height();

  imgDst.create(w,h,DImage::DImage_u8);
  pu8Src = imgSrc.dataPointer_u8();
  pu8Dst = imgDst.dataPointer_u8();

  if(fVertical){
    for(int y=1; y < h; ++y){
      for(int x=0; x < w; ++x){
	int idx;
	idx = y*w+x;
#if 1
	if((pu8Src[idx]!=255) && (pu8Src[idx]!=0)){
	  fprintf(stderr,"DTCM::getImageTCM_() only allows bitonal images!\n");
	  exit(1);
	}
#endif       
	if(pu8Src[idx] != pu8Src[idx-w]){//transition from BtoW or WtoB
	  pu8Dst[idx] = transitionVal;
	}
	else{
	  pu8Dst[idx] = 0x00;
	}
      }
    }
  }
  else{
    for(int y=0; y < h; ++y){
      for(int x=1; x < w; ++x){
	int idx;
	idx = y*w+x;
#if 1
	if((pu8Src[idx]!=255) && (pu8Src[idx]!=0)){
	  fprintf(stderr,"DTCM::getImageTCM_() only allows bitonal images!\n");
	  exit(1);
	}
#endif       
	if(pu8Src[idx] != pu8Src[idx-1]){//transition from BtoW or WtoB
	  pu8Dst[idx] = transitionVal;
	}
	else{
	  pu8Dst[idx] = 0x00;
	}
      }
    }
  }
}
