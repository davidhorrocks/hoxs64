#ifndef __RUNCOMMAND_H__
#define __RUNCOMMAND_H__

class CommandResultHelp : public IRunCommand
{
public:
	CommandResultHelp(ICommandResult *pCommandResult);
protected:
	virtual HRESULT Run();
private:
	ICommandResult *m_pCommandResult;
};

class CommandResultDisassembly : public IRunCommand
{
public:
	CommandResultDisassembly(ICommandResult *pCommandResult, bit16 startaddress, bit16 finishaddress);
protected:
	virtual HRESULT Run();

	bit16 m_address;
	bit16 m_startaddress;
	bit16 m_finishaddress;
	std::basic_string<TCHAR> m_sLineBuffer;
private:
	ICommandResult *m_pCommandResult;
};


class CommandResultText : public IRunCommand
{
public:
	CommandResultText(ICommandResult *pCommandResult, LPCTSTR pText);
protected:
	virtual HRESULT Run();
private:
	ICommandResult *m_pCommandResult;
};

#endif
