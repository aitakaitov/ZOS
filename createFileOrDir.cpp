#include <vector>
#include <cstring>
#include <iostream>
#include "FileSystem.h"
#include "LibraryMethods.h"

// Creates a directory or a file given a path and data (data only in case it is a file)
// Was created at the beginning of the project and is very crude and could be simplified
// It however works
// 0    = OK
// 1    = EXIST
// 2    = NO FREE INODE
// 3    = NO FREE BLOCK
// 4    = MAX ITEMS IN DIRECTORY (or NO FREE BLOCK)
// 5    = PATH NOT FOUND
// -1   = ERROR IN CREATING THE INODE
int FileSystem::createFileOrDir(std::string path, bool isDirectory, FILE *file)
{
    const char fslash = '/';
    inode ind = {};
    int inodeAddress;

    // Test if the path is absolute - remove the / and make our starting point root inode
    if (path.at(0) == fslash)
    {
        path = path.substr(1);
        char inodeArr[sizeof(inode)];
        this->readFromFS(inodeArr, sizeof(inode), this->sb->inodeStartAddress);
        memcpy(&ind, inodeArr, sizeof(inode));
        inodeAddress = this->sb->inodeStartAddress;
    }
    else    // Otherwise, make our starting point current inode
        {
            char inodeArr[sizeof(inode)];
            this->readFromFS(inodeArr, sizeof(inode), this->currentInodeAddress);
            memcpy(&ind, inodeArr, sizeof(inode));
            inodeAddress = this->currentInodeAddress;
        }

    std::vector<std::string> splitPath = LibraryMethods::split(path, '/');

    // Root directory cannot be created
    if (splitPath.empty())
    {
        std::cout << "EXIST" << std::endl;
        return 1;
    }
    // Create file in the 1st directory (the one for which we have inode in ind)
    else if (splitPath.size() == 1)
    {
        int address = this->findDirItemInInode(splitPath.at(0), ind);
        if (address != -1)
        {
            std::cout << "EXIST" << std::endl;
            return 1;
        }
        // We need inode to represent the item
        int inodeIndex = this->getFreeInode();
        if (inodeIndex == -1)
        {
            std::cout << "NO FREE INODE" << std::endl;
            return 2;
        }
        // We need a block for the inode.direct1
        int blockIndex = this->getFreeBlock();
        if (blockIndex == -1)
        {
            std::cout << "NO FREE BLOCK" << std::endl;
            return 3;
        }
        // Reserve a block for the new inode
        this->toggleBitInBitmap(blockIndex, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
        // We need to add a directory item to the parent inode, which could require another block,
        // in case the ones allocated are full
        char name[FILENAME_MAX_SIZE];
        char extension[EXTENSION_MAX_SIZE];
        LibraryMethods::parseName(splitPath.at(0), name, extension, isDirectory);

        int res = this->addDirItemToInode(name, extension, inodeAddress, this->sb->inodeStartAddress + inodeIndex * sizeof(inode));
        // If there is no space to allocate the DI, un-reserve the block
        if (res != 0)
        {
            this->toggleBitInBitmap(blockIndex, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
            std::cout << "MAX ITEMS IN DIRECTORY" << std::endl;
            return 4;
        }

        if (isDirectory)
        {
            res = this->createInode(inodeIndex, blockIndex, inodeAddress, NULL, isDirectory);
            // If we are unsuccessful in creating the inode, we un-allocate the block
            if (res != 0)
            {
                this->removeDirItemFromInode(splitPath.at(0), inodeAddress);
                return -1;
            }
            else    // Otherwise, we allocate the inode in the bitmap
                {
                    this->toggleBitInBitmap(inodeIndex, this->sb->inodeMapStartAddress, this->sb->inodeStartAddress - this->sb->inodeMapStartAddress);
                    return 0;
                }
        }
        else
            {
                res = this->createInode(inodeIndex, blockIndex, 0, file, isDirectory);
                if (res != 0)
                {
                    this->removeDirItemFromInode(splitPath.at(0), inodeAddress);
                    return -1;
                }
                else
                {
                    this->toggleBitInBitmap(inodeIndex, this->sb->inodeMapStartAddress, this->sb->inodeStartAddress - this->sb->inodeMapStartAddress);
                    return 0;
                }
            }

    }
    else
        {
            std::string lastFile = splitPath.at(splitPath.size() - 1);
            std::string newPath = path.substr(0, path.size() - lastFile.size());
            inodeAddress = this->getInodeAddressForPath(newPath);
            if (inodeAddress == -1)
            {
                std::cout << "PATH NOT FOUND" << std::endl;
                return 5;
            }
            char indArr[sizeof(inode)];
            this->readFromFS(indArr, sizeof(inode), inodeAddress);
            memcpy(&ind, indArr, sizeof(inode));

            int address = this->findDirItemInInode(splitPath.at(splitPath.size() - 1), ind);
            if (address != -1)
            {
                std::cout << "EXIST" << std::endl;
                return 1;
            }
            // We need inode to represent the item
            int inodeIndex = this->getFreeInode();
            if (inodeIndex == -1)
            {
                std::cout << "NO FREE INODE" << std::endl;
                return 2;
            }
            // We need a block for the inode.direct1
            int blockIndex = this->getFreeBlock();
            if (blockIndex == -1)
            {
                std::cout << "NO FREE BLOCK" << std::endl;
                return 3;
            }
            // Reserve a block for the new inode
            this->toggleBitInBitmap(blockIndex, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
            // We need to add a directory item to the parent inode, which could require another block,
            // in case the ones allocated are full
            char name[FILENAME_MAX_SIZE];
            char extension[EXTENSION_MAX_SIZE];
            LibraryMethods::parseName(lastFile, name, extension, isDirectory);

            int res = this->addDirItemToInode(name, extension, inodeAddress, this->sb->inodeStartAddress + inodeIndex * sizeof(inode));
            // If there is no space to allocate the DI, un-reserve the block
            if (res != 0)
            {
                this->toggleBitInBitmap(blockIndex, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
                std::cout << "MAX ITEMS IN DIRECTORY" << std::endl;
                return 4;
            }

            if (isDirectory)
            {
                res = this->createInode(inodeIndex, blockIndex, inodeAddress, NULL, isDirectory);
                // If we are unsuccessful in creating the inode, we un-allocate the block
                if (res != 0)
                {
                    this->removeDirItemFromInode(lastFile, inodeAddress);
                    return -1;
                }
                else    // Otherwise, we allocate the inode in the bitmap
                {
                    this->toggleBitInBitmap(inodeIndex, this->sb->inodeMapStartAddress, this->sb->inodeStartAddress - this->sb->inodeMapStartAddress);
                    return 0;
                }
            }
            else
            {
                res = this->createInode(inodeIndex, blockIndex, 0, file, isDirectory);
                if (res != 0)
                {
                    this->removeDirItemFromInode(lastFile, inodeAddress);
                    return -1;
                }
                else
                {
                    this->toggleBitInBitmap(inodeIndex, this->sb->inodeMapStartAddress, this->sb->inodeStartAddress - this->sb->inodeMapStartAddress);
                    return 0;
                }
            }

        }

    return 0;
}
