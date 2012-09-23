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
	CommandResultDisassembly(ICommandResult *pCommandResult, DBGSYM::CliCpuMode::CliCpuMode cpumode, bit16 startaddress, bit16 finishaddress);
protected:
	virtual HRESULT Run();

	bit16 m_address;
	DBGSYM::CliCpuMode::CliCpuMode m_cpumode;
	bit16 m_startaddress;
	bit16 m_finishaddress;
	std::basic_string<TCHAR> m_sLineBuffer;
private:
	ICommandResult *m_pCommandResult;
};

class CommandResultAssemble : public IRunCommand
{
public:
	CommandResultAssemble(ICommandResult *pCommandResult, DBGSYM::CliCpuMode::CliCpuMode cpumode, bit16 startaddress, bit8 *data, int dataLength);
protected:
	virtual HRESULT Run();

	bit16 m_address;
	bit8 *m_data;
	int m_dataLength;
	DBGSYM::CliCpuMode::CliCpuMode m_cpumode;
	bit16 m_startaddress;
	std::basic_string<TCHAR> m_sLineBuffer;
private:
	ICommandResult *m_pCommandResult;

	void WriteBytesToMemory(bit16 startaddress, bit8 *buffer, int dataLength);
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
