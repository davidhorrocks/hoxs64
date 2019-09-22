#include "windows.h"
#include "tchar.h"
#include "c64Keys.h"


C64Keys::KeyRC C64Keys::KeyRowCol[C64K_COUNTOFKEYS];

LPCTSTR C64Keys::GetName(C64Key c64keynumber)
{
	switch (c64keynumber)
	{
	case C64K_NONE:
		return TEXT("");
	case C64K_0:
		return TEXT("0");
	case C64K_1:
		return TEXT("1");
	case C64K_2:
		return TEXT("2");
	case C64K_3:
		return TEXT("3");
	case C64K_4:
		return TEXT("4");
	case C64K_5:
		return TEXT("5");
	case C64K_6:
		return TEXT("6");
	case C64K_7:
		return TEXT("7");
	case C64K_8:
		return TEXT("8");
	case C64K_9:
		return TEXT("9");
	case C64K_A:
		return TEXT("A");
	case C64K_B:
		return TEXT("B");
	case C64K_C:
		return TEXT("C");
	case C64K_D:
		return TEXT("D");
	case C64K_E:
		return TEXT("E");
	case C64K_F:
		return TEXT("F");
	case C64K_G:
		return TEXT("G");
	case C64K_H:
		return TEXT("H");
	case C64K_I:
		return TEXT("I");
	case C64K_J:
		return TEXT("J");
	case C64K_K:
		return TEXT("K");
	case C64K_L:
		return TEXT("L");
	case C64K_M:
		return TEXT("M");
	case C64K_N:
		return TEXT("N");
	case C64K_O:
		return TEXT("O");
	case C64K_P:
		return TEXT("P");
	case C64K_Q:
		return TEXT("Q");
	case C64K_R:
		return TEXT("R");
	case C64K_S:
		return TEXT("S");
	case C64K_T:
		return TEXT("T");
	case C64K_U:
		return TEXT("U");
	case C64K_V:
		return TEXT("V");
	case C64K_W:
		return TEXT("W");
	case C64K_X:
		return TEXT("X");
	case C64K_Y:
		return TEXT("Y");
	case C64K_Z:
		return TEXT("Z");
	case C64K_PLUS:
		return TEXT("+");
	case C64K_MINUS:
		return TEXT("-");
	case C64K_ASTERISK:
		return TEXT("*");
	case C64K_SLASH:
		return TEXT("/");
	case C64K_COMMA:
		return TEXT(",");
	case C64K_DOT:
		return TEXT(".");
	case C64K_ARROWLEFT:
		return TEXT("<-");
	case C64K_COLON:
		return TEXT(":");
	case C64K_SEMICOLON:
		return TEXT(";");
	case C64K_CONTROL:
		return TEXT("Control");
	case C64K_STOP:
		return TEXT("Stop");
	case C64K_COMMODORE:
		return TEXT("Commodore");
	case C64K_LEFTSHIFT:
		return TEXT("Left Shift");
	case C64K_RIGHTSHIFT:
		return TEXT("Right Shift");
	case C64K_RESTORE:
		return TEXT("Restore");
	case C64K_HOME:
		return TEXT("Home");
	case C64K_DEL:
		return TEXT("Del");
	case C64K_RETURN:
		return TEXT("Return");
	case C64K_ARROWUP:
		return TEXT("^");
	case C64K_POUND:
		return TEXT("£");
	case C64K_EQUAL:
		return TEXT("=");
	case C64K_CURSORDOWN:
		return TEXT("Cursor Down");
	case C64K_CURSORRIGHT:
		return TEXT("Cursor Right");
	case C64K_SPACE:
		return TEXT("Space");
	case C64K_AT:
		return TEXT("@");
	case C64K_F1:
		return TEXT("F1");
	case C64K_F2:
		return TEXT("F2");
	case C64K_F3:
		return TEXT("F3");
	case C64K_F4:
		return TEXT("F4");
	case C64K_F5:
		return TEXT("F5");
	case C64K_F6:
		return TEXT("F6");
	case C64K_F7:
		return TEXT("F7");
	case C64K_F8:
		return TEXT("F8");
	case C64K_CURSORUP:
		return TEXT("Cursor Up");
	case C64K_CURSORLEFT:
		return TEXT("Cursor Left");
	case C64K_JOY1FIRE:
		return TEXT("Joy1 Fire");
	case C64K_JOY1UP:
		return TEXT("Joy1 Up");
	case C64K_JOY1DOWN:
		return TEXT("Joy1 Down");
	case C64K_JOY1LEFT:
		return TEXT("Joy1 Left");
	case C64K_JOY1RIGHT:
		return TEXT("Joy1 Right");
	case C64K_JOY2FIRE:
		return TEXT("Joy2 Fire");
	case C64K_JOY2UP:
		return TEXT("Joy2 Up");
	case C64K_JOY2DOWN:
		return TEXT("Joy2 Down");
	case C64K_JOY2LEFT:
		return TEXT("Joy2 Left");
	case C64K_JOY2RIGHT:
		return TEXT("Joy2 Right");
	case C64K_JOY1FIRE2:
		return TEXT("Joy1 Fire 2");
	case C64K_JOY2FIRE2:
		return TEXT("Joy2 Fire 2");
	default:
		return TEXT("?");
	}
}


void C64Keys::setkeyrc(C64Key key, unsigned char row, unsigned char col)
{
	KeyRowCol[key].row = row;
	KeyRowCol[key].col = col;
	KeyRowCol[key].colmask = (unsigned char)~(1<<row);
	KeyRowCol[key].rowmask = (unsigned char)~(1<<col);
}

void C64Keys::Init()
{
unsigned int i;
	for (i = 0 ; i < _countof(KeyRowCol); i++)
	{
		KeyRowCol[i].row = 0;
		KeyRowCol[i].col = 0;
		KeyRowCol[i].rowmask = 0xff;
		KeyRowCol[i].colmask = 0xff;
	}

	setkeyrc(C64Keys::C64K_COLON, 5, 5);
	setkeyrc(C64Keys::C64K_STOP, 7, 7);
	setkeyrc(C64Keys::C64K_1, 7, 0);
	setkeyrc(C64Keys::C64K_2, 7, 3);
	setkeyrc(C64Keys::C64K_3, 1, 0);
	setkeyrc(C64Keys::C64K_4, 1, 3);
	setkeyrc(C64Keys::C64K_5, 2, 0);
	setkeyrc(C64Keys::C64K_6, 2, 3);
	setkeyrc(C64Keys::C64K_7, 3, 0);
	setkeyrc(C64Keys::C64K_8, 3, 3);
	setkeyrc(C64Keys::C64K_9, 4, 0);
	setkeyrc(C64Keys::C64K_0, 4, 3);
	setkeyrc(C64Keys::C64K_MINUS, 5, 3);
	setkeyrc(C64Keys::C64K_PLUS, 5, 0);
	setkeyrc(C64Keys::C64K_EQUAL, 6, 5);
	setkeyrc(C64Keys::C64K_POUND, 6, 0);
	setkeyrc(C64Keys::C64K_DEL, 0, 0);
	setkeyrc(C64Keys::C64K_Q, 7, 6);
	setkeyrc(C64Keys::C64K_W, 1, 1);
	setkeyrc(C64Keys::C64K_E, 1, 6);
	setkeyrc(C64Keys::C64K_R, 2, 1);
	setkeyrc(C64Keys::C64K_T, 2, 6);
	setkeyrc(C64Keys::C64K_Y, 3, 1);
	setkeyrc(C64Keys::C64K_U, 3, 6);
	setkeyrc(C64Keys::C64K_I, 4, 1);
	setkeyrc(C64Keys::C64K_O, 4, 6);
	setkeyrc(C64Keys::C64K_P, 5, 1);
	setkeyrc(C64Keys::C64K_ASTERISK, 6, 1);
	setkeyrc(C64Keys::C64K_ARROWUP, 6, 6);
	setkeyrc(C64Keys::C64K_RETURN, 0, 1);
	setkeyrc(C64Keys::C64K_CONTROL, 7, 2);
	setkeyrc(C64Keys::C64K_A, 1, 2);
	setkeyrc(C64Keys::C64K_S, 1, 5);
	setkeyrc(C64Keys::C64K_D, 2, 2);
	setkeyrc(C64Keys::C64K_F, 2, 5);
	setkeyrc(C64Keys::C64K_G, 3, 2);
	setkeyrc(C64Keys::C64K_H, 3, 5);
	setkeyrc(C64Keys::C64K_J, 4, 2);
	setkeyrc(C64Keys::C64K_K, 4, 5);
	setkeyrc(C64Keys::C64K_L, 5, 2);
	setkeyrc(C64Keys::C64K_SEMICOLON, 6, 2);
	setkeyrc(C64Keys::C64K_AT, 5, 6);
	setkeyrc(C64Keys::C64K_ARROWLEFT, 7, 1);
	setkeyrc(C64Keys::C64K_LEFTSHIFT, 1, 7);
	setkeyrc(C64Keys::C64K_COMMODORE, 7, 5);
	setkeyrc(C64Keys::C64K_Z, 1, 4);
	setkeyrc(C64Keys::C64K_X, 2, 7);
	setkeyrc(C64Keys::C64K_C, 2, 4);
	setkeyrc(C64Keys::C64K_V, 3, 7);
	setkeyrc(C64Keys::C64K_B, 3, 4);
	setkeyrc(C64Keys::C64K_N, 4, 7);
	setkeyrc(C64Keys::C64K_M, 4, 4);
	setkeyrc(C64Keys::C64K_COMMA, 5, 7);
	setkeyrc(C64Keys::C64K_DOT, 5, 4);
	setkeyrc(C64Keys::C64K_SLASH, 6, 7);
	setkeyrc(C64Keys::C64K_RIGHTSHIFT, 6, 4);
	setkeyrc(C64Keys::C64K_SPACE, 7, 4);
	setkeyrc(C64Keys::C64K_F1, 0, 4);
	setkeyrc(C64Keys::C64K_F3, 0, 5);
	setkeyrc(C64Keys::C64K_F5, 0, 6);
	setkeyrc(C64Keys::C64K_F7, 0, 3);
	setkeyrc(C64Keys::C64K_HOME, 6, 3);
	setkeyrc(C64Keys::C64K_CURSORRIGHT, 0, 2);
	setkeyrc(C64Keys::C64K_CURSORDOWN, 0, 7);
}