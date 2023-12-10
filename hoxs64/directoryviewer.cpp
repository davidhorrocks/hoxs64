#include <Windows.h>
#include <memory>
#include <algorithm> 
#include <string> 
#include "StringConverter.h"
#include "wfs.h"
#include "dirlist.h"
#include "directoryviewer.h"

namespace FileSys
{
	void DirectoryViewer::Fill()
	{
		this->CurrentDirectoryItems.clear();

		std::unique_ptr<IEnumDirectoryItem> p;
		if (ParentFullPath.size() == 0)
		{
			p = std::unique_ptr<IEnumDirectoryItem>(Dir.CreateDriveEnumerator());
		}
		else
		{
			DirectoryItem parentItem = {};

			// Add the double dot entry ".." to allow navigation to the parent.
			parentItem.Type = DirectoryItemType::ParentDirectory;
			CurrentDirectoryItems.push_back(parentItem);
			std::wstring dirname;
			for (auto pathitem = ParentFullPath.begin(); pathitem != ParentFullPath.end(); pathitem++)
			{
				DirectoryItem& di = *pathitem;
				switch (di.Type)
				{
				case DirectoryItemType::DriveLetter:
				{
					dirname.push_back(di.DriveLetter);
					dirname.push_back(L':');
					break;
				}
				case DirectoryItemType::DirectoryFileItem:
					Wfs::EnsureTrailingBackslash(dirname);
					dirname.append(di.GetNameW());
					break;
				default:
					return;
				}
			}

			Wfs::EnsureTrailingBackslash(dirname);
			dirname.push_back(L'*');

			const DirectoryItem& lastDirectoryItem = ParentFullPath.back();
			switch (lastDirectoryItem.Type)
			{
			case DirectoryItemType::DriveLetter:
				p = std::unique_ptr<IEnumDirectoryItem>(Dir.CreateDirectoryEnumerator(dirname.c_str()));
				break;
			case DirectoryItemType::DirectoryFileItem:
				p = std::unique_ptr<IEnumDirectoryItem>(Dir.CreateDirectoryEnumerator(dirname.c_str()));
				break;
			default:
				return;
			}
		}

		if (p)
		{
			DirectoryItem di;
			bool isEnd;
			std::wstring message;
			while (p->GetNext(&di, &isEnd, message))
			{
				if (di.Type == DirectoryItemType::DirectoryFileItem)
				{
					if (di.Find_data.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN))
					{
						continue;
					}

					if ((di.Find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
					{
						if (FileExtensionFilterList.size() > 0)
						{
							auto found = std::find_if(FileExtensionFilterList.begin(), FileExtensionFilterList.end(), [&](const std::wstring& s)
								{
									return ::_wcsicmp(s.c_str(), Wfs::GetFileExtension(di.GetNameW()).c_str()) == 0;
								});

							if (found == FileExtensionFilterList.end())
							{
								continue;
							}
						}
					}
				}

				this->CurrentDirectoryItems.emplace_back(DirectoryItem(di));
			}

			auto startpos = CurrentDirectoryItems.begin();
			if (startpos != CurrentDirectoryItems.end() && startpos->Type == DirectoryItemType::ParentDirectory)
			{
				startpos++;
			}

			if (startpos != CurrentDirectoryItems.end())
			{
				std::sort(startpos, CurrentDirectoryItems.end(), [](const DirectoryItem& a, const DirectoryItem& b)
					{
						if (a.IsDirectory() && !b.IsDirectory())
						{
							return true;
						}
						else if (!a.IsDirectory() && b.IsDirectory())
						{
							return false;
						}

						int r = _wcsicmp(a.GetNameW().c_str(), b.GetNameW().c_str());
						if (r <= 0)
						{
							return true;
						}
						else
						{
							return false;
						}
					}
				);
			}
		}
	}

	void DirectoryViewer::ChangeToRoot()
	{
		Set_CbmDirectoryLoaded(false);
		Set_IndexCurrentCbmDiskItem(-1);
		ParentFullPath.clear();
		Fill();
	}

	void DirectoryViewer::ChangeParentDirectory(size_t index)
	{
		Set_CbmDirectoryLoaded(false);
		Set_IndexCurrentCbmDiskItem(-1);
		if (index < this->ParentFullPath.size())
		{
			std::wstring dirfullpath = GetIndexedDir(index);
			if (dirfullpath.length() > 0)
			{
				DWORD attributes = ::GetFileAttributes(dirfullpath.c_str());
				if (attributes != INVALID_FILE_ATTRIBUTES)
				{
					DirectoryItem& di = ParentFullPath[index];
					di.Find_data.dwFileAttributes = attributes;
					if (attributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						if (ParentFullPath.size() > index)
						{
							ParentFullPath.resize(index + 1);
						}
					}

					Fill();
				}
			}
		}
	}

	void DirectoryViewer::ChangeDirectory(size_t index)
	{
		Set_CbmDirectoryLoaded(false);
		Set_IndexCurrentCbmDiskItem(-1);
		if (index < this->CurrentDirectoryItems.size())
		{
			DirectoryItem& di = CurrentDirectoryItems[index];
			if (di.Type == DirectoryItemType::ParentDirectory)
			{
				if (ParentFullPath.size() > 0)
				{
					ParentFullPath.pop_back();
				}
			}
			else
			{
				ParentFullPath.push_back(di);
			}

			Fill();
		}
	}

	void DirectoryViewer::ChangeDirectoryPath(const std::wstring directoryPath)
	{
		std::vector<DirectoryItem> listPath;
		std::wstring stritem;
		if (directoryPath.length() <= MaxApplicationPathLength && Wfs::IsAbsolutePath(directoryPath))
		{
			bool isFound = false;
			if (Wfs::DirectoryExists(directoryPath, &isFound, nullptr))
			{
				size_t i = 0;
				bool wantDriveLetter = true;
				for (i = 0; i < directoryPath.length(); i++)
				{
					size_t pos = directoryPath.find(L'\\', i);
					if (pos == i)
					{
						continue;
					}

					if (pos != std::string::npos)
					{
						if (i > pos)
						{
							// find error: should not happen.
							return;
						}

						stritem = directoryPath.substr(i, pos - i);
						i = pos;
					}
					else 
					{
						stritem = directoryPath.substr(i, directoryPath.length() - i);
						break;
					}

					if (stritem.length() == 0)
					{
						// empty not allowed.
						return;
					}

					if (std::isspace(stritem[0]) || std::isspace(stritem[stritem.length() - 1]))
					{
						// leading / trailing space not allowed.
						return;
					}

					if (wantDriveLetter)
					{
						if (stritem.length() == 2)
						{
							wchar_t letter = stritem[0];
							if (stritem[1] == L':' && ((letter >= L'A' && letter <= L'Z') || (letter >= L'a' && letter <= L'z')))
							{
								DirectoryItem diritem;
								diritem.Clear();
								diritem.DriveLetter = (CHAR)letter;
								diritem.Type = FileSys::DriveLetter;
								listPath.push_back(diritem);
								wantDriveLetter = false;
								continue;
							}
						}
					}
					else if (stritem.find_first_of(L"*\"/\\<>:|?") != std::string::npos)
					{
						// invalid chars
						return;
					}
					else
					{
						DirectoryItem diritem;
						diritem.Clear();
						diritem.NameW = stritem;
						diritem.hasNameW = true;
						diritem.Type = FileSys::DirectoryFileItem;
						listPath.push_back(diritem);
						continue;
					}

					return;
				}

			}

			if (listPath.size() == 0)
			{
				return;
			}

			ParentFullPath = listPath;
		}
	}

	std::wstring DirectoryViewer::GetCurrentDir() const
	{
		std::wstring dir;
		for (auto pathitem = ParentFullPath.begin(); pathitem != ParentFullPath.end(); pathitem++)
		{
			dir.append((*pathitem).GetNameW());
			Wfs::EnsureTrailingBackslash(dir);
		}

		return dir;
	}

	std::wstring DirectoryViewer::GetIndexedDir(size_t index)
	{
		std::wstring name;
		size_t i;
		if (index < ParentFullPath.size())
		{
			for (i = 0; i <= index; i++)
			{
				DirectoryItem& di = ParentFullPath[i];
				if (i != 0 && di.GetNameW().length() > 0)
				{
					Wfs::EnsureTrailingBackslash(name);
				}

				name.append(di.GetNameW());
			}
		}

		return name;
	}

	std::wstring DirectoryViewer::GetItemName(size_t index) const
	{
		if (index < CurrentDirectoryItems.size())
		{
			return Wfs::Path_Combine(this->GetCurrentDir(), this->CurrentDirectoryItems[index].GetNameW()).c_str();
		}
		else
		{
			return std::wstring();
		}
	}

	void DirectoryViewer::UseC64FilesFilter()
	{
		FileExtensionFilterList.clear();
		for (int i = 0; i < _countof(DirectoryItem::AllC64Extenstions); i++)
		{
			FileExtensionFilterList.push_back(DirectoryItem::AllC64Extenstions[i]);
		}
	}

	bool DirectoryViewer::Get_IsCbmDirectoryLoaded() const
	{
		return isCbmDirectoryLoaded;
	}

	void DirectoryViewer::Set_CbmDirectoryLoaded(bool isLoaded)
	{
		isCbmDirectoryLoaded = isLoaded;
	}

	bool DirectoryViewer::Get_IsCbmDirectorySuccessfullyLoaded() const
	{
		return isCbmDirectorySuccessfullyLoaded;
	}

	void DirectoryViewer::Set_CbmDirectorySuccessfullyLoaded(bool isSuccess)
	{
		isCbmDirectorySuccessfullyLoaded = isSuccess;
	}

	int DirectoryViewer::Get_IndexCurrentCbmDiskItem() const
	{
		return indexCurrentCbmDiskItem;
	}

	void DirectoryViewer::Set_IndexCurrentCbmDiskItem(int index)
	{
		indexCurrentCbmDiskItem = index;
	}

	bool DirectoryViewer::Get_IsCbmDiskTitleSelected() const
	{
		return isCbmDiskTitleSelected;
	}

	void DirectoryViewer::Set_IsCbmDiskTitleSelected(int isSelected)
	{
		isCbmDiskTitleSelected = isSelected;
	}

	bool DirectoryViewer::Get_IsQuickloadEnabled() const
	{
		return isQuickloadEnabled;
	}

	void DirectoryViewer::Set_IsQuickloadEnabled(int isEnabled)
	{
		isQuickloadEnabled = isEnabled;
	}

	bool DirectoryViewer::Get_IsReuEnabled() const
	{
		return isReuEnabled;
	}

	void DirectoryViewer::Set_IsReuEnabled(int isEnabled)
	{
		isReuEnabled = isEnabled;
	}

	HRESULT DirectoryViewer::LoadCbmDirectory(const TCHAR filename[], int maxcount, int& count, bool bPrgFilesOnly, HANDLE hevtQuit)
	{
		Set_CbmDirectoryLoaded(true);
		HRESULT hr = this->c64file.LoadDirectory(filename, maxcount, count, bPrgFilesOnly, hevtQuit);
		Set_CbmDirectorySuccessfullyLoaded(SUCCEEDED(hr));
		return hr;
	}

	void DirectoryViewer::ClearCbmDirectory()
	{
		Set_CbmDirectoryLoaded(true);
		Set_CbmDirectorySuccessfullyLoaded(false);
		Set_IndexCurrentCbmDiskItem(-1);
		this->c64file.ClearDirectory();
	}
}