#include <windows.h>
#include <tchar.h>
#include "bits.h"
#include "hexconv.h"

unsigned char HexConv::hextable[256] = {0};

void HexConv::init_hex_table(){
unsigned char i;
	for (i=0 ; i<=(unsigned char)255 ; i++){
		if (i >= '0' && i <='9')
			hextable[i]=i-'0';
		else if (i >= 'A' && i <='F')
			hextable[i]=i-'A'+10;
		else if (i >= 'a' && i <='f')
			hextable[i]=i-'a'+10;
		else
			hextable[i]=255;
	}
}

bit32 HexConv::hex_to_long(TCHAR *buffer){
bit32 i,l,v,a,d;

	if (hextable[0]==0)
		init_hex_table();
	a=0;
	l=lstrlen(buffer);
	for (i=0 ; i<l && i<8 ; i++){
		d=buffer[i];
		if (d>255)
			return a;

		v=hextable[d];
		if (v==255)
			return a;
		a=a << 4;
		a=a | v;
	}
	return a;
}

bit32 HexConv::long_to_hex(bit32 number, TCHAR *buffer, unsigned int digits){
bit32 lz,i,v,s;
bit32 mask;

	lz=0;
	i=0;	
	if (digits>=1 && digits<=8){
		mask=0x0000000F;
		mask = mask << ((digits-1)*4);
		s=(digits-1)*4;
	}
	else{
		mask=0xF0000000;
		s=7*4;
	}

	while (mask){
		v=number & mask;
		v= (v >> s) & 0xf;
		if (v <= 9)
			buffer[i++]=(TCHAR) (v + TEXT('0'));
		else
			buffer[i++]=(TCHAR) (v - 10 + TEXT('A'));

		mask = mask >> 4;
		s=s-4;
	}
	buffer[i++]=0;
	
	return number;
}

