#ifndef __HEXCONV_H__
#define __HEXCONV_H__

class HexConv
{
public:
	static unsigned char hextable[256];
	static unsigned int hex_to_long(TCHAR *buffer);
	static unsigned int long_to_hex(unsigned int number,TCHAR *buffer,unsigned int digits);
private:
	static void init_hex_table();
};

#endif