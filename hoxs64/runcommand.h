#ifndef __RUNCOMMAND_H__
#define __RUNCOMMAND_H__

class RunCommandHelp : public IRunCommand
{
public:
	RunCommandHelp(ICommandResult *pCommandResult);
protected:
	virtual HRESULT Run();
private:
	ICommandResult *m_pCommandResult;
};

class RunCommandReadMemory : public IRunCommand
{
public:
	RunCommandReadMemory(ICommandResult *pCommandResult, DBGSYM::CliCpuMode::CliCpuMode cpumode, bit16 startaddress, bit16 finishaddress);
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

class RunCommandDisassembly : public IRunCommand
{
public:
	RunCommandDisassembly(ICommandResult *pCommandResult, DBGSYM::CliCpuMode::CliCpuMode cpumode, bit16 startaddress, bit16 finishaddress);
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

class RunCommandAssemble : public IRunCommand
{
public:
	RunCommandAssemble(ICommandResult *pCommandResult, DBGSYM::CliCpuMode::CliCpuMode cpumode, bit16 startaddress, bit8 *data, int dataLength);
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

class RunCommandText : public IRunCommand
{
public:
	RunCommandText(ICommandResult *pCommandResult, LPCTSTR pText);
protected:
	virtual HRESULT Run();
private:
	ICommandResult *m_pCommandResult;
};

#endif
