#pragma once
#include "boost2005.h"
#include <string>
#include "user_message.h"
#include "defines.h"
#include "mlist.h"
#include "carray.h"
#include "cevent.h"
#include "StringConverter.h"
#include "ErrorLogger.h"
#include "errormsg.h"
#include "bits.h"
#include "util.h"
#include "register.h"
#include "c64keys.h"
#include "hconfig.h"
#include "appstatus.h"
#include "dxstuff9.h"
#include "savestate.h"
#include "cart.h"
#include "c6502.h"
#include "ram64.h"
#include "cpu6510.h"
#include "cia6526.h"
#include "cia1.h"
#include "cia2.h"
#include "vic6569.h"
#include "tap.h"
#include "filter.h"
#include "sid.h"
#include "sidfile.h"
#include "p64.h"
#include "d64.h"
#include "d1541.h"
#include "via6522.h"
#include "via1.h"
#include "via2.h"
#include "diskinterface.h"
#include "t64.h"
#include "c64file.h"
#include "assembler.h"
#include "runcommand.h"
#include "commandresult.h"
#include "monitor.h"

class SIDLoader;

class C64 : public IC64, private ITapeEvent, public IAutoLoad, public ErrorMsg
{
public:	
	C64();
	~C64();
	C64(const C64&) = delete;
	C64& operator=(const C64&) = delete;
	C64(C64&&) = delete;
	C64& operator=(C64&&) = delete;
	
	RAM64 ram;
	CPU6510 cpu;
	VIC6569 vic;
	CIA1 cia1;
	CIA2 cia2;
	SID64 sid;
	Tape64 tape64;
	DiskInterface diskdrive;
	Cart cart;

	static const bit32 NOCOMPRESSION = 0;
	static const bit32 HUFFCOMPRESSION = 1;

	IAppCommand* appCommand = nullptr;
	CAppStatus *appStatus = nullptr;
	IC64Event *pIC64Event = nullptr;
	Graphics* pGx = nullptr;
	CDX9* dx = nullptr;
	bool isInitOK = false;
	Monitor mon;
	HRESULT Init(IAppCommand *, CAppStatus *, IC64Event *, Graphics *, CDX9 *, const wchar_t* pwszAppDirectory);
	void InitReset(ICLK sysclock, bool poweronreset);
	void Reset(ICLK sysclock, bool poweronreset);
	void UpdateKeyMap() override;
	void ExecuteDiskInstruction();
	void ExecuteC64Instruction();
	void ExecuteC64Clock();
	void ExecuteDiskClock();
	bool ExecuteDebugFrame(int cpuId, BreakpointResult &breakpointResult);
	int ExecuteFrame();
	void Warm();
	void EnterDebugRun(bool bWithSound);
	void FinishDebugRun();
	void SetBasicProgramEndAddress(bit16 last_byte);
	HRESULT AutoLoad(const TCHAR *s, int directoryIndex, bool bIndexOnlyPrgFiles, const bit8 c64filename[C64DISKFILENAMELENGTH], bool bQuickLoad, bool bAlignD64Tracks);
	static HRESULT CopyC64FilenameFromString(const TCHAR *sourcestr, bit8 *c64filename, int c64FilenameBufferLength);
	HRESULT LoadCrtFile(const TCHAR *filename);
	HRESULT LoadReu1750();
	HRESULT LoadImageFile(const TCHAR *filename, bit16* pStartAddress, bit16* pSize);
	HRESULT LoadT64ImageFile(const TCHAR *filename, int t64Index, bit16* pStartAddress, bit16* pSize);
	HRESULT LoadTAPFile(const TCHAR *filename);
	HRESULT InsertDiskImageFile(const TCHAR *filename, bool alignD64Tracks, bool immediately);
	HRESULT LoadD64FromFile(const TCHAR *filename, bool alignD64Tracks, bool immediately);
	HRESULT LoadG64FromFile(const TCHAR *filename, bool immediately);
	HRESULT LoadP64FromFile(const TCHAR *filename, bool immediately);
	HRESULT LoadFDIFromFile(const TCHAR *filename, bool immediately);
	HRESULT SaveD64ToFile(const TCHAR *filename, int numberOfTracks);
	HRESULT SaveFDIToFile(const TCHAR *filename);
	HRESULT SaveP64ToFile(const TCHAR *filename);
	HRESULT SaveTrackStateV0(unsigned int trackNumber, bit32 *pTrackBuffer, TP64Image& diskP64Image, unsigned int track_size, unsigned  int *p_gap_count);
	HRESULT LoadTrackStateV0(unsigned int trackNumber, const bit32 *pTrackBuffer, TP64Image& diskP64Image, unsigned int gap_count);
	HRESULT SaveC64StateToFile(const TCHAR *filename);
	HRESULT LoadC64StateFromFile(const TCHAR *filename);

	//IAutoLoad
	void AutoLoadHandler(ICLK sysclock) override;

	//ITapeEvent
	void Pulse(ICLK sysclock) override;
	void EndOfTape(ICLK sysclock) override;
	
	//IC64
	bool IsInitOK() override;
	void HardReset(bool bCancelAutoload) override;
	void SoftReset(bool bCancelAutoload) override;
	void CartFreeze(bool bCancelAutoload) override;
	void CartReset(bool bCancelAutoload) override;
	void PostHardReset(bool bCancelAutoload) override;
	void PostSoftReset(bool bCancelAutoload) override;
	void PostCartFreeze(bool bCancelAutoload) override;
	void ResetKeyboard() override;
	void TapePressPlay() override;
	void TapePressStop() override;
	void TapePressRewind() override;
	void TapePressEject() override;
	HRESULT InsertNewDiskImage(TCHAR *diskname, bit8 id1, bit8 id2, bool bAlignD64Tracks, int numberOfTracks) override;
	void RemoveDisk() override;
	void Set_DiskProtect(bool bOn) override;
	bool Get_DiskProtect() override;
	void DiskReset() override;
	void DetachCart() override;
	bool IsCartAttached() override;
	IMonitor *GetMon() override;
	void SetupColorTables() override;
	HRESULT UpdateBackBuffer() override;
	void SynchroniseDevicesWithVIC() override;
	bool Get_EnableDebugCart() override;
	void Set_EnableDebugCart(bool bEnable) override;
	ICLK Get_LimitCycles() override;
	void Set_LimitCycles(ICLK cycles) override;
	const TCHAR *Get_ExitScreenShot() override;
	void Set_ExitScreenShot(const TCHAR * filename) override;
	int Get_ExitCode() override;
	void Set_ExitCode(int exitCode) override;
	int WriteOnceExitCode(int exitCode) override;
	bool HasExitCode() override;
	void ResetOnceExitCode() override;
	HRESULT SavePng(const TCHAR * filename) override;
	void ClearAllTemporaryBreakpoints() override;
	bool GetDriveLedMotorStatus() override;
	bool GetDriveLedDriveStatus() override;
	bool GetDriveLedWriteStatus() override;
	bit8* GetRomCharPointer() override;

	IMonitorCpu *GetCpu(int cpuid);

	enum AutoLoadType
	{
		AUTOLOAD_NONE,
		AUTOLOAD_TAP_FILE,
		AUTOLOAD_PRG_FILE,
		AUTOLOAD_T64_FILE,
		AUTOLOAD_DISK_FILE,
		AUTOLOAD_SID_FILE,
		AUTOLOAD_CRT_FILE
	};

	enum AutoLoadSequence
	{
		AUTOSEQ_RESET=0,
		AUTOSEQ_LOAD=1,
		AUTOSEQ_RUN=2
	};

	enum C64Cmd
	{
		C64CMD_NONE,
		C64CMD_HARDRESET,
		C64CMD_SOFTRESET,
		C64CMD_CARTFREEZE,
		C64CMD_CARTRESET,
	};

	void PreventClockOverflow();
	void CheckDriveLedNofication();
	static const bit16 BASICSTARTADDRESS = 0x0801;
	static const int SCREENWRITELOCATION = 1024 + 40*6;
	void WriteSysCallToScreen(bit16 startaddress);
protected:
	std::wstring wsAppDirectory;
	struct AutoLoadCommand
	{
		enum AutoLoadType type  = AutoLoadType::AUTOLOAD_NONE;
		enum AutoLoadSequence sequence = AutoLoadSequence::AUTOSEQ_RESET;
		bit16 startaddress = 0;
		bit16 imageSize = 0;
		std::wstring wsfilename;
		bit8 c64filename[C64DISKFILENAMELENGTH] = {};
		int directoryIndex = 0;
		bool bIndexOnlyPrgFiles = false;
		bool bQuickLoad = false;
		bool bAlignD64Tracks = false;
		bit8 *pImageData = nullptr;
		class SIDLoader *pSidFile = nullptr;
		ICLK startclock = 0;

		AutoLoadCommand() noexcept
		{
			startclock = 0;
			pImageData = 0;
			pSidFile = 0;
			CleanUp();
		}

		~AutoLoadCommand()
		{
			CleanUp();
		}

		AutoLoadCommand(const AutoLoadCommand&) = delete;
		AutoLoadCommand& operator=(const AutoLoadCommand&) = delete;
		AutoLoadCommand(AutoLoadCommand&&) = delete;
		AutoLoadCommand& operator=(AutoLoadCommand&&) = delete;

		void CleanUp() noexcept
		{
			try
			{
				if (pImageData)
				{
					GlobalFree(pImageData);
					pImageData = 0;

				}

				if (pSidFile)
				{
					delete(pSidFile);
					pSidFile = 0;
				}

				startclock = 0;
				type = AUTOLOAD_NONE;
				sequence = AUTOSEQ_RESET;
				startaddress = 0;
				imageSize = 0;
				memset(c64filename, 0xA0, sizeof(c64filename));
				wsfilename.clear();
				directoryIndex = 0;
				bIndexOnlyPrgFiles = false;
				bQuickLoad = false;
				bAlignD64Tracks = false;
			}
			catch(...)
			{

			}
		}
	};

	AutoLoadCommand autoLoadCommand;

private:
	void SharedSoftReset();
	void ProcessReset();
	void ExecuteRandomClocks(int minimumClocks, int maximumClocks);
	bool bPendingSystemCommand = false;
	C64Cmd m_SystemCommand = C64Cmd::C64CMD_NONE;
	bool m_bLastPostedDriveWriteLed = false;
	ICLK m_iClockOverflowCheckCounter = 0;
	bool bEnableDebugCart = false;
	ICLK limitCycles = 0;
	std::basic_string<TCHAR> exitScreenShot;
	bool bWantExitScreenShot = false;
	bool bExitCodeWritten = false;
	int exitCode = 0;
	random_device rd;
	mt19937 randengine_main;

	struct StateLoadVars
	{
		SsHeader hdr;
		SsDataChunkHeader chdr;
		SsSectionHeader sh;
		SsCpuMainV0 sbCpuMainV0;
		SsCpuMainV1 sbCpuMainV1;
		SsCia1V0 sbCia1V0;
		SsCia1V1 sbCia1V1;
		SsCia1V2 sbCia1V2;
		SsCia1V2 sbCia1V3;
		SsCia2V0 sbCia2V0;
		SsCia2V1 sbCia2V1;
		SsCia2V2 sbCia2V2;
		SsCia2V2 sbCia2V3;
		SsVic6569V0 sbVic6569V0;
		SsVic6569V1 sbVic6569V1;
		SsSid sbSidV0;
		SsSidV1 sbSidV1;
		SsSidV2 sbSidV2;
		SsSidV3 sbSidV3;
		SsSidV4 sbSidV4Number1;
		SsSidV4 sbSidV4Number2;
		SsSidV4 sbSidV4Number3;
		SsSidV4 sbSidV4Number4;
		SsSidV4 sbSidV4Number5;
		SsSidV4 sbSidV4Number6;
		SsSidV4 sbSidV4Number7;
		SsSidV4 sbSidV4Number8;
		SsTape sbTapePlayer;
		SsTapeData sbTapeDataHeader;
		SsDiskInterfaceV0 sbDriveControllerV0;
		SsDiskInterfaceV1 sbDriveControllerV1;
		SsDiskInterfaceV2 sbDriveControllerV2;
		SsVia1 sbDriveVia1;
		SsVia2 sbDriveVia2;
		SsCpuDisk sbCpuDisk;
		SsTrackHeader trackHeader;
	};

	struct StateSaveVars
	{
		SsSectionHeader sh;
		SsDataChunkHeader chdr;
		SsHeader hdr;
		SsCpuMainV1 sbCpuMain;
		SsCia1V2 sbCia1;
		SsCia2V2 sbCia2;
		SsVic6569V1 sbVic6569;
		SsSidV4 sbSid1;
		SsSidV4 sbSid2;
		SsSidV4 sbSid3;
		SsSidV4 sbSid4;
		SsSidV4 sbSid5;
		SsSidV4 sbSid6;
		SsSidV4 sbSid7;
		SsSidV4 sbSid8;
		SsTape sbTapePlayer;
		SsTapeData tapeDataHeader;
		SsCpuDisk sbCpuDisk;
		SsDiskInterfaceV2 sbDiskInterfaceV2;
		SsVia1 sbVia1;
		SsVia2 sbVia2;
		SsTrackHeader th;

	};
};
