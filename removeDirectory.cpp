#include <iostream>
#include <cstring>
#include "FileSystem.h"
#include "LibraryMethods.h"


int FileSystem::removeDirectory(std::string path)
{
    if (path.at(0) == '/' && path.size() == 1)
    {
        std::cout << "CANNOT REMOVE ROOT" << std::endl;
        return 1;
    }
    std::vector<std::string> splitPath = LibraryMethods::split(path, '/');
    std::string nameToRemove = splitPath.at(splitPath.size() - 1);
    path = path.substr(0, path.size() - nameToRemove.size());

    int parentInodeAddress = this->getInodeAddressForPath(path);
    if (parentInodeAddress == -1)
    {
        std::cout << "FILE NOT FOUND" << std::endl;
        return 2;
    }

    inode parentInd = {};
    char indArr[sizeof(inode)];
    this->readFromFS(indArr, sizeof(inode), parentInodeAddress);
    memcpy(&parentInd, indArr, sizeof(inode));

    int dirItemAddress = this->findDirItemInInode(nameToRemove, parentInd);
    if (dirItemAddress == -1)
    {
        std::cout << "FILE NOT FOUND" << std::endl;
        return 2;
    }

    if (nameToRemove == ".")
    {
        // TODO now it behaves like ubuntu terminal, maybe make dirItem to be the parent reference to this directory and inode the parent inode. Right now, the parent inode is the one in which the . is.
        std::cout << "INVALID ARGUMENT" << std::endl;
        return 3;
    }
    else if (nameToRemove == "..")
    {
        std::cout << "NOT EMPTY" << std::endl;
        return 2;
    }

    directoryItem diToRemove = {};
    char diArr[sizeof(directoryItem)];
    this->readFromFS(diArr, sizeof(directoryItem), dirItemAddress);
    memcpy(&diToRemove, diArr, sizeof(directoryItem));

    inode indToRemove = {};
    this ->readFromFS(indArr, sizeof(inode), diToRemove.inode);
    memcpy(&indToRemove, indArr, sizeof(inode));

    if (!indToRemove.isDirectory)
    {
        std::cout << "NOT A DIRECTORY" << std::endl;
        return 1;
    }

    std::vector<directoryItem> dirItems;
    if (indToRemove.direct1 != 0)
    {
        dirItems = this->getAllDirItemsFromDirect(indToRemove.direct1);
        if (dirItems.size() > 2)    // items . and .. will be there and cannot be removed
        {
            std::cout << "NOT EMPTY" << std::endl;
            return 3;
        }
    }

    if (indToRemove.direct2 != 0)
    {
        dirItems = this->getAllDirItemsFromDirect(indToRemove.direct2);
        if (!dirItems.empty())
        {
            std::cout << "NOT EMPTY" << std::endl;
            return 3;
        }
    }

    if (indToRemove.direct3 != 0)
    {
        dirItems = this->getAllDirItemsFromDirect(indToRemove.direct3);
        if (!dirItems.empty())
        {
            std::cout << "NOT EMPTY" << std::endl;
            return 3;
        }
    }

    if (indToRemove.direct4 != 0)
    {
        dirItems = this->getAllDirItemsFromDirect(indToRemove.direct4);
        if (!dirItems.empty())
        {
            std::cout << "NOT EMPTY" << std::endl;
            return 3;
        }
    }

    if (indToRemove.direct5 != 0)
    {
        dirItems = this->getAllDirItemsFromDirect(indToRemove.direct5);
        if (!dirItems.empty())
        {
            std::cout << "NOT EMPTY" << std::endl;
            return 3;
        }
    }

    // Erase the . and .. from block
    char blockArr[BLOCK_SIZE];
    memset(blockArr, 0, BLOCK_SIZE);
    this->writeToFS(blockArr, BLOCK_SIZE, indToRemove.direct1);
    // Set the block as available
    this->toggleBitInBitmap((indToRemove.direct1 - this->sb->blockStartAddress) / BLOCK_SIZE, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);

    // Erase the inode
    memset(indArr, 0, sizeof(inode));
    this->writeToFS(indArr, sizeof(inode), diToRemove.inode);
    // Set the inode as available
    this->toggleBitInBitmap(indToRemove.nodeid - 1, this->sb->inodeMapStartAddress, this->sb->inodeStartAddress - this->sb->inodeMapStartAddress);

    // Remove the DI from parent inode
    memset(diArr, 0, sizeof(directoryItem));
    this->writeToFS(diArr, sizeof(directoryItem), dirItemAddress);

    // Check if the direct block in parent contains any other diritems, if not, set it as available
    // TODO there could be error is this logic, check for bugs
    int blockIndex = (dirItemAddress - this->sb->blockStartAddress) / BLOCK_SIZE;
    int blockAddress = this->sb->blockStartAddress + blockIndex * BLOCK_SIZE;
    dirItems = getAllDirItemsFromDirect(blockAddress);
    if (dirItems.empty())
    {
        // We dont need to check direct1, as we will set it as available only when the directory gets deleted.
        if (parentInd.direct2 == blockAddress)
            parentInd.direct2 = 0;
        else if (parentInd.direct3 == blockAddress)
            parentInd.direct3 = 0;
        else if (parentInd.direct4 == blockAddress)
            parentInd.direct4 = 0;
        else
            parentInd.direct5 = 0;

        this->toggleBitInBitmap(blockIndex, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
        memset(blockArr, 0, BLOCK_SIZE);
        this->writeToFS(blockArr, BLOCK_SIZE, blockAddress);
        memcpy(indArr, &parentInd, sizeof(inode));
        this->writeToFS(indArr, sizeof(inode), parentInodeAddress);
    }

    std::cout << "OK" << std::endl;
    return 0;
}

