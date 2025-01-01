#pragma once
#include <string>
#include "errormsg.h"
#include "buffersize.h"

class CApp : CAppStatus, IAppCommand, IC64Event, CAppWindow::INotify, public ErrorMsg
{
public:
	CApp() noexcept(false);
	~CApp();
	CApp(const CApp&) = delete;
	CApp& operator=(const CApp&) = delete;
	CApp(CApp&&) = delete;
	CApp& operator=(CApp&&) = delete;

	void* operator new(size_t i);
	void operator delete(void* p);

	static const int ShellExitCycleLimit = 0x1;
	static const int ShellExitInitFailed = 0x101;
	static const int ShellExitPngFailed = 0x103;
	static constexpr size_t CchPathBufferSize = MAX_SIZE_FILE_PATH;

	void Cleanup() noexcept;
	int Run(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow);
	void CloseAppWindow();
	HRESULT RegisterWindowClasses(HINSTANCE hInstance);
	HRESULT InitInstance(int nCmdShow, PWSTR lpCmdLine);
	HRESULT RegisterKeyPressWindow(HINSTANCE hInstance);
	HRESULT RegisterVicColorWindow(HINSTANCE hInstance);
	void AllowAccessibilityShortcutKeys( bool bAllowKeys );

	// IAppCommand
	void IAppCommand::FreeDirectX() noexcept override;
	void IAppCommand::SoundHalt() override;
	void IAppCommand::SoundResume() override;
	const wchar_t* IAppCommand::GetAppTitle() override;
	const wchar_t* IAppCommand::GetAppName() override;
	VS_FIXEDFILEINFO * IAppCommand::GetVersionInfo() override;
	void IAppCommand::SetBusyApp(bool isBusy) override;
	void IAppCommand::TogglePause() override;
	void IAppCommand::ToggleSoundMute() override;
	void IAppCommand::ToggleMaxSpeed() override;
	void IAppCommand::InsertTapeDialog(HWND hWnd) override;
	void IAppCommand::LoadCrtFileDialog(HWND hWnd) override;
	void IAppCommand::InsertReuFileDialog(HWND hWnd) override;
	void IAppCommand::LoadReu1750(HWND hWnd, unsigned int extraAddressBits) override;
	void IAppCommand::LoadReu1750FromFile(HWND hWnd, const TCHAR* filename, bool autoSizeExtraAddressBits, unsigned int extraAddressBits) override;
	void IAppCommand::LoadC64ImageDialog(HWND hWnd) override;
	void IAppCommand::LoadT64Dialog(HWND hWnd) override;
	void IAppCommand::AutoLoadDialog(HWND hWnd) override;
	void IAppCommand::InsertDiskImageDialog(HWND hWnd) override;
	void IAppCommand::SaveD64ImageDialog(HWND hWnd) override;
	void IAppCommand::SaveFDIImageDialog(HWND hWnd) override;
	void IAppCommand::SaveP64ImageDialog(HWND hWnd) override;
	void IAppCommand::SaveC64StateDialog(HWND hWnd) override;
	void IAppCommand::LoadC64StateDialog(HWND hWnd) override;
	void IAppCommand::RestoreUserSettings() override;
	void IAppCommand::RestoreDefaultSettings() override;
	bool IAppCommand::RestoreSettingsFromFileDialog(HWND hWnd) override;
	void IAppCommand::RestoreSettingsFromRegistry() override;
	void IAppCommand::SaveCurrentSettings() override;
	bool IAppCommand::SaveCurrentSettingsToFileDialog(HWND hWnd) override;
	void IAppCommand::SaveCurrentSettingsToRegistry() override;
	void IAppCommand::GetUserConfig(CConfig& cfg) override;
	void IAppCommand::SetUserConfig(const CConfig& newcfg) override;
	void IAppCommand::ApplyConfig(const CConfig& newcfg) override;
	void IAppCommand::SetSidChipAddressMap(int numberOfExtraSidChips, bit16 addressOfSecondSID, bit16 addressOfThirdSID, bit16 addressOfFourthSID, bit16 addressOfFifthSID, bit16 addressOfSixthSID, bit16 addressOfSeventhSID, bit16 addressOfEighthSID) override;
	void IAppCommand::ResetSidChipAddressMap() override;
	void IAppCommand::UpdateUserConfigFromSid() override;
	
	// IAppCommand
	void IAppCommand::Resume() override;
	void IAppCommand::Trace(int cpuId) override;
	void IAppCommand::TraceWithTemporaryBreakpoints(int cpuId) override;
	void IAppCommand::TraceFrame(int cpuId) override;
	void IAppCommand::TraceSystemClocks(const TraceStepInfo& traceStepInfo) override;
	void IAppCommand::TraceStepOver(int cpuId) override;
	void IAppCommand::TraceStepOut(int cpuId, bool requireRtsRti) override;
	void IAppCommand::ClearAllTemporaryBreakpoints() override;
	void IAppCommand::ExecuteC64Clock() override;
	void IAppCommand::ExecuteDiskClock() override;
	void IAppCommand::ExecuteC64Instruction() override;
	void IAppCommand::ExecuteDiskInstruction() override;
	void IAppCommand::UpdateApplication() override;
	void IAppCommand::UpdateEmulationDisplay() override;
	HWND IAppCommand::ShowDevelopment() override;
	bool IAppCommand::IsRunning() override;
	void IAppCommand::SoundOff() override;
	void IAppCommand::SoundOn() override;
	void IAppCommand::ShowCpuDisassembly(int cpuid, DBGSYM::SetDisassemblyAddress::DisassemblyPCUpdateMode pcmode, bit16 address) override;
	HWND IAppCommand::GetMainFrameWindow() override;
	void IAppCommand::DisplayVicCursor(bool bEnabled) override;
	void IAppCommand::DisplayVicRasterBreakpoints(bool bEnabled) override;
	void IAppCommand::SetVicCursorPos(int iCycle, int iLine) override;
	void IAppCommand::GetVicCursorPos(int *piCycle, int *piLine) override;
	void IAppCommand::SetRadixHexadecimal() override;
	void IAppCommand::SetRadixDecimal() override;
	void IAppCommand::GetLastMousePosition(int* x, int* y) override;
	void IAppCommand::SetLastMousePosition(const int* x, const int* y) override;
	bool IAppCommand::GetIsMouseOverClientArea() override;
	void IAppCommand::SetIsMouseOverClientArea(bool isMouseOver) override;
	void IAppCommand::PostCloseMainWindow() override;
	void IAppCommand::PostToggleFullscreen() override;
	bool IAppCommand::PostAutoLoadFile(const wchar_t* pszFilename, int directoryIndex, bool quickload, bool prgAlwaysQuickload, bool reu) override;
	void IAppCommand::PostStartTrace(const TraceStepInfo& traceStepInfo) override;
	void IAppCommand::DeleteOneWaitingWinProcMessage(LPARAM lparam) override;
	bool IAppCommand::InsertDiskImageFromFile(const wchar_t* pszFilename) override;
	bool IAppCommand::InsertTapeImageFromFile(const wchar_t* pszFilename) override;
	void IAppCommand::EnableC64Input(bool enabled) override;
	// IAppCommand
	
	shared_ptr<CAppWindow> m_pWinAppWindow;	

	//IC64Event
	void BreakExecuteCpu64() override;
	void BreakExecuteCpuDisk() override;
	void BreakVicRasterCompare() override;
	void BreakpointChanged() override;
	void Reset() override;
	void MemoryChanged() override;
	void RadixChanged(DBGSYM::MonitorOption::Radix radix) override;
	void SetBusy(bool bBusy) override;
	void DiskMotorLed(bool bOn) override;
	void DiskDriveLed(bool bOn) override;
	void DiskWriteLed(bool bOn) override;
	void ShowErrorBox(LPCTSTR title, LPCTSTR message) override;
	void WriteExitCode(int exitCode) override;

	//CAppWindow::INotify
	void VicCursorMove(int cycle, int line) override;

	void UpdateConfigFromSid(CConfig& cfg);
	void CreateImGuiContext();
	void CloseImGuiContext() noexcept;
	void CleanWaitingWinProcMessages();

	// mainCfg is a CConfig that holds the user preferred settings.
	// CApp derives from CConfig and holds the transient in-play setting. The transient in-play setting in (CConfig)CApp can differ from the user preferred setting in CApp::mainCfg
	// such as when quick-speed is toggled on. Quick-speed keeps the user preferred settings intact and makes a change to the temporary in play settings.
	CConfig mainCfg;
	bool isImGuiStarted = false;
	C64 c64;
	EXECUTION_STATE m_Prevstate = {};
	STICKYKEYS m_StartupStickyKeys = {};
	TOGGLEKEYS m_StartupToggleKeys = {};
	FILTERKEYS m_StartupFilterKeys = {};
	bool m_bBusy = false;
	HCURSOR hCursorBusy = 0;
	HCURSOR hOldCursor = 0;
	HACCEL m_hAccelTable = 0;
	HINSTANCE m_hInstance = 0;
	std::vector<LPARAM> waitingWinProcMessages;
	bool m_bStartFullScreen = false;
	CDX9 dx;
	Graphics gx;
private:
	std::wstring wsAppName;  // Name of the app
	std::wstring wsTitle;    // The title bar text
	std::wstring wsMonitorTitle;    // The title bar text
	std::wstring wsMainWndClsName;  // Main window class
	std::wstring wsMonitorWndClsName;  // Monitor window class
	std::wstring wsMonitorDisassembleWndClsName;  //Disassemble child window class 
	std::wstring wsAppFullPath;
	std::wstring wsAppDirectory;
	POINT GetCenteredPos(int w, int h);
	int lastMouseX = 0;
	int lastMouseY = 0;
	bool isMouseOverClientArea = false;	
	random_device rd;
	mt19937 randengine;
};
