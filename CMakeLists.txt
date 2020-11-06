cmake_minimum_required(VERSION 3.17)
project(ZOS)

set(CMAKE_CXX_STANDARD 14)

add_executable(ZOS fs.cpp FileSystem.h fsIO.cpp cli.cpp createDirectoryItem.cpp LibraryMethods.cpp LibraryMethods.h findDirItemInInode.cpp getFreeInode.cpp getFreeBlock.cpp addDirItemToInode.cpp bitmapOperations.cpp createInode.cpp removeDirItemFromInode.cpp getInodeAddressForPath.cpp fsInit.cpp list.cpp removeDirectory.cpp removeFile.cpp outcp.cpp cd.cpp getAbsolutePathToInode.cpp info.cpp cat.cpp moveFile.cpp ln.cpp copyFile.cpp)