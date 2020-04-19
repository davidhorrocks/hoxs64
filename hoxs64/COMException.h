#pragma once
#include <comdef.h>
#include "StringConverter.h"

#ifdef DEBUG
#define COM_ERROR_IF_FAILED( hr, msg ) if( FAILED( hr ) ) throw COMException( hr, msg, __FILE__, __FUNCTION__, __LINE__ )
#else
#define COM_ERROR_IF_FAILED( hr, msg ) if( FAILED( hr ) ) throw COMException( hr, msg, "", "", 0)
#endif

class COMException
{
public:
	COMException(HRESULT hr, const std::string& msg, const std::string& file, const std::string& function, int line)
	{		
		this->hr = hr;
		_com_error error(hr);
		whatmsg = L"Msg: " + StringConverter::StringToWideString(std::string(msg)) + L"\n";
		whatmsg += error.ErrorMessage();
#ifdef DEBUG
		whatmsg += L"\nFile: " + StringConverter::StringToWideString(file);
		whatmsg += L"\nFunction: " + StringConverter::StringToWideString(function);
		whatmsg += L"\nLine: " + StringConverter::StringToWideString(std::to_string(line));
#endif
	}

	const wchar_t* what() const
	{
		return whatmsg.c_str();
	}

	HRESULT HResult() const
	{
		return hr;
	}
private:
	std::wstring whatmsg;
	HRESULT hr;
};