#ifndef __COMMANDRESULT_H__
#define __COMMANDRESULT_H__


#define WM_COMMANDRESULT_COMPLETED (WM_USER + 1)
#define WM_COMMANDRESULT_LINEREADY (WM_USER + 2)

class CommandResult : public ICommandResult
{
public:
	CommandResult(IMonitor *pIMonitor, DBGSYM::CliCpuMode::CliCpuMode cpumode);
	~CommandResult();
	DBGSYM::CliCommand::CliCommand cmd;
	DBGSYM::CliCommandStatus::CliCommandStatus m_status;

	//ICommandResult
	virtual HRESULT Start(HWND hWnd, LPCTSTR pszCommandString, int id);
	virtual HRESULT Stop();
	virtual bool IsComplete();
	virtual DWORD WaitComplete(DWORD timeout);
	virtual DBGSYM::CliCommandStatus::CliCommandStatus GetStatus();
	virtual void SetStatus(DBGSYM::CliCommandStatus::CliCommandStatus status);
	virtual void Reset();
	virtual void AddLine(LPCTSTR pszLine);
	virtual HRESULT GetNextLine(LPCTSTR *ppszLine);
	virtual int GetId();
	virtual IMonitor* GetMonitor();
	virtual CommandToken* GetToken();
protected:
	HRESULT CreateCliCommandResult(CommandToken *pCommandToken, IRunCommand **ppRunCommand);
	virtual HRESULT Run();
	static DWORD WINAPI ThreadProc(LPVOID lpThreadParameter);
	void PostComplete();
	void SetComplete();
	size_t line;
	std::vector<LPTSTR> a_lines;
	HWND m_hWnd;
	HANDLE m_hThread;
	HANDLE m_hevtQuit;
	HANDLE m_mux;
	DWORD m_dwThreadId;
	int m_id;
	IMonitor *m_pIMonitor;
	DBGSYM::CliCpuMode::CliCpuMode m_cpumode;
	CommandToken *m_pCommandToken;
private:
    CommandResult(CommandResult const &);
    CommandResult & operator=(CommandResult const &);

	std::basic_string<TCHAR> m_sCommandLine;
	bool m_bIsComplete;
	void InitVars();
	void Cleanup();
};

#endif
