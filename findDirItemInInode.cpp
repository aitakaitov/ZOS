#include <cstring>
#include "FileSystem.h"
#include "LibraryMethods.h"

int FileSystem::searchDirect(int address, const char *name)
{
    int dirItemsInBlock = BLOCK_SIZE / sizeof(directoryItem);
    char blockArr[BLOCK_SIZE];
    this->readFromFS(blockArr, BLOCK_SIZE, address);

    for (int i = 0; i < dirItemsInBlock; i++)
    {
        directoryItem di = {};
        memcpy(&di, blockArr + i*sizeof(directoryItem), sizeof(directoryItem));
        if (memcmp(name, di.itemName, 12) == 0)
            return address + i * (int)sizeof(directoryItem);
    }

    return -1;
}


// returns directoryItem address, -1 if not found
int FileSystem::findDirItemInInode(const std::string& name, inode ind)
{
    char itemName[12];
    memset(itemName, 0, 12);

    if (name != ".." && name != ".")
    {
        std::vector<std::string> splitName = LibraryMethods::split(name, '.');
        char cName[8];
        memset(cName, 0, 8);
        memcpy(cName, splitName.at(0).c_str(), splitName.at(0).size());

        char extension[3];
        memset(extension, 0, 3);
        if (splitName.size() == 2)
            memcpy(extension, splitName.at(1).c_str(), splitName.at(1).size());

        memset(itemName, 0, 12);
        memcpy(itemName, cName, 8);
        memcpy(itemName + 8, extension, 3);
    }
    else if (name == "..")
    {
        itemName[0] = '.';
        itemName[1] = '.';
    }
    else
        {
            itemName[0] = '.';
        }


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

