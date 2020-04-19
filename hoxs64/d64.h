#pragma once
#include "iquit.h"
#include "errormsg.h"

#define D64_TRACK_SIZE_1_17 (7693L)
#define D64_TRACK_SIZE_18_24 (7143L)
#define D64_TRACK_SIZE_24_30 (6667L)
#define D64_TRACK_SIZE_31_35 (6250L)

#define D64_MAX_SECTORS 21
#define D64_MAX_TRACKS 40

#define G64_MAX_TRACKS 84
#define HOST_MAX_TRACKS 84
#define G64_MAX_BYTES_PER_TRACK 7928
#define G64_MAX_ALLOCATED_BYTES_PER_TRACK (G64_MAX_BYTES_PER_TRACK * 2)

//Correct for 300rpm at 16Mhz disk internal clock
#define DISK_RAW_TRACK_SIZE 200000L


#define APPERR_BAD_CRC MAKE_HRESULT(0, 0xa00, 1)

# pragma pack (1)
struct G64Header
{
	char signature[8];
	unsigned char version;
	unsigned char trackCount;
	unsigned short fileAllocatedTrackSize;

};

# pragma pack ()

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


class GCRDISK : public ErrorMsg, public IQuit
{
public:

	GCRDISK();
	~GCRDISK();
	GCRDISK(const GCRDISK&) = delete;
	GCRDISK& operator=(const GCRDISK&) = delete;
	GCRDISK(GCRDISK&&) = delete;
	GCRDISK& operator=(GCRDISK&&) = delete;

	HRESULT Init();
	void Clean() noexcept;
	HRESULT LoadD64FromFile(const TCHAR *filename, bool bConvertToRAW, bool bAlignD64Tracks);
	HRESULT LoadG64FromFile(const TCHAR *filename, bool bConvertToRAW);
	HRESULT LoadG64FromFileHandle(HANDLE hfile, const TCHAR *filename, bool bConvertToRAW);
	HRESULT LoadFDIFromFile(const TCHAR *filename);
	HRESULT LoadFDIFromFileHandle(HANDLE hfile, const TCHAR *filename);
    HRESULT LoadP64FromFile(const TCHAR *filename);
    HRESULT LoadP64FromFileHandle(HANDLE hfile, const TCHAR *filename);
	HRESULT SaveD64ToFile(const TCHAR *filename, int numberOfTracks);
	HRESULT SaveG64ToFile(const TCHAR *filename);
	HRESULT SaveFDIToFile(const TCHAR *filename);
	HRESULT SaveP64ToFile(const TCHAR *filename);
	HRESULT ReadFromFile(HANDLE hfile, const TCHAR *filename, char *buffer, DWORD byteCount, DWORD *bytesRead);
	HRESULT ReadFromFileQ(HANDLE hfile, char *buffer, DWORD byteCount, DWORD *bytesRead);
	void G64SetTrackSpeedZone(bit8 trackNumber, bit8 speed);
	bit8 FDIDecodeBitRate(bit8 fdiTrackType);
	HRESULT FDIReadTrackStream(HANDLE hfile, bit64 filePointer, bit8 trackNumber);
	HRESULT FDIReadGCR(HANDLE hfile, bit64 filePointer, bit8 trackNumber, bit8 fdiTrackType);
	HRESULT FDIReadDecodedGCR(HANDLE hfile, bit64 filePointer, bit8 trackNumber, bit8 fdiTrackType);
	HRESULT FDICopyTrackStream(HANDLE hfile, bit32 pulseCount, bit32** data);
	HRESULT FDICheckCRC(HANDLE hfile, const TCHAR *filename);
	void WriteGCRBit(bit8 trackNumber, bit32 index, bit8 v);
	void WriteGCRByte(bit8 trackNumber, bit32 index, bit8 v);
	HRESULT ConvertGCRToD64(unsigned int tracks);
	void InsertNewDiskImage(const TCHAR *diskname, bit8 id1, bit8 id2, bool bAlignD64Tracks, int numberOfTracks);
	void MakeNewD64Image(const TCHAR *, bit8, bit8);
	HRESULT MakeGCRImage(bit8 *d64Binary, bit32 tracks, bit16 errorBytes, bool bAlignD64Tracks);
	bit8 GetSectorErrorCode(bit8 *d64Binary, bit16 errorBytes, bit8 trackNumber, bit8 sectorNumber);
	bit16 GetD64TrackSize(bit8 track);
	bit8 GetD64SpeedZone(bit8 track);
	void CopyRawData(bit8 *buffer, unsigned int trackNumber,unsigned int bitIndex, unsigned int count);
	HRESULT SeekSync(unsigned int trackNumber, unsigned int bitIndex, unsigned int bitScanLimit, bit8 *p_headerByte, unsigned int *p_newBitIndex, unsigned int *p_bitCounter, unsigned int *p_sync_count);
	bit8 GetByte(unsigned int trackNumber, unsigned int bitIndex);
	bit8 GetSpeedZone(bit8 trackNumber,bit16 byteIndex);
	void SetSpeedZone(bit8 trackNumber, bit16 byteIndex, bit8 speed);
	void JumpBits(unsigned int trackNumber,unsigned int &bitIndex, unsigned int bitCount);
	void ConvertGCRtoP64(bit8 trackNumber);
	HRESULT ConvertP64toGCR(bit8 trackNumber);
	HRESULT ConvertP64toGCR();
	void WriteByteGroupedToDebugger(bit8 dataByte);
	void WriteByteToDebugger(bit8 dataByte);
	void WriteLineToDebugger();
	bool IsEventQuitSignalled();
	bool IsQuit();
	void Quit();

	int linePrintCount = 0;
	bit16 d64Errors = 0;
	bit32 trackSize[HOST_MAX_TRACKS] = {};//Track size in bits.
	bit32 trackAllocatedMemorySize[HOST_MAX_TRACKS] = {};
	bit8* trackData[HOST_MAX_TRACKS] = {};
	bit8* speedZone[HOST_MAX_TRACKS] = {};
	bit8 *m_pD64Binary = nullptr;
	bit8 m_d64TrackCount = 0;
	bit8 m_d64_protectOff = 0;
	TP64Image m_P64Image = {};
	HANDLE mhevtQuit = 0;

	static p64_uint32_t CountP64TrackPulses(const TP64PulseStream& track);
	static p64_uint32_t CountP64ImageTrackPulses(const TP64Image& image, unsigned int trackNumber);
	static p64_uint32_t CountP64ImageMaxTrackPulses(const TP64Image& image);
	static bit8 D64_extract_binary_nibble(bit8* src, unsigned long i) noexcept;
	static long D64_GCR_to_Binary(bit8* src, bit8* dest, unsigned long length) noexcept;
	static void D64_Binary_to_GCR(bit8* src, bit8* dest, long length) noexcept;
	static const bit8 gcr_table[16];
	static const bit8 gcr_reverse_table[32];
	static const bit8 D64_BAM[256];
	static const struct D64_Track_Info D64_info[D64_MAX_TRACKS + 2];
private:
	enum LoadState
	{
		Missing = 0,
		Bad = 1,
		OK = 2
	};

	struct D64_Sector_Status
	{
		LoadState loaded;
		unsigned int bitIndex;
	};

	struct D64_Track_Status
	{
		unsigned int trackSize;
		D64_Sector_Status d64SectorLoadStatus[D64_MAX_SECTORS];		
	};

	D64_Track_Status d64LoadStatus[D64_MAX_TRACKS];
	void ClearD64LoadStatus();
	LoadState get_D64LoadStatus(unsigned int track, unsigned int sector);
	void set_D64LoadStatus(unsigned int track, unsigned int sector, LoadState state);
	bool IsD64LoadStatusOKForD64Track(unsigned int track);
	bit8 CountOfLoadStatus(unsigned int track, LoadState state);
	HRESULT InitTrackMemory(unsigned int trackNumber, unsigned int byteLength, bool preserveData);
};
