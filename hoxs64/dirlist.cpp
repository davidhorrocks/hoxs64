#include <windows.h>
#include "utils.h"
#include "wfs.h"
#include "StringConverter.h"
#include "dirlist.h"

namespace FileSys
{	
	const wchar_t* const DirectoryItem::AllC64Extenstions[10] = { L".64S", L".D64", L".G64", L".P64", L".FDI", L".TAP", L".T64", L".PRG", L".CRT", L".SID" };

	const wchar_t* const DirectoryItem::DiskC64Extenstions[4] = { L".D64", L".G64", L".P64", L".FDI" };

	const wchar_t* const DirectoryItem::TapeC64Extenstions[1] = { L".TAP" };

	const wchar_t* const DirectoryItem::SidC64Extenstions[1] = { L".SID" };

	bool DirectoryItem::IsDirectory()  const
	{
		if (this->Type == DirectoryItemType::DriveLetter)
		{
			return true;
		}
		else if (this->Type == DirectoryItemType::DirectoryFileItem)
		{
			if (this->Find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				return true;
			}
		}
		else if (this->Type == DirectoryItemType::ParentDirectory)
		{
			return true;
		}

		return false;
	}

	bool DirectoryItem::IsCommodore64File()  const
	{
		if (this->Type == DirectoryItemType::DirectoryFileItem)
		{
			for (int i = 0; i < _countof(AllC64Extenstions); i++)
			{
				if (Wfs::FilenameHasExtension(this->Find_data.cFileName, AllC64Extenstions[i]))
				{
					return true;
				}
			}
		}

		return false;
	}

	bool DirectoryItem::IsCommodore64Disk()  const
	{
		if (this->Type == DirectoryItemType::DirectoryFileItem)
		{
			for (int i = 0; i < _countof(DiskC64Extenstions); i++)
			{
				if (Wfs::FilenameHasExtension(this->Find_data.cFileName, DiskC64Extenstions[i]))
				{
					return true;
				}
			}
		}

		return false;
	}

	bool DirectoryItem::IsCommodore64Tape()  const
	{
		if (this->Type == DirectoryItemType::DirectoryFileItem)
		{
			for (int i = 0; i < _countof(TapeC64Extenstions); i++)
			{
				if (Wfs::FilenameHasExtension(this->Find_data.cFileName, TapeC64Extenstions[i]))
				{
					return true;
				}
			}
		}

		return false;
	}

	bool DirectoryItem::IsCommodore64Sid()  const
	{
		if (this->Type == DirectoryItemType::DirectoryFileItem)
		{
			for (int i = 0; i < _countof(SidC64Extenstions); i++)
			{
				if (Wfs::FilenameHasExtension(this->Find_data.cFileName, SidC64Extenstions[i]))
				{
					return true;
				}
			}
		}

		return false;
	}

	const std::wstring& DirectoryItem::GetNameW() const
	{
		if (hasNameW)
		{
			return NameW;
		}

		switch (this->Type)
		{
			case DirectoryItemType::None:
				NameW.clear();
				break;
			case DirectoryItemType::VolumePath:
				NameW = std::wstring(this->VolumePathItemName);
				break;
			case DirectoryItemType::DriveLetter:
			{
				std::wstring t;
				t.push_back(this->DriveLetter);
				t.push_back(L':');
				t.push_back(L'\\');
				NameW = t;
			}
			break;
			case DirectoryItemType::DirectoryFileItem:
			{
				std::wstring t;
				for (size_t i = 0; i < ARRAYSIZE(this->Find_data.cFileName); i++)
				{
					wchar_t ch = this->Find_data.cFileName[i];
					if (ch == 0)
					{
						break;
					}

					t.push_back(ch);
				}

				NameW = t;
				break;
			}
			case DirectoryItemType::ParentDirectory:
				NameW = L"..";
				break;
			default:
				NameW = L"?";
		}

		hasNameW = true;
		return NameW;
	}

	const std::string& DirectoryItem::GetNameA() const
	{
		if (hasNameA)
		{
			return NameA;
		}

		NameA = StringConverter::WideStringToString(GetNameW());
		hasNameA = true;
		return NameA;

	}

	void DirectoryItem::Clear()
	{
		Type = DirectoryItemType::None;
		hasNameA = false;
		hasNameW = false;
	}

	// Directories
	EnumDirectoryItemImpl::EnumDirectoryItemImpl(const wchar_t* pDirectoryName)
		:ffhandle(nullptr), directoryName(pDirectoryName)
	{

	}

	EnumDirectoryItemImpl::~EnumDirectoryItemImpl() 
	{
		Reset();
	}

	bool EnumDirectoryItemImpl::GetNext(DirectoryItem* pitem, bool* pisEndOfList, std::wstring message)
	{
		message.clear();
		bool isEndOfList = false;
		bool ok = true;
		for (;;)
		{
			pitem->Clear();
			pitem->Type = DirectoryItemType::DirectoryFileItem;
			if (ffhandle == nullptr)
			{
				HANDLE r = ::FindFirstFileW(directoryName.c_str(), &pitem->Find_data);
				if (r == INVALID_HANDLE_VALUE)
				{
					if (GetLastError() == ERROR_FILE_NOT_FOUND)
					{
						isEndOfList = true;
					}
					else
					{
						message = G::GetLastWin32ErrorWString();
					}

					ok = false;
				}
				else
				{
					pitem->GetNameW();
					ffhandle = r;
				}
			}
			else
			{
				BOOL br = ::FindNextFileW(ffhandle, &pitem->Find_data);
				if (br == FALSE)
				{
					if (GetLastError() == ERROR_NO_MORE_FILES)
					{
						isEndOfList = true;
					}
					else
					{
						message = G::GetLastWin32ErrorWString();
					}

					ok = false;
				}
				else
				{
					pitem->GetNameW();
				}
			}

			if (ok && !isEndOfList)
			{
				if (std::wcscmp(pitem->Find_data.cFileName, L"..") == 0 || std::wcscmp(pitem->Find_data.cFileName, L".") == 0)
				{
					continue;
				}
			}

			break;
		}

		if (pisEndOfList)
		{
			*pisEndOfList = isEndOfList;
		}

		if (!ok)
		{
			Reset();
		}

		return ok;
	}

	void EnumDirectoryItemImpl::Reset() noexcept
	{
		if (ffhandle)
		{
			FindClose(ffhandle);
			ffhandle = nullptr;
		}
	}


	// Volumes
	EnumVolumePathItemImpl::EnumVolumePathItemImpl() noexcept
		:volumehandle(nullptr), Names(nullptr), CharCount(0), NameIdx(nullptr)
	{

	}

	EnumVolumePathItemImpl::~EnumVolumePathItemImpl()
	{
		Reset();
	}

	bool EnumVolumePathItemImpl::GetNext(DirectoryItem* pitem, bool* pisEndOfList, std::wstring message)
	{
		message.clear();
		pitem->Clear();
		pitem->Type = DirectoryItemType::VolumePath;
		*pisEndOfList = false;
		WCHAR  VolumeName[MAX_PATH + 1];
		bool isEndOfPathList = false;

		for (;;)
		{
			if (volumehandle == nullptr)
			{
				HANDLE r = ::FindFirstVolumeW(VolumeName, ARRAYSIZE(VolumeName));
				if (r == INVALID_HANDLE_VALUE)
				{
					if (GetLastError() == ERROR_FILE_NOT_FOUND)
					{
						*pisEndOfList = true;
					}
					else
					{
						message = G::GetLastWin32ErrorWString();
					}

					return false;
				}
				else
				{
					volumehandle = r;
					this->currentVolumeName = VolumeName;
					pitem->VolumeName = VolumeName;
				}

				PWCHAR pPath;
				if (this->GetFirstVolumePathItem(this->currentVolumeName.c_str(), &isEndOfPathList, &pPath))
				{
					pitem->VolumePathItemName = pPath;
					return true;
				}
				else
				{
					*pisEndOfList = isEndOfPathList;
					if (!isEndOfPathList)
					{
						message = G::GetLastWin32ErrorWString();
						return false;
					}
					else
					{
						continue;
					}
				}
			}
			else
			{
				PWCHAR pPath;
				if (this->GetNextVolumePathItem(&isEndOfPathList, &pPath))
				{
					pitem->VolumePathItemName = pPath;
					return true;
				}
				else
				{
					if (!isEndOfPathList)
					{
						message = G::GetLastWin32ErrorWString();
						return false;
					}
				}

				BOOL br = ::FindNextVolumeW(volumehandle, VolumeName, ARRAYSIZE(VolumeName));
				if (br == FALSE)
				{
					if (GetLastError() == ERROR_NO_MORE_FILES)
					{
						*pisEndOfList = true;
						return false;
					}
					else
					{
						message = G::GetLastWin32ErrorWString();
						return false;
					}
				}
				else
				{
					this->currentVolumeName = VolumeName;
					pitem->VolumeName = VolumeName;
					if (this->GetFirstVolumePathItem(this->currentVolumeName.c_str(), &isEndOfPathList, &pPath))
					{
						pitem->VolumePathItemName = pPath;
						return true;
					}
					else
					{
						*pisEndOfList = isEndOfPathList;
						if (!isEndOfPathList)
						{
							message = G::GetLastWin32ErrorWString();
							return false;
						}
						else
						{
							continue;
						}
					}
				}
			}
		}
	}

	bool EnumVolumePathItemImpl::GetFirstVolumePathItem(const wchar_t* pwsVolumeName, bool* pisEndOfList, wchar_t** ppPathItem)
	{
		DWORD RequestedCharCount = MAX_PATH + 1;
		DWORD CharCopiedCount = 0;

		BOOL Success = FALSE;
		for (;;)
		{
			if (Names == nullptr)
			{				
				Names = (PWCHAR) new WCHAR[RequestedCharCount + 1];
				if (!Names)
				{
					throw std::bad_array_new_length();
				}

				CharCount = RequestedCharCount;
			}

			Success = GetVolumePathNamesForVolumeNameW(
				pwsVolumeName, Names, CharCount, &CharCopiedCount
			);

			if (Success)
			{
				break;
			}

			if (GetLastError() != ERROR_MORE_DATA)
			{
				break;
			}

			if (CharCopiedCount <= CharCount)
			{
				throw std::exception("GetVolumePathNamesForVolumeNameW failed.");
			}

			delete[] Names;
			Names = NULL;
			CharCount = 0;		
			RequestedCharCount = CharCopiedCount;
		}

		if (Success)
		{
			NameIdx = Names;
		}
		else
		{
			return false;
		}

		return GetNextVolumePathItem(pisEndOfList, ppPathItem);
	}

	bool EnumVolumePathItemImpl::GetNextVolumePathItem(bool* pisEndOfList, wchar_t** ppPathItem)
	{
		*pisEndOfList = false;

		if (NameIdx[0] != L'\0')
		{
			*ppPathItem = NameIdx;
			NameIdx += wcslen(NameIdx) + 1;
			return true;
		}
		else
		{
			*pisEndOfList = true;
			return false;
		}
	}

	void EnumVolumePathItemImpl::Reset() noexcept
	{
		if (volumehandle != nullptr)
		{
			::FindVolumeClose(volumehandle);
			volumehandle = nullptr;
		}

		NameIdx = nullptr;
		delete[] Names;
		Names = NULL;
		CharCount = 0;
	}


	// Drives
	EnumDriveItemImpl::EnumDriveItemImpl() noexcept
	{
		bitIndex = 0;
	}

	EnumDriveItemImpl::~EnumDriveItemImpl()
	{
		Reset();
	}

	bool EnumDriveItemImpl::GetNext(DirectoryItem* pitem, bool* pisEndOfList, std::wstring message)
	{
		message.clear();
		pitem->Clear();
		pitem->Type = DirectoryItemType::DriveLetter;
		*pisEndOfList = false;
		DWORD drives = GetLogicalDrives();
		if (drives == 0)
		{
			message = G::GetLastWin32ErrorWString();
			return false;
		}

		for (; bitIndex < 26; bitIndex++)
		{
			DWORD bit = 1 << bitIndex;
			if (bit & drives)
			{
				pitem->Type = DirectoryItemType::DriveLetter;
				pitem->DriveLetter = 'A' + bitIndex;
				bitIndex++;
				return true;
			}
		}

		*pisEndOfList = true;
		return false;
	}

	void EnumDriveItemImpl::Reset() noexcept
	{
		bitIndex = 0;
	}

	// DirList
	IEnumDirectoryItem *DirList::CreateDirectoryEnumerator(const wchar_t* pszDirectory)
	{
		IEnumDirectoryItem* p = new EnumDirectoryItemImpl(pszDirectory);
		return p;
	}

	IEnumDirectoryItem* DirList::CreateVolumePathEnumerator()
	{
		IEnumDirectoryItem* p = new EnumVolumePathItemImpl();
		return p;
	}

	IEnumDirectoryItem* DirList::CreateDriveEnumerator()
	{
		IEnumDirectoryItem* p = new EnumDriveItemImpl();
		return p;
	}
}
