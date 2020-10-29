#include <iostream>
#include <sstream>
#include <vector>
#include "FileSystem.h"
#include "LibraryMethods.h"

int FileSystem::processCommand(std::string command)
{
    std::vector<std::string> splitCommand;
    splitCommand = LibraryMethods::split(command, ' ');

    if (splitCommand.at(0) == "format")
    {
        int sizeBytes = LibraryMethods::parseFSSize(splitCommand.at(1));
        this->createFileSystem(this->sb->name, sizeBytes);
    }
    else if (splitCommand.at(0) == "exit")
    {
        return -1;
    }
    else if (splitCommand.at(0) == "mkdir")
    {
        this->createDirectoryItem(splitCommand.at(1), true);
    }
    else if (splitCommand.at(0) == "ls")
    {
        if (splitCommand.size() == 1)
        {
            this->list(this->currentInodeAddress);
        }
        else
            {
                int inodeAddress = this->getInodeAddressForPath(splitCommand.at(1));
                this->list(inodeAddress);
            }
    }
}

int FileSystem::commandLineLoop() {
    this->currentInodeAddress = this->sb->inodeStartAddress;
    this->currentPath = "/";

    std::string command;
    while (true)
    {
        std::cout << this->currentPath << " > ";
        getline(std::cin, command);
        int res = processCommand(command);
        if (res == -1)
            return 0;
    }
}



