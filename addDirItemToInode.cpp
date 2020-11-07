#include <cstring>
#include "FileSystem.h"

// Adds a directoryItem to a direct reference.
// Intended to be called by addDirItemToInode.
// If the block is not allocated, will attempt to do so.
// -1   = Unable to add the DI - either no free blocks or no free space in the block
// 0    = DI added to the block
// >0   = Address of a block, that was allocated
int FileSystem::addDirItemToDirect(char *name, char *extension, int blockAddress, int inodeReference)
{
    // Get the DI ready
    directoryItem di = {};
    memcpy(di.itemName, name, 8);
    memcpy(di.itemName + 8, extension, 3);
    memset(di.itemName + 11, 0, 1);
    di.inode = inodeReference;
    char diArr[sizeof(directoryItem)];
    memcpy(diArr, &di, sizeof(directoryItem));

    // If the direct is not initialized yet, get a block and write the DI in it
    if (blockAddress == 0)
    {
        int freeBlockIndex = this->getFreeBlock();
        if (freeBlockIndex == -1)
            return -1;

        this->toggleBitInBitmap(freeBlockIndex, this->sb->blockMapStartAddress, this->sb->blockStartAddress - this->sb->blockMapStartAddress);
        blockAddress = this->sb->blockStartAddress + freeBlockIndex * BLOCK_SIZE;
        this->writeToFS(diArr, sizeof(directoryItem), blockAddress);

        return blockAddress;
    }
    else
        {
            int dirItemAddress = this->searchDirect(blockAddress, "");
            if (dirItemAddress != -1)
            {
                this->writeToFS(diArr, sizeof(directoryItem), dirItemAddress);
                return 0;
            }
            else
                return -1;
        }
}

// name = file/dir name, 8 bytes long, the unused bytes filled with \0
// extension = file extension, 3 bytes long, the unused bytes filled with \0
// inodeAddress = inode we will place the item in
// inodeReference = inode to which the item will point
// 0    = OK
// -1   = NO FREE BLOCKS/FAIL
int FileSystem::addDirItemToInode(char *name, char *extension, int inodeAddress, int inodeReference)
{
    // Load the inode
    inode ind = {};
    char inodeArr[sizeof(inode)];
    this->readFromFS(inodeArr, sizeof(inode), inodeAddress);
    memcpy(&ind, inodeArr, sizeof(inode));

    // Get the DI ready
    directoryItem di = {};
    memcpy(di.itemName, name, 8);
    memcpy(di.itemName + 8, extension, 3);
    memset(di.itemName + 11, 0, 1);
    di.inode = inodeReference;
    char diArr[sizeof(directoryItem)];
    memcpy(diArr, &di, sizeof(directoryItem));

    int res = this->addDirItemToDirect(name, extension, ind.direct1, inodeReference);
    if (res == 0)
        return 0;

    // Otherwise, we either search another allocated block, or allocate it, if possible
    res = this->addDirItemToDirect(name, extension, ind.direct2, inodeReference);
    if (res > 0)
    {
        ind.direct2 = res;
        ind.fileSize += BLOCK_SIZE;
        memcpy(inodeArr, &ind, sizeof(inode));
        this->writeToFS(inodeArr, sizeof(inode), inodeAddress);
        return 0;
    }
    else if (res == 0)
        return 0;

    // Otherwise, we either search another allocated block, or allocate it, if possible
    res = this->addDirItemToDirect(name, extension, ind.direct3, inodeReference);
    if (res > 0)
    {
        ind.direct3 = res;
        ind.fileSize += BLOCK_SIZE;
        memcpy(inodeArr, &ind, sizeof(inode));
        this->writeToFS(inodeArr, sizeof(inode), inodeAddress);
        return 0;
    }
    else if (res == 0)
        return 0;

    // Otherwise, we either search another allocated block, or allocate it, if possible
    res = this->addDirItemToDirect(name, extension, ind.direct4, inodeReference);
    if (res > 0)
    {
        ind.direct4 = res;
        ind.fileSize += BLOCK_SIZE;
        memcpy(inodeArr, &ind, sizeof(inode));
        this->writeToFS(inodeArr, sizeof(inode), inodeAddress);
        return 0;
    }
    else if (res == 0)
        return 0;

    // Otherwise, we either search another allocated block, or allocate it, if possible
    res = this->addDirItemToDirect(name, extension, ind.direct5, inodeReference);
    if (res > 0)
    {
        ind.direct5 = res;
        ind.fileSize += BLOCK_SIZE;
        memcpy(inodeArr, &ind, sizeof(inode));
        this->writeToFS(inodeArr, sizeof(inode), inodeAddress);
        return 0;
    }
    else if (res == 0)
        return 0;

    return -1;
}

