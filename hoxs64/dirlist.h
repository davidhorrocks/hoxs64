#pragma once
#include <Windows.h>
#include <string>
#include "C64File.h"

namespace FileSys
{
    enum DirectoryItemType
    {
        None,
        VolumePath,
        DriveLetter,
        DirectoryFileItem,
        ParentDirectory,
    };

    class DirectoryItem
    {
    public:
        DirectoryItem() = default;
        DirectoryItem(const DirectoryItem&) = default;
        DirectoryItem(DirectoryItem&&) = default;
        ~DirectoryItem() = default;
        DirectoryItem& operator=(const DirectoryItem&) = default;
        DirectoryItem& operator=(DirectoryItem&&) = default;

        const std::wstring& GetNameW() const;
        const std::string& GetNameA() const;
        bool IsDirectory() const;
        bool IsCommodore64File() const;
        bool IsCommodore64Disk() const;
        bool IsCommodore64Tape() const;
        bool IsCommodore64Sid() const;
        void Clear();

        DirectoryItemType Type = DirectoryItemType::None;
        WIN32_FIND_DATAW Find_data = {};
        std::wstring VolumeName;
        std::wstring VolumePathItemName;
        CHAR DriveLetter = '?';
        mutable bool hasNameW = false;
        mutable bool hasNameA = false;
        mutable std::wstring NameW;
        mutable std::string NameA;

        static const wchar_t* const AllC64Extenstions[11];
        static const wchar_t* const DiskC64Extenstions[5];
        static const wchar_t* const TapeC64Extenstions[1];
        static const wchar_t* const SidC64Extenstions[1];
    };

    class IEnumDirectoryItem
    {
    public:
        IEnumDirectoryItem() = default;
        IEnumDirectoryItem(const IEnumDirectoryItem&) = default;
        IEnumDirectoryItem(IEnumDirectoryItem&&) = default;
        virtual ~IEnumDirectoryItem() = default;
        IEnumDirectoryItem& operator=(const IEnumDirectoryItem&) = default;
        IEnumDirectoryItem& operator=(IEnumDirectoryItem&&) = default;
        virtual bool GetNext(DirectoryItem* pitem, bool *pisEndOfList, std::wstring message) = 0;
        virtual void Reset() = 0;
    };
     
    class EnumDirectoryItemImpl : public IEnumDirectoryItem
    {
    public:
        EnumDirectoryItemImpl() = default;
        EnumDirectoryItemImpl(const EnumDirectoryItemImpl&) = default;
        EnumDirectoryItemImpl(EnumDirectoryItemImpl&&) = default;
        EnumDirectoryItemImpl(const wchar_t* pDirectoryName);
        ~EnumDirectoryItemImpl();
        EnumDirectoryItemImpl& operator=(const EnumDirectoryItemImpl&) = default;
        EnumDirectoryItemImpl& operator=(EnumDirectoryItemImpl&&) = default;
        bool GetNext(DirectoryItem* pitem, bool* pisEndOfList, std::wstring message) override;
        void Reset() noexcept override;
    private:
        std::wstring directoryName;
        HANDLE ffhandle;
    };

    class EnumVolumePathItemImpl : public IEnumDirectoryItem
    {
    public:
        EnumVolumePathItemImpl() noexcept;
        ~EnumVolumePathItemImpl();
        EnumVolumePathItemImpl(const EnumVolumePathItemImpl&) = default;
        EnumVolumePathItemImpl(EnumVolumePathItemImpl&&) = default;
        EnumVolumePathItemImpl& operator=(const EnumVolumePathItemImpl&) = default;
        EnumVolumePathItemImpl& operator=(EnumVolumePathItemImpl&&) = default;
        bool GetNext(DirectoryItem* pitem, bool* pisEndOfList, std::wstring message) override;
        void Reset() noexcept override;
    private:
        bool GetFirstVolumePathItem(const wchar_t* pwsVolumeName, bool* pisEndOfList, wchar_t** ppPathItem);
        bool GetNextVolumePathItem(bool* pisEndOfList, wchar_t** ppPathItem);

        HANDLE volumehandle;
        std::wstring currentVolumeName;
        PWCHAR Names = NULL;
        PWCHAR NameIdx;
        DWORD CharCount;
    };

    class EnumDriveItemImpl : public IEnumDirectoryItem
    {
    public:
        EnumDriveItemImpl() noexcept;
        ~EnumDriveItemImpl();
        EnumDriveItemImpl(const EnumDriveItemImpl&) = default;
        EnumDriveItemImpl(EnumDriveItemImpl&&) = default;
        EnumDriveItemImpl& operator=(const EnumDriveItemImpl&) = default;
        EnumDriveItemImpl& operator=(EnumDriveItemImpl&&) = default;
        bool GetNext(DirectoryItem* pitem, bool* pisEndOfList, std::wstring message) override;
        void Reset() noexcept override;
    private:
        unsigned int bitIndex;
    };

    class DirList
    {
    public:
        IEnumDirectoryItem *CreateDirectoryEnumerator(const wchar_t* pszDirectory);
        IEnumDirectoryItem* CreateDriveEnumerator();
        IEnumDirectoryItem* CreateVolumePathEnumerator();
    };

}
