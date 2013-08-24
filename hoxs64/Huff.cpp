#include <windows.h>
#include <assert.h>
#include "bits.h"
#include "carray.h"
#include "mlist.h"
#include "huff.h"

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
