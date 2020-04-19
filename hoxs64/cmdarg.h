#pragma once
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
public:
	CParseCommandArg(const TCHAR *sCmdLine);
	~CParseCommandArg();

	HRESULT ParseCommandLine(const TCHAR *sCmdLine);
	CommandArg *FindOption(const TCHAR *sOption) const;

	
private:
	CCommandArgArray *pArgs;
	int mLen;
	const TCHAR *mpsCmdLine;
	int mIndex;
	TCHAR mWordBuffer[300];
	
	void Init();
	void CleanUp();

	HRESULT Parse(const TCHAR *sCmdLine, CCommandArgArray *args);
	HRESULT ReadChar(TCHAR& ch, TCHAR& nextch);
	void UndoChar();
	HRESULT ReadWord(int &count, bool& bGotWord);
	bool IsEOF();
	bool IsWhiteSpace(TCHAR ch);

	static int GetParamCount(const CListElementTString *p);
	static HRESULT LoadCommandOption(CListElementTString **ppElement, CommandArg *commandArg);

};
