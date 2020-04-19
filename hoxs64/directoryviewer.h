#pragma once
#include "dirlist.h"
#include <vector>
#include <string>

namespace FileSys
{
	class DirectoryViewer
	{
	public:
		DirList Dir;
		void Fill();
		void ChangeToRoot();
		void ChangeParentDirectory(size_t index);
		void ChangeDirectory(size_t index);
		std::wstring GetCurrentDir() const;
		std::wstring GetItemName(size_t index) const;
		void UseC64FilesFilter();
		bool Get_IsCbmDirectoryLoaded() const;
		void Set_CbmDirectoryLoaded(bool isLoaded);
		bool Get_IsCbmDirectorySuccessfullyLoaded() const;
		void Set_CbmDirectorySuccessfullyLoaded(bool isSuccess);
		int Get_IndexCurrentCbmDiskItem() const;
		void Set_IndexCurrentCbmDiskItem(int index);
		bool Get_IsCbmDiskTitleSelected() const;
		void Set_IsCbmDiskTitleSelected(int isSelected);
		bool Get_IsQuickloadEnabled() const;
		void Set_IsQuickloadEnabled(int isEnabled);
		void ClearCbmDirectory();
		HRESULT LoadCbmDirectory(const TCHAR filename[], int maxcount, int& count, bool bPrgFilesOnly, HANDLE hevtQuit);
		C64File c64file;

		std::vector<DirectoryItem> CurrentDirectoryItems;
		std::vector<DirectoryItem> ParentFullPath;
		std::vector<std::wstring> FileExtensionFilterList;
	private:
		bool isCbmDiskTitleSelected = false;
		bool isCbmDirectoryLoaded = false;
		bool isCbmDirectorySuccessfullyLoaded = false;
		bool isQuickloadEnabled = false;
		int indexCurrentCbmDiskItem = false;
	};

}