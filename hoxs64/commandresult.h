#pragma once

#define WM_COMMANDRESULT_COMPLETED (WM_USER + 1)
#define WM_COMMANDRESULT_LINEREADY (WM_USER + 2)

class CommandResult : public ICommandResult, public enable_shared_from_this<CommandResult>
{
public:
	CommandResult(IMonitor* pIMonitor, DBGSYM::CliCpuMode::CliCpuMode cpumode, int iDebuggerMmuIndex, bit16 iDefaultAddress);
	virtual ~CommandResult();
	DBGSYM::CliCommand::CliCommand cmd;
	DBGSYM::CliCommandStatus::CliCommandStatus m_status;
	bool WantTrace = false;
	bit64 StepCount = 0;

	//ICommandResult
	HRESULT Start(HWND hWnd, LPCTSTR pszCommandString, int id) override;
	HRESULT Quit() override;
	bool IsQuit() override;
	DWORD WaitLinesTakenOrQuit(DWORD timeout) override;
	DWORD WaitAllLinesTakenOrQuit(DWORD timeout) override;
	DWORD WaitResultDataTakenOrQuit(DWORD timeout) override;
	DWORD WaitDataReady(DWORD timeout) override;
	DWORD WaitFinished(DWORD timeout) override;
	DBGSYM::CliCommandStatus::CliCommandStatus GetStatus() override;
	void SetStatus(DBGSYM::CliCommandStatus::CliCommandStatus status) override;
	void SetHwnd(HWND hWnd) override;
	void Reset() override;
	void AddLine(LPCTSTR pszLine)override;
	HRESULT GetNextLine(LPCTSTR* ppszLine)override;
	void SetDataTaken() override;
	void SetAllLinesTaken() override;
	size_t CountUnreadLines()override;
	int GetId()override;
	IMonitor* GetMonitor()override;
	CommandToken* GetToken()override;
	IRunCommand* GetRunCommand()override;
	void SetTraceStepCount(const TraceStepInfo& stepInfo) override;
	bool GetTraceStepCount(TraceStepInfo& stepInfo) override;

protected:
	HRESULT CreateRunCommand(CommandToken* pCommandToken, IRunCommand** ppRunCommand);
	virtual HRESULT Run();
	static DWORD WINAPI ThreadProc(LPVOID lpThreadParameter);
	bool PostFinished();
	size_t line;
	vector<LPTSTR> a_lines;
	HWND m_hWnd;
	HANDLE m_hThread;
	HANDLE m_hevtQuit;
	HANDLE m_hevtLineTaken;
	HANDLE m_hevtAllLinesTaken;
	HANDLE m_hevtResultDataReady;
	HANDLE m_hevtResultDataTaken;
	HANDLE m_mux;
	DWORD m_dwThreadId;
	int m_id;
	IMonitor* m_pIMonitor;
	DBGSYM::CliCpuMode::CliCpuMode m_cpumode;
	int m_iDebuggerMmuIndex;
	bit16 m_iDefaultAddress;
	CommandToken* m_pCommandToken;
	IRunCommand* m_pIRunCommand;
private:
	CommandResult(CommandResult const&);
	CommandResult& operator=(CommandResult const&);

	void InitVars();
	void Cleanup();
	void CleanThreadAllocations();
	std::basic_string<TCHAR> m_sCommandLine;
	bool m_bIsQuit;
	bool m_bIsFinished;
	TraceStepInfo traceStepResult;
};
