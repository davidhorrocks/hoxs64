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
HuffWork Class
***********************************************************************************************************************/
HuffWork::HuffWork()
{
	hfile=0;
	huffTable = NULL;
}
HuffWork::~HuffWork()
{
	if (huffTable)
		GlobalFree(huffTable);
	huffTable = NULL;
	if (hfile)
	{
		CloseHandle(hfile);
		hfile=0;
	}
}

HRESULT HuffWork::Init()
{
	huffTable = (HuffTable *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(class HuffTable) * HUFFSYMBOLS);
	if (huffTable)
		return S_OK;
	else
		return E_OUTOFMEMORY;
}

void HuffWork::HuffWriteBit(bit8 data)
{
bit8 i,m,v;
BOOL rb;
DWORD bytes_written;

	if (bufferPos +1 > HUFFBUFFERSIZE*8)
	{
		if (!writeError && hfile!=0)
		{
			rb = WriteFile(hfile, buffer, HUFFBUFFERSIZE, &bytes_written, NULL);
			if (rb==FALSE)
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

void HuffWork::HuffWriteByte(bit8 data)
{
bit8 i,m,*p,shift,v;
DWORD bytes_written;
BOOL rb;
	
	if (bufferPos +8 > HUFFBUFFERSIZE*8)
	{
		if (!writeError && hfile!=0)
		{
			rb = WriteFile(hfile, buffer, bufferPos/8, &bytes_written, NULL);
			if (rb==FALSE)
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


void HuffWork::HuffWriteWord(bit16 data)
{
	HuffWriteByte((bit8)(data>>8));
	HuffWriteByte((bit8)(data & 0xff));
}

void HuffWork::HuffEndWrite()
{
bit8 i,m,*p,shift;
BOOL rb;
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
	if (!writeError && hfile!=0)
	{
		rb = WriteFile(hfile, buffer, bufferPos/8, &bytes_written, NULL);
		if (rb==FALSE)
			writeError = true;
	}
	bufferPos=0;
}

void HuffWork::HuffWalkTreeBits(HuffNode *node)
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

void HuffWork::HuffWalkTreeValues(HuffNode *node)
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

void HuffWork::HuffSetFile(HANDLE hfile)
{
	HuffWork::hfile = hfile;
	writeError=false;
	bufferPos=0;
}

HRESULT HuffWork::HuffCompress(bit32 *src, bit32 srcSize, bit32 *dstSize)
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

/***********************************************************************************************************************
HuffNode Class
***********************************************************************************************************************/
HuffNode::HuffNode()
{
	leftNode=0;
	rightNode=0;
	parentNode=0;
	value=0;
	isLeaf=false;
};

HuffNode::~HuffNode()
{
	parentNode=leftNode=rightNode=0;
}


/*
void *HuffNode::operator new(size_t size)
{
	return GlobalAlloc(GMEM_FIXED, size);
}

void HuffNode::operator delete (void *p)
{
	GlobalFree(p);
}
*/
class HuffNode &HuffNode::operator=(class HuffNode &rhs)
{
	leftNode=rhs.leftNode;
	rightNode=rhs.rightNode;
	parentNode=rhs.parentNode;
	value=rhs.value;
	isLeaf = rhs.isLeaf;
	weight = rhs.weight;
	return rhs;
}

void HuffNode::AddLeft(class HuffNode *n)
{
	if (leftNode)
	{
		delete leftNode;
		leftNode = 0;
	}
	leftNode = n;
	if (n)
		n->parentNode = this;
}

void HuffNode::AddRight(class HuffNode *n)
{
	if (rightNode)
	{
		delete rightNode;
		rightNode = 0;
	}
	rightNode = n;
	if (n)
		n->parentNode = this;
}

HuffNode *HuffNode::FindDeepestRightNode()
{
HuffNode *n;
bool upFromRight;

	upFromRight = false;
	n=this;
	while (1)
	{
		if (!n->isLeaf && !upFromRight)
		{
			if (n->rightNode == 0)
				return n;
			if (!n->rightNode->isLeaf)
			{
				n = n->rightNode;
				upFromRight = false;
				continue;
			}

		}
		if (n->parentNode == 0)
			return 0;
		upFromRight = n->IsRightChild();
		n = n->parentNode;
	}
}

bool HuffNode::IsLeftChild()
{
	if (parentNode)
	{
		return (parentNode->leftNode == this);
	}
	return false;
}

bool HuffNode::IsRightChild()
{
	if (parentNode)
	{
		return (parentNode->rightNode == this);
	}
	return false;
}


/***********************************************************************************************************************
HuffNodeHolder Class
***********************************************************************************************************************/
HuffNodeHolder::HuffNodeHolder()
{
	holder.Resize(2048);
}
HuffNodeHolder::~HuffNodeHolder()
{
	CleanUp();
}

void HuffNodeHolder::CleanUp()
{
	bit32 i;
	HuffNode *h;
	for (i = 0 ; i<holder.Count() ; i++)
	{
		h = holder[i];
		if (h)
		{
			delete h;
			holder[i] = 0;
		}
	}
}

HRESULT HuffNodeHolder::Init(unsigned long size)
{
	CleanUp();
	holder.ClearCount();
	return holder.Resize(size);
}

HuffNode *HuffNodeHolder::Create()
{
HRESULT hr;
	HuffNode *h;
	h = new HuffNode;
	if(h==NULL)
		return NULL;
	hr = holder.Append(h);
	if(FAILED(hr))
	{
		delete h;
		return NULL;
	}
	return h;
}
