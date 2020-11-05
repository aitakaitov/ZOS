#include "FileSystem.h"
#include "LibraryMethods.h"

int FileSystem::moveFile(std::string filePath, std::string newPath)
{
    std::vector<std::string> filePathSplit = LibraryMethods::split(filePath, '/');
    std::string fileName = filePathSplit.at(filePathSplit.size() - 1);
    //filePath =
}

