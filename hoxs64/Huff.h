#ifndef __HUFF_H__ 
#define __HUFF_H__ 

#include "filestream.h"

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


class HuffCompression
{
public:
	HuffCompression();
	~HuffCompression();

	HRESULT Init();
	HRESULT Compress(bit32 *src, bit32 srcSize, bit32 *dstSize);
	HRESULT SetFile(HANDLE hfile, bool bOwnFileHandle);
	HRESULT SetFile(IStream *pStream);
private:
	void HuffWriteBit(bit8 data);
	void HuffWriteByte(bit8 data);
	void HuffWriteWord( bit16 data);

	void HuffWalkTreeBits(HuffNode *node);
	void HuffWalkTreeValues(HuffNode *node);
	void HuffEndWrite();

private:
	void InitSetFile();

	HuffTable *huffTable;
	bit8 buffer[HUFFBUFFERSIZE];
	bit32 bufferPos;
	//HANDLE hfile;
	IStream *m_pStream;
	bool writeError;
	bit32 path;
	bit8 pathLength;
	bit32 outLength;
};

class HuffDecompression
{
public:
	HuffDecompression();
	~HuffDecompression();

	HRESULT SetFile(HANDLE hfile, bool bOwnFileHandle);
	HRESULT SetFile(IStream *pStream);

	HRESULT Decompress(DWORD numberOfDoubleWords, DWORD **data);

private:
	void InitSetFile();

	IStream *m_pStream;
};

#endif