#include <windows.h>
#include <assert.h>
#include "bits.h"
#include "carray.h"
#include "mlist.h"
#include "bits.h"
#include "util.h"
#include "huff.h"
#include "FDI.h"

HuffDecompression::HuffDecompression()
{
	m_pStream=0;
}

HuffDecompression::~HuffDecompression()
{
	if (m_pStream)
	{
		m_pStream->Release();
		m_pStream=0;
	}
}

HRESULT HuffDecompression::SetFile(HANDLE hfile, bool bOwnFileHandle)
{
HRESULT hr = E_FAIL;
IStream *pstm = NULL;
	hr = FileStream::CreateObject(hfile, bOwnFileHandle, &pstm);
	if (SUCCEEDED(hr))
	{
		if (this->m_pStream)
		{
			this->m_pStream->Release();
			this->m_pStream = NULL;
		}
		this->m_pStream = pstm;
		pstm = NULL;
		InitSetFile();
	}
	return hr;
}

HRESULT HuffDecompression::SetFile(IStream *pStream)
{
	pStream->AddRef();
	if (this->m_pStream)
	{
		this->m_pStream->Release();
		this->m_pStream = NULL;
	}
	this->m_pStream = pStream;
	InitSetFile();
	return S_OK;
}

void HuffDecompression::InitSetFile()
{
	//writeError=false;
	//bufferPos=0;
}

HRESULT HuffDecompression::Decompress(DWORD numberOfDoubleWords, DWORD **data)
{
HuffNode *rootNode;
bit8 subStreamHeader1;
bit8 subStreamHeader2;
FDIStream fdiStream;
char currentByte;
short currentWord;
bit8 frame;
HuffNode *currentNode,*hn;
HRESULT hr;
DWORD i,v;
DWORD mask;
CHuffNodeArray nodeArray;
HuffNodeHolder nodeHolder;
int cnt = 0;
ULONG bytesRead = 0;
ULONG bytesToRead = 0;

	fdiStream.data = (bit8 *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, numberOfDoubleWords * 4);
	if (!fdiStream.data)
		return E_OUTOFMEMORY;

	do
	{
		cnt++;

		nodeArray.Clear();
		hr = nodeArray.Resize(256);
		if (FAILED(hr))
			return hr;
		hr = nodeHolder.Init(0x20000);
		if (FAILED(hr))
			return hr;

		rootNode = nodeHolder.Create();
		if (rootNode==NULL)
			return E_OUTOFMEMORY;
		currentNode = rootNode;

		bytesToRead = 1;
		hr = m_pStream->Read((char *)&subStreamHeader1, bytesToRead, &bytesRead);
		if (FAILED(hr) && GetLastError() != ERROR_HANDLE_EOF)
		{
			break;
		}
		else if (bytesRead < bytesToRead)
		{
			hr = E_FAIL;
		}
		if (FAILED(hr))
			return hr;

		bytesToRead = 1;
		hr = m_pStream->Read((char *)&subStreamHeader2, bytesToRead, &bytesRead);
		if (FAILED(hr) && GetLastError() != ERROR_HANDLE_EOF)
		{
			break;
		}
		else if (bytesRead < bytesToRead)
		{
			hr = E_FAIL;
		}
		if (FAILED(hr))
			return hr;

		fdiStream.highBitNumber = subStreamHeader2 & 0x7f;
		fdiStream.lowBitNumber = subStreamHeader1 & 0x7f;

		if (subStreamHeader1 & 0x80)
			fdiStream.bSignExtend = true;
		else
			fdiStream.bSignExtend = false;


		if (subStreamHeader2 & 0x80)
			fdiStream.bitSize = 16;
		else
			fdiStream.bitSize = 8;

		mask = ((DWORD)-1) >> (31 - fdiStream.highBitNumber);
		mask &=  ((DWORD)-1) << (fdiStream.lowBitNumber);

		//Build Huffman tree;
		frame = 0;
		i=0;
		while(1)
		{
			if (frame == 0)
			{
				frame = 8;
				bytesToRead = 1;
				hr = m_pStream->Read((char *)&currentByte, bytesToRead, &bytesRead);
				if (FAILED(hr) && GetLastError() != ERROR_HANDLE_EOF)
				{
					break;
				}
				else if (bytesRead < bytesToRead)
				{
					hr = E_FAIL;
				}
				if (FAILED(hr))
					return hr;
			}
			if (currentByte >= 0)
			{
				//high bit is a 0
				//currentNode is a "node"
				currentNode->AddLeft(hn = nodeHolder.Create());
				if (hn == NULL)
					return E_OUTOFMEMORY;
				currentNode->AddRight(hn = nodeHolder.Create());
				if (hn == NULL)
					return E_OUTOFMEMORY;
				currentNode = currentNode->leftNode;
			}
			else
			{
				//high bit is a 1
				//currentNode is a "leaf"
				currentNode->isLeaf = true;
				hr = nodeArray.Append(currentNode);
				if (FAILED(hr))
					return hr;
				currentNode = currentNode->FindDeepestRightNode();
				if (currentNode==0)
					break;
			}
			i++;
			if (i>0x20000)//tree limit
				return E_FAIL;

			currentByte <<= 1;
			frame--;
		}

		//Read tree values
		for (i = 0 ; i < nodeArray.Count(); i++)
		{
			if (fdiStream.bitSize == 16)
			{
				bytesToRead = 2;
				hr = m_pStream->Read((char *)&currentWord, bytesToRead, &bytesRead);
				if (FAILED(hr) && GetLastError() != ERROR_HANDLE_EOF)
				{
					break;
				}
				else if (bytesRead < bytesToRead)
				{
					hr = E_FAIL;
				}
				if (FAILED(hr))
					return hr;

				//Is this right?
				//Not really a correct decompression but it saves on another loop to
				//swap endianess of the decompressed data.
				currentWord = wordswap(currentWord);

				if (fdiStream.bSignExtend)
					nodeArray[i]->value = (unsigned long)(signed short)currentWord;
				else
					nodeArray[i]->value = (unsigned long)(unsigned short)currentWord;

			}
			else
			{
				bytesToRead = 1;
				hr = m_pStream->Read((char *)&currentByte, bytesToRead, &bytesRead);
				if (FAILED(hr) && GetLastError() != ERROR_HANDLE_EOF)
				{
					break;
				}
				else if (bytesRead < bytesToRead)
				{
					hr = E_FAIL;
				}
				if (FAILED(hr))
					return hr;

				if (fdiStream.bSignExtend)
					nodeArray[i]->value = (unsigned long)(signed char)currentByte;
				else
					nodeArray[i]->value = (unsigned long)(unsigned char)currentByte;
			}
		}


		//decode stream
		i=0;
		frame=0;
		for (i = 0 ; i < numberOfDoubleWords ; i++)
		{
			currentNode = rootNode;
			while (!currentNode->isLeaf)
			{
				if (frame == 0)
				{
					frame = 8;
					bytesToRead = 1;
					hr = m_pStream->Read((char *)&currentByte, bytesToRead, &bytesRead);
					if (FAILED(hr) && GetLastError() != ERROR_HANDLE_EOF)
					{
						break;
					}
					else if (bytesRead < bytesToRead)
					{
						hr = E_FAIL;
					}
					if (FAILED(hr))
						return hr;
				}
				if (currentByte >= 0)
				{
					if (currentNode->leftNode==0)
						return E_FAIL;
					currentNode = currentNode->leftNode;
				}
				else
				{
					if (currentNode->rightNode==0)
						return E_FAIL;
					currentNode = currentNode->rightNode;
				}

				currentByte <<= 1;
				frame--;
			}

			v = currentNode->value;
			v = v << fdiStream.lowBitNumber;
			v = v & mask;
			bit32 x = ((bit32 *)fdiStream.data)[i];
			x = (x & ~mask) | v;
			((bit32 *)fdiStream.data)[i] = x;
		}
	} while (fdiStream.lowBitNumber != 0);

	*data = (DWORD *)fdiStream.data;
	fdiStream.data = 0;
	return S_OK;
}
