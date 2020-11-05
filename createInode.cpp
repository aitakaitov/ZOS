#include <cstring>
#include <iostream>
#include "FileSystem.h"

int FileSystem::fillBlock(int blockAddress, char *bytes, int length)
{
    char blockArr[BLOCK_SIZE];
    memset(blockArr, 0, BLOCK_SIZE);
    memcpy(blockArr, bytes, length);
    this->writeToFS(blockArr, length, blockAddress);

    return 0;
}

int FileSystem::fillIndirect1(int indirectBlockAddress, FILE *file, int bytesToGo)
{
    for (int i = 0; i < BLOCK_SIZE / sizeof(int32_t); i++)
    {
        int blockIndex = this->getFreeBlock();
        this->toggleBitInBitmap(blockIndex, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
        char intArr[sizeof(int32_t)];
        int32_t blockAddress = this->sb->blockStartAddress + BLOCK_SIZE * blockIndex;
        memcpy(intArr, &blockAddress, sizeof(int32_t));
        this->writeToFS(intArr, sizeof(int32_t), indirectBlockAddress + i * sizeof(int32_t));
        char blockArr[BLOCK_SIZE];
        if (bytesToGo > BLOCK_SIZE)
        {
            fread(blockArr, 1, BLOCK_SIZE, file);
            this->fillBlock(blockAddress, blockArr, BLOCK_SIZE);
            bytesToGo -= BLOCK_SIZE;
        }
        else
        {
            fread(blockArr, 1, bytesToGo, file);
            this->fillBlock(blockAddress, blockArr, bytesToGo);
            return 0;
        }
    }

    return 1;
}

int FileSystem::fillIndirect2(int indirect2BlockAddress, FILE *file, int bytesToGo)
{
    for (int i = 0; i < BLOCK_SIZE / sizeof(int32_t); i++)
    {
        int blockIndex = this->getFreeBlock();
        this->toggleBitInBitmap(blockIndex, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
        char intArr[sizeof(int32_t)];
        int32_t indirectBlockAddress = this->sb->blockStartAddress + blockIndex * BLOCK_SIZE;
        memcpy(intArr, &indirectBlockAddress, sizeof(int32_t));
        this->writeToFS(intArr, sizeof(int32_t), indirect2BlockAddress + i * sizeof(int32_t));
        if (bytesToGo > (BLOCK_SIZE / sizeof(int32_t)) * BLOCK_SIZE)
        {
            this->fillIndirect1(indirectBlockAddress, file, bytesToGo);
            bytesToGo -= (BLOCK_SIZE / sizeof(int32_t)) * BLOCK_SIZE;
        }
        else
            {
                this->fillIndirect1(indirectBlockAddress, file, bytesToGo);
                return 0;
            }
    }

    return 0;
}

int FileSystem::createInode(int inodeIndex, int blockIndex, int parentInodeAddress, FILE *file)
{
    inode ind = {};
    memset(&ind, 0, sizeof(inode));
    int inodeAddress = this->sb->inodeStartAddress + inodeIndex * sizeof(inode);
    if (file == NULL)
    {
        ind.isDirectory = true;
        ind.fileSize = BLOCK_SIZE;
        ind.direct1 = this->sb->blockStartAddress + blockIndex * BLOCK_SIZE;
    }
    else
        {
            ind.isDirectory = false;
            fseek(file, 0L, SEEK_END);
            ind.fileSize = ftell(file);
            rewind(file);
        }

    if (ind.isDirectory)
        ind.references = 2;
    else
        ind.references = 1;

    ind.nodeid = inodeIndex + 1;

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
            this->toggleBitInBitmap(blockIndex, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
            return 1;
        }
        // add self DI to inode
        name[1] = '\0';
        res = this->addDirItemToInode(name, extension, inodeAddress, inodeAddress);
        if (res != 0)
        {
            this->removeDirItemFromInode(name, inodeAddress);
            this->toggleBitInBitmap(blockIndex, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
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
            // One block is reserved already, but thats not convenient for us
            this->toggleBitInBitmap(blockIndex, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
            int freeBlocks = this->getFreeBlocksNum();
            int referencesPerBlock = BLOCK_SIZE / sizeof(int32_t);
            int blocksNecessary;

            int temp = ind.fileSize / BLOCK_SIZE;
            if (ind.fileSize % BLOCK_SIZE != 0)
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
            int bytesToGo = ind.fileSize;
            int directs[5] = {ind.direct1, ind.direct2, ind.direct3, ind.direct4, ind.direct5};
            memset(directs, 0, 5 * sizeof(int32_t));
            for (; i < 5; i++)
            {
                if (directs[i] == 0)
                {
                    int index = this->getFreeBlock();
                    directs[i] = this->sb->blockStartAddress + BLOCK_SIZE * index;
                    this->toggleBitInBitmap(index, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
                    char bytes[BLOCK_SIZE];
                    memset(bytes, 0, BLOCK_SIZE);
                    if (bytesToGo > BLOCK_SIZE) {
                        fread(bytes, 1, BLOCK_SIZE, file);
                        this->fillBlock(directs[i], bytes, BLOCK_SIZE);
                    }
                    else {
                        fread(bytes, 1, bytesToGo, file);
                        this->fillBlock(directs[i], bytes, bytesToGo);
                        ind.direct1 = directs[0];
                        ind.direct2 = directs[1];
                        ind.direct3 = directs[2];
                        ind.direct4 = directs[3];
                        ind.direct5 = directs[4];
                        char indArr[sizeof(inode)];
                        memcpy(indArr, &ind, sizeof(inode));
                        this->writeToFS(indArr, sizeof(inode), inodeAddress);
                        return 0;
                    }
                    bytesToGo -= BLOCK_SIZE;
                }
            }

            ind.direct1 = directs[0];
            ind.direct2 = directs[1];
            ind.direct3 = directs[2];
            ind.direct4 = directs[3];
            ind.direct5 = directs[4];

            blockIndex = this->getFreeBlock();
            this->toggleBitInBitmap(blockIndex, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
            ind.indirect1 = this->sb->blockStartAddress + blockIndex * BLOCK_SIZE;
            this->fillIndirect1(ind.indirect1, file, bytesToGo);
            if (bytesToGo <= (BLOCK_SIZE / sizeof(int32_t)) * BLOCK_SIZE) {
                char indArr[sizeof(inode)];
                memcpy(indArr, &ind, sizeof(inode));
                this->writeToFS(indArr, sizeof(inode), inodeAddress);
                return 0;
            }
            else
                bytesToGo -= (BLOCK_SIZE / sizeof(int32_t)) * BLOCK_SIZE;

            blockIndex = this->getFreeBlock();
            this->toggleBitInBitmap(blockIndex, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
            ind.indirect2 = this->sb->blockStartAddress + blockIndex * BLOCK_SIZE;
            this->fillIndirect2(ind.indirect2, file, bytesToGo);

            char indArr[sizeof(inode)];
            memcpy(indArr, &ind, sizeof(inode));
            this->writeToFS(indArr, sizeof(inode), inodeAddress);
        }

    return 0;
}

