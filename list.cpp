#include <cstring>
#include <iostream>
#include <vector>
#include "FileSystem.h"

std::vector<directoryItem> FileSystem::getAllDirItemsFromDirect(int blockAddress)
{
    char blockArr[BLOCK_SIZE];
    std::vector<directoryItem> returnVector;
    this->readFromFS(blockArr, BLOCK_SIZE, blockAddress);

    for (int i = 0; i < BLOCK_SIZE / sizeof(directoryItem); i++)
    {
        directoryItem di = {};
        memcpy(&di, blockArr + i * sizeof(directoryItem), sizeof(directoryItem));
        if (strcmp(di.itemName, "") != 0)
            returnVector.emplace_back(di);
    }

    return returnVector;
}

int FileSystem::list(int inodeAddress)
{
    // Load the inode
    inode ind = {};
    char inodeArr[sizeof(inode)];
    this->readFromFS(inodeArr, sizeof(inode), inodeAddress);
    memcpy(&ind, inodeArr, sizeof(inode));

    if (!ind.isDirectory)
    {
        std::cout << "NOT A DIRECTORY" << std::endl;
        return 1;
    }

    std::vector<directoryItem> directoryItems;
    std::vector<directoryItem> partialResult;

    partialResult = getAllDirItemsFromDirect(ind.direct1);
    for (auto di : partialResult)
        directoryItems.emplace_back(di);

    if (ind.direct2 != 0)
    {
        partialResult = getAllDirItemsFromDirect(ind.direct2);
        for (auto di : partialResult)
            directoryItems.emplace_back(di);
    }

    if (ind.direct3 != 0)
    {
        partialResult = getAllDirItemsFromDirect(ind.direct3);
        for (auto di : partialResult)
            directoryItems.emplace_back(di);
    }

    if (ind.direct4 != 0)
    {
        partialResult = getAllDirItemsFromDirect(ind.direct4);
        for (auto di : partialResult)
            directoryItems.emplace_back(di);
    }

    if (ind.direct5 != 0)
    {
        partialResult = getAllDirItemsFromDirect(ind.direct5);
        for (auto di : partialResult)
            directoryItems.emplace_back(di);
    }

    for (auto di : directoryItems)
    {
        inode diInode = {};
        char arr[sizeof(inode)];
        this->readFromFS(arr, sizeof(inode), di.inode);
        memcpy(&diInode, arr, sizeof(inode));

        char name[9] = {0};
        memcpy(name, di.itemName, 8);
        char extension[4] = {0};
        memcpy(extension, di.itemName + 8, 3);

        if (diInode.isDirectory)
        {
            std::cout << "+ " << name << std::endl;
        }
        else
            {
                std::cout << "- " << name;
                if (strcmp(extension, "") != 0)
                    std::cout << "." << extension << std::endl;
                else
                    std::cout << std::endl;
            }
    }

    return 0;
}
