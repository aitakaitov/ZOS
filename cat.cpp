#include <iostream>
#include <cstring>
#include "FileSystem.h"

// Writes the contents of a file to the shell, given a path. Does not support directories.
// 1    = FILE NOT FOUND
// 2    = NOT A FILE
// 0    = OK
int FileSystem::cat(std::string filePath)
{
    int inodeAddress = this->getInodeAddressForPath(filePath);
    if (inodeAddress == -1)
    {
        std::cout << "FILE NOT FOUND" << std::endl;
        return 1;
    }

    inode ind = {};
    char indArr[sizeof(inode)];
    this->readFromFS(indArr, sizeof(inode), inodeAddress);
    memcpy(&ind, indArr, sizeof(inode));

    if (ind.isDirectory)
    {
        std::cout << "NOT A FILE";
        return 2;
    }

    char blockArr[this->sb->blockSize + 1];
    blockArr[this->sb->blockSize] = '\0';
    if (ind.direct1 != 0)
    {
        this->readFromFS(blockArr, this->sb->blockSize, ind.direct1);
        std::cout << blockArr;
    }
    if (ind.direct2 != 0)
    {
        this->readFromFS(blockArr, this->sb->blockSize, ind.direct2);
        std::cout << blockArr;
    }
    if (ind.direct3 != 0)
    {
        this->readFromFS(blockArr, this->sb->blockSize, ind.direct3);
        std::cout << blockArr;
    }
    if (ind.direct4 != 0)
    {
        this->readFromFS(blockArr, this->sb->blockSize, ind.direct4);
        std::cout << blockArr;
    }
    if (ind.direct5 != 0)
    {
        this->readFromFS(blockArr, this->sb->blockSize, ind.direct5);
        std::cout << blockArr;
    }

    if (ind.indirect1 != 0)
    {
        char indirect1Block[this->sb->blockSize];
        this->readFromFS(indirect1Block, this->sb->blockSize, ind.indirect1);
        for (int i = 0; i < this->sb->blockSize / sizeof(int32_t); i++)
        {
            int32_t address = {};
            memcpy(&address, indirect1Block + i * sizeof(int32_t), sizeof(int32_t));
            if (address != 0)
            {
                this->readFromFS(blockArr, this->sb->blockSize, address);
                std::cout << blockArr;
            }
        }
    }

    if (ind.indirect2 != 0)
    {
        char indirect2Block[this->sb->blockSize];
        this->readFromFS(indirect2Block, this->sb->blockSize, ind.indirect2);

        for (int i = 0; i < this->sb->blockSize / sizeof(int32_t); i++)
        {
            int32_t indirect1address = {};
            memcpy(&indirect1address, indirect2Block + i * sizeof(int32_t), sizeof(int32_t));

            if (indirect1address != 0)
            {
                char indirect1Block[this->sb->blockSize];
                this->readFromFS(indirect1Block, this->sb->blockSize, indirect1address);
                for (int j = 0; j < this->sb->blockSize / sizeof(int32_t); j++)
                {
                    int32_t directAddress = {};
                    memcpy(&directAddress, indirect1Block + j * sizeof(int32_t), sizeof(int32_t));
                    if (directAddress != 0)
                    {
                        this->readFromFS(blockArr, this->sb->blockSize, directAddress);
                        std::cout << blockArr;
                    }
                }
            }
        }
    }

    std::cout << std::endl;
    return 0;
}
