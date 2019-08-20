#ifndef __DEFINES_H__
#define __DEFINES_H__

#define APPNAME TEXT("Hoxs64")
#define APPMENUNAME TEXT("Hoxs64")
#define HOXS_MAIN_WND_CLASS TEXT("Hoxs64")
#define HOXS_EMULATION_WND_CLASS TEXT("Hoxs64Emu")

#define HOXS_MONITOR_WND_CLASS TEXT("Hoxs64Monitor")
#define HOXS_MONITOR_DISS_WND_CLASS TEXT("Hoxs64DisassembleMonitor")

#define DEBUG_AUDIO_CLOCK_SYNC 0

#define FRAMEUPDATEFREQ 64

//define CLOCK_SAMPLES_PER_SEC (982800)

#define MAX_STR_LEN 100
#define KEY_BUFFER_SIZE 16
#define NTSCCLOCKPERSECOND (1022727)
#define PALCLOCKSPERSECOND (985248)
#define PAL50CLOCKSPERSECOND (982800)
#define PAL50FRAMESPERSECOND (50)
#define PAL_MAX_LINE (311)
#define PAL_LINES_PER_FRAME (312)
#define PAL_CLOCKS_PER_LINE (63)
#define PAL_CLOCKS_PER_FRAME (19656)
#define PAL_VIDEO_WIDTH (63*8)

#define SID_BUFFER_SEGMENTS 6
#define SID_SOUND_BYTES_PER_SAMPLE 2L
#define SID_SOUND_NUMBER_OF_CHANNELS (2L)
#define SID_SOUND_SAMPLES_PER_SEC 44100L 

#define TODRELOAD50 (985248*5)
#define TODRELOAD60 (985248*6)

#define TODDIVIDER50 (50)
#define TODDIVIDER60 (50)

#define C64DISKFILENAMELENGTH 16

const int WIDTH_64 = 406;
const int HEIGHT_64 = 284;

#define CPUID_MAIN 0
#define CPUID_DISK 1

#define PAL_5_MINUTES (0x119E1B80)
#if defined(_USE_ICLK64)
#define OVERFLOWSAFTYTHRESHOLD (0x4000000000000000LL)
#else
#define OVERFLOWSAFTYTHRESHOLD (0x40000000L)
#endif

#endif