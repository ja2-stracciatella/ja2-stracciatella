#ifndef __VIDEOSCALE_H_
#define __VIDEOSCALE_H_

#include "HImage.h"

SGPImage * ScaleImage(SGPImage *image, DOUBLE factor, bool yInterpolation = true);
SGPImage * ScaleAlphaImage(SGPImage *image, DOUBLE factor, bool yInterpolation = true);

#endif /*__VIDEOSCALE_H_*/
