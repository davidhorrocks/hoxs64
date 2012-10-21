#ifndef __UTIL_H__
#define __UTIL_H__

#include "defines.h"
#include "bits.h"

struct null_deleter
{
    void operator()(void const *) const
    {
    }
};

namespace HCFG
{
	typedef enum tagAUDIOSPEED
	{
		AUDIO_OK=0,
		AUDIO_QUICK=1,
		AUDIO_SLOW=2
	} AUDIOSPEED;

	typedef enum tagFULLSCREENSYNCMODE
	{
		FSSM_VBL=0,
		FSSM_LINE=1,
	} FULLSCREENSYNCMODE;

	typedef enum tagEMUWINDOWSTRETCH {
		EMUWINSTR_AUTO = 0,
		EMUWINSTR_1X = 1,
		EMUWINSTR_2X = 2,
		EMUWINSTR_ASPECTSTRETCH = 3
	} EMUWINDOWSTRETCH;


	typedef enum tagEMUWINDOWFILTER {
		EMUWINFILTER_AUTO = 0,
		EMUWINFILTER_NONE = 1,
		EMUWINFILTER_POINT = 2,
		EMUWINFILTER_LINEAR = 3
	} EMUWINDOWFILTER;

	typedef enum tagEMUBORDERSIZE 
	{
		EMUBORDER_FULL = 0,
		EMUBORDER_TV = 1,
		EMUBORDER_SMALL = 2
	} EMUBORDERSIZE;

	typedef enum tagETRACKZEROSENSORSTYLE
	{
		TZSSPullHigh = 0,
		TZSSPullLow = 1,
		TZSSPositiveHigh = 2,
		TZSSPositiveLow = 3
	} ETRACKZEROSENSORSTYLE;

	typedef enum tagEMUFPS
	{
		EMUFPS_50 = 0,
		EMUFPS_50_12 = 1,
		EMUFPS_50_12_MULTI = 2,
		EMUFPS_60 = 3
	} EMUFPS;

	typedef enum tagCIAMODE
	{
		CM_CIA6526=0,
		CM_CIA6526A=1,
	} CIAMODE;
};

#define ALLOW_EMUFPS_50_12_MULTI

bit16 wordswap(bit16);
bit32 dwordswap(bit32 v);

class C64WindowDimensions
{
public:
	C64WindowDimensions();
	C64WindowDimensions(int Width, int Height, int start, int FirstRasterLine, int LastRasterLine);
	int Width;
	int Height;
	int Start;
	int FirstRasterLine;
	int LastRasterLine;
	void SetBorder(HCFG::EMUBORDERSIZE border);
	void SetBorder(int screenWidth, int screenHeight, int toolbarHeight);

	static const int iToolbarHeight = 10;

	static const int iFullHeight = HEIGHT_64;
	static const int iFullWidth = WIDTH_64;
	static const int iFullStart = 0;
	static const int iFullFirstRaster = 16;
	static const int iFullLastRaster = 299;

	static const int iTVHeight = 270;
	static const int iTVWidth = 376;
	static const int iTVStart = 20;
	static const int iTVFirstRaster = 23;
	static const int iTVLastRaster = 292;
	/*
	static const int iTVHeight = 272;
	static const int iTVWidth = 384;
	static const int iTVStart = 16;
	static const int iTVFirstRaster = 19;
	static const int iTVLastRaster = 290;
	*/

	static const int iSmallHeight = 232;
	static const int iSmallWidth = 352;
	static const int iSmallStart = 32;
	static const int iSmallFirstRaster = 35;
	static const int iSmallLastRaster = 266;


	static const int iNoBorderHeight = 200;
	static const int iNoBorderWidth = 320;
	static const int iNoBorderStart = 48;
	static const int iNoBorderFirstRaster = 51;
	static const int iNoBorderLastRaster = 250;

};

namespace DBGSYM
{
	namespace MachineIdent
	{
		enum MachineIdent
		{
			MainCpu = 0,
			DiskCpu = 1,
			Vic = 2
		};
	};
	namespace SetDisassemblyAddress
	{
		enum DisassemblyPCUpdateMode
		{
			None = 0,
			EnsurePCVisible = 1,
			EnsureAddressVisible = 2,
			SetTopAddress = 3
		};
	};
	namespace BreakpointType
	{
		enum BreakpointType
		{
			Execute = 0,
			Read = 1,
			Write = 2,
			VicRasterCompare = 3,
		};
	};

	namespace CliCommand
	{
		enum CliCommand
		{
			ClearScreen,
			Disassemble,
			Assemble,
			Help,
			ReadMemory,
			WriteMemory,
			Error,
			Unknown,
			SelectCpu,
		};
	};
	namespace CliCommandStatus
	{
		enum CliCommandStatus
		{
			CompletedOK = 0,
			Running,
			Failed,
		};
	};
	namespace CliCpuMode
	{
		enum CliCpuMode
		{
			C64 = 0,
			Disk,
		};
	};
};

#endif
