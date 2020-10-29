#include <cstring>
#include "FileSystem.h"

int FileSystem::writeToFS(char *bytes, int length, int32_t address)
{
    this->fsFile.seekp(address);
    this->fsFile.write(bytes, length);
    this->fsFile.flush();
    return 0;
}

int FileSystem::readFromFS(char *bytes, int length, int32_t address)
{
    this->fsFile.seekg(address);
    this->fsFile.read(bytes, length);
    this->fsFile.flush();
    return 0;
}



