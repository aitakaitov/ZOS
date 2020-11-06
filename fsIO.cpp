#include "FileSystem.h"

// Writes char bytes to filesystem
// bytes - what to write
// length - how much of it
// address - to where
int FileSystem::writeToFS(char *bytes, int length, int32_t address)
{
    this->fsFile.seekp(address);
    this->fsFile.write(bytes, length);
    this->fsFile.flush();
    return 0;
}

// reads char bytes from filesystem
// bytes - where to store it
// length - how much of it
// address - from where
int FileSystem::readFromFS(char *bytes, int length, int32_t address)
{
    this->fsFile.seekg(address);
    this->fsFile.read(bytes, length);
    this->fsFile.flush();
    return 0;
}



