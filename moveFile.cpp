#include <iostream>
#include "FileSystem.h"

// Moves a file from filePath to newPath
// Essentially creates a hardlink and removes the old file
// 0    = OK
// 1    = ERR
int FileSystem::moveFile(std::string filePath, std::string newPath)
{
    if (this->ln(filePath, newPath) == 0)
        if (this->removeFile(filePath) == 0)
            return 0;

    return 1;
}

