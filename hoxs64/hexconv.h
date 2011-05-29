#ifndef __HEXCONV_H__
#define __HEXCONV_H__

class HexConv
{
public:
	static unsigned char hextable[256];
	static bit32 hex_to_long(TCHAR *buffer);
	static bit32 long_to_hex(bit32 number,TCHAR *buffer,unsigned int digits);
private:
	static void init_hex_table();
};

#endif