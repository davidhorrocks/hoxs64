#include "breakpointresult.h"

BreakpointResult::BreakpointResult()
{
	this->IsBreak = false;
	this->ExitCode = 0;
	this->IsApplicationExitCode = false;
	this->IsMainExecute = false;
	this->IsDiskExecute = false;
	this->IsVicRasterLineAndCycle = false;
}
