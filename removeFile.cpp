#include <iostream>
#include <cstring>
#include "FileSystem.h"
#include "LibraryMethods.h"

// Removes a file, given a path
// 1    = NOT A FILE
// 2    = PATH NOT FOUND
// 3    = FILE NOT FOUND
// 0    = OK
int FileSystem::removeFile(std::string path)
{
    if (path.at(0) == '/' && path.size() == 1)
    {
        std::cout << "NOT A FILE" << std::endl;
        return 1;
    }

    std::vector<std::string> splitPath = LibraryMethods::split(path, '/');
    std::string nameToRemove = splitPath.at(splitPath.size() - 1);
    path = path.substr(0, path.size() - nameToRemove.size());

    int parentInodeAddress = this->getInodeAddressForPath(path);
    if (parentInodeAddress == -1)
    {
        std::cout << "PATH NOT FOUND" << std::endl;
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
        return 3;
    }
    directoryItem dirItemToRemove = {};
    char diArr[sizeof(directoryItem)];
    this->readFromFS(diArr, sizeof(directoryItem), dirItemAddress);
    memcpy(&dirItemToRemove, diArr, sizeof(directoryItem));

    int inodeAddress = dirItemToRemove.inode;
    inode indToRemove = {};
    this->readFromFS(indArr, sizeof(inode), inodeAddress);
    memcpy(&indToRemove, indArr, sizeof(inode));

    if (indToRemove.isDirectory)
    {
        std::cout << "NOT A FILE" << std::endl;
        return 1;
    }

    // Remove directory item from parent inode
    int parentBlockIndex = (dirItemAddress - this->sb->blockStartAddress) / BLOCK_SIZE;
    int parentBlockAddress = this->sb->blockStartAddress + parentBlockIndex * BLOCK_SIZE;
    memset(diArr, 0, sizeof(directoryItem));
    this->writeToFS(diArr, sizeof(directoryItem), dirItemAddress);

    // If we've emptied a direct reference in parent inode, we find out which one it was, and set the block as free
    std::vector<directoryItem> dirItems = this->getAllDirItemsFromDirect(parentBlockAddress);
    if (dirItems.empty())
    {
        if (parentInd.direct2 == parentBlockAddress)
            parentInd.direct2 = 0;
        else if (parentInd.direct3 == parentBlockAddress)
            parentInd.direct3 = 0;
        else if (parentInd.direct4 == parentBlockAddress)
            parentInd.direct4 = 0;
        else if (parentInd.direct5 == parentBlockAddress)
            parentInd.direct5 = 0;

        parentInd.fileSize -= BLOCK_SIZE;
        // decrement the references to parent
        parentInd.references--;

        this->toggleBitInBitmap(parentBlockIndex, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
        char blockArr[BLOCK_SIZE];
        memset(blockArr, 0, BLOCK_SIZE);
        this->writeToFS(blockArr, BLOCK_SIZE, parentBlockAddress);
        memcpy(indArr, &parentInd, sizeof(inode));
        this->writeToFS(indArr, sizeof(inode), parentInodeAddress);
    }

    // If another DI is pointing to this inode, decrement the number of references and leave - we've already removed the diirectory item
    if (indToRemove.references > 1)
    {
        indToRemove.references--;
        memcpy(indArr, &indToRemove, sizeof(inode));
        this->writeToFS(indArr, sizeof(inode), inodeAddress);
        return 0;
    }

    // Remove the data the inode is pointing to
    char clearBlockArr[BLOCK_SIZE];
    memset(clearBlockArr, 0, BLOCK_SIZE);
    if (indToRemove.direct1 != 0) {
        this->toggleBitInBitmap((indToRemove.direct1 - this->sb->blockStartAddress) / BLOCK_SIZE, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
        this->fillBlock(indToRemove.direct1, clearBlockArr, BLOCK_SIZE);
    }
    if (indToRemove.direct2 != 0) {
        this->toggleBitInBitmap((indToRemove.direct2 - this->sb->blockStartAddress) / BLOCK_SIZE,
                                this->sb->blockMapStartAddress,
                                this->sb->blockStartAddress - this->sb->blockMapStartAddress);
        this->fillBlock(indToRemove.direct2, clearBlockArr, BLOCK_SIZE);
    }
    if (indToRemove.direct3 != 0) {
        this->toggleBitInBitmap((indToRemove.direct3 - this->sb->blockStartAddress) / BLOCK_SIZE,
                                this->sb->blockMapStartAddress,
                                this->sb->blockStartAddress - this->sb->blockMapStartAddress);
        this->fillBlock(indToRemove.direct3, clearBlockArr, BLOCK_SIZE);
    }
    if (indToRemove.direct4 != 0) {
        this->toggleBitInBitmap((indToRemove.direct4 - this->sb->blockStartAddress) / BLOCK_SIZE,
                                this->sb->blockMapStartAddress,
                                this->sb->blockStartAddress - this->sb->blockMapStartAddress);

        this->fillBlock(indToRemove.direct4, clearBlockArr, BLOCK_SIZE);
    }
    if (indToRemove.direct5 != 0) {
        this->toggleBitInBitmap((indToRemove.direct5 - this->sb->blockStartAddress) / BLOCK_SIZE,
                                this->sb->blockMapStartAddress,
                                this->sb->blockStartAddress - this->sb->blockMapStartAddress);

        this->fillBlock(indToRemove.direct5, clearBlockArr, BLOCK_SIZE);
    }

    if (indToRemove.indirect1 != 0)
    {
        char indirect1Block[BLOCK_SIZE];
        this->readFromFS(indirect1Block, BLOCK_SIZE, indToRemove.indirect1);
        for (int i = 0; i < BLOCK_SIZE / sizeof(int32_t); i++)
        {
            int32_t address = {};
            memcpy(&address, indirect1Block + i * sizeof(int32_t), sizeof(int32_t));
            if (address != 0)
            {
                this->toggleBitInBitmap((address - this->sb->blockStartAddress) / BLOCK_SIZE, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
                this->fillBlock(address, clearBlockArr, BLOCK_SIZE);
            }
        }

        // Clean the indirect1 block and set it as free
        this->fillBlock(indToRemove.indirect1, clearBlockArr, BLOCK_SIZE);
        this->toggleBitInBitmap((indToRemove.indirect1 -  this->sb->blockStartAddress) / BLOCK_SIZE, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
    }

    if (indToRemove.indirect2 != 0)
    {
        char indirect2Block[BLOCK_SIZE];
        this->readFromFS(indirect2Block, BLOCK_SIZE, indToRemove.indirect2);

        for (int i = 0; i < BLOCK_SIZE / sizeof(int32_t); i++)
        {
            int32_t indirect1address = {};
            memcpy(&indirect1address, indirect2Block + i * sizeof(int32_t), sizeof(int32_t));

            if (indirect1address != 0)
            {
                char indirect1Block[BLOCK_SIZE];
                this->readFromFS(indirect1Block, BLOCK_SIZE, indirect1address);
                for (int j = 0; j < BLOCK_SIZE / sizeof(int32_t); j++)
                {
                    int32_t directAddress = {};
                    memcpy(&directAddress, indirect1Block + j * sizeof(int32_t), sizeof(int32_t));
                    if (directAddress != 0)
                    {
                        this->toggleBitInBitmap((directAddress - this->sb->blockStartAddress) / BLOCK_SIZE, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
                        this->fillBlock(directAddress, clearBlockArr, BLOCK_SIZE);
                    }
                }

                this->fillBlock(indirect1address, clearBlockArr, BLOCK_SIZE);
                this->toggleBitInBitmap((indirect1address -  this->sb->blockStartAddress) / BLOCK_SIZE, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
            }
        }

        this->fillBlock(indToRemove.indirect2, clearBlockArr, BLOCK_SIZE);
        this->toggleBitInBitmap((indToRemove.indirect2 -  this->sb->blockStartAddress) / BLOCK_SIZE, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
    }

    this->toggleBitInBitmap(indToRemove.nodeid - 1, this->sb->inodeMapStartAddress, this->sb->inodeStartAddress - this->sb->inodeMapStartAddress);
    memset(indArr, 0, sizeof(inode));
    this->writeToFS(indArr, sizeof(inode), inodeAddress);

    return 0;
}