#include <cstring>
#include "FileSystem.h"

// Finds a directoryItem in a directly referenced block, given an address the DI is pointing to
// >0   = DI ADDRESS
// -1   = NOT FOUND
int searchDirectAddress(int blockAddress, int targetAddress, FileSystem *fs)
{
    int dirItemsInBlock = fs->sb->blockSize / sizeof(directoryItem);
    char blockArr[fs->sb->blockSize];
    fs->readFromFS(blockArr, fs->sb->blockSize, blockAddress);

    for (int i = 0; i < dirItemsInBlock; i++)
    {
        directoryItem di = {};
        memcpy(&di, blockArr + i*sizeof(directoryItem), sizeof(directoryItem));
        if (di.inode == targetAddress)
            return blockAddress + i * (int)sizeof(directoryItem);
    }

    return -1;
}

// Finds a directoryItem in an inode, given an inode address the DI is pointing to
// >0   = DI ADDRESS
// -1   = NOT FOUND
int findDirItemInInodeByInodeAddress(int inodeAddress, inode ind, FileSystem *fs)
{
    int address = -1;
    if (ind.direct1 != 0)
        address = searchDirectAddress(ind.direct1, inodeAddress, fs);
    if (address != -1)
        return address;

    if (ind.direct2 != 0)
        address = searchDirectAddress(ind.direct2, inodeAddress, fs);
    if (address != -1)
        return address;

    if (ind.direct3 != 0)
        address = searchDirectAddress(ind.direct3, inodeAddress, fs);
    if (address != -1)
        return address;

    if (ind.direct4 != 0)
        address = searchDirectAddress(ind.direct4, inodeAddress, fs);
    if (address != -1)
        return address;

    if (ind.direct5 != 0)
        address = searchDirectAddress(ind.direct5, inodeAddress, fs);
    if (address != -1)
        return address;

    return -1;
}

// Returns an absolute path to an inode, given it's address
// Expects the address to be a valid inode address
// Backtracks using .. files until it reaches root, where is no .. file
std::string FileSystem::getAbsolutePathToInode(int address)
{
    if (address == this->sb->inodeStartAddress)
        return "/";

    inode ind = {};
    char indArr[sizeof(inode)];
    this->readFromFS(indArr, sizeof(inode), address);
    memcpy(&ind, indArr, sizeof(inode));

    // Backtrack to root
    std::vector<std::string> pathVector;
    int lastInodeAddress = address;
    while (true)
    {
        int dirItemAddress = this->findDirItemInInode("..", ind);
        if (dirItemAddress == -1)
            break;

        directoryItem di = {};
        char diArr[sizeof(directoryItem)];
        this->readFromFS(diArr, sizeof(directoryItem), dirItemAddress);
        memcpy(&di, diArr, sizeof(directoryItem));

        this->readFromFS(indArr, sizeof(inode), di.inode);
        memcpy(&ind, indArr, sizeof(inode));

        int diBackAddress = findDirItemInInodeByInodeAddress(lastInodeAddress, ind, this);
        directoryItem diBack = {};
        this->readFromFS(diArr, sizeof(directoryItem), diBackAddress);
        memcpy(&diBack, diArr, sizeof(directoryItem));

        pathVector.insert(pathVector.begin(), diBack.itemName);
        lastInodeAddress = di.inode;
    }

    std::string newPath;
    std::string fslash = "/";
    for (const auto &piece : pathVector)
    {
        newPath += fslash;
        newPath += piece;
    }
    return newPath;
}

