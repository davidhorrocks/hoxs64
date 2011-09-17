#include <windows.h>
#include "dx_version.h"
#include <d3d9.h>
#include <d3dx9core.h>
#include <dinput.h>
#include <dsound.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <intrin.h>
#include "defines.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "mlist.h"
#include "carray.h"
#include "tchar.h"
#include "register.h"
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "dxstuff9.h"
#include "c6502.h"
#include "ram64.h"
#include "cpu6510.h"
#include "ram64.h"
#include "vic6569.h"

#define vic_borderbuffer vic_pixelbuffer

#define vicRGB_BLACK 0x00000000
#define sprite0 0
#define sprite1 1
#define sprite2 2
#define sprite3 3
#define sprite4 4
#define sprite5 5
#define sprite6 6
#define sprite7 7


#define MAX_SPRITE_WIDTH (0x30) 
#define LEFT_BORDER_WIDTH (0x30) 
#define DISPLAY_START (76)
#define DISPLAY_START_A 76
#define BACKCOLORINDEX0 (0x80)
#define BACKCOLORINDEX1 (0x81)
#define BACKCOLORINDEX2 (0x82)
#define BACKCOLORINDEX3 (0x83)
#define BACKCOLORINDEX4 (0x84)
#define BORDERCOLORINDEX (0x8E)
#define VIDEOWIDTH (63 * 8)
#define LINELENGTH (63)

bit32 VIC6569::vic_color_array[256]=
{
	0x00000000,//BLACK
	0x00FFFFFF,//WHITE
	0x0068372B,//RED
	0x0070A4B2,//CYAN
	0x006F3D86,//PINK
	0x00588D43,//GREEN
	0x00352879,//BLUE
	0x00B8C76F,//YELLOW
	0x006F4F25,//ORANGE
	0x00433900,//BROWN
	0x009A6759,//LT RED
	0x00444444,//DARK GRAY
	0x006C6C6C,//MEDIUM GRAY
	0x009AD284,//LT GREEN
	0x006C5EB5,//LT BLUE
	0x00959595 //LT GRAY
};

bit8 VIC6569::vic_color_array8[256];
bit16 VIC6569::vic_color_array16[256];
bit32 VIC6569::vic_color_array24[256];
bit32 VIC6569::vic_color_array32[256];

bit8 VIC6569::BA_line_info[256][2][64]=
{
	{
		{1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   
			1, 1, 1
		},
		{1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   
			1, 0, 0, 0, 0, 0, 0, 0, 0, 0,   
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   
			0, 0, 0, 0, 1, 1, 1, 1, 1, 1,   
			1, 1, 1
		}
	}
};


const int VIC6569::vic_ii_sprites_crunch_table[64] =
{
	1,   4,   3, /* 0 */
	4,   1,   0, /* 3 */
	-1,   0,   1, /* 6 */
	4,   3,   4, /* 9 */
	1,   8,   7, /* 12 */
	8,   1,   4, /* 15 */
	3,   4,   1, /* 18 */
	0,  -1,   0, /* 21 */
	1,   4,   3, /* 24 */
	4,   1,  -8, /* 27 */
	-9,  -8,   1, /* 30 */
	4,   3,   4, /* 33 */
	1,   0,  -1, /* 36 */
	0,   1,   4, /* 39 */
	3,   4,   1, /* 42 */
	8,   7,   8, /* 45 */
	1,   4,   3, /* 48 */
	4,   1,   0, /* 51 */
	-1,   0,   1, /* 54 */
	4,   3,   4, /* 57 */
	1, -40, -41, /* 60 */
	0
};

bit8 VIC6569::vic_spr_load_x[8]=
{
	58,60,62,1,3,5,7,9
};

bit16 VIC6569::vic_spr_data_load_x[8]=
{
	0x162,0x172,0x182,0x192,0x1a2,0x1b2,0x1c2,0x1d2
};

bit16 VIC6569::SpriteIndexFromClock(bit16 clock)
{
	if (clock >= 0x194)
	{
		return  clock - 0x194;
	}
	else
	{
		return clock + (DISPLAY_START + LEFT_BORDER_WIDTH/2);
	}
}

void VICSprite::Reset()
{
	shiftStatus = srsIdle;
	armDelay = 0;
	shiftCounter = 0;
	ff_XP = 0;
	bleedMode = 0;
	xPos = 0;
	dataBuffer = 0;
	column = 0;
	drawCount = 0;
	ff_MC = 0;
}

bit16 VIC6569::GetVicXPosFromCycle(bit8 cycle, signed char offset)
{
signed short x;
	if (cycle < 14)
		x = 0x18C + ((signed short)cycle * 8);
	else
		x = ((signed short)cycle - 14) * 8 + 4;

	x += offset;
	if (x >= 0x1f8)
		x -= 0x1f8;
	else if (x < 0)
		x = 0x1f8 - x;
	return (bit16)x;
}

void VICSprite::SetXPos(bit16 x, bit8 currentColumn)
{
bit16 currentColumnXPos;
signed short xDiff;

	currentColumnXPos = VIC6569::GetVicXPosFromCycle(currentColumn, +4);
	xDiff = (signed short)(currentColumnXPos - xPos);
	if (xDiff > 0xfa)
		xDiff =  xDiff - 0x1f8;
	else if (xDiff < -0xfa)
		xDiff += 0x1f8;

	if (xPos != x)
	{
		if (shiftStatus == srsArm)
		{
			if (currentColumn == column && xDiff > 0)
			{
				/*We just moved to another X position for this column but X comparison matched the previous position*/
				InitDraw();
				shiftStatus = srsActiveXChange;
			}
		}
	}
	

	if (x >= 0x1f8)
		column = 0;
	else if (x >= 0x194)
	{
		column = (bit8)(((x - 0x18c)/8) & 0xff);
		columnX = (((bit16)column) << 3) + 0x18c;
		xPixelInit = x - 0x194;
	}
	else
	{
		column = (bit8)(((x + 4)/8 + 13) & 0xff);

		/*
		Warning: Hacky columnX can go signed negative instead of wrapping to 0x1f7. 
		This is a hack to accommodate VICSprite::InitDraw whose computation of 'drawCount' uses a simple subtraction which does not wrap around sprite position 0x1f7
		*/
		columnX = (((bit16)column - 13)<< 3) - 4;
		xPixelInit = x + (DISPLAY_START + LEFT_BORDER_WIDTH/2);
	}

	if ((column == currentColumn) && (shiftStatus == srsArm)  && (xPos != x))
	{
		xDiff = (signed short)(currentColumnXPos - x);
		if (xDiff > 0xfa)
			xDiff =  xDiff - 0x1f8;
		else if (xDiff < -0xfa)
			xDiff += 0x1f8;
		if (xDiff > 0)
		{
			/*We just set an X position for this column but missed the X comparison*/
			armDelay = 1;
		}
	}
	xPos = x;
}

int VICSprite::InitDraw()
{
	shiftStatus = srsActive;
	xPixel = xPixelInit;
	ff_XP = 0;
	currentPixel = 0xff;
	bleedMode = 0;
	ff_MC = 0;
	if (xPos > dataLoadClock && xPos < dataLoadedClock)
	{
		/* Shifter is disabled during the sprite data fetch */
		shiftCounter = 0;
		drawCount = 0;
	}
	else
	{
		shiftCounter = 24;
		drawCount = (signed short)8 - ((signed short)xPos - (signed short)columnX);
	}
	return drawCount;
}

int VICSprite::InitDraw(bit16 xPos, bit16 columnX, bit16 xPixelInit)
{
	shiftStatus = srsActive;
	xPixel = xPixelInit;
	ff_XP = 0;
	currentPixel = 0xff;
	bleedMode = 0;
	ff_MC = 0;
	if (xPos > dataLoadClock && xPos < dataLoadedClock)
	{
		/* Shifter is disabled during the sprite data fetch */
		shiftCounter = 0;
		drawCount = 0;
	}
	else
	{
		shiftCounter = 24;
		drawCount = (signed short)8 - ((signed short)xPos - (signed short)columnX);
	}
	return drawCount;
}

void VIC6569::DrawSprites(bit8 column)
{
int i;
int dc;

	//If the sprite is idle then we stil might need to process a pending collision.

	vicSpriteSpriteInt>>=1;
	vicSpriteDataInt>>=1;

	vicCurrSprite_sprite_collision = vicNextSprite_sprite_collision;
	vicCurrSprite_data_collision = vicNextSprite_data_collision;
	vicNextSprite_sprite_collision = 0;
	vicNextSprite_data_collision = 0;

	for (i=0 ; i < 8 ; i++)
	{
		VICSprite& sprite = vicSprite[i];
		switch (sprite.shiftStatus)
		{
			case VICSprite::srsIdle:
				break;
			case VICSprite::srsArm:
				if (column == sprite.column)
				{
					if (sprite.armDelay==0)
					{
						dc = sprite.InitDraw();
						sprite.DrawSprite(dc);
						break;
					}
				}
				break;
			case VICSprite::srsActiveXChange:
				sprite.DrawSprite(sprite.drawCount);
				sprite.shiftStatus = VICSprite::srsActive;
				break;
			case VICSprite::srsActive:
				sprite.DrawSprite(8);
				break;
		}
		sprite.armDelay=0;
	}

	if (vicSpriteSpriteCollision)
	{
		vicSpriteSpriteCollision|=vicCurrSprite_sprite_collision;
	}
	else
	{
		vicSpriteSpriteCollision|=vicCurrSprite_sprite_collision;
		if (vicSpriteSpriteCollision)
		{
			vicINTERRUPT_STATUS|=4;
		}
	}

	if (vicSpriteDataCollision)
	{
		vicSpriteDataCollision|=vicCurrSprite_data_collision;
	}
	else
	{
		vicSpriteDataCollision|=vicCurrSprite_data_collision;
		if (vicSpriteDataCollision)
		{
			vicINTERRUPT_STATUS|=2;
		}
	}

	if (this->CurrentClock - clockReadSpriteSpriteCollision == 0)
	{
		vicSpriteSpriteCollision = 0;
		vicNextSprite_sprite_collision = 0;
	}
	if (this->CurrentClock - clockReadSpriteDataCollision == 0)
	{
		vicSpriteDataCollision = 0;
		vicNextSprite_data_collision = 0;
	}

	SpriteCollisionUpdateArmedOrActive();

	if ((vicINTERRUPT_STATUS & vicINTERRUPT_ENABLE & 0x6)!=0)
	{
		SetSystemInterrupt();
	}
}

//Optimisation used by vic::ExcecuteCycle() to limit unecessary processing of inactive sprites.
void VIC6569::SpriteCollisionUpdateArmedOrActive()
{
	vicSpriteArmedOrActive = (vicSpriteArmedOrActive & 0xff) | ((vicSpriteDataInt | vicSpriteSpriteInt | vicNextSprite_sprite_collision | vicNextSprite_data_collision) << 8);
}

void VICSprite::DrawSprite(int c)
{
bit8 maskShift,maskByte;
bit8 foregroundmask;
bit16 mask;
bit8 color1,color2,color3;
bit8 sprite_sprite_collision;
bit8 sprite_data_collision;
bit8 *pixelbuffer;
bit8 *sprite_collision_line;
bit8 multiColourThisPixel;
bit8 multiColour;
bit8 multiColourPrev;
bit8 xExpand;
bit8 xExpandPrev;
bit8 xExpandThisPixel;
bit8 dataPriority;
bit8 dataPriorityPrev;
ICLK clockMultiColorChange;
ICLK clockSpriteXExpandChange;
ICLK clockSpriteDataPriorityChange;
ICLK vicCurrentClock;

	sprite_collision_line = vic->vic_sprite_collision_line;
	vicCurrentClock = vic->CurrentClock;
	pixelbuffer = vic->vic_pixelbuffer;
	dataPriority = vic->vicSpriteDataPriority & spriteBit;
	dataPriorityPrev = vic->vicSpriteDataPriorityPrev & spriteBit;
	multiColour = vic->vicSpriteMultiColor & spriteBit;
	multiColourPrev = vic->vicSpriteMultiColorPrev & spriteBit;
	xExpand = vic->vicSpriteXExpand & spriteBit;
	xExpandPrev = vic->vicSpriteXExpandPrev  & spriteBit;

	clockMultiColorChange = vic->clockSpriteMultiColorChange;
	clockSpriteXExpandChange = vic->clockSpriteXExpandChange;
	clockSpriteDataPriorityChange = vic->clockSpriteDataPriorityChange;
	
	color1=12 | 128;
	color2=(4 + spriteNumber) | 128;
	color3=13 | 128;

	maskShift = (xPixel + 4) & 7;
	maskByte  = (xPixel + 4) / 8;

	if (maskByte<14 || maskByte>57)
		mask = 0;
	else
	{
		mask = ((bit16)(vic->pixelMaskBuffer[maskByte])) << 8;
		mask |= (bit16)vic->pixelMaskBuffer[maskByte+1];
		mask <<= maskShift;
	}

	foregroundmask = ~(mask>>8);
	if ((c > 6) && (clockSpriteDataPriorityChange == vicCurrentClock))
	{
		if (dataPriority == 0)
			foregroundmask |= (0xff >> (c - 6));
		if (dataPriorityPrev == 0)
			foregroundmask |= (0xc0 << (8 - c));
	}
	else
	{
		if (dataPriority == 0)
			foregroundmask = -1;
	}

	//TEST sprites always on top
	//foregroundmask = -1;

	while (c>0)
	{
		if (bleedMode == 0)
		{
			if (((ff_XP) == 0))
			{
				if ((shiftCounter <= 0) && (ff_MC == 0))
				{
					if (vic->vicSpriteDisplay & spriteBit)
						shiftStatus = srsArm;
					else
					{
						shiftStatus = srsIdle;
						vic->vicSpriteArmedOrActive = vic->vicSpriteArmedOrActive & ~(bit16)spriteBit;
					}
					break;
				}

				multiColourThisPixel = multiColour;
				if ((c > 6) && (clockMultiColorChange == vicCurrentClock))
				{
					multiColourThisPixel = multiColourPrev;
				}

				if (multiColourThisPixel || ff_MC)
				{
					ff_MC ^= 1;

					if (ff_MC & 1)
					{
						
						switch ((bit8)((dataBuffer & 0xc00000) >> 22))
						{
						case 0:
							currentPixel = 0x00;
							break;
						case 1:
							currentPixel = color1;
							break;
						case 2:
							currentPixel = color2;
							break;
						case 3:
							currentPixel = color3;
							break;
						}
					}
				}
				else
				{
					if (dataBuffer & 0x800000)
						currentPixel = color2;
					else
						currentPixel = 0x00;
				}
			}

			if (c == 6 && (ff_XP == 1) && (clockMultiColorChange == vicCurrentClock))
			{
				//We are in the second expanded pixel part and the multi colour status is changing
				if (multiColourPrev == 0 && multiColour != 0)
				{
					//Multicolour is going on
					switch ((bit8)((dataBuffer & 0x800000) >> 22))
					{
					case 0:
						currentPixel = 0x00;
						break;
					case 1:
						currentPixel = color1;
						break;
					case 2:
						currentPixel = color2;
						break;
					case 3:
						currentPixel = color3;
						break;
					}
					ff_MC = 1;
				}
				else if (multiColourPrev != 0 && multiColour == 0)
				{
					//Multicolour is going off
					ff_MC = 0;
				}
			}
		}

		if (xPixel == dataLoadIndex && shiftCounter > 0)
		{
			//Same colour pixel is output 7 times
			bleedMode = 1;
			shiftCounter = 7;
			if (ff_MC != 0 && ff_XP == 0 && vic->vicSpritePointer[spriteNumber] < 0x2000)
			{
				if (((dataBuffer & 0x800000)) == 0)
					currentPixel = 0x00;
				else
					currentPixel = color2;
			}
			ff_XP = 0;
			ff_MC = 0;
		}

		//TEST sprites on//off
		//currentPixel = 0x0;			

		if (currentPixel!=0) // Is a non transparent sprite pixel
		{
			sprite_sprite_collision = sprite_data_collision = 0;
			if (sprite_collision_line[xPixel]) // Sprite to sprite collision
				sprite_sprite_collision= sprite_collision_line[xPixel] | spriteBit;
			else
				if (((signed char)foregroundmask) < 0) // Sprite data priority to foreground graphics check
					pixelbuffer[xPixel] = currentPixel;

			sprite_collision_line[xPixel]=spriteBit;
			if (((signed short)mask) < 0) // Sprite to foreground collision 
				sprite_data_collision=spriteBit;

			//TEST 4 pixel delay of sprite collision
			if (c > 4)
			{
				vic->vicCurrSprite_sprite_collision |= sprite_sprite_collision;
				vic->vicCurrSprite_data_collision |= sprite_data_collision;
			}
			else
			{
				vic->vicNextSprite_sprite_collision |= sprite_sprite_collision;
				vic->vicNextSprite_data_collision |= sprite_data_collision;
			}
			//TEST new irq behaviour
			if (sprite_sprite_collision != 0)
			{
				if (c > 4)
				{
					vic->vicSpriteSpriteInt|=1;
				}
				else
				{
					vic->vicSpriteSpriteInt|=2;
				}
			}
			if (sprite_data_collision != 0)
			{
				if (c > 4)
				{
					vic->vicSpriteDataInt|=1;
				}
				else
				{
					vic->vicSpriteDataInt|=2;
				}
			}
		}


		if (bleedMode != 0)
		{
			if (shiftCounter <= 0)
			{
				//Ensure the condition for going idle is satisfied.
				ff_XP = 0;
				ff_MC = 0;
				break;
			}
			shiftCounter--;
		}
		else
		{
			xExpandThisPixel = xExpand;
			if ((c > 6) && (clockSpriteXExpandChange == vicCurrentClock))
				xExpandThisPixel = xExpandPrev;

			if (xExpandThisPixel || ff_XP != 0)
				ff_XP ^= 1;

			if (ff_XP == 0)
			{
				dataBuffer <<= 1;
				shiftCounter--;
			}
		}

		mask <<= 1;
		foregroundmask <<= 1;
		xPixel++;
		if (xPixel >= VIDEOWIDTH)
			xPixel -= VIDEOWIDTH;

		c--;
	}

	if ((shiftCounter <= 0 && ff_XP == 0 && ff_MC == 0))
	{
		if (vic->vicSpriteDisplay & spriteBit)
			shiftStatus = srsArm;
		else
		{
			shiftStatus = srsIdle;
			vic->vicSpriteArmedOrActive = vic->vicSpriteArmedOrActive & ~(bit16)spriteBit;
		}
	}
}

void VIC6569::SetSystemInterrupt()
{
	ClockNextWakeUpClock = CurrentClock;
	cpu->Set_VIC_IRQ(CurrentClock);
}

void VIC6569::ClearSystemInterrupt()
{
	cpu->Clear_VIC_IRQ();
}

void VIC6569::WRITE_FORE_MASK_STD(bit8 gData, signed char xscroll, bit8 cycle)
{
bit16u dataw;
bit8 mask = 0xff;
bit16u maskw;
bit8s byteOffset = (xscroll/8);

	dataw.word = (bit16) (gData) << 8;
	dataw.word >>= (xscroll & 7);

	maskw.word = (bit16) (mask) << 8;
	maskw.word >>= (xscroll & 7);

	bit8 p1 = pixelMaskBuffer[cycle-1 + byteOffset];
	bit8 p2 = pixelMaskBuffer[cycle+0 + byteOffset];

	pixelMaskBuffer[cycle-1 + byteOffset] = (p1 & ~maskw.byte.hiByte) | dataw.byte.hiByte;
	pixelMaskBuffer[cycle+0 + byteOffset] = (p2 & ~maskw.byte.loByte) | dataw.byte.loByte;
}

void VIC6569::WRITE_FORE_MASK_STD_EX(bit8 gData, signed char xscroll, bit8 xstart, bit8 count, bit8 cycle)
{
bit16u dataw;
bit8 mask;
bit16u maskw;
bit8s byteOffset = (xscroll/8);

	gData <<= xstart;
	mask = (((bit8)0xff) << (8-count));
	gData = gData & mask;

	dataw.word = (bit16) (gData) << 8;
	dataw.word >>= (xscroll & 7);

	maskw.word = (bit16) (mask) << 8;
	maskw.word >>= (xscroll & 7);

	bit8 p1 = pixelMaskBuffer[cycle-1 + byteOffset];
	bit8 p2 = pixelMaskBuffer[cycle+0 + byteOffset];

	pixelMaskBuffer[cycle-1 + byteOffset] = (p1 & ~maskw.byte.hiByte) | dataw.byte.hiByte;
	pixelMaskBuffer[cycle+0 + byteOffset] = (p2 & ~maskw.byte.loByte) | dataw.byte.loByte;
}

void VIC6569::WRITE_FORE_MASK_MCM(bit8 gData, signed char xscroll, bit8 cycle)
{
bit16u dataw;
bit8 mask = 0xff;
bit16u maskw;
bit8s byteOffset = (xscroll/8);

	dataw.word = (bit16)  (foregroundMask_mcm[gData] ) << 8;
	dataw.word >>= (xscroll & 7);

	maskw.word = (bit16) (mask) << 8;
	maskw.word >>= (xscroll & 7);

	bit8 p1 = pixelMaskBuffer[cycle-1 + byteOffset];
	bit8 p2 = pixelMaskBuffer[cycle+0 + byteOffset];

	pixelMaskBuffer[cycle-1 + byteOffset] = (p1 & ~maskw.byte.hiByte) | dataw.byte.hiByte;
	pixelMaskBuffer[cycle+0 + byteOffset] = (p2 & ~maskw.byte.loByte) | dataw.byte.loByte;
}

void VIC6569::WRITE_FORE_MASK_MCM_EX(bit8 gData, signed char xscroll, bit8 xstart, bit8 count, bit8 cycle)
{
bit16u dataw;
bit8 mask;
bit16u maskw;
bit8s byteOffset = (xscroll/8);

	gData = foregroundMask_mcm[gData];
	gData <<= xstart;
	mask = (((bit8)0xff) << (8-count));
	gData = gData & mask;

	dataw.word = (bit16)gData << 8;
	dataw.word >>= (xscroll & 7);

	maskw.word = (bit16) (mask) << 8;
	maskw.word >>= (xscroll & 7);

	bit8 p1 = pixelMaskBuffer[cycle-1 + byteOffset];
	bit8 p2 = pixelMaskBuffer[cycle+0 + byteOffset];

	pixelMaskBuffer[cycle-1 + byteOffset] = (p1 & ~maskw.byte.hiByte) | dataw.byte.hiByte;
	pixelMaskBuffer[cycle+0 + byteOffset] = (p2 & ~maskw.byte.loByte) | dataw.byte.loByte;
}

//Valid for 63 >= cycle >= 3
void VIC6569::WRITE_COLOR_BYTE8(bit8 color, signed char xscroll, bit8 cycle)
{
bit8 *p;
	p=vic_pixelbuffer+(INT_PTR)((cycle*8 - 20) + (int)xscroll+8);
	*p++ = color;
	*p++ = color;
	*p++ = color;
	*p++ = color;
	*p++ = color;
	*p++ = color;
	*p++ = color;
	*p = color;
}

//Valid for 63 >= cycle >= 3
void VIC6569::COLOR_FOREGROUND(bit8 backgroundColor[], bit8 cycle)
{
bit8 *p;
	if (cycle<10 || cycle>62)
		return;
	
	p = vic_pixelbuffer + (INT_PTR)(((int)cycle*8 - 20));

	for (int i = 7; i >= 0; i--)
	{
		bit8s t = (bit8s)p[i];
		if (t < 0)
			p[i] = backgroundColor[t & 0xf];
	}
}

//Valid for 63 >= cycle >= 3
void VIC6569::WRITE_STD2_BYTE(bit8 gData, signed char xscroll,const bit8 color2[], bit8 cycle)
{
bit8 *pByte;

	pByte = vic_pixelbuffer + (INT_PTR)(((int)cycle*8 - 20) + (int)xscroll + 8);

	for (int i = 7 ; i >= 0; i--)
	{
		pByte[i] = color2[gData & 1];
		gData >>= 1;
	}
}

/* 
valid inputs
cycle 3 - 63
xstart 0 - 7
count 0 - 8
*/
void VIC6569::WRITE_STD2_BYTE_EX(bit8 gData, signed char xscroll,const bit8 color4[], bit8 cycle, bit8 xstart, bit8 count)
{
bit8 *pByte;
	if ((signed char) count <=0)
		return;
	pByte = vic_pixelbuffer + (INT_PTR)(((int)cycle*8 - 20) + (int)xscroll + 8);

	gData <<= xstart;
	gData >>= (8-count);
	for (int i = count-1 ; i >= 0; i--)
	{
		pByte[i] = color4[gData & 1];
		gData >>= 1;
	}
}

//Valid for 63 >= cycle >= 3
void VIC6569::WRITE_MCM4_BYTE (bit8 gData, signed char xscroll, const bit8 color4[], bit8 cycle)
{
bit8 *pByte;

	pByte = vic_pixelbuffer + ((int)cycle*8 - 20) + (int)xscroll + 8;

	for (int i = 6 ; i >= 0; i-=2)
	{
		bit16 w;
		w = color4[gData & 3];
		w |= (w << 8);
		*((bit16 *)(pByte+i)) = w;
		gData >>= 2;
	}
}

#pragma intrinsic(_rotl8, _rotl16)

/* 
valid inputs
cycle 3 - 63
xstart 0 - 7
count 0 - 8
*/
void VIC6569::WRITE_MCM4_BYTE_EX (bit8 gData, signed char xscroll,const bit8 color2[], bit8 cycle, bit8 xstart, bit8 count)
{
bit8 *pByte;
	if ((signed char) count <=0)
		return;
	pByte=vic_pixelbuffer + (INT_PTR)(((int)cycle*8 - 20) + (int)xscroll + 8);

	bit8 v;
	gData <<= (xstart & 0xfe);

	int i = 0;
	if (xstart & 1) 
	{
		gData = _rotl8(gData, 2);
		v = color2[gData & 3];
		pByte[i++] = v;
		count--;
	}
	while (count != 0)
	{
		gData = _rotl8(gData, 2);
		v = color2[gData & 3];
		pByte[i++] =v;
		--count;
		if (count == 0)
			break;
		pByte[i++] = v;
		--count;
	}
}


//Valid for 63 >= cycle >= 3
#define DRAW_BORDER(cycle) ptr8 = &vic_borderbuffer[((int)cycle*8 - 20)];\
	if (vicMainBorder)\
	{\
		data8 = BORDERCOLORINDEX;\
		*ptr8++ = data8;\
		*ptr8++ = data8;\
		*ptr8++ = data8;\
		*ptr8++ = data8;\
		*ptr8++ = data8;\
		*ptr8++ = data8;\
		*ptr8++ = data8;\
		*ptr8 = data8;\
	}


//Valid for 63 >= cycle >= 3
void VIC6569::DrawBorder(bit8 cycle) 
{
bit8 c;
bit8 *p;
	
	p = (bit8 *)(&vic_borderbuffer[((int)cycle*8 - 20)]);
	if (vicMainBorder)
	{ 
		c = BORDERCOLORINDEX;
		*p++ = c;
		*p++ = c;
		*p++ = c;
		*p++ = c;
		*p++ = c;
		*p++ = c;
		*p++ = c;
		*p = c;
	}
}

//Valid for 63 >= cycle >= 3
void VIC6569::DrawBorder4(bit8 cycle) 
{
bit8 c;
bit8 *p;

	p = (&vic_borderbuffer[((int)cycle*8 - 20)]);
	if (vicMainBorder_old)
	{
		c = BORDERCOLORINDEX;
		*p++ = c;
		*p++ = c;
		*p++ = c;
		*p = c;
	}
}

void VIC6569::DrawBorder7(bit8 cycle) 
{
bit8 *p;
bit8 c;

	p=&vic_borderbuffer[((int)cycle*8 - 20)];
	if (vicMainBorder)
	{
		c = BORDERCOLORINDEX;
		*p++ = c;
		*p++ = c;
		*p++ = c;
		*p++ = c;
		*p++ = c;
		*p++ = c;
		*p = c;
	}
}

void VIC6569::check_38_col_right_border()
{
	if (vicCSEL==0)
	{
		vicMainBorder=1;
	}
}

void VIC6569::draw_38_col_right_border1(bit8 cycle)
{
bit8 *p;
	p=&vic_borderbuffer[((int)cycle*8 - 20) + 7];
	if (vicMainBorder)
	{
		*p=(bit8) BORDERCOLORINDEX;
	}
}

void VIC6569::check_40_col_right_border()
{
	if (vicCSEL==1)
	{
		vicMainBorder=1;
	}
}

void VIC6569::draw_38_col_left_border(bit8 cycle)
{
bit8 *p;
bit8 c;
	p = &vic_borderbuffer[((int)cycle*8 - 20)];
	if (vic_border_part_38 & 1)
	{
		c = (bit8) BORDERCOLORINDEX;
		*p++ = c;
		*p++ = c;
		*p++ = c;
		*p++ = c;
		*p++ = c;
		*p++ = c;
		*p++ = c;
	}

	p = &vic_borderbuffer[((int)cycle*8 - 20) +7];
	if (vic_border_part_38 & 2)
	{
		c = (bit8) BORDERCOLORINDEX;
		*p++=c;
	}
}

void VIC6569::check_38_col_left_border()
{
	vic_border_part_38 = vicMainBorder;
	if (vicCSEL==0)
	{
		if (vicVerticalBorder==0) 
			vicMainBorder=0;
	}
	vic_border_part_38 |= (vicMainBorder<<1);
}

void VIC6569::draw_40_col_left_border1(bit8 cycle)
{
bit8 *p;
bit8 c;

	p = &vic_borderbuffer[((int)cycle*8 - 20) + 4];
	if (vic_border_part_40 & 1)
	{
		c = (bit8) BORDERCOLORINDEX;
		*p++ = c;
		*p++ = c;
		*p++ = c;
		*p = c;
	}
}

void VIC6569::draw_40_col_left_border2(bit8 cycle)
{
bit8 *p;
bit8 c;
	p = &vic_borderbuffer[((int)cycle*8 - 20)];
	if (vic_border_part_40 & 2)
	{
		c = (bit8) BORDERCOLORINDEX;
		*p++ = c;
		*p++ = c;
		*p++ = c;
		*p = c;
	}
}

void VIC6569::check_40_col_left_border()
{
	vic_border_part_40=vicMainBorder;
	if (vicCSEL==1)
	{
		if (vicVerticalBorder==0)
			vicMainBorder=0;
	}
	vic_border_part_40|=(vicMainBorder<<1);
}

void VIC6569::C_ACCESS()
{	
	if (vic_allow_c_access)
	{
		if (vicAEC<0)
			VideoMatrix[vicVMLI] = vic_ph2_read_byte(vicMemptrVM | vicVC) | ((bit16)vic_read_color_byte(vicVC) << 8);
		else
		{
			VideoMatrix[vicVMLI] = ((bit16)(cpu_next_op_code & 0xF) << 8) | 0xFF;
		}
	}
}



bit8 VIC6569::G_ACCESS(const bit8 ecm_bmm_mcm, bit32& lastCData)
{
bit8 gData;
	if (vicIDLE | vicIDLE_DELAY)
	{
		//fix for stray black pixels in Scorpion demo
		if (vicCharDataOutputDisabled==0)
		{
			lastCData = 0;
		}

		switch (ecm_bmm_mcm)
		{
		case 0: // idle std text
			//g access
			gData = vic_ph1_read_3fff_byte();
			break;
		case 1: // idle mcm text
			//g access
			gData = vic_ph1_read_3fff_byte();
			break;
		case 2: // idle bitmap
			//g access
			//no load of lastCData to prevent stray black pixels in the heart screen of the scorpion demo
			gData = vic_ph1_read_3fff_byte();
			break;
		case 3: // idle multi color bitmap
			//g access
			gData = vic_ph1_read_3fff_byte();
			break;
		case 4: // idle ecm text
			//g access
			gData = vic_ph1_read_byte(0x39ff);
			break;
		case 5: //idle invalid text mode
			//g access
			gData = vic_ph1_read_byte(0x39ff);
			break;
		case 6: // idle invalid bitmap mode1
			//g access
			gData = vic_ph1_read_byte(0x39ff);
			break;
		case 7: //idle invalid bitmap mode2
			//g access
			gData = vic_ph1_read_byte(0x39ff);
			break;
		}
		if (vicIDLE_DELAY)
		{
			vicIDLE_DELAY=0;
			gData = 0;
		}
	}
	else
	{
		//fix for stray black pixels in Scorpion demo
		if (vicCharDataOutputDisabled==0)
		{
			bit8 iWritevicVMLI1 = vicVMLI;
			if ( vic_badline || vicRC != 0)
			{
				lastCData = VideoMatrix[iWritevicVMLI1];
			}
			else
			{
				//Odd case where the last badline was started less than 40 clocks ago yet the VIC is in display state and there is currenly no badline.
				ICLK lastBadlineCount = CurrentClock - clockFirstForcedBadlineCData - 1;
				if (lastBadlineCount >= NUM_SCREEN_COLUMNS)
				{
					lastCData = VideoMatrix[iWritevicVMLI1];
				}
				else
				{
					bit8 iWritevicVMLI2 = (bit8)(lastBadlineCount & 0xff);
					bit32 cdata_carry;
					bit8 gap = iWritevicVMLI2 - iWritevicVMLI1;
					bit32& char0 = VideoMatrix[(iWritevicVMLI2-1) % gap];
					bit32& char1 = VideoMatrix[iWritevicVMLI1];
					bit32& char2 = VideoMatrix[iWritevicVMLI2];
					if (((vic_raster_cycle +1) & 3) == 0)
					{
						 cdata_carry = 0x8a6 & char0;
					}
					else
					{
						cdata_carry = 0x8a6 & (vicCDataCarry | char0);
						
					}
					lastCData = char1 = char2 = ((cdata_carry | (char2 & 0x13f )) & (char1 | char2)) | (~cdata_carry & (char1 & char2));
					vicCDataCarry = cdata_carry;
				}
			}
		}
		switch (ecm_bmm_mcm)
		{
		case 0: // std text
			//g access
			gData = vic_ph1_read_byte(vicMemptrCB | (((bit16)lastCData & 0x00FF)<<3) | vicRC);
			break;
		case 1: // mcm text
			//g access
			gData = vic_ph1_read_byte(vicMemptrCB | (((bit16)lastCData & 0x00FF)<<3) | vicRC);
			break;
		case 2: // std bitmap
			//g access
			gData = vic_ph1_read_byte((vicMemptrCB & 0x2000) | vicVC<<3 | vicRC);
			break;
		case 3: // multi color bitmap
			//g access
			gData = vic_ph1_read_byte((vicMemptrCB & 0x2000) | vicVC<<3 | vicRC);
			break;
		case 4: //ecm text
			//g access
			gData = vic_ph1_read_byte(vicMemptrCB | (((bit16)lastCData & 0x003F)<<3) | vicRC);
			break;
		case 5: //invalid text mode
			//g access
			gData = vic_ph1_read_byte(vicMemptrCB | (((bit16)lastCData & 0x003F)<<3) | vicRC);
			break;
		case 6: //invalid bitmap mode1
			//g access
			gData = vic_ph1_read_byte((vicMemptrCB & 0x2000) | (vicVC & 0x33f)<<3 | vicRC);
			break;
		case 7: //invalid bitmap mode2
			//g access
			gData = vic_ph1_read_byte((vicMemptrCB & 0x2000) | (vicVC & 0x33f)<<3 | vicRC);
		}

		vicVC = (vicVC +1) & 0x3ff;
		vicVMLI++;
	}
	
	return gData;
}

bit8 VIC6569::vic_ph1_read_byte(bit16 address)
{
	return (de00_byte=vic_memory_map_read[(address & 0x3fff)>>12][address & 0x3fff]);
}

bit8 VIC6569::vic_ph1_read_3fff_byte()
{
	return (de00_byte = *vic_3fff_ptr);
}

bit8 VIC6569::vic_ph2_read_byte(bit16 address)
{
	if (m_bVicBankChanging)
	{
		m_bVicBankChanging = false;
		SetMMU(vicBankChangeByte);
	}
	return (vic_memory_map_read[(address & 0x3fff)>>12][address & 0x3fff]);
}


bit8 VIC6569::vic_ph2_read_aec_byte(bit16 address)
{
	if (m_bVicBankChanging)
	{
		m_bVicBankChanging = false;
		SetMMU(vicBankChangeByte);
	}
	if (vicAEC<0)
		return vic_memory_map_read[(address & 0x3fff)>>12][address & 0x3fff];
	else
		return 0xFF;
}

bit8 VIC6569::vic_ph2_read_3fff_aec_byte()
{
	if (m_bVicBankChanging)
	{
		m_bVicBankChanging = false;
		SetMMU(vicBankChangeByte);
	}
	if (vicAEC<0)
		return *vic_3fff_ptr;
	else
		return 0xFF;
}

bit8 VIC6569::vic_read_color_byte(bit16 address)
{
  return ram->mColorRAM[address & 0x03ff] & 0xF;
}

void VIC6569::SetMMU(bit8 index)
{
	ram->ConfigureVICMMU(index, &vic_memory_map_read, &vic_3fff_ptr);
	vicMemoryBankIndex = index;

}

void VIC6569::DrawForeground0(const bit8 xscroll,bit32 cData, bit8 ecm_bmm_mcm, const bit8 cycle)
{
bit8 color;

	bit8 pixelsToSkip = DF_PixelsToSkip;
	if (pixelsToSkip > 0)
	{
		DF_PixelsToSkip = 0;
		DrawForegroundEx(0, xscroll + (signed char)pixelsToSkip, cData, ecm_bmm_mcm, 0, 0, pixelsToSkip, 8 - pixelsToSkip, cycle);
		return;
	}

	switch (ecm_bmm_mcm)
	{
	case 0: // std text
		color = BACKCOLORINDEX0;
		break;
	case 1: // mcm text
		color = BACKCOLORINDEX0;
		break;
	case 2: // std bitmap
		//process_mode_change_lag(cData, ecm_bmm_mcm, ecm_bmm_mcm_prev, 0);
		color =((bit8)(cData)) & 0xf;
		break;
	case 3: // multi color bitmap
		color = BACKCOLORINDEX0;
		break;
	case 4: //ecm text
		//process_mode_change_lag(cData, ecm_bmm_mcm, ecm_bmm_mcm_prev, 0);
		switch (((bit8)(cData) & 0xC0) >> 6)
		{
		case 0:
			color=(bit8)BACKCOLORINDEX0;
			break;
		case 1:
			color=(bit8)BACKCOLORINDEX1;
			break;
		case 2:
			color=(bit8)BACKCOLORINDEX2;
			break;
		case 3:
			color=(bit8)BACKCOLORINDEX3;
			break;
		}
		break;
	case 5: //invalid text mode
		color=(bit8)vicBLACK;
		break;
	case 6: //invalid bitmap mode1
		color=(bit8)vicBLACK;
		break;
	case 7: //invalid bitmap mode2
		color=(bit8)vicBLACK;
		break;
	}
	WRITE_COLOR_BYTE8(color, xscroll, cycle);
}

void VIC6569::DrawForeground(bit8 gData, const signed char xscroll, bit32 cData, bit8 ecm_bmm_mcm, bit8 pixelshiftmode, const bit8 verticalBorder, const bit8 cycle)
{
bit8 color;
bit8 a_color4[4];

	if (verticalBorder)
		gData = 0;

	bit8 pixelsToSkip = DF_PixelsToSkip;
	if (pixelsToSkip > 0)
	{
		DF_PixelsToSkip = 0;
		DrawForegroundEx(gData, xscroll + (signed char)pixelsToSkip, cData, ecm_bmm_mcm, pixelshiftmode, verticalBorder, pixelsToSkip, 8 - pixelsToSkip, cycle);
		return;
	}

	switch (ecm_bmm_mcm)
	{
	case 0: // std text
		//gfx pixel write
		a_color4[0]=(bit8)BACKCOLORINDEX0;
		a_color4[1]=((bit8)(cData>>8)) & 0xf;
		WRITE_STD2_BYTE(gData, xscroll, a_color4, cycle);
		
		//data mask store
		WRITE_FORE_MASK_STD(gData, xscroll, cycle);
		break;
	case 1: // mcm text
		color = (bit8)(cData>>8);
		if (color & 8)
		{
			//gfx pixel write
			a_color4[0]=(bit8)BACKCOLORINDEX0;
			a_color4[1]=(bit8)BACKCOLORINDEX1;
			a_color4[2]=(bit8)BACKCOLORINDEX2;
			a_color4[3]=color & 7;
			WRITE_MCM4_BYTE(gData, xscroll, a_color4, cycle);

			//data mask store
			WRITE_FORE_MASK_MCM(gData, xscroll, cycle);
		}
		else
		{
			//gfx pixel write
			a_color4[0]=(bit8)BACKCOLORINDEX0;
			a_color4[1]=color & 7;
			WRITE_STD2_BYTE(gData, xscroll, a_color4, cycle);
		
			//data mask store
			WRITE_FORE_MASK_STD(gData, xscroll, cycle);
		}
		break;
	case 2:// std bitmap
		//gfx pixel write
		a_color4[0]=((bit8)(cData)) & 0xf;
		a_color4[1]=((bit8)(cData >> 4))  & 0xf;
		WRITE_STD2_BYTE(gData, xscroll, a_color4, cycle);
		
		//data mask store
		WRITE_FORE_MASK_STD(gData, xscroll, cycle);
		break;
	case 3: // multi color bitmap
		a_color4[0]=(bit8)BACKCOLORINDEX0;
		a_color4[1]=((bit8)(cData >> 4)) & 0xf;
		a_color4[2]=((bit8)(cData)) & 0xf;
		a_color4[3]=((bit8)(cData >> 8)) & 0xf;
		WRITE_MCM4_BYTE(gData, xscroll, a_color4, cycle);

		//data mask store
		WRITE_FORE_MASK_MCM(gData, xscroll, cycle);
		break;
	case 4: //ecm text
		//gfx pixel write
		a_color4[1]=(bit8)(cData >> 8);
		switch (((bit8)(cData) & 0xC0) >> 6)
		{
		case 0:
			a_color4[0]=(bit8)BACKCOLORINDEX0;
			break;
		case 1:
			a_color4[0]=(bit8)BACKCOLORINDEX1;
			break;
		case 2:
			a_color4[0]=(bit8)BACKCOLORINDEX2;
			break;
		case 3:
			a_color4[0]=(bit8)BACKCOLORINDEX3;
			break;
		}
		WRITE_STD2_BYTE(gData, xscroll, a_color4, cycle);
		
		//data mask store
		WRITE_FORE_MASK_STD(gData, xscroll, cycle);
		break;
	case 5: //invalid text mode
		color = (bit8)(cData>>8);
		if (color & 8)
		{
			//gfx pixel write
			a_color4[0]=(bit8)vicBLACK;
			a_color4[1]=(bit8)vicBLACK;
			a_color4[2]=(bit8)vicBLACK;
			a_color4[3]=(bit8)vicBLACK;
			WRITE_MCM4_BYTE(gData, xscroll, a_color4, cycle);

			//data mask store
			WRITE_FORE_MASK_MCM(gData, xscroll, cycle);
		} 
		else
		{
			//gfx pixel write
			a_color4[0]=(bit8)vicBLACK;
			a_color4[1]=(bit8)vicBLACK;
			WRITE_STD2_BYTE(gData, xscroll, a_color4, cycle);
		
			//data mask store
			WRITE_FORE_MASK_STD(gData, xscroll, cycle);
		}
		break;
	case 6: //invalid bitmap mode1
		//gfx pixel write
		a_color4[0]=(bit8)vicBLACK;
		a_color4[1]=(bit8)vicBLACK;
		WRITE_STD2_BYTE(gData, xscroll, a_color4, cycle);
		
		//data mask store
		WRITE_FORE_MASK_STD(gData, xscroll, cycle);
		break;
	case 7: //invalid bitmap mode2
		//gfx pixel write
		a_color4[0]=(bit8)vicBLACK;
		a_color4[1]=(bit8)vicBLACK;
		a_color4[2]=(bit8)vicBLACK;
		a_color4[3]=(bit8)vicBLACK;
		WRITE_MCM4_BYTE(gData, xscroll, a_color4, cycle);

		//data mask store
		WRITE_FORE_MASK_MCM(gData, xscroll, cycle);
		break;
	}
}

void VIC6569::DrawForegroundEx(bit8 gData, const signed char xscroll, bit32 cData, bit8 ecm_bmm_mcm, bit8 pixelshiftmode, const bit8 verticalBorder, const bit8 xstart, const bit8 count, const bit8 cycle)
{
bit8 color;
bit8 a_color4[4];

	if (count==0 || count > 8 || xstart > 8)
		return;
	if (verticalBorder)
		gData = 0;

	bit8 pixelsToSkip = DF_PixelsToSkip;
	if (pixelsToSkip > 0)
	{
		DF_PixelsToSkip = 0;
		DrawForegroundEx(gData, xscroll + (signed char)pixelsToSkip, cData, ecm_bmm_mcm, pixelshiftmode, verticalBorder, xstart + pixelsToSkip, count - pixelsToSkip, cycle);
		return;
	}

	switch (ecm_bmm_mcm)
	{
	case 0: // std text
		//gfx pixel write
		a_color4[0]=(bit8)BACKCOLORINDEX0;
		a_color4[1]=((bit8)(cData>>8)) & 0xf;
		WRITE_STD2_BYTE_EX(gData, xscroll, a_color4, cycle, xstart, count);
		
		//data mask store
		WRITE_FORE_MASK_STD_EX(gData, xscroll, xstart, count, cycle);
		break;
	case 1: // mcm text
		color = (bit8)(cData>>8);
		if (color & 8)
		{
			if (pixelshiftmode == 0)
			{
				//gfx pixel write
				a_color4[0]=(bit8)BACKCOLORINDEX0;
				a_color4[1]=(bit8)BACKCOLORINDEX1;
				a_color4[2]=(bit8)BACKCOLORINDEX2;
				a_color4[3]=color & 7;
				WRITE_MCM4_BYTE_EX(gData, xscroll, a_color4, cycle, xstart, count);

				//data mask store
				WRITE_FORE_MASK_MCM_EX(gData, xscroll, xstart, count, cycle);
			}
			else
			{
				//1 bit of g-data paired with a zero is passed through the colour selector.
				//new emulation detect from Mariusz
				//gfx pixel write
				a_color4[0]=(bit8)BACKCOLORINDEX0;
				a_color4[1]=(bit8)BACKCOLORINDEX2;
				WRITE_STD2_BYTE_EX(gData, xscroll, a_color4, cycle, xstart, count);
			
				//data mask store
				WRITE_FORE_MASK_STD_EX(gData, xscroll, xstart, count, cycle);
			}
		}
		else
		{
			//Parados, Coma Light 87; with 1 bit of g-data mode
			//gfx pixel write
			a_color4[0]=(bit8)BACKCOLORINDEX0;
			a_color4[1]=color & 7;
			WRITE_STD2_BYTE_EX(gData, xscroll, a_color4, cycle, xstart, count);
		
			//data mask store
			WRITE_FORE_MASK_STD_EX(gData, xscroll, xstart, count, cycle);
		}
		break;
	case 2:// std bitmap
		//gfx pixel write
		a_color4[0]=((bit8)(cData)) & 0xf;
		a_color4[1]=((bit8)(cData >> 4))  & 0xf;
		WRITE_STD2_BYTE_EX(gData, xscroll, a_color4, cycle, xstart, count);
		
		//data mask store
		WRITE_FORE_MASK_STD_EX(gData, xscroll, xstart, count, cycle);
		break;
	case 3: // multi color bitmap
		if (pixelshiftmode == 0)
		{
			a_color4[0]=(bit8)BACKCOLORINDEX0;
			a_color4[1]=((bit8)(cData >> 4)) & 0xf;
			a_color4[2]=((bit8)(cData)) & 0xf;
			a_color4[3]=((bit8)(cData >> 8)) & 0xf;
			WRITE_MCM4_BYTE_EX(gData, xscroll, a_color4, cycle, xstart, count);

			//data mask store
			WRITE_FORE_MASK_MCM_EX(gData, xscroll, xstart, count, cycle);
		}
		else
		{
			//TEST
			//Inverse1 and Dont Meet Crest Note can go here.
			//Onslaught Bobby Bounceback+4 Burkah
			//gfx pixel write
			a_color4[0]=(bit8)BACKCOLORINDEX0;
			a_color4[1]=((bit8)(cData)) & 0xf;
			WRITE_STD2_BYTE_EX(gData, xscroll, a_color4, cycle, xstart, count);
		
			//data mask store
			WRITE_FORE_MASK_STD_EX(gData, xscroll, xstart, count, cycle);
		}
		break;
	case 4: //ecm text
		//gfx pixel write
		a_color4[1]=(bit8)(cData >> 8);
		switch (((bit8)(cData) & 0xC0) >> 6)
		{
		case 0:
			a_color4[0]=(bit8)BACKCOLORINDEX0;
			break;
		case 1:
			a_color4[0]=(bit8)BACKCOLORINDEX1;
			break;
		case 2:
			a_color4[0]=(bit8)BACKCOLORINDEX2;
			break;
		case 3:
			a_color4[0]=(bit8)BACKCOLORINDEX3;
			break;
		}
		WRITE_STD2_BYTE_EX(gData, xscroll, a_color4, cycle, xstart, count);
		
		//data mask store
		WRITE_FORE_MASK_STD_EX(gData, xscroll, xstart, count, cycle);
		break;
	case 5: //invalid text mode
		color = (bit8)(cData>>8);
		if (color & 8)
		{
			if (pixelshiftmode == 0)
			{
				//gfx pixel write
				a_color4[0]=(bit8)vicBLACK;
				a_color4[1]=(bit8)vicBLACK;
				a_color4[2]=(bit8)vicBLACK;
				a_color4[3]=(bit8)vicBLACK;
				WRITE_MCM4_BYTE_EX(gData, xscroll, a_color4, cycle, xstart, count);

				//data mask store
				WRITE_FORE_MASK_MCM_EX(gData, xscroll, xstart, count, cycle);
			}
			else
			{
				//gfx pixel write
				a_color4[0]=(bit8)vicBLACK;
				a_color4[1]=(bit8)vicBLACK;
				WRITE_STD2_BYTE_EX(gData, xscroll, a_color4, cycle, xstart, count);
			
				//data mask store
				WRITE_FORE_MASK_STD_EX(gData, xscroll, xstart, count, cycle);
			}
		}
		else
		{
			//gfx pixel write
			a_color4[0]=(bit8)vicBLACK;
			a_color4[1]=(bit8)vicBLACK;
			WRITE_STD2_BYTE_EX(gData, xscroll, a_color4, cycle, xstart, count);
		
			//data mask store
			WRITE_FORE_MASK_STD_EX(gData, xscroll, xstart, count, cycle);
		}
		break;
	case 6: //invalid bitmap mode1
		//gfx pixel write
		a_color4[0]=(bit8)vicBLACK;
		a_color4[1]=(bit8)vicBLACK;
		WRITE_STD2_BYTE_EX(gData, xscroll, a_color4, cycle, xstart, count);
		
		//data mask store
		WRITE_FORE_MASK_STD_EX(gData, xscroll, xstart, count, cycle);
		break;
	case 7: //invalid bitmap mode2
		//gfx pixel write
		if (pixelshiftmode == 0)
		{
			a_color4[0]=(bit8)vicBLACK;
			a_color4[1]=(bit8)vicBLACK;
			a_color4[2]=(bit8)vicBLACK;
			a_color4[3]=(bit8)vicBLACK;
			WRITE_MCM4_BYTE_EX(gData, xscroll, a_color4, cycle, xstart, count);

			//data mask store
			WRITE_FORE_MASK_MCM_EX(gData, xscroll, xstart, count, cycle);
		}
		else
		{
			//gfx pixel write
			a_color4[0]=(bit8)vicBLACK;
			a_color4[1]=(bit8)vicBLACK;
			WRITE_STD2_BYTE_EX(gData, xscroll, a_color4, cycle, xstart, count);
			
			//data mask store
			WRITE_FORE_MASK_STD_EX(gData, xscroll, xstart, count, cycle);
		}
		break;
	}
}


VIC6569::VIC6569()
{
	m_pBackBuffer = NULL;
	ram=NULL;
	cpu=NULL;
	dx=NULL;
	cfg=NULL;
	appStatus=NULL;
	vic_pixelbuffer=NULL;
	//vic_borderbuffer=NULL;
	for (int i=0; i < 8; i++)
	{
		vicSprite[i].vic = this;
	}
	FrameNumber = 0;
	m_iLastBackedUpFrameNumber = -1;
}

void VIC6569::Reset(ICLK sysclock)
{
int i,j;
bit32 initial_raster_line = PAL_MAX_LINE;
	CurrentClock = sysclock;
	ClockNextWakeUpClock=sysclock;
	FrameNumber = 0;
	m_iLastBackedUpFrameNumber = -1;
	vicMemoryBankIndex = 0;
	LP_TRIGGER=0;
	vicLightPen=1;
	vic_check_irq_in_cycle2=0;
	vicAEC=3;
	vicMainBorder=1;
	vicVerticalBorder=1;
	vicCharDataOutputDisabled=1;
	vicIDLE=1;
	vicIDLE_DELAY=0;
	vicLastCData=0;
	vicLastCDataPrev=0;
	vicLastCDataPrev2=0;
	vicCDataCarry=0;
	vicLastGData=0;
	vicLastGDataPrev=0;
	vicLastGDataPrev2=0;
	ClearSystemInterrupt();

	vic_in_display_y=0;

	vic_top_compare=0x37;
	vic_bottom_compare=0xf7;
	vic_left_compare=0x1f;
	vic_right_compare=0x14f;


	// control reg 1
	vicECM=0;
	vicBMM=0;
	vicDEN=0;
	vicRSEL=0;
	vic_top_compare=0x37;
	vic_bottom_compare=0xf7;
	/*
		if vicRSEL = 1
		vic_top_compare=0x33;
		vic_bottom_compare=0xfb;
	*/
	vicYSCROLL=0;

	// control reg 2
	vicRES=0;
	vicMCM=0;
	vicCSEL=0;
	vicXSCROLL=0;

	//Hacky 9 pixel in the border accommodation; FIXME
	vicXSCROLL_Cycle57=0;


	//Memory pointers
	vicMemptrVM=0;
	vicMemptrCB=0;

	//Screen colors
	vicBorderColor=0;
	memset(vicBackgroundColor, 0, sizeof(vicBackgroundColor));

	//RASTER
	vicRASTER_compare=0;
	bVicRasterMatch = false;
	vic_raster_line = initial_raster_line;
	vic_raster_cycle = 63;

	//INTERRUPT
	vicINTERRUPT_STATUS=0;
	vicINTERRUPT_ENABLE=0;

	//Internal
	vicVC=0;
	vicRC=0;
	vicVCBASE=0;
	vicVMLI=0;
	clockFirstForcedBadlineCData = sysclock - NUM_SCREEN_COLUMNS;
	vicECM_BMM_MCM=0;
	vicECM_BMM_MCM_prev=0;
	vicDRAMRefresh = 0xff;

	m_bVicBankChanging = false;
	m_bVicModeChanging = false;

	//Sprites
	for (i=0 ; i<8 ; i++)
	{
		vicSprite[i].spriteNumber = i;
		vicSprite[i].spriteBit = (1 << i);
		vicSprite[i].dataLoadClock = vic_spr_data_load_x[i];
		vicSprite[i].dataLoadedClock = vicSprite[i].dataLoadClock + 13;
		vicSprite[i].dataLoadIndex = SpriteIndexFromClock(vicSprite[i].dataLoadClock);
		vicSprite[i].Reset();
		vicSpriteX[i]=0;
		vicSpriteY[i]=0;
		MC_INCR[i]=3;
		SpriteXChange(i,0,1);
	}
	vicSpriteArmedOrActive = 0;
	vicSpriteDMA=0;
	vicSpriteDMAPrev=0;
	vicSpriteDisplay=0;
	vicSpriteYMatch=0;

	vic_badline=0;
	line_info= &BA_line_info[vicSpriteDMA];
	vic_address_line_info = &(*line_info)[vic_badline];

	ff_YP=0;
	vicSpriteMSBX=0;
	vicSpriteEnable=0;
	vicSpriteYExpand=0;
	vicSpriteDataPriority=0;
	vicSpriteMultiColor=0;
	vicSpriteMultiColorPrev=0;
	clockSpriteMultiColorChange=0;
	clockSpriteXExpandChange=0;
	clockSpriteDataPriorityChange=0;
	clockReadSpriteDataCollision=0;
	clockReadSpriteSpriteCollision=0;

	vicSpriteXExpand=0;
	vicSpriteXExpandPrev=0;
	vicSpriteDataPriorityPrev=0;
	vicSpriteSpriteCollision=0;
	vicSpriteDataCollision=0;

	vicNextSprite_sprite_collision=0;
	vicNextSprite_data_collision=0;
	vicCurrSprite_sprite_collision=0;
	vicCurrSprite_data_collision=0;
	vicSpriteSpriteInt=0;
	vicSpriteDataInt=0;

	for (i=0 ; i < _countof(ScreenPixelBuffer[PIXELBUFFER_MAIN_INDEX]) ; i++)
	{
		for (j=0 ; j < _countof(ScreenPixelBuffer[PIXELBUFFER_MAIN_INDEX][0])  ; j++)
		{
			ScreenPixelBuffer[PIXELBUFFER_MAIN_INDEX][i][j]=vicBLACK;
			ScreenPixelBuffer[PIXELBUFFER_BACKUP_INDEX][i][j]=vicBLACK;
		}
	}

	vic_pixelbuffer = ScreenPixelBuffer[PIXELBUFFER_MAIN_INDEX][initial_raster_line];
	ZeroMemory(pixelMaskBuffer,sizeof(pixelMaskBuffer));
	DF_PixelsToSkip = 0;
}

void VIC6569::PreventClockOverflow()
{
	const ICLKS CLOCKSYNCBAND_NEAR = 0x4000;
	const ICLKS CLOCKSYNCBAND_FAR = 0x40000000;
	ICLK ClockBehindNear = CurrentClock - CLOCKSYNCBAND_NEAR;
	
	if ((ICLKS)(CurrentClock - clockSpriteMultiColorChange) >= CLOCKSYNCBAND_FAR)
	{
		clockSpriteMultiColorChange = ClockBehindNear;
		vicSpriteMultiColorPrev = vicSpriteMultiColor;
	}
	if ((ICLKS)(CurrentClock - clockSpriteXExpandChange) >= CLOCKSYNCBAND_FAR)
	{
		clockSpriteXExpandChange = ClockBehindNear;
		vicSpriteXExpandPrev = vicSpriteXExpand;
	}
	if ((ICLKS)(CurrentClock - clockSpriteDataPriorityChange) >= CLOCKSYNCBAND_FAR)
	{
		clockSpriteDataPriorityChange = ClockBehindNear;
		vicSpriteDataPriorityPrev = vicSpriteDataPriority;
	}

	if ((ICLKS)(CurrentClock - clockReadSpriteDataCollision) > CLOCKSYNCBAND_FAR)
		clockReadSpriteDataCollision = ClockBehindNear;
	if ((ICLKS)(CurrentClock - clockReadSpriteSpriteCollision) > CLOCKSYNCBAND_FAR)
		clockReadSpriteSpriteCollision = ClockBehindNear;

	if ((ICLKS)(CurrentClock - clockFirstForcedBadlineCData) > CLOCKSYNCBAND_FAR)
		clockFirstForcedBadlineCData = ClockBehindNear;
}

VIC6569::~VIC6569()
{
	Cleanup();
}

HRESULT VIC6569::Init(CConfig *cfg, CAppStatus *appStatus, CDX9 *dx, RAM64 *ram, CPU6510 *cpu)
{
	Cleanup();

	this->cfg = cfg;
	this->appStatus = appStatus;
	this->dx = dx;
	this->ram = ram;
	this->cpu = cpu;

	SetMMU(0);
	setup_multicolor_mask_table();
	setup_vic_ba();

	Reset(CurrentClock);
	return S_OK;
}

void VIC6569::SetLPLine(bit8 lineState)
{
const signed char LIGHTPENOFFSET = 6;
	if (vicLightPen!=0 && lineState==0)
	{
		if (LP_TRIGGER==0 && vic_raster_line != PAL_MAX_LINE)
		{
			LP_TRIGGER=1;
			vicINTERRUPT_STATUS|=8;
			if (vicINTERRUPT_STATUS & vicINTERRUPT_ENABLE & 0x8)
			{
				SetSystemInterrupt();
			}

			bit16 xpos = GetVicXPosFromCycle(vic_raster_cycle, LIGHTPENOFFSET);
			vic_lpx = (bit8)(xpos / 2);

			vic_lpy=(bit8)vic_raster_line;
			if (xpos +2 >= 0x194 && (xpos + 2 - LIGHTPENOFFSET) < 0x194)
				vic_lpy++;
		}
	}	
	vicLightPen = lineState;
}

void VIC6569::SetLPLineClk(ICLK sysclock, bit8 lineState)
{
	ExecuteCycle(sysclock);
	SetLPLine(lineState);
}

void VIC6569::setup_multicolor_mask_table()
{
bit16 i;
bit8 k;

	for (i=0 ; i < 256 ; i++)
	{
		k = (i & 0xAA) | ((i & 0xAA) >> 1);
		foregroundMask_mcm[(bit8)i]=k;
	}
	
}

void VIC6569::setup_color_tables(D3DFORMAT format)
{
bit16 i;
bit8 red,green,blue;
bit32 cl;
IDirect3DSurface9 *pSurface;

	for (i=16 ; i < 256 ; i++)
	{
		vic_color_array[i]=vic_color_array[i & 15];
		vic_color_array8[i] = (bit8)i;
	}

	//32
	for (i=0 ; i < 256 ; i++)
	{
		red=(bit8)((vic_color_array[i & 15] & 0x00ff0000) >> 16);
		green=(bit8)((vic_color_array[i & 15] & 0x0000ff00) >> 8);
		blue=(bit8)((vic_color_array[i & 15] & 0x000000ff));

		cl = dx->ConvertColour(format, RGB(red, green, blue));
		vic_color_array32[i] =  cl;		
	}

	//24
	for (i=0 ; i < 256 ; i++)
	{
		red=(bit8)((vic_color_array[i & 15] & 0x00ff0000) >> 16);
		green=(bit8)((vic_color_array[i & 15] & 0x0000ff00) >> 8);
		blue=(bit8)((vic_color_array[i & 15] & 0x000000ff));

		cl = dx->ConvertColour(format, RGB(red, green, blue));
		vic_color_array24[i] = cl;		
	}

	//16
	for (i=0 ; i < 256 ; i++)
	{
		red=(bit8)((vic_color_array[i & 15] & 0x00ff0000) >> 16);
		green=(bit8)((vic_color_array[i & 15] & 0x0000ff00) >> 8);
		blue=(bit8)((vic_color_array[i & 15] & 0x000000ff));

		cl = dx->ConvertColour(format, RGB(red, green, blue));
		vic_color_array16[i] = (bit16) cl;		
	}

	switch (format)
	{
		case D3DFMT_P8:
		case D3DFMT_A8:
		case D3DFMT_L8:
			if (D3D_OK == dx->m_pd3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pSurface))
			{
				if (pSurface)
				{
				
					//8
					for (i=0 ; i < 256 ; i++)
					{
						red=(bit8)((vic_color_array[i & 15] & 0x00ff0000) >> 16);
						green=(bit8)((vic_color_array[i & 15] & 0x0000ff00) >> 8);
						blue=(bit8)((vic_color_array[i & 15] & 0x000000ff));

						cl= (bit8) dx->DDColorMatch(pSurface, RGB(red, green, blue));
						vic_color_array8[i] = (bit8) cl;		
					}
					pSurface->Release();
					pSurface = NULL;
				}
			}
			break;
	}
}

void VIC6569::setup_vic_ba()
{
int i,j,sprite_dma;

	for (i=1 ; i < 256 ; i++)
	{
		for (j=1 ; j <= 63 ; j++)
		{
			BA_line_info[i][0][j] = BA_line_info[0][0][j];
			BA_line_info[i][1][j] = BA_line_info[0][1][j];
		}
	}
	
	for (sprite_dma=0 ; sprite_dma < 256 ; sprite_dma++)
	{
		if (sprite_dma & 1)
		{
			BA_line_info[sprite_dma][0][55] = 0;
			BA_line_info[sprite_dma][1][55] = 0;
			BA_line_info[sprite_dma][0][56] = 0;
			BA_line_info[sprite_dma][1][56] = 0;
			BA_line_info[sprite_dma][0][57] = 0;
			BA_line_info[sprite_dma][1][57] = 0;
			BA_line_info[sprite_dma][0][58] = 0;
			BA_line_info[sprite_dma][1][58] = 0;
			BA_line_info[sprite_dma][0][59] = 0;
			BA_line_info[sprite_dma][1][59] = 0;
		}
		if (sprite_dma & 2)
		{
			BA_line_info[sprite_dma][0][57] = 0;
			BA_line_info[sprite_dma][1][57] = 0;
			BA_line_info[sprite_dma][0][58] = 0;
			BA_line_info[sprite_dma][1][58] = 0;
			BA_line_info[sprite_dma][0][59] = 0;
			BA_line_info[sprite_dma][1][59] = 0;
			BA_line_info[sprite_dma][0][60] = 0;
			BA_line_info[sprite_dma][1][60] = 0;
			BA_line_info[sprite_dma][0][61] = 0;
			BA_line_info[sprite_dma][1][61] = 0;
		}

		if (sprite_dma & 4)
		{
			BA_line_info[sprite_dma][0][59] = 0;
			BA_line_info[sprite_dma][1][59] = 0;
			BA_line_info[sprite_dma][0][60] = 0;
			BA_line_info[sprite_dma][1][60] = 0;
			BA_line_info[sprite_dma][0][61] = 0;
			BA_line_info[sprite_dma][1][61] = 0;
			BA_line_info[sprite_dma][0][62] = 0;
			BA_line_info[sprite_dma][1][62] = 0;
			BA_line_info[sprite_dma][0][63] = 0;
			BA_line_info[sprite_dma][1][63] = 0;
		}

		if (sprite_dma & 8)
		{
			BA_line_info[sprite_dma][0][61] = 0;
			BA_line_info[sprite_dma][1][61] = 0;
			BA_line_info[sprite_dma][0][62] = 0;
			BA_line_info[sprite_dma][1][62] = 0;
			BA_line_info[sprite_dma][0][63] = 0;
			BA_line_info[sprite_dma][1][63] = 0;
			BA_line_info[sprite_dma][0][1] = 0;
			BA_line_info[sprite_dma][1][1] = 0;
			BA_line_info[sprite_dma][0][2] = 0;
			BA_line_info[sprite_dma][1][2] = 0;
		}

		if (sprite_dma & 16)
		{
			BA_line_info[sprite_dma][0][63] = 0;
			BA_line_info[sprite_dma][1][63] = 0;
			BA_line_info[sprite_dma][0][1] = 0;
			BA_line_info[sprite_dma][1][1] = 0;
			BA_line_info[sprite_dma][0][2] = 0;
			BA_line_info[sprite_dma][1][2] = 0;
			BA_line_info[sprite_dma][0][3] = 0;
			BA_line_info[sprite_dma][1][3] = 0;
			BA_line_info[sprite_dma][0][4] = 0;
			BA_line_info[sprite_dma][1][4] = 0;
		}

		if (sprite_dma & 32)
		{
			BA_line_info[sprite_dma][0][2] = 0;
			BA_line_info[sprite_dma][1][2] = 0;
			BA_line_info[sprite_dma][0][3] = 0;
			BA_line_info[sprite_dma][1][3] = 0;
			BA_line_info[sprite_dma][0][4] = 0;
			BA_line_info[sprite_dma][1][4] = 0;
			BA_line_info[sprite_dma][0][5] = 0;
			BA_line_info[sprite_dma][1][5] = 0;
			BA_line_info[sprite_dma][0][6] = 0;
			BA_line_info[sprite_dma][1][6] = 0;
		}

		if (sprite_dma & 64)
		{
			BA_line_info[sprite_dma][0][4] = 0;
			BA_line_info[sprite_dma][1][4] = 0;
			BA_line_info[sprite_dma][0][5] = 0;
			BA_line_info[sprite_dma][1][5] = 0;
			BA_line_info[sprite_dma][0][6] = 0;
			BA_line_info[sprite_dma][1][6] = 0;
			BA_line_info[sprite_dma][0][7] = 0;
			BA_line_info[sprite_dma][1][7] = 0;
			BA_line_info[sprite_dma][0][8] = 0;
			BA_line_info[sprite_dma][1][8] = 0;
		}

		if (sprite_dma & 128)
		{
			BA_line_info[sprite_dma][0][6] = 0;
			BA_line_info[sprite_dma][1][6] = 0;
			BA_line_info[sprite_dma][0][7] = 0;
			BA_line_info[sprite_dma][1][7] = 0;
			BA_line_info[sprite_dma][0][8] = 0;
			BA_line_info[sprite_dma][1][8] = 0;
			BA_line_info[sprite_dma][0][9] = 0;
			BA_line_info[sprite_dma][1][9] = 0;
			BA_line_info[sprite_dma][0][10] = 0;
			BA_line_info[sprite_dma][1][10] = 0;
		}
	}
}

void VIC6569::Cleanup()
{
}


void VIC6569::BackupMainPixelBuffers()
{
int i, j;
	for (i=0 ; i < _countof(ScreenPixelBuffer[PIXELBUFFER_MAIN_INDEX]) ; i++)
	{
		for (j=0 ; j < _countof(ScreenPixelBuffer[PIXELBUFFER_MAIN_INDEX][0])  ; j++)
		{
			ScreenPixelBuffer[PIXELBUFFER_BACKUP_INDEX][i][j] = ScreenPixelBuffer[PIXELBUFFER_MAIN_INDEX][i][j];
		}
	}
	m_iLastBackedUpFrameNumber = FrameNumber;
}

//HRESULT VIC6569::UpdateBackBufferLine(bit16 line, bit8 cycle)
//{
//HRESULT hr = E_FAIL;
//D3DLOCKED_RECT lrLockRect; 
//
//	if (dx == NULL)
//		return E_FAIL;
//	if (dx->m_pd3dDevice == NULL)
//		return E_FAIL;
//	if (line < 0 || line > PAL_MAX_LINE)
//		return E_FAIL;
//	if (cycle < 1 || cycle > PAL_CLOCKS_PER_LINE)
//		return E_FAIL;
//
//#ifdef USESYSMEMSURFACE
//	IDirect3DSurface9 *pBackBuffer = dx->GetSysMemSurface();
//#else
//	IDirect3DSurface9 *pBackBuffer = dx->GetSmallSurface();
//#endif
//	if (pBackBuffer)
//	{
//		hr = pBackBuffer->LockRect(&lrLockRect, NULL, D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
//		if (SUCCEEDED(hr))
//		{
//			hr = UpdateBackBufferLine((bit8 *) lrLockRect.pBits, lrLockRect.Pitch, line, cycle);
//			pBackBuffer->UnlockRect();
//		}
//		pBackBuffer->Release();
//		pBackBuffer = NULL;
//	}
//	return hr;
//}

HRESULT VIC6569::UpdateBackBufferLine(bit8 *pDestSurfLine, int videoPitch, bit16 line, bit8 cycle)
{
	if (dx == NULL)
		return E_FAIL;
	if (dx->m_pd3dDevice == NULL)
		return E_FAIL;
	if (line < 0 || line > PAL_MAX_LINE)
		return E_FAIL;
	if (cycle < 1 || cycle > PAL_CLOCKS_PER_LINE)
		return E_FAIL;


	int buffer_line = 0;
	int current_line = line;
	int cursor_index = ((int)cycle*8 - 20);
	cursor_index += 8;
	if (cursor_index < 0)
	{
		current_line--;
		if (current_line < 0)
			current_line = PAL_MAX_LINE;
	}
	int start_line = current_line;

	cursor_index = (cursor_index + VIDEOWIDTH) % VIDEOWIDTH;

	//Overwrite any in place pixel pipeline artifacts with data from the last backed up frame.
	const int MINCORRECTIVEPIXELS = 32;
	int i = 0;
	int j = 0;
	bool bIsLeftOfCursor = true;
	while (buffer_line <= 1)
	{
		if (bIsLeftOfCursor && i == cursor_index)
			bIsLeftOfCursor = false;
		if (bIsLeftOfCursor)
			LinePixelBuffer[buffer_line][i] = ScreenPixelBuffer[PIXELBUFFER_MAIN_INDEX][current_line][i];
		else
		{
			LinePixelBuffer[buffer_line][i] = ScreenPixelBuffer[PIXELBUFFER_BACKUP_INDEX][current_line][i];
			j++;
		}
		i++;
		if (i >= VIDEOWIDTH)
		{
			i = 0;
			buffer_line++;
			current_line++;
			if (current_line > PAL_MAX_LINE)
				current_line = 0;
			if (j >= MINCORRECTIVEPIXELS)
				break;
		}
	}

	int height = 1;
	if (buffer_line > _countof(LinePixelBuffer))
		height = _countof(LinePixelBuffer);
	else
		height = buffer_line;

	int width = dx->m_displayWidth;
	int startx = dx->m_displayStart;
	int starty = start_line;

	int ypos = 0;
	int xpos = 0;
	int bufferPitch = _countof(ScreenPixelBuffer[PIXELBUFFER_MAIN_INDEX][0]);

	for (int h = 0; h < height; h++)
	{
		if ((unsigned int)starty >= dx->m_displayFirstVicRaster && (unsigned int)starty <= dx->m_displayLastVicRaster)
		{
			bit8 *pPixelBuffer = LinePixelBuffer[h];

			ypos = starty - dx->m_displayFirstVicRaster;
			if (appStatus->m_bUseCPUDoubler)
			{
				ypos = ypos * 2;
			}

			render(appStatus->m_ScreenDepth, appStatus->m_bUseCPUDoubler, pDestSurfLine, xpos, ypos, width, 1, pPixelBuffer, startx, videoPitch, bufferPitch);
		}
		starty++;
		if (starty >= PAL_MAX_LINE)
		{
			starty = 0;
		}
	}

	return S_OK;
}

HRESULT VIC6569::UpdateBackBuffer()
{
HRESULT hr = E_FAIL;
D3DLOCKED_RECT lrLockRect; 

	if (dx == NULL)
		return E_FAIL;
	if (dx->m_pd3dDevice == NULL)
		return E_FAIL;
#ifdef USESYSMEMSURFACE
	IDirect3DSurface9 *pBackBuffer = dx->GetSysMemSurface();
#else
	IDirect3DSurface9 *pBackBuffer = dx->GetSmallSurface();
#endif
	if (pBackBuffer)
	{
		hr = pBackBuffer->LockRect(&lrLockRect, NULL, D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
		if (SUCCEEDED(hr))
		{
			bit8 *pDestSurfLine = (bit8 *) lrLockRect.pBits;
			int height = dx->m_displayHeight;
			int width = dx->m_displayWidth;
			int startx = dx->m_displayStart;

			int bufferPitch = _countof(ScreenPixelBuffer[PIXELBUFFER_MAIN_INDEX][0]);
			bit8 *pPixelBuffer = ScreenPixelBuffer[PIXELBUFFER_MAIN_INDEX][dx->m_displayFirstVicRaster];

			render(appStatus->m_ScreenDepth, appStatus->m_bUseCPUDoubler, pDestSurfLine, 0, 0, width, height, pPixelBuffer, startx, lrLockRect.Pitch, bufferPitch);

			if (appStatus->m_bDebug)
			{
				hr = UpdateBackBufferLine(pDestSurfLine, lrLockRect.Pitch, (bit16)vic_raster_line, vic_raster_cycle);
			}

			pBackBuffer->UnlockRect();
		}
		pBackBuffer->Release();
		pBackBuffer = NULL;
	}
	return hr;
}

void VIC6569::SpriteArm(int spriteNo)
{
	VICSprite& sprite = vicSprite[spriteNo];
	
	if (sprite.shiftStatus == VICSprite::srsIdle)
	{
		sprite.shiftStatus = VICSprite::srsArm;
		vicSpriteArmedOrActive = vicSpriteArmedOrActive | (bit16)sprite.spriteBit;
		if ((vicSpriteDisplay & (1 << spriteNo)) == 0)
		{
			//If the sprite display was off then any sprite at x==0x163 gets drawn at 0x164
			if (sprite.xPos == SPRITE_DISPLAY_CHECK_XPOS - 1)
			{
				sprite.InitDraw(SPRITE_DISPLAY_CHECK_XPOS, SPRITE_DISPLAY_CHECK_XPOS, sprite.xPixelInit + 1);
				sprite.shiftStatus = VICSprite::srsActiveXChange;
			}
		}
	}
	sprite.armDelay=0;
	
}

void VIC6569::SpriteIdle(int spriteNo)
{
	VICSprite& sprite = vicSprite[spriteNo];
	if (sprite.shiftStatus == VICSprite::srsArm)
	{
		sprite.shiftStatus = VICSprite::srsIdle;
		vicSpriteArmedOrActive = vicSpriteArmedOrActive & ~(bit16)sprite.spriteBit;
	}
}


void VIC6569::SetBA(ICLK &cycles, bit8 cycle)
{
	bit8 ba = (*vic_address_line_info)[cycle];
	if (ba)
	{
		cpu->SetBAHigh(CurrentClock);
		vicAEC=3;
	}
	else
	{
		cpu->SetBALow(CurrentClock);
		if (vicAEC>=0)
			--vicAEC;
		//If cpu->isWriteCycle == false or cycles != 0 then it means that the cpu has just had read cycle that should have caused a cpu BA delay.
		//This is because we synchronise the vic with all cpu-write cycles but allow the cpu to run ahead with reads from RAM.
		//'cycles' will always be zero if we call here on a cpu write-cycle
		if (!cpu->m_bDebug)
		{
			if (!cpu->m_bIsWriteCycle || cycles != 0)
			{
				cpu->AddClockDelay();
				++cycles;
			}
		}
	}
}

void VIC6569::init_line_start()
{
	switch (vic_raster_line)
	{
	case 1:
		break;
	case 16:
		vic_in_display_y=1;
		break;
	case 0x30:
		vic_latchDEN=vicDEN;
		break;
	case 300:
		vic_in_display_y=0;
		break;
	case PAL_MAX_LINE:
		vic_in_display_y=0;
		break;
	}
	if (vic_raster_line>=0x30 && vic_raster_line<=0xf7 && ((vic_raster_line & 7)==vicYSCROLL) && vic_latchDEN!=0)
	{
		vic_badline=1;
		vicIDLE=0;
		vic_address_line_info = &BA_line_info[vicSpriteDMA][1];
	}
	else
	{
		vic_badline=0;
		vic_address_line_info = &BA_line_info[vicSpriteDMA][0];
	}

	if (vic_raster_line == vic_top_compare && vicDEN!=0)
	{
		vicVerticalBorder=0;
		vicCharDataOutputDisabled=0;
	}

	if (vic_raster_line == vic_bottom_compare)
	{
		vicVerticalBorder=1;
		vicCharDataOutputDisabled=1;
	}
}

//pragma optimize( "g", off )
#define LOADSPRITEDATA(spriteNo, column) ((bit8 *)(&vicSpriteData[spriteNo]))[column]

void VIC6569::ExecuteCycle(ICLK sysclock)
{
int i;
bit8 ibit8;
bit8 cycle;
bit8 cyclePrev;
ICLK clocks;
bool bNextLineCouldMayBeBad;
bit32 nextLine;
bit8 *ptr8;
bit8 data8;

	clocks = sysclock - CurrentClock;
	while ((ICLKS)clocks-- > 0)
	{
		CurrentClock++;
		cyclePrev = vic_raster_cycle++;
		if (vic_raster_cycle > 63)
			vic_raster_cycle = 1;
		cycle = vic_raster_cycle;
	
		switch (cycle)
		{
		case 1:
			if (vicSpriteArmedOrActive != 0)
				DrawSprites(cyclePrev);
			if(vic_raster_line == PAL_MAX_LINE)
			{
				vic_check_irq_in_cycle2=1;
				vic_pixelbuffer = ScreenPixelBuffer[PIXELBUFFER_MAIN_INDEX][0];
				FrameNumber++;
				if (appStatus->m_bDebug)
					BackupMainPixelBuffers();
			}
			else
			{
				vic_raster_line++;
				vic_pixelbuffer = ScreenPixelBuffer[PIXELBUFFER_MAIN_INDEX][vic_raster_line];
				if (vic_raster_line == vicRASTER_compare)
				{
					if (!bVicRasterMatch)
					{
						bVicRasterMatch = true;
						vicINTERRUPT_STATUS |= 0x1; // Raster Interrupt Flag
						if ((vicINTERRUPT_STATUS & vicINTERRUPT_ENABLE & 0x1)!=0)
						{
							SetSystemInterrupt();
						}
					}
				}
				else
					bVicRasterMatch = false;
			}

			//TEST
			//if (vic_raster_line == 0xeb)//Onslaught Bobby Bounceback+4 Burkah
			//	vic_raster_line = vic_raster_line;
			//if (vic_raster_line == 0xf5)//Onslaught Bobby Bounceback+4 Burkah
			//	vic_raster_line = vic_raster_line;
			//if (vic_raster_line == 0x93)//Chromance scroller
			//	vic_raster_line = vic_raster_line;
			//if (vic_raster_line == 0x39)//Patterzoom
			//	vic_raster_line = vic_raster_line;
			//if (vic_raster_line == 0x87)//comalight 87
			//	vic_raster_line = vic_raster_line;
			//if (vic_raster_line == 0x7f)
			//	vic_raster_line = vic_raster_line;
			//if (vic_raster_line == 0xee)
			//	vic_raster_line = vic_raster_line;
			//if (vic_raster_line == 0xba)
			//	vic_raster_line = vic_raster_line;
			//if (vic_raster_line == 0xc9)//Starion sprite 7
			//	vic_raster_line = vic_raster_line;

			init_line_start();
			SetBA(clocks, cycle);

			pixelMaskBuffer[16] = pixelMaskBuffer[56]=0;
			ZeroMemory(vic_sprite_collision_line,sizeof(vic_sprite_collision_line));

			vicSpritePointer[sprite3] = (bit16)vic_ph1_read_byte(vicMemptrVM + 0x3f8 + sprite3) * 64;
			if (vicSpriteDMA & (1<<sprite3))
			{
				LOADSPRITEDATA(sprite3, 2) = vic_ph2_read_byte(vicSpritePointer[sprite3] + MC[sprite3]);
				MC[sprite3] = (MC[sprite3] + 1) & 63;
			}
			else
				LOADSPRITEDATA(sprite3, 2) = vic_ph2_read_3fff_aec_byte();//0xff;

			break;
		case 2:
			if (vicSpriteArmedOrActive != 0)
				DrawSprites(cyclePrev);
			if(vic_check_irq_in_cycle2)
			{
				vic_check_irq_in_cycle2=0;
				vic_raster_line=0;
				vicDRAMRefresh=0xFF;
				LP_TRIGGER=0;

				//TEST adjusted light pen behaviour
				if (vicLightPen==0)
				{
   					vicLightPen=1;
					SetLPLine(0);
				}
				
				vic_latchDEN=0;
				vic_in_display_y=0;
				vicVCBASE=0;

				if (vicRASTER_compare == 0)
				{
					if (!bVicRasterMatch)
					{
						bVicRasterMatch = true;
						vicINTERRUPT_STATUS |= 0x1; // Raster Interrupt Flag
						if ((vicINTERRUPT_STATUS & vicINTERRUPT_ENABLE & 0x1)!=0)
						{					
							SetSystemInterrupt();
						}
					}
				}
				else
					bVicRasterMatch = false;

			}
			SetBA(clocks, cycle);
			if (vicSpriteDMA & (1<<sprite3))
			{
				LOADSPRITEDATA(sprite3, 1) = vic_ph1_read_byte(vicSpritePointer[sprite3] + MC[sprite3]);
				MC[sprite3] = (MC[sprite3] + 1) & 63;
				LOADSPRITEDATA(sprite3, 0) = vic_ph2_read_byte(vicSpritePointer[sprite3] + MC[sprite3]);
			}
			else
			{
				LOADSPRITEDATA(sprite3, 1) = vic_ph1_read_3fff_byte();
				LOADSPRITEDATA(sprite3, 0) = vic_ph2_read_3fff_aec_byte();//0xff;
			}
			vicSprite[sprite3].dataBuffer = vicSpriteData[sprite3];
			break;
		case 3:
			if (vicSpriteArmedOrActive != 0)
				DrawSprites(cyclePrev);
			SetBA(clocks, cycle);
			vicSpritePointer[sprite4] = (bit16)vic_ph1_read_byte(vicMemptrVM + 0x3f8 + sprite4) * 64;
			if (vicSpriteDMA & (1<<sprite4))
			{
				LOADSPRITEDATA(sprite4, 2) = vic_ph2_read_byte(vicSpritePointer[sprite4] + MC[sprite4]);
				MC[sprite4] = (MC[sprite4] + 1) & 63;
			}
			else
				LOADSPRITEDATA(sprite4, 2) = vic_ph2_read_3fff_aec_byte();//0xff;
			break;
		case 4:
			if (vicSpriteArmedOrActive != 0)
				DrawSprites(cyclePrev);
			SetBA(clocks, cycle);
			if (vicSpriteDMA & (1<<sprite4))
			{
				LOADSPRITEDATA(sprite4, 1) = vic_ph1_read_byte(vicSpritePointer[sprite4] + MC[sprite4]);
				MC[sprite4] = (MC[sprite4] + 1) & 63;
				LOADSPRITEDATA(sprite4, 0) = vic_ph2_read_byte(vicSpritePointer[sprite4] + MC[sprite4]);
			}
			else
			{
				LOADSPRITEDATA(sprite4, 1) = vic_ph1_read_3fff_byte();
				LOADSPRITEDATA(sprite4, 0) = vic_ph2_read_3fff_aec_byte();//0xff;
			}
			vicSprite[sprite4].dataBuffer = vicSpriteData[sprite4];
			break;
		case 5:
			if (vicSpriteArmedOrActive != 0)
				DrawSprites(cyclePrev);
			SetBA(clocks, cycle);
			vicSpritePointer[sprite5] = (bit16)vic_ph1_read_byte(vicMemptrVM + 0x3f8 + sprite5) * 64;
			if (vicSpriteDMA & (1<<sprite5))
			{
				LOADSPRITEDATA(sprite5, 2) = vic_ph2_read_byte(vicSpritePointer[sprite5] + MC[sprite5]);
				MC[sprite5] = (MC[sprite5] + 1) & 63;
			}
			else
				LOADSPRITEDATA(sprite5, 2) = vic_ph2_read_3fff_aec_byte();//0xff;
			break;
		case 6:
			if (vicSpriteArmedOrActive != 0)
				DrawSprites(cyclePrev);
			SetBA(clocks, cycle);
			if (vicSpriteDMA & (1<<sprite5))
			{
				LOADSPRITEDATA(sprite5, 1) = vic_ph1_read_byte(vicSpritePointer[sprite5] + MC[sprite5]);
				MC[sprite5] = (MC[sprite5] + 1) & 63;
				LOADSPRITEDATA(sprite5, 0) = vic_ph2_read_byte(vicSpritePointer[sprite5] + MC[sprite5]);
			}
			else
			{
				LOADSPRITEDATA(sprite5, 1) = vic_ph1_read_3fff_byte();
				LOADSPRITEDATA(sprite5, 0) = vic_ph2_read_3fff_aec_byte();//0xff;
			}
			vicSprite[sprite5].dataBuffer = vicSpriteData[sprite5];
			break;
		case 7:
			if (vicSpriteArmedOrActive != 0)
				DrawSprites(cyclePrev);
			SetBA(clocks, cycle);
			vicSpritePointer[sprite6] = (bit16)vic_ph1_read_byte(vicMemptrVM + 0x3f8 + sprite6) * 64;
			if (vicSpriteDMA & (1<<sprite6))
			{
				LOADSPRITEDATA(sprite6, 2) = vic_ph2_read_byte(vicSpritePointer[sprite6] + MC[sprite6]);
				MC[sprite6] = (MC[sprite6] + 1) & 63;
			}
			else
				LOADSPRITEDATA(sprite6, 2) = vic_ph2_read_3fff_aec_byte();//0xff;
			break;
		case 8:
			if (vicSpriteArmedOrActive != 0)
				DrawSprites(cyclePrev);
			SetBA(clocks, cycle);
			if (vicSpriteDMA & (1<<sprite6))
			{
				LOADSPRITEDATA(sprite6, 1) = vic_ph1_read_byte(vicSpritePointer[sprite6] + MC[sprite6]);
				MC[sprite6] = (MC[sprite6] + 1) & 63;
				LOADSPRITEDATA(sprite6, 0) = vic_ph2_read_byte(vicSpritePointer[sprite6] + MC[sprite6]);
			}
			else
			{
				LOADSPRITEDATA(sprite6, 1) = vic_ph1_read_3fff_byte();
				LOADSPRITEDATA(sprite6, 0) = vic_ph2_read_3fff_aec_byte();//0xff;
			}
			vicSprite[sprite6].dataBuffer = vicSpriteData[sprite6];
			break;
		case 9:
			DrawForeground0(vicXSCROLL, vicLastCData, vicECM_BMM_MCM, cycle);
			if (vicSpriteArmedOrActive != 0)
				DrawSprites(cyclePrev);

			SetBA(clocks, cycle);
			vicSpritePointer[sprite7] = (bit16)vic_ph1_read_byte(vicMemptrVM + 0x3f8 + sprite7) * 64;
			if (vicSpriteDMA & (1<<sprite7))
			{
				LOADSPRITEDATA(sprite7, 2) = vic_ph2_read_byte(vicSpritePointer[sprite7] + MC[sprite7]);
				MC[sprite7] = (MC[sprite7] + 1) & 63;
			}
			else
				LOADSPRITEDATA(sprite7, 2) = vic_ph2_read_3fff_aec_byte();//0xff;
			break;
		case 10: //0x1e0:		
			DrawForeground0(vicXSCROLL, vicLastCData, vicECM_BMM_MCM, cycle);
			if (vicSpriteArmedOrActive != 0)
				DrawSprites(cyclePrev);
			COLOR_FOREGROUND(vicBackgroundColor, cycle);

			SetBA(clocks, cycle);
			if (vicSpriteDMA & (1<<sprite7))
			{
				LOADSPRITEDATA(sprite7, 1) = vic_ph1_read_byte(vicSpritePointer[sprite7] + MC[sprite7]);
				MC[sprite7] = (MC[sprite7] + 1) & 63;
				LOADSPRITEDATA(sprite7, 0) = vic_ph2_read_byte(vicSpritePointer[sprite7] + MC[sprite7]);
			}
			else
			{
				LOADSPRITEDATA(sprite7, 1) = vic_ph1_read_3fff_byte();
				LOADSPRITEDATA(sprite7, 0) = vic_ph2_read_3fff_aec_byte();//0xff;
			}
			vicSprite[sprite7].dataBuffer = vicSpriteData[sprite7];
			break;
		case 11:
			DrawForeground0(vicXSCROLL, vicLastCData, vicECM_BMM_MCM, cycle);
			if (vicSpriteArmedOrActive != 0)
				DrawSprites(cyclePrev);
			DRAW_BORDER(cycle);
			COLOR_FOREGROUND(vicBackgroundColor, cycle);

			SetBA(clocks, cycle);
			vic_ph1_read_byte(0x3f00 | vicDRAMRefresh--);
			break;
		case 12:
			//Left most colourised pixels in full borders mode.
			DrawForeground0(vicXSCROLL, vicLastCData, vicECM_BMM_MCM, cycle);
			if (vicSpriteArmedOrActive != 0)
				DrawSprites(cyclePrev);
			DRAW_BORDER(cycle);
			COLOR_FOREGROUND(vicBackgroundColor, cycle);

			SetBA(clocks, cycle);
			vic_ph1_read_byte(0x3f00 | vicDRAMRefresh--);
			break;
		case 13:
			DrawForeground0(vicXSCROLL, vicLastCData, vicECM_BMM_MCM, cycle);
			if (vicSpriteArmedOrActive != 0)
				DrawSprites(cyclePrev);
			DRAW_BORDER(cycle);
			COLOR_FOREGROUND(vicBackgroundColor, cycle);

			SetBA(clocks, cycle);
			vic_ph1_read_byte(0x3f00 | vicDRAMRefresh--);
			break;
		case 14:
			DrawForeground0(vicXSCROLL, vicLastCData, vicECM_BMM_MCM, cycle);
			if (vicSpriteArmedOrActive != 0)
				DrawSprites(cyclePrev);
			DRAW_BORDER(cycle);
			COLOR_FOREGROUND(vicBackgroundColor, cycle);

			SetBA(clocks, cycle);
			vic_ph1_read_byte(0x3f00 | vicDRAMRefresh--);
			vicVC = vicVCBASE;
			vicVMLI = 0;
			if (vic_badline)
			{
				vicRC = 0;
			}
			break;
		case 15:
			DrawForeground0(vicXSCROLL, vicLastCData, vicECM_BMM_MCM, cycle);
			if (vicSpriteArmedOrActive != 0)
				DrawSprites(cyclePrev);
			DRAW_BORDER(cycle);
			COLOR_FOREGROUND(vicBackgroundColor, cycle);

			SetBA(clocks, cycle);
			vic_ph1_read_byte(0x3f00 | vicDRAMRefresh--);
			vic_allow_c_access = vic_badline;
			vicIDLE_DELAY=0;

			C_ACCESS();
			break;
		case 16: //0x18:
			DrawForeground0(vicXSCROLL, vicLastCData, vicECM_BMM_MCM, cycle);
			if (vicSpriteArmedOrActive != 0)
				DrawSprites(cyclePrev);
			DRAW_BORDER(cycle);
			COLOR_FOREGROUND(vicBackgroundColor, cycle);

			vicSpriteDMAPrev = vicSpriteDMA;
			for (ibit8=1,i=0 ; i < 8 ; i++,ibit8<<=1)
			{
				if (ff_YP & ibit8)
					MCBASE[i]= (MCBASE[i]+MC_INCR[i]) & 63;
				if (MCBASE[i]==63)
				{
					vicSpriteDMA &= ~ibit8;
				}
			}

			vic_address_line_info = &BA_line_info[vicSpriteDMA][vic_badline];
			SetBA(clocks, cycle);
			vicMainBorder_old = vicMainBorder;

			vicLastCDataPrev2 = vicLastCDataPrev;
			vicLastGDataPrev2 = vicLastGDataPrev;
			vicLastCDataPrev = vicLastCData;
			vicLastGDataPrev = vicLastGData;
			vicLastGData = G_ACCESS(vicECM_BMM_MCM_prev, vicLastCData);
			C_ACCESS();
			break;
		case 17://0x1f:
			check_40_col_left_border();

			DrawForeground(vicLastGData, vicXSCROLL, vicLastCData, vicECM_BMM_MCM, 0, vicCharDataOutputDisabled, cycle);
			if (vicSpriteArmedOrActive != 0)
				DrawSprites(cyclePrev);
			DrawBorder4(cycle);//uses vicMainBorder_old
			draw_40_col_left_border1(cycle); // uses vic_border_part_40
			COLOR_FOREGROUND(vicBackgroundColor, cycle);

			SetBA(clocks, cycle);
			vicMainBorder_old = vicMainBorder;

			vicLastCDataPrev2 = vicLastCDataPrev;
			vicLastGDataPrev2 = vicLastGDataPrev;
			vicLastCDataPrev = vicLastCData;
			vicLastGDataPrev = vicLastGData;
			vicLastGData = G_ACCESS(vicECM_BMM_MCM_prev, vicLastCData);
			C_ACCESS();
			break;
		case 18:
			DrawForeground(vicLastGData, vicXSCROLL, vicLastCData, vicECM_BMM_MCM, 0, vicCharDataOutputDisabled, cycle);

			if (vicSpriteArmedOrActive != 0)
				DrawSprites(cyclePrev);

			check_38_col_left_border();
			draw_40_col_left_border2(cycle); // uses vic_border_part_40
			draw_38_col_left_border(cycle);
			COLOR_FOREGROUND(vicBackgroundColor, cycle);

			SetBA(clocks, cycle);

			vicLastCDataPrev2 = vicLastCDataPrev;
			vicLastGDataPrev2 = vicLastGDataPrev;
			vicLastCDataPrev = vicLastCData;
			vicLastGDataPrev = vicLastGData;
			vicLastGData = G_ACCESS(vicECM_BMM_MCM_prev, vicLastCData);
			C_ACCESS();
			break;
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
		case 31:
		case 32:
		case 33:
		case 34:
		case 35:
		case 36:
		case 37:
		case 38:
		case 39:
		case 40:
		case 41:
		case 42:
		case 43:
		case 44:
		case 45:
		case 46:
		case 47:
		case 48:
		case 49:
		case 50:
		case 51:
		case 52:
		case 53:
		case 54:
			DrawForeground(vicLastGData, vicXSCROLL, vicLastCData, vicECM_BMM_MCM, 0, vicCharDataOutputDisabled, cycle);
			if (vicSpriteArmedOrActive != 0)
				DrawSprites(cyclePrev);
			DRAW_BORDER(cycle);
			COLOR_FOREGROUND(vicBackgroundColor, cycle);

			SetBA(clocks, cycle);

			vicLastCDataPrev2 = vicLastCDataPrev;
			vicLastGDataPrev2 = vicLastGDataPrev;
			vicLastCDataPrev = vicLastCData;
			vicLastGDataPrev = vicLastGData;
			vicLastGData = G_ACCESS(vicECM_BMM_MCM_prev, vicLastCData);
			C_ACCESS();
			break;
		case 55:
			DrawForeground(vicLastGData, vicXSCROLL, vicLastCData, vicECM_BMM_MCM, 0, vicCharDataOutputDisabled, cycle);
			if (vicSpriteArmedOrActive != 0)
				DrawSprites(cyclePrev);
			DRAW_BORDER(cycle);
			COLOR_FOREGROUND(vicBackgroundColor, cycle);

			vicSpriteYMatch=0;
			ff_YP= ((~ff_YP) & vicSpriteYExpand) | (~vicSpriteYExpand & ff_YP);

			for (ibit8=1,i=0 ; i < 8 ; i++)
			{
				if ((~vicSpriteDMA & vicSpriteEnable & ibit8)!=0 && vicSpriteY[i]==(vic_raster_line & 0xFF) )
				{
					vicSpriteDMA|=ibit8;
					MCBASE[i]=0;
					if (vicSpriteYExpand & ibit8)
						ff_YP &= ~ibit8;
				}
				ibit8<<=1;
			}

			vic_address_line_info = &BA_line_info[vicSpriteDMA][0];
			SetBA(clocks, cycle);

			vic_allow_c_access = FALSE;

			vicLastCDataPrev2 = vicLastCDataPrev;
			vicLastGDataPrev2 = vicLastGDataPrev;
			vicLastCDataPrev = vicLastCData;
			vicLastGDataPrev = vicLastGData;
			vicLastGData = G_ACCESS(vicECM_BMM_MCM_prev, vicLastCData);
			break;
		case 56:
			DrawForeground(vicLastGData, vicXSCROLL, vicLastCData, vicECM_BMM_MCM, 0, vicCharDataOutputDisabled, cycle);

			if (vicCharDataOutputDisabled!=0)
				pixelMaskBuffer[cycle] = 0;
			if (vicSpriteArmedOrActive != 0)
				DrawSprites(cyclePrev);
			DrawBorder7(cycle);
			check_38_col_right_border();
			draw_38_col_right_border1(cycle);
			COLOR_FOREGROUND(vicBackgroundColor, cycle);

			for (ibit8=1,i=0 ; i < 8 ; i++,ibit8<<=1)
			{
				if ((~vicSpriteDMA & vicSpriteEnable & ibit8)!=0 && vicSpriteY[i]==(vic_raster_line & 0xFF))
				{
					MCBASE[i]=0;
					vicSpriteDMA|=ibit8;
					if (vicSpriteYExpand & ibit8)
						ff_YP &= ~ibit8;
				}
				MC_INCR[i] = (ff_YP & ibit8) ? 3 : 0;
			}

			vic_address_line_info = &BA_line_info[vicSpriteDMA][0];
			SetBA(clocks, cycle);
			vic_ph1_read_3fff_byte();

			//If a write to $D016 occurs in cycle 56 then we simulate the X-Scroll going to 7 to ensure that any 9 pixel characters get displayed.
			//Hacky 9 pixel in the border accommodation; FIXME
			vicXSCROLL_Cycle57 = vicXSCROLL;

			vicLastCDataPrev2 = vicLastCDataPrev;
			vicLastGDataPrev2 = vicLastGDataPrev;
			vicLastCDataPrev = vicLastCData;
			vicLastGDataPrev = vicLastGData;
			vicLastGData = 0;
			break;
		case 57:
			//DrawForeground0(vicXSCROLL, vicLastCData, vicECM_BMM_MCM, cycle);
			//Hacky 9 pixel in the border accommodation; FIXME
			DrawForegroundEx(0, vicXSCROLL_Cycle57, vicLastCData, vicECM_BMM_MCM, 0, vicCharDataOutputDisabled, 0, 8-vicXSCROLL_Cycle57+vicXSCROLL, cycle);
			vicXSCROLL_Cycle57 = vicXSCROLL;

			if (vicSpriteArmedOrActive != 0)
				DrawSprites(cyclePrev);
			DRAW_BORDER(cycle);
			check_40_col_right_border();
			COLOR_FOREGROUND(vicBackgroundColor, cycle);

			SetBA(clocks, cycle);
			vic_ph1_read_3fff_byte();

			vicLastCDataPrev2 = vicLastCDataPrev;
			vicLastGDataPrev2 = vicLastGDataPrev;
			vicLastGDataPrev = 0;
			break;
		case 58:
			DrawForeground0(vicXSCROLL, vicLastCData, vicECM_BMM_MCM, cycle);
			if (vicSpriteArmedOrActive != 0)
				DrawSprites(cyclePrev);
			DRAW_BORDER(cycle);
			COLOR_FOREGROUND(vicBackgroundColor, cycle);

			vicLastGDataPrev2 = 0;

			SetBA(clocks, cycle);

			vicSpriteDisplay &= vicSpriteDMA;
			for (ibit8=1,i=0 ; i < 8 ; i++,ibit8<<=1)
			{
				MC[i]=MCBASE[i];
				//Fixed bugged circle in Technological	Snow by Agony
				if (((vicSpriteDMA & vicSpriteEnable & ibit8)!=0) && vicSpriteY[i]==(vic_raster_line & 0xFF))
				{
					SpriteArm(i);
					vicSpriteDisplay|=ibit8;
				}

				if ((vicSpriteDisplay & ibit8) ==0)
					SpriteIdle(i);
			}

			vicSpritePointer[sprite0] = (bit16)vic_ph1_read_byte(vicMemptrVM + 0x3f8 + sprite0) * 64;
			if (vicSpriteDMA & (1<<sprite0))
			{
				//Enabling a sprite in cycle 55 may not allow the 3 cycle notice need for the VIC gain access to the bus during second clock half.
				LOADSPRITEDATA(sprite0, 2) = vic_ph2_read_aec_byte(vicSpritePointer[sprite0] + MC[sprite0]);
				MC[sprite0] = (MC[sprite0] + 1) & 63;
			}
			else
				LOADSPRITEDATA(sprite0, 2) = vic_ph2_read_3fff_aec_byte();//0xff;

			if (vicRC==7)
			{
				if (vic_badline==0) 
				{
					vicIDLE=1;
				}
				vicVCBASE = vicVC;
			}
			if (vicIDLE==0)
				vicRC=(vicRC +1) & 7;

			nextLine = (vic_raster_line + 1) % (PAL_MAX_LINE + 1);
			bNextLineCouldMayBeBad = false;
			if (nextLine>=0x30 && nextLine<=0xf7 && ((nextLine & 7)==vicYSCROLL))
			{
				bNextLineCouldMayBeBad = true;
			}
			if ((bNextLineCouldMayBeBad == false) && (vicSpriteDMA == 0) && (nextLine != 0 || (vicINTERRUPT_ENABLE & 0x8) == 0) && (nextLine != vicRASTER_compare || (vicINTERRUPT_ENABLE & 0x1) == 0) && (vicSpriteArmedOrActive == 0 || (vicINTERRUPT_ENABLE & 0x6) == 0))
			{
				ClockNextWakeUpClock = CurrentClock + 59;
			}
			else
			{
				ClockNextWakeUpClock = CurrentClock;
			}
			break;
		case 59:
			DrawForeground0(vicXSCROLL, vicLastCData, vicECM_BMM_MCM, cycle);
			if (vicSpriteArmedOrActive != 0)
				DrawSprites(cyclePrev);
			DRAW_BORDER(cycle);
			COLOR_FOREGROUND(vicBackgroundColor, cycle);

			SetBA(clocks, cycle);
			if (vicSpriteDMA & (1<<sprite0))
			{
				LOADSPRITEDATA(sprite0, 1) = vic_ph1_read_byte(vicSpritePointer[sprite0] + MC[sprite0]);
				MC[sprite0] = (MC[sprite0] + 1) & 63;
				LOADSPRITEDATA(sprite0, 0) = vic_ph2_read_byte(vicSpritePointer[sprite0] + MC[sprite0]);
			}
			else
			{
				LOADSPRITEDATA(sprite0, 1) = vic_ph1_read_3fff_byte();
				LOADSPRITEDATA(sprite0, 0) = vic_ph2_read_3fff_aec_byte();//0xff;
			}
			vicSprite[sprite0].dataBuffer = vicSpriteData[sprite0];
			break;
		case 60:
			DrawForeground0(vicXSCROLL, vicLastCData, vicECM_BMM_MCM, cycle);
			if (vicSpriteArmedOrActive != 0)
				DrawSprites(cyclePrev);
			DRAW_BORDER(cycle);
			COLOR_FOREGROUND(vicBackgroundColor, cycle);

			SetBA(clocks, cycle);
			vicSpritePointer[sprite1] = (bit16)vic_ph1_read_byte(vicMemptrVM + 0x3f8 + sprite1) * 64;
			if (vicSpriteDMA & (1<<sprite1))
			{
				LOADSPRITEDATA(sprite1, 2) = vic_ph2_read_byte(vicSpritePointer[sprite1] + MC[sprite1]);
				MC[sprite1] = (MC[sprite1] + 1) & 63;
			}
			else
				LOADSPRITEDATA(sprite1, 2) = vic_ph2_read_3fff_aec_byte();//0xff;
			break;
		case 61:
			DrawForeground0(vicXSCROLL, vicLastCData, vicECM_BMM_MCM, cycle);
			if (vicSpriteArmedOrActive != 0)
				DrawSprites(cyclePrev);
			DRAW_BORDER(cycle);
			COLOR_FOREGROUND(vicBackgroundColor, cycle);

			SetBA(clocks, cycle);
			if (vicSpriteDMA & (1<<sprite1))
			{
				LOADSPRITEDATA(sprite1, 1) = vic_ph1_read_byte(vicSpritePointer[sprite1] + MC[sprite1]);
				MC[sprite1] = (MC[sprite1] + 1) & 63;
				LOADSPRITEDATA(sprite1, 0) = vic_ph2_read_byte(vicSpritePointer[sprite1] + MC[sprite1]);
			}
			else
			{
				LOADSPRITEDATA(sprite1, 1) = vic_ph1_read_3fff_byte();
				LOADSPRITEDATA(sprite1, 0) = vic_ph2_read_3fff_aec_byte();//0xff;
			}
			vicSprite[sprite1].dataBuffer = vicSpriteData[sprite1];
			break;
		case 62:
			if (vicSpriteArmedOrActive != 0)
				DrawSprites(cyclePrev);
			DRAW_BORDER(cycle);
			COLOR_FOREGROUND(vicBackgroundColor, cycle);

			SetBA(clocks, cycle);
			vicSpritePointer[sprite2] = (bit16)vic_ph1_read_byte(vicMemptrVM + 0x3f8 + sprite2) * 64;
			if (vicSpriteDMA & (1<<sprite2))
			{
				LOADSPRITEDATA(sprite2, 2) = vic_ph2_read_byte(vicSpritePointer[sprite2] + MC[sprite2]);
				MC[sprite2] = (MC[sprite2] + 1) & 63;
			}
			else
				LOADSPRITEDATA(sprite2, 2) = vic_ph2_read_3fff_aec_byte();//0xff;
			break;
		case 63: 
			if (vicSpriteArmedOrActive != 0)
				DrawSprites(cyclePrev);

			SetBA(clocks, cycle);
			if (vicSpriteDMA & (1<<sprite2))
			{
				LOADSPRITEDATA(sprite2, 1) = vic_ph1_read_byte(vicSpritePointer[sprite2] + MC[sprite2]);
				MC[sprite2] = (MC[sprite2] + 1) & 63;
				LOADSPRITEDATA(sprite2, 0) = vic_ph2_read_byte(vicSpritePointer[sprite2] + MC[sprite2]);
			}
			else
			{
				LOADSPRITEDATA(sprite2, 1) = vic_ph1_read_3fff_byte();
				LOADSPRITEDATA(sprite2, 0) = vic_ph2_read_3fff_aec_byte();//0xff;
			}
			vicSprite[sprite2].dataBuffer = vicSpriteData[sprite2];
			if (vic_raster_line == vic_bottom_compare)
			{
				vicVerticalBorder=1;
				vicCharDataOutputDisabled=1;
			}
			else if(vic_raster_line == vic_top_compare && vicDEN!=0) 
			{
				vicVerticalBorder=0;
				vicCharDataOutputDisabled=0;
			}
			break;
		}
		if (m_bVicModeChanging)
		{
			m_bVicModeChanging = false;
			vicECM_BMM_MCM_prev = vicECM_BMM_MCM;
		}
		if (m_bVicBankChanging)
		{
			m_bVicBankChanging = false;
			SetMMU(vicBankChangeByte);
		}
		//TEST
		//vicBorderColor=7;
		//vicBackgroundColor[0] = 0;
	}
}


//pragma optimize( "g", on )

bit8 VIC6569::ReadRegister(bit16 address, ICLK sysclock)
{
bit8 t;
bit8 data;
	ExecuteCycle(sysclock);
	//WARNING The cpu optimisation can read ahead on read accesses 
	//and if it turned out that BA went low and the cpu's clock was bumped adhead 
	//then sysclock may not equal cpu->CurrentClock when we get here.
	if (address >= 0xD400)
	{
		return de00_byte;
	}
	switch (address & 0x3f)
	{
	case 0x0:	//sprite0 X
		data = (bit8)vicSpriteX[0];
		break;
	case 0x1:	//sprite0 Y
		data = (bit8)vicSpriteY[0];
		break;
	case 0x2:	//sprite1 X
		data = (bit8)vicSpriteX[1];
		break;
	case 0x3:	//sprite1 Y
		data = (bit8)vicSpriteY[1];
		break;
	case 0x4:	//sprite2 X
		data = (bit8)vicSpriteX[2];
		break;
	case 0x5:	//sprite2 Y
		data = (bit8)vicSpriteY[2];
		break;
	case 0x6:	//sprite3 X
		data = (bit8)vicSpriteX[3];
		break;
	case 0x7:	//sprite3 Y
		data = (bit8)vicSpriteY[3];
		break;
	case 0x8:	//sprite4 X
		data = (bit8)vicSpriteX[4];
		break;
	case 0x9:	//sprite4 Y
		data = (bit8)vicSpriteY[4];
		break;
	case 0xa:	//sprite5 X
		data = (bit8)vicSpriteX[5];
		break;
	case 0xb:	//sprite5 Y
		data = (bit8)vicSpriteY[5];
		break;
	case 0xc:	//sprite6 X
		data = (bit8)vicSpriteX[6];
		break;
	case 0xd:	//sprite6 Y
		data = (bit8)vicSpriteY[6];
		break;
	case 0xe:	//sprite7 X
		data = (bit8)vicSpriteX[7];
		break;
	case 0xf:	//sprite7 Y
		data = (bit8)vicSpriteY[7];
		break;
	case 0x10:	//sprite MSB X
		data = (bit8)vicSpriteMSBX;
		break;
	case 0x11:	//control 1
		data = (bit8)(vicYSCROLL | (vicRSEL<<3) | (vicDEN<<4) | ((vicECM_BMM_MCM & 6)<<4)
			| ((vic_raster_line & 0x100)>>1));
		break;
	case 0x12:	//raster
		data = (bit8)(vic_raster_line & 0xFF);
		break;
	case 0x13:	//vic_lpx
		data = vic_lpx;
		break;
	case 0x14:	//vic_lpy
		data = vic_lpy;
		break;
	case 0x15:	//sprite enable
		data = (bit8)vicSpriteEnable;
		break;
	case 0x16:	//control 2
		data = (bit8)(vicXSCROLL | (vicCSEL<<3) | ((vicECM_BMM_MCM & 1)<<4) | (vicRES) | 0xC0);
		break;
	case 0x17:	//sprite Y expand
		data = (bit8)vicSpriteYExpand;
		break;
	case 0x18:	//memory pointers
		data =  (bit8)(vicMemptrVM>>6 | vicMemptrCB>>10 | 1);
		break;
	case 0x19:	//interrupt status
		data = ((bit8)vicINTERRUPT_STATUS  & 0xF) | 0x70 | ((bit8)cpu->IRQ_VIC & 1)<<7;
		break;
	case 0x1a:	//interrupt enable
		data = (bit8)vicINTERRUPT_ENABLE | 0xF0;
		break;
	case 0x1b:	//sprite data priority
		data = (bit8)vicSpriteDataPriority;
		break;
	case 0x1c:	//sprite multicolor
		data = (bit8)vicSpriteMultiColor;
		break;
	case 0x1d:	//sprite X expand
		data = (bit8)vicSpriteXExpand;
		break;
	case 0x1e:	//sprite-sprite collision
		ClockNextWakeUpClock = sysclock;
		t = (bit8)vicSpriteSpriteCollision;
		vicSpriteSpriteInt = 1;
		SpriteCollisionUpdateArmedOrActive();
		clockReadSpriteSpriteCollision = cpu->CurrentClock + 1;
		data = t;
		break;
	case 0x1f:	//sprite-data collision
		ClockNextWakeUpClock = sysclock;
		t = (bit8)vicSpriteDataCollision;
		vicSpriteDataInt = 1;
		SpriteCollisionUpdateArmedOrActive();
		clockReadSpriteDataCollision = cpu->CurrentClock + 1;
		data = t;
		break;
	case 0x20:	//border color
		data = (bit8)vicBorderColor | 0xF0;
		break;
	case 0x21:	//background color0
		data = (bit8)vicBackgroundColor[0] | 0xF0;
		break;
	case 0x22:	//background color1
		data = (bit8)vicBackgroundColor[1] | 0xF0;
		break;
	case 0x23:	//background color2
		data = (bit8)vicBackgroundColor[2] | 0xF0;
		break;
	case 0x24:	//background color3
		data = (bit8)vicBackgroundColor[3] | 0xF0;
		break;
	case 0x25:	//sprite multicolor0
		data = (bit8)vicBackgroundColor[12] | 0xF0;
		break;
	case 0x26:	//sprite multicolor1
		data = (bit8)vicBackgroundColor[13] | 0xF0;
		break;
	case 0x27:	//color sprite0
		data = (bit8)vicBackgroundColor[4] | 0xF0;
		break;
	case 0x28:	//color sprite1
		data = (bit8)vicBackgroundColor[5] | 0xF0;
		break;
	case 0x29:	//color sprite2
		data = (bit8)vicBackgroundColor[6] | 0xF0;
		break;
	case 0x2a:	//color sprite3
		data = (bit8)vicBackgroundColor[7] | 0xF0;
		break;
	case 0x2b:	//color sprite4
		data = (bit8)vicBackgroundColor[8] | 0xF0;
		break;
	case 0x2c:	//color sprite5
		data = (bit8)vicBackgroundColor[9] | 0xF0;
		break;
	case 0x2d:	//color sprite6
		data = (bit8)vicBackgroundColor[10] | 0xF0;
		break;
	case 0x2e:	//color sprite7
		data = (bit8)vicBackgroundColor[11] | 0xF0;
		break;
	default:
		data = 255;
	}
	DmaFixUp(vic_raster_cycle, data);
	return data;
}

bit8 VIC6569::ReadRegister_no_affect(bit16 address, ICLK sysclock)
{
	ExecuteCycle(sysclock);

	switch (address & 0x3f)
	{
	case 0x1e:
		return (bit8)vicSpriteSpriteCollision;
	case 0x1f:
		return (bit8)vicSpriteDataCollision;
	default:
		return ReadRegister(address, sysclock);
	}
}

void VIC6569::SpriteXChange(bit8 spriteNo, bit16 x_new, bit8 cycle)
{
	vicSprite[spriteNo].SetXPos(x_new, cycle);
}

void VIC6569::CheckRasterCompare(bit8 cycle)
{
	//Edge triggered raster IRQ noticed in Octopus In Red Wine demo.
	if (vic_check_irq_in_cycle2 == 0)
	{
		if (cycle != 63)
		{
			if (vic_raster_line == vicRASTER_compare)
			{
				vicINTERRUPT_STATUS |= 0x1; // Raster Interrupt Flag
				if ((vicINTERRUPT_STATUS & vicINTERRUPT_ENABLE & 0x1)!=0)
				{
					SetSystemInterrupt();
				}
				bVicRasterMatch = true;
			}
			else 
				bVicRasterMatch = false;
		}
	}
	else
	{
		if (cycle != 1)
		{
			if (vic_raster_line == PAL_MAX_LINE)
			{
				vicINTERRUPT_STATUS |= 0x1; // Raster Interrupt Flag
				bVicRasterMatch = true;
				if ((vicINTERRUPT_STATUS & vicINTERRUPT_ENABLE & 0x1)!=0)
				{
					SetSystemInterrupt();
				}
			}
			else 
				bVicRasterMatch = false;
		}
	}
}

void VIC6569::DmaFixUp(bit8 cycle, bit8 data)
{

	switch (cycle)
	{
	case 1:		
		LOADSPRITEDATA(sprite3, 2) = data;
		break;
	case 2:
		LOADSPRITEDATA(sprite3, 0) = data;
		vicSprite[sprite3].dataBuffer = vicSpriteData[sprite3];
		break;
	case 3:
		LOADSPRITEDATA(sprite4, 2) = data;
		break;
	case 4:
		LOADSPRITEDATA(sprite4, 0) = data;
		vicSprite[sprite4].dataBuffer = vicSpriteData[sprite4];
		break;
	case 5:	
		LOADSPRITEDATA(sprite5, 2) = data;
		break;
	case 6:
		LOADSPRITEDATA(sprite5, 0) = data;
		vicSprite[sprite5].dataBuffer = vicSpriteData[sprite5];
		break;
	case 7:
		LOADSPRITEDATA(sprite6, 2) = data;
		break;
	case 8:
		LOADSPRITEDATA(sprite6, 0) = data;
		vicSprite[sprite6].dataBuffer = vicSpriteData[sprite6];
		break;
	case 9:
		LOADSPRITEDATA(sprite7, 2) = data;
		break;
	case 10:
		LOADSPRITEDATA(sprite7, 0) = data;
		vicSprite[sprite7].dataBuffer = vicSpriteData[sprite7];
		break;
	case 58:
		LOADSPRITEDATA(sprite0, 2) = data;
		break;
	case 59:
		LOADSPRITEDATA(sprite0, 0) = data;
		vicSprite[sprite0].dataBuffer = vicSpriteData[sprite0];
		break;
	case 60:
		LOADSPRITEDATA(sprite1, 2) = data;
		break;
	case 61:
		LOADSPRITEDATA(sprite1, 0) = data;
		vicSprite[sprite1].dataBuffer = vicSpriteData[sprite1];
		break;
	case 62:
		LOADSPRITEDATA(sprite2, 2) = data;
		break;
	case 63:
		LOADSPRITEDATA(sprite2, 0) = data;
		vicSprite[sprite2].dataBuffer = vicSpriteData[sprite2];
		break;
	}
}

void VIC6569::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
bit16 old_raster_compare;
bit8 i,ibit8;
bit8 cycle;
bit8 oldVicRSEL;
bit8 oldVicDEN;
bit8 oldVicECM_BMM_MCM;
bit8 oldXScroll;
bit8 newXScroll;
bool bRSELChanging;
bit8 modeNew;
bit8 modeOld;

	ExecuteCycle(sysclock);
	cycle = vic_raster_cycle;
	DmaFixUp(cycle, data);

	switch (address & 0x3f)
	{
	case 0x0:	//sprite0 X
		SpriteXChange(0, data + (((bit16)(vicSpriteMSBX & 1)) << 8), cycle);
		vicSpriteX[0] = data;
		break;
	case 0x1:	//sprite0 Y
		vicSpriteY[0] = data;
		break;
	case 0x2:	//sprite1 X
		SpriteXChange(1, data + (((bit16)(vicSpriteMSBX & 2)) << 7), cycle);
		vicSpriteX[1] = data;
		break;
	case 0x3:	//sprite1 Y
		vicSpriteY[1] = data;
		break;
	case 0x4:	//sprite2 X
		SpriteXChange(2, data + (((bit16)(vicSpriteMSBX & 4)) << 6), cycle);
		vicSpriteX[2] = data;
		break;
	case 0x5:	//sprite2 Y
		vicSpriteY[2] = data;
		break;
	case 0x6:	//sprite3 X
		SpriteXChange(3, data + (((bit16)(vicSpriteMSBX & 8)) << 5), cycle);
		vicSpriteX[3] = data;
		break;
	case 0x7:	//sprite3 Y
		vicSpriteY[3] = data;
		break;
	case 0x8:	//sprite4 X
		SpriteXChange(4, data + (((bit16)(vicSpriteMSBX & 16)) << 4), cycle);
		vicSpriteX[4] = data;
		break;
	case 0x9:	//sprite4 Y
		vicSpriteY[4] = data;
		break;
	case 0xa:	//sprite5 X
		SpriteXChange(5, data + (((bit16)(vicSpriteMSBX & 32)) << 3), cycle);
		vicSpriteX[5] = data;
		break;
	case 0xb:	//sprite5 Y
		vicSpriteY[5] = data;
		break;
	case 0xc:	//sprite6 X
		SpriteXChange(6, data + (((bit16)(vicSpriteMSBX & 64)) << 2), cycle);
		vicSpriteX[6] = data;
		break;
	case 0xd:	//sprite6 Y
		vicSpriteY[6] = data;
		break;
	case 0xe:	//sprite7 X
		SpriteXChange(7, data + (((bit16)(vicSpriteMSBX & 128)) << 1), cycle);
		vicSpriteX[7] = data;
		break;
	case 0xf:	//sprite7 Y
		vicSpriteY[7] = data;
		break;
	case 0x10:	//sprite MSB X
		if ((vicSpriteMSBX ^ data) & 1)
			SpriteXChange(0, vicSpriteX[0] + (((bit16)(data & 1)) << 8), cycle);
		if ((vicSpriteMSBX ^ data) & 2)
			SpriteXChange(1, vicSpriteX[1] + (((bit16)(data & 2)) << 7), cycle);
		if ((vicSpriteMSBX ^ data) & 4)
			SpriteXChange(2, vicSpriteX[2] + (((bit16)(data & 4)) << 6), cycle);
		if ((vicSpriteMSBX ^ data) & 8)
			SpriteXChange(3, vicSpriteX[3] + (((bit16)(data & 8)) << 5), cycle);
		if ((vicSpriteMSBX ^ data) & 16)
			SpriteXChange(4, vicSpriteX[4] + (((bit16)(data & 16)) << 4), cycle);
		if ((vicSpriteMSBX ^ data) & 32)
			SpriteXChange(5, vicSpriteX[5] + (((bit16)(data & 32)) << 3), cycle);
		if ((vicSpriteMSBX ^ data) & 64)
			SpriteXChange(6, vicSpriteX[6] + (((bit16)(data & 64)) << 2), cycle);
		if ((vicSpriteMSBX ^ data) & 128)
			SpriteXChange(7, vicSpriteX[7] + (((bit16)(data & 128)) << 1), cycle);
		vicSpriteMSBX = data;
		break;
	case 0x11:	//control 1
		ClockNextWakeUpClock = sysclock;
		modeOld = vicECM_BMM_MCM;
		m_bVicModeChanging = true;
		oldXScroll = vicXSCROLL;
		oldVicRSEL = (bit8)vicRSEL;
		oldVicDEN = vicDEN;
		oldVicECM_BMM_MCM = vicECM_BMM_MCM;
		bRSELChanging = (((data & 8)>>3) ^ vicRSEL) != 0;
		if (vic_raster_line == vic_top_compare)
		{
			//I demo that I cannot remember required this to display properly.
			if (bRSELChanging && vicDEN!=0)
			{
				vicVerticalBorder=0;
				vicCharDataOutputDisabled=0;
			}
		}
		if (vic_raster_line == vic_bottom_compare)
		{//memories demo			
			if (bRSELChanging)
			{
				vicVerticalBorder=1;
			}
		}
		vicYSCROLL = data & 7;
		vicRSEL = (data & 8)>>3;
		vicDEN = (data & 0x10)>>4;
		vicECM = (data & 0x40)>>6;
		vicBMM = (data & 0x20)>>5;
		vicECM_BMM_MCM = (vicECM<<2) | (vicBMM<<1) | (modeOld & 1);
		modeNew = vicECM_BMM_MCM;

		old_raster_compare = vicRASTER_compare;
		vicRASTER_compare = (vicRASTER_compare & 0x0FF) | (((bit16)data & 0x80)<<1);

		if (vicRSEL == 0)
		{
  			vic_top_compare = 0x37;
			vic_bottom_compare = 0xf7;
		}
		else
		{
  			vic_top_compare = 0x33;
			vic_bottom_compare = 0xfb;
		}

		if (vic_raster_line == vic_top_compare && cycle!=63)
		{
			//The game Elven Warrior used to require this to display properly?!
			if (bRSELChanging && vicDEN!=0)
			{
				vicVerticalBorder=0;
				vicCharDataOutputDisabled=0;
			}
		}

		if (vic_raster_line == vic_bottom_compare && cycle!=63)
		{//memories demo
			if (bRSELChanging)
			{
				vicVerticalBorder=1;
			}
		}

		if (vicRASTER_compare != old_raster_compare)
		{
			CheckRasterCompare(cycle);
		}		

		if (vic_raster_line==0x30)
			vic_latchDEN|=vicDEN;
		if (vic_raster_line>=0x30 && vic_raster_line<=0xf7 && ((vic_raster_line & 7)==vicYSCROLL) && vic_latchDEN!=0 && cycle !=63)
		{			
			if (vicIDLE != 0)
			{
				vicIDLE_DELAY=1;
				if (vic_badline == 0 && cycle>=15 && cycle < 54)
				{
					clockFirstForcedBadlineCData = sysclock + 1;
				}
			}

			vic_badline=1;
			if (cycle>=15 && cycle<=54)
			{
				vic_allow_c_access = 1;
			}
			vicCDataCarry=0;

			cpu_next_op_code = cpu->MonReadByte(cpu->mPC.word, -1);
			
			vicIDLE=0;
			vic_address_line_info = &BA_line_info[vicSpriteDMA][1];
		}
		else
		{
			vic_allow_c_access = vic_badline =0;
			vic_address_line_info = &BA_line_info[vicSpriteDMA][0];
		}

		if (modeOld != modeNew && (cycle >=9 && cycle <=60))
		{
			DF_PixelsToSkip = 0;
			bit8 gData2 = vicLastGData;
			bit8 gData1 = vicLastGDataPrev;
			bit8 gData0 = vicLastGDataPrev2;
			bit32 cData0 = vicLastCDataPrev2;
			bit32 cData1 = vicLastCDataPrev;
			bit32 cData2 = vicLastCData;
			bit8 t;

			bool bBMM_old= (modeOld & 2)!=0;
			bool bBMM_new= (modeNew & 2)!=0;
			bool bECM_old= (modeOld & 4)!=0;
			bool bECM_new= (modeNew & 4)!=0;
			bool bMCM = modeOld & 1;

			bit8 bForceSingleWidthPixels_char1 = 0;
			bit8 bForceSingleWidthPixels_char2 = 0;
			bool bWidePixels_char1 = false;
			bool bWidePixels_char2 = false;
			bool bMCColourRoute_char1 = false;
			bool bMCColourRoute_char2 = false;
			bit8 gData1Transformed = gData1;
			bit8 gData2Transformed = gData2;


			/*************/
			//BMM: 0->1 
			//Mode change starts at pixel postion 9
			//shows early c-data fetched for the pixel width flag.

			//for remaning portion of char 1 that is x-scrolled over or past pixel position 9
			//the pixel width flag comes from MCM
			//the MC color routing flag comes from c-data1 & MCM

			//for char 2
			//if x-scroll<4 then 
			//the pixel width flag comes from MCM
			//the MC colour routing flag comes from c-data1 & MCM

			//for char 2
			//if x-scroll>=4 then 
			//the pixel width flag comes from MCM
			//the MC colour routing flag comes from MCM

			/*************/
			//BMM: 1->0
			//Mode change is delayed by 1 pixel.
			//Mode change starts at pixel position 10

			//for remaning portion of char 1 that is x-scrolled over or past pixel position 10
			//the pixel width flag comes from MCM
			//the MC color routing flag from c-data1 & MCM

			//for char 2
			//if x-scroll <= 4 then 
			//the pixel width flag comes from MCM
			//the MC color routing flag from c-data2 & MCM

			//for char 2
			//if x-scroll > 4 then 
			//the pixel width flag comes c-data2 & MCM
			//the MC color routing flag from c-data2 & MCM

			if (bBMM_old && !bBMM_new)
			{
				//BMM: 1->0				
				bWidePixels_char1    = bMCM;				
				bMCColourRoute_char1 = bMCM && (cData1 & 0x800)!=0;
				if (oldXScroll > 4)
				{
					bWidePixels_char2 =    bMCM && (cData2 & 0x800)!=0;
					bMCColourRoute_char2 = bMCM && (cData2 & 0x800)!=0;
				}
				else
				{
					bWidePixels_char2    = bMCM;
					bMCColourRoute_char2 = bMCM && (cData2 & 0x800)!=0;
				}
			}
			else if (!bBMM_old && bBMM_new)
			{
				//BMM: 0->1
				bWidePixels_char1    = bMCM && (cData1 & 0x800)!=0;
				bMCColourRoute_char1 = bMCM;
				if (oldXScroll >= 4)
				{
					bWidePixels_char2    = bMCM;
					bMCColourRoute_char2 = bMCM;
				}
				else
				{
					bWidePixels_char2    = bMCM && (cData2 & 0x800)!=0;
					bMCColourRoute_char2 = bMCM;
				}
			}
			else if (bBMM_old)
			{
				//BMM is staying on.
				bWidePixels_char1 = bMCM;
				bMCColourRoute_char1 = bMCM;
				bWidePixels_char2 = bMCM;
				bMCColourRoute_char2 = bMCM;
			}
			else
			{
				//BMM is staying off.
				bWidePixels_char1 = bMCM && (cData1 & 0x800)!=0;
				bMCColourRoute_char1 = bWidePixels_char1;
				bWidePixels_char2 = bMCM && (cData2 & 0x800)!=0;;
				bMCColourRoute_char2 = bWidePixels_char2;
			}

			if (bWidePixels_char1)
			{
				bForceSingleWidthPixels_char1 = 0;
				if (!bMCColourRoute_char1)
				{
					//Wide pixels standard colour route.
					/*
					00 -> 00
					01 -> 00
					10 -> 11
					11 -> 11
					*/
					t = gData1 & 0xaa;
					gData1Transformed = t | (t >> 1);
				}
			}
			else
			{
				bForceSingleWidthPixels_char1 = 1; 
			}
			if (bWidePixels_char2)
			{
				bForceSingleWidthPixels_char2 = 0;
				if (!bMCColourRoute_char2)
				{
					//Wide pixels standard colour route.
					/*
					00 -> 00
					01 -> 00
					10 -> 11
					11 -> 11
					*/
					t = gData2 & 0xaa;
					gData2Transformed = t | (t >> 1);
				}
			}
			else
			{
				bForceSingleWidthPixels_char2 = 1; 
			}

			//BMM going on affects char2
			switch (((modeOld << 1) & 0xc) | ((modeNew >> 1) & 3))
			{
			case 0:
				//ECM:0->0
				//BMM:0->0
				//no change
				//break;
			case 5:
				//ECM:0->0
				//BMM:1->1
				//no change
				//break;
			case 10:
				//ECM:1->1
				//BMM:0->0
				//no change
				//break;
			case 15:
				//ECM:1->1
				//BMM:1->1
				//no change
				break;
			case 1:
				//ECM:0->0
				//BMM:0->1
				//break;
			case 3:
				//ECM:0->1
				//BMM:0->1
				//break;
			case 11:
				//ECM:1->1
				//BMM:0->1
				//break;
				DrawForegroundEx(gData1Transformed, 0, cData1, modeNew, bForceSingleWidthPixels_char1, vicCharDataOutputDisabled, 8 - oldXScroll, oldXScroll, cycle + 1);
				DrawForegroundEx(gData2Transformed, oldXScroll, cData2, modeNew, bForceSingleWidthPixels_char2, vicCharDataOutputDisabled, 0, 8, cycle + 1);
				DF_PixelsToSkip = 8;
				break;
			case 2:
				//ECM:0->1
				//BMM:0->0
				//break;
			case 7:
				//ECM:0->1
				//BMM:1->1
				//break;
				DrawForegroundEx(gData1Transformed, 0, cData1, modeNew, 0, vicCharDataOutputDisabled, 8 - oldXScroll, oldXScroll, cycle + 1);
				DF_PixelsToSkip = 0;
				break;
			case 4:
				//ECM:0->0
				//BMM:1->0
				//break;
			case 6:
				//ECM:0->1
				//BMM:1->0
				//break;
			case 14:
				//ECM:1->1
				//BMM:1->0
				//break;

				//BMM going off.
				//Mode change is delayed by 1 pixel.				
				if (oldXScroll == 0)
				{
					//Draw BMM as though still on for one more pixel.
					DrawForegroundEx(gData2, 0, cData2, modeNew | 2, 0, vicCharDataOutputDisabled, 0, 1, cycle + 1);
					DrawForegroundEx(gData2Transformed, 1, cData2, modeNew, bForceSingleWidthPixels_char2, vicCharDataOutputDisabled, +1, 7, cycle + 1);
					DF_PixelsToSkip = 8;
				}
				else
				{
					//Draw BMM as though still on for one more pixel.
					DrawForegroundEx(gData1, 0, cData1, modeNew | 2, 0, vicCharDataOutputDisabled, 8 - oldXScroll, 1, cycle + 1);					
					//Draw BMM as though off for remaining pixels.
					DrawForegroundEx(gData1Transformed, 1, cData1, modeNew, 0, vicCharDataOutputDisabled, 8 - oldXScroll + 1, oldXScroll - 1, cycle + 1);
					DrawForegroundEx(gData2Transformed, oldXScroll, cData2, modeNew, bForceSingleWidthPixels_char2, vicCharDataOutputDisabled, 0, 8, cycle + 1);
					DF_PixelsToSkip = 8;
				}
				break;
			case 8:
				//ECM:1->0
				//BMM:0->0
				//break;
			case 13:
				//ECM:1->0
				//BMM:1->1
				//break;
				//ECM going off.
				//Mode change is delayed by 1 pixel.				
				if (oldXScroll == 0)
				{
					//Draw ECM as though still on for one more pixel.
					DrawForegroundEx(gData2, 0, cData2, modeNew | 4, 0, vicCharDataOutputDisabled, 0, 1, cycle + 1);
					DF_PixelsToSkip = 1;
				}
				else
				{
					//Draw ECM as though still on for one more pixel.
					DrawForegroundEx(gData1, 0, cData1, modeNew | 4, 0, vicCharDataOutputDisabled, 8 - oldXScroll, 1, cycle + 1);					
					//Draw ECM as though off for remaining pixels.
					DrawForegroundEx(gData1, 1, cData1, modeNew, 0, vicCharDataOutputDisabled, 8 - oldXScroll + 1, oldXScroll - 1, cycle + 1);
					DF_PixelsToSkip = 0;
				}
				break;
			case 9:
				//ECM:1->0
				//BMM:0->1
				//ECM going off.
				//Mode change is delayed by 1 pixel.				
				if (oldXScroll == 0)
				{
					//Draw ECM as though still on for one more pixel.
					DrawForegroundEx(gData2Transformed, 0, cData2, modeNew | 4, 0, vicCharDataOutputDisabled, 0, 1, cycle + 1);
					DrawForegroundEx(gData2Transformed, 1, cData2, modeNew, bForceSingleWidthPixels_char2, vicCharDataOutputDisabled, +1, 7, cycle + 1);
					DF_PixelsToSkip = 8;
				}
				else
				{
					//Draw ECM as though still on for one more pixel.
					DrawForegroundEx(gData1Transformed, 0, cData1, modeNew | 4, 0, vicCharDataOutputDisabled, 8 - oldXScroll, 1, cycle + 1);					
					//Draw ECM as though off for remaining pixels.
					DrawForegroundEx(gData1Transformed, 1, cData1, modeNew, 0, vicCharDataOutputDisabled, 8 - oldXScroll + 1, oldXScroll - 1, cycle + 1);
					DrawForegroundEx(gData2Transformed, oldXScroll, cData2, modeNew, bForceSingleWidthPixels_char2, vicCharDataOutputDisabled, 0, 8, cycle + 1);
					DF_PixelsToSkip = 8;
				}
				break;
			case 12:
				//ECM:1->0
				//BMM:1->0
				//ECM going off.
				//BMM going off.
				//Mode change is delayed by 1 pixel.				
				if (oldXScroll == 0)
				{
					//Draw ECM+BMM as though still on for one more pixel.
					DrawForegroundEx(gData2, 0, cData2, modeNew | 6, 0, vicCharDataOutputDisabled, 0, 1, cycle + 1);
					DrawForegroundEx(gData2Transformed, 1, cData2, modeNew, bForceSingleWidthPixels_char2, vicCharDataOutputDisabled, +1, 7, cycle + 1);
					DF_PixelsToSkip = 8;
				}
				else
				{
					//Draw ECM+BMM as though still on for one more pixel.
					DrawForegroundEx(gData1Transformed, 0, cData1, modeNew | 6, 0, vicCharDataOutputDisabled, 8 - oldXScroll, 1, cycle + 1);					
					//Draw ECM+BMM as though off for remaining pixels.
					DrawForegroundEx(gData1Transformed, 1, cData1, modeNew, 0, vicCharDataOutputDisabled, 8 - oldXScroll + 1, oldXScroll - 1, cycle + 1);
					DrawForegroundEx(gData2Transformed, oldXScroll, cData2, modeNew, bForceSingleWidthPixels_char2, vicCharDataOutputDisabled, 0, 8, cycle + 1);
					DF_PixelsToSkip = 8;
				}
				break;
			}
						
			/*
			//The the mode change starts at either pixel position 8 or 9
			//Starting sooner causes the the stray bars in Diskshake to be too long.
			*/
		}
		break;
	case 0x12:	//raster
		old_raster_compare = vicRASTER_compare;
		vicRASTER_compare = (vicRASTER_compare & 0x100) | (data);
		
		if (vicRASTER_compare != old_raster_compare)
		{
			//nextWakeUpClock = sysclock;
			CheckRasterCompare(cycle);
		}		
		break;
	case 0x13:	//vic_lpx
		break;
	case 0x14:	//vic_lpy
		break;
	case 0x15:	//sprite enable
		vicSpriteEnable = data;
		if (data != 0)
			ClockNextWakeUpClock = sysclock;
		break;
	case 0x16:	//control 2
		ClockNextWakeUpClock = sysclock;
		modeOld = vicECM_BMM_MCM;
		m_bVicModeChanging = true;
		oldXScroll = vicXSCROLL;
		vicCSEL = (data & 8)>>3;
		vicMCM = (data & 0x10)>>4;
		vicECM_BMM_MCM = (vicECM<<2) | (vicBMM<<1) | (vicMCM);
		vicRES = data & 0x20;
		vicXSCROLL = newXScroll = (data & 7);
		modeNew = vicECM_BMM_MCM;

		//Hacky 9 pixel in the border accommodation; FIXME
		if (cycle == 56)
		{
			//Any 9 pixel chars in the last column always display the 9th pixel under the border even when x-scroll does not increase.
			//So fake an x-scroll going to 7 to ensure that any 9 pixel chars in the last column get displayed.
			newXScroll = 7;
		}
		//Hacky 9 pixel in the border accommodation; FIXME
		vicXSCROLL_Cycle57 = newXScroll;

		if (cycle >=9 && cycle <=60)
		{
			bit32 cData0 = vicLastCDataPrev2;
			bit32 cData1 = vicLastCDataPrev;
			bit32 cData2 = vicLastCData;
			if (newXScroll > oldXScroll)
			{
				//Draw the background pixels for each increase in x-scroll.
				DrawForegroundEx(0, oldXScroll, cData1, modeNew, 0, 0, 0, newXScroll - oldXScroll, cycle + 1);
			}

			//Faulty failed attempt to fix the 9 pixels in the border hack.
			//if (cycle == 56)
			//{
			//	//Any 9 pixel chars in the last column always display the 9th pixel under the border even when x-scroll does not increase.
			//	//So fake an increasing x-scroll to ensure that any 9 pixel chars in the last column get displayed.
			//	DrawForegroundEx(0, oldXScroll, cData1, modeNew, 0, 0, 0, 8 - oldXScroll, cycle + 1);
			//}

			if (modeNew != modeOld)
			{
				bit8 t;

				bit8 gData2 = vicLastGData;
				bit8 gData1 = vicLastGDataPrev;
				bit8 gData0 = vicLastGDataPrev2;

				bit8 gData1Transformed = gData1;
				bit8 gData0Transformed = gData0;

				bool bBMM = (modeOld & 2) != 0;
				bool bECM = (modeOld & 4) != 0;

				signed char xScrollDecrement = 0;
				
				if (oldXScroll > newXScroll)
					xScrollDecrement = oldXScroll - newXScroll;

				//We may have already drawn part of the next cycle with an old and incorrect modeOld so we make a correction
				//Need to review if this DrawForegroundEx call is still needed.
				DrawForegroundEx(gData1, 0, cData1, modeNew, 0, vicCharDataOutputDisabled, 8 - oldXScroll, oldXScroll, cycle + 1);

				if ((modeOld & 1) !=0)//Multi-color going off
				{
					bit32 cData1b = cData1;
					//Need to check what happens with bitmap / ecm modes with decreasing x-scroll.
					if (bBMM)
					{
						//BMM==1
						if (xScrollDecrement >= 4)
						{
							//Early c-data colour fetch
							cData1b = cData2 & 0xfff;
						}
					}
					else
					{
						//BMM==0
						if (xScrollDecrement >= 4)
						{
							//Early c-data colour fetch
							cData1b = (cData1 & 0x0ff) | (cData2 & 0xf00);
						}
						if (bECM)
						{
							//ECM==1
							if (xScrollDecrement > 4)
							{
								//Early c-data colour fetch
								cData1b = (cData1b & 0xf3f) | (cData2 & 0x0c0);
							}
						}
					}

					if ((cData1 & 0x800) != 0 || bBMM)
					{
						//pixel positions 5 and 6
						//Demo "Tense Years" by Onslaught Design
						//Prestige - part DYXCP!
						//Wide pixels standard colour route.
						/*
						00 -> 00
						01 -> 00
						10 -> 11
						11 -> 11
						*/
						t = gData1 & 0xaa;
						gData1Transformed = t | (t >> 1);
					}
					if ((cData0 & 0x800) != 0 || bBMM)
					{
						//pixel positions 5 and 6
						//Demo "Tense Years" by Onslaught Design
						//Prestige - part DYXCP!
						//Wide pixels standard colour route.
						/*
						00 -> 00
						01 -> 00
						10 -> 11
						11 -> 11
						*/
						t = gData0 & 0xaa;
						gData0Transformed = t | (t >> 1);
					}
					
					//Required for Mariusz's "pattern zoom" test and the removal of stray pixels in some demos.
					//"Tense Years" by Onslaught Design
					switch (oldXScroll)
					{
					case 0:
						//Chromance
						//Pattern zoom 04-c64c.png
						DrawForegroundEx(gData1Transformed, +4, cData1, modeNew, 0, vicCharDataOutputDisabled, 4 - oldXScroll, 3, cycle - 0);	
						DrawForegroundEx(gData1, +4 + 3, cData1, modeNew, 0, vicCharDataOutputDisabled, 4 - oldXScroll + 3, 1, cycle - 0);
						break;
					case 1:
						//Pattern zoom 03-c64c.png
						//Prestige - part DYXCP!
						DrawForegroundEx(gData1Transformed, +4, cData1, modeNew, 0, vicCharDataOutputDisabled, 4 - oldXScroll, 3, cycle - 0);
						DrawForegroundEx(gData1, +4 + 3, cData1, modeNew, 0, vicCharDataOutputDisabled, 4 - oldXScroll + 3, 2, cycle - 0);
						break;
					case 2:
						DrawForegroundEx(gData1Transformed, +4, cData1, modeNew, 0, vicCharDataOutputDisabled, 4 - oldXScroll, 3, cycle - 0);	
						DrawForegroundEx(gData1, +4 + 3, cData1, modeNew, 0, vicCharDataOutputDisabled, 4 - oldXScroll + 3, 3, cycle - 0);
						break;
					case 3:
						//Pattern zoom "DEMO"
						DrawForegroundEx(gData1Transformed, +4, cData1, modeNew, 0, vicCharDataOutputDisabled, 4 - oldXScroll, 3, cycle - 0);
						DrawForegroundEx(gData1, +4 + 3, cData1, modeNew, 0, vicCharDataOutputDisabled, 4 - oldXScroll + 3, 4, cycle - 0);
						break;
					case 4:
						DrawForegroundEx(gData1Transformed, +4, cData1b, modeNew, 0, vicCharDataOutputDisabled, 4 - oldXScroll, 3, cycle - 0);
						DrawForegroundEx(gData1, +4 + 3, cData1b, modeNew, 0, vicCharDataOutputDisabled, 4 - oldXScroll + 3, 5, cycle - 0);
						break;
					case 5:
						//Pattern zoom 02-c64c.png
						DrawForegroundEx(gData0Transformed, +4, cData0, modeNew, 0, vicCharDataOutputDisabled, 12 - oldXScroll, 1, cycle - 0);
						DrawForegroundEx(gData1Transformed, +4 + (+1), cData1b, modeNew, 0, vicCharDataOutputDisabled, 0, 2, cycle - 0);
						DrawForegroundEx(gData1, +4 + (+3), cData1b, modeNew, 0, vicCharDataOutputDisabled, 2, 6, cycle - 0);
						break;
					case 6:
						//Pattern zoom "DE"
						DrawForegroundEx(gData0Transformed, +4, cData0, modeNew, 0, vicCharDataOutputDisabled, 12 - oldXScroll, 2, cycle - 0);
						DrawForegroundEx(gData1Transformed, +4 + (+2), cData1b, modeNew, 0, vicCharDataOutputDisabled, 0, 1, cycle - 0);
						DrawForegroundEx(gData1, +4 + (+3), cData1b, modeNew, 0, vicCharDataOutputDisabled, 1, 7, cycle - 0);
						break;
					case 7:
						//Pattern zoom 01-c64c.png
						DrawForegroundEx(gData0Transformed, +4, cData0, modeNew, 0, vicCharDataOutputDisabled, 12 - oldXScroll, 3, cycle - 0);
						DrawForegroundEx(gData1, +4 + (+3), cData1b, modeNew, 0, vicCharDataOutputDisabled, 0, 8, cycle - 0);
						break;
					}
				}
				else //Multi-color going on
				{
					bit8 gData1b;
					bit8 gData1c;
					bit32 cData1b;
					bit8 decShiftPixelShiftMode;

					decShiftPixelShiftMode = 0;
	
					if (oldXScroll != 7)
					{
						if (oldXScroll & 1)
						{
							//Multicolor misalignment with odd x-scrolls.
							gData1b = gData1 << 1;
							gData1c = gData1 << 1;
						}
						else
						{
							gData1b = gData1;
							gData1c = gData1;
						}
						if ((cData1 & 0x800) != 0 || bBMM)
						{
							//pixel position 7,8 {11->10, 01->00} {MC on}
							gData1b = gData1b & 0xAA;
						}
					}
					else
					{
						//x-scroll 7 is specially handled in case 7:
						gData1b = gData1;
						gData1c = gData1;
					}
					
					
					if (bBMM)
					{
						//BMM==1
						if (xScrollDecrement >= 4)
						{
							//Early c-data colour fetch
							cData1b = cData2 & 0xfff;
						}
						else
							cData1b = cData1;
					}
					else
					{
						//BMM==0
						if (xScrollDecrement >= 4)
						{
							//Early c-data colour fetch
							if (xScrollDecrement == 4)
							{
								//Early c-data colour fetch; 
								//cData1-->Pixel Width
								//cData1-->MC Route
								//cData2-->Colours
								cData1b = (cData1 & 0x8ff) | (cData2 & 0x700);
							}
							else
							{
								//Early c-data colour fetch;
								//cData1-->Pixel Width
								//cData2-->MC Route
								//cData2-->Colours
								if ((cData1 & 0x800)==0)
								{
									//Single pixels
									decShiftPixelShiftMode = 1;
								}
								else
								{
									//Wide pixels
									if ((cData2 & 0x800)==0)
									{
										//First pixel is copied to the second pixel.
										//Wide pixels standard colour route.
										/*
										00 -> 00
										01 -> 00
										10 -> 11
										11 -> 11
										*/
										t = gData1b & 0xaa;
										gData1b = t | (t >> 1);

										t = gData1c & 0xaa;
										gData1c = t | (t >> 1);
									}
								}
								//Early c-data colour fetch;
								//cData2-->MC Route
								//cData2-->Colours
								cData1b = (cData1 & 0x0ff) | (cData2 & 0xf00);
							}
						}
						else
							cData1b = cData1;
					}
											
					//Orreys demo
					//Onslaught Bobby Bounceback+4 Burkah : Burkah uses Bitmap -> Multicolor Bitmap
					switch (oldXScroll)
					{
					case 0:
						//Comalight 87 raster line 0x87
						//Chromance scroller
						DrawForegroundEx(gData1, +4, cData1, modeNew, 1, vicCharDataOutputDisabled, 4, 2, cycle - 0);
						DrawForegroundEx(gData1b, +4 + 2, cData1, modeNew, 0, vicCharDataOutputDisabled, 6, 2, cycle - 0);
						break;
					case 1:
						//Chromance scroller x-scroll=1
						DrawForegroundEx(gData1, +4, cData1, modeNew, 1, vicCharDataOutputDisabled, 3, 2, cycle - 0);
						DrawForegroundEx(gData1b, +4 + 2, cData1, modeNew, 0, vicCharDataOutputDisabled, 4, 2, cycle - 0);
						DrawForegroundEx(gData1c, +4 + 4, cData1, modeNew, 0, vicCharDataOutputDisabled, 6, 2, cycle - 0);
						break;
					case 2:
						//Chromance scroller x-scroll=2
						DrawForegroundEx(gData1, +4, cData1, modeNew, 1, vicCharDataOutputDisabled, 2, 2, cycle - 0);
						DrawForegroundEx(gData1b, +4 + 2, cData1, modeNew, 0, vicCharDataOutputDisabled, 4, 2, cycle - 0);
						DrawForegroundEx(gData1c, +4 + 4, cData1, modeNew, 0, vicCharDataOutputDisabled, 6, 2, cycle - 0);
						break;
					case 3:
						//Chromance scroller x-scroll=3
						DrawForegroundEx(gData1, +4, cData1, modeNew, 1, vicCharDataOutputDisabled, 1, 2, cycle - 0);
						DrawForegroundEx(gData1b, +4 + 2, cData1, modeNew, 0, vicCharDataOutputDisabled, 2, 2, cycle - 0);
						DrawForegroundEx(gData1c, +4 + 4, cData1, modeNew, 0, vicCharDataOutputDisabled, 4, 4, cycle - 0);
						break;
					case 4:
						//Chromance scroller x-scroll=4
						DrawForegroundEx(gData1, +4, cData1b, modeNew, 1, vicCharDataOutputDisabled, 0, 2, cycle - 0);
						DrawForegroundEx(gData1b, +4 + 2, cData1b, modeNew, decShiftPixelShiftMode, vicCharDataOutputDisabled, 2, 2, cycle - 0);
						DrawForegroundEx(gData1c, +4 + 4, cData1b, modeNew, decShiftPixelShiftMode, vicCharDataOutputDisabled, 4, 4, cycle - 0);
						break;
					case 5:
						//Chromance scroller x-scroll=5
						DrawForegroundEx(gData0, +4, cData0, modeNew, 1, vicCharDataOutputDisabled, 7, 1, cycle - 0);
						DrawForegroundEx(gData1, +4 +1, cData1b, modeNew, 1, vicCharDataOutputDisabled, 0, 1, cycle - 0);
						DrawForegroundEx(gData1b, +4 + 2, cData1b, modeNew, decShiftPixelShiftMode, vicCharDataOutputDisabled, 0, 2, cycle - 0);
						DrawForegroundEx(gData1c, +4 + 4, cData1b, modeNew, decShiftPixelShiftMode, vicCharDataOutputDisabled, 2, 6, cycle - 0);
						break;
					case 6:
						//Chromance scroller x-scroll=6
						DrawForegroundEx(gData0, +4, cData0, modeNew, 1, vicCharDataOutputDisabled, 6, 2, cycle - 0);
						DrawForegroundEx(gData1b, +4 + 2, cData1b, modeNew, decShiftPixelShiftMode, vicCharDataOutputDisabled, 0, 2, cycle - 0);
						DrawForegroundEx(gData1c, +4 + 4, cData1b, modeNew, decShiftPixelShiftMode, vicCharDataOutputDisabled, 2, 6, cycle - 0);
						break;
					case 7:
						//Chromance scroller x-scroll=7
						DrawForegroundEx(gData0, +4, cData0, modeNew, 1, vicCharDataOutputDisabled, 5, 2, cycle - 0);
						DrawForegroundEx(((gData0 & 0x55) << 1), +4 +2, cData0, modeNew, decShiftPixelShiftMode, vicCharDataOutputDisabled, 6, 1, cycle - 0);
						DrawForegroundEx(gData1c, +4 +3, cData1b, modeNew, decShiftPixelShiftMode, vicCharDataOutputDisabled, 0, 8, cycle - 0);
						break;
					}
				}
			}
			//Faulty failed attempt to fix the 9 pixels in the border hack.
			//if (cycle == 56)
			//{
			//	//Any 9 pixel chars in the last column always display the 9th pixel under the border even when x-scroll does not increase.
			//	//So fake an increasing x-scroll to ensure that any 9 pixel chars in the last column get displayed.
			//	DF_PixelsToSkip = 8;
			//}
		}
		break;
	case 0x17:	//sprite Y expand
		for (ibit8=1,i=0 ; i < 8 ; i++,ibit8<<=1)
		{
			if (!(data & ibit8) && !(ff_YP & ibit8))
			{
				//the flip-flop is clear while writing a 0 
				/* Sprite crunch!  */
				if (cycle == 15)
					MC_INCR[i] = vic_ii_sprites_crunch_table[MCBASE[i]];
				else if (cycle < 15)
					MC_INCR[i] =3;
				else if (cycle > 55) // Fix for "Spice Up Your Life"
					MC_INCR[i] =3;
			}
		}
		
		if (cycle == 55)
		{
			//This fix was added to make the "Booze Design-Starion" demo work.
			//clock 55 is special because this is where the VIC alters ff_YP based on vicSpriteYExpand.
				
			ff_YP = (vicSpriteYExpand & ff_YP) | (~vicSpriteYExpand & ff_YP & ~data);
		}
		ff_YP = (ff_YP & data) | ~data;
		vicSpriteYExpand = data;
		break;
	case 0x18:	//memory pointers
		vicMemptrVM = ((bit16) data & 0xF0)<<6;
		vicMemptrCB = ((bit16) data & 0xE)<<10;
		break;
	case 0x19:	//interrupt status
		vicINTERRUPT_STATUS = vicINTERRUPT_STATUS & 0xF & ~data;
		if ((vicINTERRUPT_STATUS & vicINTERRUPT_ENABLE & 0xF)==0)
			ClearSystemInterrupt();
		break;
	case 0x1a:	//interrupt enable
		vicINTERRUPT_ENABLE = data & 0xF;
		if ((vicINTERRUPT_STATUS & vicINTERRUPT_ENABLE & 0xF)!=0)
		{
			//nextWakeUpClock = sysclock;
			SetSystemInterrupt();
		}
		else
			ClearSystemInterrupt();
		break;
	case 0x1b:	//sprite data priority
		vicSpriteDataPriorityPrev = vicSpriteDataPriority;
		clockSpriteDataPriorityChange = sysclock + 1;
		vicSpriteDataPriority = data;
		break;
	case 0x1c:	//sprite multicolor
		vicSpriteMultiColorPrev = vicSpriteMultiColor;
		clockSpriteMultiColorChange = sysclock + 1;
		
		vicSpriteMultiColor = data;
		break;
	case 0x1d:	//sprite X expand
		vicSpriteXExpandPrev = vicSpriteXExpand;
		clockSpriteXExpandChange = sysclock + 1;

		vicSpriteXExpand = data;
		break;
	case 0x1e:	//sprite-sprite collision
		break;
	case 0x1f:	//sprite-data collision
		break;
	case 0x20:	//border color
		vicBorderColor = data & 0xf;
		vicBackgroundColor[14] = data & 0xf;
		break;
	case 0x21:	//background color0
		vicBackgroundColor[0] = data & 0xf;
		break;
	case 0x22:	//background color1
		vicBackgroundColor[1] = data & 0xf;
		break;
	case 0x23:	//background color2
		vicBackgroundColor[2] = data & 0xF;
		break;
	case 0x24:	//background color3
		vicBackgroundColor[3] = data & 0xF;
		break;
	case 0x25:	//sprite multicolor0
		vicBackgroundColor[12] = data & 0xF;
		break;
	case 0x26:	//sprite multicolor1
		vicBackgroundColor[13] = data & 0xF;
		break;
	case 0x27:	//color sprite0
		vicBackgroundColor[4] = data & 0xF;
		break;
	case 0x28:	//color sprite1
		vicBackgroundColor[5] = data & 0xF;
		break;
	case 0x29:	//color sprite2
		vicBackgroundColor[6] = data & 0xF;
		break;
	case 0x2a:	//color sprite3
		vicBackgroundColor[7] = data & 0xF;
		break;
	case 0x2b:	//color sprite4
		vicBackgroundColor[8] = data & 0xF;
		break;
	case 0x2c:	//color sprite5
		vicBackgroundColor[9] = data & 0xF;
		break;
	case 0x2d:	//color sprite6
		vicBackgroundColor[10] = data & 0xF;
		break;
	case 0x2e:	//color sprite7
		vicBackgroundColor[11] = data & 0xF;
		break;
	}
}

/*
pRow: Pointer to the destination buffer.
xpos: Destination x co-ordinate to start rendering.
ypos: Destination y co-ordinate to start rendering.
width: Source width in pixels.
height: Source height in pixels.
pBorderBuffer: Pointer to the source border pixel buffer.
pPixelBuffer: Pointer to the source foreground pixel buffer.
startx: Pixel x co-ordinate offset into the source buffers to start reading from.
videoPitch: Destination buffer Y pitch.
bufferPitch: Source buffer Y pitch.
*/
void VIC6569::render(long depth, bool bPixelDoubler, unsigned char *pRow, int xpos, int ypos, int width, int height, bit8 pPixelBuffer[], int startx, int videoPitch, int bufferPitch)
{
	if (bPixelDoubler)
	{
		switch (depth)
		{
		case 32:
			render_32bit_2x(pRow, xpos, ypos, width, height, pPixelBuffer, startx, videoPitch, bufferPitch);
			break;
		case 24:
			render_24bit_2x(pRow, xpos, ypos, width, height, pPixelBuffer, startx, videoPitch, bufferPitch);
			break;
		case 16:
			render_16bit_2x(pRow, xpos, ypos, width, height, pPixelBuffer, startx, videoPitch, bufferPitch);
			break;
		case 8:
			render_8bit_2x(pRow, xpos, ypos, width, height, pPixelBuffer, startx, videoPitch, bufferPitch);
			break;
		}
	}
	else
	{
		switch (appStatus->m_ScreenDepth)
		{
		case 32:
			render_32bit(pRow, xpos, ypos, width, height, pPixelBuffer, startx, videoPitch, bufferPitch);
			break;
		case 24:
			render_24bit(pRow, xpos, ypos, width, height, pPixelBuffer, startx, videoPitch, bufferPitch);
			break;
		case 16:
			render_16bit(pRow, xpos, ypos, width, height, pPixelBuffer, startx, videoPitch, bufferPitch);
			break;
		case 8:
			render_8bit(pRow, xpos, ypos, width, height, pPixelBuffer, startx, videoPitch, bufferPitch);
			break;
		}
	}
}

void VIC6569::render_8bit_2x(unsigned char *pRow, int xpos, int ypos, int width, int height, bit8 pPixelBuffer[], int startx, int videoPitch, int bufferPitch)
{
//TEST ME
bit8 *p = (bit8 *)(pRow + (INT_PTR)(ypos * videoPitch + xpos));
bit8 *q = (bit8 *)(pRow + (INT_PTR)((ypos+1) * videoPitch + xpos));

int i,j;
bit8 v;
bit16 cl;

unsigned short h;
	for (h = 0; h < height; h++)
	{
		for(i=0,j = DISPLAY_START+startx; i < (int)width ; i++,j++)
		{
			v = pPixelBuffer[j];
			cl = vic_color_array8[v];
			cl = cl | (cl << 8);
			*((bit16 *)(p + (2*i))) = cl;

			*((bit16 *)(q + (2*i))) = cl;
		}
		p = (bit8 *)((INT_PTR)p + 2*videoPitch);
		q = (bit8 *)((INT_PTR)q + 2*videoPitch);
		pPixelBuffer += bufferPitch;
	}
}


void VIC6569::render_8bit(unsigned char *pRow, int xpos, int ypos, int width, int height, bit8 pPixelBuffer[], int startx, int videoPitch, int bufferPitch)
{
//TEST ME
bit8 *p = (bit8 *)(pRow + (INT_PTR)(ypos * videoPitch + xpos));

int i,j;
bit8 v;
bit8 cl;

unsigned short h;
	for (h = 0; h < height; h++)
	{
		for(i=0,j = DISPLAY_START+startx; i < (int)width ; i++,j++)
		{
			v = pPixelBuffer[j];
			cl = vic_color_array8[v];
			*((bit8 *)(p + ((i)))) = cl;
		}
		p = (bit8 *)((INT_PTR)p + videoPitch);
		pPixelBuffer += bufferPitch;
	}
}

void VIC6569::render_16bit(unsigned char *pRow, int xpos, int ypos, int width, int height, bit8 pPixelBuffer[], int startx, int videoPitch, int bufferPitch)
{
bit8 *p = (bit8 *)(pRow + (INT_PTR)(ypos * videoPitch + xpos * 2));

int i,j;
bit8 v;
bit16 cl;

unsigned short h;
	for (h = 0; h < height; h++)
	{
		for(i=0,j = DISPLAY_START+startx; i < (int)width ; i++,j++)
		{
			v = pPixelBuffer[j];
			cl = vic_color_array16[v];
			*((bit16 *)(p + (INT_PTR)((2*i)))) = cl;
		}
		p = (bit8 *)((INT_PTR)p + videoPitch);
		pPixelBuffer += bufferPitch;
	}
}

void VIC6569::render_16bit_2x(unsigned char *pRow, int xpos, int ypos, int width, int height, bit8 pPixelBuffer[], int startx, int videoPitch, int bufferPitch)
{
bit8 *p = (bit8 *)(pRow + (INT_PTR)(ypos * videoPitch + xpos * 2));
bit8 *q = (bit8 *)(pRow + (INT_PTR)((ypos+1) * videoPitch + xpos * 2));

int i,j;
bit8 v;
bit32 cl;

unsigned short h;
	for (h = 0; h < height; h++)
	{
		for(i=0,j = DISPLAY_START+startx; i < (int)width ; i++,j++)
		{
			v = pPixelBuffer[j];
			cl = vic_color_array16[v];
			cl = cl | (cl << 16);
			*((bit32 *)(p + (INT_PTR)((2*i) * 2))) = cl;

			*((bit32 *)(q + (INT_PTR)((2*i) * 2))) = cl;
		}
		p = (bit8 *)((INT_PTR)p + 2*videoPitch);
		q = (bit8 *)((INT_PTR)q + 2*videoPitch);
		pPixelBuffer += bufferPitch;
	}
}

void VIC6569::render_24bit(unsigned char *pRow, int xpos, int ypos, int width, int height, bit8 pPixelBuffer[], int startx, int videoPitch, int bufferPitch)
{
//TEST ME
bit8 *p = (bit8 *)(pRow + (INT_PTR)(ypos * videoPitch + xpos * 3));

int i,j;
bit8 v;
bit32 cl;
	
unsigned short h;
	for (h = 0; h < height; h++)
	{
		for(i=0,j = DISPLAY_START+startx; i < (int)width ; i++,j++)
		{
			v = pPixelBuffer[j];
			cl = vic_color_array24[v] & 0xffffff;
			*((bit8 *)(p + (INT_PTR)(i * 3))) = (bit8)(cl & 0xff);
			*((bit16 *)(p + (INT_PTR)((i * 3) +1))) = (bit16)((cl >> 8) & 0xffff);
		}
		p = (bit8 *)((INT_PTR)p + videoPitch);
		pPixelBuffer += bufferPitch;
	}
}

void VIC6569::render_24bit_2x(unsigned char *pRow, int xpos, int ypos, int width, int height, bit8 pPixelBuffer[], int startx, int videoPitch, int bufferPitch)
{
//TEST ME
bit8 *p = (bit8 *)(pRow + (INT_PTR)(ypos * videoPitch + xpos * 3));
bit8 *q = (bit8 *)(pRow + (INT_PTR)((ypos+1) * videoPitch + xpos * 3));

int i,j;
bit8 v;
bit32 cl;

unsigned short h;
	for (h = 0; h < height; h++)
	{
		for(i=0,j = DISPLAY_START+startx; i < (int)width ; i++,j++)
		{
			v = pPixelBuffer[j];
			cl = vic_color_array24[v] & 0xffffff;
			cl = cl | ((cl & 0xff) << 24);
			*((bit32 *)(p + (INT_PTR)((2*i) * 3))) = cl;
			*((bit16 *)(p + (INT_PTR)(((2*i) * 3) +4))) = (bit16)((cl >> 8) & 0xffff);


			*((bit32 *)(q + (INT_PTR)((2*i) * 3))) = cl;
			*((bit16 *)(q + (INT_PTR)(((2*i) * 3) +4))) = (bit16)((cl >> 8) & 0xffff);
		}
		p = (bit8 *)((INT_PTR)p + 2*videoPitch);
		q = (bit8 *)((INT_PTR)q + 2*videoPitch);
		pPixelBuffer += bufferPitch;
	}
}

void VIC6569::render_32bit(unsigned char *pRow, int xpos, int ypos, int width, int height, bit8 pPixelBuffer[], int startx, int videoPitch, int bufferPitch)
{
bit32 *p = (bit32 *)(pRow + (INT_PTR)(ypos * videoPitch + xpos * 4));

int i,j;
bit8 v;

unsigned short h;
	for (h = 0; h < height; h++)
	{
		for(i=0,j = DISPLAY_START+startx; i < (int)width ; i++,j++)
		{
			v = pPixelBuffer[j];
			p[i] = vic_color_array32[v];
		}
		p = (bit32 *)((INT_PTR)p + videoPitch);
		pPixelBuffer += bufferPitch;
	}
}

void VIC6569::render_32bit_2x(unsigned char *pRow, int xpos, int ypos, int width, int height, bit8 pPixelBuffer[], int startx, int videoPitch, int bufferPitch)
{
#ifdef _WIN64
bit64 *p = (bit64 *)(pRow + (INT_PTR)(ypos * videoPitch + xpos * 4));
bit64 *q = (bit64 *)(pRow + (INT_PTR)((ypos+1) * videoPitch + xpos * 4));

int i,j;
bit8 v;
bit64 cl;

unsigned short h;
	for (h = 0; h < height; h++)
	{
		for(i=0,j = DISPLAY_START+startx; i < (int)width ; i++,j++)
		{
			v = pPixelBuffer[j];
			cl = vic_color_array32[v];
			cl = (cl << 32) | cl;
			p[i] = cl;
			q[i] = cl;
		}
		p = (bit64 *)((INT_PTR)p + 2*videoPitch);
		q = (bit64 *)((INT_PTR)q + 2*videoPitch);
		pPixelBuffer += bufferPitch;
	}
#else
bit32 *p = (bit32 *)(pRow + (INT_PTR)(ypos * videoPitch + xpos * 4));
bit32 *q = (bit32 *)(pRow + (INT_PTR)((ypos+1) * videoPitch + xpos * 4));

int i,j;
bit8 v;
bit32 cl;

unsigned short h;
	for (h = 0; h < height; h++)
	{
		for(i=0,j = DISPLAY_START+startx; i < (int)width ; i++,j++)
		{
			v = pPixelBuffer[j];
			cl = vic_color_array32[v];
			p[2*i] = cl;
			p[2*i+1] = cl;
			q[2*i] = cl;
			q[2*i+1] = cl;
		}
		p = (bit32 *)((INT_PTR)p + 2*videoPitch);
		q = (bit32 *)((INT_PTR)q + 2*videoPitch);
		pPixelBuffer += bufferPitch;
	}
#endif
}


bit16 VIC6569::GetRasterLine()
{
	return (bit16)this->vic_raster_line;
}

bit8 VIC6569::GetRasterCycle()
{
	return this->vic_raster_cycle;
}

