#ifndef __CRC_H__
#define __CRC_H__

#define CRC32POLY 0x04C11DB7L

class CRC32
{
public:
	CRC32();
	CRC32(DWORD poly, DWORD init, DWORD xorOut, bool isReflected);
	~CRC32();
	static DWORD Reflect(DWORD);
	void Init();
	void Init(DWORD poly, DWORD init, DWORD xorOut, bool isReflected);
	void ProcessByte(BYTE b);

	DWORD Value();
	
private:
	DWORD CrcTable[256];
	DWORD poly,reg,init,xorOut;
	bool isReflected;
};

class CRC32Alloc
{
public:
	CRC32Alloc(DWORD poly, DWORD init, DWORD xorOut, bool isReflected);
	CRC32Alloc();
	~CRC32Alloc();

	bool isOK;
	CRC32 *pCRC32;
};
#endif
