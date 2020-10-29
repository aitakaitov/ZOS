#include <cstring>
#include "FileSystem.h"

int FileSystem::searchDirect(int address, std::string name)
{
    int dirItemsInBlock = BLOCK_SIZE / sizeof(directoryItem);
    char blockArr[BLOCK_SIZE];
    this->readFromFS(blockArr, BLOCK_SIZE, address);

    for (int i = 0; i < dirItemsInBlock; i++)
    {
        directoryItem di = {};
        memcpy(&di, blockArr + i*sizeof(directoryItem), sizeof(directoryItem));
        if (di.itemName == name)
            return address + i * (int)sizeof(directoryItem);
    }

    return -1;
}

/*int FileSystem::searchIndirect1(int address, std::string name)
{
    char blockArr[BLOCK_SIZE];
    this->readFromFS(blockArr, BLOCK_SIZE, address);
    int pointersInBlock = BLOCK_SIZE / sizeof(int32_t);
    for (int i = 0; i < pointersInBlock; i++)
    {
        int32_t add;
        memcpy(&add, blockArr + i * sizeof(int32_t), sizeof(int32_t));
        int returnValue = searchDirect(add, name);
        if (returnValue != -1)
            return returnValue;
    }

    return -1;
}

int FileSystem::searchIndirect2(int address, std::string name)
{
    char blockArr[BLOCK_SIZE];
    this->readFromFS(blockArr, BLOCK_SIZE, address);
    int pointersInBlock = BLOCK_SIZE / sizeof(int32_t);
    for (int i = 0; i < pointersInBlock; i++)
    {
        int32_t add;
        memcpy(reinterpret_cast<void *>(add), blockArr + i * sizeof(int32_t), sizeof(int32_t));
        int returnValue = searchIndirect1(add, name);
        if (returnValue != -1)
            return returnValue;
    }

    return -1;
}*/

// returns directoryItem address, -1 if not found
int FileSystem::findDirItemInInode(const std::string& name, inode ind)
{
    int address = -1;
    if (ind.direct1 != 0)
        address = searchDirect(ind.direct1, name);
    if (address != -1)
        return address;

    if (ind.direct2 != 0)
        address = searchDirect(ind.direct2, name);
    if (address != -1)
        return address;

    if (ind.direct3 != 0)
        address = searchDirect(ind.direct3, name);
    if (address != -1)
        return address;

    if (ind.direct4 != 0)
        address = searchDirect(ind.direct4, name);
    if (address != -1)
        return address;

    if (ind.direct5 != 0)
        address = searchDirect(ind.direct5, name);
    if (address != -1)
        return address;

    /*if (ind.indirect1 != 0)
        address = searchIndirect1(ind.indirect1, name);
    if (address != -1)
        return address;

    if (ind.indirect2 != 0)
        address = searchIndirect2(ind.indirect2, name);
    if (address != -1)
        return address;*/

    return -1;
}

