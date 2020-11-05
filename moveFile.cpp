#include <iostream>
#include <cstring>
#include "FileSystem.h"
#include "LibraryMethods.h"

int FileSystem::moveFile(std::string filePath, std::string newPath)
{
    if (this->ln(filePath, newPath) == 0)
        if (this->removeFile(filePath) == 0)
            return 0;

    return 1;
}

