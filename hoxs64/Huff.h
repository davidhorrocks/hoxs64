#ifndef __HUFF_H__ 
#define __HUFF_H__ 

#define HUFFBUFFERSIZE (32)

#define HUFFSYMBOLS (0x10000)


class HuffNode
{
public:
	HuffNode();
	~HuffNode();

	class HuffNode &operator=(class HuffNode &rhs);
	//void *operator new(size_t size);
	//void  operator delete (void *p);
	HuffNode *FindDeepestRightNode();
	void AddLeft(class HuffNode *);
	void AddRight(class HuffNode *);
	bool IsLeftChild();
	bool IsRightChild();

	class HuffNode *parentNode;
	class HuffNode *leftNode;
	class HuffNode *rightNode;
	bit32 value;
	bit32 weight;
	bool isLeaf;
};

class HuffNodeHolder
{
public:
	HuffNodeHolder();
	~HuffNodeHolder();
	HRESULT Init(unsigned long size);
	HuffNode *Create();
private:
	void CleanUp();
	CArray<HuffNode *> holder;
};

class HuffTable
{
public:
	bit32 weight;
	bit32 path;
	bit8 pathLength;
};

typedef class CArray<HuffNode *> CHuffNodeArray;
typedef class MList<HuffNode *> CHuffList;
typedef class MListElement<HuffNode *> CHuffListElement;


class HuffWork
{
public:
	HuffWork();
	~HuffWork();

	HRESULT Init();
	HRESULT HuffCompress(bit32 *src, bit32 srcSize, bit32 *dstSize);
	void HuffWriteBit(bit8 data);
	void HuffWriteByte(bit8 data);
	void HuffWriteWord( bit16 data);

	void HuffWalkTreeBits(HuffNode *node);
	void HuffWalkTreeValues(HuffNode *node);
	void HuffSetFile(HANDLE hfile);
	void HuffEndWrite();

private:
	HuffTable *huffTable;
	bit8 buffer[HUFFBUFFERSIZE];
	bit32 bufferPos;
	HANDLE hfile;
	bool writeError;
	bit32 path;
	bit8 pathLength;
	bit32 outLength;
};



#endif