//
// Created by Chiara on 17/08/2016.
// 
// Modified by David Horrocks 03/12/2020
// The parser now support the following new features.
// Line continuations.
// Backslash character escape codes.
// Quoted strings.
// Values are now parsed as a comma separated list of values.
// Improved comment handling.

#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include "..\StringConverter.h"
#include "..\hexconv.h"
#include "IniFileManager.h"

bool Section::operator <(const Section& other) const
{
    return this->name < other.name;
}

bool KeyValueAssign::operator<(const KeyValueAssign& other) const
{
    return this->name < other.name;
}

void KeyValueAssign::clear()
{
    name.clear();
    values.clear();
    comment.clear();
}

const char IniFileManager::bomUtf8[3] = { '\xEF', '\xBB', '\xBF'};

void IniFileManager::clearLastError()
{
    lastErrorMessage.erase();
}

bool IniFileManager::parse(const std::wstring& filename)
{
    lineNumber = 1;
    clearLastError();
    bool isBOM_Utf8 = false;
    std::ifstream file;
    file.open(filename, std::ios_base::in);
    if (file.fail())
    {
        std::wostringstream os;
        os << "Unable to open " << filename << std::endl;
        //std::wcout << os.str();
        this->lastErrorMessage.append(os.str());
        return false;
    }

    char bom_buffer[BOM_COUNT_UTF8];
    file.read(bom_buffer, BOM_COUNT_UTF8);
    if (file.gcount() == BOM_COUNT_UTF8)
    {
        if (memcmp(bom_buffer, bomUtf8, BOM_COUNT_UTF8) == 0)
        {
            isBOM_Utf8 = true;
        }
    }

    if (!isBOM_Utf8)
    {
        if (!file.seekg(0))
        {
            return false;
        }
    }

    std::vector<std::wstring> listOfValues;
    //Variable handling one section at a time:
    Section currentSection;
    KeyValueAssign currentSetting;

    //Parsing: file is read line by line:
    std::string line;

    IniToken token;
    token.Init();
    bool iserror = false;
    bool iseof = false;
    while (file.good() && token.IsOK() && !iserror)
    {
        if (!readNextToken(file, true, false, token))
        {
            iserror = true;
            break;
        }

        if (token.TokenType == IniToken::ATokenType::SectionName)
        {
            if (!currentSection.name.empty()) {

                //If so, the previous one is over, and it can store it in the list:
                sections.push_back(currentSection);

                //Clears data for a reuse of the variable:
                currentSection.name.clear();
                currentSection.settings.clear();
                currentSection.comments.clear();
            }

            //Gets the name of the section, removing '[' and ']':
            currentSection.name = StringConverter::StringToWideString(token.StringText);
        }
        else if (token.TokenType == IniToken::ATokenType::StringValue)
        {
            if (currentSection.name.empty())
            {
                std::wostringstream os;
                os << "Key " << StringConverter::StringToWideString(token.StringText) << " does not have a section header." << StringConverter::StringToWideString(token.ErrorText) << std::endl;
                ////std::wcout << os.str();
                this->lastErrorMessage.append(os.str());
                iserror = true;
                continue;
            }

            currentSetting.clear();
            if (token.StringText.empty())
            {
                token.SetErrorSyntax();
                break;
            }

            currentSetting.name = StringConverter::StringToWideString(CP_UTF8, token.StringText);
            if (!readNextToken(file, false, false, token))
            {
                iserror = true;
            }

            if (token.TokenType == IniToken::ATokenType::Symbol && token.StringText == "=")
            {
                bool wantValue = true;
                while (file.good() && token.IsOK() && !iserror)
                {
                    if (!readNextToken(file, false, wantValue, token))
                    {
                        iserror = true;
                    }

                    if (token.TokenType == IniToken::ATokenType::Symbol && token.StringText == ",")
                    {
                        if (wantValue)
                        {
                            currentSetting.values.push_back(L"");
                        }

                        wantValue = true;
                    }
                    else if (token.TokenType == IniToken::ATokenType::StringValue)
                    {
                        if (wantValue)
                        {
                            currentSetting.values.push_back(StringConverter::StringToWideString(CP_UTF8, token.StringText));
                            wantValue = false;
                        }
                    }
                    else if (token.TokenType == IniToken::ATokenType::Comment)
                    {
                        if (wantValue)
                        {
                            currentSetting.values.push_back(L"");
                        }

                        currentSetting.comment = StringConverter::StringToWideString(CP_UTF8, token.StringText);
                        break;
                    }
                    else if (token.TokenType == IniToken::ATokenType::NewLine)
                    {
                        if (wantValue)
                        {
                            currentSetting.values.push_back(L"");
                        }

                        break;
                    }
                    else if (token.IsEOF())
                    {
                        if (wantValue)
                        {
                            currentSetting.values.push_back(L"");
                            wantValue = false;
                        }

                        break;
                    }
                    else
                    {
                        token.SetErrorSyntax();
                        break;
                    }
                }

                if (file.fail() || token.IsError() || iserror)
                {
                    break;
                }

                currentSection.settings.push_back(currentSetting);
            }
            else
            {
                token.SetErrorSyntax();
                break;
            }
        }
        else if (token.TokenType == IniToken::ATokenType::Comment)
        {
            currentSection.comments.push_back(StringConverter::StringToWideString(CP_UTF8, token.StringText));
        }
        else if (token.TokenType == IniToken::ATokenType::Symbol)
        {
            token.SetErrorSymbol();
            break;
        }
        else if (token.TokenType == IniToken::ATokenType::EndOfInput)
        {
            break;
        }
        else if (token.TokenType == IniToken::ATokenType::NewLine)
        {
            continue;
        }
        else if (token.IsError())
        {
            break;
        }
        else
        {
            iserror = true;
            break;
        }
    }

    //When no more lines are left, it stores the last section being handled into the list:
    if (!currentSection.name.empty()) {

        sections.push_back(currentSection);
    }

    if (token.IsError() || iserror)
    {
        iserror = true;
        std::wostringstream os;
        os << "Unable to parse the ini file at line " << lineNumber << "." << std::endl;
        if (token.IsError())
        {
            os << StringConverter::StringToWideString(token.ErrorText) << std::endl;
        }

        //std::wcout << os.str();
        this->lastErrorMessage.append(os.str());
    }
    else if (file.fail())
    {
        iserror = true;
        std::wostringstream os;
        os << "A file read error occured while reading the ini file." << std::endl;
        //std::wcout << os.str();
        this->lastErrorMessage.append(os.str());
    }

    if (iserror)
    {
        sections.clear();
    }

    file.close();
    return !iserror;
}

bool IniFileManager::readNextToken(std::basic_istream<char>& strm, bool acceptKeyName, bool acceptKeyValue, IniToken& token)
{
    bool isCommentAllowed = acceptKeyName && !acceptKeyValue;
    token.Init();
    while (strm.good())
    {
        int ch = strm.peek();
        if (ch < 0)
        {
            break;
        }

        if (ch == '\r')
        {
            ch = strm.get();
            if (strm.peek() == '\n')
            {
                ch = strm.get();
            }

            ++lineNumber;
            token.TokenType = IniToken::ATokenType::NewLine;
            return true;
        }
        else if (ch == '\n')
        {
            strm.get();
            ++lineNumber;
            token.TokenType = IniToken::ATokenType::NewLine;
            return true;
        }

        if (isWhiteSpace(ch))
        {
            isCommentAllowed = true;
            strm.get();
            continue;
        }

        if (ch == '\\')
        {
            // line continuation
            strm.get();
            ch = strm.peek();
            if (ch == '\r')
            {
                strm.get();
                if (strm.peek() == '\n')
                {
                    strm.get();
                }

                ++lineNumber;
                continue;
            }
            else if (ch == '\n')
            {
                strm.get();
                ++lineNumber;
                continue;
            }
            else if (ch < 0)
            {
                break;
            }
            else
            {
                token.SetErrorSyntax();
                return false;
            }
        }
        else if (isCommentAllowed && (ch == ';' || ch == '#'))
        {
            strm.get();
            token.TokenType = IniToken::ATokenType::Comment;
            while (strm.good())
            {
                ch = strm.peek();
                if (ch < 0)
                {
                    if (strm.fail())
                    {
                        token.SetErrorFile();
                    }

                    return true;
                }

                if (ch == '\\')
                {
                    // line continuation
                    strm.get();
                    ch = strm.peek();
                    if (ch < 0)
                    {
                        if (strm.fail())
                        {
                            token.SetErrorFile();
                        }

                        return true;
                    }

                    if (ch == '\r')
                    {
                        strm.get();
                        if (strm.peek() == '\n')
                        {
                            strm.get();
                        }

                        lineNumber++;
                    }
                    else if (ch == '\n')
                    {
                        strm.get();
                        lineNumber++;
                    }
                    else
                    {
                        token.SetErrorSyntax();
                        return false;
                    }

                    continue;
                }
                else if (ch == '\r' || ch == '\n')
                {
                    return true;
                }
                else
                {
                    strm.get();
                    if (token.StringText.length() <= IniToken::MaxStringLength)
                    {
                        token.StringText.append(1, (char)ch);
                    }
                }
            }

            return true;
        }
        else if (ch == '=')
        {
            if (acceptKeyValue)
            {
                return getStringValue(strm, false, true, token);
            }
            else
            {
                strm.get();
                token.TokenType = IniToken::ATokenType::Symbol;
                token.StringText.append(1, (char)ch);
            }

            return true;
        }
        else if (ch == ',')
        {
            strm.get();
            token.TokenType = IniToken::ATokenType::Symbol;
            token.StringText.append(1, (char)ch);
            return true;
        }
        else
        {
            if (acceptKeyName)
            {
                if (ch == '[')
                {
                    strm.get();
                    token.TokenType = IniToken::ATokenType::SectionName;
                    while (!strm.eof())
                    {
                        ch = strm.peek();
                        if (ch < 0)
                        {
                            if (strm.fail())
                            {
                                token.SetErrorFile();
                            }

                            break;
                        }

                        if (ch == ']')
                        {
                            strm.get();
                            removeMarginWhites(token.StringText);
                            return true;
                        }

                        if (ch == '\r')
                        {
                            strm.get();
                            if (strm.peek() == '\n')
                            {
                                strm.get();
                            }

                            ++lineNumber;
                            token.SetErrorMissingSectionEndChar();
                            return false;
                        }
                        else if (ch == '\n')
                        {
                            strm.get();
                            ++lineNumber;
                            token.SetErrorMissingSectionEndChar();
                            return false;
                        }

                        if (isWhiteSpace(ch))
                        {
                            strm.get();
                            continue;
                        }

                        if (!isSectionNameChar(ch))
                        {
                            token.SetError();
                            return false;
                        }

                        strm.get();
                        token.StringText.append(1, (char)ch);
                        if (token.StringText.length() > IniToken::MaxStringLength)
                        {
                            token.SetErrorStringTooLong();
                            return false;
                        }
                    }

                    token.SetErrorMissingSectionEndChar();
                    return false;
                }

                if (isKeyNameChar(ch))
                {
                    return getStringValue(strm, true, false, token);
                }
                else
                {
                    token.SetError();
                    return false;
                }
            }
            else if (acceptKeyValue)
            {
                if (isKeyNameChar(ch) || ch == '\"')
                {
                    return getStringValue(strm, false, true, token);
                }
                else
                {
                    token.SetError();
                    return false;
                }
            }
            else
            {
                token.SetError();
                return false;
            }
        }
    }

    if (strm.eof())
    {
        if (token.TokenType == IniToken::ATokenType::None)
        {
            token.SetEOF();
        }

        return true;
    }
    else if (strm.fail())
    {
        token.SetErrorFile();
        return false;
    }

    return true;
}

bool IniFileManager::getStringValue(std::basic_istream<char>& strm, bool acceptKeyName, bool acceptKeyValue, IniToken& token)
{
    constexpr unsigned int NumBufferDigits = 4;
    constexpr unsigned int NumOctDigits = 3;
    constexpr unsigned int NumHexDigits = 2;
    char buffer[NumBufferDigits];
    char escaped_byte;
    bool isContinuation = false;
    token.Init();
    token.TokenType = IniToken::ATokenType::StringValue;
    char ch = strm.peek();
    if (ch == '\"')
    {
        strm.get();
        while (strm.good())
        {
            if (token.StringText.length() > IniToken::MaxStringLength)
            {
                token.SetErrorStringTooLong();
                return false;
            }

            ch = strm.peek();
            if (ch < 0)
            {
                if (strm.fail())
                {
                    token.SetErrorFile();
                }
                else
                {
                    token.SetErrorMissingDoubleQuoteChar();
                }

                return false;
            }

            if (ch == '\"')
            {
                strm.get();
                return true;
            }

            if (ch == '\r' || ch == '\n')
            {
                token.SetErrorMissingDoubleQuoteChar();
                return false;
            }
            else if (ch == '\t')
            {
                ch = strm.get();
                token.StringText.append(1, ch);
                return false;
            }
            else if (ch == '\\')
            {
                ch = strm.get();
                if (ch < 0)
                {
                    if (strm.fail())
                    {
                        token.SetErrorFile();
                    }
                    else
                    {
                        token.SetErrorMissingDoubleQuoteChar();
                    }

                    return false;
                }

                ch = strm.peek();
                switch (ch)
                {
                case '\r':
                    // line continuation
                    strm.get();
                    if (strm.peek() == '\n')
                    {
                        strm.get();
                    }

                    lineNumber++;
                    break;
                case '\n':
                    // line continuation
                    strm.get();
                    lineNumber++;
                    break;
                case 'a':
                    strm.get();
                    token.StringText.append(1, '\a');
                    break;
                case 'b':
                    strm.get();
                    token.StringText.append(1, '\b');
                    break;
                case 'f':
                    strm.get();
                    token.StringText.append(1, '\f');
                    break;
                case 'r':
                    strm.get();
                    token.StringText.append(1, '\r');
                    break;
                case 'n':
                    strm.get();
                    token.StringText.append(1, '\n');
                    break;
                case 'v':
                    strm.get();
                    token.StringText.append(1, '\v');
                    break;
                case '\'':
                    strm.get();
                    token.StringText.append(1, '\'');
                    break;
                case '\"':
                    strm.get();
                    token.StringText.append(1, '\"');
                    break;
                case '\\':
                    strm.get();
                    token.StringText.append(1, '\\');
                    break;
                case '?':
                    strm.get();
                    token.StringText.append(1, '?');
                    break;
                case 'o':
                case 'O':
                    strm.get();
                    unsigned int octdigit;
                    for (octdigit = 0; octdigit < NumOctDigits; octdigit)
                    {
                        ch = strm.peek();
                        if (ch < 0)
                        {
                            if (strm.fail())
                            {
                                token.SetErrorFile();
                            }
                            else
                            {
                                token.SetErrorSyntax();
                            }

                            return false;
                        }

                        if (!isOctalDigit(ch))
                        {
                            token.SetErrorSyntax();
                            return false;
                        }

                        ch = strm.get();
                        buffer[octdigit] = ch;
                    }

                    buffer[octdigit] = 0;
                    escaped_byte = 0;
                    for (octdigit = 0; octdigit < NumOctDigits; octdigit)
                    {
                        escaped_byte <<= 3;
                        escaped_byte |= (buffer[octdigit] & 7);
                    }

                    token.StringText.append(1, escaped_byte);
                    break;
                case 'x':
                case 'X':
                    strm.get();
                    unsigned int hexdigit;
                    for (hexdigit = 0; hexdigit < NumHexDigits; hexdigit)
                    {                        
                        ch = strm.peek();
                        if (ch < 0)
                        {
                            if (strm.fail())
                            {
                                token.SetErrorFile();
                            }
                            else
                            {
                                token.SetErrorSyntax();
                            }

                            return false;
                        }

                        if (!isHexDigit(ch))
                        {
                            token.SetErrorSyntax();
                            return false;
                        }

                        ch = strm.get();
                        buffer[hexdigit] = ch;
                    }

                    buffer[hexdigit] = 0;
                    escaped_byte = (HexConv<char>::hex_to_long(buffer, NumHexDigits) & 0xff);
                    token.StringText.append(1, escaped_byte);
                    break;
                default:
                    token.SetError("Unsupported escape sequence.");
                    return false;
                }
            }
            else if (!isQuotedStringEscapeRequired(ch))
            {
                ch = strm.get();
                token.StringText.append(1, (char)ch);
            }
            else
            {
                token.SetErrorSymbol(ch);
                return false;
            }
        }

        token.SetErrorMissingDoubleQuoteChar();
        return false;
    }
    else
    {
        if (isAcceptChar(ch, acceptKeyName, acceptKeyValue))
        {
            token.TokenType = IniToken::ATokenType::StringValue;
            ch = strm.get();
            token.StringText.append(1, (char)ch);
            while (strm.good())
            {
                if (token.StringText.length() > IniToken::MaxStringLength)
                {
                    token.SetErrorStringTooLong();
                    return false;
                }

                ch = strm.peek();
                if (ch < 0)
                {
                    if (strm.fail())
                    {
                        token.SetErrorFile();
                        return false;
                    }

                    break;
                }

                if (ch == '\\')
                {
                    // line continuation
                    ch = strm.get();
                    if (ch < 0)
                    {
                        if (strm.fail())
                        {
                            token.SetErrorFile();
                            return false;
                        }

                        break;
                    }

                    ch = strm.peek();
                    if (ch != '\r' && ch != '\n')
                    {
                        token.SetErrorSyntax();
                        return false;
                    }

                    if (ch == '\r')
                    {
                        strm.get();
                        if (strm.peek() == '\n')
                        {
                            strm.get();
                        }

                        lineNumber++;
                    }
                    else if (ch == '\n')
                    {
                        strm.get();
                        lineNumber++;
                    }

                    continue;
                }
                else if (isAcceptChar(ch, acceptKeyName, acceptKeyValue))
                {
                    ch = strm.get();
                    token.StringText.append(1, (char)ch);
                }
                else
                {
                    return true;
                }
            }
        }
    }

    return true;
}

bool IniFileManager::read(const std::wstring& whichSection, const std::wstring& whichKey, KeyValueAssign** pkeyValue) 
{
    auto itr = getSection(whichSection);

    if (itr == sections.end()) 
    {
        return false;
    }
    else 
    {
        Section& section = (*itr);
        auto search = std::find_if(section.settings.begin(), section.settings.end(), [&](KeyValueAssign x) -> bool
            {
                return x.name == whichKey;
            });

        //If no key is found:
        if (search == section.settings.end())
        {
            return false;
        }
        else 
        {
            if (pkeyValue != nullptr)
            {
                *pkeyValue = &(*search);
            }

            return true;
        }
    }
}

void IniFileManager::editKeyValue(const std::wstring& whichSection, const std::wstring& whichKey, const std::wstring& userValue) 
{
    auto itr = getSection(whichSection);
    if (itr == sections.cend()) 
    {
        std::wostringstream os;
        os << "Could not find any section named " << whichSection << " in your file ." << std::endl << std::endl;
        //std::wcout << os.str();
        this->lastErrorMessage.append(os.str());
        return;
    }

    Section& section = (*itr);
    if (section.name == whichSection)
    {
        auto search = std::find_if(section.settings.begin(), section.settings.end(), [&](KeyValueAssign x) -> bool
            {
                return x.name == whichKey;
            });

        //If no key is found:
        if (search == section.settings.end()) 
        {
            std::wostringstream os;
            os << "Could not find any key named " << whichKey << " in section " << section.name << " of your file." << std::endl;
            //std::wcout << os.str();
            this->lastErrorMessage.append(os.str());
            return;
        } 
        else 
        {
            KeyValueAssign& key = (*search);
            //If the key is found, the value is modified:
            key.values.clear();
            key.values.push_back(userValue);
            copyIsNecessary = true;
        }
    }
}

void IniFileManager::editKeyValues(const std::wstring& whichSection, const std::wstring& whichKey, const std::vector<std::wstring>& userValues)
{
    //Removes all whitespaces from user input:
    auto itr = getSection(whichSection);
    if (itr == sections.cend())
    {
        std::wostringstream os;
        os << "Could not find any section named " << whichSection << " in your file ." << std::endl;
        //std::wcout << os.str();
        this->lastErrorMessage.append(os.str());
        return;
    }

    Section& section = (*itr);
    if (section.name == whichSection)
    {

        auto search = std::find_if(section.settings.begin(), section.settings.end(), [&](KeyValueAssign x) -> bool
            {
                return x.name == whichKey;
            });

        //If no key is found:
        if (search == section.settings.end())
        {
            std::wostringstream os;
            os << "Could not find any key named " << whichKey << " in section " << section.name << " of your file." << std::endl;
            //std::wcout << os.str();
            this->lastErrorMessage.append(os.str());
            return;
        }
        else
        {
            KeyValueAssign& key = (*search);
            //If the key is found, the value is modified:
            key.values.clear();
            key.values.assign(userValues.begin(), userValues.end());
            copyIsNecessary = true;
        }
    }
}

void IniFileManager::removeSpaces(std::wstring& s)
{
    s.erase(std::remove(s.begin(), s.end(), ' '), s.end());
}

void IniFileManager::saveFile(const std::wstring& filename)
{
    _wremove(filename.c_str());
    generateCopy(filename);
}

void IniFileManager::generateCopy(const std::wstring& filename) {
    // Create new file
    std::ofstream newFile(filename);
    auto sectionItr = sections.begin();
    if (sectionItr != sections.end())
    {        
        // Loop sections
        for (;sectionItr != sections.end(); sectionItr++)
        {
            const Section& section = *sectionItr;
            newFile << "[" + StringConverter::WideStringToString(CP_UTF8, section.name) + "]" << std::endl;

            // Loop comments
            for (auto comItr = (section.comments).begin(); comItr != (section.comments).end(); comItr++)
            {
                newFile << StringConverter::WideStringToString(CP_UTF8, (*comItr)) << std::endl;
            }

            std::wstring values;
            //Loop settings
            auto settingsItr = section.settings.begin();
            for (; settingsItr != section.settings.end(); settingsItr++)
            {
                const KeyValueAssign& ka = *settingsItr;
                newFile << StringConverter::WideStringToString(CP_UTF8, ka.name) << "=";
                unsigned long long c = 0;
                auto valuesItr = ka.values.begin();
                for (; valuesItr != ka.values.end(); valuesItr++, c++)
                {
                    if (c != 0)
                    {
                        newFile << ",";
                    }

                    std::string& valueUtf8 = StringConverter::WideStringToString(CP_UTF8, *valuesItr);
                    bool wantQuotes = false;
                    for (auto chItr = valueUtf8.cbegin(); chItr != valueUtf8.cend(); chItr++)
                    {
                        if (this->isQuotedStringEscapeRequired(*chItr))
                        {
                            wantQuotes = true;
                            break;
                        }
                    }

                    if (wantQuotes)
                    {
                        newFile << "\"";
                        for (auto chItr = valueUtf8.cbegin(); chItr != valueUtf8.cend(); chItr++)
                        {
                            char ch = *chItr;
                            switch (ch)
                            {
                            case '\r':
                                newFile << "\\r";
                                break;
                            case '\n':
                                newFile << "\\n";
                                break;
                            case '\a':
                                newFile << "\\a";
                                break;
                            case '\b':
                                newFile << "\\b";
                                break;
                            case '\f':
                                newFile << "\\f";
                                break;
                            case '\v':
                                newFile << "\\v";
                                break;
                            case '\"':
                                newFile << "\\\"";
                                break;
                            case '\\':
                                newFile << "\\\\";
                                break;
                            case '?':
                                newFile << "\\?";
                                break;
                            default:
                                if (this->isQuotedStringEscapeRequired(ch))
                                {
                                    newFile << StringConverter::format_string("\\%02hhX", ch);
                                }
                                else
                                {
                                    newFile << ch;
                                }

                                break;
                            }
                        }

                        newFile << "\"";
                    }
                    else
                    {
                        newFile << valueUtf8;
                    }
                }

                newFile << std::endl;
            }

            newFile << std::endl;
        }
    }

    newFile.close();
}

std::list<KeyValueAssign>::iterator IniFileManager::ensureKey(const std::wstring& whichSection, const std::wstring& whichKey)
{
    auto itr = ensureSection(whichSection);
    Section& section = (*itr);
    std::list<KeyValueAssign>::iterator search = std::find_if(section.settings.begin(), section.settings.end(), [&](KeyValueAssign x) -> bool
        {
            return x.name == whichKey;
        });

    //If not, adds the pair to the corresponding map:
    if (search == section.settings.end())
    {
        KeyValueAssign ka;
        ka.name = whichKey;
        section.settings.push_back(std::move(ka));
        search = section.settings.end();
        --search;
        copyIsNecessary = true;
    }

    return search;
}

void IniFileManager::ensureKeyValue(const std::wstring& whichSection, const std::wstring& whichKey, const std::wstring& userValue)
{
    auto itr = ensureSection(whichSection);
    Section& section = (*itr);
    auto search = std::find_if(section.settings.begin(), section.settings.end(), [&](KeyValueAssign x) -> bool
        {
            return x.name == whichKey;
        });

    //If no key is found:
    if (search == section.settings.end())
    {
        addKeyValue(whichSection, whichKey, userValue);
    }
    else
    {
        //If the key is found, the value is modified:
        KeyValueAssign& ka = *search;
        ka.values.clear();
        ka.values.push_back(userValue);
        copyIsNecessary = true;
    }
}

void IniFileManager::ensureKeyValues(const std::wstring& whichSection, const std::wstring& whichKey, const std::vector<std::wstring>& userValues)
{
    //Removes all whitespaces from user input:
    auto itr = ensureSection(whichSection);
    Section& section = (*itr);
    auto search = std::find_if(section.settings.begin(), section.settings.end(), [&](KeyValueAssign x) -> bool
        {
            return x.name == whichKey;
        });

    //If no key is found:
    if (search == section.settings.end())
    {
        addKeyValues(whichSection, whichKey, userValues);
    }
    else
    {
        KeyValueAssign& ka = *search;
        //If the key is found, the value is modified:
        ka.values.clear();
        ka.values.assign(userValues.begin(), userValues.end());
        copyIsNecessary = true;
    }
}

void IniFileManager::addKeyValue(const std::wstring& whichSection, const std::wstring& newKey, const std::wstring& newValue)
{
    //Removes all whitespaces from user input:
    auto itr = getSection(whichSection);
    if (itr == sections.end()) 
    {
        std::wostringstream os;
        os << "Could not find any section named " << whichSection << " in your file."<< std::endl;
        //std::wcout << os.str();
        this->lastErrorMessage.append(os.str());
    } 
    else if (itr->name == whichSection) 
    {
        Section& section = (*itr);
        auto search = std::find_if(section.settings.begin(), section.settings.end(), [&](KeyValueAssign x) -> bool
            {
                return x.name == newKey;
            });

        //If not, adds the pair to the corresponding map:
        if (search == section.settings.end()) 
        {
            KeyValueAssign ka;
            ka.name = newKey;
            ka.values.push_back(newValue);
            section.settings.push_back(std::move(ka));
            copyIsNecessary = true;
        } 
        else 
        {
            std::wostringstream os;
            os << "Cannot add line because provided key already exists." << std::endl;
            //std::wcout << os.str();
            this->lastErrorMessage.append(os.str());
        }
    }
}

void IniFileManager::addKeyValues(const std::wstring& whichSection, const std::wstring& newKey, const std::vector<std::wstring>& newValues)
{
    //Removes all whitespaces from user input:
    auto itr = getSection(whichSection);
    if (itr == sections.end())
    {
        std::wostringstream os;
        os << "Could not find any section named " << whichSection << " in your file." << std::endl;
        //std::wcout << os.str();
        this->lastErrorMessage.append(os.str());
    }
    else if (itr->name == whichSection)
    {
        Section& section = (*itr);
        auto search = std::find_if(section.settings.begin(), section.settings.end(), [&](KeyValueAssign x) -> bool
            {
                return x.name == newKey;
            });

        //If not, adds the pair to the corresponding map:
        if (search == section.settings.end())
        {
            KeyValueAssign ka;
            ka.name = newKey;
            ka.values.assign(newValues.begin(), newValues.end());
            section.settings.push_back(std::move(ka));
            copyIsNecessary = true;
        }
        else
        {
            std::wostringstream os;
            os << "Cannot add line because provided key already exists." << std::endl;
            //std::wcout << os.str();
            this->lastErrorMessage.append(os.str());

        }
    }
}

void IniFileManager::deleteKey(const std::wstring& whichSection, const std::wstring& whichKey) {

    auto itr = getSection(whichSection);
    if (itr == sections.end()) 
    {
        return;
    } 
    else 
    {
        Section& section = (*itr);
        if (section.name == whichSection)
        {
            auto search = std::find_if(section.settings.begin(), section.settings.end(), [&](KeyValueAssign x) -> bool
                {
                    return x.name == whichKey;
                });

            //If no key is found:
            if (search == section.settings.end())
            {
                return;
            }
            else
            {
                section.settings.erase(search);
                copyIsNecessary = true;
            }
        }
    }
}

void IniFileManager::addSection(const std::wstring& sectionName) 
{
    //Checks if a section with the same name already exists:
    auto itr = getSection(sectionName);

    if (itr == sections.end()) {
        Section newSection;
        newSection.name = sectionName;
        sections.push_back(newSection);
        copyIsNecessary = true;
    }
    else
    {
        std::wostringstream os;
        os << "A section named " << sectionName << " already exists." << std::endl;
        //std::wcout << os.str();
        this->lastErrorMessage.append(os.str());
    }
}

std::list<Section>::iterator IniFileManager::ensureSection(const std::wstring& sectionName)
{
    //Checks if a section with the same name already exists:
    auto itr = getSection(sectionName);

    if (itr == sections.end()) {
        Section newSection;
        newSection.name = sectionName;
        sections.push_back(newSection);
        itr = sections.end();
        itr--;
    }

    return itr;
}

void IniFileManager::deleteSection(const std::wstring& sectionName) {

    //Checks if a section with the provided name exists:
    auto itr = getSection(sectionName);

    if (itr == sections.end())
    {
        std::wostringstream os;
        os << "Could not find any section named " << sectionName << " in your file." << std::endl;
        //std::wcout << os.str();
        this->lastErrorMessage.append(os.str());
    }
    else
    {
        sections.erase(itr);
        copyIsNecessary = true;
    }
}

std::list<Section>::iterator IniFileManager::getSection(const std::wstring& whichSection) {

    auto itr = sections.begin();

    //Looks for a section with the provided name:
    while (itr != sections.end() && itr->name != whichSection)
        itr++;

    return itr;
}

void IniFileManager::addComment(const std::wstring& stringWhichSection, const std::wstring& stringNewComment) {

    std::wstring whichSection = stringWhichSection;
    std::wstring newComment = stringNewComment;
    removeSpaces(whichSection);
    removeSpaces(newComment);
    auto itr = getSection(whichSection);

    if (itr == sections.end())
    {
        std::wostringstream os;
        os << "Could not find any section named " << whichSection << " in your file." << std::endl;
        //std::wcout << os.str();
        this->lastErrorMessage.append(os.str());
        return;
    }

    if (itr->name == whichSection) {

        removeMarginWhites(newComment);

        if (newComment[0] == L';' || newComment[0] == L'#')
            (itr->comments).push_back(newComment);
        else
            (itr->comments).push_back(L';' + newComment);

        copyIsNecessary = true;
        return;
    }
}

void IniFileManager::removeMarginWhites(std::wstring& s) {
//Removes whitespaces at the beginning and at the end of the given string:

    auto strBegin = s.find_first_not_of(L' ');

    if (strBegin == std::string::npos)
        s.erase(std::remove(s.begin(), s.end(), L' '), s.end());
    else {
        auto strEnd = s.find_last_not_of(L' ');
        s = s.substr(strBegin, strEnd - strBegin + 1);
    }
}

void IniFileManager::removeMarginWhites(std::string& s) {
    //Removes whitespaces at the beginning and at the end of the given string:

    if (s.empty())
    {
        return;
    }

    auto strBegin = std::find_if_not(s.cbegin(), s.cend(), [&](char ch) {return isWhiteSpace(ch); });
    auto strEnd = std::find_if_not(s.crbegin(), s.crend(), [&](char ch) {return isWhiteSpace(ch); });

    if (strBegin == s.cend())
    {
        s.clear();
    }
    else {
        //auto strEnd = s.find_last_not_of(' ');
        auto start = std::distance(std::cbegin(s), strBegin);
        auto len = std::distance(strBegin, strEnd.base());
        s = s.substr(start, len);
    }
}

void IniFileManager::clearSection(const std::wstring& stringWhichSection) {

    //Removes all whitespaces from user input:
    std::wstring sectionName = stringWhichSection;
    removeSpaces(sectionName);    
    auto itr = getSection(sectionName);
    if (itr == sections.end())
    {
        std::wostringstream os;
        os << "Could not find any section named " << sectionName << " in your file." << std::endl;
        //std::wcout << os.str();
        this->lastErrorMessage.append(os.str());
        return;
    }
    
    if (itr->name == sectionName) {
        (itr->settings).clear();
        copyIsNecessary = true;
        return;
    }
}

bool IniFileManager::testPath(const std::wstring& filename) {

    //If the user doesn't provide a filename:
    if ((filename.find(L'/') == std::wstring::npos) && (filename.find(L"\\") == std::wstring::npos)) {
        return false;
    }

//Checks if file can be opened:
    std::ifstream infile(filename);
    return infile.good();
}

const std::wstring IniFileManager::readAComment(const std::wstring& stringWhichSection, unsigned int commentIndex) {

    //Removes all whitespaces from user input:
    std::wstring whichSection = stringWhichSection;
    removeSpaces(whichSection);

    auto itr = getSection(whichSection);
    if (itr == sections.end())
    {
        std::wostringstream os;
        os << "Could not find any section named " << whichSection << " in your file." << std::endl;
        //std::wcout << os.str();
        this->lastErrorMessage.append(os.str());
        return L"";
    }

    if (commentIndex >= (itr->comments).size())
    {
        std::wostringstream os;
        os << "Index is not valid." << std::endl;
        if ((itr->comments).size() == 1)
        {
            os << "There is " << itr->comments.size() << " comment in section " << whichSection << "." << std::endl << std::endl;
        }
        else
        {
            os << "There are " << itr->comments.size() << " comment in section " << whichSection << "." << std::endl << std::endl;
        }

        //std::wcout << os.str();
        this->lastErrorMessage.append(os.str());
        return L"";
    }
    else
    {
        return (itr->comments)[commentIndex];
    }
}


void IniFileManager::sortSections()
{
    this->sections.sort(std::less<Section>());
    for (auto it = sections.begin(); it != sections.end(); it++)
    {
        it->settings.sort(std::less<KeyValueAssign>());
    }
}


bool IniFileManager::isWhiteSpace(char ch) const
{
    return std::isspace(ch);
}

bool IniFileManager::isOctalDigit(char ch) const
{
    return (ch >= '0' && ch <= '7');
}

bool IniFileManager::isHexDigit(char ch) const
{
    return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}

bool IniFileManager::isDigit(char ch) const
{
    return ch >= '0' && ch <= '9';
}

bool IniFileManager::isAlpha(char ch) const
{
    return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
}


bool IniFileManager::isSectionNameChar(char ch) const
{
    return isValueChar(ch);
}


bool IniFileManager::isKeyNameChar(char ch) const
{
    return isValueChar(ch) && ch != '=';
}

bool IniFileManager::isValueChar(char ch) const
{
    return !((ch >= 0 && ch <= ' ') || (ch >= 0x7F && ch <= 0x9F) || ch == ',' || ch == '\\' || ch == '\"' || ch == '\'');
}

bool IniFileManager::isAcceptChar(char ch, bool acceptKeyName, bool acceptKeyValue) const
{
    return (acceptKeyName && isKeyNameChar(ch)) || (acceptKeyValue && isValueChar(ch));
}

bool IniFileManager::isQuotedStringEscapeRequired(char ch) const
{
    return ((ch >= 0 && ch <= ' ') || (ch >= 0x7F && ch <= 0x9F) || ch == ',' || ch == '\\' || ch == '\"' || ch == '\'');
}

void IniFileManager::IniToken::Init()
{
    this->TokenType = IniFileManager::IniToken::ATokenType::None;
    this->StringText.clear();
}

void IniFileManager::IniToken::SetError()
{
    this->TokenType = IniFileManager::IniToken::ATokenType::Error;
}

void IniFileManager::IniToken::SetError(const std::string& errorMessage)
{
    this->TokenType = IniFileManager::IniToken::ATokenType::Error;
    this->ErrorText = errorMessage;
}

void IniFileManager::IniToken::SetErrorSymbol()
{
    this->TokenType = IniFileManager::IniToken::ATokenType::Error;
    if (this->StringText.length() == 0)
    {
        this->ErrorText = StringConverter::format_string("Unexpected symbol.");
    }
    else
    {
        SetErrorSymbol(this->ErrorText[0]);
    }
}

void IniFileManager::IniToken::SetErrorSymbol(char ch)
{
    this->TokenType = IniFileManager::IniToken::ATokenType::Error;
    this->ErrorText = StringConverter::format_string("Unexpected symbol %02hhX.", ch);
}

void IniFileManager::IniToken::SetErrorMissingSectionEndChar()
{
    this->TokenType = IniFileManager::IniToken::ATokenType::Error;
    this->ErrorText = "Section header not closed with ] char.";
}

void IniFileManager::IniToken::SetErrorMissingDoubleQuoteChar()
{
    this->TokenType = IniFileManager::IniToken::ATokenType::Error;
    this->ErrorText = "Double quoted string not closed with \" char.";
}

void IniFileManager::IniToken::SetErrorStringTooLong()
{
    this->TokenType = IniFileManager::IniToken::ATokenType::Error;
    this->ErrorText = "String too long.";
}

void IniFileManager::IniToken::SetErrorSyntax()
{
    this->TokenType = IniFileManager::IniToken::ATokenType::Error;
    this->ErrorText = "Syntax error.";
}

void IniFileManager::IniToken::SetErrorFile()
{
    this->TokenType = IniFileManager::IniToken::ATokenType::Error;
    this->ErrorText = "File read error.";
}

void IniFileManager::IniToken::SetEOF()
{
    this->TokenType = IniFileManager::IniToken::ATokenType::EndOfInput;
}

bool IniFileManager::IniToken::IsEOF()
{
    return this->TokenType == IniFileManager::IniToken::ATokenType::EndOfInput;
}

bool IniFileManager::IniToken::IsError()
{
    return this->TokenType == IniFileManager::IniToken::ATokenType::Error;
}

bool IniFileManager::IniToken::IsOK()
{
    return this->TokenType != IniFileManager::IniToken::ATokenType::EndOfInput && this->TokenType != IniFileManager::IniToken::ATokenType::Error;
}
