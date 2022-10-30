#pragma once
template<class T>
class HexConv
{
public:
	static unsigned char hextable[256];
	static unsigned long hex_to_long(T *buffer, size_t cchBuffer);
	static unsigned long long_to_hex(unsigned long number,T *buffer,unsigned long digits);
private:
	static void init_hex_table();
};

//emplate class HexConv<char>;
//emplate class HexConv<wchar_t>;
