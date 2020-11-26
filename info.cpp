#include <iostream>
#include <cstring>
#include "FileSystem.h"
#include "LibraryMethods.h"

// Writes info about an inode (file/dir) given a path
// 0    = OK
// 1    = FILE NOT FOUND
int FileSystem::info(std::string path)
{
    // Special cases
    if (path == "/")
        path += ".";

    if (path == "")
        path += ".";

    std::string name;
    std::vector<std::string> splitPath = LibraryMethods::split(path, '/');
    name = splitPath.at(splitPath.size() - 1);
    path = path.substr(0, path.size() - name.size());

    int parentInodeAddress = this->getInodeAddressForPath(path);
    if (parentInodeAddress == -1)
    {
        std::cout << "FILE NOT FOUND" << std::endl;
        return 1;
    }

    inode parentInd = {};
    char indArr[sizeof(inode)];
    this->readFromFS(indArr, sizeof(inode), parentInodeAddress);
    memcpy(&parentInd, indArr, sizeof(inode));

    int dirItemAddress = this->findDirItemInInode(name, parentInd);
    if (dirItemAddress == -1)
    {
        std::cout << "FILE NOT FOUND" << std::endl;
        return 1;
    }

    directoryItem di = {};
    char diArr[sizeof(directoryItem)];
    this->readFromFS(diArr, sizeof(directoryItem), dirItemAddress);
    memcpy(&di, diArr, sizeof(directoryItem));

    int inodeAddress = di.inode;

    inode targetInd = {};
    this->readFromFS(indArr, sizeof(inode), inodeAddress);
    memcpy(&targetInd, indArr, sizeof(inode));

    char cName[FILENAME_MAX_SIZE + 1];
    char cExtension[EXTENSION_MAX_SIZE + 1];
    memset(cName, 0, FILENAME_MAX_SIZE + 1);
    memset(cExtension, 0, EXTENSION_MAX_SIZE + 1);
    memcpy(cName, di.itemName, FILENAME_MAX_SIZE);
    memcpy(cExtension, di.itemName + FILENAME_MAX_SIZE, EXTENSION_MAX_SIZE);

    std::cout << cName;
    if (strcmp(cExtension, "") != 0)
        std::cout << "." << cExtension;

    std::cout << " - " << targetInd.fileSize << " - " << targetInd.nodeid << " - " << this->getBlocksNecessary(targetInd.fileSize) << std::endl;

    return 0;
}
