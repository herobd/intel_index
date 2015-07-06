#ifndef DTEXTLINESEPARATOR_H
#define DTEXTLINESEPARATOR_H

#include "dimage.h"

class DRect;//forward declaration (drect.h included from dtextlineseparator.cpp)

class DTextlineSeparator{
public:
  static int estimateAvgHeight(DImage &imgBinary, int ROIx0=0, int ROIy0=0,
			       int ROIx1=-1, int ROIy1=-1,
			       char *stDebugBaseName = NULL);
  static int estimateAvgHeight2(DImage &imgBinary, int numStrips = 10,
				char *stDebugBaseName = NULL);
  // static void getTextlineRects(DImage &img, int *numTextLines,
  // 			       DRect **rgTextlineRects, int avgHeight = -1,
  // 			       int ROIx0=0, int ROIy0=0,
  // 			       int ROIx1=-1, int ROIy1=-1);

  static void getTextlineRects(DImage &img, int *numTextlines,
			       DRect **rgTextlineRects,
			       int *spacingEst=NULL,
			       char *stDebugBaseName=NULL);


  // static void geMedianPeakInfo(unsigned int *prof, int len, int *medPeakDist,
  // 			       int *medPeakThickness,
  // 			       int *medTroughThickness, int *medPeakHeight,
  // 			       int *threshVal, int *numPeaksAtThresh);
};

#endif
