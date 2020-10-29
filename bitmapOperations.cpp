#include "FileSystem.h"
#include "LibraryMethods.h"

int FileSystem::toggleBitInBitmap(int bitIndex, int bitmapAddress, int bitmapSize)
{
    char map[bitmapSize];
    this->readFromFS(map, bitmapSize, bitmapAddress);
    char c = map[bitIndex / 8];
    map[bitIndex / 8] = LibraryMethods::toggleBit(c, bitIndex % 8);
    this->writeToFS(map, bitmapSize, bitmapAddress);

    return 0;
}

