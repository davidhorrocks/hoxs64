#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include "bits.h"
#include "assert.h"
#include "mlist.h"
#include "carray.h"
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

HRESULT Assembler::AssembleText(LPCTSTR pszText, bit8 *pCode, int iBuffersize, int *piBytesWritten)
{
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

	AssemblyToken tk;
	GetNextToken();
	GetNextToken();
	if (m_CurrentToken.Symbol == AssemblyToken::EndOfInput)
	{
		return E_FAIL;
	}
	else if (m_CurrentToken.Symbol == AssemblyToken::IdentifierString)
	{
		GetNextToken();
		if (m_CurrentToken.Symbol == AssemblyToken::EndOfInput)
		{
			//Implied adderssing.
			return S_OK;
		}
		else if (m_CurrentToken.Symbol == AssemblyToken::Number8)
		{
			GetNextToken();
			if (m_CurrentToken.Symbol == AssemblyToken::EndOfInput)
			{
				return S_OK;
			}
			else if (m_CurrentToken.Symbol == AssemblyToken::Symbol)
			{
				if (m_CurrentToken.SymbolChar == _T(','))
				{
					GetNextToken();
					if (m_CurrentToken.SymbolChar == _T('X'))
					{
					}
					else if (m_CurrentToken.SymbolChar == _T('Y'))
					{
					}
					else
					{
						return E_FAIL;
					}
				}
				else
				{
					return E_FAIL;
				}
			}
			else
			{
				return E_FAIL;
			}
		}
		else if (m_CurrentToken.Symbol == AssemblyToken::Number16)
		{
			GetNextToken();
			if (m_CurrentToken.Symbol == AssemblyToken::EndOfInput)
			{
				return S_OK;
			}
			else if (m_CurrentToken.Symbol == AssemblyToken::Symbol)
			{
				if (m_CurrentToken.SymbolChar == _T(','))
				{
					GetNextToken();
					if (m_CurrentToken.SymbolChar == _T('X'))
					{
					}
					else if (m_CurrentToken.SymbolChar == _T('Y'))
					{
					}
					else
					{
						return E_FAIL;
					}
				}
				else
				{
					return E_FAIL;
				}
			}
			else
			{
				return E_FAIL;
			}
		}
		else if (m_CurrentToken.Symbol == AssemblyToken::Symbol)
		{
			if (m_CurrentToken.SymbolChar == _T('#'))
			{
				//Immediate adderssing.
				GetNextToken();
				if (m_CurrentToken.Symbol == AssemblyToken::Number8)
				{
				}
				else
				{
					return E_FAIL;
				}
			}
			else if (m_CurrentToken.SymbolChar == _T('('))
			{
				GetNextToken();
				if (m_CurrentToken.Symbol == AssemblyToken::Number8)
				{
					GetNextToken();
					if (m_CurrentToken.Symbol == AssemblyToken::Symbol)
					{
						if (m_CurrentToken.SymbolChar == _T(','))
						{
							GetNextToken();
							if (m_CurrentToken.SymbolChar == _T('X'))
							{
							}
							else if (m_CurrentToken.SymbolChar == _T('Y'))
							{
							}
							else
							{
								return E_FAIL;
							}
						}
						else if (m_CurrentToken.SymbolChar == _T(')'))
						{
						}
						else
						{
							return E_FAIL;
						}
					}
					else
					{
						return E_FAIL;
					}
				}
				else if (m_CurrentToken.Symbol == AssemblyToken::Number16)
				{
					GetNextToken();
					if (m_CurrentToken.Symbol == AssemblyToken::Symbol)
					{
						if (m_CurrentToken.SymbolChar == _T(','))
						{
							GetNextToken();
							if (m_CurrentToken.SymbolChar == _T('X'))
							{
							}
							else if (m_CurrentToken.SymbolChar == _T('Y'))
							{
							}
							else
							{
								return E_FAIL;
							}
						}
						else if (m_CurrentToken.SymbolChar == _T(')'))
						{
						}
						else
						{
							return E_FAIL;
						}
					}
					else
					{
						return E_FAIL;
					}
				}
				else
				{
					return E_FAIL;
				}
			}
			else
			{
				return E_FAIL;
			}
		}
	}
	else
	{
		return E_FAIL;
	}
	return S_OK;
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
			else if (ch == _T('$'))
			{
				iHexDigitCount = 0;
				m_LexState = LexState::HexString;
				GetNextChar();
				break;
			}
			else if (ch == _T('#') || ch == _T('(') || ch == _T(')'))
			{
				m_NextToken = AssemblyToken();
				m_NextToken.TokenType = AssemblyToken::Symbol;
				m_NextToken.SymbolChar = ch;
				GetNextChar();
				return;
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
				_tcsncpy_s(m_NextToken.IdentifierText, _countof(m_NextToken.IdentifierText), m_bufIdentifierString, _countof(m_bufIdentifierString));
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
				else
					m_bufNumber = m_bufNumber + (int)(ch - _T('A')) + 10;
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

/*
	GetNextToken();
	if (current_token.TType == TOKEN_EOF)
		return E_FAIL;
	if (current_token.TType != TOKEN_MNEMONIC)
		return E_FAIL;
	GetNextToken();
	if (current_token.TType == TOKEN_EOF)
		return E_FAIL;

	if (current_token.TType == TOKEN_NUMBER)
	{
		GetNextToken();
		if (current_token.TType != TOKEN_MNEMONIC)
	}
*/