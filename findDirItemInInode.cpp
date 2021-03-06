#include <cstring>
#include "FileSystem.h"
#include "LibraryMethods.h"

// searches a directly referenced block for a directoryItem
// the name parameter is expected to be 12 bytes long
// >0   = DI ADDRESS
// -1   = NOT FOUND
int FileSystem::searchDirect(int address, const char *name)
{

    int dirItemsInBlock = this->sb->blockSize / sizeof(directoryItem);
    char blockArr[this->sb->blockSize];
    this->readFromFS(blockArr, this->sb->blockSize, address);

    for (int i = 0; i < dirItemsInBlock; i++)
    {
        directoryItem di = {};
        memcpy(&di, blockArr + i*sizeof(directoryItem), sizeof(directoryItem));
        if (memcmp(name, di.itemName, FILENAME_MAX_SIZE + EXTENSION_MAX_SIZE + 1) == 0)
            return address + i * (int)sizeof(directoryItem);
    }

    this->readFromFS(blockArr, this->sb->blockSize, address);
    for (int i = 0; i < dirItemsInBlock; i++)
    {
        directoryItem di = {};
        memcpy(&di, blockArr + i*sizeof(directoryItem), sizeof(directoryItem));
        if (memcmp(name, di.itemName, FILENAME_MAX_SIZE + EXTENSION_MAX_SIZE + 1) == 0)
            return address + i * (int)sizeof(directoryItem);
    }

    return -1;
}


// Finds a directory item in an inode
// The name is expected to be unformatted, the method handles the conversion into FS name format.
// >0   = DI ADDRESS
// -1   = NOT FOUND
int FileSystem::findDirItemInInode(const std::string& sName, inode ind)
{
    // Try to find a file
    char name[FILENAME_MAX_SIZE];
    char extension[EXTENSION_MAX_SIZE];
    LibraryMethods::parseName(sName, name, extension, false);
    char itemName[FILENAME_MAX_SIZE + EXTENSION_MAX_SIZE + 1];
    memset(itemName, 0, FILENAME_MAX_SIZE + EXTENSION_MAX_SIZE + 1);
    memcpy(itemName, name, FILENAME_MAX_SIZE);
    memcpy(itemName + FILENAME_MAX_SIZE, extension, EXTENSION_MAX_SIZE);

    int address = -1;
    if (ind.direct1 != 0)
        address = searchDirect(ind.direct1, itemName);
    if (address != -1)
        return address;

    if (ind.direct2 != 0)
        address = searchDirect(ind.direct2, itemName);
    if (address != -1)
        return address;

    if (ind.direct3 != 0)
        address = searchDirect(ind.direct3, itemName);
    if (address != -1)
        return address;

    if (ind.direct4 != 0)
        address = searchDirect(ind.direct4, itemName);
    if (address != -1)
        return address;

    if (ind.direct5 != 0)
        address = searchDirect(ind.direct5, itemName);
    if (address != -1)
        return address;

    // Try to find a directory
    LibraryMethods::parseName(sName, name, extension, true);
    memset(itemName, 0, FILENAME_MAX_SIZE + EXTENSION_MAX_SIZE + 1);
    memcpy(itemName, name, FILENAME_MAX_SIZE);
    memcpy(itemName + 8, extension, EXTENSION_MAX_SIZE);

    address = -1;
    if (ind.direct1 != 0)
        address = searchDirect(ind.direct1, itemName);
    if (address != -1)
        return address;

    if (ind.direct2 != 0)
        address = searchDirect(ind.direct2, itemName);
    if (address != -1)
        return address;

    if (ind.direct3 != 0)
        address = searchDirect(ind.direct3, itemName);
    if (address != -1)
        return address;

    if (ind.direct4 != 0)
        address = searchDirect(ind.direct4, itemName);
    if (address != -1)
        return address;

    if (ind.direct5 != 0)
        address = searchDirect(ind.direct5, itemName);
    if (address != -1)
        return address;

    return -1;
}

