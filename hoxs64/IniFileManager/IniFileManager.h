//
// Created by Chiara on 17/08/2016.
//

#ifndef INIFILEMANAGER_INIFILEMANAGER_H
#define INIFILEMANAGER_INIFILEMANAGER_H


#include <string>
#include <unordered_map>
#include <list>
#include <vector>

enum Mode {open, create};

class Section {
public:
    std::string name;
    std::vector<std::string> comments;
    std::unordered_map<std::string, std::string> settings;
};


class IniFileManager {
public:
    IniFileManager(const std::string path, Mode m);

    const std::string read(std::string whichSection, std::string whichKey);

    void editValue(std::string whichSection, std::string whichKey, std::string userValue);

    void addLine(std::string whichSection, std::string whichKey, std::string userValue);
    void deleteLine(std::string whichSection, std::string whichKey);

    void addSection(std::string sectionName);
    void deleteSection(std::string sectionName);
    void clearSection(std::string sectionName);

    void addComment(std::string whichSection, std::string newComment);
    const std::string readAComment(std::string whichSection, unsigned int commentIndex);

    ~IniFileManager();

private:
    void parse();
    void generateCopy(std::string path);
    bool testPath();
    void removeMarginWhites(std::string & string);
    std::list<Section>::iterator getSection (const std::string whichSection);

    std::list<Section> sections;
    std::string path;
    Mode mode;
    static bool copyIsNecessary;
};


#endif //INIFILEMANAGER_INIFILEMANAGER_H
