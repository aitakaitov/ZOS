#include <iostream>
#include <vector>
#include "FileSystem.h"
#include "LibraryMethods.h"

int FileSystem::processCommand(std::string command)
{
    std::vector<std::string> splitCommand;
    splitCommand = LibraryMethods::split(command, ' ');

    if (splitCommand.empty())
        return 0;

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
        if (this->createDirectoryItem(splitCommand.at(1), true) == 0)
            std::cout << "OK" << std::endl;
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
    else if (splitCommand.at(0) == "rmdir")
    {
        if (this->removeDirectory(splitCommand.at(1)) == 0)
            std::cout << "OK" << std::endl;
    }
    else if (splitCommand.at(0) == "rm")
    {
        if (this->removeFile(splitCommand.at(1)) == 0)
            std::cout << "OK" << std::endl;
    }
    else if (splitCommand.at(0) == "incp")
    {
        FILE *file = fopen(splitCommand.at(1).c_str(), "r");
        if (file == NULL)
        {
            std::cout << "FILE NOT FOUND" << std::endl;
            return 0;
        }
        if (this->createDirectoryItem(splitCommand.at(2), false, file) == 0)
            std::cout << "OK" << std::endl;
    }
    else if (splitCommand.at(0) == "outcp")
    {
        if (this->outcp(splitCommand.at(1), splitCommand.at(2)) == 0)
            std::cout << "OK" << std::endl;
    }
    else if (splitCommand.at(0) == "freeb")
    {
        std::cout << this->getFreeBlocksNum() << std::endl;
    }
    else if (splitCommand.at(0) == "freei")
    {
        std::cout << this->getFreeInodesNum() << std::endl;
    }
    else if (splitCommand.at(0) == "cd")
    {
        this->cd(splitCommand.at(1));
    }
    else if (splitCommand.at(0) == "pwd")
    {
        std::cout << this->currentPath << std::endl;
    }
    else if (splitCommand.at(0) == "info")
    {
        this->info(splitCommand.at(1));
    }
    else if (splitCommand.at(0) == "cat")
    {
        this->cat(splitCommand.at(1));
    }
    else if (splitCommand.at(0) == "mv")
    {
        if (this->moveFile(splitCommand.at(1), splitCommand.at(2)) == 0)
            std::cout << "OK" << std::endl;
    }
    else if (splitCommand.at(0) == "ln")
    {
        if (this->ln(splitCommand.at(1), splitCommand.at(2)) == 0)
            std::cout << "OK" << std::endl;
    }
    else if (splitCommand.at(0) == "cp")    // TODO implement
    {
        if (this->copyFile(splitCommand.at(1), splitCommand.at(2)) == 0)
            std::cout << "OK" << std::endl;
    }
    else if (splitCommand.at(0) == "load")
    {
        std::ifstream loadFile;
        loadFile.open(splitCommand.at(1));
        std::string loadCommand;

        while (std::getline(loadFile, loadCommand))
            this->processCommand(loadCommand);

        loadFile.close();
    }
    else
        {
            std::cout << "INVALID COMMAND" << std::endl;
        }

    return 0;
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



