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
std::vector<bit8 *> lstChipData;
std::vector<CrtChip> lstChip;
bit8 *p = NULL;
__int64 filesize=0;
const int MAXBANKS = 256;

	ClearError();
	
	try
	{
		bool ok = true;
		do
		{
			try
			{
				this->m_lstChip.reserve(MAXBANKS);
				this->m_lstChipData.reserve(MAXBANKS);
			}
			catch(std::exception&)
			{
				ok = false;
				hr = SetError(E_FAIL, S_READFAILED, filename);
				break;
			}

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
				p = (bit8 *)GlobalAlloc(GPTR, chip.ROMImageSize);
				if (!p)
				{
					hr = SetError(E_FAIL, S_READFAILED, filename);
					ok = false;
					break;
				}
				br = ReadFile(hFile, p, chip.ROMImageSize, &nBytesRead, NULL);
				if (!br)
				{
					hr = SetError(E_FAIL, S_READFAILED, filename);
					ok = false;
					break;
				}
				lstChip.push_back(chip);
				lstChipData.push_back(p);
				p = NULL;
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
	if (p)
	{
		GlobalFree(p);
		p = NULL;
	}

	if (SUCCEEDED(hr))
	{
		if (lstChip.size() == 0 || lstChipData.size() != lstChip.size())
		{
			hr = SetError(E_FAIL, S_READFAILED, filename);
		}
	}
	if (SUCCEEDED(hr))
	{
		try
		{
			this->m_lstChip.reserve(lstChip.size());
			this->m_lstChipData.reserve(lstChipData.size());
		}
		catch(std::exception&)
		{
			hr = E_FAIL;
		}
	}
	if (SUCCEEDED(hr))
	{
		this->m_lstChip.assign(lstChip.begin(), lstChip.end());
		this->m_lstChipData.assign(lstChipData.begin(), lstChipData.end());
	}
	else
	{
		for (std::vector<bit8 *>::iterator it = lstChipData.begin(); it != lstChipData.end(); it++)
		{
			if (*it != NULL)
			{
				GlobalFree(*it);
				*it = NULL;
			}
		}
	}
	return hr;
}

void Cart::CleanUp()
{
	for (std::vector<bit8 *>::iterator it = m_lstChipData.begin(); it != m_lstChipData.end(); it++)
	{
		if(*it)
		{
			GlobalFree(*it);
			*it = 0;
		}
	}
	m_lstChipData.clear();
	m_lstChip.clear();
}

