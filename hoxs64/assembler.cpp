#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include "bits.h"
#include "assert.h"
#include "mlist.h"
#include "carray.h"

#include "defines.h"
#include "bits.h"
#include "util.h"
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "register.h"

#include "c6502.h"
#include "assembler.h"

AssemblyToken::AssemblyToken()
{
	TokenType = AssemblyToken::EndOfInput;
	IdentifierText[0] = 0;
	Value8=0;
	Value16=0;
	SymbolChar = 0;
}

void AssemblyToken::SetEOF(AssemblyToken* t)
{
	*t = AssemblyToken();
	t->TokenType =  AssemblyToken::EndOfInput;
}

void AssemblyToken::SetError(AssemblyToken* t)
{
	*t = AssemblyToken();
	t->TokenType =  AssemblyToken::Error;
}

bool Assembler::AppendToIdentifierString(TCHAR ch)
{
	if (m_ibufPos + 1 >= _countof(m_bufIdentifierString))
		return false;
	m_bufIdentifierString[m_ibufPos] = ch;
	m_ibufPos++;
	m_bufIdentifierString[m_ibufPos] = 0;
	return true;
}

HRESULT Assembler::AssembleText(bit16 address, LPCTSTR pszText, bit8 *pCode, int iBuffersize, int *piBytesWritten)
{
HRESULT hr;
	m_pos = 0;
	m_bIsStartChar = true;
	m_bIsStartToken = true;
	m_bCurrentEOF = true;
	m_bNextEOF = true;

	m_iLenText = lstrlen(pszText);
	if (m_iLenText == 0)
		return E_FAIL;
	m_pszText = pszText;
	GetNextChar();
	GetNextChar();

	GetNextToken();
	GetNextToken();

	hr = AssembleOneInstruction(address, pCode, iBuffersize, piBytesWritten);
	if (FAILED(hr))
		return hr;

	if (m_CurrentToken.TokenType != AssemblyToken::EndOfInput)
		return E_FAIL;

	return S_OK;
}

HRESULT Assembler::AssembleOneInstruction(bit16 address, bit8 *pCode, int iBuffersize, int *piBytesWritten)
{
HRESULT hr;
AssemblyToken tk;
bit8 num8;
bit16 num16;
unsigned int iValue = 0;
	if (m_CurrentToken.TokenType == AssemblyToken::EndOfInput)
	{
		return E_FAIL;
	}
	else if (m_CurrentToken.TokenType == AssemblyToken::Number8)
	{
		bit8 v[256];
		int i;
		for (i=0; i <_countof(v) && m_CurrentToken.TokenType == AssemblyToken::Number8; i++)
		{
			v[i] = m_CurrentToken.Value8;
			GetNextToken();
		}
		int w = instcopy(pCode, iBuffersize, v, i);
		if (piBytesWritten!=NULL)
			*piBytesWritten = w;
		return S_OK;
	}
	else if (m_CurrentToken.TokenType == AssemblyToken::IdentifierString)
	{
		tk = m_CurrentToken;
		GetNextToken();
		if (m_CurrentToken.TokenType == AssemblyToken::EndOfInput)
		{
			//Implied addressing.
			hr = AssembleImplied(tk.IdentifierText, pCode, iBuffersize,  piBytesWritten);
			return hr;
		}
		else if (m_CurrentToken.TokenType == AssemblyToken::Number8)
		{
			num8 = m_CurrentToken.Value8;
			GetNextToken();
			if (m_CurrentToken.TokenType == AssemblyToken::EndOfInput)
			{
				//$00
				hr = AssembleZeroPage(tk.IdentifierText, num8, pCode, iBuffersize, piBytesWritten);
				if (SUCCEEDED(hr))
					return S_OK;
				hr = AssembleAbsolute(tk.IdentifierText, (bit16) num8, pCode, iBuffersize, piBytesWritten);
				return hr;
			}
			else if (m_CurrentToken.TokenType == AssemblyToken::Symbol)
			{
				if (m_CurrentToken.SymbolChar == _T(','))
				{
					GetNextToken();
					if (m_CurrentToken.TokenType == AssemblyToken::IdentifierString)
					{
						if (_tcsicmp(m_CurrentToken.IdentifierText, TEXT("X")) == 0)
						{
							GetNextToken();
							//$00,X
							hr = AssembleZeroPageX(tk.IdentifierText, num8, pCode, iBuffersize, piBytesWritten);
							if (SUCCEEDED(hr))
								return S_OK;
							hr = AssembleAbsoluteX(tk.IdentifierText, (bit16) num8, pCode, iBuffersize, piBytesWritten);
							return hr;
						}
						else if (_tcsicmp(m_CurrentToken.IdentifierText, TEXT("Y")) == 0)
						{
							GetNextToken();
							//$00,Y
							hr = AssembleZeroPageY(tk.IdentifierText, num8, pCode, iBuffersize, piBytesWritten);
							if (SUCCEEDED(hr))
								return S_OK;
							hr = AssembleAbsoluteY(tk.IdentifierText, (bit16) num8, pCode, iBuffersize, piBytesWritten);
							return hr;
						}
					}
				}
			}
		}
		else if (m_CurrentToken.TokenType == AssemblyToken::Number16)
		{
			num16 = m_CurrentToken.Value16;
			GetNextToken();
			if (m_CurrentToken.TokenType == AssemblyToken::EndOfInput)
			{
				//$0000
				hr = AssembleAbsolute(tk.IdentifierText, num16, pCode, iBuffersize, piBytesWritten);
				if (SUCCEEDED(hr))
					return hr;
				if (num16 <= 0xff)
				{
					hr = AssembleZeroPage(tk.IdentifierText, (bit8)(num16 & 0xff), pCode, iBuffersize, piBytesWritten);
					if (SUCCEEDED(hr))
						return hr;
				}
				int diff = ((int)num16) - ((int)address+2);
				if (diff <= 0x7f && diff >= -0x80)
				{
					hr = AssembleRelative(tk.IdentifierText, (bit8)(diff & 0xff), pCode, iBuffersize, piBytesWritten);
					if (SUCCEEDED(hr))
						return hr;
				}
			}
			else if (m_CurrentToken.TokenType == AssemblyToken::Symbol)
			{
				if (m_CurrentToken.SymbolChar == _T(','))
				{
					GetNextToken();
					if (m_CurrentToken.TokenType == AssemblyToken::IdentifierString)
					{
						if (_tcsicmp(m_CurrentToken.IdentifierText, TEXT("X")) == 0)
						{
							GetNextToken();
							//$0000,X
							hr = AssembleAbsoluteX(tk.IdentifierText, num16, pCode, iBuffersize, piBytesWritten);
							return hr;
						}
						else if (_tcsicmp(m_CurrentToken.IdentifierText, TEXT("Y")) == 0)
						{
							GetNextToken();
							//$0000,Y
							hr = AssembleAbsoluteY(tk.IdentifierText, num16, pCode, iBuffersize, piBytesWritten);
							return hr;
						}
					}
				}
			}
		}
		else if (m_CurrentToken.TokenType == AssemblyToken::Symbol)
		{
			if (m_CurrentToken.SymbolChar == _T('#'))
			{
				//Immediate addressing.
				GetNextToken();
				if (m_CurrentToken.TokenType == AssemblyToken::Number8 || m_CurrentToken.TokenType == AssemblyToken::Number16)
				{
					if (m_CurrentToken.TokenType == AssemblyToken::Number8)
						iValue = m_CurrentToken.Value8;
					else
						iValue = m_CurrentToken.Value16;
					GetNextToken();
					if (iValue <= 0xff)
					{
						//#$00
						hr = AssembleImmediate(tk.IdentifierText, (bit8)(iValue & 0xff), pCode, iBuffersize, piBytesWritten);
						if (SUCCEEDED(hr))
							return S_OK;
						hr = AssembleRelative(tk.IdentifierText, (bit8)(iValue & 0xff), pCode, iBuffersize, piBytesWritten);
						if (SUCCEEDED(hr))
							return S_OK;
					}
				}
			}
			else if (m_CurrentToken.SymbolChar == _T('('))
			{
				GetNextToken();
				if (m_CurrentToken.TokenType == AssemblyToken::Number8 || (m_CurrentToken.TokenType == AssemblyToken::Number16))
				{
					if (m_CurrentToken.TokenType == AssemblyToken::Number8)
						iValue = m_CurrentToken.Value8;
					else
						iValue = m_CurrentToken.Value16;
					GetNextToken();
					if (m_CurrentToken.TokenType == AssemblyToken::Symbol)
					{
						if (m_CurrentToken.SymbolChar == _T(','))
						{
							GetNextToken();
							if (m_CurrentToken.TokenType == AssemblyToken::IdentifierString)
							{
								if (_tcsicmp(m_CurrentToken.IdentifierText, TEXT("X")) == 0)
								{
									GetNextToken();
									if (m_CurrentToken.SymbolChar == _T(')'))
									{
										GetNextToken();
										//($00,X)
										if (iValue <= 0xff)
										{
											hr = AssembleIndirectX(tk.IdentifierText, (bit8)(iValue & 0xff), pCode, iBuffersize, piBytesWritten);
											return hr;
										}
									}
								}
							}
						}
						else if (m_CurrentToken.SymbolChar == _T(')'))
						{
							GetNextToken();
							//($00)...
							if (m_CurrentToken.SymbolChar == _T(','))
							{
								GetNextToken();
								if (m_CurrentToken.TokenType == AssemblyToken::IdentifierString)
								{
									if (_tcsicmp(m_CurrentToken.IdentifierText, TEXT("Y")) == 0)
									{
										GetNextToken();
										//($00),Y
										if (iValue <= 0xff)
										{
											hr = AssembleIndirectY(tk.IdentifierText, (bit8)(iValue & 0xff), pCode, iBuffersize, piBytesWritten);
											return hr;
										}
									}
								}
							}
							else
							{
								//($00)->($0000)
								hr = AssembleIndirect(tk.IdentifierText, (bit16)iValue, pCode, iBuffersize, piBytesWritten);
								return hr;
							}
						}
					}
				}
			}
		}
	}
	return E_FAIL;
}

int Assembler::instcopy(bit8 *pDest, int iSizeDest, bit8 *pSrc, int iSizeSrc)
{
int i;
	if (pDest==NULL || pSrc==NULL)
		return iSizeSrc;
	for (i = 0; i < iSizeSrc && i < iSizeDest; i++)
	{
		pDest[i] = pSrc[i];
	}
	return i;
}

HRESULT Assembler::AssembleImplied(LPCTSTR pszMnemonic, bit8 *pCode, int iBuffersize, int *piBytesWritten)
{
bit8 v[1];

	for(int i=0; i <= 0xff; i++)
	{
		const InstructionInfo& inf = CPU6502::AssemblyData[i];
		if (inf.address_mode == amIMPLIED && _tcsicmp(pszMnemonic, CPU6502::AssemblyData[i].mnemonic) == 0)
		{
			v[0] = inf.opcode;
			int w = instcopy(pCode, iBuffersize, v, _countof(v));
			if (piBytesWritten)
				*piBytesWritten = w;
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT Assembler::AssembleImmediate(LPCTSTR pszMnemonic, bit8 arg, bit8 *pCode, int iBuffersize, int *piBytesWritten)
{
bit8 v[2];

	for(int i=0; i <= 0xff; i++)
	{
		const InstructionInfo& inf = CPU6502::AssemblyData[i];
		if (inf.address_mode == amIMMEDIATE && _tcsicmp(pszMnemonic, CPU6502::AssemblyData[i].mnemonic) == 0)
		{
			v[0] = inf.opcode;
			v[1] = arg;
			int w = instcopy(pCode, iBuffersize, v, _countof(v));
			if (piBytesWritten)
				*piBytesWritten = w;
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT Assembler::AssembleZeroPage(LPCTSTR pszMnemonic, bit8 arg, bit8 *pCode, int iBuffersize, int *piBytesWritten)
{
bit8 v[2];

	for(int i=0; i <= 0xff; i++)
	{
		const InstructionInfo& inf = CPU6502::AssemblyData[i];
		if (inf.address_mode == amZEROPAGE && _tcsicmp(pszMnemonic, CPU6502::AssemblyData[i].mnemonic) == 0)
		{
			v[0] = inf.opcode;
			v[1] = arg;
			int w = instcopy(pCode, iBuffersize, v, _countof(v));
			if (piBytesWritten)
				*piBytesWritten = w;
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT Assembler::AssembleZeroPageX(LPCTSTR pszMnemonic, bit8 arg, bit8 *pCode, int iBuffersize, int *piBytesWritten)
{
bit8 v[2];

	for(int i=0; i <= 0xff; i++)
	{
		const InstructionInfo& inf = CPU6502::AssemblyData[i];
		if (inf.address_mode == amZEROPAGEX && _tcsicmp(pszMnemonic, CPU6502::AssemblyData[i].mnemonic) == 0)
		{
			v[0] = inf.opcode;
			v[1] = arg;
			int w = instcopy(pCode, iBuffersize, v, _countof(v));
			if (piBytesWritten)
				*piBytesWritten = w;
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT Assembler::AssembleZeroPageY(LPCTSTR pszMnemonic, bit8 arg, bit8 *pCode, int iBuffersize, int *piBytesWritten)
{
bit8 v[2];

	for(int i=0; i <= 0xff; i++)
	{
		const InstructionInfo& inf = CPU6502::AssemblyData[i];
		if (inf.address_mode == amZEROPAGEY && _tcsicmp(pszMnemonic, CPU6502::AssemblyData[i].mnemonic) == 0)
		{
			v[0] = inf.opcode;
			v[1] = arg;
			int w = instcopy(pCode, iBuffersize, v, _countof(v));
			if (piBytesWritten)
				*piBytesWritten = w;
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT Assembler::AssembleAbsolute(LPCTSTR pszMnemonic, bit16 arg, bit8 *pCode, int iBuffersize, int *piBytesWritten)
{
bit8 v[3];

	for(int i=0; i <= 0xff; i++)
	{
		const InstructionInfo& inf = CPU6502::AssemblyData[i];
		if (inf.address_mode == amABSOLUTE && _tcsicmp(pszMnemonic, CPU6502::AssemblyData[i].mnemonic) == 0)
		{
			v[0] = inf.opcode;
			v[1] = arg & 0xff;
			v[2] = (arg >> 8) & 0xff;
			int w = instcopy(pCode, iBuffersize, v, _countof(v));
			if (piBytesWritten)
				*piBytesWritten = w;
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT Assembler::AssembleAbsoluteX(LPCTSTR pszMnemonic, bit16 arg, bit8 *pCode, int iBuffersize, int *piBytesWritten)
{
bit8 v[3];

	for(int i=0; i <= 0xff; i++)
	{
		const InstructionInfo& inf = CPU6502::AssemblyData[i];
		if (inf.address_mode == amABSOLUTEX && _tcsicmp(pszMnemonic, CPU6502::AssemblyData[i].mnemonic) == 0)
		{
			v[0] = inf.opcode;
			v[1] = arg & 0xff;
			v[2] = (arg >> 8) & 0xff;
			int w = instcopy(pCode, iBuffersize, v, _countof(v));
			if (piBytesWritten)
				*piBytesWritten = w;
			return S_OK;
		}
	}
	return E_FAIL;

}

HRESULT Assembler::AssembleAbsoluteY(LPCTSTR pszMnemonic, bit16 arg, bit8 *pCode, int iBuffersize, int *piBytesWritten)
{
bit8 v[3];

	for(int i=0; i <= 0xff; i++)
	{
		const InstructionInfo& inf = CPU6502::AssemblyData[i];
		if (inf.address_mode == amABSOLUTEY && _tcsicmp(pszMnemonic, CPU6502::AssemblyData[i].mnemonic) == 0)
		{
			v[0] = inf.opcode;
			v[1] = arg & 0xff;
			v[2] = (arg >> 8) & 0xff;
			int w = instcopy(pCode, iBuffersize, v, _countof(v));
			if (piBytesWritten)
				*piBytesWritten = w;
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT Assembler::AssembleIndirect(LPCTSTR pszMnemonic, bit16 arg, bit8 *pCode, int iBuffersize, int *piBytesWritten)
{
bit8 v[3];

	for(int i=0; i <= 0xff; i++)
	{
		const InstructionInfo& inf = CPU6502::AssemblyData[i];
		if (inf.address_mode == amINDIRECT && _tcsicmp(pszMnemonic, CPU6502::AssemblyData[i].mnemonic) == 0)
		{
			v[0] = inf.opcode;
			v[1] = arg & 0xff;
			v[2] = (arg >> 8) & 0xff;
			int w = instcopy(pCode, iBuffersize, v, _countof(v));
			if (piBytesWritten)
				*piBytesWritten = w;
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT Assembler::AssembleIndirectX(LPCTSTR pszMnemonic, bit8 arg, bit8 *pCode, int iBuffersize, int *piBytesWritten)
{
bit8 v[2];

	for(int i=0; i <= 0xff; i++)
	{
		const InstructionInfo& inf = CPU6502::AssemblyData[i];
		if (inf.address_mode == amINDIRECTX && _tcsicmp(pszMnemonic, CPU6502::AssemblyData[i].mnemonic) == 0)
		{
			v[0] = inf.opcode;
			v[1] = arg;
			int w = instcopy(pCode, iBuffersize, v, _countof(v));
			if (piBytesWritten)
				*piBytesWritten = w;
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT Assembler::AssembleIndirectY(LPCTSTR pszMnemonic, bit8 arg, bit8 *pCode, int iBuffersize, int *piBytesWritten)
{
bit8 v[2];

	for(int i=0; i <= 0xff; i++)
	{
		const InstructionInfo& inf = CPU6502::AssemblyData[i];
		if (inf.address_mode == amINDIRECTY && _tcsicmp(pszMnemonic, CPU6502::AssemblyData[i].mnemonic) == 0)
		{
			v[0] = inf.opcode;
			v[1] = arg;
			int w = instcopy(pCode, iBuffersize, v, _countof(v));
			if (piBytesWritten)
				*piBytesWritten = w;
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT Assembler::AssembleRelative(LPCTSTR pszMnemonic, bit8 arg, bit8 *pCode, int iBuffersize, int *piBytesWritten)
{
bit8 v[2];

	for(int i=0; i <= 0xff; i++)
	{
		const InstructionInfo& inf = CPU6502::AssemblyData[i];
		if (inf.address_mode == amRELATIVE && _tcsicmp(pszMnemonic, CPU6502::AssemblyData[i].mnemonic) == 0)
		{
			v[0] = inf.opcode;
			v[1] = arg;
			int w = instcopy(pCode, iBuffersize, v, _countof(v));
			if (piBytesWritten)
				*piBytesWritten = w;
			return S_OK;
		}
	}
	return E_FAIL;
}

bool Assembler::IsWhiteSpace(TCHAR ch)
{
	return (ch == _T(' ') || ch == _T('\n') || ch == _T('\r') || ch == _T('\t'));
}

bool Assembler::IsLetter(TCHAR ch)
{
	return ((ch >= _T('a') && ch <= _T('z')) || (ch >= _T('A') && ch <= _T('Z')));
}

bool Assembler::IsDigit(TCHAR ch)
{
	return (ch >= _T('0') && ch <= _T('9'));
}

bool Assembler::IsHexDigit(TCHAR ch)
{
	return (ch >= _T('0') && ch <= _T('9')) || (ch >= _T('a') && ch <= _T('f')) || (ch >= _T('A') && ch <= _T('F'));
}

void Assembler::GetNextToken()
{	
TCHAR ch;
	int iHexDigitCount = 0;
	m_LexState = LexState::Start;
	m_ibufPos = 0;
	m_bufIdentifierString[0] = 0;
	m_bufNumber = 0;
	m_CurrentToken = m_NextToken;

	while (true)
	{
		//Look at current char but set next token
		ch = m_CurrentCh;
		switch (m_LexState)
		{
		case LexState::Start:
			if (this->m_bCurrentEOF)
			{
				AssemblyToken::SetEOF(&m_NextToken);
				return;
			}
			else if (IsWhiteSpace(m_CurrentCh))
			{
				m_LexState = LexState::Start;
				GetNextChar();
				break;
			}
			else if (IsLetter(ch))
			{
				if (!AppendToIdentifierString(ch))
				{
					//String too long
					AssemblyToken::SetError(&m_NextToken);
					return;
				}
				m_LexState = LexState::IdentifierString;
				GetNextChar();
				break;
			}
			else if (ch == _T('$') && IsHexDigit(m_NextCh))
			{
				iHexDigitCount = 0;
				m_LexState = LexState::HexString;
				GetNextChar();
				break;
			}
			else if (ch == _T('#') || ch == _T('(') || ch == _T(')') || ch == _T(','))
			{
				m_NextToken = AssemblyToken();
				m_NextToken.TokenType = AssemblyToken::Symbol;
				m_NextToken.SymbolChar = ch;
				GetNextChar();
				return;
			}
			else if (IsDigit(ch))
			{
				m_LexState = LexState::Number;
				break;
			}
			else
			{
				//Invalid character
				AssemblyToken::SetError(&m_NextToken);
				GetNextChar();
				return;
			}
			break;
		case LexState::IdentifierString:
			if (IsLetter(ch) || IsDigit(ch))
			{
				if (!AppendToIdentifierString(ch))
				{
					//String too long
					AssemblyToken::SetError(&m_NextToken);
					return;
				}
				m_LexState = LexState::IdentifierString;
				GetNextChar();
				break;
			}
			else
			{
				m_NextToken = AssemblyToken();
				m_NextToken.TokenType = AssemblyToken::IdentifierString;
				_tcsncpy_s(m_NextToken.IdentifierText, _countof(m_NextToken.IdentifierText), m_bufIdentifierString, _countof(m_bufIdentifierString));
				return;
			}
			break;
		case LexState::Number:
			if (IsDigit(ch))
			{
				m_bufNumber = m_bufNumber * 10;
				m_bufNumber = m_bufNumber + (int)(ch - _T('0'));
				if (m_bufNumber > 0xffff)
				{
					//Number too big
					AssemblyToken::SetError(&m_NextToken);
					return;
				}
				m_LexState = LexState::Number;
				GetNextChar();
				break;
			}
			else
			{
				m_NextToken = AssemblyToken();
				if (m_bufNumber > 0xff)
				{
					m_NextToken.TokenType = AssemblyToken::Number16;
					m_NextToken.Value16 = m_bufNumber;
				}
				else
				{
					m_NextToken.TokenType = AssemblyToken::Number8;
					m_NextToken.Value8 = m_bufNumber;
				}
				return;
			}
			break;
		case LexState::HexString:
			if (IsHexDigit(ch))
			{
				iHexDigitCount++;
				m_bufNumber = m_bufNumber * 16;
				if (IsDigit(ch))
					m_bufNumber = m_bufNumber + (int)(ch - _T('0'));
				else if (ch >= _T('A') && ch <= _T('F'))
					m_bufNumber = m_bufNumber + (int)(ch - _T('A')) + 10;
				else if (ch >= _T('a') && ch <= _T('f'))
					m_bufNumber = m_bufNumber + (int)(ch - _T('a')) + 10;
				if (m_bufNumber > 0xffff)
				{
					//Number too big
					AssemblyToken::SetError(&m_NextToken);
					return;
				}
				m_LexState = LexState::HexString;
				GetNextChar();
				break;
			}
			else
			{
				m_NextToken = AssemblyToken();
				if (m_bufNumber > 0xff || iHexDigitCount > 2)
				{
					m_NextToken.TokenType = AssemblyToken::Number16;
					m_NextToken.Value16 = m_bufNumber;
				}
				else
				{
					m_NextToken.TokenType = AssemblyToken::Number8;
					m_NextToken.Value8 = m_bufNumber;
				}
				_tcsncpy_s(m_NextToken.IdentifierText, _countof(m_NextToken.IdentifierText), m_bufIdentifierString, _countof(m_bufIdentifierString));
				return;
			}
			break;
		default:
			//Unknown state. Should not happen.
			AssemblyToken::SetError(&m_NextToken);
			return;
		}
	}
	return;
}

HRESULT Assembler::GetNextChar()
{
	m_CurrentCh = m_NextCh;
	m_bCurrentEOF = m_bNextEOF;

	if (m_pos < m_iLenText)
	{
		m_NextCh = m_pszText[m_pos];
		m_bNextEOF = false;
		m_pos++;
	}
	else
	{
		m_NextCh = 0;
		m_bNextEOF = true;
	}
	return S_OK;
}

