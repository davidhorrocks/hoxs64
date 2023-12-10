#pragma once

# pragma pack (1)
struct FDITrackDescription
{
	bit8 type = 0;
	bit8 size = 0;
};

struct FDIHeader
{
	char signature[27] = {};
	char creator[30] = {};
	char cr[2] = {};
	char comment[80] = {};
	bit8 eof = 0;
	bit16 version = 0;
	bit16 ltrack = 0;
	bit8 lhead = 0;
	bit8 type = 0;
	bit8 rotspeed = 0;
	bit8 flags = 0;
	bit8 tpi = 0;
	bit8 headwidth = 0;
	bit16 reserved = 0;
	struct FDITrackDescription trackDescription[176] = {};
	bit32 dataCRC = 0;
	bit32 headerCRC = 0;
};

struct FDIRawTrackHeader
{
	bit32 numPulses = 0;
	bit8 aveSize[3] = {};
	bit8 minSize[3] = {};
	bit8 maxSize[3] = {};
	bit8 idxSize[3] = {};
};

# pragma pack ()

class FDIStreamsHeader
{
public:
	FDIStreamsHeader();
	~FDIStreamsHeader();
	FDIStreamsHeader(const FDIStreamsHeader&) = delete;
	FDIStreamsHeader& operator=(const FDIStreamsHeader&) = delete;
	FDIStreamsHeader(FDIStreamsHeader&&) = delete;
	FDIStreamsHeader& operator=(FDIStreamsHeader&&) = delete;

	bit32 numPulses = 0;
	bit32 aveSize = 0;
	bit32 aveCompression = 0;
	bit32 minSize = 0;
	bit32 minCompression = 0;
	bit32 maxSize = 0;
	bit32 maxCompression = 0;
	bit32 idxSize = 0;
	bit32 idxCompression = 0;
	bit32 *aveData = nullptr;
	bit32 *minData = nullptr;
	bit32 *maxData = nullptr;
	bit32 *idxData = nullptr;
};

class FDIStream
{
public:
	FDIStream();
	~FDIStream();
	FDIStream(const FDIStream&) = delete;
	FDIStream& operator=(const FDIStream&) = delete;
	FDIStream(FDIStream&&) = delete;
	FDIStream& operator=(FDIStream&&) = delete;

	bit8 lowBitNumber = 0;
	bit8 highBitNumber = 0;
	bit8 *data = nullptr;
	bool bSignExtend = false;
	bit8 bitSize = 0;
	bool ownsTheDataAutomaticFree = true;
};

class FDIData
{
public:
	FDIData();
	~FDIData();
	FDIData(const FDIData&) = delete;
	FDIData& operator=(const FDIData&) = delete;
	FDIData(FDIData&&) = delete;
	FDIData& operator=(FDIData&&) = delete;
	bit8 *data = nullptr;
};
