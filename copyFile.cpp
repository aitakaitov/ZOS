#include <iostream>
#include <cstring>
#include "FileSystem.h"
#include "LibraryMethods.h"

// Copies contents of indirect1 to another inode's indirect1. Expects the two files we are copying have the same size,
// no checks are performed for 0-only addresses, it relies solely on file size to know, when to stop.
// Intended for use with copyFile
// 0    = OK
int fillIndirect1(int indirectBlockAddress, int indirectSourceBlockAddress, int bytesToGo, FileSystem *fs)
{
    char blockArr[fs->sb->blockSize];
    char indirectSrcBlockArr[fs->sb->blockSize];
    fs->readFromFS(indirectSrcBlockArr, fs->sb->blockSize, indirectSourceBlockAddress);

    for (int i = 0; i < fs->sb->blockSize / sizeof(int32_t); i++)
    {
        int blockIndex = fs->getFreeBlock();
        fs->toggleBitInBitmap(blockIndex, fs->sb->blockMapStartAddress, fs->sb->blockStartAddress - fs->sb->blockMapStartAddress);
        char intArr[sizeof(int32_t)];
        int32_t blockAddress = fs->sb->blockStartAddress + fs->sb->blockSize * blockIndex;
        memcpy(intArr, &blockAddress, sizeof(int32_t));
        fs->writeToFS(intArr, sizeof(int32_t), indirectBlockAddress + i * sizeof(int32_t));

        if (bytesToGo > fs->sb->blockSize)
        {
            int32_t srcAddr = {};
            memcpy(&srcAddr, indirectSrcBlockArr + i * sizeof(int32_t), sizeof(int32_t));
            fs->readFromFS(blockArr, fs->sb->blockSize, srcAddr);
            fs->fillBlock(blockAddress, blockArr, fs->sb->blockSize);
            bytesToGo -= fs->sb->blockSize;
        }
        else
        {
            int32_t srcAddr = {};
            memcpy(&srcAddr, indirectSrcBlockArr + i * sizeof(int32_t), sizeof(int32_t));
            fs->readFromFS(blockArr, bytesToGo, srcAddr);
            fs->fillBlock(blockAddress, blockArr, bytesToGo);
            return 0;
        }
    }

    return 0;
}

// Copies contents of indirect2 to another inode's indirect2. Expects the two files we are copying have the same size,
// no checks are performed for 0-only addresses, it relies solely on file size to know, when to stop.
// Intended for use with copyFile
// 0    = OK
int fillIndirect2(int indirect2BlockAddress, int indirect2SourceBlockAddress, int bytesToGo, FileSystem *fs)
{
    // Loop trough all the addresses
    for (int i = 0; i < fs->sb->blockSize / sizeof(int32_t); i++)
    {
        int blockIndex = fs->getFreeBlock();
        fs->toggleBitInBitmap(blockIndex, fs->sb->blockMapStartAddress, fs->sb->blockStartAddress - fs->sb->blockMapStartAddress);
        char intArr[sizeof(int32_t)];
        int32_t indirectBlockAddress = fs->sb->blockStartAddress + blockIndex * fs->sb->blockSize;
        memcpy(intArr, &indirectBlockAddress, sizeof(int32_t));
        fs->writeToFS(intArr, sizeof(int32_t), indirect2BlockAddress + i * sizeof(int32_t));
        if (bytesToGo > (fs->sb->blockSize / sizeof(int32_t)) * fs->sb->blockSize)
        {
            int32_t srcAddr = {};
            fs->readFromFS(intArr, sizeof(int32_t), indirect2SourceBlockAddress + i * sizeof(int32_t));
            memcpy(&srcAddr, intArr, sizeof(int32_t));
            fillIndirect1(indirectBlockAddress, srcAddr, bytesToGo, fs);
            bytesToGo -= (fs->sb->blockSize / sizeof(int32_t)) * fs->sb->blockSize;
        }
        else
        {
            int32_t srcAddr = {};
            fs->readFromFS(intArr, sizeof(int32_t), indirect2SourceBlockAddress + i * sizeof(int32_t));
            memcpy(&srcAddr, intArr, sizeof(int32_t));
            fillIndirect1(indirectBlockAddress, srcAddr, bytesToGo, fs);
            return 0;
        }
    }

    return 0;
}

// Copies a file (only) from sourcePath to destinationPath
// sourcePath must exist
// destinationPath must exist up to the new file name, in the parent folder a file with the same name must not exist
// method checks for free blocks and free inodes
// 0    = OK
// 1    = FILE NOT FOUND (source) / NOT A FILE (source)
// 2    = PATH NOT FOUND (dest)
// 3    = EXIST (in the destination directory a file with the same name exists)
// 4    = NO FREE INODES
// 5    = MAX ITEMS IN DIRECTORY (or NO FREE BLOCKS)
// 6    = NOT ENOUGH FREE BLOCKS
// 7    = FILE TOO LARGE FOR THE FS
int FileSystem::copyFile(std::string sourcePath, std::string destinationPath)
{
    // Check if the source exists
    int sourceInodeAddress = this->getInodeAddressForPath(sourcePath);
    if (sourceInodeAddress == -1)
    {
        std::cout << "FILE NOT FOUND" << std::endl;
        return 1;
    }

    inode sourceInd = {};
    char indArr[sizeof(inode)];
    this->readFromFS(indArr, sizeof(inode), sourceInodeAddress);
    memcpy(&sourceInd, indArr, sizeof(inode));

    if (sourceInd.isDirectory)
    {
        std::cout << "NOT A FILE" << std::endl;
        return 1;
    }

    // Check if the destination dir exists and if there is not another dirItem with the same name
    std::vector<std::string> splitDestinationPath = LibraryMethods::split(destinationPath, '/');
    std::string destinationFileName = splitDestinationPath.at(splitDestinationPath.size() - 1);
    std::string destinationParentPath = destinationPath.substr(0, destinationPath.size() - destinationFileName.size());

    int destParentInodeAddr = this->getInodeAddressForPath(destinationParentPath);
    if (destParentInodeAddr == -1)
    {
        std::cout << "PATH NOT FOUND" << std::endl;
        return 2;
    }

    inode destParentInd = {};
    this->readFromFS(indArr, sizeof(inode), destParentInodeAddr);
    memcpy(&destParentInd, indArr, sizeof(inode));

    if (this->findDirItemInInode(destinationPath, destParentInd) != -1)
    {
        std::cout << "EXIST" << std::endl;
        return 3;
    }

    // Get the inode
    int inodeIndex = this->getFreeInode();
    if (inodeIndex == -1)
    {
        std::cout << "NO FREE INODES" << std::endl;
        return 4;
    }

    int destInodeAddress = this->sb->inodeStartAddress + inodeIndex * sizeof(inode);

    // Add new dirItem to destination parent
    std::vector<std::string> splitName = LibraryMethods::split(destinationFileName, '.');
    char cName[8];
    memset(cName, 0, 8);
    memcpy(cName, splitName.at(0).c_str(), splitName.at(0).size());

    char extension[3];
    memset(extension, 0, 3);
    if (splitName.size() == 2)
        memcpy(extension, splitName.at(1).c_str(), splitName.at(1).size());

    if (this->addDirItemToInode(cName, extension, destParentInodeAddr, destInodeAddress) == -1)
    {
        std::cout << "MAX ITEMS IN DIRECTORY" << std::endl;
        return 5;
    }

    // Get the new inode ready
    inode destInd = {};
    memset(&destInd, 0, sizeof(inode));
    destInd.nodeid = inodeIndex + 1;
    destInd.references = 1;
    destInd.fileSize = sourceInd.fileSize;
    destInd.isDirectory = false;

    // Check if we have enough space
    int freeBlocks = this->getFreeBlocksNum();
    int referencesPerBlock = this->sb->blockSize / sizeof(int32_t);
    int blocksNecessary;

    int temp = destInd.fileSize / this->sb->blockSize;
    if (destInd.fileSize % this->sb->blockSize != 0)
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
                this->removeDirItemFromInode(destinationFileName, destParentInodeAddr);
                std::cout << "FILE TOO LARGE" << std::endl;
                return 7;
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
        this->removeDirItemFromInode(destinationFileName, destParentInodeAddr);
        return 6;
    }

    // Do the copying itself - it's more or less a mirror of reading bytes from file, we only read the blocks from the source inode
    int i = 0;
    int bytesToGo = destInd.fileSize;
    int destDirects[5];
    int sourceDirects[5] = {sourceInd.direct1, sourceInd.direct2, sourceInd.direct3, sourceInd.direct4, sourceInd.direct5};
    memset(destDirects, 0, 5 * sizeof(int32_t));
    for (; i < 5; i++)
    {
        if (destDirects[i] == 0)
        {
            int index = this->getFreeBlock();
            destDirects[i] = this->sb->blockStartAddress + this->sb->blockSize * index;
            this->toggleBitInBitmap(index, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
            char bytes[this->sb->blockSize];
            memset(bytes, 0, this->sb->blockSize);
            if (bytesToGo > this->sb->blockSize) {
                this->readFromFS(bytes, this->sb->blockSize, sourceDirects[i]);
                this->fillBlock(destDirects[i], bytes, this->sb->blockSize);
            }
            else {
                this->readFromFS(bytes, bytesToGo, sourceDirects[i]);
                this->fillBlock(destDirects[i], bytes, bytesToGo);
                destInd.direct1 = destDirects[0];
                destInd.direct2 = destDirects[1];
                destInd.direct3 = destDirects[2];
                destInd.direct4 = destDirects[3];
                destInd.direct5 = destDirects[4];
                memcpy(indArr, &destInd, sizeof(inode));
                this->writeToFS(indArr, sizeof(inode), destInodeAddress);
                return 0;
            }
            bytesToGo -= this->sb->blockSize;
        }
    }

    destInd.direct1 = destDirects[0];
    destInd.direct2 = destDirects[1];
    destInd.direct3 = destDirects[2];
    destInd.direct4 = destDirects[3];
    destInd.direct5 = destDirects[4];

    int blockIndex = this->getFreeBlock();
    this->toggleBitInBitmap(blockIndex, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
    destInd.indirect1 = this->sb->blockStartAddress + blockIndex * this->sb->blockSize;
    fillIndirect1(destInd.indirect1, sourceInd.indirect1, bytesToGo, this);

    // If the remaining bytes fitted into the indirect1
    if (bytesToGo <= (this->sb->blockSize / sizeof(int32_t)) * this->sb->blockSize) {
        memcpy(indArr, &destInd, sizeof(inode));
        this->writeToFS(indArr, sizeof(inode), destInodeAddress);
        return 0;
    }
    else    // else we continue
        bytesToGo -= (this->sb->blockSize / sizeof(int32_t)) * this->sb->blockSize;

    blockIndex = this->getFreeBlock();
    this->toggleBitInBitmap(blockIndex, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
    destInd.indirect2 = this->sb->blockStartAddress + blockIndex * this->sb->blockSize;
    fillIndirect2(destInd.indirect2, sourceInd.indirect2, bytesToGo, this);

    // Write the inode and toggle the bit in the map
    this->toggleBitInBitmap(inodeIndex, this->sb->inodeMapStartAddress, this->sb->inodeStartAddress - this->sb->inodeMapStartAddress);
    memcpy(indArr, &destInd, sizeof(inode));
    this->writeToFS(indArr, sizeof(inode), destInodeAddress);

    return 0;
}