#include <windows.h>
#include <commctrl.h>
#include "dx_version.h"
#include <ddraw.h>
#include <dinput.h>
#include <dsound.h>
#include <stdio.h>
#include <stdarg.h>
#include "boost2005.h"
#include "defines.h"
#include "CDPI.h"
#include "tchar.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "register.h"
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "c6502.h"
#include "ram64.h"
#include "cpu6510.h"
#include "sidfile.h"
#include "resource.h"


SIDLoader::SIDLoader()
{
	ZeroMemory(&sfh, sizeof(sfh));
	pSIDFile = NULL;
	pDriver = NULL;
}

SIDLoader::~SIDLoader()
{
	cleanup();
}

void SIDLoader::cleanup()
{
	if(pSIDFile)
		GlobalFree(pSIDFile);

	pSIDFile = NULL;
}


HRESULT SIDLoader::LoadSIDFile(TCHAR *filename)
{
HRESULT hr;
HANDLE hfile=0;
BOOL r;
DWORD bytes_read,file_size,bytesToRead;
DWORD maxcodesize;
const int sizeheader = 2;

	if(pSIDFile)
		GlobalFree(pSIDFile);
	pSIDFile = NULL;
	startSID = 0;
	lenSID = 0;
	RSID=FALSE;
	RSID_BASIC=FALSE;

	hfile=CreateFile(filename,GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL); 
	if (hfile==INVALID_HANDLE_VALUE)
	{
		hr = SetError(E_FAIL,TEXT("Could not open %s."),filename);
		goto cleanup;
	}
	file_size = GetFileSize(hfile, 0);
	if (INVALID_FILE_SIZE == file_size)
	{
		hr = SetError(E_FAIL,TEXT("Could not open %s."),filename);
		goto cleanup;
	}
	if (file_size < sizeof(sfh))
	{ 
		hr = SetError(E_FAIL,TEXT("%s is not a supported SID file."),filename);
		goto cleanup;
	}

	bytesToRead = sizeof(struct SIDFileHeader1);
	r=ReadFile(hfile,&sfh,bytesToRead,&bytes_read,NULL);
	if (r==0)
	{
		return SetError(E_FAIL,TEXT("Could not read from %s."),filename);
		goto cleanup;
	}
	if (bytes_read!=bytesToRead)
	{
		hr = SetError(E_FAIL,TEXT("Could not read from %s."),filename);
		goto cleanup;
	}

	if (memcmp(sfh.magicID ,"PSID", 4)!=0 && memcmp(sfh.magicID ,"RSID", 4)!=0)
	{
		hr = SetError(E_FAIL,TEXT("%s is not a supported SID file."), filename);
		goto cleanup;
	}
	if (memcmp(sfh.magicID ,"RSID", 4)==0)
	{
		RSID=TRUE;
	}

	sfh.version = wordswap(sfh.version);
	sfh.initAddress = wordswap(sfh.initAddress);
	sfh.loadAddress = wordswap(sfh.loadAddress);
	sfh.dataOffset = wordswap(sfh.dataOffset);
	sfh.playAddress = wordswap(sfh.playAddress);

	sfh.songs = wordswap(sfh.songs);
	sfh.startSong = wordswap(sfh.startSong);

	sfh.speed = dwordswap(sfh.speed);

	if (sfh.version == 1)
	{
		if (RSID)
		{
			hr = SetError(E_FAIL,TEXT("Unsupported RSID file version %d. Supported RSID version is 2."), (int)sfh.version);
			goto cleanup;
		}
		sfh.flags=0;
		sfh.startPage=0;
		sfh.pageLength=0;
		sfh.reserved=0;
	}
	else if (sfh.version == 2)
	{
		bytesToRead = sizeof(struct SIDFileHeader2) - sizeof(struct SIDFileHeader1);
		r = ReadFile(hfile,&sfh.flags,bytesToRead ,&bytes_read,NULL);
		if (r==0)
		{
			return SetError(E_FAIL,TEXT("Could not read from %s."),filename);
			goto cleanup;
		}
		if (bytes_read!=bytesToRead)
		{
			hr = SetError(E_FAIL,TEXT("Could not read from %s."),filename);
			goto cleanup;
		}
		sfh.flags = wordswap(sfh.flags);
		sfh.reserved = wordswap(sfh.reserved);
	}
	else
	{
		hr = SetError(E_FAIL,TEXT("Unsupported file version %d. Supported versions are 1 and 2."), (int)sfh.version);
		goto cleanup;
	}

	if (sfh.dataOffset >= file_size || (short)sfh.dataOffset<0)
	{
		hr = SetError(E_FAIL,TEXT("Invalid file format. Bad data offset."));
		goto cleanup;
	}

	r= SetFilePointer (hfile, sfh.dataOffset, 0L, FILE_BEGIN);
	if (r == INVALID_SET_FILE_POINTER)
	{
		return SetError(E_FAIL,TEXT("Could not seek in file %s."),filename);
		goto cleanup;
	}

	startSID = 0;
	r = ReadFile(hfile,&startSID,sizeheader,&bytes_read,NULL);
	if (r==0)
	{
		return SetError(E_FAIL,TEXT("Could not read from %s."),filename);
		goto cleanup;
	}

	maxcodesize = 0x10000 - startSID;
	lenSID = file_size - sfh.dataOffset - sizeheader;
	if (lenSID > maxcodesize)
		lenSID = maxcodesize;
	pSIDFile = (BYTE *)GlobalAlloc(GMEM_FIXED, lenSID);
	if (pSIDFile==0)
	{
		return SetError(E_FAIL,TEXT("Out of memory."));
		goto cleanup;
	}
	r = ReadFile(hfile,pSIDFile,lenSID,&bytes_read,NULL);
	if (r==0)
	{
		return SetError(E_FAIL,TEXT("Could not read from %s."),filename);
		goto cleanup;
	}

	CloseHandle(hfile);

	if (RSID && (sfh.flags & 2) && sfh.initAddress ==0)
	{
		RSID_BASIC = TRUE;
	}


	return S_OK;
cleanup:
	if (hfile)
	{
		CloseHandle(hfile);
		hfile=0;
	}
	if(pSIDFile)
		GlobalFree(pSIDFile);
	pSIDFile = NULL;
	return hr;
}


HRESULT SIDLoader::LoadSIDDriverResource(WORD loadAddress,BYTE *pMemory)
{
HRSRC v=0;
HGLOBAL hv=NULL;
UINT l=0;
DWORD a,b,d;
WORD reloc;
WORD rtext,rdata,rbss,rzero;
WORD newText,newData,newBss;
WORD addr;
BYTE b1,b2;
const BYTE magicid[3] =  {0x6f, 0x36, 0x35};

	if (pDriver == NULL)
	{
		v=FindResource(NULL,MAKEINTRESOURCE(IDR_SIDDRIVER),TEXT("BINARY"));
		if (v)
		{
			driverSize = SizeofResource(0L,v);
			if (driverSize != 0)
			{
				hv=LoadResource(NULL,v);
				if (hv)
				{
					pDriver = (BYTE *)LockResource(hv);
					if (!pDriver)
					{
						return E_FAIL;
					}
				}
			}
		}
	}

	if (driverSize <= sizeof(fixedheader16))
		return E_FAIL;

	src = pDriver;
	eof=FALSE;
	CopyMemory(&fh, src, FIELD_OFFSET(struct fixedheader32,tbase));
	if(fh.non64marker != 0x0001)
		return E_FAIL;
	if(fh.non64marker != 0x0001)
		return E_FAIL;
	
	if (memcmp(magicid, fh.magicnumber, sizeof(fh.magicnumber))!=0)
		return E_FAIL;

	if (fh.mode & 0x2000)
		sizetlen=2;
	else
		sizetlen=1;

	src = &pDriver[FIELD_OFFSET(struct fixedheader32,tbase)];
	fh.tbase = readSizetDWord();
	fh.tlen = readSizetDWord();
	fh.dbase = readSizetDWord();
	fh.dlen = readSizetDWord();
	fh.bbase = readSizetDWord();
	fh.blen = readSizetDWord();
	fh.zbase = readSizetDWord();
	fh.zlen = readSizetDWord();
	fh.stack = readSizetDWord();

	if (eof)
		return E_FAIL;

	if (fh.tbase>0xffff)
		return E_FAIL;
	if (fh.dbase>0xffff)
		return E_FAIL;
	if (fh.bbase>0xffff)
		return E_FAIL;

	if (fh.tlen>0xffff)
		return E_FAIL;
	if (fh.dlen>0xffff)
		return E_FAIL;
	if (fh.blen>0xffff)
		return E_FAIL;

	if (fh.tbase + fh.tlen + fh.dlen + fh.blen -1 > 0xffff)
		return E_FAIL;

	if (loadAddress + fh.tlen + fh.dlen + fh.blen -1 > 0xffff)
		return E_FAIL;


	do
	{
		b1 = (BYTE)readByte();
		if (eof)
			return E_FAIL;

		if (b1==0)
			break;

		if (!driverspace(b1))
			return E_FAIL;
		src += b1;

	} while (1);

	if (fh.tlen > 0)
	{
		if (!driverspace(fh.tlen))
			return E_FAIL;

		if (loadAddress + fh.tlen -1 > 0xffff)
			return E_FAIL;
		if (pMemory)
			CopyMemory(&pMemory[loadAddress], src, fh.tlen);
		src += fh.tlen;
	}

	if (fh.dlen > 0)
	{
		if (!driverspace(fh.dlen))
			return E_FAIL;

		if (loadAddress + fh.tlen + fh.dlen -1 > 0xffff)
			return E_FAIL;
		if (pMemory)
			CopyMemory(&pMemory[loadAddress + fh.tlen], src, fh.dlen);
		src += fh.dlen;
	}

	codeSize = (WORD)(fh.tlen + fh.dlen + fh.blen);

	if (!pMemory)
		return 0;

	d = readSizetDWord();
	if (eof)
		return E_FAIL;

	for (a = 0 ; a < d ; a++)
	{
		do
		{
			b = readByte();
			if (eof)
				return E_FAIL;
		} while (b != 0);
	}

	newText = loadAddress;
	newData = newText + (WORD)fh.tlen;
	newBss = newData + (WORD)fh.dlen ;

	rtext = newText - 1;
	rdata = newData -1;
	rbss = newBss - 1;
	rzero = 0;
	do
	{
		reloc = 0;

		do
		{
			b1 = (BYTE)readByte();
			if (eof)
				return E_FAIL;
			if (b1 == 0)
				break;
			if (b1 == 0xff)
				reloc += 0xfe;
			else
				reloc += b1;
		} while (b1 == 0xff);
		if (reloc == 0 && b1 == 0)
			break;
		
		b1 = (BYTE)readByte();
		if (eof)
			return E_FAIL;

		
		switch (b1 & 0x0f)
		{
		case 2://text segment
			rtext += reloc;
			break;
		case 3://data segment
			rdata += reloc;
			break;
		case 4://bss segment
			rbss += reloc;
			break;
		}
		
		if (rtext > 0xffff)
			return E_FAIL;
		if (rdata > 0xffff)
			return E_FAIL;
		if (rbss > 0xffff)
			return E_FAIL;

		switch (b1 & 0xf0)
		{
		case 0x80:// 2 byte address
			switch (b1 & 0x0f)
			{
			case 0://undefined
				d = readSizetDWord();
				if (eof)
					return E_FAIL;
				break;
			case 1://absolute value
				d = readSizetDWord();
				if (eof)
					return E_FAIL;
				break;
			case 2://text segment
				if (rtext + 1 > 0xffff)
					return E_FAIL;
				addr = *((WORD *)(((BYTE *)pMemory) + rtext));
				addr = addr - (WORD)fh.tbase + newText;
				*((WORD *)(((BYTE *)pMemory) + rtext)) = addr;
				break;
			case 3://data segment
				if (rdata + 1 > 0xffff)
					return E_FAIL;
				addr = *((WORD *)(((BYTE *)pMemory) + rdata));
				addr = addr - (WORD)fh.dbase + newData;
				*((WORD *)(((BYTE *)pMemory) + rdata)) = addr;
				break;
			case 4://bss segment
				if (rbss + 1 > 0xffff)
					return E_FAIL;
				addr = *((WORD *)(((BYTE *)pMemory) + rbss));
				addr = addr - (WORD)fh.bbase + newBss;
				*((WORD *)(((BYTE *)pMemory) + rbss)) = addr;
				break;
			case 5://zero segment
				break;
			}
			break;
		case 0x40:// hi byte address
			switch (b1 & 0x0f)
			{
			case 0://undefined
				d = readSizetDWord();
				if (eof)
					return E_FAIL;
				d = readByte();
				if (eof)
					return E_FAIL;
				break;
			case 1://absolute value
				d = readSizetDWord();
				if (eof)
					return E_FAIL;
				d = readByte();
				if (eof)
					return E_FAIL;
				break;
			case 2://text segment
				b2 = (BYTE)readByte();
				if (eof)
					return E_FAIL;
				addr = *(((BYTE *)pMemory) + rtext);
				addr = (addr << 8) | (b2 & 0xff);
				addr = addr - (WORD)fh.tbase + newText;
				addr >>= 8;
				*(((BYTE *)pMemory) + rtext) = (BYTE)addr;
				break;
			case 3://data segment
				b2 = (BYTE)readByte();
				if (eof)
					return E_FAIL;
				addr = *(((BYTE *)pMemory) + rdata);
				addr = (addr << 8) | (b2 & 0xff);
				addr = addr - (WORD)fh.dbase + newData;
				addr >>= 8;
				*(((BYTE *)pMemory) + rdata) = (BYTE)addr;
				break;
			case 4://bss segment
				b2 = (BYTE)readByte();
				if (eof)
					return E_FAIL;
				addr = *(((BYTE *)pMemory) + rbss);
				addr = (addr << 8) | (b2 & 0xff);
				addr = addr - (WORD)fh.bbase + newBss;
				addr >>= 8;
				*(((BYTE *)pMemory) + rbss) = (BYTE)addr;
				break;
			case 5://zero segment
				b2 = (BYTE)readByte();
				if (eof)
					return E_FAIL;
				break;
			}
			break;
		case 0x20:// lo byte address
			switch (b1 & 0x0f)
			{
			case 0://undefined
				d = readSizetDWord();
				if (eof)
					return E_FAIL;
				break;
			case 1://absolute value
				d = readSizetDWord();
				if (eof)
					return E_FAIL;
				break;
			case 2://text segment
				addr = *(((BYTE *)pMemory) + rtext);
				addr = addr & 0xff;
				addr = addr - (WORD)fh.tbase + newText;
				addr = addr & 0xff;
				*(((BYTE *)pMemory) + rtext) = (BYTE)addr;
				break;
			case 3://data segment
				addr = *(((BYTE *)pMemory) + rdata);
				addr = addr & 0xff;
				addr = addr - (WORD)fh.dbase + newData;
				addr = addr & 0xff;
				*(((BYTE *)pMemory) + rdata) = (BYTE)addr;
				break;
			case 4://bss segment
				addr = *(((BYTE *)pMemory) + rbss);
				addr = addr & 0xff;
				addr = addr - (WORD)fh.bbase + newBss;
				addr = addr & 0xff;
				*(((BYTE *)pMemory) + rbss) = (BYTE)addr;
				break;
			case 5://zero segment
				break;
			}
			break;
		case 0xc0:// 3 byte segment address
		case 0xa0:// segment byte
			return E_FAIL;
		}
	} while (1);
	
	return 0;
}

HRESULT SIDLoader::LoadSID(bit8 *ramMemory, TCHAR *filename, bool bUseDefaultStartSong, WORD startSong)
{
HRESULT hr;
	
	ClearError();
	hr = LoadSIDFile(filename);
	if (FAILED(hr))
		return hr;
	hr = LoadSID(ramMemory, bUseDefaultStartSong, startSong);
	if (FAILED(hr))
		return hr;
	return S_OK;
}

HRESULT SIDLoader::LoadSID(bit8 *ramMemory, bool bUseDefaultStartSong, WORD startSong)
{
HRESULT hr;
BYTE requiredPages,s;

	ClearError();
	if (pSIDFile==NULL)
		return E_POINTER;

	CopyMemory(&ramMemory[startSID], pSIDFile, lenSID);

	hr = LoadSIDDriverResource(0x0000, 0L);
	if (FAILED(hr))
	{
		return SetError(E_FAIL,TEXT("Could not load SID driver."));
	}

	if ((sfh.version == 1) || (sfh.startPage == 0 && sfh.pageLength == 0))
	{
		if (startSID > (0x0834 + codeSize))
		{
			driverLoadAddress = 0x0834;
		}
		else if ((startSID + (WORD)lenSID - 1) < ((WORD)0xd000 - codeSize))
		{
			driverLoadAddress = (WORD)0xd000 - (WORD)codeSize;
		}
		else
			return SetError(E_FAIL,TEXT("Could not load SID driver."));

	}
	else
	{
		s=sfh.startPage;
		if ((s < 4) 
			|| ((s >= 0xa0) && (s < 0xc0))
			|| (s >= 0xd0))
			return SetError(E_FAIL,TEXT("Could not load SID driver."));

		if (s>=4 && s<=7)
			s=8;
		if ((WORD)s + (WORD)sfh.pageLength - 1 > 0xff)
			return SetError(E_FAIL,TEXT("Could not load SID driver."));

		requiredPages = codeSize >> 8;
		if (codeSize & 0x00ff)
			requiredPages++;
		if (sfh.pageLength < requiredPages)
			return SetError(E_FAIL,TEXT("Could not load SID driver."));
		driverLoadAddress = (WORD)s << 8;
	}


	hr = LoadSIDDriverResource(driverLoadAddress, ramMemory);
	if (FAILED(hr))
		return hr;



	CopyMemory(&ramMemory[driverLoadAddress + 3], &sfh, sizeof(sfh));

	if (!bUseDefaultStartSong)
		((SIDFileHeader2 *)(&ramMemory[driverLoadAddress + 3]))->startSong=startSong;

	return S_OK;
}

DWORD SIDLoader::readByte()
{
DWORD l;

	if ((src - pDriver + sizeof(BYTE)) > driverSize)
	{
		eof = TRUE;
		return 0;
	}

	l = *((BYTE *)src);
	src+=sizeof(BYTE);
	return l;
}

DWORD SIDLoader::readWord()
{
DWORD l;

	if ((src - pDriver + sizeof(WORD)) > driverSize)
	{
		eof = TRUE;
		return 0;
	}

	l = *((WORD *)src);
	src+=sizeof(WORD);
	return l;
}

DWORD SIDLoader::readDWord()
{
DWORD l;

	if ((src - pDriver + sizeof(DWORD)) > driverSize)
	{
		eof = TRUE;
		return 0;
	}

	l = *((DWORD *)src);
	src+=sizeof(DWORD);
	return l;
}

DWORD SIDLoader::readSizetWord()
{
	if (sizetlen==1)
		return readByte();
	else
		return readWord();
}

DWORD SIDLoader::readSizetDWord()
{
	if (sizetlen==1)
		return readWord();
	else
		return readDWord();
}

BOOL SIDLoader::driverspace(DWORD len)
{
	return !((src - pDriver + len) > driverSize);
}