#include "FileSystem.h"
#include "LibraryMethods.h"

int FileSystem::info(std::string path)
{
    std::string name;
    std::vector<std::string> splitPath = LibraryMethods::split(path, '/');
    name = splitPath.at(splitPath.size() - 1);

    return 0;
}

