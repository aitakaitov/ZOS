#include <cstring>
#include <iostream>
#include "FileSystem.h"
#include "LibraryMethods.h"

int FileSystem::fillBlock(int blockAddress, char *bytes, int length)
{
    char blockArr[BLOCK_SIZE];
    memset(blockArr, 0, BLOCK_SIZE);
    memcpy(blockArr, bytes, length);
    this->writeToFS(blockArr, length, blockAddress);
}

int FileSystem::createInode(int inodeIndex, int blockIndex, int parentInodeAddress, const char *bytes, int length)
{
    inode ind = {};
    memset(&ind, 0, sizeof(inode));
    int inodeAddress = this->sb->inodeStartAddress + inodeIndex * sizeof(inode);
    if (bytes == NULL)
    {
        ind.isDirectory = true;
    }
    else
        {
            ind.isDirectory = false;
            ind.fileSize = length;
        }

    ind.direct1 = this->sb->blockStartAddress + blockIndex * BLOCK_SIZE;

    if (ind.isDirectory)
        ind.references = 2;
    else
        ind.references = 1;

    ind.nodeid = inodeIndex + 1;
    ind.fileSize = BLOCK_SIZE;

    char inodeArr[sizeof(inode)];
    memcpy(inodeArr, &ind, sizeof(inode));
    this->writeToFS(inodeArr, sizeof(inode), inodeAddress);

    // We add two DIs, one pointing to the same inode and one pointing to the parent inode
    if (ind.isDirectory)
    {
        /*directoryItem diParent = {};
        directoryItem diSelf = {};
        memset(&diParent, 0, sizeof(directoryItem));
        memset(&diSelf, 0, sizeof(directoryItem));
        memcpy(diParent.itemName, "..", 2);
        memcpy(diSelf.itemName, ".", 1);
        diParent.inode = parentInodeAddress;
        diSelf.inode = inodeAddress;*/

        // add parent DI to inode
        char name[8] = {'.', '.', '\0', '\0', '\0', '\0', '\0', '\0'};
        char extension[3] = {'\0', '\0', '\0'};
        int res = this->addDirItemToInode(name, extension, inodeAddress, parentInodeAddress);
        if (res != 0)
        {
            std::cout << "NO FREE BLOCKS" << std::endl;
            return 1;
        }
        // add self DI to inode
        name[1] = '\0';
        res = this->addDirItemToInode(name, extension, inodeAddress, inodeAddress);
        if (res != 0)
        {
            this->removeDirItemFromInode(name, inodeAddress);
            name[1] = '.';
            std::cout << "NO FREE BLOCKS" << std::endl;
            return 1;
        }

        // Increment the number of references in parent
        inode indParent = {};
        char indParentArr[sizeof(inode)];
        this->readFromFS(indParentArr, sizeof(inode), parentInodeAddress);
        memcpy(&indParent, indParentArr, sizeof(inode));
        indParent.references++;
        memcpy(indParentArr, &indParent, sizeof(inode));
        this->writeToFS(indParentArr, sizeof(inode), parentInodeAddress);
    }
    else
        {
            // We have one already reserved
            int freeBlocks = this->getFreeBlocksNum() + 1;
            int referencesPerBlock = BLOCK_SIZE / sizeof(int32_t);
            int blocksNecessary = 0;

            int temp = length / BLOCK_SIZE;
            if (length % BLOCK_SIZE != 0)
                temp++;

            if (temp > 5)
            {
                blocksNecessary = 5;
                temp = temp - 5;
                if (temp > referencesPerBlock)
                {
                    blocksNecessary += referencesPerBlock + 1;
                    temp = temp - referencesPerBlock - 1;

                    if (temp > referencesPerBlock * referencesPerBlock)
                    {
                        std::cout << "FILE TOO LARGE" << std::endl;
                        return 1;
                    }
                    else
                        {
                            blocksNecessary += 1;
                            if (temp / referencesPerBlock == 0)
                                blocksNecessary += 1 + temp;
                            else
                                {
                                    blocksNecessary += (temp / referencesPerBlock) * (referencesPerBlock + 1);
                                    if (temp % referencesPerBlock != 0)
                                        blocksNecessary += 1 + (temp % referencesPerBlock);
                                }
                        }
                }
                else
                    blocksNecessary += temp + 1;
            }
            else
                blocksNecessary = temp;

            if (freeBlocks < blocksNecessary)
            {
                std::cout << "NOT ENOUGH FREE BLOCKS" << std::endl;
                return 1;
            }

            int i = 0;
            int bytesToGo = length;
            int directs[5] = {ind.direct1, ind.direct2, ind.direct3, ind.direct4, ind.direct5};
            memset(directs, 0, 5);
            for (; i < 5 | i < blocksNecessary; i++)
            {
                if (directs[i] == 0)
                {
                    int index = this->getFreeBlock();
                    directs[i] = this->sb->blockStartAddress + BLOCK_SIZE * index;
                    this->toggleBitInBitmap(index, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
                    bytesToGo -= BLOCK_SIZE;
                    if (bytesToGo > BLOCK_SIZE)
                        this->fillBlock(directs[i], (char*)(bytes + i * BLOCK_SIZE), BLOCK_SIZE);
                    else
                        this->fillBlock(directs[i], (char*)(bytes + i * BLOCK_SIZE), bytesToGo);
                }
            }
            ind.direct1 = directs[0];
            ind.direct2 = directs[1];
            ind.direct3 = directs[2];
            ind.direct4 = directs[3];
            ind.direct5 = directs[4];

        }
}

