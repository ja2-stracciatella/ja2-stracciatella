#include "EncodingCorrectors.h"

/** Fix one UTF-16 code point. */
uint16_t RussianEncodingCorrector::fix(uint16_t codePoint) const
{
  /* The Russian data files are incorrectly encoded. The original texts seem to
   * be encoded in CP1251, but then they were converted from CP1252 (!) to
   * UTF-16 to store them in the data files. Undo this damage here. */
  if (0xC0 <= codePoint && codePoint <= 0xFF)
  {
    codePoint += 0x350;
  }
  return codePoint;
}

