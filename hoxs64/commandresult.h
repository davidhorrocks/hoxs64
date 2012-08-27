#ifndef __COMMANDRESULT_H__
#define __COMMANDRESULT_H__


#define WM_COMMANDRESULT_COMPLETED (WM_USER + 1)
#define WM_COMMANDRESULT_LINEREADY (WM_USER + 2)


class ICommandResult
{
public:
	virtual HRESULT Start(HWND hWnd)=0;
	virtual HRESULT Stop()=0;
	virtual DWORD WaitComplete(DWORD timeout)=0;
	virtual HRESULT GetNextLine(LPCTSTR *ppszLine)=0;
	virtual void Reset()=0;

};

class CommandResult : public ICommandResult
{
public:
	CommandResult();
	CommandResult(LPCTSTR pText);
	virtual ~CommandResult();
	DBGSYM::CliCommand::CliCommand cmd;
	void AddLine(LPCTSTR pszLine);

	//ICommandResult
	virtual HRESULT Start(HWND hWnd);
	virtual HRESULT Stop();
	virtual DWORD WaitComplete(DWORD timeout);
	virtual void Reset();
	virtual HRESULT GetNextLine(LPCTSTR *ppszLine);
protected:
	virtual void Run()=0;
	static DWORD WINAPI ThreadProc(LPVOID lpThreadParameter);
	void PostComplete(int status);

	size_t line;
	std::vector<LPTSTR> a_lines;
	HWND m_hWnd;
	HANDLE m_hThread;
	HANDLE m_hevtQuit;
	HANDLE m_mux;
	DWORD m_dwThreadId;
	bool m_bUseThread;
private:
	void InitVars();
	void Cleanup();
};

class CommandResultHelp : public CommandResult
{
public:
	CommandResultHelp();
protected:
	virtual void Run();
};

class CommandResultDisassembly : public CommandResult
{
public:
	CommandResultDisassembly(bit16 startaddress, bit16 finishaddress);
protected:
	virtual void Run();

	bit16 address;
	bit16 startaddress;
	bit16 finishaddress;
};


class CommandResultText : public CommandResult
{
public:
	CommandResultText(LPCTSTR pText);
protected:
	virtual void Run();
};

#endif
