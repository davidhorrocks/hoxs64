#include <windows.h>
#include <tchar.h>
#include "register.h"
#include "hconfig.h"
#include "appstatus.h"
#include "savestate.h"
#include "cart.h"
#include "c6502.h"
#include "ram64.h"
#include "cpu6510.h"
#include "viciipalette.h"
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

void VICSprite::Reset(bool poweronreset)
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

/*
VICSprite::GetVicXPosFromCycle

Summary
Gets a sprite x-position value from a VIC cycle. An optional pixel offset will adjust the final result.

[cycle]
Can be a unsigned value 1 - 63. This is the VIC cycle number.

[offset]
Is a signed value <= 8 or >=-8 that is added as an offset to the sprite x-position. 
The offset will be "added" in monitor viewer pixel space and not simply added numerically. 
A positive offset moves to the right. A negative offset move to the left.
*/
bit16 VIC6569::GetVicXPosFromCycle(bit8 cycle, signed char offset)
{
signed short x;
	if (cycle < 14)
	{
		x = 0x18C + ((signed short)cycle * 8);
	}
	else
	{
		x = ((signed short)cycle - 14) * 8 + 4;
	}

	x += offset;
	if (x >= 0x1f8)
	{
		x -= 0x1f8;
	}
	else if (x < 0)
	{
		x = 0x1f8 - x;
	}

	return (bit16)x;
}

/*
VICSprite::SetXPos

Summary
=======
Update sprite X-Position registers (LSB and MSB) as though the CPU write to an X-Pos register occurred in a specific cycle.

Parameters
==========

[x]
Can be a unsigned value 0x000 - 0x1FF. [x] is the new sprite x-position as updated by the CPU.

[currentColumn]
Can be a unsigned value 1 - 63. This is the VIC cycle that the CPU writes to the sprites X-Position register

Members
=======

[xPos]
The sprite X Position register (VIC coordinates)
*/
void VICSprite::SetXPos(bit16 x, bit8 currentColumn)
{
bit16 currentColumnXPos;
signed short xDiff;

	// Get the sprite X position that the CPU writes to the VIC into currentColumnXPos
	// Add a +4 offset to simulate the CPU writing in the second phase of the cycle.
	currentColumnXPos = VIC6569::GetVicXPosFromCycle(currentColumn, +4);
	xDiff = (signed short)(currentColumnXPos - xPos);
	if (xDiff > 0xfa)
	{
		xDiff =  xDiff - 0x1f8;
	}
	else if (xDiff < -0xfa)
	{
		xDiff += 0x1f8;
	}

	// Check if xPos is changing.
	if (xPos != x)
	{
		// xPos is changing.
		if (shiftStatus == srsArm)
		{
			// The sprite is already armed.
			// Check if the old xPos would match.
			if (currentColumn == column && xDiff > 0)
			{
				// Nah! We're too late to change it.
				// The X comparison has already matched before this update could hit the sprite X register.
				// We just attempt moved the sprite to new X position but X comparison matched the previous position.

				// HOST SPECIFIC
				// Prepare to draw.
				InitDraw();

				// Make the sprite active, while that noting that xPos had just been changed.
				shiftStatus = srsActiveXChange;
			}
		}
	}
	

	if (x >= 0x1f8)
	{
		// The sprites comparison never matches x >= 0x1f8
		// Assigning 0 to column is a trick to prevent the sprites first "column" from matching a real column 1 - 63.		
		column = 0;
	}
	else if (x >= 0x194)
	{
		// HOST SPECIFIC
		// Cache the column number that the sprite is first drawn in.
		// Is quicker in C++ to check the [column] for a match only 63 times per line instead of each pixel clock.
		column = (bit8)(((x - 0x18c)/8) & 0xff);

		// This is new true VIC sprite X position.
		// Update columnX to be the new x-position.
		columnX = (((bit16)column) << 3) + 0x18c;

		// HOST SPECIFIC
		// Update the host emulation array index pointer.
		xPixelInit = x - 0x194;
	}
	else
	{
		// HOST SPECIFIC
		// Cache the column number that the sprite is first drawn in.
		column = (bit8)(((x + 4)/8 + 13) & 0xff);

		// WARNING!
		// Hacky columnX can go signed negative instead of wrapping to 0x1f7. 
		// This is a hack to accommodate VICSprite::InitDraw whose computation of 'drawCount' uses a simple subtraction which does not wrap around sprite position 0x1f7		
		columnX = (((bit16)column - 13)<< 3) - 4;

		// WARNING!
		// An other implementation might need to do:
		/*		
		if ((signed short)columnX < 0)
		{
			columnX = (signed short)0x1f8 + (signed short)columnX;
		}
		*/

		// HOST SPECIFIC
		// Update the host emulation array index pointer.
		xPixelInit = x + (DISPLAY_START + LEFT_BORDER_WIDTH/2);
	}

	if ((column == currentColumn) && (shiftStatus == srsArm)  && (xPos != x))
	{
		// Uh Oh!
		// We are setting a new sprite X position that could trigger a match.
		// We better check and see.
		xDiff = (signed short)(currentColumnXPos - x);
		if (xDiff > 0xfa)
		{
			xDiff =  xDiff - 0x1f8;
		}
		else if (xDiff < -0xfa)
		{
			xDiff += 0x1f8;
		}

		if (xDiff > 0)
		{
			// Nah! We missed it.
			// The sprite was armed. We just set an X position for this [currentColumn] but we just missed the sprite X comparison.
			// The sprite shifter will not trigger but it does remain in the armed state.

			// HOST SPECIFIC
			// armDelay is a trick that prevents from matching in this column and line only.
			armDelay = 1;
		}
		//else
		//{
		//	// Phew! we just made it in. That was close!
		//  // The sprite X comparison will now match at the new position.
		//}
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

/*
VICSprite::DrawSprite

Summary
=======
Draws up to 8 pixels of a sprite into a "column". The column is related to that VIC cycle where the CPU 
can first visibly affect the sprite x-expand or multi-colour mode or sprite data priority. 
Such an affect will be seen to transition between two modes in this column.

pixelsLeftToDrawInThisColumn is effectively used as a phase indicator that helps determine if a write to a VIC register 
by the CPU can affect the current pixel.

Parameters
==========

[pixelsLeftToDrawInThisColumn]
The number of pixels to draw. If 8, then the sprite fills the column. 
If less than 8 then the sprite starts at (8 - pixelsLeftToDrawInThisColumn) pixels into the column.

Members
=======

[xPos]
The sprite X Position register (VIC coordinates)
*/
void VICSprite::DrawSprite(int pixelsLeftToDrawInThisColumn)
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

	//Copy member variables into local variables.
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

	// HOST SPECIFIC
	// About colour representation.
	// When bit 7 is set then the colour is deferred to colour register
	// that is custom indexed (not by address) by bits 0 - 4.

	// Sprite multi-colour 0
	color1=12 | 128;

	// Sprite n colour
	color2=(4 + spriteNumber) | 128;

	// Sprite multi-colour 1
	color3=13 | 128;

	// HOST SPECIFIC
	// xPixel is not the VIC's numerical sprite x-position but is the host emulation's index into a pixel array.
	maskShift = (xPixel + 4) & 7;
	maskByte  = (xPixel + 4) / 8;
	
	// HOST SPECIFIC
	// mask is used to check for collision of the sprite with foreground data.
	if (maskByte<14 || maskByte>57)
	{
		mask = 0;
	}
	else
	{
		mask = ((bit16)(vic->pixelMaskBuffer[maskByte])) << 8;
		mask |= (bit16)vic->pixelMaskBuffer[maskByte+1];
		mask <<= maskShift;
	}

	// foregroundmask is used to mask sprites to appear on top of foreground data.
	foregroundmask = ~(mask>>8);
	if ((pixelsLeftToDrawInThisColumn > 6) && (clockSpriteDataPriorityChange == vicCurrentClock))
	{
		// Handle case where the sprite priority is changed during the cycle.
		if (dataPriority == 0)
		{
			// This number 6 represents where in the phase the change is seen.
			foregroundmask |= (0xff >> (pixelsLeftToDrawInThisColumn - 6));
		}
		
		if (dataPriorityPrev == 0)
		{
			foregroundmask |= (0xc0 << (8 - pixelsLeftToDrawInThisColumn));
		}
	}
	else
	{
		if (dataPriority == 0)
		{
			// Foreground data does not have any priority.
			foregroundmask = -1;
		}
	}

	/* Comment out for normal operation	
	foregroundmask = -1; // Test sprites always on top
	*/

	/*
	Loop for each physical pixel to draw.

	A column here references a somewhat arbitrary index into the VIC's 12 pixel pipeline.
	8 pixels get entered into the pixel pipeline for each VIC cycle.
	There will typically be 8 pixels left to draw in a column when we call in here.
	If a sprite was started not at a column boundary then we would call here with less than 8 pixel left
	for the initial part of the sprite. The true number of pixels left in the sprite data shifter buffer 
	is kept in shiftCounter.
	*/
	while (pixelsLeftToDrawInThisColumn > 0)
	{
		/********
		bleedMode : Bleed mode flag
		*********

		Summary
		Indicates whether the sprite data buffer shifter [dataBuffer] is in 
		normal operation or is "bleeding" the last pixel.

		Values
		0: Normal shifter.

		1: Bleed mode shifter.
		*/
		if (bleedMode == 0)
		{
			/****
			ff_XP : The X-Expansion Flip Flop
			*****

			Summary
			Controls X expansion by either repeating the last pixel or getting a new pixel.

			Values			
			0: New pixel mode. This indicates to read a new pixel in to "currentPixel" from the front of the sprite data buffer.

			1: Repeat last pixel mode. This indicates to reuse the pixel data in already in "currentPixel"
			*/
			if (((ff_XP) == 0))
			{
				// Krestage 3
				// It is not enough to stop after shifting 24 pixels. The MC flip-flop must also be in "new pixel mode".
				// If shiftCounter has shifted all 24 pixels and if MC flip-flop has been in
				// "repeat last pixel mode" then more pixel data is produced.
				if ((shiftCounter <= 0) && (ff_MC == 0))
				{
					// MC flip-flop is in "new pixel mode" but the shiftCounter indicates there are no more 
					// bits to shift out.
					if (vic->vicSpriteDisplay & spriteBit)
					{
						// If the sprite display is on then shifter is armed.
						shiftStatus = srsArm;
					}
					else
					{
						// If the sprite display is off then shifter is idled.
						shiftStatus = srsIdle;
						vic->vicSpriteArmedOrActive = vic->vicSpriteArmedOrActive & ~(bit16)spriteBit;
					}

					break;
				}

				multiColourThisPixel = multiColour;
				// This number "6" represents where in the phase the change is seen.
				if ((pixelsLeftToDrawInThisColumn > 6) && (clockMultiColorChange == vicCurrentClock))
				{
					// Multi-colour mode has just been changed in this column, but an VIC 8565 still needs to 
					// draw the first 2 pixels of the column with the previous colour rule.
					// Operation on a VIC 6569 is unverified. Probably, it either is the same or 2 pixels more than a 8565.
					multiColourThisPixel = multiColourPrev;
				}

				/****
				ff_MC : The MC Flip Flop
				*****

				Controls multi-colour expansion by either repeating the last pixel or getting a new pixel.

				Summary				
				0: New pixel mode. This indicates to read new pixel data from the front of the sprite data buffer.

				1: Repeat last pixel mode. This indicates to reuse the pixel data in already in "currentPixel"
				*/

				// Check for multi-colour mode or multi-colour repeat mode.
				if (multiColourThisPixel || ff_MC)
				{
					// Draw this pixel using multi-colour rules.

					// Flip the MC flip-flop.
					ff_MC ^= 1;
					
					if (ff_MC & 1)
					{
						// We leave ff_MC in repeat mode, but we get new pixel data now.
						// Multi-colour mode checks 2 bits at the front of the shifter.
						switch ((bit8)((dataBuffer & 0xc00000) >> 22))
						{
						case 0:
							// Transparent
							currentPixel = 0x00;
							break;
						case 1:
							// Sprite multi-colour 0
							currentPixel = color1;
							break;
						case 2:
							// Sprite's own colour
							currentPixel = color2;
							break;
						case 3:
							// Sprite multi-colour 1
							currentPixel = color3;
							break;
						}
					}
				}
				else
				{
					// Draw this pixel using hi-res rules.
					// Hi-res mode checks only 1 bit at the front of the shifter.
					if (dataBuffer & 0x800000)
					{
						// Sprite's own colour
						currentPixel = color2;
					}
					else
					{
						// Transparent
						currentPixel = 0x00;
					}
				}
			}

			// This number "6" represents where in the phase the change is seen.
			if (pixelsLeftToDrawInThisColumn == 6 && (ff_XP == 1) && (clockMultiColorChange == vicCurrentClock))
			{
				// An edge case occurs where multi-colour mode has just been changed in 
				// this column with exactly 6 pixel left and 
				// the expansion flip-flop is also in repeat pixel mode.
				if (multiColourPrev == 0 && multiColour != 0)
				{
					// Multi-colour is going on
					switch ((bit8)((dataBuffer & 0x800000) >> 22))
					{
					case 0:
						// Transparent
						currentPixel = 0x00;
						break;
					case 1:
						// Sprite multi-colour 0
						currentPixel = color1;
						break;
					case 2:
						// Sprite's own colour
						currentPixel = color2;
						break;
					case 3:
						// Sprite multi-colour 1
						currentPixel = color3;
						break;
					}

					// Leave MC in repeat pixel mode.
					ff_MC = 1;
				}
				else if (multiColourPrev != 0 && multiColour == 0)
				{
					// Multi-colour is going off
					// Leave MC in new pixel mode.
					ff_MC = 0;
				}
			}
		}

		// Check if this sprite has just reached its own sprite number specific bleed position
		// and also that shiftCounter indicates more pixels are remaining.		

		// HOST SPECIFIC
		// Both xPixel and dataLoadIndex are host emualtion C++ array indexes.
		// xPixel is calculated from function SpriteIndexFromClock() that maps VIC columns to host array indexes
		if (xPixel == dataLoadIndex && shiftCounter > 0)
		{			
			// Enable bleed mode.
			bleedMode = 1;

			// The same colour pixel is output 7 times in bleed mode.
			// Force the count to be 7 no matter what.
			shiftCounter = 7; 

			// Mariusz Mlynski reported a difference with multi-colour that is observed when bit 7 of the sprite pointer is clear.
			// If we are right at the start of the bleed position and the MC flip-flop is in repeat mode 
			// and the X-Expand flip-flop is in new mode then we get hi-res pixels.
			if (ff_MC != 0 && ff_XP == 0 && vic->vicSpritePointer[spriteNumber] < 0x2000)
			{
				if (((dataBuffer & 0x800000)) == 0)
				{
					// Transparent
					currentPixel = 0x00;
				}
				else
				{
					// Sprite's own colour
					currentPixel = color2;
				}
			}

			ff_XP = 0;
			ff_MC = 0;
		}

		////TEST sprites on//off
		////currentPixel = 0x0;			

		if (currentPixel != 0)
		{
			// This is a non transparent sprite pixel.
			sprite_sprite_collision = sprite_data_collision = 0;
			if (sprite_collision_line[xPixel])
			{
				// Sprite to sprite collision
				sprite_sprite_collision= sprite_collision_line[xPixel] | spriteBit;
			}
			else
			{
				if (((signed char)foregroundmask) < 0)
				{
					// Sprite data appears through the mask.
					pixelbuffer[xPixel] = currentPixel;
				}
			}

			sprite_collision_line[xPixel]=spriteBit;
			if (((signed short)mask) < 0)
			{
				// Sprite to foreground collision 
				sprite_data_collision=spriteBit;
			}

#define SPRITEPIXELOFFSET 4
			// Sprite collision latency.
			// Collisons that occur early enough with in the column are seen in the registers in this cycle.
			// Both vicCurrSprite_sprite_collision and vicCurrSprite_data_collision
			// get ORed into their repestive VIC collision registers after this function finishes.
			// This number "SPRITEPIXELOFFSET" represents where in the phase the collsion can be read in this cycle.
			if (pixelsLeftToDrawInThisColumn > SPRITEPIXELOFFSET)
			{
				vic->vicCurrSprite_sprite_collision |= sprite_sprite_collision;
				vic->vicCurrSprite_data_collision |= sprite_data_collision;
			}
			else
			{
				vic->vicNextSprite_sprite_collision |= sprite_sprite_collision;
				vic->vicNextSprite_data_collision |= sprite_data_collision;
			}

			// Check for sprite sprite IRQ flag.
			if (sprite_sprite_collision != 0)
			{
				// Collisons with sprites that occur early enough with in this column will trigger an IRQ in this cycle.
				// This number "SPRITEPIXELOFFSET" represents where in the phase the IRQ can make this cycle.
				if (pixelsLeftToDrawInThisColumn > SPRITEPIXELOFFSET)
				{
					// We are early in the column so we see the IRQ early.
					vic->vicSpriteSpriteInt|=1;
				}
				else
				{
					// We are late in the column so we see the IRQ late.
					vic->vicSpriteSpriteInt|=2;
				}
			}

			if (sprite_data_collision != 0)
			{
				// Collisons with data that occur early enough with in this column will trigger an IRQ in this cycle.
				// This number "SPRITEPIXELOFFSET" represents where in the phase the IRQ can make this cycle.
				if (pixelsLeftToDrawInThisColumn > SPRITEPIXELOFFSET)
				{
					// We are early in the column so we see the IRQ early.
					vic->vicSpriteDataInt|=1;
				}
				else
				{
					// We are late in the column so we see the IRQ late.
					vic->vicSpriteDataInt|=2;
				}
			}
		}

		if (bleedMode != 0)
		{
			// We are in bleed mode.
			if (shiftCounter <= 0)
			{
				//Ensure the condition for going idle is satisfied.
				ff_XP = 0;
				ff_MC = 0;
				break;
			}

			// Count this pixel as shifted out of the sprite data buffer.
			shiftCounter--;
		}
		else
		{
			// We are normal mode.
			xExpandThisPixel = xExpand;
			if ((pixelsLeftToDrawInThisColumn > 6) && (clockSpriteXExpandChange == vicCurrentClock))
			{
				// Use the previous sprite expand register value if we are early in the column.
				xExpandThisPixel = xExpandPrev;
			}

			if (xExpandThisPixel || ff_XP != 0)
			{
				// Toggle the X-Expand flip-flop if either we are in X-Expanded mode or 
				// the X-Expansion flip-flop was in repeat mode.
				ff_XP ^= 1;
			}

			if (ff_XP == 0)
			{
				// if we find X-Expansion flip-flop in new pixel mode at the end of this run loop then
				// count this pixel as shifted out of the sprite data buffer.
				dataBuffer <<= 1;
				shiftCounter--;
			}
		}

		mask <<= 1;
		foregroundmask <<= 1;

		// HOST SPECIFIC
		// Update host emulation index pointer.		
		xPixel++;
		if (xPixel >= VIDEOWIDTH)
		{
			// Handle array wraparound.
			xPixel -= VIDEOWIDTH;
		}

		// Count down a pixel in this VIC column cycle as done.
		pixelsLeftToDrawInThisColumn--;
	}

	// Check if we can stop shifting.
	if ((shiftCounter <= 0 && ff_XP == 0 && ff_MC == 0))
	{		
		// There must be no more pixels and both ff_XP and ff_MC must be in "new pixel mode".
		if (vic->vicSpriteDisplay & spriteBit)
		{
			// If the sprite display is on then shifter becomes armed.
			shiftStatus = srsArm;
		}
		else
		{
			// If the sprite display is on then shifter becomes idle.
			shiftStatus = srsIdle;

			// HOST SPECIFIC
			// optimisation
			vic->vicSpriteArmedOrActive = vic->vicSpriteArmedOrActive & ~(bit16)spriteBit;
		}
	}
}

void VICSprite::GetState(SsVicSprite &state)
{
	ZeroMemory(&state, sizeof(state));
	state.dataBuffer = dataBuffer;
	state.bufferPos = bufferPos;
	state.xPos = xPos;
	state.column = column;
	state.columnX = columnX;
	state.xPixelInit = xPixelInit;
	state.xPixel = xPixel;
	state.dataLoadClock = dataLoadClock;
	state.dataLoadedClock = dataLoadedClock;
	state.dataLoadIndex = dataLoadIndex;
	state.shiftStatus = shiftStatus;//enum SpriteShiftRegister
	state.shiftCounter = shiftCounter;
	state.ff_XP = ff_XP;
	state.ff_MC = ff_MC;
	state.bleedMode = bleedMode;
	state.currentPixel = currentPixel;
	state.armDelay = armDelay;
	state.drawCount = drawCount;
	state.spriteNumber = spriteNumber;
	state.spriteBit = spriteBit;
}

void VICSprite::SetState(const SsVicSprite &state)
{
	dataBuffer = state.dataBuffer;
	bufferPos = state.bufferPos;
	xPos = state.xPos;
	column = state.column;
	columnX = state.columnX;
	xPixelInit = state.xPixelInit;
	xPixel = state.xPixel;
	dataLoadClock = state.dataLoadClock;
	dataLoadedClock = state.dataLoadedClock;
	dataLoadIndex = state.dataLoadIndex;
	shiftStatus = (SpriteShiftRegister)state.shiftStatus;
	shiftCounter = state.shiftCounter;
	ff_XP = state.ff_XP;
	ff_MC = state.ff_MC;
	bleedMode = state.bleedMode;
	currentPixel = state.currentPixel;
	armDelay = state.armDelay;
	drawCount = state.drawCount;
	spriteNumber = state.spriteNumber;
	spriteBit = state.spriteBit;
}

void VIC6569::SetSystemInterrupt()
{
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
	{
		return;
	}

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
		{
			break;
		}

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
	if (vicCSEL == 0)
	{
		if (vicVerticalBorder == 0) 
		{
			vicMainBorder = 0;
		}
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
	if (vicCSEL == 1)
	{
		if (vicVerticalBorder == 0)
		{
			vicMainBorder=0;
		}
	}

	vic_border_part_40|=(vicMainBorder<<1);
}

void VIC6569::C_ACCESS()
{	
	if (vic_allow_c_access)
	{
		if (vicAEC < 0)
		{
			VideoMatrix[vicVMLI] = vic_ph2_read_byte(vicMemptrVM | vicVC) | ((bit16)vic_read_color_byte(vicVC) << 8);
		}
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
			//gData = 0;
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
					bit32& char0 = VideoMatrix[((iWritevicVMLI2-1) % gap) % NUM_SCREEN_COLUMNS];
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
	return (de00_byte=vic_memory_map_read[(address>>12) & 3][address & 0x0fff]);
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
	return (vic_memory_map_read[(address>>12) & 3][address & 0x0fff]);
}


bit8 VIC6569::vic_ph2_read_aec_byte(bit16 address)
{
	if (m_bVicBankChanging)
	{
		m_bVicBankChanging = false;
		SetMMU(vicBankChangeByte);
	}

	if (vicAEC < 0)
	{
		// AEC stays low in second phase.
		return vic_memory_map_read[(address>>12) & 3][address & 0x0fff];
	}
	else
	{
		return 0xFF;
	}
}

bit8 VIC6569::vic_ph2_read_3fff_aec_byte()
{
	if (m_bVicBankChanging)
	{
		m_bVicBankChanging = false;
		SetMMU(vicBankChangeByte);
	}

	if (vicAEC < 0)
	{
		// AEC stays low in second phase.
		return *vic_3fff_ptr;
	}
	else
	{
		return 0xFF;
	}
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
	{
		gData = 0;
	}

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
	{
		return;
	}

	if (verticalBorder)
	{
		gData = 0;
	}

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
	int i;
	for (i = 0; i < VicIIPalette::NumColours; i++)
	{
		vic_color_array[i] = (bit32)VicIIPalette::Pepto[i];
	}

	ram = NULL;
	cpu = NULL;
	pGx = NULL;
	appStatus = NULL;
	vic_pixelbuffer = NULL;
	for (i = 0; i < 8; i++)
	{
		vicSprite[i].vic = this;
	}

	frameNumber = 0;
	lastBackedUpFrameNumber = 0;
	currentPixelBufferNumber = 0;
}

void VIC6569::InitReset(ICLK sysclock, bool poweronreset)
{
int i,j;
bit32 initial_raster_line = PAL_MAX_LINE;

	CurrentClock = sysclock;
	frameNumber = 0;
	lastBackedUpFrameNumber = 0;
	lastBackedUpFrameNumber--;
	vicMemoryBankIndex = 0;
	LP_TRIGGER=0;
	vicLightPen=1;
	vic_check_irq_in_cycle2=false;
	vicAEC=3;
	vicBA = 1;
	clockBALow = 0;
	clockBAHigh = 0;
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

	vic_in_display_y=0;

	vic_top_compare=0x37;
	vic_bottom_compare=0xf7;
	vic_left_compare=0x1f;
	vic_right_compare=0x14f;

	cpu_next_op_code = 0;
	vic_allow_c_access = false;

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
	vic_raster_cycle = PAL_CLOCKS_PER_LINE;

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
		vicSprite[i].Reset(poweronreset);
		vicSpriteX[i]=0;
		vicSpriteY[i]=0;
		SpriteXChange(i,0,1);
	}
	vicClearingYExpandRegInClock15 = 0;
	vicSpriteArmedOrActive = 0;
	vicSpriteDMA=0;
	vicSpriteDMAPrev=0;
	vicSpriteDisplay=0;
	vicSpriteYMatch=0;

	vic_badline=0;
	vic_address_line_info = &BA_line_info[vicSpriteDMA][vic_badline];

	ff_YP=0xff;
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

	vic_pixelbuffer = ScreenPixelBuffer[this->currentPixelBufferNumber][initial_raster_line];
	ZeroMemory(pixelMaskBuffer,sizeof(pixelMaskBuffer));
	DF_PixelsToSkip = 0;
}

void VIC6569::Reset(ICLK sysclock, bool poweronreset)
{
	InitReset(sysclock, poweronreset);
	ClearSystemInterrupt();
}

void VIC6569::PreventClockOverflow()
{
	const ICLKS CLOCKSYNCBAND_NEAR = PAL_5_MINUTES;
	const ICLKS CLOCKSYNCBAND_FAR = OVERFLOWSAFTYTHRESHOLD;
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
	{
		clockReadSpriteDataCollision = ClockBehindNear;
	}

	if ((ICLKS)(CurrentClock - clockReadSpriteSpriteCollision) > CLOCKSYNCBAND_FAR)
	{
		clockReadSpriteSpriteCollision = ClockBehindNear;
	}

	if ((ICLKS)(CurrentClock - clockFirstForcedBadlineCData) > CLOCKSYNCBAND_FAR)
	{
		clockFirstForcedBadlineCData = ClockBehindNear;
	}

	if ((ICLKS)(CurrentClock - clockBALow) > CLOCKSYNCBAND_FAR)
	{
		clockBALow = ClockBehindNear;
	}

	if ((ICLKS)(CurrentClock - clockBAHigh) > CLOCKSYNCBAND_FAR)
	{
		clockBAHigh = ClockBehindNear;
	}	
}

ICLK VIC6569::GetCurrentClock()
{
	return CurrentClock;
}

void VIC6569::SetCurrentClock(ICLK sysclock)
{
ICLK v = sysclock - CurrentClock;
	CurrentClock += v;	
	clockSpriteMultiColorChange += v;
	clockSpriteXExpandChange += v;
	clockSpriteDataPriorityChange += v;
	clockReadSpriteDataCollision += v;
	clockReadSpriteSpriteCollision += v;
	clockFirstForcedBadlineCData += v;
}

VIC6569::~VIC6569()
{
	Cleanup();
}

HRESULT VIC6569::Init(CAppStatus *appStatus, Graphics* pGx, RAM64 *ram, IC6510 *cpu, ICartInterface* cart, IBreakpointManager *pIBreakpointManager)
{
	Cleanup();

	this->appStatus = appStatus;
	this->pGx = pGx;
	this->ram = ram;
	this->cpu = cpu;
	this->cart = cart;
	this->m_pIBreakpointManager = pIBreakpointManager;
	SetMMU(0);
	setup_multicolor_mask_table();
	setup_vic_ba();
	this->setup_color_tables();
	Reset(CurrentClock, true);
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

void VIC6569::setup_color_tables()
{
bit16 i;
bit8 red,green,blue;
bit32 cl;

	for (i = 0; i < 256; i++)
	{
		if (appStatus != NULL)
		{
			cl = appStatus->m_colour_palette[i & 0xf];
		}
		else
		{
			cl = VicIIPalette::Pepto[i & 0xf];
		}

		vic_color_array[i] = cl;
	}

	DXGI_FORMAT format = GraphicsHelper::DefaultPixelFormat;
	//if (pGx)
	//{
	//	pGx->GetCurrentBufferFormat(&format);
	//}

	// Assume 32 bit colour
	for (i=0 ; i < 256 ; i++)
	{
		red=(bit8)((vic_color_array[i & 15] & 0x00ff0000) >> 16);
		green=(bit8)((vic_color_array[i & 15] & 0x0000ff00) >> 8);
		blue=(bit8)((vic_color_array[i & 15] & 0x000000ff));

		cl = GraphicsHelper::ConvertColour(format, RGB(red, green, blue));
		vic_color_array32[i] =  cl;		
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
	lastBackedUpFrameNumber = frameNumber;
}

HRESULT VIC6569::UpdateBackBufferLine(bit8 *pDestSurfLine, int videoPitch, bit16 line, bit8 cycle)
{
	if (pGx == NULL)
	{
		return E_FAIL;
	}

	if (!pGx->isInitOK)
	{
		return E_FAIL;
	}

	if (line < 0 || line > PAL_MAX_LINE)
	{
		return E_FAIL;
	}

	if (cycle < 1 || cycle > PAL_CLOCKS_PER_LINE)
	{
		return E_FAIL;
	}

	int currentFramePixelBufferNumber = currentPixelBufferNumber;
	int previousFramePixelBufferNumber = currentFramePixelBufferNumber ^ 1;
	int buffer_line = 0;
	int current_line = line;
	int cursor_index = ((int)cycle*8 - 20);
	cursor_index += 8;
	if (cursor_index < 0)
	{
		current_line--;
		if (current_line < 0)
		{
			current_line = PAL_MAX_LINE;
		}
		cursor_index = (cursor_index + PAL_VIDEO_WIDTH) % PAL_VIDEO_WIDTH;
	}

	bool bIsLeftOfCursor;
	for (int x = 0; x < _countof(LinePixelBuffer[0]); x++)
	{
		if (x < cursor_index)
		{
			bIsLeftOfCursor = true;
		}
		else
		{
			bIsLeftOfCursor = false;
		}

		if (bIsLeftOfCursor)
		{
			LinePixelBuffer[0][x] = ScreenPixelBuffer[currentFramePixelBufferNumber][current_line][x];
		}
		else
		{
			LinePixelBuffer[0][x] = ScreenPixelBuffer[previousFramePixelBufferNumber][current_line][x];
		}
	}

	C64Display& c64display = pGx->c64display;
	int height = 1;
	int width = c64display.displayWidth;
	int startx = c64display.displayStart;
	int starty = current_line;
	int ypos = 0;
	int xpos = 0;
	int bufferPitch = _countof(ScreenPixelBuffer[0][0]);
	if ((unsigned int)starty >= c64display.displayFirstVicRaster && (unsigned int)starty <= c64display.displayLastVicRaster)
	{
		bit8 *pPixelBuffer = LinePixelBuffer[0];
		ypos = starty - c64display.displayFirstVicRaster;
		render(pDestSurfLine, xpos, ypos, width, height, pPixelBuffer, pPixelBuffer, startx, videoPitch, bufferPitch, 0);
	}

	return S_OK;
}

HRESULT VIC6569::UpdateBackBuffer()
{
	HRESULT hr = E_FAIL;
	if (pGx == NULL)
	{
		return E_FAIL;
	}

	if (!pGx->isInitOK)
	{
		return E_FAIL;
	}

	if (appStatus->m_bDebug)
	{
		if (lastBackedUpFrameNumber != frameNumber)
		{
			BackupMainPixelBuffers();
		}
	}

	int currentFramePixelBufferNumber = currentPixelBufferNumber;
	bit16 current_line = (bit16)vic_raster_line;
	if (vic_check_irq_in_cycle2)
	{
		current_line = 0;
	}

	int previousFramePixelBufferNumber = currentFramePixelBufferNumber ^ 1;
	Microsoft::WRL::ComPtr<ID3D11Resource> screenResource;
	C64Display& c64display = pGx->c64display;
	hr = c64display.screenTexture.GetTextureResource(&screenResource);
	if (SUCCEEDED(hr))
	{
		if (screenResource != nullptr)
		{
			D3D11_MAPPED_SUBRESOURCE mapped;
			hr = pGx->deviceContext->Map(screenResource.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			if (SUCCEEDED(hr))
			{				
				bit8* pDestSurfLine = (bit8*)mapped.pData;
				int height = c64display.displayHeight;
				int width = c64display.displayWidth;
				int startx = c64display.displayStart;
				int bufferPitch = _countof(ScreenPixelBuffer[0][0]);
				bit8* pPixelBufferCurrentFrame = &ScreenPixelBuffer[currentFramePixelBufferNumber][c64display.displayFirstVicRaster][0];
				bit8* pPixelBufferPreviousFrame = &ScreenPixelBuffer[previousFramePixelBufferNumber][c64display.displayFirstVicRaster][0];
				render(pDestSurfLine, 0, 0, width, height, pPixelBufferCurrentFrame, pPixelBufferPreviousFrame, startx, mapped.RowPitch, bufferPitch, c64display.displayFirstVicRaster);
				if (appStatus->m_bDebug)
				{
					hr = UpdateBackBufferLine(pDestSurfLine, mapped.RowPitch, current_line, vic_raster_cycle);
				}

				pGx->deviceContext->Unmap(screenResource.Get(), 0);
			}
		}
		else
		{ 
			hr = E_POINTER;
		}
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

		// Mariusz discovered that a VIC bug such that a sprite with x-position 0x163 gets drawn at 0x164.
		// My main C64C with separate colour SRAM machine once exhibited this bug some years ago but I cannot reproduce it on that same machine now.
		// A another C64 with old VIC 6569 does not have the bug. Of two C64's each with a new VIC 8565, only one of them has the bug.
		// A recent VICE test VICII\sb_sprite_fetch\sbsprf24-163.prg indicates that the bug may not be the norm.
		// Therefore I will remove this possibly uncommon bug until it is determinded that the bug is the preferred normal.

		//if ((vicSpriteDisplay & (1 << spriteNo)) == 0)
		//{
		//	//If the sprite display was off then any sprite at x==0x163 gets drawn at 0x164
		//	if (sprite.xPos == SPRITE_DISPLAY_CHECK_XPOS - 1)
		//	{
		//		sprite.InitDraw(SPRITE_DISPLAY_CHECK_XPOS, SPRITE_DISPLAY_CHECK_XPOS, sprite.xPixelInit + 1);
		//		sprite.shiftStatus = VICSprite::srsActiveXChange;
		//	}
		//}
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
		if (ba != vicBA)
		{
			vicBA = ba;
			clockBAHigh = CurrentClock;
			cpu->SetVicRdyHigh(CurrentClock);
			vicAEC = 3;
		}
	}
	else
	{
		if (ba != vicBA)
		{
			vicBA = ba;
			clockBALow = CurrentClock;
			cpu->SetVicRdyLow(CurrentClock);
		}

		if (vicAEC >= 0)
		{
			--vicAEC;
		}

		// If cpu->IsVicNotifyCpuWriteCycle() == false or cycles != 0 then it means that the cpu has recently executed a read cycle which should have caused a cpu BA delay.
		// We synchronise the vic with all cpu-write cycles but allow the cpu to run ahead with reads from RAM.
		// If we call here on a cpu write cycle then 'cycles' will be zero though 'cycles' may be zero for cpu read cycles too.
		if (!cpu->IsDebug())
		{
			if (!cpu->IsVicNotifyCpuWriteCycle() || cycles != 0)
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
bit8 *ptr8;
bit8 data8;

	clocks = sysclock - CurrentClock;
	while ((ICLKS)clocks-- > 0)
	{
		CurrentClock++;
		cyclePrev = vic_raster_cycle++;
		if (vic_raster_cycle > PAL_CLOCKS_PER_LINE)
		{
			vic_raster_cycle = 1;
		}

		cycle = vic_raster_cycle;	
		switch (cycle)
		{
		case 1:
			if (vicSpriteArmedOrActive != 0)
			{
				DrawSprites(cyclePrev);
			}

			if(vic_raster_line == PAL_MAX_LINE)
			{
				vic_check_irq_in_cycle2=true;
				currentPixelBufferNumber ^= 1;
				vic_pixelbuffer = ScreenPixelBuffer[currentPixelBufferNumber][0];
				frameNumber++;
				if (appStatus->m_bDebug)
				{
					BackupMainPixelBuffers();
				}
			}
			else
			{
				vic_raster_line++;
				vic_pixelbuffer = ScreenPixelBuffer[currentPixelBufferNumber][vic_raster_line];
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
                {
					bVicRasterMatch = false;
                }
			}

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
			{
				LOADSPRITEDATA(sprite3, 2) = vic_ph2_read_3fff_aec_byte();//0xff;
			}

			break;
		case 2:
			if (vicSpriteArmedOrActive != 0)
			{
				DrawSprites(cyclePrev);
			}

			if(vic_check_irq_in_cycle2)
			{
				vic_check_irq_in_cycle2=false;
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
                {
					bVicRasterMatch = false;
                }
			}

			SetBA(clocks, cycle);
			if (vicSpriteDMA & (1<<sprite3))
			{
				LOADSPRITEDATA(sprite3, 1) = vic_ph1_read_byte(vicSpritePointer[sprite3] + MC[sprite3]);
				MC[sprite3] = (MC[sprite3] + 1) & 63;
				LOADSPRITEDATA(sprite3, 0) = vic_ph2_read_byte(vicSpritePointer[sprite3] + MC[sprite3]);
				MC[sprite3] = (MC[sprite3] + 1) & 63;
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
			{
				DrawSprites(cyclePrev);
			}

			SetBA(clocks, cycle);
			vicSpritePointer[sprite4] = (bit16)vic_ph1_read_byte(vicMemptrVM + 0x3f8 + sprite4) * 64;
			if (vicSpriteDMA & (1<<sprite4))
			{
				LOADSPRITEDATA(sprite4, 2) = vic_ph2_read_byte(vicSpritePointer[sprite4] + MC[sprite4]);
				MC[sprite4] = (MC[sprite4] + 1) & 63;
			}
			else
			{
				LOADSPRITEDATA(sprite4, 2) = vic_ph2_read_3fff_aec_byte();//0xff;
			}

			break;
		case 4:
			if (vicSpriteArmedOrActive != 0)
			{
				DrawSprites(cyclePrev);
			}

			SetBA(clocks, cycle);
			if (vicSpriteDMA & (1<<sprite4))
			{
				LOADSPRITEDATA(sprite4, 1) = vic_ph1_read_byte(vicSpritePointer[sprite4] + MC[sprite4]);
				MC[sprite4] = (MC[sprite4] + 1) & 63;
				LOADSPRITEDATA(sprite4, 0) = vic_ph2_read_byte(vicSpritePointer[sprite4] + MC[sprite4]);
				MC[sprite4] = (MC[sprite4] + 1) & 63;
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
			{
				DrawSprites(cyclePrev);
			}

			SetBA(clocks, cycle);
			vicSpritePointer[sprite5] = (bit16)vic_ph1_read_byte(vicMemptrVM + 0x3f8 + sprite5) * 64;
			if (vicSpriteDMA & (1<<sprite5))
			{
				LOADSPRITEDATA(sprite5, 2) = vic_ph2_read_byte(vicSpritePointer[sprite5] + MC[sprite5]);
				MC[sprite5] = (MC[sprite5] + 1) & 63;
			}
			else
			{
				LOADSPRITEDATA(sprite5, 2) = vic_ph2_read_3fff_aec_byte();//0xff;
			}

			break;
		case 6:
			if (vicSpriteArmedOrActive != 0)
			{
				DrawSprites(cyclePrev);
			}

			SetBA(clocks, cycle);
			if (vicSpriteDMA & (1<<sprite5))
			{
				LOADSPRITEDATA(sprite5, 1) = vic_ph1_read_byte(vicSpritePointer[sprite5] + MC[sprite5]);
				MC[sprite5] = (MC[sprite5] + 1) & 63;
				LOADSPRITEDATA(sprite5, 0) = vic_ph2_read_byte(vicSpritePointer[sprite5] + MC[sprite5]);
				MC[sprite5] = (MC[sprite5] + 1) & 63;
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
			{
				DrawSprites(cyclePrev);
			}

			SetBA(clocks, cycle);
			vicSpritePointer[sprite6] = (bit16)vic_ph1_read_byte(vicMemptrVM + 0x3f8 + sprite6) * 64;
			if (vicSpriteDMA & (1<<sprite6))
			{
				LOADSPRITEDATA(sprite6, 2) = vic_ph2_read_byte(vicSpritePointer[sprite6] + MC[sprite6]);
				MC[sprite6] = (MC[sprite6] + 1) & 63;
			}
			else
			{
				LOADSPRITEDATA(sprite6, 2) = vic_ph2_read_3fff_aec_byte();//0xff;
			}

			break;
		case 8:
			if (vicSpriteArmedOrActive != 0)
			{
				DrawSprites(cyclePrev);
			}

			SetBA(clocks, cycle);
			if (vicSpriteDMA & (1<<sprite6))
			{
				LOADSPRITEDATA(sprite6, 1) = vic_ph1_read_byte(vicSpritePointer[sprite6] + MC[sprite6]);
				MC[sprite6] = (MC[sprite6] + 1) & 63;
				LOADSPRITEDATA(sprite6, 0) = vic_ph2_read_byte(vicSpritePointer[sprite6] + MC[sprite6]);
				MC[sprite6] = (MC[sprite6] + 1) & 63;
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
			{
				DrawSprites(cyclePrev);
			}

			SetBA(clocks, cycle);
			vicSpritePointer[sprite7] = (bit16)vic_ph1_read_byte(vicMemptrVM + 0x3f8 + sprite7) * 64;
			if (vicSpriteDMA & (1<<sprite7))
			{
				LOADSPRITEDATA(sprite7, 2) = vic_ph2_read_byte(vicSpritePointer[sprite7] + MC[sprite7]);
				MC[sprite7] = (MC[sprite7] + 1) & 63;
			}
			else
			{
				LOADSPRITEDATA(sprite7, 2) = vic_ph2_read_3fff_aec_byte();//0xff;
			}

			break;
		case 10: //0x1e0:		
			DrawForeground0(vicXSCROLL, vicLastCData, vicECM_BMM_MCM, cycle);
			if (vicSpriteArmedOrActive != 0)
			{
				DrawSprites(cyclePrev);
			}

			COLOR_FOREGROUND(vicBackgroundColor, cycle);
			SetBA(clocks, cycle);
			if (vicSpriteDMA & (1<<sprite7))
			{
				LOADSPRITEDATA(sprite7, 1) = vic_ph1_read_byte(vicSpritePointer[sprite7] + MC[sprite7]);
				MC[sprite7] = (MC[sprite7] + 1) & 63;
				LOADSPRITEDATA(sprite7, 0) = vic_ph2_read_byte(vicSpritePointer[sprite7] + MC[sprite7]);
				MC[sprite7] = (MC[sprite7] + 1) & 63;
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
			{
				DrawSprites(cyclePrev);
			}

			DRAW_BORDER(cycle);
			COLOR_FOREGROUND(vicBackgroundColor, cycle);
			SetBA(clocks, cycle);
			vic_ph1_read_byte(0x3f00 | vicDRAMRefresh--);
			break;
		case 12:
			//Left most colourised pixels in full borders mode.
			DrawForeground0(vicXSCROLL, vicLastCData, vicECM_BMM_MCM, cycle);
			if (vicSpriteArmedOrActive != 0)
			{
				DrawSprites(cyclePrev);
			}

			DRAW_BORDER(cycle);
			COLOR_FOREGROUND(vicBackgroundColor, cycle);
			SetBA(clocks, cycle);
			vic_ph1_read_byte(0x3f00 | vicDRAMRefresh--);
			break;
		case 13:
			DrawForeground0(vicXSCROLL, vicLastCData, vicECM_BMM_MCM, cycle);
			if (vicSpriteArmedOrActive != 0)
			{
				DrawSprites(cyclePrev);
			}

			DRAW_BORDER(cycle);
			COLOR_FOREGROUND(vicBackgroundColor, cycle);
			SetBA(clocks, cycle);
			vic_ph1_read_byte(0x3f00 | vicDRAMRefresh--);
			break;
		case 14:
			DrawForeground0(vicXSCROLL, vicLastCData, vicECM_BMM_MCM, cycle);
			if (vicSpriteArmedOrActive != 0)
			{
				DrawSprites(cyclePrev);
			}

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
			{
				DrawSprites(cyclePrev);
			}

			DRAW_BORDER(cycle);
			COLOR_FOREGROUND(vicBackgroundColor, cycle);
			SetBA(clocks, cycle);
			vic_ph1_read_byte(0x3f00 | vicDRAMRefresh--);
			vic_allow_c_access = vic_badline!=0;
			vicIDLE_DELAY=0;
			C_ACCESS();
			break;
		case 16: //0x18:
			DrawForeground0(vicXSCROLL, vicLastCData, vicECM_BMM_MCM, cycle);
			if (vicSpriteArmedOrActive != 0)
			{
				DrawSprites(cyclePrev);
			}

			DRAW_BORDER(cycle);
			COLOR_FOREGROUND(vicBackgroundColor, cycle);
			vicSpriteDMAPrev = vicSpriteDMA;
			for (ibit8=1,i=0 ; i < 8 ; i++,ibit8<<=1)
			{
				if (ff_YP & ibit8)
				{
					if (vicClearingYExpandRegInClock15 & ibit8)
					{
						MCBASE[i] = (0x2A & MCBASE[i] & MC[i]) | (0x15 & (MCBASE[i] | MC[i]));
					}
					else
					{
						MCBASE[i] = MC[i];
					}
				}				
				if (MCBASE[i]==63)
				{
					vicSpriteDMA &= ~ibit8;
				}
			}

			vicClearingYExpandRegInClock15 = 0;
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
			{
				DrawSprites(cyclePrev);
			}

			DrawBorder4(cycle);//uses vicMainBorder_old
			draw_40_col_left_border1(cycle); // uses vic_border_part_40
			COLOR_FOREGROUND(vicBackgroundColor, cycle);
			SetBA(clocks, cycle);
			vicSpriteDMAPrev = vicSpriteDMA;
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
			{
				DrawSprites(cyclePrev);
			}

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
			{
				DrawSprites(cyclePrev);
			}

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
			{
				DrawSprites(cyclePrev);
			}

			DRAW_BORDER(cycle);
			COLOR_FOREGROUND(vicBackgroundColor, cycle);
			vic_allow_c_access = false;
			vicSpriteDMAPrev = vicSpriteDMA;
			vicSpriteYMatch=0;
			ff_YP= ((~ff_YP) & vicSpriteYExpand) | (~vicSpriteYExpand & ff_YP);
			for (ibit8=1,i=0 ; i < 8 ; i++)
			{
				if ((~vicSpriteDMA & vicSpriteEnable & ibit8)!=0 && vicSpriteY[i]==(vic_raster_line & 0xFF) )
				{
					vicSpriteDMA|=ibit8;
					MCBASE[i]=0;
					if (vicSpriteYExpand & ibit8)
					{
						ff_YP &= ~ibit8;
					}

				}

				ibit8<<=1;
			}

			vic_address_line_info = &BA_line_info[vicSpriteDMA][0];
			SetBA(clocks, cycle);
			vicLastCDataPrev2 = vicLastCDataPrev;
			vicLastGDataPrev2 = vicLastGDataPrev;
			vicLastCDataPrev = vicLastCData;
			vicLastGDataPrev = vicLastGData;
			vicLastGData = G_ACCESS(vicECM_BMM_MCM_prev, vicLastCData);
			break;
		case 56:
			DrawForeground(vicLastGData, vicXSCROLL, vicLastCData, vicECM_BMM_MCM, 0, vicCharDataOutputDisabled, cycle);
			if (vicCharDataOutputDisabled!=0)
			{
				pixelMaskBuffer[cycle] = 0;
			}

			if (vicSpriteArmedOrActive != 0)
			{
				DrawSprites(cyclePrev);
			}

			DrawBorder7(cycle);
			check_38_col_right_border();
			draw_38_col_right_border1(cycle);
			COLOR_FOREGROUND(vicBackgroundColor, cycle);
			vicSpriteDMAPrev = vicSpriteDMA;
			for (ibit8=1,i=0 ; i < 8 ; i++,ibit8<<=1)
			{
				if ((~vicSpriteDMA & vicSpriteEnable & ibit8)!=0 && vicSpriteY[i]==(vic_raster_line & 0xFF))
				{
					MCBASE[i]=0;
					vicSpriteDMA|=ibit8;
					if (vicSpriteYExpand & ibit8)
					{
						ff_YP &= ~ibit8;
					}
				}
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
			vicSpriteDMAPrev = vicSpriteDMA;
			if (vicSpriteArmedOrActive != 0)
			{
				DrawSprites(cyclePrev);
			}

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
			{
				DrawSprites(cyclePrev);
			}

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
				{
					SpriteIdle(i);
				}
			}

			vicSpritePointer[sprite0] = (bit16)vic_ph1_read_byte(vicMemptrVM + 0x3f8 + sprite0) * 64;
			if (vicSpriteDMA & (1<<sprite0))
			{
				//Enabling a sprite in cycle 55 may not allow the 3 cycle notice need for the VIC gain access to the bus during second clock half.
				LOADSPRITEDATA(sprite0, 2) = vic_ph2_read_aec_byte(vicSpritePointer[sprite0] + MC[sprite0]);
				MC[sprite0] = (MC[sprite0] + 1) & 63;
			}
			else
			{
				LOADSPRITEDATA(sprite0, 2) = vic_ph2_read_3fff_aec_byte();//0xff;
			}

			if (vicRC==7)
			{
				if (vic_badline==0) 
				{
					vicIDLE=1;
				}

				vicVCBASE = vicVC;
			}

			if (vicIDLE==0)
			{
				vicRC=(vicRC +1) & 7;
			}

			break;
		case 59:
			DrawForeground0(vicXSCROLL, vicLastCData, vicECM_BMM_MCM, cycle);
			if (vicSpriteArmedOrActive != 0)
			{
				DrawSprites(cyclePrev);
			}

			DRAW_BORDER(cycle);
			COLOR_FOREGROUND(vicBackgroundColor, cycle);
			SetBA(clocks, cycle);
			if (vicSpriteDMA & (1<<sprite0))
			{
				LOADSPRITEDATA(sprite0, 1) = vic_ph1_read_byte(vicSpritePointer[sprite0] + MC[sprite0]);
				MC[sprite0] = (MC[sprite0] + 1) & 63;
				LOADSPRITEDATA(sprite0, 0) = vic_ph2_read_byte(vicSpritePointer[sprite0] + MC[sprite0]);
				MC[sprite0] = (MC[sprite0] + 1) & 63;
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
			{
				DrawSprites(cyclePrev);
			}

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
			{
				LOADSPRITEDATA(sprite1, 2) = vic_ph2_read_3fff_aec_byte();//0xff;
			}

			break;
		case 61:
			DrawForeground0(vicXSCROLL, vicLastCData, vicECM_BMM_MCM, cycle);
			if (vicSpriteArmedOrActive != 0)
			{
				DrawSprites(cyclePrev);
			}

			DRAW_BORDER(cycle);
			COLOR_FOREGROUND(vicBackgroundColor, cycle);
			SetBA(clocks, cycle);
			if (vicSpriteDMA & (1<<sprite1))
			{
				LOADSPRITEDATA(sprite1, 1) = vic_ph1_read_byte(vicSpritePointer[sprite1] + MC[sprite1]);
				MC[sprite1] = (MC[sprite1] + 1) & 63;
				LOADSPRITEDATA(sprite1, 0) = vic_ph2_read_byte(vicSpritePointer[sprite1] + MC[sprite1]);
				MC[sprite1] = (MC[sprite1] + 1) & 63;
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
			{
				DrawSprites(cyclePrev);
			}

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
			{
				LOADSPRITEDATA(sprite2, 2) = vic_ph2_read_3fff_aec_byte();//0xff;
			}

			break;
		case 63: 
			if (vicSpriteArmedOrActive != 0)
			{
				DrawSprites(cyclePrev);
			}

			SetBA(clocks, cycle);
			if (vicSpriteDMA & (1<<sprite2))
			{
				LOADSPRITEDATA(sprite2, 1) = vic_ph1_read_byte(vicSpritePointer[sprite2] + MC[sprite2]);
				MC[sprite2] = (MC[sprite2] + 1) & 63;
				LOADSPRITEDATA(sprite2, 0) = vic_ph2_read_byte(vicSpritePointer[sprite2] + MC[sprite2]);
				MC[sprite2] = (MC[sprite2] + 1) & 63;
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

		cart->ExecuteCycle(CurrentClock);
	}
}

//pragma optimize( "g", on )

void VIC6569::WriteAccessToDebugger(bit16 line, bit8 cycle, bit16 pc, bit8 dataByte)
{
#ifdef DEBUG
	TCHAR sDebug[50];
	_stprintf_s(sDebug, _countof(sDebug), TEXT("%08X %04X %02X %04X %02X"), this->frameNumber, line, cycle, pc, dataByte);
	OutputDebugString(sDebug);
	OutputDebugString(TEXT("\n"));
#endif
}

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
		data = ((bit8)vicINTERRUPT_STATUS  & 0xF) | 0x70 | ((bit8)cpu->Get_IRQ_VIC() & 1)<<7;
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
		t = (bit8)vicSpriteSpriteCollision;
		vicSpriteSpriteInt = 1;
		SpriteCollisionUpdateArmedOrActive();
		clockReadSpriteSpriteCollision = cpu->CurrentClock + 1;
		data = t;		
		//WriteAccessToDebugger((bit16)this->vic_raster_line, this->vic_raster_cycle, cpu->m_CurrentOpcodeAddress.word, data);
		break;
	case 0x1f:	//sprite-data collision
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
	if (!vic_check_irq_in_cycle2)
	{
		if (cycle != PAL_CLOCKS_PER_LINE)
		{
			if (vic_raster_line == vicRASTER_compare)
			{
				vicINTERRUPT_STATUS |= 0x1; // Raster Interrupt Flag
				if ((vicINTERRUPT_STATUS & vicINTERRUPT_ENABLE & 0x1)!=0)
				{
					//SetSystemInterrupt();
                    //vandalismnews64 wants a 1 clock delay.
                    //passes the rastercompareirq beta test.
                    cpu->Set_VIC_IRQ(CurrentClock + 1);
				}
				bVicRasterMatch = true;
			}
			else 
            {
				bVicRasterMatch = false;
            }
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
					//SetSystemInterrupt();
                    //vandalismnews64 wants a 1 clock delay.
                    cpu->Set_VIC_IRQ(CurrentClock + 1);
				}
			}
			else 
            {
				bVicRasterMatch = false;
            }
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
		{
			SpriteXChange(0, vicSpriteX[0] + (((bit16)(data & 1)) << 8), cycle);
		}

		if ((vicSpriteMSBX ^ data) & 2)
		{
			SpriteXChange(1, vicSpriteX[1] + (((bit16)(data & 2)) << 7), cycle);
		}

		if ((vicSpriteMSBX ^ data) & 4)
		{
			SpriteXChange(2, vicSpriteX[2] + (((bit16)(data & 4)) << 6), cycle);
		}

		if ((vicSpriteMSBX ^ data) & 8)
		{
			SpriteXChange(3, vicSpriteX[3] + (((bit16)(data & 8)) << 5), cycle);
		}

		if ((vicSpriteMSBX ^ data) & 16)
		{
			SpriteXChange(4, vicSpriteX[4] + (((bit16)(data & 16)) << 4), cycle);
		}

		if ((vicSpriteMSBX ^ data) & 32)
		{
			SpriteXChange(5, vicSpriteX[5] + (((bit16)(data & 32)) << 3), cycle);
		}

		if ((vicSpriteMSBX ^ data) & 64)
		{
			SpriteXChange(6, vicSpriteX[6] + (((bit16)(data & 64)) << 2), cycle);
		}

		if ((vicSpriteMSBX ^ data) & 128)
		{
			SpriteXChange(7, vicSpriteX[7] + (((bit16)(data & 128)) << 1), cycle);
		}

		vicSpriteMSBX = data;
		break;
	case 0x11:	//control 1
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

		if (vic_raster_line == vic_top_compare && cycle!=PAL_CLOCKS_PER_LINE)
		{
			//The game Elven Warrior used to require this to display properly?!
			if (bRSELChanging && vicDEN!=0)
			{
				vicVerticalBorder=0;
				vicCharDataOutputDisabled=0;
			}
		}

		if (vic_raster_line == vic_bottom_compare && cycle!=PAL_CLOCKS_PER_LINE)
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
		{
			vic_latchDEN|=vicDEN;
		}

		if (vic_raster_line>=0x30 && vic_raster_line<=0xf7 && ((vic_raster_line & 7)==vicYSCROLL) && vic_latchDEN!=0 && cycle !=PAL_CLOCKS_PER_LINE)
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
				vic_allow_c_access = true;
			}

			vicCDataCarry=0;

			// Reads from the system 1 clock too soon. This is fine for reading RAM or ROM.
			cpu_next_op_code = cpu->MonReadByte(cpu->GetPC(), -1);			
			vicIDLE=0;
			vic_address_line_info = &BA_line_info[vicSpriteDMA][1];
		}
		else
		{
			vic_badline = 0;
			vic_allow_c_access = false;
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
			CheckRasterCompare(cycle);
		}		
		break;
	case 0x13:	//vic_lpx
		break;
	case 0x14:	//vic_lpy
		break;
	case 0x15:	//sprite enable
		vicSpriteEnable = data;
		break;
	case 0x16:	//control 2
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

			signed char xScrollDecrement = 0;				
			if (oldXScroll > newXScroll)
			{
				xScrollDecrement = oldXScroll - newXScroll;
			}

			if (modeNew != modeOld || xScrollDecrement >= 4)
			{
				bit8 t;

				bit8 gData2 = vicLastGData;
				bit8 gData1 = vicLastGDataPrev;
				bit8 gData0 = vicLastGDataPrev2;

				bit8 gData1Transformed = gData1;
				bit8 gData0Transformed = gData0;

				bool bBMM = (modeOld & 2) != 0;
				bool bECM = (modeOld & 4) != 0;

				//We may have already drawn part of the next cycle with an old and incorrect modeOld so we make a correction
				DrawForegroundEx(gData1, 0, cData1, modeNew, 0, vicCharDataOutputDisabled, 8 - oldXScroll, oldXScroll, cycle + 1);

				if ((modeNew & 1) == 0)
				{
					// Multicolor going off or staying off.
					// MCM: 1 -> 0
					// MCM: 0 -> 0
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

					if (modeNew != modeOld)
					{
						//MCM: 1 -> 0
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
				else if ((modeOld & 1) == 0)
				{
					//Multicolor going on
					//MCM:  0 -> 1
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
						{
							cData1b = cData1;
						}
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
						{
							cData1b = cData1;
						}
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
				else if (xScrollDecrement >= 4)
				{
					//Multicolor staying on
					//MCM:  1 -> 1
					bit8 gData1b;
					bit8 gData1c;
					bit32 cData1b;
					bit8 decShiftPixelShiftMode = 0;
					gData1b = gData1;
					gData1c = gData1;					

					if (bBMM)
					{
						//BMM==1
						//Early c-data colour fetch
						cData1b = cData2 & 0xfff;
					}
					else
					{
						//BMM==0
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
											
					switch (oldXScroll)
					{
					case 4:
						DrawForegroundEx(gData1b, +4, cData1b, modeNew, decShiftPixelShiftMode, vicCharDataOutputDisabled, 0, 4, cycle - 0);
						break;
					case 5:
						DrawForegroundEx(gData1b, +4 +1, cData1b, modeNew, decShiftPixelShiftMode, vicCharDataOutputDisabled, 0, 4, cycle - 0);
						break;
					case 6:
						DrawForegroundEx(gData1b, +4 +2, cData1b, modeNew, decShiftPixelShiftMode, vicCharDataOutputDisabled, 0, 4, cycle - 0);
						break;
					case 7:
						DrawForegroundEx(gData1b, +4 +3, cData1b, modeNew, decShiftPixelShiftMode, vicCharDataOutputDisabled, 0, 4, cycle - 0);
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
		if (cycle == 15)
		{
			vicClearingYExpandRegInClock15 = ~(data | ff_YP);
		}		
		if (cycle == 55)
		{
			//This fix was added to make the "Booze Design-Starion" demo work.
			//Clear the Y expansion flip-flop if Y expansion register bit transitions on (0 -> 1) in cycle 55.	
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
        {
			ClearSystemInterrupt();
        }
		break;
	case 0x1a:	//interrupt enable
		vicINTERRUPT_ENABLE = data & 0xF;
		if ((vicINTERRUPT_STATUS & vicINTERRUPT_ENABLE & 0xF)!=0)
		{
			SetSystemInterrupt();
		}
		else
        {
			ClearSystemInterrupt();
        }
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
void VIC6569::render(unsigned char *pRow, int xpos, int ypos, int width, int height, bit8 pPixelBufferCurrentFrame[], bit8 pPixelBufferPreviousFrame[], int startx, int videoPitch, int bufferPitch, bit32 firstVicRasterLine)
{
	render_32bit(pRow, xpos, ypos, width, height, pPixelBufferCurrentFrame, pPixelBufferPreviousFrame, startx, videoPitch, bufferPitch, firstVicRasterLine);
}

void VIC6569::render_32bit(unsigned char *pRow, int xpos, int ypos, int width, int height, bit8 pPixelBufferCurrentFrame[], bit8 pPixelBufferPreviousFrame[], int startx, int videoPitch, int bufferPitch, bit32 firstVicRasterLine)
{
bit8 *pPixelBuffer;
int maxCurrentFrameLine = vic_raster_line - firstVicRasterLine;
bit32 *p = (bit32 *)(pRow + (INT_PTR)(ypos * videoPitch + xpos * 4));
int i,j;
bit8 v;
int h;

	for (h = 0; h < height; h++)
	{
		if (h >= maxCurrentFrameLine)
		{
			pPixelBuffer = pPixelBufferPreviousFrame;
		}
		else
		{
			pPixelBuffer = pPixelBufferCurrentFrame;
		}

		for(i=0,j = DISPLAY_START+startx; i < (int)width ; i++,j++)
		{
			v = pPixelBuffer[j];
			p[i] = vic_color_array32[v];
		}

		p = (bit32 *)((INT_PTR)p + videoPitch);
		pPixelBufferCurrentFrame += bufferPitch;
		pPixelBufferPreviousFrame += bufferPitch;
	}
}

unsigned int VIC6569::GetFrameCounter()
{
	return this->frameNumber;
}

bit16 VIC6569::GetCurrentRasterLine()
{
	return vic_raster_line;
}

bit8 VIC6569::GetCurrentRasterCycle()
{
	return vic_raster_cycle;
}

bit16 VIC6569::GetNextRasterLine()
{
	if (vic_check_irq_in_cycle2)
	{
		return 0;
	}
	else 
	{
		if (vic_raster_cycle < PAL_CLOCKS_PER_LINE)
			return (bit16)vic_raster_line;
		else if (vic_raster_line < PAL_MAX_LINE)
			return (bit16)(vic_raster_line + 1);
		else
			return 0;
	}
}

bit8 VIC6569::GetNextRasterCycle()
{
	return (bit8)((vic_raster_cycle < PAL_CLOCKS_PER_LINE) ? vic_raster_cycle + 1 : 1);
}

bool VIC6569::SetBreakpointRasterCompare(int line, int cycle, bool enabled, int initialSkipOnHitCount, int currentSkipOnHitCount)
{
	BreakpointItem bp(DBGSYM::MachineIdent::Vic, DBGSYM::BreakpointType::VicRasterCompare, 0, enabled, initialSkipOnHitCount, currentSkipOnHitCount);
	bp.vic_line = line;
	bp.vic_cycle = cycle;
	return this->m_pIBreakpointManager->BM_SetBreakpoint(bp);
}

bool VIC6569::GetBreakpointRasterCompare(int line, int cycle, BreakpointItem& breakpoint)
{
	BreakpointItem key(DBGSYM::MachineIdent::Vic, DBGSYM::BreakpointType::VicRasterCompare, 0);
	key.vic_line = line;
	key.vic_cycle = cycle;
	return this->m_pIBreakpointManager->BM_GetBreakpoint(key, breakpoint);
}

int VIC6569::CheckBreakpointRasterCompare(int line, int cycle, bool bHitIt)
{
int i = -1;

	BreakpointItem bp;
	if (GetBreakpointRasterCompare(line, cycle, bp))
	{
		if (bp.enabled)
		{
			i = bp.currentSkipOnHitCount;
			if (bHitIt && i == 0)
			{
				bp.currentSkipOnHitCount = bp.initialSkipOnHitCount;
				this->m_pIBreakpointManager->BM_SetBreakpoint(bp);
			}
		}			
	}

	return i;
}

bool VIC6569::Get_BA()
{
	return vicBA;
}

ICLK VIC6569::Get_ClockBALow()
{
	return clockBALow;
}

ICLK VIC6569::Get_ClockBAHigh()
{
	return clockBAHigh;
}

ICLK VIC6569::Get_CountBALow()
{
	if (vicBA != 0)
	{
		return 0;
	}
	else
	{
		return CurrentClock - clockBALow + 1;
	}
}

ICLK VIC6569::Get_CountBAHigh()
{
	if (vicBA == 0)
	{
		return 0;
	}
	else
	{
		return CurrentClock - clockBAHigh + 1;
	}
}

bit8 VIC6569::GetDExxByte(ICLK sysclock)
{
	ExecuteCycle(sysclock);
	return de00_byte;
}

bit8 VIC6569::SpriteDMATurningOn()
{
	return vicSpriteDMA & ~vicSpriteDMAPrev;
}


void VIC6569::GetState(SsVic6569V1 &state)
{
	ZeroMemory(&state, sizeof(state));
	state.Version = 1;
	state.CurrentClock = CurrentClock;
	state.cpu_next_op_code = cpu_next_op_code;
	state.vicECM = vicECM;
	state.vicBMM = vicBMM;
	state.vicDEN = vicDEN;
	state.vicRSEL = vicRSEL;
	state.vicYSCROLL = vicYSCROLL;
	state.vicECM_BMM_MCM = vicECM_BMM_MCM;
	state.vicECM_BMM_MCM_prev = vicECM_BMM_MCM_prev;
	state.vicRES = vicRES;
	state.vicMCM = vicMCM;
	state.vicCSEL = vicCSEL;
	state.vicXSCROLL = vicXSCROLL;
	state.vicXSCROLL_Cycle57 = vicXSCROLL_Cycle57;
	state.vicMemptrVM = vicMemptrVM;
	state.vicMemptrCB = vicMemptrCB;
	state.vicBorderColor = vicBorderColor;	
	state.vicSaveBackgroundColor0 = vicSaveBackgroundColor0;
	state.vicRASTER_compare = vicRASTER_compare;
	state.vic_raster_cycle = vic_raster_cycle;
	state.vic_raster_line = vic_raster_line;
	state.vicINTERRUPT_STATUS = vicINTERRUPT_STATUS;
	state.vicINTERRUPT_ENABLE = vicINTERRUPT_ENABLE;
	state.vic_check_irq_in_cycle2 = vic_check_irq_in_cycle2;
	state.vicSpriteArmedOrActive = vicSpriteArmedOrActive;
	state.ff_YP = ff_YP;
	state.vicSpriteDMA = vicSpriteDMA;
	state.vicSpriteDMAPrev = vicSpriteDMAPrev;
	state.vicSpriteDisplay = vicSpriteDisplay;
	state.vicSpriteYMatch = vicSpriteYMatch;
	state.vicClearingYExpandRegInClock15 = vicClearingYExpandRegInClock15;
	state.vicSpriteMSBX = vicSpriteMSBX;
	state.vicSpriteEnable = vicSpriteEnable;
	state.vicSpriteYExpand = vicSpriteYExpand;
	state.vicSpriteXExpand = vicSpriteXExpand;
	state.vicSpriteXExpandPrev = vicSpriteXExpandPrev;
	state.vicSpriteDataPriority = vicSpriteDataPriority;
	state.vicSpriteDataPriorityPrev = vicSpriteDataPriorityPrev;
	state.vicSpriteMultiColor = vicSpriteMultiColor;
	state.vicSpriteMultiColorPrev = vicSpriteMultiColorPrev;
	state.vicSpriteSpriteCollision = vicSpriteSpriteCollision;
	state.vicSpriteDataCollision = vicSpriteDataCollision;
	state.vicNextSprite_sprite_collision = vicNextSprite_sprite_collision;
	state.vicNextSprite_data_collision = vicNextSprite_data_collision;
	state.vicCurrSprite_sprite_collision = vicCurrSprite_sprite_collision;
	state.vicCurrSprite_data_collision = vicCurrSprite_data_collision;
	state.vicSpriteSpriteInt = vicSpriteSpriteInt;
	state.vicSpriteDataInt = vicSpriteDataInt;
	state.clockSpriteMultiColorChange = clockSpriteMultiColorChange;
	state.clockSpriteDataPriorityChange = clockSpriteDataPriorityChange;
	state.clockSpriteXExpandChange = clockSpriteXExpandChange;
	state.clockReadSpriteDataCollision = clockReadSpriteDataCollision;
	state.clockReadSpriteSpriteCollision = clockReadSpriteSpriteCollision;
	state.clockFirstForcedBadlineCData = clockFirstForcedBadlineCData;
	state.vicVC = vicVC;
	state.vicRC = vicRC;
	state.vicVCBASE = vicVCBASE;
	state.vicVMLI = vicVMLI;
	state.vicAEC = vicAEC;
	state.vicBA = vicBA;
	state.vicIDLE = vicIDLE;
	state.vicIDLE_DELAY = vicIDLE_DELAY;
	state.vicLastCData = vicLastCData;
	state.vicLastCDataPrev = vicLastCDataPrev;
	state.vicLastCDataPrev2 = vicLastCDataPrev2;
	state.vicCDataCarry = vicCDataCarry;
	state.vicLastGData = vicLastGData;
	state.vicLastGDataPrev = vicLastGDataPrev;
	state.vicLastGDataPrev2 = vicLastGDataPrev2;
	state.vic_latchDEN = vic_latchDEN;
	state.vic_lpx = vic_lpx;
	state.vic_lpy = vic_lpy;
	state.vicDRAMRefresh = vicDRAMRefresh;
	state.LP_TRIGGER = LP_TRIGGER;
	state.vicLightPen = vicLightPen;
	state.m_bVicBankChanging = m_bVicBankChanging;
	state.vicBankChangeByte = vicBankChangeByte;
	state.de00_byte = de00_byte;
	state.frameNumber = frameNumber;
	state.bVicRasterMatch = bVicRasterMatch;
	state.vic_in_display_y = vic_in_display_y;
	state.vic_badline = vic_badline;
	state.vicBA_new = vicBA_new;
	state.vic_top_compare = vic_top_compare;
	state.vic_bottom_compare = vic_bottom_compare;
	state.vic_left_compare = vic_left_compare;
	state.vic_right_compare = vic_right_compare;
	state.vicMainBorder = vicMainBorder;
	state.vicVerticalBorder = vicVerticalBorder; 
	state.vicCharDataOutputDisabled = vicCharDataOutputDisabled; 
	state.vicMainBorder_old = vicMainBorder_old;
	state.vic_border_part_38 = vic_border_part_38;
	state.vic_border_part_40 = vic_border_part_40;
	state.vic_allow_c_access = vic_allow_c_access;
	state.m_bVicModeChanging = m_bVicModeChanging;
	state.vicMemoryBankIndex = vicMemoryBankIndex;
	state.lastBackedUpFrameNumber = lastBackedUpFrameNumber;

	for (int i = 0; i < _countof(vicSprite); i++)
	{
		 vicSprite[i].GetState(state.vicSprite[i]);
	}
	memcpy_s(state.vicBackgroundColor, sizeof(state.vicBackgroundColor), vicBackgroundColor, sizeof(vicBackgroundColor));
	memcpy_s(state.vicSpritePointer, sizeof(state.vicSpritePointer), vicSpritePointer, sizeof(vicSpritePointer));
	memcpy_s(state.vicSpriteData, sizeof(state.vicSpriteData), vicSpriteData, sizeof(vicSpriteData));	
	memcpy_s(state.vicSpriteX, sizeof(state.vicSpriteX), vicSpriteX, sizeof(vicSpriteX));
	memcpy_s(state.vicSpriteY, sizeof(state.vicSpriteY), vicSpriteY, sizeof(vicSpriteY));
	memcpy_s(state.MC, sizeof(state.MC), MC, sizeof(MC));
	memcpy_s(state.MCBASE, sizeof(state.MCBASE), MCBASE, sizeof(MCBASE));
	memcpy_s(state.ScreenPixelBuffer, sizeof(state.ScreenPixelBuffer), ScreenPixelBuffer, sizeof(ScreenPixelBuffer));
	memcpy_s(state.LinePixelBuffer, sizeof(state.LinePixelBuffer), LinePixelBuffer, sizeof(LinePixelBuffer));
	memcpy_s(state.pixelMaskBuffer, sizeof(state.pixelMaskBuffer), pixelMaskBuffer, sizeof(pixelMaskBuffer));
	memcpy_s(state.vic_sprite_collision_line, sizeof(state.vic_sprite_collision_line), vic_sprite_collision_line, sizeof(vic_sprite_collision_line));
	memcpy_s(state.VideoMatrix, sizeof(state.VideoMatrix), VideoMatrix, sizeof(VideoMatrix));
	state.clockBALow = clockBALow;
	state.clockBAHigh = clockBAHigh;
}

void VIC6569::SetState(const SsVic6569V1 &state)
{
	CurrentClock = state.CurrentClock;
	cpu_next_op_code = state.cpu_next_op_code;
	vicECM = state.vicECM;
	vicBMM = state.vicBMM;
	vicDEN = state.vicDEN;
	vicRSEL = state.vicRSEL;
	vicYSCROLL = state.vicYSCROLL;
	vicECM_BMM_MCM = state.vicECM_BMM_MCM;
	vicECM_BMM_MCM_prev = state.vicECM_BMM_MCM_prev;
	vicRES = state.vicRES;
	vicMCM = state.vicMCM;
	vicCSEL = state.vicCSEL;
	vicXSCROLL = state.vicXSCROLL;
	vicXSCROLL_Cycle57 = state.vicXSCROLL_Cycle57;
	vicMemptrVM = state.vicMemptrVM;
	vicMemptrCB = state.vicMemptrCB;
	vicBorderColor = state.vicBorderColor;	
	vicSaveBackgroundColor0 = state.vicSaveBackgroundColor0;
	vicRASTER_compare = state.vicRASTER_compare;
	vic_raster_cycle = state.vic_raster_cycle;
	vic_raster_line = state.vic_raster_line;
	vicINTERRUPT_STATUS = state.vicINTERRUPT_STATUS;
	vicINTERRUPT_ENABLE = state.vicINTERRUPT_ENABLE;
	vic_check_irq_in_cycle2 = state.vic_check_irq_in_cycle2 != 0;
	vicSpriteArmedOrActive = state.vicSpriteArmedOrActive;
	ff_YP = state.ff_YP;
	vicSpriteDMA = state.vicSpriteDMA;
	vicSpriteDMAPrev = state.vicSpriteDMAPrev;
	vicSpriteDisplay = state.vicSpriteDisplay;
	vicSpriteYMatch = state.vicSpriteYMatch;
	vicClearingYExpandRegInClock15 = state.vicClearingYExpandRegInClock15;
	vicSpriteMSBX = state.vicSpriteMSBX;
	vicSpriteEnable = state.vicSpriteEnable;
	vicSpriteYExpand = state.vicSpriteYExpand;
	vicSpriteXExpand = state.vicSpriteXExpand;
	vicSpriteXExpandPrev = state.vicSpriteXExpandPrev;
	vicSpriteDataPriority = state.vicSpriteDataPriority;
	vicSpriteDataPriorityPrev = state.vicSpriteDataPriorityPrev;
	vicSpriteMultiColor = state.vicSpriteMultiColor;
	vicSpriteMultiColorPrev = state.vicSpriteMultiColorPrev;
	vicSpriteSpriteCollision = state.vicSpriteSpriteCollision;
	vicSpriteDataCollision = state.vicSpriteDataCollision;
	vicNextSprite_sprite_collision = state.vicNextSprite_sprite_collision;
	vicNextSprite_data_collision = state.vicNextSprite_data_collision;
	vicCurrSprite_sprite_collision = state.vicCurrSprite_sprite_collision;
	vicCurrSprite_data_collision = state.vicCurrSprite_data_collision;
	vicSpriteSpriteInt = state.vicSpriteSpriteInt;
	vicSpriteDataInt = state.vicSpriteDataInt;
	clockSpriteMultiColorChange = state.clockSpriteMultiColorChange;
	clockSpriteDataPriorityChange = state.clockSpriteDataPriorityChange;
	clockSpriteXExpandChange = state.clockSpriteXExpandChange;
	clockReadSpriteDataCollision = state.clockReadSpriteDataCollision;
	clockReadSpriteSpriteCollision = state.clockReadSpriteSpriteCollision;
	clockFirstForcedBadlineCData = state.clockFirstForcedBadlineCData;
	vicVC = state.vicVC;
	vicRC = state.vicRC;
	vicVCBASE = state.vicVCBASE;
	vicVMLI = state.vicVMLI <= NUM_SCREEN_COLUMNS ? state.vicVMLI : NUM_SCREEN_COLUMNS;
	vicAEC = state.vicAEC;
	vicBA = state.vicBA;
	clockBALow = state.clockBALow;
	vicIDLE = state.vicIDLE;
	vicIDLE_DELAY = state.vicIDLE_DELAY;
	vicLastCData = state.vicLastCData;
	vicLastCDataPrev = state.vicLastCDataPrev;
	vicLastCDataPrev2 = state.vicLastCDataPrev2;
	vicCDataCarry = state.vicCDataCarry;
	vicLastGData = state.vicLastGData;
	vicLastGDataPrev = state.vicLastGDataPrev;
	vicLastGDataPrev2 = state.vicLastGDataPrev2;
	vic_latchDEN = state.vic_latchDEN;
	vic_lpx = state.vic_lpx;
	vic_lpy = state.vic_lpy;
	vicDRAMRefresh = state.vicDRAMRefresh;
	LP_TRIGGER = state.LP_TRIGGER;
	vicLightPen = state.vicLightPen;
	m_bVicBankChanging = state.m_bVicBankChanging != 0;
	vicBankChangeByte = state.vicBankChangeByte;
	de00_byte = state.de00_byte;
	frameNumber = state.frameNumber;
	bVicRasterMatch = state.bVicRasterMatch != 0;
	vic_in_display_y = state.vic_in_display_y;
	vic_badline = state.vic_badline;
	vicBA_new = state.vicBA_new;
	vic_top_compare = state.vic_top_compare;
	vic_bottom_compare = state.vic_bottom_compare;
	vic_left_compare = state.vic_left_compare;
	vic_right_compare = state.vic_right_compare;
	vicMainBorder = state.vicMainBorder;
	vicVerticalBorder = state.vicVerticalBorder; 
	vicCharDataOutputDisabled = state.vicCharDataOutputDisabled; 
	vicMainBorder_old = state.vicMainBorder_old;
	vic_border_part_38 = state.vic_border_part_38;
	vic_border_part_40 = state.vic_border_part_40;
	vic_allow_c_access = state.vic_allow_c_access != 0;
	m_bVicModeChanging = state.m_bVicModeChanging != 0;
	vicMemoryBankIndex = state.vicMemoryBankIndex & 3;
	lastBackedUpFrameNumber = state.lastBackedUpFrameNumber;

	for (int i = 0; i < _countof(vicSprite); i++)
	{
		 vicSprite[i].SetState(state.vicSprite[i]);
	}
	memcpy_s(vicBackgroundColor, sizeof(vicBackgroundColor), state.vicBackgroundColor, sizeof(state.vicBackgroundColor));
	memcpy_s(vicSpritePointer, sizeof(vicSpritePointer), state.vicSpritePointer, sizeof(state.vicSpritePointer));
	memcpy_s(vicSpriteData, sizeof(vicSpriteData), state.vicSpriteData, sizeof(state.vicSpriteData));	
	memcpy_s(vicSpriteX, sizeof(vicSpriteX), state.vicSpriteX, sizeof(state.vicSpriteX));
	memcpy_s(vicSpriteY, sizeof(vicSpriteY), state.vicSpriteY, sizeof(state.vicSpriteY));
	memcpy_s(MC, sizeof(MC), state.MC, sizeof(state.MC));
	memcpy_s(MCBASE, sizeof(MCBASE), state.MCBASE, sizeof(state.MCBASE));
	memcpy_s(ScreenPixelBuffer, sizeof(ScreenPixelBuffer), state.ScreenPixelBuffer, sizeof(state.ScreenPixelBuffer));
	memcpy_s(LinePixelBuffer, sizeof(LinePixelBuffer), state.LinePixelBuffer, sizeof(state.LinePixelBuffer));
	memcpy_s(pixelMaskBuffer, sizeof(pixelMaskBuffer), state.pixelMaskBuffer, sizeof(state.pixelMaskBuffer));
	memcpy_s(vic_sprite_collision_line, sizeof(vic_sprite_collision_line), state.vic_sprite_collision_line, sizeof(state.vic_sprite_collision_line));
	memcpy_s(VideoMatrix, sizeof(VideoMatrix), state.VideoMatrix, sizeof(state.VideoMatrix));
	clockBALow = state.clockBALow;
	clockBAHigh = state.clockBAHigh;
}


void VIC6569::UpgradeStateV0ToV1(const SsVic6569V0& in, SsVic6569V1& out)
{
	ZeroMemory(&out, sizeof(SsVic6569V1));
	*((SsVic6569V0*)&out) = in;
	out.Version = 0;// Let C64::LoadC64StateFromFile know that vicBA needs fixing up to match the CPU RDY signal.
	out.vicBA = 1; // The BA signal is now separate from RDY and will need a fix up.
	out.clockBALow = 0;
	out.clockBAHigh = 0;
}
