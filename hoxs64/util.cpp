#include <windows.h>
#include <assert.h>
#include <tchar.h>
#include <math.h>
#include <vector>
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

C64WindowDimensions::Scaling::Scaling(ScalingType scaleType, double scale)
	: scaleType(scaleType), scale(scale)
{
}

C64WindowDimensions::Scaling::Scaling()
	: scaleType(Scaling::TopBottomOuter), scale((double)(WDNoBorderWidth + 2 * BEZEL) / (double)(WDNoBorderHeight + 2 * BEZEL))
{
}

void C64WindowDimensions::SetBorder2(int screenWidth, int screenHeight, int toolbarHeight)
{
	int h;
	int w;
	int lr;
	int fr;
	int s;
	double c64ratio, c64wbratio, screenratio;
	Scaling scaleInner;
	Scaling scaleOuter;
	Scaling scale;
	int clientHeight = screenHeight - toolbarHeight;
	int clientWidth = screenWidth;
	int withBorderHeight = WDNoBorderHeight + 2 * BEZEL;
	int withBorderWidth = WDNoBorderWidth + 2 * BEZEL;
	c64ratio = (double)WDNoBorderWidth/ (double)WDNoBorderHeight;
	c64wbratio = (double)(withBorderWidth)/ (double)(withBorderHeight);
	screenratio = (double)clientWidth / (double)(clientHeight);
	Scaling scaleTopBottomOuter(Scaling::TopBottomOuter, (double)clientHeight / (double)(withBorderHeight));
	Scaling scaleTopBottomInner(Scaling::TopBottomInner, (double)clientHeight / (double)(WDNoBorderHeight));
	Scaling scaleLeftRightOuter(Scaling::LeftRightOuter, (double)clientWidth / (double)(withBorderWidth));
	Scaling scaleLeftRightInner(Scaling::LeftRightInner, (double)clientWidth / (double)(WDNoBorderWidth));
	if (scaleTopBottomInner.scale < scaleLeftRightInner.scale)
	{
		scaleInner = scaleTopBottomInner;
	}
	else
	{
		scaleInner = scaleLeftRightInner;
	}

	if (scaleTopBottomOuter.scale > scaleLeftRightOuter.scale)
	{
		scaleOuter = scaleTopBottomOuter;
	}
	else
	{
		scaleOuter = scaleLeftRightOuter;
	}

	if (scaleOuter.scale < scaleInner.scale)
	{
		scale = scaleOuter;
	}
	else
	{
		scale = scaleInner;
	}

	switch (scale.scaleType)
	{
	case Scaling::TopBottomOuter:
		h = withBorderHeight;
		w = min((int)floor(clientWidth / scale.scale), withBorderWidth);
		s = WDNoBorderStart + (WDNoBorderWidth - w) / 2;
		fr = WDNoBorderFirstRaster - BEZEL;		
		lr = fr + h - 1;
		break;
	case Scaling::TopBottomInner:
		h = WDNoBorderHeight;
		w = min((int)floor(clientWidth / scale.scale), withBorderWidth);
		s = WDNoBorderStart + (WDNoBorderWidth - w) / 2;
		fr = WDNoBorderFirstRaster;
		lr = fr + h - 1;
		break;
	case Scaling::LeftRightInner:
		w = WDNoBorderWidth;
		h = min((int)floor(clientHeight / scale.scale), withBorderHeight);
		s = WDNoBorderStart;
		fr = WDNoBorderFirstRaster + (WDNoBorderHeight - h) / 2;
		lr = fr + h - 1;
		break;
	case Scaling::LeftRightOuter:
		w = withBorderWidth;
		h = min((int)floor(clientHeight / scale.scale), withBorderHeight);
		s = WDNoBorderStart - BEZEL;
		fr = WDNoBorderFirstRaster + (WDNoBorderHeight - h) / 2;
		lr = fr + h - 1;
		break;
	}

	Width = w;
	Height = h;
	Start = s;
	FirstRasterLine = fr;
	LastRasterLine = lr;
}
