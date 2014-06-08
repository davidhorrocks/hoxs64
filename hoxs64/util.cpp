#include <assert.h>
#include <math.h>
#include "defines.h"
#include "bits.h"
#include "util.h"

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
		Width = WDFullWidth;
		Height = WDFullHeight;
		Start = WDFullStart;
		FirstRasterLine = WDFullFirstRaster;
		LastRasterLine = WDFullLastRaster;
		break;
	case HCFG::EMUBORDER_TV:
		Width = WDTVWidth;
		Height = WDTVHeight;
		Start = WDTVStart;
		FirstRasterLine = WDTVFirstRaster;
		LastRasterLine = WDTVLastRaster;
		break;
	case HCFG::EMUBORDER_SMALL:
		Width = WDSmallWidth;
		Height = WDSmallHeight;
		Start = WDSmallStart;
		FirstRasterLine = WDSmallFirstRaster;
		LastRasterLine = WDSmallLastRaster;
		break;
	case HCFG::EMUBORDER_NOSIDE:
		Width = WDNoBorderWidth;
		Height = WDTVHeight;
		Start = WDNoBorderStart;
		FirstRasterLine = WDTVFirstRaster;
		LastRasterLine = WDTVLastRaster;
		break;
	case HCFG::EMUBORDER_NOTOP:
		Width = WDTVWidth;
		Height = WDNoBorderHeight;
		Start = WDTVStart;
		FirstRasterLine = WDNoBorderFirstRaster;
		LastRasterLine = WDNoBorderLastRaster;
		break;
	case HCFG::EMUBORDER_NOBORDER:
		Width = WDNoBorderWidth;
		Height = WDNoBorderHeight;
		Start = WDNoBorderStart;
		FirstRasterLine = WDNoBorderFirstRaster;
		LastRasterLine = WDNoBorderLastRaster;
		break;
	default:
		Width = WDFullWidth;
		Height = WDFullHeight;
		Start = WDFullStart;
		FirstRasterLine = WDFullFirstRaster;
		LastRasterLine = WDFullLastRaster;
	}
	assert(LastRasterLine == FirstRasterLine + Height - 1);
}

void C64WindowDimensions::SetBorder(int screenWidth, int screenHeight, int toolbarHeight)
{
	int w = screenWidth;
	if (w < 0)
		w = 0;
	if (w > WDFullWidth)
		w = WDFullWidth;

	int h = screenHeight - toolbarHeight;
	if (h < 0)
		h = 0;
	if (h > WDFullHeight)
		h = WDFullHeight;

	int s = WDNoBorderStart + (WDNoBorderWidth - screenWidth) / 2;
	if (s < WDFullStart)
		s = WDFullStart;

	int fr = WDNoBorderFirstRaster + (WDNoBorderHeight - h) / 2;
	if (fr < WDFullFirstRaster)
		fr = WDFullFirstRaster;

	int lr = fr + h - 1;
	if (lr > WDFullLastRaster)
		lr = WDFullLastRaster;


	Width = w;
	Height = h;
	Start = s;
	FirstRasterLine = fr;
	LastRasterLine = lr;
}

void C64WindowDimensions::SetBorder2(int screenWidth, int screenHeight, int toolbarHeight)
{
	int h;
	int w;
	int lr;
	int fr;
	int s;
	const int BEZEL = 28;
	double c64ratio, screenratio;
	c64ratio = (double)WDNoBorderWidth/ (double)WDNoBorderHeight;
	screenratio = (double)screenWidth / (double)(screenHeight - toolbarHeight);
	if (screenratio >= c64ratio)
	{
		int bw = (int)floor(((double)WDNoBorderHeight * screenratio - (double)WDNoBorderWidth) / 2.0);
		if (bw >= BEZEL)
		{
			w = BEZEL * 2 + WDNoBorderWidth;
			h = WDNoBorderHeight;
			fr = WDNoBorderFirstRaster;
			lr = WDNoBorderLastRaster;
			s = WDNoBorderStart - BEZEL;
		}
		else
		{
			w = BEZEL * 2 + WDNoBorderWidth;
			h = (int)floor((double)w / screenratio);
			fr = WDNoBorderFirstRaster - (h - WDNoBorderHeight) / 2;
			lr = WDNoBorderLastRaster + (h - WDNoBorderHeight) / 2;
			s = WDNoBorderStart - BEZEL;
		}
	}
	else
	{
		int bw = (int)floor(((double)WDNoBorderWidth / (double)screenratio - WDNoBorderHeight) / 2.0);
		if (bw >= BEZEL)
		{
			w = WDNoBorderWidth;
			h = BEZEL * 2 + WDNoBorderHeight;
			fr = WDNoBorderFirstRaster - BEZEL;
			lr = WDNoBorderLastRaster + BEZEL;
			s = WDNoBorderStart;
		}
		else
		{
			h = BEZEL * 2 + WDNoBorderHeight;
			w = (int)floor((double)h * screenratio);
			fr = WDNoBorderFirstRaster - BEZEL;
			lr = WDNoBorderLastRaster + BEZEL;
			s = WDNoBorderStart - (w - WDNoBorderWidth) / 2;
		}
	}
	Width = w;
	Height = h;
	Start = s;
	FirstRasterLine = fr;
	LastRasterLine = lr;
}
