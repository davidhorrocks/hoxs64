#ifndef __COMMANDRESULT_H__
#define __COMMANDRESULT_H__


#define WM_COMMANDRESULT_COMPLETED (WM_USER + 1)
#define WM_COMMANDRESULT_LINEREADY (WM_USER + 2)

class CommandResult : public ICommandResult, public std::enable_shared_from_this<CommandResult>
{
public:
	CommandResult(IMonitor *pIMonitor, DBGSYM::CliCpuMode::CliCpuMode cpumode, int iDebuggerMmuIndex, bit16 iDefaultAddress);
	~CommandResult();
	DBGSYM::CliCommand::CliCommand cmd;
	DBGSYM::CliCommandStatus::CliCommandStatus m_status;

	//ICommandResult
	virtual HRESULT Start(HWND hWnd, LPCTSTR pszCommandString, int id);
	virtual HRESULT Quit();
	virtual bool IsQuit();
	virtual DWORD WaitLinesTakenOrQuit(DWORD timeout);
	virtual DWORD WaitAllLinesTakenOrQuit(DWORD timeout);
	virtual DWORD WaitResultDataTakenOrQuit(DWORD timeout);
	virtual DWORD WaitDataReady(DWORD timeout);
	virtual DWORD WaitFinished(DWORD timeout);
	virtual DBGSYM::CliCommandStatus::CliCommandStatus GetStatus();
	virtual void SetStatus(DBGSYM::CliCommandStatus::CliCommandStatus status);
	virtual void SetHwnd(HWND hWnd);
	virtual void Reset();
	virtual void AddLine(LPCTSTR pszLine);
	virtual HRESULT GetNextLine(LPCTSTR *ppszLine);
	virtual void SetDataTaken();
	virtual void SetAllLinesTaken();
	virtual size_t CountUnreadLines();
	virtual int GetId();
	virtual IMonitor* GetMonitor();
	virtual CommandToken* GetToken();
	virtual IRunCommand* GetRunCommand();
protected:
	HRESULT CreateRunCommand(CommandToken *pCommandToken, IRunCommand **ppRunCommand);
	virtual HRESULT Run();
	static DWORD WINAPI ThreadProc(LPVOID lpThreadParameter);
	bool PostFinished();
	size_t line;
	std::vector<LPTSTR> a_lines;
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
	IMonitor *m_pIMonitor;
	DBGSYM::CliCpuMode::CliCpuMode m_cpumode;
	int m_iDebuggerMmuIndex;
	bit16 m_iDefaultAddress;
	CommandToken *m_pCommandToken;
	IRunCommand *m_pIRunCommand;
private:
    CommandResult(CommandResult const &);
    CommandResult & operator=(CommandResult const &);

	std::basic_string<TCHAR> m_sCommandLine;
	bool m_bIsQuit;
	bool m_bIsFinished;
	void InitVars();
	void Cleanup();
	void CleanThreadAllocations();
};

#endif
