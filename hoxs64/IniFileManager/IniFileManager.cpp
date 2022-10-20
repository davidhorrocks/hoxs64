//
// Created by Chiara on 17/08/2016.
//

#include <iostream>
#include <algorithm>
#include <fstream>

#include "IniFileManager.h"

bool IniFileManager::copyIsNecessary = false;

IniFileManager::IniFileManager(const std::string p, Mode m) : path(p), mode(m) {

    if (mode == open && testPath())
        parse();
    else if (mode == open && !testPath()) {
        std::cout << "Error: file at given path can't be opened." << std::endl;
        std::cout << "Try again, making sure your file exists, "
                "has the right extension and its path is written correctly." << std::endl << std::endl;
    }

}

void IniFileManager::parse() {

    std::ifstream file;

    file.open(path);

    //Variable handling one section at a time:
    Section currentSection;

    //Parsing: file is read line by line:
    std::string line;

    while (std::getline(file, line)) {

        //As it doesn't know what the line is, it initially only removes whitespaces at the margins of the line:
        removeMarginWhites(line);

        //If it finds the beginning of a new section:
        if (line[0] == '[') {

            //Removes excess whitespaces from the name of the section:
            line.erase(std::remove(line.begin(), line.end(), ' '), line.end());

            unsigned long int end = line.find_first_of(']');

            if (end != std::string::npos) {

                //Checks if currentSection is already handling a section:
                if (!currentSection.name.empty()) {

                    //If so, the previous one is over, and it can store it in the list:
                    sections.push_back(currentSection);

                    //Clears data for a reuse of the variable:
                    currentSection.name.clear();
                    currentSection.settings.clear();
                    currentSection.comments.clear();
                }
                //Gets the name of the section, removing '[' and ']':
                currentSection.name = line.substr(1, end - 1);

            } else {

                //No ']' found:
                std::cout << "Error: No ']' found in section title." << std::endl;
                std::cout << "Make sure your "
                        "file is written according to the ini standard." << std::endl << std::endl;

                //Parsing is aborted to prevent wrong memorizations:
                sections.clear();
                return;
            }

        } else if (!line.empty() && (line[0] == ';' || line[0] == '#')) {

            //If it finds a comment, whitespaces are not removed and it stores it in the vector:
            currentSection.comments.push_back(line);

        } else if (line.empty()) {

            //If the line is empty, nothing is memorized;

        } else {

            //Last case possible is that the line is of the kind "key=value": all whitespaces are deleted
            line.erase(std::remove(line.begin(), line.end(), ' '), line.end());

            unsigned long int delim;
            delim = line.find_first_of('=');

            if (delim != std::string::npos) {
                std::string key = line.substr(0, delim);
                std::string value = line.substr(delim + 1);
                //Inserimento:
                currentSection.settings.insert(std::make_pair(key, value));

            } else {

                //If it couldn't find a '=' in the line...
                std::cout << "Error: detected"
                        " line in your file which is not in the form key = value. " << std::endl;
                std::cout << "Make sure your "
                        "file is written according to the ini standard." << std::endl << std::endl;

                //... parsing is aborted:
                sections.clear();
                return;
            }
        }
    }

    //When no more lines are left, it stores the last section being handled in the list:
    if (!currentSection.name.empty()) {

        sections.push_back(currentSection);
    }

    file.close();
}

const std::string IniFileManager::read(std::string whichSection, std::string whichKey) {

    //Removes all whitespaces from user input:
    whichSection.erase(std::remove(whichSection.begin(), whichSection.end(), ' '), whichSection.end());
    whichKey.erase(std::remove(whichKey.begin(), whichKey.end(), ' '), whichKey.end());

    auto itr = getSection(whichSection);

    if (itr == sections.end()) {
        std::cout << "Error: could not find any section"
                " named " << whichSection << " in your file." << std::endl << std::endl;
        return "";
    } else if ((*itr).name == whichSection) {

        //If the section is found, it looks for the key in the corresponding map:
        auto search = (*itr).settings.find(whichKey);

        //If no key is found:
        if (search == (*itr).settings.end()) {
            std::cout << "Error: could not find any key named " << whichKey << " in section "
                    "" << (*itr).name << " of your file." << std::endl << std::endl;
            return "";
        } else {
            return search->second;
        }
    }
}

void IniFileManager::editValue(std::string whichSection, std::string whichKey, std::string userValue) {

    //Removes all whitespaces from user input:
    whichSection.erase(std::remove(whichSection.begin(), whichSection.end(), ' '), whichSection.end());
    whichKey.erase(std::remove(whichKey.begin(), whichKey.end(), ' '), whichKey.end());
    whichKey.erase(std::remove(userValue.begin(), userValue.end(), ' '), userValue.end());

    auto itr = getSection(whichSection);

    if (itr == sections.end()) {
        std::cout << "Error: could not find any section"
                " named " << whichSection << " in your file ." << std::endl << std::endl;
        return;
    }

    if ((*itr).name == whichSection) {

        //If the section is found, it looks for the key in the corresponding map:
        auto search = (*itr).settings.find(whichKey);

        //If no key is found:
        if (search == (*itr).settings.end()) {
            std::cout << "Error: could not find any key named " << whichKey << " in section "
                    "" << (*itr).name << " of your file." << std::endl << std::endl;
            return;
        } else {

            //If the key is found, the value is modified:
            search->second = userValue;

            copyIsNecessary = true;
        }
    }
}

IniFileManager::~IniFileManager() {

    if (mode == create) {
        generateCopy(path);
        if (!testPath())
            std::cout << "Your path was not valid.";
    } else {
        if (copyIsNecessary) {
            //It deletes the old file and creates an updated one with the same name:
            std::remove(path.c_str());
            generateCopy(path);
        }
    }
}

void IniFileManager::generateCopy(std::string path) {
    //Se si è modificato un file esistente, ne crea uno aggiornato nella medesima directory:
    std::ofstream newFile(path);

    auto itr = sections.begin();

    //Ricopiatura:
    while (itr != sections.end()) {
        //All'inizio si copia il nome della sezione:
        newFile << "[" + itr->name + "]" << std::endl;

        //Poi i commenti:
        for (auto comItr = (itr->comments).begin(); comItr != (itr->comments).end(); comItr++)
            newFile << (*comItr) << std::endl;

        //Poi tutti i campi:
        auto mapitr = (itr->settings).begin();
        while (mapitr != (itr->settings).end()) {
            newFile << mapitr->first + "=" + mapitr->second << std::endl;
            mapitr++;
        }
        newFile << std::endl;
        itr++;
    }

    newFile.close();
}

void IniFileManager::addLine(std::string whichSection, std::string newKey, std::string newValue) {

    //Removes all whitespaces from user input:
    whichSection.erase(std::remove(whichSection.begin(), whichSection.end(), ' '), whichSection.end());
    newKey.erase(std::remove(newKey.begin(), newKey.end(), ' '), newKey.end());
    newValue.erase(std::remove(newValue.begin(), newValue.end(), ' '), newValue.end());

    auto itr = getSection(whichSection);

    if (itr == sections.end()) {
        std::cout << "Error: could not find any section named " << whichSection << " in your file."<< std::endl;
    } else if (itr->name == whichSection) {

        //If the section is found, it checks if a key with the same name already exists:
        auto search = (*itr).settings.find(newKey);

        //If not, adds the pair to the corresponding map:
        if (search == (*itr).settings.end()) {
            (itr->settings).insert(std::make_pair(newKey, newValue));
            copyIsNecessary = true;
        } else {
            std::cout << "Error: cannot add line because provided key already exists." << std::endl;
        }
    }
}

void IniFileManager::deleteLine(std::string whichSection, std::string whichKey) {

    //Removes all whitespaces from user input:
    whichSection.erase(std::remove(whichSection.begin(), whichSection.end(), ' '), whichSection.end());
    whichKey.erase(std::remove(whichKey.begin(), whichKey.end(), ' '), whichKey.end());

    auto itr = getSection(whichSection);

    if (itr == sections.end()) {
        std::cout << "Error: could not find any section named " << whichSection << " in your file."<< std::endl;
    } else if (itr->name == whichSection) {

        //If the section is found, it looks for the key in the map:
        auto search = (*itr).settings.find(whichKey);

        //If no key is found:
        if (search == (*itr).settings.end()) {
            std::cout << "Error: could not find any setting named " << whichKey << " in section "
                    "" << (*itr).name << " of your file."<< std::endl;
        } else {
            (itr->settings).erase(search);
            copyIsNecessary = true;
        }
    }
}

void IniFileManager::addSection(std::string sectionName) {

    //Checks if a section with the same name already exists:
    auto itr = getSection(sectionName);

    if (itr == sections.end()) {
        Section newSection;
        newSection.name = sectionName;
        sections.push_back(newSection);
        copyIsNecessary = true;
    } else
        std::cout << "Error: a section named " << sectionName << " already exists." << std::endl;

}

void IniFileManager::deleteSection(std::string sectionName) {

    //Checks if a section with the provided name exists:
    auto itr = getSection(sectionName);

    if (itr == sections.end()) {
        std::cout << "Error: could not find any section named " << sectionName << " in your file."<< std::endl;
    } else {
        sections.erase(itr);
        copyIsNecessary = true;
    }

}

std::list<Section>::iterator IniFileManager::getSection(const std::string whichSection) {

    auto itr = sections.begin();

    //Looks for a section with the provided name:
    while (itr != sections.end() && itr->name != whichSection)
        itr++;

    return itr;
}

void IniFileManager::addComment(std::string whichSection, std::string newComment) {

    auto itr = getSection(whichSection);

    if (itr == sections.end()) {
        std::cout << "Error: could not find any section named " << whichSection << " in your file."<< std::endl;
        return;
    }
    if (itr->name == whichSection) {

        removeMarginWhites(newComment);

        if (newComment[0] == ';' || newComment[0] == '#')
            (itr->comments).push_back(newComment);
        else
            (itr->comments).push_back(';' + newComment);

        copyIsNecessary = true;
        return;
    }
}

void IniFileManager::removeMarginWhites(std::string & string) {
//Removes whitespaces at the beginning and at the end of the given string:

    auto strBegin = string.find_first_not_of(' ');

    if (strBegin == std::string::npos)
        string.erase(std::remove(string.begin(), string.end(), ' '), string.end());
    else {
        auto strEnd = string.find_last_not_of(' ');
        string = string.substr(strBegin, strEnd - strBegin + 1);
    }
}

void IniFileManager::clearSection(std::string sectionName) {

    //Removes all whitespaces from user input:
    sectionName.erase(std::remove(sectionName.begin(), sectionName.end(), ' '), sectionName.end());

    auto itr = getSection(sectionName);

    if (itr == sections.end()) {
        std::cout << "Error: could not find any section named " << sectionName << " in your file."<< std::endl;
        return;
    }
    if (itr->name == sectionName) {
        (itr->settings).clear();
        copyIsNecessary = true;
        return;
    }
}

bool IniFileManager::testPath() {

    //If the user doesn't provide a path:
    if ((path.find('/') == std::string::npos) && (path.find("\\") == std::string::npos)) {
        return false;
    }

//Checks if file at given path can be opened:
    std::ifstream infile(path);
    return infile.good();
}

const std::string IniFileManager::readAComment(std::string whichSection, unsigned int commentIndex) {

    //Removes all whitespaces from user input:
    whichSection.erase(std::remove(whichSection.begin(), whichSection.end(), ' '), whichSection.end());

    auto itr = getSection(whichSection);

    if (itr == sections.end()) {
        std::cout << "Error: could not find any section named " << whichSection << " in your file."<< std::endl;
        return "";
    }
    if (itr->name == whichSection) {
        if (commentIndex >= (itr->comments).size()) {
            std::cout << "Error: index is not valid." << std::endl;
            if ((itr->comments).size() == 1)
                std::cout << "There is " << itr->comments.size() << " comment"
                        " in section " << whichSection << "." << std::endl << std::endl;
            else
                std::cout << "There are " << itr->comments.size() << " comment"
                        " in section " << whichSection << "."<< std::endl << std::endl;
            return "";
        } else
            return (itr->comments)[commentIndex];
    }
}







































