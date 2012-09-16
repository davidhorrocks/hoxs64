#ifndef __COMMANDRESULT_H__
#define __COMMANDRESULT_H__


#define WM_COMMANDRESULT_COMPLETED (WM_USER + 1)
#define WM_COMMANDRESULT_LINEREADY (WM_USER + 2)


class ICommandResult
{
public:
	virtual HRESULT Start(HWND hWnd, LPCTSTR pszCommandString)=0;
	virtual HRESULT Stop()=0;
	virtual bool IsComplete() = 0;
	virtual DWORD WaitComplete(DWORD timeout)=0;
	virtual DBGSYM::CliCommandStatus::CliCommandStatus GetStatus()=0;
	virtual void SetStatus(DBGSYM::CliCommandStatus::CliCommandStatus status)=0;
	virtual HRESULT GetNextLine(LPCTSTR *ppszLine)=0;
	virtual void Reset()=0;
	virtual ~ICommandResult(){};
};


class IRunCommand
{
public:
	virtual HRESULT Run() = 0;
};

class CommandResult : public ICommandResult
{
public:
	CommandResult();
	virtual ~CommandResult();
	DBGSYM::CliCommand::CliCommand cmd;
	DBGSYM::CliCommandStatus::CliCommandStatus m_status;
	void AddLine(LPCTSTR pszLine);

	//ICommandResult
	virtual HRESULT Start(HWND hWnd, LPCTSTR pszCommandString);
	virtual HRESULT Stop();
	virtual bool IsComplete();
	virtual DWORD WaitComplete(DWORD timeout);
	virtual DBGSYM::CliCommandStatus::CliCommandStatus GetStatus();
	virtual void SetStatus(DBGSYM::CliCommandStatus::CliCommandStatus status);
	virtual void Reset();
	virtual HRESULT GetNextLine(LPCTSTR *ppszLine);
protected:
	HRESULT CreateCliCommandResult(CommandToken *pCommandTToken, IRunCommand **ppRunCommand);
	virtual HRESULT Run();
	static DWORD WINAPI ThreadProc(LPVOID lpThreadParameter);
	void PostComplete(int status);
	void SetComplete();
	size_t line;
	std::vector<LPTSTR> a_lines;
	HWND m_hWnd;
	HANDLE m_hThread;
	HANDLE m_hevtQuit;
	HANDLE m_mux;
	DWORD m_dwThreadId;
private:
	std::basic_string<TCHAR> m_sCommandLine;
	bool m_bIsComplete;
	void InitVars();
	void Cleanup();
};

class CommandResultHelp : public IRunCommand
{
public:
	CommandResultHelp(CommandResult *pCommandResult);
protected:
	virtual HRESULT Run();
private:
	CommandResult *m_pCommandResult;
};

class CommandResultDisassembly : public IRunCommand
{
public:
	CommandResultDisassembly(CommandResult *pCommandResult, bit16 startaddress, bit16 finishaddress);
protected:
	virtual HRESULT Run();

	bit16 address;
	bit16 startaddress;
	bit16 finishaddress;
private:
	CommandResult *m_pCommandResult;
};


class CommandResultText : public IRunCommand
{
public:
	CommandResultText(CommandResult *pCommandResult, LPCTSTR pText);
protected:
	virtual HRESULT Run();
private:
	CommandResult *m_pCommandResult;
};

#endif
