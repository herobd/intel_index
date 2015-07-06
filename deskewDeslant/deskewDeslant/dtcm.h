#ifndef DTCM_H
#define DTCM_H

#include "dimage.h"

class DTCM{
public:
  static void getImageTCM_(DImage &imgDst, DImage &imgSrc,
			   int radiusX, int radiusY, bool fVertical = false,
			   char *stDebugBaseName=NULL);
  static void getTransitionImage_(DImage &imgDst, DImage &imgSrc,
				  bool fScale = false,
				  bool fVertical = false);
};

#endif
