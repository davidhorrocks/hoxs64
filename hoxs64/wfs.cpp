#include <string>
#include "StringConverter.h"
#include "wfs.h"
#include "utils.h"

const std::wstring Wfs::LongNamePrefixUnc(L"\\\\?\\UNC\\");
const std::wstring Wfs::LongNamePrefix(L"\\\\?\\");
const std::wstring Wfs::ShortNamePrefixUnc(L"\\\\");
const std::wstring Wfs::RootPrefix(L"\\");

bool Wfs::IsAbsolutePath(const std::wstring& path)
{
    if (StringConverter::IsEmptyOrWhiteSpace(path))
    {
        return false;
    }

    if (path.find(L"\\") == 0 || path.find(L"/") == 0)
    {
        return true;
    }

    size_t i = path.find(L":");
    if (i > 0)
    {
        if (path.length() > i + 1)
        {
            if (path[i + 1] == L'/' || path[i + 1] == L'\\')
            {
                return true;
            }
        }
    }

    return false;
}

std::wstring Wfs::EnsureLongNamePrefix(const std::wstring& path)
{
    if (!IsAbsolutePath(path))
    {
        return std::wstring(path);
    }
    else if (StringConverter::IsEmptyOrWhiteSpace(path) || path.find(Wfs::LongNamePrefixUnc) == 0 || path.find(Wfs::LongNamePrefix) == 0)
    {
        return std::wstring(path);
    }
    else if (path.find(L"\\\\") == 0)
    {
        size_t k = 2;
        return Wfs::LongNamePrefixUnc + path.substr(k, path.length() - k);
    }
    else
    {
        return Wfs::LongNamePrefix + path;
    }
}

std::wstring Wfs::EnsureRemoveLongNamePrefix(const std::wstring& path)
{
    if (StringConverter::IsEmptyOrWhiteSpace(path))
    {
        return std::wstring(path);
    }
    else if (path.find(Wfs::LongNamePrefixUnc) == 0)
    {
        size_t k = Wfs::LongNamePrefixUnc.length();
        return L"\\\\" + path.substr(k, path.length() - k);
    }
    else if (path.find(Wfs::LongNamePrefix) == 0)
    {
        size_t k = Wfs::LongNamePrefix.length();
        return path.substr(k, path.length() - k);
    }
    else
    {
        return std::wstring(path);
    }
}

void Wfs::EnsureTrailingBackslash(std::wstring& s)
{
    if (s.length() == 0 || s.back() != '\\')
    {
        s.push_back('\\');
    }
}

void Wfs::RemoveTrailingBackslash(std::wstring& s)
{
    while (s.length() > 0 && s.back() == '\\')
    {
        s.pop_back();
    }
}

void Wfs::Path_Append(std::wstring& pathToBeChanged, const std::wstring& pathToBeAppended)
{
    if (StringConverter::IsEmptyOrWhiteSpace(pathToBeChanged))
    {
        pathToBeChanged = pathToBeAppended;
        return;
    }

    if (StringConverter::IsEmptyOrWhiteSpace(pathToBeAppended))
    {
        return;
    }

    if (pathToBeChanged.back() != '\\')
    {
        if (pathToBeAppended.front() != '\\')
        {
            pathToBeChanged.append(L"\\" + pathToBeAppended);
        }
        else
        {
            pathToBeChanged.append(pathToBeAppended);
        }
    }
    else
    {
        if (pathToBeAppended.front() != '\\')
        {
            pathToBeChanged.append(pathToBeAppended);
        }
        else
        {
            int k = 1;
            pathToBeChanged.append(pathToBeAppended.substr(k, pathToBeAppended.length() - k));
        }
    }
}

std::wstring Wfs::Path_Combine(const std::wstring& path1, const std::wstring& path2)
{
    if (StringConverter::IsEmptyOrWhiteSpace(path1))
    {
        return path2;
    }

    if (StringConverter::IsEmptyOrWhiteSpace(path2))
    {
        return path1;
    }

    if (path1.back() != '\\')
    {
        if (path2.front() != '\\')
        {
            return path1 + L"\\" + path2;
        }
        else
        {
            return path1 + path2;
        }
    }
    else
    {
        if (path2.front() != '\\')
        {
            return path1 + path2;
        }
        else
        {
            int k = 1;
            return path1 + path2.substr(k, path2.length() - k);
        }
    }
}

bool Wfs::FileExists(const std::wstring& path, bool* pisFound, DWORD* plastError)
{
    DWORD dwAttr = ::GetFileAttributes(Wfs::EnsureLongNamePrefix(path).c_str());
    DWORD lastError = 0;
    bool isError = false;
    bool found = false;
    if (dwAttr == INVALID_FILE_ATTRIBUTES)
    {
        lastError = ::GetLastError();
        if (lastError == ERROR_FILE_NOT_FOUND)
        {
            found = false;
        }
        else if (lastError == ERROR_PATH_NOT_FOUND)
        {
            found = false;
        }
        else if (lastError == ERROR_ACCESS_DENIED)
        {
            isError = true;
            found = false;
        }
        else
        {
            isError = true;
            found = false;
        }
    }
    else
    {
        if ((dwAttr & FILE_ATTRIBUTE_DIRECTORY) != 0)
        {
            found = false;
        }
        else
        {
            found = true;
        }
    }

    if (plastError)
    {
        *plastError = lastError;
    }

    if (pisFound)
    {
        *pisFound = pisFound;
    }

    return !isError;
}

bool Wfs::FileExists(const std::wstring& path, bool* pisFound, std::wstring& errorMessage)
{
    DWORD lastError;
    bool isFound;
    bool isSuccess = FileExists(path, &isFound, &lastError);
    if (!isSuccess)
    {
        errorMessage = G::GetLastWin32ErrorWString(lastError);
    }

    return isSuccess;
}

std::wstring Wfs::GetFileExtension(const std::wstring& filename)
{
    std::wstring s;
    if (StringConverter::IsEmptyOrWhiteSpace(filename))
    {
        return s;
    }

    size_t i = filename.find_last_of(L'.');
    if (i == std::wstring::npos)
    {
        return s;
    }

    return filename.substr(i, filename.length() - i);
}

bool Wfs::FilenameHasExtension(const wchar_t filename[], const wchar_t ext[])
{
    if (filename == nullptr || ext == nullptr)
    {
        return false;
    }

    size_t filenameLength = _tcslen(filename);
    size_t extLength = _tcslen(ext);

    size_t i = 0;
    size_t j = 0;
    const TCHAR* p;
    const TCHAR* q;
    if (filenameLength >= 0 && extLength >= 0)
    {
        p = &ext[extLength - 1];
        q = &filename[filenameLength - 1];
        for (i = extLength, j = filenameLength; i != 0 && j != 0; i--, j--, p--, q--)
        {
            if (std::toupper(*p) != std::toupper(*q))
            {
                return false;
            }
        }

        if (i == 0)
        {
            return true;
        }
    }

    return false;
}

void Wfs::SplitRootPath(const std::wstring path, std::wstring& root, std::wstring& directorypath, std::wstring& filename)
{
    std::wstring sb;
    size_t i;
    int scount;
    directorypath.clear();
    root.clear();
    if (path.find(Wfs::LongNamePrefixUnc) == 0)
    {
        sb.append(Wfs::LongNamePrefixUnc);
        scount = 2;
        for (i = sb.length(); i < path.length() && scount > 0; i++)
        {
            switch (path[i])
            {
            case '\\':
                scount--;
                break;
            }

            if (scount > 0)
            {
                sb.push_back(path[i]);
            }
        }

        if (sb.length() < path.length())
        {
            size_t k = path.length() - sb.length();
            directorypath = path.substr(path.length() - k, k);
        }
    }
    else if (path.find(Wfs::LongNamePrefix) == 0)
    {
        sb.append(Wfs::LongNamePrefix);
        scount = 1;
        for (i = sb.length(); i < path.length() && scount > 0; i++)
        {
            switch (path[i])
            {
            case ':':
                scount--;
                break;
            }

            sb.push_back(path[i]);
        }

        if (sb.length() < path.length())
        {
            size_t k = path.length() - sb.length();
            directorypath = path.substr(path.length() - k, k);
        }
    }
    else if (path.find(Wfs::ShortNamePrefixUnc) == 0)
    {
        sb.append(Wfs::ShortNamePrefixUnc);
        scount = 2;
        for (i = sb.length(); i < path.length() && scount > 0; i++)
        {
            switch (path[i])
            {
            case '\\':
                scount--;
                break;
            }

            if (scount > 0)
            {
                sb.push_back(path[i]);
            }
        }

        if (sb.length() < path.length())
        {
            size_t k = path.length() - sb.length();
            directorypath = path.substr(path.length() - k, k);
        }
    }
    else if (path.find(Wfs::RootPrefix) == 0)
    {
        sb.append(Wfs::RootPrefix);
        scount = 2;
        for (i = sb.length(); i < path.length() && scount > 0; i++)
        {
            switch (path[i])
            {
            case '\\':
                scount--;
                break;
            }

            if (scount > 0)
            {
                sb.push_back(path[i]);
            }
        }

        if (sb.length() < path.length())
        {
            size_t k = path.length() - sb.length();
            directorypath = path.substr(path.length() - k, k);
        }
    }
    else
    {
        scount = 1;
        for (i = sb.length(); i < path.length() && scount > 0; i++)
        {
            switch (path[i])
            {
            case ':':
                scount--;
                break;
            }

            sb.push_back(path[i]);
        }

        if (sb.length() < path.length())
        {
            size_t k = path.length() - sb.length();
            directorypath = path.substr(path.length() - k, k);
        }
    }

    root = sb;

    i = directorypath.find_last_of('\\');
    if (i != std::wstring::npos)
    {
        size_t k = i + 1;
        if (directorypath.length() > k)
        {
            filename = directorypath.substr(k, directorypath.length() - k);
            directorypath.resize(k);
        }
    }
}
