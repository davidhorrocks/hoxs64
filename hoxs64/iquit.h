#pragma once

class IQuit
{
public:
	virtual bool IsQuit() = 0;
	virtual void Quit() = 0;
};