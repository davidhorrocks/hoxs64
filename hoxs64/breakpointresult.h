#pragma once

struct BreakpointResult
{
	BreakpointResult();

	int ExitCode;

	bool IsBreak;

	bool IsApplicationExitCode;

	bool IsMainExecute;

	bool IsDiskExecute;

	bool IsVicRasterLineAndCycle;
};
