#include <iostream>
#include <cstring>
#include "FileSystem.h"
#include "LibraryMethods.h"

int FileSystem::fillDirectWithDI(int blockAddress, std::vector<directoryItem> &dirItems)
{
    int itemsPerBlock = this->sb->blockSize / sizeof(directoryItem);

    char diArr[sizeof(directoryItem)];

    int i;
    for (i = 0; i < itemsPerBlock; i++)
    {
        directoryItem di = dirItems.at(0);
        memcpy(diArr, &di, sizeof(directoryItem));
        this->writeToFS(diArr, sizeof(directoryItem), blockAddress + i * sizeof(directoryItem));
        dirItems.erase(dirItems.begin());

        if (dirItems.empty())
        {
            i++;
            break;
        }

    }

    memset(diArr, 0, sizeof(directoryItem));
    for (i; i < itemsPerBlock; i++)
        this->writeToFS(diArr, sizeof(directoryItem), blockAddress + i * sizeof(directoryItem));

    return 0;
}


int FileSystem::defragmentDirects(inode &ind)
{
    int32_t directsBefore[5] = {ind.direct1, ind.direct2, ind.direct3, ind.direct4, ind.direct5};

    std::vector<directoryItem> dirItems;

    if (ind.direct1 != 0)
        for (auto di : this->getAllDirItemsFromDirect(ind.direct1))
            dirItems.emplace_back(di);

    if (ind.direct2 != 0)
        for (auto di : this->getAllDirItemsFromDirect(ind.direct2))
            dirItems.emplace_back(di);

    if (ind.direct3 != 0)
        for (auto di : this->getAllDirItemsFromDirect(ind.direct3))
            dirItems.emplace_back(di);

    if (ind.direct4 != 0)
        for (auto di : this->getAllDirItemsFromDirect(ind.direct4))
            dirItems.emplace_back(di);

    if (ind.direct5 != 0)
        for (auto di : this->getAllDirItemsFromDirect(ind.direct5))
            dirItems.emplace_back(di);

    int32_t directsAfter[5] = {-1, -1, -1, -1, -1};
    directsAfter[0] = ind.direct1;
    this->fillDirectWithDI(ind.direct1, dirItems);

    if (ind.direct2 != 0 && !dirItems.empty())
    {
        this->fillDirectWithDI(ind.direct2, dirItems);
        directsAfter[1] = ind.direct2;
    }
    else if (dirItems.empty())
         directsAfter[1] = 0;

    if (ind.direct3 != 0 && !dirItems.empty())
    {
        directsAfter[2] = ind.direct3;
        this->fillDirectWithDI(ind.direct3, dirItems);
    }
    else if (dirItems.empty())
        directsAfter[2] = 0;

    if (ind.direct4 != 0 && !dirItems.empty())
    {
        directsAfter[3] = ind.direct4;
        this->fillDirectWithDI(ind.direct4, dirItems);
    }
    else if (dirItems.empty())
        directsAfter[3] = 0;

    if (ind.direct5 != 0 && !dirItems.empty())
    {
        directsAfter[4] = ind.direct5;
        this->fillDirectWithDI(ind.direct5, dirItems);
    }
    else if (dirItems.empty())
        directsAfter[4] = 0;

    ind.fileSize = 0;

    for (int i = 0; i < 5; i++)
    {
        if (directsBefore[i] != 0 && directsAfter[i] == 0)
        {
            int blockIndex = (directsBefore[i] - this->sb->blockMapStartAddress) / this->sb->blockSize;
            this->toggleBitInBitmap(blockIndex, this->sb->blockMapStartAddress,this->sb->blockStartAddress - this->sb->blockMapStartAddress);
            char blockArr[this->sb->blockSize];
            memset(blockArr, 0, this->sb->blockSize);
            this->writeToFS(blockArr, this->sb->blockSize, this->sb->blockStartAddress + blockIndex * this->sb->blockSize);
        }

        if (directsBefore[i] != 0 && directsAfter[i] != 0)
            directsAfter[i] = directsBefore[i];

        if (directsAfter[i] != 0)
            ind.fileSize += this->sb->blockSize;
    }

    ind.direct1 = directsAfter[0];
    ind.direct2 = directsAfter[1];
    ind.direct3 = directsAfter[2];
    ind.direct4 = directsAfter[3];
    ind.direct5 = directsAfter[4];

    return 0;
}

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
    int parentBlockIndex = (dirItemAddress - this->sb->blockStartAddress) / this->sb->blockSize;
    int parentBlockAddress = this->sb->blockStartAddress + parentBlockIndex * BLOCK_SIZE;
    memset(diArr, 0, sizeof(directoryItem));
    this->writeToFS(diArr, sizeof(directoryItem), dirItemAddress);

    defragmentDirects(parentInd);     // TODO CHECK IF WORKS
    memcpy(indArr, &parentInd, sizeof(inode));
    this->writeToFS(indArr, sizeof(inode), parentInodeAddress);

    /*std::vector<directoryItem> dirItems = this->getAllDirItemsFromDirect(parentBlockAddress);
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

        this->toggleBitInBitmap(parentBlockIndex, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
        char blockArr[BLOCK_SIZE];
        memset(blockArr, 0, BLOCK_SIZE);
        this->writeToFS(blockArr, BLOCK_SIZE, parentBlockAddress);
        memcpy(indArr, &parentInd, sizeof(inode));
        this->writeToFS(indArr, sizeof(inode), parentInodeAddress);
    }*/

    if (indToRemove.references > 1)
    {
        indToRemove.references--;
        memcpy(indArr, &indToRemove, sizeof(inode));
        this->writeToFS(indArr, sizeof(inode), inodeAddress);
        return 0;
    }

    // Remove the data the inode is pointing to
    char clearBlockArr[this->sb->blockSize];
    memset(clearBlockArr, 0, this->sb->blockSize);
    if (indToRemove.direct1 != 0) {
        this->toggleBitInBitmap((indToRemove.direct1 - this->sb->blockStartAddress) / this->sb->blockSize, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
        this->fillBlock(indToRemove.direct1, clearBlockArr, this->sb->blockSize);
    }
    if (indToRemove.direct2 != 0) {
        this->toggleBitInBitmap((indToRemove.direct2 - this->sb->blockStartAddress) / this->sb->blockSize,
                                this->sb->blockMapStartAddress,
                                this->sb->blockStartAddress - this->sb->blockMapStartAddress);
        this->fillBlock(indToRemove.direct2, clearBlockArr, this->sb->blockSize);
    }
    if (indToRemove.direct3 != 0) {
        this->toggleBitInBitmap((indToRemove.direct3 - this->sb->blockStartAddress) / this->sb->blockSize,
                                this->sb->blockMapStartAddress,
                                this->sb->blockStartAddress - this->sb->blockMapStartAddress);
        this->fillBlock(indToRemove.direct3, clearBlockArr, this->sb->blockSize);
    }
    if (indToRemove.direct4 != 0) {
        this->toggleBitInBitmap((indToRemove.direct4 - this->sb->blockStartAddress) / this->sb->blockSize,
                                this->sb->blockMapStartAddress,
                                this->sb->blockStartAddress - this->sb->blockMapStartAddress);

        this->fillBlock(indToRemove.direct4, clearBlockArr, this->sb->blockSize);
    }
    if (indToRemove.direct5 != 0) {
        this->toggleBitInBitmap((indToRemove.direct5 - this->sb->blockStartAddress) / this->sb->blockSize,
                                this->sb->blockMapStartAddress,
                                this->sb->blockStartAddress - this->sb->blockMapStartAddress);

        this->fillBlock(indToRemove.direct5, clearBlockArr, this->sb->blockSize);
    }

    if (indToRemove.indirect1 != 0)
    {
        char indirect1Block[this->sb->blockSize];
        this->readFromFS(indirect1Block, this->sb->blockSize, indToRemove.indirect1);
        for (int i = 0; i < this->sb->blockSize / sizeof(int32_t); i++)
        {
            int32_t address = {};
            memcpy(&address, indirect1Block + i * sizeof(int32_t), sizeof(int32_t));
            if (address != 0)
            {
                this->toggleBitInBitmap((address - this->sb->blockStartAddress) / this->sb->blockSize, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
                this->fillBlock(address, clearBlockArr, this->sb->blockSize);
            }
        }

        // Clean the indirect1 block and set it as free
        this->fillBlock(indToRemove.indirect1, clearBlockArr, this->sb->blockSize);
        this->toggleBitInBitmap((indToRemove.indirect1 -  this->sb->blockStartAddress) / this->sb->blockSize, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
    }

    if (indToRemove.indirect2 != 0)
    {
        char indirect2Block[this->sb->blockSize];
        this->readFromFS(indirect2Block, this->sb->blockSize, indToRemove.indirect2);

        for (int i = 0; i < this->sb->blockSize / sizeof(int32_t); i++)
        {
            int32_t indirect1address = {};
            memcpy(&indirect1address, indirect2Block + i * sizeof(int32_t), sizeof(int32_t));

            if (indirect1address != 0)
            {
                char indirect1Block[this->sb->blockSize];
                this->readFromFS(indirect1Block, this->sb->blockSize, indirect1address);
                for (int j = 0; j < this->sb->blockSize / sizeof(int32_t); j++)
                {
                    int32_t directAddress = {};
                    memcpy(&directAddress, indirect1Block + j * sizeof(int32_t), sizeof(int32_t));
                    if (directAddress != 0)
                    {
                        this->toggleBitInBitmap((directAddress - this->sb->blockStartAddress) / this->sb->blockSize, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
                        this->fillBlock(directAddress, clearBlockArr, this->sb->blockSize);
                    }
                }

                this->fillBlock(indirect1address, clearBlockArr, this->sb->blockSize);
                this->toggleBitInBitmap((indirect1address -  this->sb->blockStartAddress) / this->sb->blockSize, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
            }
        }

        this->fillBlock(indToRemove.indirect2, clearBlockArr, this->sb->blockSize);
        this->toggleBitInBitmap((indToRemove.indirect2 -  this->sb->blockStartAddress) / this->sb->blockSize, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
    }

    this->toggleBitInBitmap(indToRemove.nodeid - 1, this->sb->inodeMapStartAddress, this->sb->inodeStartAddress - this->sb->inodeMapStartAddress);
    memset(indArr, 0, sizeof(inode));
    this->writeToFS(indArr, sizeof(inode), inodeAddress);

    return 0;
}



























