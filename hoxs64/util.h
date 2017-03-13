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
		EMUWINSTR_ASPECTSTRETCH = 3,
		EMUWINSTR_ASPECTSTRETCHBORDERCLIP = 4
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
		EMUBORDER_SMALL = 2,
		EMUBORDER_NOSIDE = 3,
		EMUBORDER_NOTOP = 4,
		EMUBORDER_NOBORDER = 5
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
		CM_CIA6526A=1
	} CIAMODE;

	typedef enum tagJoyObjectKind
	{
		JoyKindNone=0,
		JoyKindAxis=1,
		JoyKindPov=2,
		JoyKindButton=3
	} JOYOBJECTKIND;
};

#define ALLOW_EMUFPS_50_12_MULTI

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
	void SetBorder2(int screenWidth, int screenHeight, int toolbarHeight);

	static const int WDToolbarHeight = 10;

	static const int WDFullHeight = HEIGHT_64;
	static const int WDFullWidth = WIDTH_64;
	static const int WDFullStart = 0;
	static const int WDFullFirstRaster = 16;
	static const int WDFullLastRaster = 299;

	static const int WDTVHeight = 270;
	static const int WDTVWidth = 376;
	static const int WDTVStart = 20;
	static const int WDTVFirstRaster = 23;
	static const int WDTVLastRaster = 292;

	static const int WDSmallHeight = 232;
	static const int WDSmallWidth = 352;
	static const int WDSmallStart = 32;
	static const int WDSmallFirstRaster = 35;
	static const int WDSmallLastRaster = 266;


	static const int WDNoBorderHeight = 200;
	static const int WDNoBorderWidth = 320;
	static const int WDNoBorderStart = 48;
	static const int WDNoBorderFirstRaster = 51;
	static const int WDNoBorderLastRaster = 250;

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
			SelectCpu,
			MapMemory,
			ShowCpu,
			ShowCpu64,
			ShowCpuDisk,
			ShowVic,
			ShowCia1,
			ShowCia2,
			ShowSid,
			ShowVia1,
			ShowVia2,
			Error,
			Unknown,
		};
	};
	namespace CliCommandStatus
	{
		enum CliCommandStatus
		{
			NotStarted,
			Running,
			CompletedOK,
			Failed,
			Finished
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
	namespace CliMapMemory
	{
		enum CliMapMemory : int
		{
			VIEWCURRENT = 0,
			BASIC = 1,
			KERNAL = 2,
			IO = 4,
			CHARGEN = 8,
			ROML = 16,
			ROMH = 32,
			RAM = 64,
			SETCURRENT = 128,
			_ALL = -1,
		};
	};
};

#endif
