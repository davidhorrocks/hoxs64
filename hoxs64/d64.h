#ifndef __D64_H__ 
#define __D64_H__ 

#define D64_TRACK_SIZE_1_17 (7693L)
#define D64_TRACK_SIZE_18_24 (7143L)
#define D64_TRACK_SIZE_24_30 (6667L)
#define D64_TRACK_SIZE_31_35 (6250L)

#define D64_MAX_SECTORS 21
#define D64_MAX_TRACKS 40

#define G64_MAX_TRACKS 84
#define G64_MAX_BYTES_PER_TRACK 7928

//Correct for 300rpm at 16Mhz disk internal clock
//define DISK_RAW_TRACK_SIZE 200000L
//define DISK_RAW_TRACK_SIZE 208000L
#define DISK_RAW_TRACK_SIZE 200000L


#define APPERR_BAD_CRC MAKE_HRESULT(0, 0xa00, 1)

# pragma pack (1)
struct G64Header
{
	char signature[8];
	unsigned char version;
	unsigned char trackCount;
	unsigned short trackSize;

};

struct FDITrackDescription
{
	bit8 type;
	bit8 size;
};

struct FDIHeader
{
	char signature[27];
	char creator[30];
	char cr[2];
	char comment[80];
	bit8 eof;
	bit16 version;
	bit16 ltrack;
	bit8 lhead;
	bit8 type;
	bit8 rotspeed;
	bit8 flags;
	bit8 tpi;
	bit8 headwidth;
	bit16 reserved;
	struct FDITrackDescription trackDescription[176];
	bit32 dataCRC;
	bit32 headerCRC;
};

struct FDIRawTrackHeader
{
	bit32 numPulses;
	bit8 aveSize[3];
	bit8 minSize[3];
	bit8 maxSize[3];
	bit8 idxSize[3];
};

# pragma pack ()

class FDIStreamsHeader
{
public:
	FDIStreamsHeader();
	~FDIStreamsHeader();
	bit32 numPulses;
	bit32 aveSize;
	bit32 aveCompression;
	bit32 minSize;
	bit32 minCompression;
	bit32 maxSize;
	bit32 maxCompression;
	bit32 idxSize;
	bit32 idxCompression;
	bit32 *aveData;
	bit32 *minData;
	bit32 *maxData;
	bit32 *idxData;
};

class FDIStream
{
public:
	FDIStream();
	~FDIStream();
	bit8 lowBitNumber;
	bit8 highBitNumber;
	bit8 *data;
	bool bSignExtend;
	bit8 bitSize;
};

class FDIData
{
public:
	FDIData();
	~FDIData();
	bit8 *data;
};

#define MAX_D64_SIZE (0x32200)

struct D64_Track_Info
{
	bit32 sector_count;
	bit32 file_offset;
	bit32 gcr_byte_count;
	bit8 defaultSpeed;
	bit32 sector_total;
	double track_stagger;
};


class GCRDISK : public ErrorMsg
{
public:

	GCRDISK();
	~GCRDISK();
	HRESULT Init();
	void Clean();
	HRESULT LoadD64FromFile(TCHAR *filename, bool bConvertToRAW, bool bAlignD64Tracks);
	HRESULT LoadG64FromFile(TCHAR *filename, bool bConvertToRAW);
	HRESULT LoadG64FromFileHandle(HANDLE hfile, TCHAR *filename, bool bConvertToRAW);
	HRESULT LoadFDIFromFile(TCHAR *filename);
	HRESULT LoadFDIFromFileHandle(HANDLE hfile, TCHAR *filename);
	HRESULT SaveD64ToFile(TCHAR *filename, int numberOfTracks);
	HRESULT SaveG64ToFile(TCHAR *filename);
	HRESULT SaveFDIToFile(TCHAR *filename);

	HRESULT ReadFromFile(HANDLE hfile, TCHAR *filename, char *buffer, DWORD byteCount, DWORD *bytesRead);
	HRESULT ReadFromFileQ(HANDLE hfile, char *buffer, DWORD byteCount, DWORD *bytesRead);

	void G64SetTrackSpeedZone(bit8 trackNumber, bit8 speed);
	bit8 FDIDecodeBitRate(bit8 fdiTrackType);
	HRESULT FDIReadTrackStream(HANDLE hfile, DWORD filePointer, bit8 trackNumber);
	HRESULT FDIReadRawGCR(HANDLE hfile, DWORD filePointer, bit8 trackNumber, bit8 fdiTrackType);
	HRESULT FDIReadDecodedGCR(HANDLE hfile, DWORD filePointer, bit8 trackNumber, bit8 fdiTrackType);
	HRESULT FDIDecompress(HANDLE hfile, DWORD pulseCount, DWORD **data);
	HRESULT FDICopyTrackStream(HANDLE hfile, DWORD pulseCount, DWORD **data);
	HRESULT FDICheckCRC(HANDLE hfile, TCHAR *filename, DWORD file_size);

	void WriteGCRBit(bit8 trackNumber, bit32 index, bit8 v);
	void WriteGCRByte(bit8 trackNumber, bit32 index, bit8 v);


	HRESULT ConvertGCRToD64(bit32 tracks);
	//HRESULT LoadSector(bit8 track_num, bit8 sector_num, struct sector_header *sec_header, struct sector_data *sec_data);

	void InsertNewDiskImage(TCHAR *diskname, bit8 id1, bit8 id2, bool bAlignD64Tracks, int numberOfTracks);
	void MakeNewD64Image(TCHAR *, bit8, bit8);
	void MakeGCRImage(bit8 *d64Binary, bit32 tracks, bit16 errorBytes, bool bAlignD64Tracks);
	bit8 GetSectorErrorCode(bit8 *d64Binary, bit16 errorBytes, bit8 trackNumber, bit8 sectorNumber);
	bit16 GetD64TrackSize(bit8 track);
	bit8 GetD64SpeedZone(bit8 track);
	void CopyRawData(bit8 *buffer, bit8 trackNumber,bit16 byteIndex,bit8 bitIndex, bit16 count);
	long SeekSync(bit8 trackNumber, bit16 byteIndex, bit8 bitIndex, bit32 bitScanLimit, bit8 *headerByte, bit16 *newByteIndex, bit8 *newBitIndex, bit32 *bitCounter);

	bit8 GetByte(bit8 trackNumber,bit16 byteIndex,bit8 bitIndex);
	bit8 GetSpeedZone(bit8 trackNumber,bit16 byteIndex);
	void SetSpeedZone(bit8 trackNumber, bit16 byteIndex, bit8 speed);
	void JumpBits(bit8 trackNumber,bit16 &byteIndex,bit8 &bitIndex, bit32 bitCount);


	void ConvertGCRtoRAW(bit8 trackNumber);
	void ConvertRAWtoGCR(bit8 trackNumber);
	void ConvertRAWtoGCR();

	bit8 GetDisk16(bit8 m_currentTrackNumber, bit32 headIndex);
	void PutDisk16(bit8 trackNumber, bit32 headIndex, bit8 data);

	static bit8 D64_extract_binary_nibble (bit8 *src,unsigned long i);
	static long D64_GCR_to_Binary(bit8 *src, bit8 *dest, unsigned long length);
	static void D64_Binary_to_GCR(bit8 *src, bit8 *dest, long length);

	bit16 d64Errors;
	bit32 trackSize[G64_MAX_TRACKS];//Track size in bits.
	bit8 *trackData[G64_MAX_TRACKS];
	bit8 *speedZone[G64_MAX_TRACKS];
	bit8 *m_pD64Binary;
	bit8 m_d64TrackCount;
	bit8 m_d64_protectOff;

	bit8 *m_rawTrackData[G64_MAX_TRACKS];

	static const bit8 gcr_table[16];
	static const bit8 gcr_reverse_table[32];
	static const bit8 D64_BAM[256];
	static const struct D64_Track_Info D64_info[D64_MAX_TRACKS+2];
	bool IsEventQuitSignalled();
	HANDLE mhevtQuit;

private:
	enum LoadState
	{
		Missing = 0,
		Bad = 1,
		OK = 2
	};
	LoadState d64SectorLoadStatus[D64_MAX_SECTORS];

	void ClearD64LoadStatus();
	LoadState get_D64LoadStatus(bit8 sector);
	void set_D64LoadStatus(bit8 sector, LoadState state);
	bool IsD64LoadStatusOKForD64Track(bit8 track);
	bit8 CountOfLoadStatus(LoadState state);
};


#endif