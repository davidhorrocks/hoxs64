#pragma once

struct AssemblyToken
{
	static const int MAX_IDENTIFIER_SIZE = 10;

	AssemblyToken();
	enum tagTType
	{
		EndOfInput,
		IdentifierString,
		Number8,
		Number16,
		Number32,
		Number64,
		Symbol,
		Error
	};
	tagTType TokenType;
	TCHAR IdentifierText[MAX_IDENTIFIER_SIZE];
	TCHAR SymbolChar;
	bit64 Value;

	static void SetEOF(AssemblyToken* t);
	static void SetError(AssemblyToken* t);
};

class CommandToken;

class Assembler
{
public:
	Assembler();
	HRESULT AssembleText(bit16 address, LPCTSTR pszText, bit8 *pCode, unsigned int iBuffersize, unsigned int *piBytesWritten);
	HRESULT ParseAddress16(LPCTSTR pszText, bit16 *piAddress);
	HRESULT CreateCliCommandToken(LPCTSTR pszText, CommandToken **ppcmdr);
	CommandToken *GetCommandTokenAssembleLine();
	CommandToken* GetCommandTokenStep();
	CommandToken *GetCommandTokenReadMemory();
	CommandToken *GetCommandTokenWriteMemory();
	CommandToken *GetCommandTokenMapMemory();
	CommandToken *GetCommandTokenDisassembleLine();
	CommandToken *GetCommandTokenCpu();
	CommandToken *GetCommandTokenShowCpu();
	void SetRadix(DBGSYM::MonitorOption::Radix radix);
	//DBGSYM::MonitorOption::Radix GetRadix();
	static HRESULT TryParseAddress16(LPCTSTR pszText, DBGSYM::MonitorOption::Radix radix, bit16 *piAddress);
private:
	struct LexState
	{
		typedef enum tagELexState
		{
			Start,
			IdentifierString,
			Number,
			HexString,
			Finish,
			Error
		} ELexState;
	};

	static __int64 constexpr MaxAllowedNumber = 0xffffffffffff;

	TCHAR m_CurrentCh;
	TCHAR m_NextCh;
	bool m_bCurrentEOF;
	bool m_bNextEOF;
	AssemblyToken m_CurrentToken;
	AssemblyToken m_NextToken;	
	bool m_bIsStartChar;
	bool m_bIsStartToken;
	int m_iLenText;
	LexState::ELexState m_LexState;
	LPCTSTR m_pszText;
	int m_pos;
	TCHAR m_bufIdentifierString[AssemblyToken::MAX_IDENTIFIER_SIZE];
	int m_ibufPos;
	__int64 m_bufNumber;
	DBGSYM::MonitorOption::Radix radix;

	HRESULT InitParser(LPCTSTR pszText);
	void GetNextToken();
	HRESULT GetNextChar();
	bool TryConvertIdentifierTokenToHexNumber(AssemblyToken &);
	bool IsWhiteSpace(TCHAR ch);
	bool IsLetter(TCHAR ch);
	bool IsDigit(TCHAR ch);
	bool IsHexDigit(TCHAR ch);
	bool AppendToIdentifierString(TCHAR ch);
	HRESULT instcopy(bit8 *pDest, unsigned int iSizeDest, bit8 *pSrc, unsigned int iSizeSrc, unsigned int *piBytesWritten);

	HRESULT AssembleBytes(bit16 address, bit8 *pCode, unsigned int iBuffersize, unsigned int *piBytesWritten);

	HRESULT AssembleOneInstruction(bit16 address, bit8 *pCode, unsigned int iBuffersize, unsigned int *piBytesWritten);

	HRESULT AssembleImplied(LPCTSTR pszMnemonic, bit8 *pCode, unsigned int iBuffersize, unsigned int *piBytesWritten);

	HRESULT AssembleImmediate(LPCTSTR pszMnemonic, bit8 arg, bit8 *pCode, unsigned int iBuffersize, unsigned int *piBytesWritten);

	HRESULT AssembleZeroPage(LPCTSTR pszMnemonic, bit8 arg, bit8 *pCode, unsigned int iBuffersize, unsigned int *piBytesWritten);

	HRESULT AssembleZeroPageX(LPCTSTR pszMnemonic, bit8 arg, bit8 *pCode, unsigned int iBuffersize, unsigned int *piBytesWritten);

	HRESULT AssembleZeroPageY(LPCTSTR pszMnemonic, bit8 arg, bit8 *pCode, unsigned int iBuffersize, unsigned int *piBytesWritten);

	HRESULT AssembleAbsolute(LPCTSTR pszMnemonic, bit16 arg, bit8 *pCode, unsigned int iBuffersize, unsigned int *piBytesWritten);

	HRESULT AssembleAbsoluteX(LPCTSTR pszMnemonic, bit16 arg, bit8 *pCode, unsigned int iBuffersize, unsigned int *piBytesWritten);

	HRESULT AssembleAbsoluteY(LPCTSTR pszMnemonic, bit16 arg, bit8 *pCode, unsigned int iBuffersize, unsigned int *piBytesWritten);

	HRESULT AssembleIndirect(LPCTSTR pszMnemonic, bit16 arg, bit8 *pCode, unsigned int iBuffersize, unsigned int *piBytesWritten);

	HRESULT AssembleIndirectX(LPCTSTR pszMnemonic, bit8 arg, bit8 *pCode, unsigned int iBuffersize, unsigned int *piBytesWritten);

	HRESULT AssembleIndirectY(LPCTSTR pszMnemonic, bit8 arg, bit8 *pCode, unsigned int iBuffersize, unsigned int *piBytesWritten);

	HRESULT AssembleRelative(LPCTSTR pszMnemonic, bit8 arg, bit8 *pCode, unsigned int iBuffersize, unsigned int *piBytesWritten);

	HRESULT ParseAddressRange(bit16 *piStartAddress, bit16 *piEndAddress);
	HRESULT ParseAddress(bit16 *piAddress);
	HRESULT ParseNumber16(bit16 *piNumber);
	HRESULT ParseNumber16(bit16 *piNumber, bool *pIs16bit);
	bool TryParseHexWord(LPCTSTR str, bit16& result);
	bool TryParseHex(LPCTSTR str, bit64& result);
	HRESULT ParseNumber(bit64& result);
};
