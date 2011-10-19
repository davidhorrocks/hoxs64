#ifndef __ASSEMBLER_H__
#define __ASSEMBLER_H__

#define MAX_IDENTIFIER_SIZE 10
struct AssemblyToken
{
	AssemblyToken();
	enum tagTType
	{
		EndOfInput,
		IdentifierString,
		Number8,
		Number16,
		Symbol,
		Error
	};
	tagTType TokenType;
	TCHAR IdentifierText[MAX_IDENTIFIER_SIZE];
	TCHAR SymbolChar;
	bit8 Value8;
	bit16 Value16;

	static void SetEOF(AssemblyToken* t);
	static void SetError(AssemblyToken* t);
};

class Assembler
{
public:
	//Assembler();
	HRESULT AssembleText(LPCTSTR pszText, bit8 *pCode, int iBuffersize, int *piBytesWritten);
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
	TCHAR m_bufIdentifierString[MAX_IDENTIFIER_SIZE];
	int m_ibufPos;
	int m_bufNumber;
	bool m_bTokenSuppressGetChar;

	void GetNextToken();
	HRESULT GetNextChar();
	bool IsWhiteSpace(TCHAR ch);
	bool IsLetter(TCHAR ch);
	bool IsDigit(TCHAR ch);
	bool IsHexDigit(TCHAR ch);

	bool AppendToIdentifierString(TCHAR ch);
};

#endif
