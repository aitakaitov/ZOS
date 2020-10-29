#include <cstring>
#include <iostream>
#include <vector>
#include "FileSystem.h"
#include "LibraryMethods.h"

int FileSystem::getInodeAddressForPath(std::string path)
{
    const char fslash = '/';
    inode currentInode = {};
    char inodeArr[sizeof(inode)];
    int inodeAddress;

    // Test if the path is absolute - remove the / and make our starting point root inode
    if (path.at(0) == fslash)
    {
        path = path.substr(1);
        this->readFromFS(inodeArr, sizeof(inode), this->sb->inodeStartAddress);
        memcpy(&currentInode, inodeArr, sizeof(inode));
        inodeAddress = this->sb->inodeStartAddress;
    }
    else    // Otherwise, make our starting point current inode
    {
        this->readFromFS(inodeArr, sizeof(inode), this->currentInodeAddress);
        memcpy(&currentInode, inodeArr, sizeof(inode));
        inodeAddress = this->currentInodeAddress;
    }

    if (path.size() == 0)
        return inodeAddress;

    std::vector<std::string> splitPath = LibraryMethods::split(path, '/');
    for (int i = 0; i < splitPath.size(); i++)
    {
        std::string name = splitPath.at(i);
        int found = this->findDirItemInInode(name, currentInode);
        if (found == -1)
            return -1;
        else
            {
                // Get the inode reference from DI
                inodeAddress = 0;
                this->readFromFS(reinterpret_cast<char *>(&inodeAddress), sizeof(int32_t), found);
                // Read the inode
                this->readFromFS(inodeArr, sizeof(inode), inodeAddress);
                memcpy(&currentInode, inodeArr, sizeof(inode));
            }
    }

    return inodeAddress;
}

