#include <cstring>
#include "FileSystem.h"
#include "LibraryMethods.h"

int FileSystem::removeDirItemFromInode(std::string name, int inodeAddress)
{
    inode ind = {};
    char inodeArr[sizeof(char)];
    this->readFromFS(inodeArr, sizeof(inode), inodeAddress);
    memcpy(&ind, inodeArr, sizeof(inode));
    int dirItemAddress = this->findDirItemInInode(name, ind);

    if (dirItemAddress != -1)
    {
        char diArr[sizeof(directoryItem)];
        memset(diArr, 0, sizeof(directoryItem));
        this->writeToFS(diArr, sizeof(directoryItem), dirItemAddress);
    }

    return 0;
}

