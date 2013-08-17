#include <windows.h>
#include <assert.h>
#include "defines.h"
#include "carray.h"
#include "mlist.h"
#include "bits.h"
#include "util.h"
#include "huff.h"

int MyCompare(HuffNode *&a,HuffNode *&b)
{
	if (a==NULL || b==NULL)
	{
		if (b!=NULL)
			return -1;
		else if (a!=NULL)
			return 1;
		else
			return 0;

	}
	else
		return (a->weight - b->weight);
}


/***********************************************************************************************************************
HuffCompression Class
***********************************************************************************************************************/
HuffCompression::HuffCompression()
{
	m_pStream=0;
	huffTable = NULL;
}
HuffCompression::~HuffCompression()
{
	if (huffTable)
		GlobalFree(huffTable);
	huffTable = NULL;
	if (m_pStream)
	{
		m_pStream->Release();
		m_pStream=0;
	}
}

HRESULT HuffCompression::Init()
{
	huffTable = (HuffTable *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(class HuffTable) * HUFFSYMBOLS);
	if (huffTable)
		return S_OK;
	else
		return E_OUTOFMEMORY;
}

void HuffCompression::HuffWriteBit(bit8 data)
{
bit8 i,m,v;
HRESULT hr;
DWORD bytes_written;

	if (bufferPos +1 > HUFFBUFFERSIZE*8)
	{
		if (!writeError && m_pStream!=NULL)
		{
			hr = m_pStream->Write(buffer, HUFFBUFFERSIZE, &bytes_written);
			if (FAILED(hr))
				writeError = true;
		}
		bufferPos = 0;
	}
	i = buffer[bufferPos/8];
	m = 1 << (7-(bufferPos & 7));
	v = data << (7-(bufferPos & 7));
	buffer[bufferPos/8] = (i & ~m) | v;
	bufferPos = (bufferPos + 1);
	outLength+=1;
}

void HuffCompression::HuffWriteByte(bit8 data)
{
bit8 i,m,*p,shift,v;
DWORD bytes_written;
HRESULT hr;
	
	if (bufferPos +8 > HUFFBUFFERSIZE*8)
	{
		if (!writeError && m_pStream!=0)
		{
			hr = m_pStream->Write(buffer, bufferPos/8, &bytes_written);
			if (FAILED(hr))
				writeError = true;
		}
		buffer[0] = buffer[HUFFBUFFERSIZE-1];
		bufferPos &= 7;
	}
	p = &buffer[bufferPos/8];
	shift = (bit8)bufferPos & 7;
	i = *p;
	m = 0xff;
	m = m << (8-shift);
	v = data >> shift;
	*p = (i & m) | v;
	if (shift>0)
	{
		p++;
		i = *p;
		m = 0xff >> shift;
		v = data << (8-shift);
		*p = (i & m) | v;
	}
	bufferPos = (bufferPos + 8);
	outLength+=8;
}

void HuffCompression::HuffWriteWord(bit16 data)
{
	HuffWriteByte((bit8)(data>>8));
	HuffWriteByte((bit8)(data & 0xff));
}

void HuffCompression::HuffEndWrite()
{
bit8 i,m,*p,shift;
HRESULT hr;
DWORD bytes_written;

	shift = (bit8)bufferPos & 7;
	if (shift>0)
	{
		p = &buffer[bufferPos/8];
		i = *p;
		m = 0xff << (8-shift);
		*p = (i & m);
		bufferPos = (bufferPos + (8-shift));
	}
	if (!writeError && m_pStream!=0)
	{
		hr = m_pStream->Write(buffer, bufferPos/8, &bytes_written);
		if (FAILED(hr))
			writeError = true;
	}
	bufferPos=0;
}

void HuffCompression::HuffWalkTreeBits(HuffNode *node)
{
	assert(node!=0);
	//assert(pathLength<=16);
	assert(pathLength<=32);
	if (node->isLeaf)
	{
		assert(node->leftNode==NULL);
		assert(node->rightNode==NULL);
		HuffWriteBit(1);
		huffTable[node->value].path = path;
		huffTable[node->value].pathLength = pathLength;
	}
	else
	{
		assert(node->leftNode!=NULL);
		assert(node->rightNode!=NULL);
		HuffWriteBit(0);
		pathLength++;
		path <<= 1;
		HuffWalkTreeBits(node->leftNode);
		path |= 1;
		HuffWalkTreeBits(node->rightNode);
		pathLength--;
		path >>= 1;
	}
}

void HuffCompression::HuffWalkTreeValues(HuffNode *node)
{
	assert(node!=0);
	if (node->isLeaf)
	{
		assert(node->leftNode==NULL);
		assert(node->rightNode==NULL);
		HuffWriteWord((bit16)node->value);
	}
	else
	{
		assert(node->leftNode!=NULL);
		assert(node->rightNode!=NULL);
		HuffWalkTreeValues(node->leftNode);
		HuffWalkTreeValues(node->rightNode);
	}
}

HRESULT HuffCompression::SetFile(HANDLE hfile, bool bOwnFileHandle)
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

HRESULT HuffCompression::SetFile(IStream *pStream)
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

void HuffCompression::InitSetFile()
{
	writeError=false;
	bufferPos=0;
}

HRESULT HuffCompression::Compress(bit32 *src, bit32 srcSize, bit32 *dstSize)
{
bit32 i;
CHuffList list;
HuffNode *hn=NULL;
HuffNode *recentParent=NULL;
HuffNode *parent=NULL;
HuffNode *lChild=NULL;
HuffNode *rChild=NULL;
bit16 v16,s;
bit32 v32;
bit8 pl;
int streamNumber;
bool streamRequired;
CHuffListElement *pSearchListElement;
HuffNodeHolder nodeHolder;
HRESULT hr;

	pathLength=0;
	path=0;
	bufferPos = 0;
	writeError=0;
	outLength=0;
	hr = nodeHolder.Init(0x20000);
	if (FAILED(hr))
		return hr;

	for (streamNumber=1 ; streamNumber>=0 ; streamNumber--)
	{
		if (srcSize==0 || src==NULL)
			return E_FAIL;


		ZeroMemory(&huffTable[0],  sizeof(HuffTable) * HUFFSYMBOLS);

		//get symbol weightings
		streamRequired = false;
		for (i=0 ; i < srcSize ; i++)
		{
			if (streamNumber==0)
				v16 = (bit16)(src[i]);
			else
				v16 = (bit16)(src[i]>>16);
			huffTable[v16].weight++;
			if(v16)
				streamRequired=true;
		}

		if ((streamNumber!=0) && !streamRequired)
			continue;

		list.Clear();
		//add symbols to list
		for (i=0 ; i < HUFFSYMBOLS ; i++)
		{
			if (huffTable[i].weight!=0)
			{
				hn = nodeHolder.Create();
				if (hn==NULL)
					return E_OUTOFMEMORY;
				hn->value = i;
				hn->weight = huffTable[i].weight;
				hn->isLeaf = true;
				hr = list.Append(hn);
				if (FAILED(hr))
					return hr;
			}
		}
		//sort symbol list based in weights: lowest first.
		list.MergeSort(&MyCompare);

		//build Huffman tree
		pSearchListElement=NULL;
		while (list.Count()>1)
		{
			//take lowest two symbols and make them into a parent with two children
			parent = nodeHolder.Create();
			if (parent==NULL)
				return E_OUTOFMEMORY;
			lChild = list.Head()->m_data;
			list.Head()->m_data = 0;//prevent clean during the remove;
			//TRIAL
			if (pSearchListElement == list.Head() || pSearchListElement == list.Head()->Next())
				pSearchListElement = NULL;
			list.Remove(list.Head());

			rChild = list.Head()->m_data;

			list.Head()->m_data = parent;

			parent->AddLeft(lChild);
			parent->AddRight(rChild);
			parent->weight = lChild->weight + rChild->weight;
			parent->isLeaf = false;
			//re-sort the new parent huff node;
			//TRIAL
			if (pSearchListElement==NULL)
				pSearchListElement=list.Head()->Next();
			if (pSearchListElement!=NULL)
			{
				hn  = pSearchListElement->m_data;
				while (pSearchListElement!=NULL)
				{
					hn  = pSearchListElement->m_data;
					if (hn->weight >= parent->weight)
						break;
					pSearchListElement = pSearchListElement->Next();
				}
				if (pSearchListElement==NULL)
				{
					list.Head()->MoveToAfter(list.Tail());
					pSearchListElement=list.Tail();
				}
				else
				{
					list.Head()->MoveToBefore(pSearchListElement);
					pSearchListElement=pSearchListElement->Prev();
				}
			}
		}

		if (streamNumber==0)
		{
			HuffWriteByte(0x00);
			HuffWriteByte(0x8f);
		}
		else
		{
			HuffWriteByte(0x10);
			HuffWriteByte(0x9f);
		}

		hn = list.Head()->m_data;
		if (hn->isLeaf)
			return E_FAIL;

		path=0;
		pathLength=0;
		HuffWalkTreeBits(hn);
		HuffEndWrite();
		HuffWalkTreeValues(hn);
		HuffEndWrite();
		if (writeError)
			return E_FAIL;
		for (i=0; i < srcSize; i++)
		{
			if (streamNumber==0)
				s = (bit16)(src[i]);
			else
				s = (bit16)(src[i]>>16);
			v32 = huffTable[s].path;
			pl = huffTable[s].pathLength;

			assert(pl>0 && pl<=32);

			v32 <<= (32-pl);
			while (pl>0)
			{

				if ((signed long)v32 < 0)
					HuffWriteBit(1);
				else
					HuffWriteBit(0);
				pl--;
				v32<<=1;
			}
			if (writeError)
				return E_FAIL;
		}
		HuffEndWrite();
		if (writeError)
			return E_FAIL;
	}

	if (writeError)
		return E_FAIL;
	*dstSize= (outLength+7)/8;
	return S_OK;
}
