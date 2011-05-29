#include "assert.h"
#include "defines.h"
#include "bits.h"
#include "util.h"


bit16 wordswap(bit16 v)
{
	return (v>>8) | ((v & 0xff) << 8);
}

bit32 dwordswap(bit32 v)
{

	return ((v & 0xff000000)>>24) | ((v & 0x000000ff) << 24) | ((v & 0x00ff0000) >> 8) | ((v & 0x0000ff00) << 8);
}

C64WindowDimensions::C64WindowDimensions()
{
	SetBorder(HCFG::EMUBORDER_FULL);
}

C64WindowDimensions::C64WindowDimensions(int width, int height, int start, int firstRasterLine, int lastRasterLine)
{
	Width = width;
	Height = height;
	Start = start;
	FirstRasterLine = firstRasterLine;
	LastRasterLine = lastRasterLine;
}

void C64WindowDimensions::SetBorder(HCFG::EMUBORDERSIZE border)
{
	switch (border)
	{
	case HCFG::EMUBORDER_FULL:
		Width = iFullWidth;
		Height = iFullHeight;
		Start = iFullStart;
		FirstRasterLine = iFullFirstRaster;
		LastRasterLine = iFullLastRaster;
		break;
	case HCFG::EMUBORDER_TV:
		Width = iTVWidth;
		Height = iTVHeight;
		Start = iTVStart;
		FirstRasterLine = iTVFirstRaster;
		LastRasterLine = iTVLastRaster;
		break;
	case HCFG::EMUBORDER_SMALL:
		Width = iSmallWidth;
		Height = iSmallHeight;
		Start = iSmallStart;
		FirstRasterLine = iSmallFirstRaster;
		LastRasterLine = iSmallLastRaster;
		break;
	default:
		Width = iFullWidth;
		Height = iFullHeight;
		Start = iFullStart;
		FirstRasterLine = iFullFirstRaster;
		LastRasterLine = iFullLastRaster;
	}
	assert(LastRasterLine == FirstRasterLine + Height - 1);
}

void C64WindowDimensions::SetBorder(int screenWidth, int screenHeight, int toolbarHeight)
{
	int cHeight = screenHeight - toolbarHeight;
	int w = (iNoBorderWidth + screenWidth - iNoBorderWidth);
	if (w < 0)
		w = 0;
	if (w > iFullWidth)
		w = iFullWidth;

	int h = cHeight;
	if (h < 0)
		h = 0;
	if (h > iFullHeight)
		h = iFullHeight;

	int s = iNoBorderStart + (iNoBorderWidth - screenWidth) / 2;
	if (s < iFullStart)
		s = iFullStart;

	int fr = iNoBorderFirstRaster + (iNoBorderHeight - h) / 2;
	if (fr < iFullFirstRaster)
		fr = iFullFirstRaster;

	int lr = fr + h - 1;
	if (lr > iFullLastRaster)
		lr = iFullLastRaster;


	Width = w;
	Height = h;
	Start = s;
	FirstRasterLine = fr;
	LastRasterLine = lr;
}
