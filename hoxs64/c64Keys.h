#pragma once
#include <windows.h>
#include <tchar.h>

class C64Keys
{
public:
	typedef enum tagC64Key : unsigned char
	{
		C64K_NONE = 0,
		C64K_0 = 1,
		C64K_1 = 2,
		C64K_2 = 3,
		C64K_3 = 4,
		C64K_4 = 5,
		C64K_5 = 6,
		C64K_6 = 7,
		C64K_7 = 8,
		C64K_8 = 9,
		C64K_9 = 10,
		C64K_A = 11,
		C64K_B = 12,
		C64K_C = 13,
		C64K_D = 14,
		C64K_E = 15,
		C64K_F = 16,
		C64K_G = 17,
		C64K_H = 18,
		C64K_I = 19,
		C64K_J = 20,
		C64K_K = 21,
		C64K_L = 22,
		C64K_M = 23,
		C64K_N = 24,
		C64K_O = 25,
		C64K_P = 26,
		C64K_Q = 27,
		C64K_R = 28,
		C64K_S = 29,
		C64K_T = 30,
		C64K_U = 31,
		C64K_V = 32,
		C64K_W = 33,
		C64K_X = 34,
		C64K_Y = 35,
		C64K_Z = 36,
		C64K_PLUS = 37,
		C64K_MINUS = 38,
		C64K_ASTERISK = 39,
		C64K_SLASH = 40,
		C64K_COMMA = 41,
		C64K_DOT = 42,
		C64K_ARROWLEFT = 43,
		C64K_COLON = 44,
		C64K_SEMICOLON = 45,
		C64K_CONTROL = 46,
		C64K_STOP = 47,
		C64K_COMMODORE = 48,
		C64K_LEFTSHIFT = 49,
		C64K_RIGHTSHIFT = 50,
		C64K_RESTORE = 51,
		C64K_HOME = 52,
		C64K_DEL = 53,
		C64K_RETURN = 54,
		C64K_ARROWUP = 55,
		C64K_POUND = 56,
		C64K_EQUAL = 57,
		C64K_CURSORDOWN = 58,
		C64K_CURSORRIGHT = 59,
		C64K_SPACE = 60,
		C64K_AT = 61,
		C64K_F1 = 62,
		C64K_F2 = 63,
		C64K_F3 = 64,
		C64K_F4 = 65,
		C64K_F5 = 66,
		C64K_F6 = 67,
		C64K_F7 = 68,
		C64K_F8 = 69,

		C64K_CURSORUP = 70,
		C64K_CURSORLEFT = 71,

		C64K_JOY1FIRE = 72,
		C64K_JOY1UP = 73,
		C64K_JOY1DOWN = 74,
		C64K_JOY1LEFT = 75,
		C64K_JOY1RIGHT = 76,

		C64K_JOY2FIRE = 77,
		C64K_JOY2UP = 78,
		C64K_JOY2DOWN = 79,
		C64K_JOY2LEFT = 80,
		C64K_JOY2RIGHT = 81,

		C64K_JOY1FIRE2 = 82,
		C64K_JOY2FIRE2 = 83,

		C64K_COUNTOFKEYS = 84
	} C64Key;

	struct KeyRC
	{
		unsigned char row;
		unsigned char col;
		unsigned char rowmask;
		unsigned char colmask;
	};

	static KeyRC KeyRowCol[C64K_COUNTOFKEYS];
	static LPCTSTR GetName(C64Key c64keynumber);
	static void Init() noexcept;
private:
	static  void setkeyrc(C64Key key, unsigned char row, unsigned char col) noexcept;
};