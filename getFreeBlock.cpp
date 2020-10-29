#include "FileSystem.h"
#include "LibraryMethods.h"

int FileSystem::getFreeBlock()
{
    int bytes = this->sb->blockStartAddress - this->sb->blockMapStartAddress;
    char map[bytes];
    this->readFromFS(map, bytes, this->sb->blockMapStartAddress);

    int c = 0;
    for (int i = 0; i < bytes; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            c++;
            if (c > this->sb->blockCount)
                return -1;
            if (LibraryMethods::checkBit(map[i], j) == 0)
                return i * 8 + j;
        }
    }

    return -1;
}

int FileSystem::getFreeBlocksNum()
{
    int bytes = this->sb->blockStartAddress - this->sb->blockMapStartAddress;
    char map[bytes];
    this->readFromFS(map, bytes, this->sb->blockMapStartAddress);

    int c = 0;
    int free = 0;
    for (int i = 0; i < bytes; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            c++;
            if (c > this->sb->inodeCount)
                return free;
            if (LibraryMethods::checkBit(map[i], j) == 0)
                free++;
        }
    }

    return free;
}


