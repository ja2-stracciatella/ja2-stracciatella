#ifndef __VIDEOSCALE_H_
#define __VIDEOSCALE_H_

#include "HImage.h"

typedef void ScaleCallback(int i, int w, int h, UINT8 *buf);

SGPImage * ScaleImage(SGPImage *image, DOUBLE factorX, DOUBLE factorY, bool yInterpolation = true, ScaleCallback callback=nullptr);
SGPImage * ScaleAlphaImage(SGPImage *image, DOUBLE factorX, DOUBLE factorY, bool yInterpolation = true);

#endif /*__VIDEOSCALE_H_*/
