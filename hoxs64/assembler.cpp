#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <assert.h>
#include "boost2005.h"
#include "bits.h"
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

HRESULT Assembler::InitParser(LPCTSTR pszText)
{
	if (!pszText)
		return E_POINTER;
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

	return S_OK;
}

HRESULT Assembler::TryParseAddress16(LPCTSTR pszText, bit16 *piAddress)
{
Assembler as;
	return as.ParseAddress16(pszText, piAddress);
}

HRESULT Assembler::ParseAddress16(LPCTSTR pszText, bit16 *piAddress)
{
	if (!piAddress)
		return E_POINTER;
	HRESULT hr = InitParser(pszText);
	if (FAILED(hr))
		return hr;

	if (m_CurrentToken.TokenType == AssemblyToken::Number8)
	{
		*piAddress = m_CurrentToken.Value8;
		GetNextToken();
	}
	else if (m_CurrentToken.TokenType == AssemblyToken::Number16)
	{
		*piAddress = m_CurrentToken.Value16;
		GetNextToken();
	}
	else
	{
		*piAddress = 0;
		return E_FAIL;
	}


	if (m_CurrentToken.TokenType != AssemblyToken::EndOfInput)
		return E_FAIL;

	return S_OK;
}

HRESULT Assembler::AssembleText(bit16 address, LPCTSTR pszText, bit8 *pCode, int iBuffersize, int *piBytesWritten)
{
HRESULT hr;

	hr = InitParser(pszText);
	if (FAILED(hr))
		return hr;

	hr = AssembleOneInstruction(address, pCode, iBuffersize, piBytesWritten);
	if (FAILED(hr))
		return hr;

	if (m_CurrentToken.TokenType != AssemblyToken::EndOfInput)
		return E_FAIL;

	return S_OK;
}

HRESULT Assembler::AssembleBytes(bit16 address, bit8 *pCode, int iBuffersize, int *piBytesWritten)
{
unsigned int iValue = 0;

	if (m_CurrentToken.TokenType == AssemblyToken::Number8)
	{
		int i = 0;
		bit8 *p = pCode;
		for (i = 0; m_CurrentToken.TokenType == AssemblyToken::Number8; i++)
		{
			if (p != NULL)
			{
				if (i < iBuffersize)
					p[i] = m_CurrentToken.Value8;
				else
					return E_FAIL;
			}
			GetNextToken();
		}
		if (m_CurrentToken.TokenType != AssemblyToken::EndOfInput)
			return E_FAIL;

		if (piBytesWritten != NULL)
			*piBytesWritten = i;
		return S_OK;
	}
	return E_FAIL;
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
		int i = 0;
		bit8 *p = pCode;
		for (i=0; m_CurrentToken.TokenType == AssemblyToken::Number8; i++)
		{
			if (p != NULL && i < iBuffersize)
				p[i] = m_CurrentToken.Value8;
			GetNextToken();
		}
		if (m_CurrentToken.TokenType == AssemblyToken::EndOfInput)
		{
			if (piBytesWritten != NULL)
				*piBytesWritten = i;
			if (pCode != NULL && iBuffersize < i)
				return E_FAIL;
			return S_OK;
		}
		else if (m_CurrentToken.TokenType == AssemblyToken::IdentifierString)
		{
			return AssembleOneInstruction(address, pCode, iBuffersize, piBytesWritten);
		}
		else
		{
			return E_FAIL;
		}
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

HRESULT Assembler::instcopy(bit8 *pDest, int iSizeDest, bit8 *pSrc, int iSizeSrc, int *piBytesWritten)
{
int i;
	if (pSrc==NULL)
		return E_POINTER;
	if (piBytesWritten)
		*piBytesWritten = iSizeSrc;
	if (pDest != NULL && iSizeDest < iSizeSrc)
		return E_FAIL;
	if (pDest==NULL)
		return S_OK;
	for (i = 0; i < iSizeSrc && i < iSizeDest; i++)
	{
		pDest[i] = pSrc[i];
	}
	if (piBytesWritten)
		*piBytesWritten = i;
	return S_OK;
}

HRESULT Assembler::AssembleImplied(LPCTSTR pszMnemonic, bit8 *pCode, int iBuffersize, int *piBytesWritten)
{
bit8 v[1];

	for(int i=0; i <= 0xff; i++)
	{
		const InstructionInfo& inf = CPU6502::AssemblyData[i];
		if (inf.address_mode == amIMPLIED && _tcsicmp(pszMnemonic, CPU6502::AssemblyData[i].mnemonic) == 0 && inf.undoc == 0)
		{
			v[0] = inf.opcode;
			return instcopy(pCode, iBuffersize, v, _countof(v), piBytesWritten);
		}
	}
	for(int i=0; i <= 0xff; i++)
	{
		const InstructionInfo& inf = CPU6502::AssemblyData[i];
		if (inf.address_mode == amIMPLIED && _tcsicmp(pszMnemonic, CPU6502::AssemblyData[i].mnemonic) == 0)
		{
			v[0] = inf.opcode;
			return instcopy(pCode, iBuffersize, v, _countof(v), piBytesWritten);
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
			return instcopy(pCode, iBuffersize, v, _countof(v), piBytesWritten);
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
			return instcopy(pCode, iBuffersize, v, _countof(v), piBytesWritten);
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
			return instcopy(pCode, iBuffersize, v, _countof(v), piBytesWritten);
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
			return instcopy(pCode, iBuffersize, v, _countof(v), piBytesWritten);
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
			return instcopy(pCode, iBuffersize, v, _countof(v), piBytesWritten);
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
			return instcopy(pCode, iBuffersize, v, _countof(v), piBytesWritten);
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
			return instcopy(pCode, iBuffersize, v, _countof(v), piBytesWritten);
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
			return instcopy(pCode, iBuffersize, v, _countof(v), piBytesWritten);
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
			return instcopy(pCode, iBuffersize, v, _countof(v), piBytesWritten);
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
			return instcopy(pCode, iBuffersize, v, _countof(v), piBytesWritten);
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
			return instcopy(pCode, iBuffersize, v, _countof(v), piBytesWritten);
		}
	}
	return E_FAIL;
}

bool Assembler::IsWhiteSpace(TCHAR ch)
{
	return (ch == _T(' ') || ch == _T('\n') || ch == _T('\r') || ch == _T('\t') || ch == _T('\b') || ch == _T('\v')  || ch == _T('\f'));
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
			else if (ch == _T('#') || ch == _T('(') || ch == _T(')') || ch == _T(',') || ch == _T('-') || ch == _T('?'))
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

HRESULT Assembler::_ParseAddressRange(bit16 *piStartAddress, bit16 *piEndAddress)
{
	if SUCCEEDED(_ParseAddress(piStartAddress))
	{
		if (m_CurrentToken.TokenType == AssemblyToken::Symbol)
		{
			GetNextToken();
		}
		if SUCCEEDED(_ParseAddress(piEndAddress))
		{
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT Assembler::_ParseAddress(bit16 *piAddress)
{
	return _ParseNumber16(piAddress);
}

HRESULT Assembler::_ParseNumber16(bit16 *piNumber)
{
	if (m_CurrentToken.TokenType == AssemblyToken::Number8)
	{
		if (piNumber)
			*piNumber = m_CurrentToken.Value8;
		GetNextToken();
		return S_OK;
	}
	else if (m_CurrentToken.TokenType == AssemblyToken::Number16)
	{
		if (piNumber)
			*piNumber = m_CurrentToken.Value16;
		GetNextToken();
		return S_OK;
	}
	else
	{
		*piNumber = 0;
		return E_FAIL;
	}
}

HRESULT Assembler::CreateCliCommandToken(LPCTSTR pszText, CommandToken **ppmdr)
{
HRESULT hr;
CommandToken *pcr = NULL;
	try
	{
		hr = InitParser(pszText);
		if (FAILED(hr))
			return hr;

		if ((m_CurrentToken.TokenType == AssemblyToken::Symbol && m_CurrentToken.SymbolChar==TEXT('?')) || (m_CurrentToken.TokenType == AssemblyToken::IdentifierString && (_tcsicmp(m_CurrentToken.IdentifierText, TEXT("?")) == 0 || _tcsicmp(m_CurrentToken.IdentifierText, TEXT("help")) == 0 || _tcsicmp(m_CurrentToken.IdentifierText, TEXT("man")) == 0)))
		{
			pcr = new CommandToken();
			if (pcr == 0)
				throw std::bad_alloc();
			GetNextToken();
			if (m_CurrentToken.TokenType == AssemblyToken::IdentifierString)
			{
				pcr->SetTokenHelp(m_CurrentToken.IdentifierText);
			}
			else
			{
				pcr->SetTokenHelp(NULL);
			}
		}
		else if (m_CurrentToken.TokenType == AssemblyToken::IdentifierString)
		{
			if (_tcsicmp(m_CurrentToken.IdentifierText, TEXT("MAP")) == 0 && false)
			{
				pcr = GetCommandTokenMapMemory();
			}
			else if (_tcsicmp(m_CurrentToken.IdentifierText, TEXT("M")) == 0)
			{
				pcr = GetCommandTokenReadMemory();
			}
			else if (_tcsicmp(m_CurrentToken.IdentifierText, TEXT("A")) == 0)
			{
				pcr = GetCommandTokenAssembleLine();
			}
			else if (_tcsicmp(m_CurrentToken.IdentifierText, TEXT("D")) == 0)
			{
				GetNextToken();
				bit16 startaddress, endaddress;
				hr = _ParseAddressRange(&startaddress, &endaddress);
				if (SUCCEEDED(hr) && m_CurrentToken.TokenType == AssemblyToken::EndOfInput)
				{
					pcr = new CommandToken();
					if (pcr == 0)
						throw std::bad_alloc();
					pcr->SetTokenDisassembly(startaddress, endaddress);
				}
				else
				{
					pcr = new CommandToken();
					if (pcr == 0)
						throw std::bad_alloc();
					pcr->SetTokenError(TEXT("Invalid address.\r"));
				}
			}
			else if (_tcsicmp(m_CurrentToken.IdentifierText, TEXT("cpu")) == 0)
			{
				do
				{
					GetNextToken();
					bit16 num;
					hr = _ParseNumber16(&num);
					if (SUCCEEDED(hr) && m_CurrentToken.TokenType == AssemblyToken::EndOfInput)
					{
						DBGSYM::CliCpuMode::CliCpuMode cpumode = (DBGSYM::CliCpuMode::CliCpuMode)num;
						switch (cpumode)
						{
						case DBGSYM::CliCpuMode::C64:
						case DBGSYM::CliCpuMode::Disk:
							pcr = new CommandToken();
							if (pcr == 0)
								throw std::bad_alloc();
							pcr->SetTokenSelectCpu((DBGSYM::CliCpuMode::CliCpuMode)num);
							break;
						}
					}
				} while (false);
				if (!pcr)
				{
					pcr = new CommandToken();
					if (pcr == 0)
						throw std::bad_alloc();
					pcr->SetTokenHelp(TEXT("cpu"));
				}
			}
			else if (_tcsicmp(m_CurrentToken.IdentifierText, TEXT("cls")) == 0)
			{
				GetNextToken();
				pcr = new CommandToken();
				if (pcr == 0)
					throw std::bad_alloc();
				pcr->SetTokenClearScreen();
			}
		}
		if (pcr==NULL)
		{
			pcr = new CommandToken();
			if (pcr == 0)
				throw std::bad_alloc();
			pcr->SetTokenError(TEXT("Unrecognised command. Type 'help'\r"));
		}
		if (ppmdr)
			*ppmdr = pcr;
		return S_OK;
	}
	catch(std::exception)
	{
		if (pcr)
			delete pcr;
		return E_FAIL;
	}
}

CommandToken *Assembler::GetCommandTokenAssembleLine()
{
HRESULT hr;
bit16 address;
bit8 v[256];
int w;
	CommandToken *pcr = NULL;
	try
	{
		pcr = new CommandToken();
		if (pcr == 0)
			throw std::bad_alloc();
		GetNextToken();
		if (m_CurrentToken.TokenType == AssemblyToken::EndOfInput)
		{
			pcr->SetTokenError(TEXT("Usage: a address assembly-code\r"));
		}
		else
		{
			hr = _ParseNumber16(&address);
			if (SUCCEEDED(hr))
			{
				if (m_CurrentToken.TokenType == AssemblyToken::EndOfInput)
				{
					pcr->SetTokenAssemble(address, v, 0);
				}
				else
				{
					hr = this->AssembleOneInstruction(address, v, _countof(v), &w);
					if (SUCCEEDED(hr))
					{
						pcr->SetTokenAssemble(address, v, w);
					}
					else
					{
						pcr->SetTokenError(TEXT("Assemble failed.\r"));
					}
				}
			}
			else 
			{
				pcr->SetTokenError(TEXT("Invalid address.\r"));
			}
		}
		return pcr;
	}
	catch(std::exception)
	{
		if (pcr)
			delete pcr;
		throw;
	}
}

CommandToken *Assembler::GetCommandTokenMapMemory()
{
	return NULL;
}

CommandToken *Assembler::GetCommandTokenReadMemory()
{
HRESULT hr;
bit16 startaddress;
bit16 finishaddress;

	CommandToken *pcr = NULL;
	try
	{
		pcr = new CommandToken();
		if (pcr == 0)
			throw std::bad_alloc();
		GetNextToken();
		if (m_CurrentToken.TokenType == AssemblyToken::EndOfInput)
		{
			pcr->SetTokenError(TEXT("Usage: m start-address [end-address].\r"));
		}
		else
		{
			hr = _ParseNumber16(&startaddress);
			if (SUCCEEDED(hr))
			{
				if (m_CurrentToken.TokenType == AssemblyToken::EndOfInput)
				{
					pcr->SetTokenReadMemory(startaddress, (startaddress + 0xff) & 0xffff);
				}
				else
				{
					hr = _ParseNumber16(&finishaddress);
					if (SUCCEEDED(hr))
					{
						if (m_CurrentToken.TokenType == AssemblyToken::EndOfInput)
						{
							pcr->SetTokenReadMemory(startaddress, finishaddress);
						}
						else
						{
							pcr->SetTokenError(TEXT("Read memory failed.\r"));
						}
					}
					else 
					{
						pcr->SetTokenError(TEXT("Invalid finish-address.\r"));
					}
				}
			}
			else 
			{
				pcr->SetTokenError(TEXT("Invalid start-address.\r"));
			}
		}
		return pcr;
	}
	catch(std::exception)
	{
		if (pcr)
			delete pcr;
		throw;
	}
}

CommandToken *Assembler::GetCommandTokenWriteMemory()
{
HRESULT hr;
bit16 address;
bit8 v[256];
int w;
	CommandToken *pcr = NULL;
	try
	{
		pcr = new CommandToken();
		if (pcr == 0)
			throw std::bad_alloc();
		GetNextToken();
		hr = _ParseNumber16(&address);
		if (SUCCEEDED(hr))
		{
			if (m_CurrentToken.TokenType == AssemblyToken::EndOfInput)
			{
				pcr->SetTokenWriteMemory(address, v, 0);
			}
			else
			{
				hr = this->AssembleBytes(address, v, _countof(v), &w);
				if (SUCCEEDED(hr))
				{
					pcr->SetTokenWriteMemory(address, v, w);
				}
				else
				{
					pcr->SetTokenError(TEXT("Write memory failed.\r"));
				}
			}
		}
		else 
		{
			pcr->SetTokenError(TEXT("Invalid address.\r"));
		}
		return pcr;
	}
	catch(std::exception)
	{
		if (pcr)
			delete pcr;
		throw;
	}
}

CommandToken::CommandToken()
{
	cmd = DBGSYM::CliCommand::Unknown;
}

CommandToken::~CommandToken()
{
}

void CommandToken::SetTokenHelp(LPCTSTR name)
{
	cmd = DBGSYM::CliCommand::Help;
	if (name != NULL)
		this->text.assign(name);
	else
		this->text.clear();
}

void CommandToken::SetTokenDisassembly(bit16 startaddress, bit16 finishaddress)
{
	cmd = DBGSYM::CliCommand::Disassemble;
	this->startaddress = startaddress;
	this->finishaddress = finishaddress;
}

void CommandToken::SetTokenError(LPCTSTR pszErrortext)
{
	cmd = DBGSYM::CliCommand::Error;
	text.append(pszErrortext);
}

void CommandToken::SetTokenSelectCpu(DBGSYM::CliCpuMode::CliCpuMode cpumode)
{
	cmd = DBGSYM::CliCommand::SelectCpu;
	this->cpumode = cpumode;
}

void CommandToken::SetTokenMapMemory(DBGSYM::CliMapMemory::CliMapMemory mapmemory)
{
	cmd = DBGSYM::CliCommand::MapMemory;
	this->mapmemory = mapmemory;
}

void CommandToken::SetTokenShowCpuRegisters()
{
	cmd = DBGSYM::CliCommand::ShowCpu;
}

void CommandToken::SetTokenShowCpu64Registers()
{
	cmd = DBGSYM::CliCommand::ShowCpu64;
}

void CommandToken::SetTokenShowCpuDiskRegisters()
{
	cmd = DBGSYM::CliCommand::ShowCpuDisk;
}

void CommandToken::SetTokenShowVicRegisters()
{
	cmd = DBGSYM::CliCommand::ShowVic;
}

void CommandToken::SetTokenShowCia1Registers()
{
	cmd = DBGSYM::CliCommand::ShowCia1;
}

void CommandToken::SetTokenShowCia2Registers()
{
	cmd = DBGSYM::CliCommand::ShowCia2;
}

void CommandToken::SetTokenShowSidRegisters()
{
	cmd = DBGSYM::CliCommand::ShowSid;
}

void CommandToken::SetTokenShowVia1Registers()
{
	cmd = DBGSYM::CliCommand::ShowVia1;
}

void CommandToken::SetTokenShowVia2Registers()
{
	cmd = DBGSYM::CliCommand::ShowVia2;
}

void CommandToken::SetTokenClearScreen()
{
	cmd = DBGSYM::CliCommand::ClearScreen;
}

void CommandToken::SetTokenReadMemory(bit16 startaddress, bit16 finishaddress)
{
	cmd = DBGSYM::CliCommand::ReadMemory;
	this->startaddress = startaddress;
	this->finishaddress = finishaddress;
}

void CommandToken::SetTokenWriteMemory(bit16 address, bit8 *pData, int bufferSize)
{
	cmd = DBGSYM::CliCommand::WriteMemory;
	dataLength = bufferSize;
	startaddress = address;
	if (dataLength > _countof(buffer))
	{
		//FIXME allow unlimited buffer.
		dataLength = _countof(buffer);
	}
	finishaddress = address + dataLength - 1;
	memcpy(buffer, pData, dataLength);
}

void CommandToken::SetTokenAssemble(bit16 address, bit8 *pData, int bufferSize)
{
	cmd = DBGSYM::CliCommand::Assemble;
	dataLength = bufferSize;
	startaddress = address;
	if (dataLength > _countof(buffer))
	{
		//FIXME allow unlimited buffer.
		dataLength = _countof(buffer);
	}
	finishaddress = address + dataLength - 1;
	memcpy(buffer, pData, dataLength);
}
