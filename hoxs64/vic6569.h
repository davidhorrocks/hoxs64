#ifndef __VIC6569_H__
#define __VIC6569_H__

#define PAL_MAX_LINE (311)
#define NUM_SCREEN_COLUMNS (40)
#define PAL_CLOCKS_PER_LINE (63)
#define GACCESS_COL1_CYCLE (16)
#define SPRITE_DISPLAY_CHECK_XPOS (0x164)

#define PIXELBUFFER_SIZE (65 *8 + 48)

#define PIXELBUFFER_MAIN_INDEX 0
#define PIXELBUFFER_BACKUP_INDEX 1
#define PIXELBUFFER_COUNT 2

class VIC6569;

struct rgb24
{
	bit8 blue;
	bit8 green;
	bit8 red;
};

struct VicColumnInfo
{
	bit32 BA;
	bit32 ExtraFetch;

};

struct VICSprite
{
	VIC6569 *vic;
	enum SpriteShiftRegister
	{
		srsIdle=0,
		srsArm=1,
		srsActive=3,
		srsActiveXChange=4
	};
	bit32 dataBuffer;
	bit16 bufferPos;
	//The vic sprite x position for the sprite
	bit16 xPos;
	//The vic cycle (1-63) that contains the sprite position. 0 means the sprite has an x position that never matches a the x comparison.
	bit8 column;
	//The vic x position of the start of a vic cycle which may be be less than or equal to the sprite's x position.
	bit16 columnX;
	//The index to the host emulation pixel buffer array for first pixel of the sprite.
	bit16 xPixelInit;
	//The index to the host emulation pixel buffer array
	bit16 xPixel;
	bit16 dataLoadClock;
	bit16 dataLoadedClock;
	bit16 dataLoadIndex;
	enum SpriteShiftRegister shiftStatus;
	signed char shiftCounter;
	bit8 ff_XP;
	bit8 ff_MC;
	bit8 bleedMode;
	bit8 currentPixel;
	bit8 armDelay;
	bit8 drawCount;
	bit8 spriteNumber;
	bit8 spriteBit;


	void Reset();
	void SetXPos(bit16 x, bit8 currentColumn);
	int InitDraw();
	int InitDraw(bit16 xPos, bit16 columnX, bit16 xPixelInit);
	void DrawSprite(int count);
	friend VIC6569;
};


class VIC6569 : public IRegister, public ILightPen, public IMonitorVic, public ErrorMsg
{
public:
	VIC6569();
	~VIC6569();

	enum ColorValue
	{
		vicBLACK = 0,
		vicWHITE = 1,
		vicRED = 2,
		vicCYAN = 3,
		vicPINK = 4,
		vicGREEN = 5,
		vicBLUE = 6,
		vicYELLOW = 7,
		vicORANGE = 8,
		vicBROWN = 9,
		vicLIGHT_RED = 10,
		vicDARK_GRAY = 11,
		vicMEDIUM_GRAY = 12,
		vicLIGHT_GREEN = 13,
		vicLIGHT_BLUE = 14,
		vicLIGHT_GRAY = 15
	};

	CDX9 *dx;
	CAppStatus *appStatus;
	CConfig *cfg;

	RAM64 *ram;
	CPU6510 *cpu;

	LPDIRECT3DSURFACE9 m_pBackBuffer;
	D3DLOCKED_RECT m_LockRect;

	ICLK ClockNextWakeUpClock;

	bit8 cpu_next_op_code;

	// control reg 1
	bit8 vicECM;
	bit8 vicBMM;
	bit8 vicDEN;
	bit8 vicRSEL;
	bit32 vicYSCROLL;
	bit8 vicECM_BMM_MCM;
	bit8 vicECM_BMM_MCM_prev;
	// control reg 2
	bit8 vicRES;
	bit8 vicMCM;
	bit8 vicCSEL;
	bit8 vicXSCROLL;
	bit8 vicXSCROLL_Cycle57;
	//Memory pointers
	bit16 vicMemptrVM;
	bit16 vicMemptrCB;
	//Screen colors
	bit8 vicBorderColor;
	bit8 vicBackgroundColor[16];
	bit8 vicSaveBackgroundColor0;
	//RASTER
	bit16 vicRASTER_compare;

	bit8 vic_raster_cycle; // 1-63
	bit32 vic_raster_line;  // 0-311 PAL
	//INTERRUPT
	bit32 vicINTERRUPT_STATUS;
	bit32 vicINTERRUPT_ENABLE;
	bit8 vic_check_irq_in_cycle2;

	//Sprites
	VICSprite vicSprite[8];
	bit16 vicSpriteArmedOrActive;
	bit16 vicSpritePointer[8];
	bit8 ff_YP;
	bit8 vicSpriteDMA;
	bit8 vicSpriteDMAPrev;
	bit8 vicSpriteDisplay;
	bit8 vicSpriteYMatch;
	bit32 vicSpriteData[8];
	bit8 vicSpriteX[8];
	bit8 vicSpriteY[8];
	bit16 MC[8],MCBASE[8];
	bit16 MC_INCR[8];
	bit8 vicSpriteMSBX;
	bit8 vicSpriteEnable;
	bit8 vicSpriteYExpand;
	bit8 vicSpriteXExpand;
	bit8 vicSpriteXExpandPrev;
	bit8 vicSpriteDataPriority;
	bit8 vicSpriteDataPriorityPrev;
	bit8 vicSpriteMultiColor;
	bit8 vicSpriteMultiColorPrev;
	bit8 vicSpriteSpriteCollision;
	bit8 vicSpriteDataCollision;
	bit8 vicNextSprite_sprite_collision;
	bit8 vicNextSprite_data_collision;
	bit8 vicCurrSprite_sprite_collision;
	bit8 vicCurrSprite_data_collision;
	bit8 vicSpriteSpriteInt;
	bit8 vicSpriteDataInt;
	ICLK clockSpriteMultiColorChange;
	ICLK clockSpriteDataPriorityChange;
	ICLK clockSpriteXExpandChange;
	ICLK clockReadSpriteDataCollision;
	ICLK clockReadSpriteSpriteCollision;

	ICLK clockFirstForcedBadlineCData;

	bit16 vicVC;
	bit16 vicRC;
	bit16 vicVCBASE;
	bit8 vicVMLI;
	long vicAEC;
	bit8 vicIDLE;
	bit8 vicIDLE_DELAY;
	bit32 vicLastCData;
	bit32 vicLastCDataPrev;
	bit32 vicLastCDataPrev2;
	bit32 vicCDataCarry;
	bit8 vicLastGData;
	bit8 vicLastGDataPrev;
	bit8 vicLastGDataPrev2;
	bit32 vic_latchDEN;
	bit8 vic_lpx,vic_lpy;
	bit8 vicDRAMRefresh;
	bit8 LP_TRIGGER;
	bit8 vicLightPen;

	bool m_bVicBankChanging;
	bit8 vicBankChangeByte;

	//IMonitorVic
	virtual bit16 GetRasterLine();
	virtual bit8 GetRasterCycle();
	virtual IEnumBreakpointItem *CreateEnumBreakpointExecute();
	virtual bool SetBreakpointRasterCompare(int line, int cycle, bool enabled, int initialSkipOnHitCount, int currentSkipOnHitCount);
	virtual bool GetBreakpointRasterCompare(int line, int cycle, Sp_BreakpointItem& breakpoint);
	virtual void ClearAllBreakpoints();
	virtual int CheckBreakpointRasterCompare(int line, int cycle, bool bHitIt);

	HRESULT Init(CConfig *, CAppStatus *, CDX9 *dx, RAM64 *ram, CPU6510 *cpu);
	void Cleanup();
	void setup_color_tables(D3DFORMAT format);
	void SetMMU(bit8 index);
	bit8 de00_byte;

	//IResgister
	virtual void Reset(ICLK sysclock);
	virtual void ExecuteCycle(ICLK sysclock);
	virtual bit8 ReadRegister(bit16 address, ICLK sysclock);
	virtual void WriteRegister(bit16 address, ICLK sysclock, bit8 data);
	virtual bit8 ReadRegister_no_affect(bit16 address, ICLK sysclock);

	//ILightPen
	virtual void SetLPLine(bit8 lineState);
	virtual void SetLPLineClk(ICLK sysclock, bit8 lineState);

	void PreventClockOverflow();
	HRESULT UpdateBackBuffer();
	//HRESULT UpdateBackBufferLine(bit16 line, bit8 cycle);
	HRESULT UpdateBackBufferLine(bit8 *pDestSurfLine, int videoPitch, bit16 line, bit8 cycle);

	bit8 ScreenPixelBuffer[PIXELBUFFER_COUNT][PAL_MAX_LINE+1][PIXELBUFFER_SIZE+1];
	bit8 LinePixelBuffer[2][PIXELBUFFER_SIZE+1];
	//bit8 ScreenBorderBuffer[PIXELBUFFER_COUNT][PAL_MAX_LINE+1][PIXELBUFFER_SIZE+1];
	int FrameNumber;

	void BackupMainPixelBuffers();

	static bit32 vic_color_array[256];
	static bit32 vic_color_array32[256];
	static bit32 vic_color_array24[256];
	static bit16 vic_color_array16[256];
	static bit8 vic_color_array8[256];
private:
	//Used for edge triggered raster IRQ noticed in Octopus In Red Wine demo.
	bool bVicRasterMatch;
	bit8 vic_in_display_y;
	bit8 vic_badline, vicBA_new;
	bit32 vic_top_compare;
	bit32 vic_bottom_compare;
	bit32 vic_left_compare;
	bit32 vic_right_compare;
	bit8 vicMainBorder;
	bit8 vicVerticalBorder; 
	bit8 vicCharDataOutputDisabled; 
	bit8 vicMainBorder_old;

	bit8 vic_border_part_38;
	bit8 vic_border_part_40;

	bit8 *vic_pixelbuffer;
	//bit8 *vic_borderbuffer;
	bit8 pixelMaskBuffer[(PIXELBUFFER_SIZE + 1) / 8];
	bit8 vic_sprite_collision_line[PIXELBUFFER_SIZE];
	bit8 (*line_info)[2][64];
	bit8 (*vic_address_line_info)[64];
	bit8 foregroundMask_mcm[256];
	bit8 vic_allow_c_access;
	bool m_bVicModeChanging;
	bit32 VideoMatrix[100];//NUM_SCREEN_COLUMNS
	static bit8 BA_line_info[256][2][64];
	static const int vic_ii_sprites_crunch_table[64];
	static bit8 vic_spr_load_x[8];
	static bit16 vic_spr_data_load_x[8];


	void SetSystemInterrupt();
	void ClearSystemInterrupt();
	void DmaFixUp(bit8 cycle, bit8 data);
	void SpriteCollisionUpdateArmedOrActive();
	void CheckRasterCompare(bit8 cycle);
	void DrawSprites(bit8 column);
	bit16 SpriteIndexFromClock(bit16 clock);
	void SpriteXChange(bit8 spriteNo, bit16 x_new, bit8 cycle);
	static bit16 GetVicXPosFromCycle(bit8 cycle, signed char offset);
	void WRITE_FORE_MASK_STD(bit8 gData, signed char xscroll, bit8 cycle);
	void WRITE_FORE_MASK_STD_EX(bit8 gData, signed char xscroll, bit8 xstart, bit8 count, const bit8 cycle);
	void WRITE_FORE_MASK_MCM(bit8 gData, signed char xscroll, bit8 cycle);
	void WRITE_FORE_MASK_MCM_EX(bit8 gData, signed char xscroll, bit8 xstart, bit8 count, const bit8 cycle);
	void WRITE_COLOR_BYTE8(bit8 color, const signed char xscroll, const bit8 cycle);
	void COLOR_FOREGROUND(bit8 backgroundColor[], const bit8 cycle);
	void WRITE_STD2_BYTE(bit8 gData, signed char xscroll,const bit8 color2[], const bit8 cycle);
	void WRITE_STD2_BYTE_EX(bit8 gData, signed char xscroll,const bit8 color2[], const bit8 cycle, bit8 xstart, bit8 count);
	void WRITE_MCM4_BYTE (bit8 gData, signed char xscroll,const bit8 color4[], const bit8 cycle);
	void WRITE_MCM4_BYTE_EX (bit8 gData, signed char xscroll,const bit8 color4[], const bit8 cycle, bit8 xstart, bit8 count);

	void C_ACCESS();
	bit8 G_ACCESS(const bit8 ecm_bmm_mcm, bit32& lastCData);
	//1st VIC phase
	bit8 vic_ph1_read_byte(bit16 address);
	//1st VIC phase fixed to 3fff
	bit8 vic_ph1_read_3fff_byte();
	//2nd VIC phase DMA is fully on
	bit8 vic_ph2_read_byte(bit16 address);
	//2nd VIC phase DMA controled by AEC
	bit8 vic_ph2_read_aec_byte(bit16 address);
	//2nd VIC phase DMA controled by AEC fixed to 3fff
	bit8 vic_ph2_read_3fff_aec_byte();
	bit8 vic_read_color_byte(bit16 address);
	void DrawForeground0(bit8 xscroll, bit32 cData, bit8 ecm_bmm_mcm, bit8 cycle);
	void DrawForeground(bit8 gData, signed char xscroll, bit32 cData, bit8 ecm_bmm_mcm, bit8 pixelshiftmode, bit8 verticalBorder, bit8 cycle);
	void DrawForegroundEx(bit8 gData, signed char xscroll, bit32 cData, bit8 ecm_bmm_mcm, bit8 pixelshiftmode, bit8 verticalBorder, bit8 xstart, bit8 count, bit8 cycle);
	bit8 DF_PixelsToSkip;

	void DrawBorder(bit8 cycle);
	void DrawBorder4(bit8 cycle);
	void DrawBorder7(bit8 cycle);
	void check_38_col_right_border();
	void draw_38_col_right_border1(bit8 cycle);
	void check_40_col_right_border();
	void draw_38_col_left_border(bit8 cycle);
	void check_38_col_left_border();
	void draw_40_col_left_border1(bit8 cycle);
	void draw_40_col_left_border2(bit8 cycle);
	void check_40_col_left_border();

	void setup_multicolor_mask_table();
	void setup_vic_ba();
	void SpriteArm(int spriteNo);
	void SpriteIdle(int spriteNo);
	__forceinline void SetBA(ICLK &cycles, bit8 cycle);
	__forceinline void init_line_start();

	void render(long depth, bool bPixelDoubler, unsigned char *pRow, int xpos, int ypos, int width, int height, bit8 pPixelBuffer[], int startx, int videoPitch, int bufferPitch);
	void render_8bit_2x(unsigned char *pRow, int xpos, int ypos, int width, int height, bit8 pPixelBuffer[], int startx, int videoPitch, int bufferPitch);
	void render_8bit(unsigned char *pRow, int xpos, int ypos, int width, int height, bit8 pPixelBuffer[], int startx, int videoPitch, int bufferPitch);
	void render_16bit(unsigned char *pRow, int xpos, int ypos, int width, int height, bit8 pPixelBuffer[], int startx, int videoPitch, int bufferPitch);
	void render_16bit_2x(unsigned char *pRow, int xpos, int ypos, int width, int height, bit8 pPixelBuffer[], int startx, int videoPitch, int bufferPitch);
	void render_24bit(unsigned char *pRow, int xpos, int ypos, int width, int height, bit8 pPixelBuffer[], int startx, int videoPitch, int bufferPitch);
	void render_24bit_2x(unsigned char *pRow, int xpos, int ypos, int width, int height, bit8 pPixelBuffer[], int startx, int videoPitch, int bufferPitch);
	void render_32bit(unsigned char *pRow, int xpos, int ypos, int width, int height, bit8 pPixelBuffer[], int startx, int videoPitch, int bufferPitch);
	void render_32bit_2x(unsigned char *pRow, int xpos, int ypos, int width, int height, bit8 pPixelBuffer[], int startx, int videoPitch, int bufferPitch);

	bit8 vicMemoryBankIndex;
	bit8 **vic_memory_map_read;
	bit8 *vic_3fff_ptr;

	int m_iLastBackedUpFrameNumber;
	BpMap m_MapBpVic;
public:

	friend VICSprite;
};
#endif