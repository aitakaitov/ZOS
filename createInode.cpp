#include <cstring>
#include <iostream>
#include "FileSystem.h"

// Given a file size, calculates the number of blocks necessary to store the data
// -2   = FILE TOO LARGE FOR FS
// >0   = BLOCKS NECESSARY
int FileSystem::getBlocksNecessary(int sizeBytes)
{
    int blocksNecessary = 0;
    int referencesPerBlock = this->sb->blockSize / sizeof(int32_t);

    int temp = sizeBytes / this->sb->blockSize;
    if (sizeBytes % this->sb->blockSize != 0)
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
                return -2;
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

    return blocksNecessary;
}

// Fills block with bytes, given a block address (in fs)
// expects the bytes not NULL
// expects the length <= BLOCK_SIZE
// 0    = OK
int FileSystem::fillBlock(int blockAddress, char *bytes, int length)
{
    char blockArr[this->sb->blockSize];
    memset(blockArr, 0, this->sb->blockSize);
    memcpy(blockArr, bytes, length);
    this->writeToFS(blockArr, length, blockAddress);

    return 0;
}

// Fills an indirect1 block address with bytes from a FILE*. Expects all args not NULL.
// Intended to be used by createInode().
// 0    = OK
int fillIndirect1(int indirectBlockAddress, FILE *file, int bytesToGo, FileSystem *fs)
{
    // Loop trough all the direct addresses in the indirect1 block.
    for (int i = 0; i < fs->sb->blockSize / sizeof(int32_t); i++)
    {
        // Allocate new block
        int blockIndex = fs->getFreeBlock();
        fs->toggleBitInBitmap(blockIndex, fs->sb->blockMapStartAddress, fs->sb->blockStartAddress - fs->sb->blockMapStartAddress);
        char intArr[sizeof(int32_t)];
        int32_t blockAddress = fs->sb->blockStartAddress + fs->sb->blockSize * blockIndex;
        memcpy(intArr, &blockAddress, sizeof(int32_t));
        fs->writeToFS(intArr, sizeof(int32_t), indirectBlockAddress + i * sizeof(int32_t));
        char blockArr[fs->sb->blockSize];

        // If this block is not the last one
        if (bytesToGo > fs->sb->blockSize)
        {
            fread(blockArr, 1, fs->sb->blockSize, file);
            fs->fillBlock(blockAddress, blockArr, fs->sb->blockSize);
            bytesToGo -= fs->sb->blockSize;
        }
        else
        {
            // Otherwise don't copy a whole block
            fread(blockArr, 1, bytesToGo, file);
            fs->fillBlock(blockAddress, blockArr, bytesToGo);
            return 0;
        }
    }

    return 0;
}

// Fills an indirect2 block with bytes from a FILE*. Expects all args not NULL.
// Intended to be used by createInode().
// 0    = OK
int fillIndirect2(int indirect2BlockAddress, FILE *file, int bytesToGo, FileSystem *fs)
{
    // Loop trough all the addresses
    for (int i = 0; i < fs->sb->blockSize / sizeof(int32_t); i++)
    {
        // Allocate an indirect1 block
        int blockIndex = fs->getFreeBlock();
        fs->toggleBitInBitmap(blockIndex, fs->sb->blockMapStartAddress, fs->sb->blockStartAddress - fs->sb->blockMapStartAddress);
        char intArr[sizeof(int32_t)];
        int32_t indirectBlockAddress = fs->sb->blockStartAddress + blockIndex * fs->sb->blockSize;
        memcpy(intArr, &indirectBlockAddress, sizeof(int32_t));
        fs->writeToFS(intArr, sizeof(int32_t), indirect2BlockAddress + i * sizeof(int32_t));

        // Fill the indirect1 with data
        if (bytesToGo > (fs->sb->blockSize / sizeof(int32_t)) * fs->sb->blockSize)
        {
            fillIndirect1(indirectBlockAddress, file, bytesToGo, fs);
            bytesToGo -= (fs->sb->blockSize / sizeof(int32_t)) * fs->sb->blockSize;
        }
        else
            {
                fillIndirect1(indirectBlockAddress, file, bytesToGo, fs);
                return 0;
            }
    }

    return 0;
}

// Creates and inode, either directory or a file.
// Expects the calling method to allocate the inode in the bitmap and un-allocate it in case of failure.
// Expects the calling method to allocate a block in the bitmap. The un-allocation in case of failure is handled by this method.
// parentInodeAddress is a FS address of the inode, in which the directoryItem, pointing to the new inode, has been placed
// If the FILE* file is NULL, method will create a directory. Otherwise method will create a file wth data from the FILE*.
// The FILE* is expected to be opened if not NULL. Method will not close it in case of failure.
// 0    = OK
// 1    = NO FREE BLOCKS
// 2    = FILE TOO LARGE
// 3    = NOT ENOUGH FREE BLOCKS
int FileSystem::createInode(int inodeIndex, int blockIndex, int parentInodeAddress, FILE *file)
{
    inode ind = {};
    memset(&ind, 0, sizeof(inode));
    int inodeAddress = this->sb->inodeStartAddress + inodeIndex * sizeof(inode);
    if (file == NULL)
    {
        ind.isDirectory = true;
        ind.fileSize = this->sb->blockSize;
        ind.direct1 = this->sb->blockStartAddress + blockIndex * this->sb->blockSize;
    }
    else
        {
            ind.isDirectory = false;
            fseek(file, 0L, SEEK_END);
            ind.fileSize = ftell(file);
            rewind(file);
        }

    if (ind.isDirectory)
        ind.references = 0;
    else
        ind.references = 1;

    ind.nodeid = inodeIndex + 1;

    char inodeArr[sizeof(inode)];
    memcpy(inodeArr, &ind, sizeof(inode));
    this->writeToFS(inodeArr, sizeof(inode), inodeAddress);

    // We add two DIs, one pointing to the same inode and one pointing to the parent inode
    if (ind.isDirectory)
    {
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
        /*inode indParent = {};
        char indParentArr[sizeof(inode)];
        this->readFromFS(indParentArr, sizeof(inode), parentInodeAddress);
        memcpy(&indParent, indParentArr, sizeof(inode));
        indParent.references++;
        memcpy(indParentArr, &indParent, sizeof(inode));
        this->writeToFS(indParentArr, sizeof(inode), parentInodeAddress);*/
    }
    else
        {
            // One block is reserved already, but thats not convenient for us
            this->toggleBitInBitmap(blockIndex, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
            int freeBlocks = this->getFreeBlocksNum();
            int blocksNecessary = this->getBlocksNecessary(ind.fileSize);

            if (blocksNecessary == -2)
            {
                std::cout << "FILE TOO LARGE FOR FS" << std::endl;
            }

            if (freeBlocks < blocksNecessary)
            {
                std::cout << "NOT ENOUGH FREE BLOCKS" << std::endl;
                return 3;
            }

            // Fill the directs with data
            int i = 0;
            int bytesToGo = ind.fileSize;
            int directs[5] = {ind.direct1, ind.direct2, ind.direct3, ind.direct4, ind.direct5};
            memset(directs, 0, 5 * sizeof(int32_t));
            for (; i < 5; i++)
            {
                if (directs[i] == 0)
                {
                    int index = this->getFreeBlock();
                    directs[i] = this->sb->blockStartAddress + this->sb->blockSize * index;
                    this->toggleBitInBitmap(index, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
                    char bytes[this->sb->blockSize];
                    memset(bytes, 0, this->sb->blockSize);
                    if (bytesToGo > this->sb->blockSize) {
                        fread(bytes, 1, this->sb->blockSize, file);
                        this->fillBlock(directs[i], bytes, this->sb->blockSize);
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
                    bytesToGo -= this->sb->blockSize;
                }
            }

            ind.direct1 = directs[0];
            ind.direct2 = directs[1];
            ind.direct3 = directs[2];
            ind.direct4 = directs[3];
            ind.direct5 = directs[4];

            // Fill indirect1 with data
            blockIndex = this->getFreeBlock();
            this->toggleBitInBitmap(blockIndex, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
            ind.indirect1 = this->sb->blockStartAddress + blockIndex * this->sb->blockSize;
            fillIndirect1(ind.indirect1, file, bytesToGo, this);
            if (bytesToGo <= (this->sb->blockSize / sizeof(int32_t)) * this->sb->blockSize) {
                char indArr[sizeof(inode)];
                memcpy(indArr, &ind, sizeof(inode));
                this->writeToFS(indArr, sizeof(inode), inodeAddress);
                return 0;
            }
            else
                bytesToGo -= (this->sb->blockSize / sizeof(int32_t)) * this->sb->blockSize;

            // Fill indirect2 with data
            blockIndex = this->getFreeBlock();
            this->toggleBitInBitmap(blockIndex, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
            ind.indirect2 = this->sb->blockStartAddress + blockIndex * this->sb->blockSize;
            fillIndirect2(ind.indirect2, file, bytesToGo, this);

            // Write the inode to FS
            char indArr[sizeof(inode)];
            memcpy(indArr, &ind, sizeof(inode));
            this->writeToFS(indArr, sizeof(inode), inodeAddress);
        }

    return 0;
}

