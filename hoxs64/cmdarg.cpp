#include <windows.h>
#include <tchar.h>
#include <assert.h>
#include "defines.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "carray.h"
#include "mlist.h"
#include "cmdarg.h"


CommandArg::CommandArg()
{
	pOption = NULL;
	pParam = NULL;
	ParamCount = 0;
}

CommandArg::~CommandArg()
{
	cleanup();
}

void CommandArg::cleanup()
{
	if (pOption)
	{
		free(pOption);
	}
	if (pParam != NULL && ParamCount > 0)
	{
		for (int i=0; i<ParamCount; i++)
		{
			if (pParam[i])
				free(pParam[i]);
			pParam[i] = NULL;
		}
		delete []pParam;
	}
	pOption = NULL;
	pParam = NULL;
	ParamCount = 0;
}

CParseCommandArg::CParseCommandArg()
{
	mLen = 0;
	mIndex = 0;
	mpsCmdLine = NULL;
}

bool CParseCommandArg::IsEOF()
{
	return mIndex >= mLen;
}

bool CParseCommandArg::IsWhiteSpace(TCHAR ch)
{
	switch (ch)
	{
	case _T(' '):
	case _T('\t'):
	case _T('\r'):
	case _T('\n'):
		return true;
	default:
		return false;
	}
}

HRESULT CParseCommandArg::ReadChar(TCHAR& ch)
{
	if (IsEOF())
		return E_FAIL;

	ch = mpsCmdLine[mIndex++];

	switch (ch)
	{
	case _T('\0'):
		return E_FAIL;
	}

	return S_OK;
}

void CParseCommandArg::UndoChar()
{
	if (mIndex>0)
		mIndex--;
}

HRESULT CParseCommandArg::ReadWord(int& count, bool& bGotWord)
{
int i=0;
bool bQuoteOpen = false;
TCHAR ch;
TCHAR qoute = _T('\0');
HRESULT hr; 
	
	ZeroMemory(&mWordBuffer[0], sizeof(mWordBuffer));
	mWordBuffer[0] = _T('\0');
	count = 0;
	bGotWord = false;
	
	while(1)
	{
		if (IsEOF())
			return S_OK;
		hr = ReadChar(ch);
		if (FAILED(hr))
			return hr;

		if (IsWhiteSpace(ch))
			continue;
		break;
	}

	if (ch == _T('\"') || ch == _T('\''))
	{
		qoute = ch;
		while(1)
		{
			if (IsEOF())
				break;
			hr = ReadChar(ch);
			if (FAILED(hr))
				return hr;
			if (ch == qoute)
				break;

			if (i >= (_countof(mWordBuffer) - 1) )
				return E_FAIL;

			mWordBuffer[i++] = ch;
		}
	}
	else
	{
		mWordBuffer[i++] = ch;
		while(1)
		{
			if (IsEOF())
				break;
			hr = ReadChar(ch);
			if (FAILED(hr))
				return hr;
			if (IsWhiteSpace(ch))
				break;
			else if (ch == _T('\"') || ch == _T('\''))
			{
				UndoChar();
				break;
			}

			if (i >= (_countof(mWordBuffer) - 1) )
				return E_FAIL;

			mWordBuffer[i++] = ch;
		}
	}

	bGotWord = true;
	mWordBuffer[i] = _T('\0');
	count = i;
	return S_OK;
}

HRESULT CParseCommandArg::Parse(const TCHAR *sCmdLine, CCommandArgArray *args)
{
CommandArg ca , caDefault;
HRESULT hr;
CListTString lstString;
int c;
bool bGotWord;

	if (sCmdLine == NULL)
		return E_FAIL;

	mLen = lstrlen(sCmdLine);
	mpsCmdLine = sCmdLine;
	mIndex = 0;

	while(1)
	{
		hr = ReadWord(c, bGotWord);
		if (FAILED(hr))
		{
			goto clean;
		}
		if (!bGotWord) //EOF reached
			break;

		if (lstString.Count() > 100)
		{
			hr = E_FAIL;
			goto clean;
		}

		TCHAR *s = _tcsdup(&mWordBuffer[0]);
		if (s==NULL)
		{
			hr =  E_OUTOFMEMORY;
			goto clean;
		}

		hr = lstString.Append(s);
		if (FAILED(hr))
		{
			free(s);
			s = NULL;
			goto clean;
		}
	}

	if (lstString.Count() == 0)
		return S_OK;

	CListElementTString *p;
	for (p = lstString.Head() ; p != NULL ;)
	{
		if (p == NULL)
			break;
		assert (lstrlen(p->m_data) > 0);
			
		hr = LoadCommandOption(&p, &ca);
		if (FAILED(hr))
		{
			goto clean;
		}
		hr = args->Append(ca);
		if (FAILED(hr))
		{
			goto clean;
		}
		//The args CArray is now contolling the life time of the strings 
		//so prevent local clean of ca which links to the same strings
		ca = caDefault;
		if (p == NULL)
			break;
	}
	hr = S_OK;

clean:
	for (p = lstString.Head() ; p != NULL ; p = p->Next())
	{
		if (p == NULL)
			break;
		if (p->m_data != NULL)
		{
			delete p->m_data;
			p->m_data = NULL;
		}
	}
	lstString.Clear();

	return hr;
}

int CParseCommandArg::GetParamCount(const CListElementTString *p)
{
	int c=0;
	while(p!=NULL)
	{
		if (p->m_data[0] == '-')
			break;
		p = p->Next();
		c++;
	}
	return c;
}

HRESULT CParseCommandArg::LoadCommandOption(CListElementTString **ppElement, CommandArg *commandArg)
{
int k=0,c=0;
CommandArg ca, caDefault;


	if (ppElement == NULL || commandArg == NULL)
		return E_POINTER;

	CListElementTString *p = *ppElement;
	if (p->m_data[0] == '-')
	{
		ca.pOption = _tcsdup(&p->m_data[0]);
		if (ca.pOption == NULL)
			return E_OUTOFMEMORY;

		p = p->Next();
	}
	else
	{
		ca.pOption = NULL;
	}

	k = GetParamCount(p);
	if (k == 0)
	{
		//option with no parameters
		ca.ParamCount = 0;
		ca.pParam = NULL;
	}
	else
	{
		ca.pParam = new TCHAR *[k];
		if (ca.pParam == NULL)
		{
			return E_OUTOFMEMORY;
		}
		//ZeroMemory(ca.pParam, sizeof(char *) * k);
		ca.ParamCount = k;

		for (c = 0; c<ca.ParamCount && p!=NULL; c++,p = p->Next())
		{
			ca.pParam[c] = _tcsdup(&p->m_data[0]);
		}
	}

	*ppElement = p;


	*commandArg = ca;
	//prevent local clean of ca whose memory pointers we a returning to the caller.
	ca = caDefault;
	return S_OK;
}

HRESULT CParseCommandArg::ParseCommandLine(const TCHAR *sCmdLine, CCommandArgArray **args)
{
HRESULT hr;
CParseCommandArg caa;

	*args = NULL;

	if (sCmdLine == NULL)
		return E_FAIL;

	CCommandArgArray *pArr = new CCommandArgArray();
	if (pArr == NULL)
		return E_OUTOFMEMORY;

	hr = pArr->Resize(30);
	if (FAILED(hr))
	{
		delete pArr;
		return E_OUTOFMEMORY;
	}

	hr = caa.Parse(sCmdLine, pArr);
	if (SUCCEEDED(hr))
	{
		*args = pArr;
		return S_OK;
	}
	else
	{
		delete pArr;
		return hr;
	}	
}


CommandArg *CParseCommandArg::FindOption(const CCommandArgArray *args, const TCHAR *sOption)
{
unsigned long i;
	for(i=0; i<args->Count(); i++)
	{
		TCHAR *opt;
		opt = (*args)[i].pOption;
		if (opt != NULL)
		{
			if (_tcsicmp((*args)[i].pOption, sOption) == 0)
				return &(*args)[i];
		}
	}
	return NULL;
}
