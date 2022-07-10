#pragma once
#include "errormsg.h"
#include "graphics.h"
#include "viciipalette.h"
#include "vicpixelbuffer.h"

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
	VIC6569 *vic = nullptr;
	enum SpriteShiftRegister
	{
		// The sprite shift register is idle.
		srsIdle=0,

		// The sprite shift register armed waiting for an xPos match.
		srsArm=1,

		// The sprite shift register actively shifting out data.
		srsActive=3,

		// The sprite shift register became active mid column. Either 8 or less pixel could be produced 
		// for the first column.
		srsActiveXChange=4
	};

	// The sprite data shift buffer.
	// Data is shifted out at bits 23 and 22.
	bit32 dataBuffer = 0;

	bit16 bufferPos = 0;
	
	// The vic sprite x position for the sprite
	bit16 xPos = 0;
	
	// The vic cycle (1-63) that contains the sprite position. 0 means the sprite has an x position that never matches a the x comparison.
	bit8 column = 0;
	
	// The vic x position of the start of a vic cycle which may be be less than or equal to the sprite's x position.
	bit16 columnX = 0;
	
	// The index to the host emulation pixel buffer array for first pixel of the sprite.
	bit16 xPixelInit = 0;
	
	// The index to the host emulation pixel buffer array
	bit16 xPixel = 0;;

	bit16 dataLoadClock = 0;;
	bit16 dataLoadedClock = 0;;
	bit16 dataLoadIndex = 0;;
	enum SpriteShiftRegister shiftStatus = SpriteShiftRegister::srsIdle;
	signed char shiftCounter = 0;

	// The sprite X expansion flip-flop.
	bit8 ff_XP = 0;

	// The sprite multi-colour expansion flip-flop.
	bit8 ff_MC = 0;
	bit8 bleedMode = 0;
	bit8 currentPixel = 0;
	bit8 armDelay = 0;
	bit8 drawCount = 0;
	bit8 spriteNumber = 0;
	bit8 spriteBit = 0;

	void Reset(bool poweronreset);
	void SetXPos(bit16 x, bit8 currentColumn);
	int InitDraw();
	int InitDraw(bit16 xPos, bit16 columnX, bit16 xPixelInit);
	void DrawSprite(int count);

	void GetState(SsVicSprite &state);
	void SetState(const SsVicSprite &state);

	friend VIC6569;
};

class VIC6569 : public IMonitorVic, public ErrorMsg
{
public:
	VIC6569();
	~VIC6569();
	VIC6569(const VIC6569&) = delete;
	VIC6569& operator=(const VIC6569&) = delete;
	VIC6569(VIC6569&&) = delete;
	VIC6569& operator=(VIC6569&&) = delete;

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

	Graphics *pGx = nullptr;
	CAppStatus *appStatus = nullptr;
	RAM64 *ram = nullptr;
	IC6510 *cpu = nullptr;
	ICartInterface* cart = nullptr;
	bit8 cpu_next_op_code =0;

	// control reg 1
	bit8 vicECM = 0;
	bit8 vicBMM = 0;
	bit8 vicDEN = 0;
	bit8 vicRSEL = 0;
	bit32 vicYSCROLL = 0;
	bit8 vicECM_BMM_MCM = 0;
	bit8 vicECM_BMM_MCM_prev = 0;
	// control reg 2
	bit8 vicRES = 0;
	bit8 vicMCM = 0;
	bit8 vicCSEL = 0;
	bit8 vicXSCROLL = 0;
	bit8 vicXSCROLL_Cycle57 = 0;
	//Memory pointers
	bit16 vicMemptrVM = 0;
	bit16 vicMemptrCB = 0;
	//Screen colors
	bit8 vicBorderColor = 0;
	bit8 vicBackgroundColor[16] = {};
	bit8 vicSaveBackgroundColor0 = 0;
	//RASTER
	bit16 vicRASTER_compare = 0;

	bit8 vic_raster_cycle = 0; // 1-63
	bit32 vic_raster_line = 0;  // 0-311 PAL
	//INTERRUPT
	bit32 vicINTERRUPT_STATUS = 0;
	bit32 vicINTERRUPT_ENABLE = 0;
	bool vic_check_irq_in_cycle2 = false;

	//Sprites
	VICSprite vicSprite[8] = {};
	bit16 vicSpriteArmedOrActive = 0;
	bit16 vicSpritePointer[8] = {};
	bit8 ff_YP = 0;
	bit8 vicSpriteDMA = 0;
	bit8 vicSpriteDMAPrev = 0;
	bit8 vicSpriteDisplay = 0;
	bit8 vicSpriteYMatch = 0;
	bit32 vicSpriteData[8] = {};
	bit8 vicSpriteX[8] = {};
	bit8 vicSpriteY[8] = {};
	bit16 MC[8] = {};
	bit16 MCBASE[8] = {};
	bit8 vicClearingYExpandRegInClock15 = 0;
	bit8 vicSpriteMSBX = 0;
	bit8 vicSpriteEnable = 0;
	bit8 vicSpriteYExpand = 0;
	bit8 vicSpriteXExpand = 0;
	bit8 vicSpriteXExpandPrev = 0;
	bit8 vicSpriteDataPriority = 0;
	bit8 vicSpriteDataPriorityPrev = 0;
	bit8 vicSpriteMultiColor = 0;
	bit8 vicSpriteMultiColorPrev = 0;
	bit8 vicSpriteSpriteCollision = 0;
	bit8 vicSpriteDataCollision = 0;
	bit8 vicNextSprite_sprite_collision = 0;
	bit8 vicNextSprite_data_collision = 0;
	bit8 vicCurrSprite_sprite_collision = 0;
	bit8 vicCurrSprite_data_collision = 0;
	bit8 vicSpriteSpriteInt = 0;
	bit8 vicSpriteDataInt = 0;
	ICLK clockSpriteMultiColorChange = 0;
	ICLK clockSpriteDataPriorityChange = 0;
	ICLK clockSpriteXExpandChange = 0;
	ICLK clockReadSpriteDataCollision = 0;
	ICLK clockReadSpriteSpriteCollision = 0;
	ICLK clockFirstForcedBadlineCData = 0;
	ICLK clockBALow = 0;
	ICLK clockBAHigh = 0;
	bit16 vicVC = 0;
	bit16 vicRC = 0;
	bit16 vicVCBASE = 0;
	bit8 vicVMLI = 0;
	bit8s vicAEC = 0;
	bit8 vicBA = 0;
	bit8 vicIDLE = 0;
	bit8 vicIDLE_DELAY = 0;
	bit32 vicLastCData = 0;
	bit32 vicLastCDataPrev = 0;
	bit32 vicLastCDataPrev2 = 0;
	bit32 vicCDataCarry = 0;
	bit8 vicLastGData = 0;
	bit8 vicLastGDataPrev = 0;
	bit8 vicLastGDataPrev2 = 0;
	bit32 vic_latchDEN = 0;
	bit8 vic_lpx = 0;
	bit8 vic_lpy = 0;
	bit8 vicDRAMRefresh = 0;
	bit8 LP_TRIGGER = 0;
	bit8 vicLightPen = 0;
	bool m_bVicBankChanging = false;
	bit8 vicBankChangeByte = false;
	bit8 de00_byte = 0;

	//IResgister
	void Reset(ICLK sysclock, bool poweronreset) override;
	void ExecuteCycle(ICLK sysclock) override;
	bit8 ReadRegister(bit16 address, ICLK sysclock) override;
	void WriteRegister(bit16 address, ICLK sysclock, bit8 data) override;
	bit8 GetDExxByte(ICLK sysclock) override;
	bit8 SpriteDMATurningOn() override;
	bit8 ReadRegister_no_affect(bit16 address, ICLK sysclock) override;
	ICLK GetCurrentClock() override;
	void SetCurrentClock(ICLK sysclock) override;

	//IMonitorVic
	unsigned int GetFrameCounter()  override;
	bit16 GetCurrentRasterLine() override;
	bit8 GetCurrentRasterCycle() override;
	bit16 GetNextRasterLine() override;
	bit8 GetNextRasterCycle() override;
	bool GetBreakpointRasterCompare(int line, int cycle, BreakpointItem& breakpoint) override;
	bool SetBreakpointRasterCompare(int line, int cycle, bool enabled, int initialSkipOnHitCount, int currentSkipOnHitCount) override;
	int CheckBreakpointRasterCompare(int line, int cycle, bool bHitIt) override;
	void SetLPLine(bit8 lineState) override;
	void SetLPLineClk(ICLK sysclock, bit8 lineState) override;
	bool Get_BA() override;
	ICLK Get_ClockBALow() override;
	ICLK Get_CountBALow() override;
	ICLK Get_ClockBAHigh() override;
	ICLK Get_CountBAHigh() override;
	

	HRESULT Init(CAppStatus *, Graphics* pGx, RAM64 *ram, IC6510 *cpu, ICartInterface *cart, IBreakpointManager *pIBreakpointManager);
	void Cleanup();
	void setup_color_tables();
	void SetMMU(bit8 index);
	void InitReset(ICLK sysclock, bool poweronreset);
	void WriteAccessToDebugger(bit16 line, bit8 cycle, bit16 pc, bit8 dataByte);
	void PreventClockOverflow();
	HRESULT UpdateBackBuffer();
	HRESULT UpdateBackBufferLine(bit8 *pDestSurfLine, int videoPitch, bit16 line, bit8 cycle);

	bit8 ScreenPixelBuffer[PIXELBUFFER_COUNT][PAL_MAX_LINE + 1][PIXELBUFFER_SIZE + 1] = {};
	bit8 LinePixelBuffer[PIXELBUFFER_COUNT][PIXELBUFFER_SIZE + 1] = {};
	int currentPixelBufferNumber = 0;
	unsigned int frameNumber = 0;

	void BackupMainPixelBuffers();
	void GetState(SsVic6569V1 &state);
	void SetState(const SsVic6569V1 &state);
	static void UpgradeStateV0ToV1(const SsVic6569V0& in, SsVic6569V1& out);

	static bit32 vic_color_array[256];
	static bit32 vic_color_array32[256];
	static bit32 vic_color_array24[256];
	static bit16 vic_color_array16[256];
private:
	//Used for edge triggered raster IRQ noticed in Octopus In Red Wine demo.
	bool bVicRasterMatch = false;
	bit8 vic_in_display_y = 0;
	bit8 vic_badline = 0;
	bit8 vicBA_new = 0;
	bit32 vic_top_compare = 0;
	bit32 vic_bottom_compare = 0;
	bit32 vic_left_compare = 0;
	bit32 vic_right_compare = 0;
	bit8 vicMainBorder = 0;
	bit8 vicVerticalBorder = 0;
	bit8 vicCharDataOutputDisabled = 0;
	bit8 vicMainBorder_old = 0;

	bit8 vic_border_part_38 = 0;
	bit8 vic_border_part_40 = 0;

	bit8 *vic_pixelbuffer = nullptr;
	bit8 pixelMaskBuffer[(PIXELBUFFER_SIZE + 1) / 8] = {};
	bit8 vic_sprite_collision_line[PIXELBUFFER_SIZE + 1] = {};
	bit8(*vic_address_line_info)[64] = {};
	bit8 foregroundMask_mcm[256] = {};
	bool vic_allow_c_access = 0;
	bool m_bVicModeChanging = 0;
	bit32 VideoMatrix[NUM_SCREEN_COLUMNS] = {};
	static bit8 BA_line_info[256][2][64];
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

	void render(unsigned char *pRow, int xpos, int ypos, int width, int height, bit8 pPixelBufferCurrentFrame[], bit8 pPixelBufferPreviousFrame[], int startx, int videoPitch, int bufferPitch, bit32 firstVicRasterLine);
	void render_32bit(unsigned char *pRow, int xpos, int ypos, int width, int height, bit8 pPixelBufferCurrentFrame[], bit8 pPixelBufferPreviousFrame[], int startx, int videoPitch, int bufferPitch, bit32 firstVicRasterLine);

public:
	bit8 vicMemoryBankIndex = 0;
private:
	bit8 **vic_memory_map_read = nullptr;
	bit8 *vic_3fff_ptr = nullptr;
	bit8 DF_PixelsToSkip = 0;
	unsigned int lastBackedUpFrameNumber = 0;
	//BpMap m_MapBpVic;
	IBreakpointManager *m_pIBreakpointManager = nullptr;
public:

	friend VICSprite;
};
