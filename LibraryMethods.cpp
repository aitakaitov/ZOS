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

// Parses a name based on it being a directory or not
// Expects cName and cExtension to be allocated to FILENAME_MAX_SIZE and EXTENSION_MAX_SIZE, they don't need to be zeroed
void LibraryMethods::parseName(const std::string& name, char *cName, char *cExtension, bool isDirectory)
{
    memset(cName, 0, FILENAME_MAX_SIZE);
    memset(cExtension, 0, EXTENSION_MAX_SIZE);

    if (name.length() == 0)
        return;

    // special cases
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

    // if it's a directory, copy into cName and shorten if necessary
    if (isDirectory)
    {
        if (name.size() > FILENAME_MAX_SIZE)
            memcpy(cName, name.c_str(), FILENAME_MAX_SIZE);
        else
            memcpy(cName, name.c_str(), name.size());
    }
    else        // IF it's a file, we need to split it by .
        {
            std::vector<std::string> splitName = LibraryMethods::split(name, '.');
            if (splitName.at(0).size() > FILENAME_MAX_SIZE)
                memcpy(cName, splitName.at(0).c_str(), FILENAME_MAX_SIZE);
            else
                memcpy(cName, splitName.at(0).c_str(), splitName.at(0).size());

            if (splitName.size() == 2)
            {
                if (splitName.at(1).size() > EXTENSION_MAX_SIZE)
                    memcpy(cExtension, splitName.at(1).c_str(), EXTENSION_MAX_SIZE);
                else
                    memcpy(cExtension, splitName.at(1).c_str(), splitName.at(1).size());
            }
        }
}