#ifndef __BITS_H__
#define __BITS_H__

typedef unsigned char bit8;
typedef signed char bit8s;
typedef unsigned short bit16;
typedef signed short bit16s;
typedef unsigned long bit32;
typedef signed long bit32s;

#if defined(_WIN64)
	typedef unsigned __int64 ICLK;
	typedef signed __int64 ICLKS;
#else
	typedef unsigned __int32 ICLK;
	typedef signed __int32 ICLKS;
#endif

# pragma pack (1)
typedef union {	
	volatile bit32 dword;
	struct {
		volatile bit8 ths;
		volatile bit8 sec;
		volatile bit8 min;
		volatile bit8 hr;
	} byte;		
} cia_tod;

typedef union {
	
	unsigned short word;
	struct {
		unsigned char loByte;
		unsigned char hiByte;
	} byte;
		
} bit16u;

//typedef struct tagbit64
//{	
//	bit32 lo_bit32;
//	bit32 hi_bit32;	
//} bit64;

typedef unsigned __int64 bit64;

# pragma pack ()

#endif