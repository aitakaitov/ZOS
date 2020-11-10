#include <iostream>
#include <cstring>
#include "FileSystem.h"

// Copies the file in filePath to outputPath (outside of FS)
// 1    = FILE COULD NOT BE CREATED
// 2    = FILE DOES NOT EXIST
// 3    = NOT A FILE
// 0    = OK
int FileSystem::outcp(std::string filePath, const std::string& outputPath)
{
    // Create the file outside of FS
    FILE *file = fopen(outputPath.c_str(), "w");
    if (file == NULL)
    {
        std::cout << "PATH NOT FOUND" << std::endl;
        fclose(file);
        return 1;
    }
    rewind(file);

    // Check if file exists
    int inodeAddress = this->getInodeAddressForPath(filePath);
    if (inodeAddress == -1)
    {
        std::cout << "FILE NOT FOUND" << std::endl;
        fclose(file);
        return 2;
    }

    inode ind = {};
    char indArr[sizeof(inode)];
    this->readFromFS(indArr, sizeof(inode), inodeAddress);
    memcpy(&ind, indArr, sizeof(inode));

    if (ind.isDirectory)
    {
        std::cout << "NOT A FILE" << std::endl;
        return 3;
    }

    int bytesToGo = ind.fileSize;

    // Loop trough directs
    char blockArr[this->sb->blockSize];
    if (ind.direct1 != 0 && bytesToGo > this->sb->blockSize)
    {
        this->readFromFS(blockArr, this->sb->blockSize, ind.direct1);
        fwrite(blockArr, 1, this->sb->blockSize, file);
        bytesToGo -= this->sb->blockSize;
    }
    else
        {
            this->readFromFS(blockArr, bytesToGo, ind.direct1);
            fwrite(blockArr, 1, bytesToGo, file);
            fclose(file);
            return 0;
        }

    if (ind.direct2 != 0 && bytesToGo > this->sb->blockSize)
    {
        this->readFromFS(blockArr, this->sb->blockSize, ind.direct2);
        fwrite(blockArr, 1, this->sb->blockSize, file);
        bytesToGo -= this->sb->blockSize;
    }
    else
        {
            this->readFromFS(blockArr, bytesToGo, ind.direct2);
            fwrite(blockArr, 1, bytesToGo, file);
            fclose(file);
            return 0;
        }

    if (ind.direct3 != 0 && bytesToGo > this->sb->blockSize)
    {
        this->readFromFS(blockArr, this->sb->blockSize, ind.direct3);
        fwrite(blockArr, 1, this->sb->blockSize, file);
        bytesToGo -= this->sb->blockSize;
    }
    else
        {
            this->readFromFS(blockArr, bytesToGo, ind.direct3);
            fwrite(blockArr, 1, bytesToGo, file);
            fclose(file);
            return 0;
        }

    if (ind.direct4 != 0 && bytesToGo > this->sb->blockSize)
    {
        this->readFromFS(blockArr, this->sb->blockSize, ind.direct4);
        fwrite(blockArr, 1, this->sb->blockSize, file);
        bytesToGo -= this->sb->blockSize;
    }
    else
        {
            this->readFromFS(blockArr, bytesToGo, ind.direct4);
            fwrite(blockArr, 1, bytesToGo, file);
            fclose(file);
            return 0;
        }

    if (ind.direct5 != 0 && bytesToGo > this->sb->blockSize)
    {
        this->readFromFS(blockArr, this->sb->blockSize, ind.direct5);
        fwrite(blockArr, 1, this->sb->blockSize, file);
        bytesToGo -= this->sb->blockSize;
    }
    else
        {
            this->readFromFS(blockArr, bytesToGo, ind.direct5);
            fwrite(blockArr, 1, bytesToGo, file);
            fclose(file);
            return 0;
        }

    // Loop trough indirect1
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
                if (bytesToGo > this->sb->blockSize)
                {
                    this->readFromFS(blockArr, this->sb->blockSize, address);
                    fwrite(blockArr, 1, this->sb->blockSize, file);
                    bytesToGo -= this->sb->blockSize;
                }
                else
                    {
                        this->readFromFS(blockArr, bytesToGo, address);
                        fwrite(blockArr, 1, bytesToGo, file);
                        fclose(file);
                        return 0;
                    }
            }
        }
    }

    // Loop trough indirect2
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
                        if (bytesToGo > this->sb->blockSize)
                        {
                            this->readFromFS(blockArr, this->sb->blockSize, directAddress);
                            fwrite(blockArr, 1, this->sb->blockSize, file);
                            bytesToGo -= this->sb->blockSize;
                        }
                        else
                            {
                                this->readFromFS(blockArr, bytesToGo, directAddress);
                                fwrite(blockArr, 1, bytesToGo, file);
                                fclose(file);
                                return 0;
                            }
                    }
                }
            }
        }
    }

    fclose(file);
    return 0;
}
