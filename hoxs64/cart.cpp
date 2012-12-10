#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <assert.h>
#include <vector>
#include <list>

#include "boost2005.h"
#include "user_message.h"
#include "defines.h"
#include "mlist.h"
#include "carray.h"
#include "cevent.h"
#include "errormsg.h"
#include "bits.h"
#include "util.h"
#include "CDPI.h"
#include "utils.h"
#include "cart.h"


Cart::Cart()
{
}

Cart::~Cart()
{
	CleanUp();
}

HRESULT Cart::LoadCrtFile(LPCTSTR filename)
{
HRESULT hr = E_FAIL;
HANDLE hFile = NULL;
DWORD nBytesRead;
BOOL br;
LPCTSTR S_READFAILED = TEXT("Could not read file %s.");
CrtHeader hdr;
__int64 pos = 0;
__int64 spos = 0;
bit8 S_SIGHEADER[] = "C64 CARTRIDGE";
bit8 S_SIGCHIP[] = "CHIP";
CrtChipAndDataList lstChipAndData;
bit8 *pBank = NULL;
__int64 filesize=0;
const int MAXBANKS = 256;

	ClearError();
	
	try
	{
		bool ok = true;
		do
		{
			hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				hr =  SetError(E_FAIL, TEXT("Could not open crt file %s."), filename);
				ok = false;
				break;
			}
			filesize = G::FileSize(hFile);
			if (filesize < 0)
			{
				hr = SetError(E_FAIL, S_READFAILED, filename);
				ok = false;
				break;
			}

			br = ReadFile(hFile, &hdr, sizeof(hdr), &nBytesRead, NULL);
			if (!br)
			{
				hr = SetError(E_FAIL, S_READFAILED, filename);
				ok = false;
				break;
			}

			if (_strnicmp((char *)&hdr.Signature, (char *)&S_SIGHEADER[0], _countof(S_SIGHEADER) - 1) != 0)
			{
				hr = SetError(E_FAIL, S_READFAILED, filename);
				ok = false;
				break;
			}

			hdr.HardwareType = wordswap(hdr.HardwareType);
			hdr.FileHeaderLength = dwordswap(hdr.FileHeaderLength);
			hdr.Version = wordswap(hdr.Version);
			if (hdr.FileHeaderLength != 0 && hdr.FileHeaderLength != sizeof(hdr))
			{
				pos = G::FileSeek(hFile, (__int64)hdr.FileHeaderLength, FILE_BEGIN);
				if (pos < 0  || pos != hdr.FileHeaderLength)
				{
					hr = SetError(E_FAIL, S_READFAILED, filename);
					ok = false;
					break;
				}
			}

			int nChipCount = 0;
			do
			{
				CrtChip chip;
				spos = G::FileSeek(hFile, 0, FILE_CURRENT);
				if (spos < 0)
				{
					hr = SetError(E_FAIL, S_READFAILED, filename);
					ok = false;
					break;
				}
				if (filesize - spos < sizeof(chip))
				{
					hr = S_OK;
					break;
				}
				br = ReadFile(hFile, &chip, sizeof(chip), &nBytesRead, NULL);
				if (!br)
				{
					hr = SetError(E_FAIL, S_READFAILED, filename);
					ok = false;
					break;
				}

				chip.BankLocation = wordswap(chip.BankLocation);
				chip.LoadAddressRange = wordswap(chip.LoadAddressRange);
				chip.ROMImageSize = wordswap(chip.ROMImageSize);
				chip.TotalPacketLength = dwordswap(chip.TotalPacketLength);

				if (_strnicmp((char *)&chip.Signature, (char *)&S_SIGCHIP[0], _countof(S_SIGCHIP) - 1) != 0)
				{
					hr = S_OK;
					break;
				}
				if (nChipCount > MAXBANKS)
				{
					hr = SetError(E_FAIL, S_READFAILED, filename);
					ok = false;
					break;
				}
				if (chip.ROMImageSize ==0)
				{
					hr = SetError(E_FAIL, S_READFAILED, filename);
					ok = false;
					break;
				}
				pBank = (bit8 *)GlobalAlloc(GPTR, chip.ROMImageSize);
				if (!pBank)
				{
					hr = SetError(E_FAIL, S_READFAILED, filename);
					ok = false;
					break;
				}
				br = ReadFile(hFile, pBank, chip.ROMImageSize, &nBytesRead, NULL);
				if (!br)
				{
					hr = SetError(E_FAIL, S_READFAILED, filename);
					ok = false;
					break;
				}
				Sp_CrtChipAndData sp(new CrtChipAndData(chip, pBank));
				if (sp == 0)
					throw std::bad_alloc();
				pBank = NULL;
				lstChipAndData.push_back(sp);
				nChipCount++;
				pos = G::FileSeek(hFile, 0, FILE_CURRENT);
				if (pos < 0)
				{
					hr = SetError(E_FAIL, S_READFAILED, filename);
					ok = false;
					break;
				}
				if (chip.TotalPacketLength != pos - spos)
				{
					__int64 nextpos = spos + (__int64)chip.TotalPacketLength;
					if (nextpos >= filesize)
					{
						hr = S_OK;
						break;
					}
					pos = G::FileSeek(hFile, nextpos, FILE_BEGIN);
					if (pos < 0)
					{
						hr = SetError(E_FAIL, S_READFAILED, filename);
						ok = false;
						break;
					}
				}
			} while (ok);
			if (!ok)
				break;
		} while (false);
	}
	catch (std::exception&)
	{
		hr = SetError(E_FAIL, S_READFAILED, filename);
	}
	if (hFile)
	{
		CloseHandle(hFile);
		hFile = NULL;
	}
	if (pBank)
	{
		GlobalFree(pBank);
		pBank = NULL;
	}

	if (SUCCEEDED(hr))
	{
		if (lstChipAndData.size() == 0)
		{
			hr = SetError(E_FAIL, S_READFAILED, filename);
		}
		else
		{
			lstChipAndData.sort(LessChipAndDataBank());
			m_lstChipAndData = lstChipAndData;
		}
	}
	return hr;
}

void Cart::CleanUp()
{
	m_lstChipAndData.clear();
}


bool LessChipAndDataBank::operator()(const Sp_CrtChipAndData x, const Sp_CrtChipAndData y) const
{
	return x->chip.BankLocation < y->chip.BankLocation;
}

CrtChipAndData::~CrtChipAndData()
{
	if (this->pData)
	{
		GlobalFree(this->pData);
		this->pData = 0;
	}
}

CrtChipAndData::CrtChipAndData(CrtChip &chip, bit8 *pData)
{
	this->chip = chip;
	this->pData = pData;
}
