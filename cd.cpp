#include <iostream>
#include <cstring>
#include "FileSystem.h"

// Changes the working directory
// Expects a relative or absolute path to a directory
// 1   = PATH NOT FOUND
// 2   = NOT A DIRECTORY
// 0   = OK
int FileSystem::cd(std::string path)
{
    // Perform checks
    int targetInodeAddress = this->getInodeAddressForPath(path);
    if (targetInodeAddress == -1)
    {
        std::cout << "PATH NOT FOUND" << std::endl;
        return 1;
    }

    inode ind = {};
    char indArr[sizeof(inode)];
    this->readFromFS(indArr, sizeof(inode), targetInodeAddress);
    memcpy(&ind, indArr, sizeof(inode));
    if (!ind.isDirectory)
    {
        std::cout << "NOT A DIRECTORY" << std::endl;
        return 2;
    }

    // Switch the working directory
    this->currentInodeAddress = targetInodeAddress;

    // Update the current path
    this->currentPath = getAbsolutePathToInode(targetInodeAddress);

    return 0;
}

