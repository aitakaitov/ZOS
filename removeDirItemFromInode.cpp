#include <cstring>
#include "FileSystem.h"

// Removes a dirItem from inode
void FileSystem::removeDirItemFromInode(const std::string& name, int inodeAddress)
{
    inode ind = {};
    char inodeArr[sizeof(inode)];
    this->readFromFS(inodeArr, sizeof(inode), inodeAddress);
    memcpy(&ind, inodeArr, sizeof(inode));
    int dirItemAddress = this->findDirItemInInode(name, ind);

    if (dirItemAddress != -1)
    {
        char diArr[sizeof(directoryItem)];
        memset(diArr, 0, sizeof(directoryItem));
        this->writeToFS(diArr, sizeof(directoryItem), dirItemAddress);
    }
}

