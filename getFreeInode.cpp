#include "FileSystem.h"
#include "LibraryMethods.h"

// Checks the inode bitmap and returns the index of the free inode. If no inode is free, returns -1;
int FileSystem::getFreeInode()
{
    int bytes = this->sb->inodeStartAddress - this->sb->inodeMapStartAddress;
    char map[bytes];
    this->readFromFS(map, bytes, this->sb->inodeMapStartAddress);

    int c = 0;
    for (int i = 0; i < bytes; i++)
    {
        char byte = map[i];
        for (int j = 0; j < 8; j++)
        {
            c++;
            if (c > this->sb->inodeCount)
                return -1;
            if (LibraryMethods::checkBit(byte, j) == 0)
                return i*8 + j;
        }
    }

    return -1;
}

