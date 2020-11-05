#include <iostream>
#include <cstring>
#include "FileSystem.h"
#include "LibraryMethods.h"

int FileSystem::ln(std::string pathToFile, std::string pathToLink)
{
    int fileInodeAddress = this->getInodeAddressForPath(pathToFile);
    if (fileInodeAddress == -1)
    {
        std::cout << "FILE NOT FOUND" << std::endl;
        return 1;
    }

    std::vector<std::string> splitPath = LibraryMethods::split(pathToLink, '/');
    std::string linkName = splitPath.at(splitPath.size() - 1);
    pathToLink = pathToLink.substr(0, pathToLink.size() - linkName.size());

    int linkParentInodeAddress = this->getInodeAddressForPath(pathToLink);
    if (linkParentInodeAddress == -1)
    {
        std::cout << "PATH NOT FOUND" << std::endl;
        return 2;
    }

    inode linkParentInode = {};
    char indArr[sizeof(inode)];
    this->readFromFS(indArr, sizeof(inode), linkParentInodeAddress);
    memcpy(&linkParentInode, indArr, sizeof(inode));

    std::vector<std::string> splitName = LibraryMethods::split(linkName, '.');
    char cName[8];
    memset(cName, 0, 8);
    memcpy(cName, splitName.at(0).c_str(), splitName.at(0).size());

    char extension[3];
    memset(extension, 0, 3);
    if (splitName.size() == 2)
        memcpy(extension, splitName.at(1).c_str(), splitName.at(1).size());

    if (findDirItemInInode(linkName, linkParentInode) != -1)
    {
        std::cout << "EXIST" << std::endl;
        return 3;
    }

    int res = this->addDirItemToInode(cName, extension, linkParentInodeAddress, fileInodeAddress);
    if (res != 0)
    {
        std::cout << "MAX ITEMS IN DIRECTORY" << std::endl;
        return 4;
    }

    this->readFromFS(indArr, sizeof(inode), fileInodeAddress);
    inode ind = {};
    memcpy(&ind, indArr, sizeof(inode));
    ind.references++;

    memcpy(indArr, &ind, sizeof(inode));
    this->writeToFS(indArr, sizeof(inode), fileInodeAddress);

    std::cout << "OK" << std::endl;
    return 0;
}

