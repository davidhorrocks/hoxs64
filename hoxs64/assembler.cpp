#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include "register.h"
#include "c6502.h"
#include "assembler.h"

AssemblyToken::AssemblyToken()
{
	TokenType = AssemblyToken::EndOfInput;
	IdentifierText[0] = 0;
	Value = 0;
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

Assembler::Assembler()
{
	this->radix = DBGSYM::MonitorOption::Hex;
}

void Assembler::SetRadix(DBGSYM::MonitorOption::Radix radix)
{
	this->radix = radix;
}


bool Assembler::AppendToIdentifierString(TCHAR ch)
{
	if (m_ibufPos + 1 >= _countof(m_bufIdentifierString))
	{
		return false;
	}

	m_bufIdentifierString[m_ibufPos] = ch;
	m_ibufPos++;
	m_bufIdentifierString[m_ibufPos] = 0;
	return true;
}

HRESULT Assembler::InitParser(LPCTSTR pszText)
{
	if (!pszText)
	{
		return E_POINTER;
	}

	m_pos = 0;
	m_bIsStartChar = true;
	m_bIsStartToken = true;
	m_bCurrentEOF = true;
	m_bNextEOF = true;

	m_iLenText = lstrlen(pszText);
	if (m_iLenText == 0)
	{
		return E_FAIL;
	}

	m_pszText = pszText;
	GetNextChar();
	GetNextChar();

	GetNextToken();
	GetNextToken();

	return S_OK;
}

HRESULT Assembler::TryParseAddress16(LPCTSTR pszText, DBGSYM::MonitorOption::Radix radix, bit16 *piAddress)
{
	Assembler as;
	as.SetRadix(radix);
	return as.ParseAddress16(pszText, piAddress);
}

HRESULT Assembler::ParseAddress16(LPCTSTR pszText, bit16 *piAddress)
{
	if (!piAddress)
	{
		return E_POINTER;
	}

	HRESULT hr = InitParser(pszText);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = ParseNumber16(piAddress);
	if (SUCCEEDED(hr))
	{
		if (m_CurrentToken.TokenType != AssemblyToken::EndOfInput)
		{
			return E_FAIL;
		}
	}

	return hr;
}

HRESULT Assembler::AssembleText(bit16 address, LPCTSTR pszText, bit8 *pCode, unsigned int iBuffersize, unsigned int *piBytesWritten)
{
HRESULT hr;

	hr = InitParser(pszText);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = AssembleOneInstruction(address, pCode, iBuffersize, piBytesWritten);
	if (FAILED(hr))
	{
		return hr;
	}

	if (m_CurrentToken.TokenType != AssemblyToken::EndOfInput)
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT Assembler::AssembleBytes(bit16 address, bit8 *pCode, unsigned int iBuffersize, unsigned int *piBytesWritten)
{
unsigned int iValue = 0;
unsigned int i = 0;
HRESULT hr = S_OK;	
bit16 v;
bit8 *p = pCode;
bool is16bit = false;
	if (p == NULL)
	{
		iBuffersize = 0x10000;
	}

	for (i = 0; i < iBuffersize && m_CurrentToken.TokenType != AssemblyToken::EndOfInput; i++)
	{
		hr = ParseNumber16(&v, &is16bit);
		if (SUCCEEDED(hr))
		{
			if (is16bit)
			{
				if (i + 1 < iBuffersize)
				{
					if (p)
					{
						p[i] = v & 0xff;
					}

					i++;

					if (p)
					{
						p[i] = (v >> 8) & 0xff;
					}
				}
				else
				{
					hr = E_FAIL;
					break;
				}
			}
			else
			{
				if (p)
				{
					p[i] = v & 0xff;
				}
			}
		}
		else
		{
			break;
		}
	}

	if (m_CurrentToken.TokenType != AssemblyToken::EndOfInput)
	{
		return E_FAIL;
	}

	if (piBytesWritten != NULL)
	{
		*piBytesWritten = i;
	}

	return hr;
	//if (m_CurrentToken.TokenType == AssemblyToken::Number8)
	//{
	//	int i = 0;
	//	for (i = 0; m_CurrentToken.TokenType == AssemblyToken::Number8; i++)
	//	{
	//		if (p != NULL)
	//		{
	//			if (i < iBuffersize)
	//			{
	//				p[i] = m_CurrentToken.Value8;
	//			}
	//			else
	//			{
	//				return E_FAIL;
	//			}
	//		}

	//		GetNextToken();
	//	}

	//	if (m_CurrentToken.TokenType != AssemblyToken::EndOfInput)
	//	{
	//		return E_FAIL;
	//	}

	//	if (piBytesWritten != NULL)
	//	{
	//		*piBytesWritten = i;
	//	}

	//	return S_OK;
	//}

	//return E_FAIL;
}

HRESULT Assembler::AssembleOneInstruction(bit16 address, bit8 *pCode, unsigned int iBuffersize, unsigned int *piBytesWritten)
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
	else if (m_CurrentToken.TokenType == AssemblyToken::Number8 || m_CurrentToken.TokenType == AssemblyToken::Number16 || (m_CurrentToken.TokenType == AssemblyToken::IdentifierString && _tcsicmp(m_CurrentToken.IdentifierText, TEXT("m")) == 0))
	{
		int i = 0;
		bit8 *p = pCode;
		if (m_CurrentToken.TokenType == AssemblyToken::IdentifierString)
		{
			GetNextToken();
		}

		hr = AssembleBytes(address, pCode, iBuffersize, piBytesWritten);
		return hr;
	}
	else if (m_CurrentToken.TokenType == AssemblyToken::IdentifierString)
	{
		tk = m_CurrentToken;
		GetNextToken();
		TryConvertIdentifierTokenToHexNumber(m_CurrentToken);
		if (m_CurrentToken.TokenType == AssemblyToken::EndOfInput)
		{
			//Implied addressing.
			hr = AssembleImplied(tk.IdentifierText, pCode, iBuffersize,  piBytesWritten);
			return hr;
		}
		else if (m_CurrentToken.TokenType == AssemblyToken::Number8)
		{
			num8 = (bit8)m_CurrentToken.Value;
			GetNextToken();			
			if (m_CurrentToken.TokenType == AssemblyToken::EndOfInput)
			{
				//$00
				hr = AssembleZeroPage(tk.IdentifierText, num8, pCode, iBuffersize, piBytesWritten);
				if (SUCCEEDED(hr))
				{
					return S_OK;
				}

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
							{
								return S_OK;
							}

							hr = AssembleAbsoluteX(tk.IdentifierText, (bit16) num8, pCode, iBuffersize, piBytesWritten);
							return hr;
						}
						else if (_tcsicmp(m_CurrentToken.IdentifierText, TEXT("Y")) == 0)
						{
							GetNextToken();
							//$00,Y
							hr = AssembleZeroPageY(tk.IdentifierText, num8, pCode, iBuffersize, piBytesWritten);
							if (SUCCEEDED(hr))
							{
								return S_OK;
							}

							hr = AssembleAbsoluteY(tk.IdentifierText, (bit16) num8, pCode, iBuffersize, piBytesWritten);
							return hr;
						}
					}
				}
			}
		}
		else if (m_CurrentToken.TokenType == AssemblyToken::Number16)
		{
			num16 = (bit16)m_CurrentToken.Value;
			GetNextToken();
			if (m_CurrentToken.TokenType == AssemblyToken::EndOfInput)
			{
				//$0000
				hr = AssembleAbsolute(tk.IdentifierText, num16, pCode, iBuffersize, piBytesWritten);
				if (SUCCEEDED(hr))
				{
					return hr;
				}

				if (num16 <= 0xff)
				{
					hr = AssembleZeroPage(tk.IdentifierText, (bit8)(num16 & 0xff), pCode, iBuffersize, piBytesWritten);
					if (SUCCEEDED(hr))
					{
						return hr;
					}
				}

				int diff = ((int)num16) - ((int)address+2);
				if (diff <= 0x7f && diff >= -0x80)
				{
					hr = AssembleRelative(tk.IdentifierText, (bit8)(diff & 0xff), pCode, iBuffersize, piBytesWritten);
					if (SUCCEEDED(hr))
					{
						return hr;
					}
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
				TryConvertIdentifierTokenToHexNumber(m_CurrentToken);
				if (m_CurrentToken.TokenType == AssemblyToken::Number8 || m_CurrentToken.TokenType == AssemblyToken::Number16)
				{
					if (m_CurrentToken.TokenType == AssemblyToken::Number8)
					{
						iValue = (bit8)m_CurrentToken.Value;
					}
					else
					{
						iValue = (bit16)m_CurrentToken.Value;
					}

					GetNextToken();
					if (iValue <= 0xff)
					{
						//#$00
						hr = AssembleImmediate(tk.IdentifierText, (bit8)(iValue & 0xff), pCode, iBuffersize, piBytesWritten);
						if (SUCCEEDED(hr))
						{
							return S_OK;
						}

						hr = AssembleRelative(tk.IdentifierText, (bit8)(iValue & 0xff), pCode, iBuffersize, piBytesWritten);
						if (SUCCEEDED(hr))
						{
							return S_OK;
						}
					}
				}
			}
			else if (m_CurrentToken.SymbolChar == _T('('))
			{
				GetNextToken();
				TryConvertIdentifierTokenToHexNumber(m_CurrentToken);
				if (m_CurrentToken.TokenType == AssemblyToken::Number8 || (m_CurrentToken.TokenType == AssemblyToken::Number16))
				{
					if (m_CurrentToken.TokenType == AssemblyToken::Number8)
					{
						iValue = (bit8)m_CurrentToken.Value;
					}
					else
					{
						iValue = (bit16)m_CurrentToken.Value;
					}

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

HRESULT Assembler::instcopy(bit8 *pDest, unsigned int iSizeDest, bit8 *pSrc, unsigned int iSizeSrc, unsigned int *piBytesWritten)
{
unsigned int i;
	if (pSrc==NULL)
	{
		return E_POINTER;
	}

	if (piBytesWritten)
	{
		*piBytesWritten = iSizeSrc;
	}

	if (pDest != NULL && iSizeDest < iSizeSrc)
	{
		return E_FAIL;
	}

	if (pDest==NULL)
	{
		return S_OK;
	}

	for (i = 0; i < iSizeSrc && i < iSizeDest; i++)
	{
		pDest[i] = pSrc[i];
	}

	if (piBytesWritten)
	{
		*piBytesWritten = i;
	}

	return S_OK;
}

HRESULT Assembler::AssembleImplied(LPCTSTR pszMnemonic, bit8 *pCode, unsigned int iBuffersize, unsigned int *piBytesWritten)
{
bit8 v[1];

	for(unsigned int i=0; i <= 0xff; i++)
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

HRESULT Assembler::AssembleImmediate(LPCTSTR pszMnemonic, bit8 arg, bit8 *pCode, unsigned int iBuffersize, unsigned int *piBytesWritten)
{
bit8 v[2];

	for(unsigned int i=0; i <= 0xff; i++)
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

HRESULT Assembler::AssembleZeroPage(LPCTSTR pszMnemonic, bit8 arg, bit8 *pCode, unsigned int iBuffersize, unsigned int *piBytesWritten)
{
bit8 v[2];

	for(unsigned int i=0; i <= 0xff; i++)
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

HRESULT Assembler::AssembleZeroPageX(LPCTSTR pszMnemonic, bit8 arg, bit8 *pCode, unsigned int iBuffersize, unsigned int *piBytesWritten)
{
bit8 v[2];

	for(unsigned int i=0; i <= 0xff; i++)
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

HRESULT Assembler::AssembleZeroPageY(LPCTSTR pszMnemonic, bit8 arg, bit8 *pCode, unsigned int iBuffersize, unsigned int *piBytesWritten)
{
bit8 v[2];

	for(unsigned int i=0; i <= 0xff; i++)
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

HRESULT Assembler::AssembleAbsolute(LPCTSTR pszMnemonic, bit16 arg, bit8 *pCode, unsigned int iBuffersize, unsigned int *piBytesWritten)
{
bit8 v[3];

	for(unsigned int i=0; i <= 0xff; i++)
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

HRESULT Assembler::AssembleAbsoluteX(LPCTSTR pszMnemonic, bit16 arg, bit8 *pCode, unsigned int iBuffersize, unsigned int *piBytesWritten)
{
bit8 v[3];

	for(unsigned int i=0; i <= 0xff; i++)
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

HRESULT Assembler::AssembleAbsoluteY(LPCTSTR pszMnemonic, bit16 arg, bit8 *pCode, unsigned int iBuffersize, unsigned int *piBytesWritten)
{
bit8 v[3];

	for(unsigned int i=0; i <= 0xff; i++)
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

HRESULT Assembler::AssembleIndirect(LPCTSTR pszMnemonic, bit16 arg, bit8 *pCode, unsigned int iBuffersize, unsigned int *piBytesWritten)
{
bit8 v[3];

	for(unsigned int i=0; i <= 0xff; i++)
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

HRESULT Assembler::AssembleIndirectX(LPCTSTR pszMnemonic, bit8 arg, bit8 *pCode, unsigned int iBuffersize, unsigned int *piBytesWritten)
{
bit8 v[2];

	for(unsigned int i=0; i <= 0xff; i++)
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

HRESULT Assembler::AssembleIndirectY(LPCTSTR pszMnemonic, bit8 arg, bit8 *pCode, unsigned int iBuffersize, unsigned int *piBytesWritten)
{
bit8 v[2];

	for(unsigned int i=0; i <= 0xff; i++)
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

HRESULT Assembler::AssembleRelative(LPCTSTR pszMnemonic, bit8 arg, bit8 *pCode, unsigned int iBuffersize, unsigned int *piBytesWritten)
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
			else if (ch == _T('.') && IsDigit(m_NextCh))
			{
				m_LexState = LexState::Number;
				GetNextChar();
				break;
			}
			else if (IsDigit(ch))
			{
				if (this->radix == DBGSYM::MonitorOption::Dec)
				{
					m_LexState = LexState::Number;
				}
				else
				{
					iHexDigitCount = 0;
					m_LexState = LexState::HexString;
				}

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
				if (m_bufNumber > MaxAllowedNumber)
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
				if (m_bufNumber > 0xffffffff)
				{
					m_NextToken.TokenType = AssemblyToken::Number64;
					m_NextToken.Value = m_bufNumber;
				}
				if (m_bufNumber > 0xffff)
				{
					m_NextToken.TokenType = AssemblyToken::Number32;
					m_NextToken.Value = m_bufNumber;
				}
				if (m_bufNumber > 0xff)
				{
					m_NextToken.TokenType = AssemblyToken::Number16;
					m_NextToken.Value = m_bufNumber;
				}
				else
				{
					m_NextToken.TokenType = AssemblyToken::Number8;
					m_NextToken.Value = m_bufNumber;
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
				{
					m_bufNumber = m_bufNumber + (int)(ch - _T('0'));
				}
				else if (ch >= _T('A') && ch <= _T('F'))
				{
					m_bufNumber = m_bufNumber + (int)(ch - _T('A')) + 10;
				}
				else if (ch >= _T('a') && ch <= _T('f'))
				{
					m_bufNumber = m_bufNumber + (int)(ch - _T('a')) + 10;
				}

				if (m_bufNumber > MaxAllowedNumber)
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
				if (m_bufNumber > 0xffffffff || iHexDigitCount > 8)
				{
					m_NextToken.TokenType = AssemblyToken::Number64;
					m_NextToken.Value = m_bufNumber;
				}
				if (m_bufNumber > 0xffff || iHexDigitCount > 4)
				{
					m_NextToken.TokenType = AssemblyToken::Number32;
					m_NextToken.Value = m_bufNumber;
				}
				else if (m_bufNumber > 0xff || iHexDigitCount > 2)
				{
					m_NextToken.TokenType = AssemblyToken::Number16;
					m_NextToken.Value = m_bufNumber;
				}
				else
				{
					m_NextToken.TokenType = AssemblyToken::Number8;
					m_NextToken.Value = m_bufNumber;
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

HRESULT Assembler::ParseAddressRange(bit16 *piStartAddress, bit16 *piEndAddress)
{
	if SUCCEEDED(ParseAddress(piStartAddress))
	{
		if (m_CurrentToken.TokenType == AssemblyToken::Symbol)
		{
			GetNextToken();
		}
		if SUCCEEDED(ParseAddress(piEndAddress))
		{
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT Assembler::ParseAddress(bit16 *piAddress)
{
	return ParseNumber16(piAddress);
}

HRESULT Assembler::ParseNumber16(bit16 *piNumber)
{
	bool is16bit;
	return ParseNumber16(piNumber, &is16bit);
}

HRESULT Assembler::ParseNumber16(bit16 *piNumber, bool *pIs16bit)
{
	if (pIs16bit)
	{
		*pIs16bit = false;
	}

	if (m_CurrentToken.TokenType == AssemblyToken::Number8)
	{
		if (piNumber)
		{			
			*piNumber = (bit8)m_CurrentToken.Value;
		}

		GetNextToken();
		return S_OK;
	}
	else if (m_CurrentToken.TokenType == AssemblyToken::Number16)
	{
		if (piNumber)
		{			
			*piNumber = (bit16)m_CurrentToken.Value;
		}

		if (pIs16bit)
		{
			*pIs16bit = true;
		}

		GetNextToken();
		return S_OK;
	}
	else if (m_CurrentToken.TokenType == AssemblyToken::IdentifierString && this->radix == DBGSYM::MonitorOption::Hex)
	{
		bool ok = false;
		int len = lstrlen(m_CurrentToken.IdentifierText);
		
		if (len > 2)
		{
			if (pIs16bit)
			{
				*pIs16bit = true;
			}
		}

		bit16 v;
		ok = TryParseHexWord(m_CurrentToken.IdentifierText, v);
		if (ok)
		{
			if (piNumber)
			{
				*piNumber = v;
			}

			GetNextToken();
			return S_OK;
		}
	}

	if (piNumber)
	{
		*piNumber = 0;
	}

	return E_FAIL;
}

HRESULT Assembler::ParseNumber(bit64& result)
{
	result = 0;
	switch (m_CurrentToken.TokenType)
	{
	case AssemblyToken::Number8:
	case AssemblyToken::Number16:
	case AssemblyToken::Number32:
	case AssemblyToken::Number64:
		result = m_CurrentToken.Value;
		GetNextToken();
		return S_OK;
	default:
		break;
	}

	if (m_CurrentToken.TokenType == AssemblyToken::IdentifierString && this->radix == DBGSYM::MonitorOption::Hex)
	{
		bit64 v = 0;
		if (TryParseHex(m_CurrentToken.IdentifierText, v))
		{
			result = v;
			GetNextToken();
			return S_OK;
		}
	}

	return E_FAIL;
}

bool Assembler::TryParseHex(LPCTSTR str, bit64& result)
{
	bool ok = false;
	bit64 v = 0;
	if (str != NULL)
	{
		unsigned int len = lstrlen(str);
		if (len > 0 && len <= 16)
		{
			ok = true;
			unsigned int i;
			for (i = 0; i < len; i++)
			{
				TCHAR ch = str[i];
				if (!IsHexDigit(ch))
				{
					ok = false;
					break;
				}

				v <<= 4;
				if (IsDigit(ch))
				{
					v = v + (unsigned int)(ch - _T('0'));
				}
				else if (ch >= _T('A') && ch <= _T('F'))
				{
					v = v + (unsigned int)(ch - _T('A')) + 10;
				}
				else if (ch >= _T('a') && ch <= _T('f'))
				{
					v = v + (unsigned int)(ch - _T('a')) + 10;
				}
			}
		}
	}

	result = v;
	return true;
}

bool Assembler::TryParseHexWord(LPCTSTR str, bit16& result)
{
	bit64 v = 0;
	result = 0;
	if (TryParseHex(str, v))
	{
		if (v <= 0xffff)
		{
			result = (bit16)v;
			return true;
		}
	}

	return false;
}

bool Assembler::TryConvertIdentifierTokenToHexNumber(AssemblyToken &token)
{
	if (this->radix == DBGSYM::MonitorOption::Hex && token.TokenType == AssemblyToken::IdentifierString)
	{
		bit64 v;
		if (TryParseHex(token.IdentifierText, v))
		{
			if (lstrlen(token.IdentifierText) > 8)
			{
				token.TokenType = AssemblyToken::Number64;
				token.Value = v;
			}
			if (lstrlen(token.IdentifierText) > 4)
			{
				token.TokenType = AssemblyToken::Number32;
				token.Value = v;
			}
			if (lstrlen(token.IdentifierText) > 2)
			{
				token.TokenType = AssemblyToken::Number16;
				token.Value = v;
			}
			else
			{
				token.TokenType = AssemblyToken::Number8;
				token.Value = v;
			}

			return true;
		}
	}

	return false;
}

HRESULT Assembler::CreateCliCommandToken(LPCTSTR pszText, CommandToken **ppmdr)
{
HRESULT hr;
CommandToken *pcr = NULL;
	try
	{
		hr = InitParser(pszText);
		if (FAILED(hr))
		{
			return hr;
		}

		if ((m_CurrentToken.TokenType == AssemblyToken::Symbol && m_CurrentToken.SymbolChar==TEXT('?')) || (m_CurrentToken.TokenType == AssemblyToken::IdentifierString && (_tcsicmp(m_CurrentToken.IdentifierText, TEXT("?")) == 0 || _tcsicmp(m_CurrentToken.IdentifierText, TEXT("help")) == 0 || _tcsicmp(m_CurrentToken.IdentifierText, TEXT("man")) == 0)))
		{
			pcr = new CommandToken();
			if (pcr == 0)
			{
				throw std::bad_alloc();
			}

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
			if (_tcsicmp(m_CurrentToken.IdentifierText, TEXT("MAP")) == 0)
			{
				pcr = GetCommandTokenMapMemory();
			}
			else if (_tcsicmp(m_CurrentToken.IdentifierText, TEXT("M")) == 0)
			{
				pcr = GetCommandTokenReadMemory();
			}
			else if (_tcsicmp(m_CurrentToken.IdentifierText, TEXT("T")) == 0)
			{
				pcr = GetCommandTokenStep();
			}
			else if (_tcsicmp(m_CurrentToken.IdentifierText, TEXT("A")) == 0)
			{
				pcr = GetCommandTokenAssembleLine();
			}
			else if (_tcsicmp(m_CurrentToken.IdentifierText, TEXT("D")) == 0)
			{
				pcr = GetCommandTokenDisassembleLine();
			}
			else if (_tcsicmp(m_CurrentToken.IdentifierText, TEXT("cpu")) == 0)
			{
				pcr = GetCommandTokenCpu();
			}
			else if (_tcsicmp(m_CurrentToken.IdentifierText, TEXT("r")) == 0)
			{
				pcr = GetCommandTokenShowCpu();
			}
			else if (_tcsicmp(m_CurrentToken.IdentifierText, TEXT("cls")) == 0)
			{
				GetNextToken();
				pcr = new CommandToken();
				if (pcr == 0)
				{
					throw std::bad_alloc();
				}

				pcr->SetTokenClearScreen();
			}
		}

		if (pcr==NULL)
		{
			pcr = new CommandToken();
			if (pcr == 0)
			{
				throw std::bad_alloc();
			}

			pcr->SetTokenError(TEXT("Unrecognised command. Type 'help'\r"));
		}

		if (ppmdr)
		{
			*ppmdr = pcr;
		}

		return S_OK;
	}
	catch(std::exception&)
	{
		if (pcr)
		{
			delete pcr;
		}

		return E_FAIL;
	}
}

CommandToken *Assembler::GetCommandTokenShowCpu()
{
	CommandToken *pcr = NULL;
	try
	{
		pcr = new CommandToken();
		if (pcr == 0)
		{
			throw std::bad_alloc();
		}

		GetNextToken();
		if (m_CurrentToken.TokenType == AssemblyToken::EndOfInput)
		{
			pcr->SetTokenShowCpu();
		}

		return pcr;
	}
	catch(std::exception&)
	{
		if (pcr)
			delete pcr;
		throw;
	}
}

CommandToken *Assembler::GetCommandTokenCpu()
{
	CommandToken *pcr = NULL;
	try
	{
		pcr = new CommandToken();
		if (pcr == 0)
		{
			throw std::bad_alloc();
		}

		bool bViewCpu = true;
		bool bOk = false;
		DBGSYM::CliCpuMode::CliCpuMode cpumode  = DBGSYM::CliCpuMode::C64;
		GetNextToken();
		if (m_CurrentToken.TokenType == AssemblyToken::EndOfInput)
		{
			bViewCpu = true;
			bOk = true;
		}
		else
		{
			if (m_CurrentToken.TokenType == AssemblyToken::Number8)
			{
				if (m_CurrentToken.Value >= 0 && m_CurrentToken.Value <= 1)
				{
					cpumode = (DBGSYM::CliCpuMode::CliCpuMode)m_CurrentToken.Value;
					bViewCpu = false;
					bOk = true;
					GetNextToken();
				}
			}
			else
			{
				if (_tcsicmp(m_CurrentToken.IdentifierText, TEXT("C64")) == 0)
				{
					bViewCpu = false;
					bOk = true;
					cpumode  = DBGSYM::CliCpuMode::C64;
					GetNextToken();
				}
				else if (_tcsicmp(m_CurrentToken.IdentifierText, TEXT("DISK")) == 0)
				{
					bViewCpu = false;
					bOk = true;
					cpumode  = DBGSYM::CliCpuMode::Disk;
					GetNextToken();
				}
			}
		}

		if (m_CurrentToken.TokenType != AssemblyToken::EndOfInput)
		{
			bOk = false;
		}

		if (bOk)
		{
			pcr->SetTokenSelectCpu(cpumode, bViewCpu);
		}
		else
		{
			pcr->SetTokenHelp(TEXT("cpu"));
		}

		return pcr;
	}
	catch(std::exception&)
	{
		if (pcr)
		{
			delete pcr;
		}

		throw;
	}
}

CommandToken *Assembler::GetCommandTokenDisassembleLine()
{
HRESULT hr;
bit16 startaddress;
bit16 length;
bit16 finishaddress;

	CommandToken *pcr = NULL;
	try
	{
		pcr = new CommandToken();
		if (pcr == 0)
		{
			throw std::bad_alloc();
		}

		GetNextToken();
		if (m_CurrentToken.TokenType == AssemblyToken::EndOfInput)
		{
			pcr->SetTokenDisassembly();
		}
		else
		{
			hr = ParseNumber16(&startaddress);
			if (SUCCEEDED(hr))
			{
				if (m_CurrentToken.TokenType == AssemblyToken::EndOfInput)
				{
					pcr->SetTokenDisassembly(startaddress);
				}
				else
				{
					if (m_CurrentToken.TokenType == AssemblyToken::Symbol && m_CurrentToken.SymbolChar == TEXT('-'))
					{
						GetNextToken();
						hr = ParseNumber16(&finishaddress);
						if (FAILED(hr))
						{
							pcr->SetTokenError(TEXT("Invalid finish-address.\r"));
						}
					}
					else
					{
						hr = ParseNumber16(&length);
						if (SUCCEEDED(hr))
						{
							if (length == 0)
							{
								length++;
							}

							finishaddress = startaddress + length - 1;
						}
						else
						{
							if (FAILED(hr))
							{
								pcr->SetTokenError(TEXT("Invalid length.\r"));
							}
						}
					}

					if (SUCCEEDED(hr))
					{
						if (m_CurrentToken.TokenType == AssemblyToken::EndOfInput)
						{
							pcr->SetTokenDisassembly(startaddress, finishaddress);
						}
						else
						{
							pcr->SetTokenError(TEXT("Too many parameters.\r"));
						}
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
	catch(std::exception&)
	{
		if (pcr)
		{
			delete pcr;
		}

		throw;
	}
}

CommandToken *Assembler::GetCommandTokenAssembleLine()
{
HRESULT hr;
bit16 address;
bit8 v[256];
unsigned int w;
	CommandToken *pcr = NULL;
	try
	{
		pcr = new CommandToken();
		if (pcr == 0)
		{
			throw std::bad_alloc();
		}

		GetNextToken();
		if (m_CurrentToken.TokenType == AssemblyToken::EndOfInput)
		{
			pcr->SetTokenError(TEXT("Require start-address.\r"));
		}
		else
		{
			hr = ParseNumber16(&address);
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
	catch(std::exception&)
	{
		if (pcr)
		{
			delete pcr;
		}

		throw;
	}
}

CommandToken *Assembler::GetCommandTokenMapMemory()
{
//HRESULT hr;

	CommandToken *pcr = NULL;
	try
	{
		pcr = new CommandToken();
		if (pcr == 0)
		{
			throw std::bad_alloc();
		}

		DBGSYM::CliMapMemory::CliMapMemory map = DBGSYM::CliMapMemory::VIEWCURRENT;
		while (m_CurrentToken.TokenType != AssemblyToken::EndOfInput)
		{
			GetNextToken();
			if (m_CurrentToken.TokenType == AssemblyToken::IdentifierString && _tcsicmp(m_CurrentToken.IdentifierText, TEXT("RAM")) == 0)
			{
				map = (DBGSYM::CliMapMemory::CliMapMemory)((int)map | (int)DBGSYM::CliMapMemory::RAM);
			}
			else if (m_CurrentToken.TokenType == AssemblyToken::IdentifierString && _tcsicmp(m_CurrentToken.IdentifierText, TEXT("BASIC")) == 0)
			{
				map = (DBGSYM::CliMapMemory::CliMapMemory)((int)map | (int)DBGSYM::CliMapMemory::BASIC);
			}
			else if (m_CurrentToken.TokenType == AssemblyToken::IdentifierString && _tcsicmp(m_CurrentToken.IdentifierText, TEXT("CHARGEN")) == 0)
			{
				map = (DBGSYM::CliMapMemory::CliMapMemory)((int)map | (int)DBGSYM::CliMapMemory::CHARGEN);
			}
			else if (m_CurrentToken.TokenType == AssemblyToken::IdentifierString && _tcsicmp(m_CurrentToken.IdentifierText, TEXT("IO")) == 0)
			{
				map = (DBGSYM::CliMapMemory::CliMapMemory)((int)map | (int)DBGSYM::CliMapMemory::IO);
			}
			else if (m_CurrentToken.TokenType == AssemblyToken::IdentifierString && (_tcsicmp(m_CurrentToken.IdentifierText, TEXT("KERNAL")) == 0 || _tcsicmp(m_CurrentToken.IdentifierText, TEXT("KERNEL")) == 0))
			{
				map = (DBGSYM::CliMapMemory::CliMapMemory)((int)map | (int)DBGSYM::CliMapMemory::KERNAL);
			}
			else if (m_CurrentToken.TokenType == AssemblyToken::IdentifierString && _tcsicmp(m_CurrentToken.IdentifierText, TEXT("ROMH")) == 0)
			{
				map = (DBGSYM::CliMapMemory::CliMapMemory)((int)map | (int)DBGSYM::CliMapMemory::ROMH);
			}
			else if (m_CurrentToken.TokenType == AssemblyToken::IdentifierString && _tcsicmp(m_CurrentToken.IdentifierText, TEXT("ROML")) == 0)
			{
				map = (DBGSYM::CliMapMemory::CliMapMemory)((int)map | (int)DBGSYM::CliMapMemory::ROML);
			}
			else if (m_CurrentToken.TokenType == AssemblyToken::IdentifierString && _tcsicmp(m_CurrentToken.IdentifierText, TEXT("C64")) == 0)
			{
				map = (DBGSYM::CliMapMemory::CliMapMemory)((int)map | (int)DBGSYM::CliMapMemory::SETCURRENT);
			}
			else if (m_CurrentToken.TokenType != AssemblyToken::EndOfInput)
			{
				pcr->SetTokenError(TEXT("Invalid map.\r"));
				break;
			}
		}
		if (pcr->cmd != DBGSYM::CliCommand::Error)
		{
			if(map & DBGSYM::CliMapMemory::SETCURRENT)
			{
				map = DBGSYM::CliMapMemory::SETCURRENT;
			}

			pcr->SetTokenMapMemory(map);
		}

		return pcr;
	}
	catch(std::exception&)
	{
		if (pcr)
		{
			delete pcr;
		}

		throw;
	}
}

CommandToken* Assembler::GetCommandTokenStep()
{
	HRESULT hr;
	bit64 count;

	CommandToken* pcr = NULL;
	try
	{
		pcr = new CommandToken();
		if (pcr == 0)
		{
			throw std::bad_alloc();
		}

		GetNextToken();
		if (m_CurrentToken.TokenType == AssemblyToken::EndOfInput)
		{
			pcr->SetTokenStep((bit64)-1);
		}
		else
		{
			hr = ParseNumber(count);
			if (SUCCEEDED(hr))
			{
				if (m_CurrentToken.TokenType == AssemblyToken::EndOfInput)
				{
					pcr->SetTokenStep(count);
				}
				else
				{
					pcr->SetTokenError(TEXT("Too many parameters.\r"));
				}
			}
			else
			{
				pcr->SetTokenError(TEXT("Invalid step count.\r"));
			}
		}

		return pcr;
	}
	catch (std::exception&)
	{
		if (pcr)
		{
			delete pcr;
		}

		throw;
	}
}

CommandToken *Assembler::GetCommandTokenReadMemory()
{
HRESULT hr;
bit16 startaddress;
bit16 length;
bit16 finishaddress;

	CommandToken *pcr = NULL;
	try
	{
		pcr = new CommandToken();
		if (pcr == 0)
		{
			throw std::bad_alloc();
		}

		GetNextToken();
		if (m_CurrentToken.TokenType == AssemblyToken::EndOfInput)
		{
			pcr->SetTokenReadMemory();
		}
		else
		{
			hr = ParseNumber16(&startaddress);
			if (SUCCEEDED(hr))
			{
				if (m_CurrentToken.TokenType == AssemblyToken::EndOfInput)
				{
					pcr->SetTokenReadMemory(startaddress);
				}
				else 
				{
					if (m_CurrentToken.TokenType == AssemblyToken::Symbol && m_CurrentToken.SymbolChar == TEXT('-'))
					{
						GetNextToken();
						hr = ParseNumber16(&finishaddress);
						if (FAILED(hr))
						{
							pcr->SetTokenError(TEXT("Invalid finish-address.\r"));
						}
					}
					else
					{
						hr = ParseNumber16(&length);
						if (SUCCEEDED(hr))
						{
							if (length == 0)
							{
								length++;
							}

							finishaddress = startaddress + length - 1;
						}
						else
						{
							if (FAILED(hr))
							{
								pcr->SetTokenError(TEXT("Invalid length.\r"));
							}
						}
					}

					if (SUCCEEDED(hr))
					{
						if (m_CurrentToken.TokenType == AssemblyToken::EndOfInput)
						{
							pcr->SetTokenReadMemory(startaddress, finishaddress);
						}
						else
						{
							pcr->SetTokenError(TEXT("Too many parameters.\r"));
						}
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
	catch(std::exception&)
	{
		if (pcr)
		{
			delete pcr;
		}

		throw;
	}
}

CommandToken *Assembler::GetCommandTokenWriteMemory()
{
HRESULT hr;
bit16 address;
bit8 v[256];
unsigned int w;
	CommandToken *pcr = NULL;
	try
	{
		pcr = new CommandToken();
		if (pcr == 0)
		{
			throw std::bad_alloc();
		}

		GetNextToken();
		hr = ParseNumber16(&address);
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
	catch(std::exception&)
	{
		if (pcr)
		{
			delete pcr;
		}

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
	{
		this->text.assign(name);
	}
	else
	{
		this->text.clear();
	}
}

void CommandToken::SetTokenDisassembly()
{
	cmd = DBGSYM::CliCommand::Disassemble;
	bHasStartAddress = false;
	bHasFinishAddress = false;
}

void CommandToken::SetTokenDisassembly(bit16 startaddress)
{
	cmd = DBGSYM::CliCommand::Disassemble;
	this->startaddress = startaddress;
	bHasStartAddress = true;
	bHasFinishAddress = false;
}

void CommandToken::SetTokenDisassembly(bit16 startaddress, bit16 finishaddress)
{
	cmd = DBGSYM::CliCommand::Disassemble;
	this->startaddress = startaddress;
	this->finishaddress = finishaddress;
	bHasStartAddress = true;
	bHasFinishAddress = true;
}

void CommandToken::SetTokenError(LPCTSTR pszErrortext)
{
	cmd = DBGSYM::CliCommand::Error;
	text.append(pszErrortext);
}

void CommandToken::SetTokenSelectCpu(DBGSYM::CliCpuMode::CliCpuMode cpumode, bool bViewCurrent)
{
	cmd = DBGSYM::CliCommand::SelectCpu;
	this->cpumode = cpumode;
	this->bViewCurrent = bViewCurrent;
}

void CommandToken::SetTokenShowCpu()
{
	cmd = DBGSYM::CliCommand::ShowCpu;
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

void CommandToken::SetTokenStep(bit64 stepClocks)
{
	cmd = DBGSYM::CliCommand::StepSystem;
	this->stepClocks = stepClocks;
}

void CommandToken::SetTokenReadMemory()
{
	cmd = DBGSYM::CliCommand::ReadMemory;
	bHasStartAddress = false;
	bHasFinishAddress = false;
}

void CommandToken::SetTokenReadMemory(bit16 startaddress)
{
	cmd = DBGSYM::CliCommand::ReadMemory;
	this->startaddress = startaddress;
	bHasStartAddress = true;
	bHasFinishAddress = false;
}

void CommandToken::SetTokenReadMemory(bit16 startaddress, bit16 finishaddress)
{
	cmd = DBGSYM::CliCommand::ReadMemory;
	this->startaddress = startaddress;
	this->finishaddress = finishaddress;
	bHasStartAddress = true;
	bHasFinishAddress = true;
}

void CommandToken::SetTokenWriteMemory(bit16 address, bit8 *pData, unsigned int bufferSize)
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

void CommandToken::SetTokenAssemble(bit16 address, bit8 *pData, unsigned int bufferSize)
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
