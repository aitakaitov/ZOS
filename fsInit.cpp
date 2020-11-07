#include <iostream>
#include <cstring>
#include "FileSystem.h"
#include "LibraryMethods.h"

// Creates a new filesystem given a name, and a size
// 1    = ERR
// 0    = OK
int FileSystem::createFileSystem(std::string path, int sizeBytes) {
    this->fsFile.close();
    remove(path.c_str());

    if (path.size() > FS_NAME_LENGTH)
    {
        path = path.substr(0, 12);
    }

    this->fsFile.open(path, std::ios::out);

    if (!this->fsFile) {
        std::cout << "CANNOT CREATE FILE" << std::endl;
        return 1;
    }

    for (int i = 0; i < sizeBytes; i++) {
        this->fsFile.write("\0", sizeof(char));
    }

    this->fsFile.close();
    this->fsFile.open(path, std::ios::out | std::ios::in | std::ios::binary);

    if (!this->fsFile) {
        std::cout << "COULD NOT OPEN FILE" << std::endl;
        return 1;
    }

    int32_t blockCount, blockStartAddress, inodeStartAddress, blockMapStartAddress, inodeMapStartAddress;
    int inodeCount = (sizeBytes / BYTES_PER_INODE);// + ((sizeBytes / BYTES_PER_INODE) % 8);
    inodeMapStartAddress = sizeof(superblock);
    inodeStartAddress = inodeMapStartAddress + (inodeCount / 8) * sizeof(char);
    blockMapStartAddress = inodeStartAddress + inodeCount * sizeof(inode);

    blockCount = 0;
    int freeSpace = sizeBytes - blockMapStartAddress;
    for (int i = 0; i < freeSpace; i++) {
        if (i * BLOCK_SIZE + i * (1 / 8) > freeSpace) {
            blockCount = i - 1;
            break;
        }
    }

    blockStartAddress = blockMapStartAddress + (blockCount / 8 + 1) * sizeof(char);

    this->sb = new superblock {};
    sb->diskSize = sizeBytes;
    sb->blockSize = BLOCK_SIZE;
    sb->blockCount = blockCount;
    sb->inodeCount = inodeCount;
    sb->inodeMapStartAddress = inodeMapStartAddress;
    sb->blockMapStartAddress = blockMapStartAddress;
    sb->inodeStartAddress = inodeStartAddress;
    sb->blockStartAddress = blockStartAddress;

    memset(&sb->name, 0, FS_NAME_LENGTH);
    memcpy(&sb->name, &path[0], path.size());

    std::cout << "size: " << sb->diskSize << std::endl << "Superblock start: 0\nInode map start: " << inodeMapStartAddress << "\nInode start address: " << inodeStartAddress << std::endl;
    std::cout << "Block map start: " << blockMapStartAddress << "\nBlock start address: " << blockStartAddress << "\nBlock end address: " << blockStartAddress + blockCount * BLOCK_SIZE << std::endl;
    std::cout << "Block count: " << blockCount << "(block size = " << BLOCK_SIZE << ")" << std::endl << "Inode count: " << inodeCount << "(inode size = " << sizeof(inode) << ")" << std::endl;

    char superblockCharArr[sizeof(superblock)];
    memset(superblockCharArr, 0, sizeof(superblock));
    memcpy(superblockCharArr, this->sb, sizeof(superblock));
    this->writeToFS(superblockCharArr, sizeof(superblock), 0);
    this->createRoot();

    return 0;
}

// Loads a filesystem from a file, given a path
// 1    = ERR
// 0    = OK
int FileSystem::loadFileSystem(std::string path) {
    this->fsFile.open(path, std::ios::out | std::ios::in | std::ios::binary);

    if (!FileSystem::fsFile)
    {
        std::cout << "COULD NOT LOAD FILE" << std::endl;
        return 1;
    }

    char superblockCharArr[sizeof(superblock)];

    this->readFromFS(superblockCharArr, sizeof(superblock), 0);
    this->sb = new superblock {0,0,0,0,0,0,0};
    memcpy(this->sb, superblockCharArr, sizeof(superblock));

    return 0;
}

// Creates the root directory
// 0    = OK
int FileSystem::createRoot()
{
    int inodeAddress = this->sb->inodeStartAddress;
    int blockAddress = this->sb->blockStartAddress;
    int inodeMapAddress = this->sb->inodeMapStartAddress;
    int blockMapAddress = this->sb->blockMapStartAddress;

    inode ind = inode {};
    ind.isDirectory = true;
    ind.nodeid = 1;
    ind.direct1 = blockAddress;
    ind.references = 1;
    ind.fileSize = BLOCK_SIZE;
    char indArr[sizeof(inode)];
    memset(indArr, 0, sizeof(inode));
    memcpy(indArr, &ind, sizeof(inode));
    this->writeToFS(indArr, sizeof(inode), inodeAddress);

    directoryItem di = directoryItem {};
    memset(&di, 0, sizeof(directoryItem));
    di.itemName[0] = '.';
    di.inode = inodeAddress;

    char diArr[sizeof(directoryItem)];
    memset(diArr, 0, sizeof(directoryItem));
    memcpy(diArr, &di, sizeof(directoryItem));
    this->writeToFS(diArr, sizeof(directoryItem), blockAddress);

    char c1 = LibraryMethods::toggleBit('\0', 0);

    this->writeToFS(&c1, 1, inodeMapAddress);
    this->writeToFS(&c1, 1, blockMapAddress);

    this->currentInodeAddress = inodeAddress;

    return 0;
}

