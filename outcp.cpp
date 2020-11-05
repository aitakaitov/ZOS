#include <iostream>
#include <cstring>
#include "FileSystem.h"

int FileSystem::outcp(std::string filePath, const std::string& outputPath)
{
    FILE *file = fopen(outputPath.c_str(), "w");
    if (file == NULL)
    {
        std::cout << "FILE COULD NOT BE CREATED" << std::endl;
        fclose(file);
        return 1;
    }

    rewind(file);

    int inodeAddress = this->getInodeAddressForPath(filePath);
    if (inodeAddress == -1)
    {
        std::cout << "FILE DOES NOT EXIST" << std::endl;
        fclose(file);
        return 2;
    }

    inode ind = {};
    char indArr[sizeof(inode)];
    this->readFromFS(indArr, sizeof(inode), inodeAddress);
    memcpy(&ind, indArr, sizeof(inode));

    char blockArr[BLOCK_SIZE];
    if (ind.direct1 != 0)
    {
        this->readFromFS(blockArr, BLOCK_SIZE, ind.direct1);
        fwrite(blockArr, 1, BLOCK_SIZE, file);
    }
    if (ind.direct2 != 0)
    {
        this->readFromFS(blockArr, BLOCK_SIZE, ind.direct2);
        fwrite(blockArr, 1, BLOCK_SIZE, file);
    }
    if (ind.direct3 != 0)
    {
        this->readFromFS(blockArr, BLOCK_SIZE, ind.direct3);
        fwrite(blockArr, 1, BLOCK_SIZE, file);
    }
    if (ind.direct4 != 0)
    {
        this->readFromFS(blockArr, BLOCK_SIZE, ind.direct4);
        fwrite(blockArr, 1, BLOCK_SIZE, file);
    }
    if (ind.direct5 != 0)
    {
        this->readFromFS(blockArr, BLOCK_SIZE, ind.direct5);
        fwrite(blockArr, 1, BLOCK_SIZE, file);
    }

    if (ind.indirect1 != 0)
    {
        char indirect1Block[BLOCK_SIZE];
        this->readFromFS(indirect1Block, BLOCK_SIZE, ind.indirect1);
        for (int i = 0; i < BLOCK_SIZE / sizeof(int32_t); i++)
        {
            int32_t address = {};
            memcpy(&address, indirect1Block + i * sizeof(int32_t), sizeof(int32_t));
            if (address != 0)
            {
                this->readFromFS(blockArr, BLOCK_SIZE, address);
                fwrite(blockArr, 1, BLOCK_SIZE, file);
            }
        }
    }

    if (ind.indirect2 != 0)
    {
        char indirect2Block[BLOCK_SIZE];
        this->readFromFS(indirect2Block, BLOCK_SIZE, ind.indirect2);

        for (int i = 0; i < BLOCK_SIZE / sizeof(int32_t); i++)
        {
            int32_t indirect1address = {};
            memcpy(&indirect1address, indirect2Block + i * sizeof(int32_t), sizeof(int32_t));

            if (indirect1address != 0)
            {
                char indirect1Block[BLOCK_SIZE];
                this->readFromFS(indirect1Block, BLOCK_SIZE, indirect1address);
                for (int j = 0; j < BLOCK_SIZE / sizeof(int32_t); j++)
                {
                    int32_t directAddress = {};
                    memcpy(&directAddress, indirect1Block + j * sizeof(int32_t), sizeof(int32_t));
                    if (directAddress != 0)
                    {
                        this->readFromFS(blockArr, BLOCK_SIZE, directAddress);
                        fwrite(blockArr, 1, BLOCK_SIZE, file);
                    }
                }
            }
        }
    }

    fclose(file);
    return 0;
}
