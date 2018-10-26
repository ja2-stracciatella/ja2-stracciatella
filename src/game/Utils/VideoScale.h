#ifndef __VIDEOSCALE_H_
#define __VIDEOSCALE_H_

#include "HImage.h"

SGPImage * ScaleImage(SGPImage *image, DOUBLE factor);
SGPImage * ScaleAlphaImage(SGPImage *image, DOUBLE factor);

#endif /*__VIDEOSCALE_H_*/
