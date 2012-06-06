#ifndef __COMMANDRESULT_H__
#define __COMMANDRESULT_H__

class CommandResult
{
public:
	CommandResult();
	CommandResult(LPCTSTR pszLine);
	virtual ~CommandResult();
	DBGSYM::CliCommand::CliCommand cmd;
	void AddLine(LPCTSTR pszLine);
	virtual void Reset();
	virtual HRESULT GetNextLine(LPCTSTR *ppszLine);
protected:
	size_t line;
	std::vector<LPTSTR> a_lines;
};


class CommandResultHelp : public CommandResult
{
public:
	CommandResultHelp();
};

class CommandResultDisassembly : public CommandResult
{
public:
	CommandResultDisassembly(bit16 startaddress, bit16 finishaddress);
	virtual HRESULT GetNextLine(LPCTSTR *ppszLine);
protected:
	bit16 address;
	bit16 startaddress;
	bit16 finishaddress;
};

#endif
