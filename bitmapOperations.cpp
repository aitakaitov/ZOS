#include "FileSystem.h"
#include "LibraryMethods.h"

// Toggles bitIndex-th bit in a bitmap.
// bitIndex - index of the bit
// bitmapAddress - where in the FS the bitmap starts
// bitmapSize - how long the bitmap is (bytes)
// 0    = OK
int FileSystem::toggleBitInBitmap(int bitIndex, int bitmapAddress, int bitmapSize)
{
    char map[bitmapSize];
    this->readFromFS(map, bitmapSize, bitmapAddress);
    char c = map[bitIndex / 8];
    map[bitIndex / 8] = LibraryMethods::toggleBit(c, bitIndex % 8);
    this->writeToFS(map, bitmapSize, bitmapAddress);

    return 0;
}

