#ifndef __CMDARG_H__
#define __CMDARG_H__

typedef class MList<TCHAR *> CListTString;
typedef class MListElement<TCHAR *> CListElementTString;

struct CommandArg
{
public:
	CommandArg();
	virtual ~CommandArg();
	TCHAR *pOption;
	TCHAR **pParam;
	int ParamCount;

	void cleanup();
};

typedef class CArray<CommandArg> CCommandArgArray;



class CParseCommandArg
{
private:
	CParseCommandArg();
public:
	static HRESULT ParseCommandLine(const TCHAR *sCmdLine, CCommandArgArray **args);
	static CommandArg *FindOption(const CCommandArgArray *args, const TCHAR *sOption);

	
private:
	HRESULT Parse(const TCHAR *sCmdLine, CCommandArgArray *args);
	HRESULT ReadChar(TCHAR& ch);
	void UndoChar();
	HRESULT ReadWord(int &count, bool& bGotWord);
	int mLen;
	const TCHAR *mpsCmdLine;
	int mIndex;
	TCHAR mWordBuffer[300];
	bool IsEOF();
	bool IsWhiteSpace(TCHAR ch);

	static int GetParamCount(const CListElementTString *p);
	static HRESULT LoadCommandOption(CListElementTString **ppElement, CommandArg *commandArg);


public:
	CCommandArgArray CommandArgArray;
};

#endif