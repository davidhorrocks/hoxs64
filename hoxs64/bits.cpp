#include "bits.h"

bit16 wordswap(bit16 v)
{
	return (v>>8) | ((v & 0xff) << 8);
}

bit32 dwordswap(bit32 v)
{
	return ((v & 0xff000000)>>24) | ((v & 0x000000ff) << 24) | ((v & 0x00ff0000) >> 8) | ((v & 0x0000ff00) << 8);
}