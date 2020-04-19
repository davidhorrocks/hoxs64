#pragma once

typedef unsigned char bit8;
typedef signed char bit8s;
typedef unsigned short bit16;
typedef signed short bit16s;
typedef unsigned __int32 bit32;
typedef signed __int32 bit32s;
typedef unsigned __int64 bit64;
typedef signed __int64 bit64s;

#if defined(_USE_ICLK64)
	typedef unsigned __int64 ICLK;
	typedef signed __int64 ICLKS;
#else
	typedef unsigned __int32 ICLK;
	typedef signed __int32 ICLKS;
#endif

# pragma pack (1)
typedef union {	
	bit32 dword;
	struct {
		bit8 ths;
		bit8 sec;
		bit8 min;
		bit8 hr;
	} byte;		
} cia_tod;

typedef union {
	
	unsigned short word;
	struct {
		unsigned char loByte;
		unsigned char hiByte;
	} byte;
		
} bit16u;

typedef unsigned __int64 bit64;

# pragma pack ()

bit16 wordswap(bit16);
bit32 dwordswap(bit32 v);
bit16 makeWordLittleEndian(bit16);
bit16 makeWordBigEndian(bit16);

