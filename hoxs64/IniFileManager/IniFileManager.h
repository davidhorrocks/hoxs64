#pragma once
//
// Created by Chiara on 17/08/2016.
// Modified by David Horrocks on 18/11/2022
//
#include <string>
#include <map>
#include <list>
#include <vector>

class KeyValueAssign {
public:
    std::wstring name;
    std::vector<std::wstring> values;
    std::wstring comment;
    bool operator<(const KeyValueAssign& other) const;
    void clear();
};

class Section {
public:
    std::wstring name;
    std::vector<std::wstring> comments;
    std::list<KeyValueAssign> settings;
    bool operator<(const Section& other) const;
};

class IniFileManager 
{
public:
    bool read(const std::wstring& whichSection, const std::wstring& whichKey, KeyValueAssign **pkeyValue);
    std::list<KeyValueAssign>::iterator ensureKey(const std::wstring& whichSection, const std::wstring& whichKey);
    void ensureKeyValue(const std::wstring& whichSection, const std::wstring& whichKey, const std::wstring& userValue);
    void ensureKeyValues(const std::wstring& whichSection, const std::wstring& whichKey, const std::vector<std::wstring>& userValue);

    void editKeyValue(const std::wstring& whichSection, const std::wstring& whichKey, const std::wstring& userValue);
    void editKeyValues(const std::wstring& whichSection, const std::wstring& whichKey, const std::vector<std::wstring>& userValues);

    void addKeyValue(const std::wstring& whichSection, const std::wstring& newKey, const std::wstring& newValue);
    void addKeyValues(const std::wstring& whichSection, const std::wstring& newKey, const std::vector<std::wstring>& newValues);
    void deleteKey(const std::wstring& whichSection, const std::wstring& whichKey);

    void addSection(const std::wstring& sectionName);
    void deleteSection(const std::wstring& sectionName);
    void clearSection(const std::wstring& sectionName);

    void addComment(const std::wstring& whichSection, const std::wstring& newComment);
    const std::wstring readAComment(const std::wstring& whichSection, unsigned int commentIndex);

    bool parse(const std::wstring& filename);
    void saveFile(const std::wstring& filename);
    void sortSections();
    void clearLastError();

    std::list<Section>::iterator ensureSection(const std::wstring& sectionName);
    std::list<Section>::iterator getSection(const std::wstring& whichSection);
    std::list<Section> sections;
    std::wstring lastErrorMessage;
private:
    void generateCopy(const std::wstring& filename);
    bool testPath(const std::wstring& filename);
    void removeMarginWhites(std::wstring& s);
    void removeMarginWhites(std::string& s);
    void removeSpaces(std::wstring& s);

    //Mode mode;
    bool copyIsNecessary = false;    
    static constexpr int BOM_COUNT_UTF8 = 3;
    static const char bomUtf8[3];
    const char whitespacelist[4] = { ' ', '\t', '\r', '\n' };
    bool isWhiteSpace(char ch) const;
    bool isOctalDigit(char ch) const;
    bool isHexDigit(char ch) const;
    bool isDigit(char ch) const;
    bool isAlpha(char ch) const;
    bool isSectionNameChar(char ch) const;
    bool isKeyNameChar(char ch) const;
    bool isValueChar(char ch) const;
    bool isAcceptChar(char ch, bool acceptKeyName, bool acceptKeyValue) const;
    bool isQuotedStringEscapeRequired(char ch) const;
    unsigned long long lineNumber = 0;

    struct IniToken
    {
        static constexpr std::size_t MaxStringLength = 1024 * 1024;
        struct ATokenType
        {
            enum tagTType
            {
                None,
                EndOfInput,
                StringValue,
                Symbol,
                Comment,
                SectionName,
                NewLine,
                Error
            };
        };

        ATokenType::tagTType TokenType = ATokenType::None;
        std::string StringText;
        std::string ErrorText;
        void Init();
        void SetError();
        void SetError(const std::string& errorMessage);
        void SetErrorSymbol();
        void SetErrorSymbol(char ch);
        void SetErrorMissingSectionEndChar();
        void SetErrorMissingDoubleQuoteChar();
        void SetErrorStringTooLong();
        void SetErrorSyntax();
        void SetErrorFile();
        void SetEOF();
        bool IsEOF();
        bool IsError();
        bool IsOK();
    };

    bool readNextToken(std::basic_istream<char>& strm, bool acceptKeyName, bool acceptKeyValue, IniToken& token);
    bool getStringValue(std::basic_istream<char>& strm, bool acceptKeyName, bool acceptKeyValue, IniToken& token);
};

