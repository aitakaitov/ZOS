#include <string>
#include <vector>
#include <sstream>
#include <sys/stat.h>
#include <cstring>
#include "LibraryMethods.h"
#include "FileSystem.h"

// Parses a size
// Size format is (N)*[B|KB|MB|GB]
// returns the size in bytes
// or -1, if the format is invalid
int LibraryMethods::parseFSSize(std::string fsSize)
{
    if (!isdigit(fsSize[0]))
    {
        return -1;
    }

    int fsFizeBytes;
    int lastDigitPos = -1;
    for (int i = 0; i < fsSize.length(); i++)
    {
        if (!isdigit(fsSize[i]))
        {
            lastDigitPos = i - 1;
            break;
        }
    }

    if (lastDigitPos == -1 || lastDigitPos == fsSize.length() - 1)
    {
        return -1;
    }

    fsFizeBytes = stoi(fsSize.substr(0, lastDigitPos + 1));
    std::string units = fsSize.substr(lastDigitPos + 1, fsSize.length() - 1);

    if (units == "B")
    {
        fsFizeBytes *= 1;
    }
    else if (units == "KB")
    {
        fsFizeBytes *= 1024;
    }
    else if (units == "MB")
    {
        fsFizeBytes *= 1048576;
    }
    else if (units == "GB")
    {
        fsFizeBytes *= 1073741824;
    }
    else
    {
        return -1;
    }

    return fsFizeBytes;
}

// Toggles a bit in a byte (0 -> 1 or 1 -> 0)
char LibraryMethods::toggleBit(char byte, int pos) {
    byte ^= 1 << pos;
    return byte;
}

// returns the value of a bit in a byte
int LibraryMethods::checkBit(char byte, int pos) {
    return (byte >> pos) & 1U;
}

// splits a string given a delimiter and returns it
std::vector<std::string> LibraryMethods::split(const std::string& str, char delim)
{
    std::istringstream ss(str);
    std::string token;
    std::vector<std::string> tokens;

    while(std::getline(ss, token, delim))
    {
        tokens.push_back(token);
    }

    return tokens;
}

// Checks if a file exists
bool LibraryMethods::fileExists(const std::string& path)
{
    struct stat buffer{};
    return (stat (path.c_str(), &buffer) == 0);
}

void LibraryMethods::parseName(const std::string& name, char *cName, char *cExtension, bool isDirectory)
{
    memset(cName, 0, 8);
    memset(cExtension, 0, 3);

    if (name.length() == 0)
        return;

    if (name == "..")
    {
        cName[0] = '.';
        cName[1] = '.';
        return;
    }

    if (name == ".")
    {
        cName[0] = '.';
        return;
    }


    if (isDirectory)
    {
        if (name.size() > 8)
            memcpy(cName, name.c_str(), 8);
        else
            memcpy(cName, name.c_str(), name.size());
    }
    else
        {
            std::vector<std::string> splitName = LibraryMethods::split(name, '.');
            if (splitName.at(0).size() > 8)
                memcpy(cName, splitName.at(0).c_str(), 8);
            else
                memcpy(cName, splitName.at(0).c_str(), splitName.at(0).size());

            if (splitName.size() == 2)
            {
                if (splitName.at(1).size() > 3)
                    memcpy(cExtension, splitName.at(1).c_str(), 3);
                else
                    memcpy(cExtension, splitName.at(1).c_str(), splitName.at(1).size());
            }
        }
}