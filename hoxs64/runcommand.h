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

class RunCommandCpuRegisters : public IRunCommand
{
public:
	RunCommandCpuRegisters(ICommandResult *pCommandResult, DBGSYM::CliCpuMode::CliCpuMode cpumode);
protected:
	virtual HRESULT Run();

	DBGSYM::CliCpuMode::CliCpuMode m_cpumode;
	std::basic_string<TCHAR> m_sLineBuffer;
private:
	ICommandResult *m_pCommandResult;
};

class RunCommandMapMemory : public IRunCommand
{
public:
	bool m_bSetDebuggerToFollowC64Mmu;
	bool m_bViewDebuggerC64Mmu;
	int m_iMmuIndex;
	RunCommandMapMemory(ICommandResult *pCommandResult, int iDebuggerMmuIndex, DBGSYM::CliMapMemory::CliMapMemory map);
protected:
	virtual HRESULT Run();
	std::basic_string<TCHAR> m_sLineBuffer;
private:
	ICommandResult *m_pCommandResult;
	DBGSYM::CliMapMemory::CliMapMemory m_map;
	int m_iDebuggerMmuIndex;
	int GetMmuIndexFromMap(DBGSYM::CliMapMemory::CliMapMemory map);
};

class RunCommandReadMemory : public IRunCommand
{
public:
	RunCommandReadMemory(ICommandResult *pCommandResult, DBGSYM::CliCpuMode::CliCpuMode cpumode, int iDebuggerMmuIndex, bit16 startaddress, bit16 finishaddress);
protected:
	virtual HRESULT Run();

	bit16 m_address;
	DBGSYM::CliCpuMode::CliCpuMode m_cpumode;
	int m_iDebuggerMmuIndex;
	bit16 m_startaddress;
	bit16 m_finishaddress;
	std::basic_string<TCHAR> m_sLineBuffer;
private:
	ICommandResult *m_pCommandResult;
};

class RunCommandDisassembly : public IRunCommand
{
public:
	RunCommandDisassembly(ICommandResult *pCommandResult, DBGSYM::CliCpuMode::CliCpuMode cpumode, int iDebuggerMmuIndex, bit16 startaddress, bit16 finishaddress);
protected:
	virtual HRESULT Run();

	bit16 m_address;
	DBGSYM::CliCpuMode::CliCpuMode m_cpumode;
	int m_iDebuggerMmuIndex;
	bit16 m_startaddress;
	bit16 m_finishaddress;
	std::basic_string<TCHAR> m_sLineBuffer;
private:
	ICommandResult *m_pCommandResult;
};

class RunCommandAssemble : public IRunCommand
{
public:
	RunCommandAssemble(ICommandResult *pCommandResult, DBGSYM::CliCpuMode::CliCpuMode cpumode, int iDebuggerMmuIndex, bit16 startaddress, bit8 *data, int dataLength);
protected:
	virtual HRESULT Run();

	bit16 m_address;
	bit8 *m_data;
	int m_dataLength;
	DBGSYM::CliCpuMode::CliCpuMode m_cpumode;
	int m_iDebuggerMmuIndex;
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
