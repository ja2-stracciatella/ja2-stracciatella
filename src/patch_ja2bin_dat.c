#include <stdio.h>
#include <stdint.h>

/*
 * Usage: patch_ja2_bin <ja2bin.dat >ja2bin.dat.patched
 *
 */
uint16_t animInsts[320][100];

int main(int argc, char **argv)
{
  size_t rb = fread(animInsts, 1, sizeof(animInsts), stdin);
  if (rb == sizeof(animInsts))
  {
    animInsts[224][16] = 753;
    animInsts[224][17] = 499;

    size_t wb = fwrite(animInsts, 1, sizeof(animInsts), stdout);
    return wb == sizeof(animInsts) ? 0 : 3;
  }
  return 2;
}
