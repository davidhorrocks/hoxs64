#include <windows.h>
#include <tchar.h>
#include "hexconv.h"

template<class T>
unsigned char HexConv<T>::hextable[256] = {0};

template<class T>
void HexConv<T>::init_hex_table()
{
	unsigned int i;
	for (i = 0; i <= 0xff; i++)
	{
		if (i >= '0' && i <= '9')
		{
			hextable[i] = i - '0';
		}
		else if (i >= 'A' && i <= 'F')
		{
			hextable[i] = i - 'A' + 10;
		}
		else if (i >= 'a' && i <= 'f')
		{
			hextable[i] = i - 'a' + 10;
		}
		else
		{
			hextable[i] = 0xff;
		}
	}
}

template<class T>
unsigned long HexConv<T>::hex_to_long(T* buffer, size_t cchBuffer)
{
	unsigned long i, v, a, d;

	if (hextable[0] == 0)
	{
		init_hex_table();
	}

	a = 0;
	for (i = 0; i < 8 && i < cchBuffer; i++)
	{
		d = buffer[i];
		if (d > 0xff || d == 0)
		{
			return a;
		}

		v = hextable[d];
		if (v == 0xff)
		{
			return a;
		}

		a = a << 4;
		a = a | v;
	}

	return a;
}

template<class T>
unsigned long HexConv<T>::long_to_hex(unsigned long number, T* buffer, unsigned long digits)
{
	unsigned long lz, i, v, s;
	unsigned long mask;

	lz = 0;
	i = 0;
	if (digits >= 1 && digits <= 8)
	{
		mask = 0x0000000F;
		mask = mask << ((digits - 1) * 4);
		s = (digits - 1) * 4;
	}
	else
	{
		mask = 0xF0000000;
		s = 7 * 4;
	}

	while (mask)
	{
		v = number & mask;
		v = (v >> s) & 0xf;
		if (v <= 9)
		{
			buffer[i++] = (T)(v + '0');
		}
		else
		{
			buffer[i++] = (T)(v - 10 + 'A');
		}

		mask = mask >> 4;
		s = s - 4;
	}

	buffer[i++] = 0;
	return number;
}

template class HexConv<char>;
template class HexConv<wchar_t>;
