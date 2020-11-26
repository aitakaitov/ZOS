#include <iostream>
#include <vector>
#include "FileSystem.h"
#include "LibraryMethods.h"

// Processes a shell command, calls other methods
// -1    = exit
// 0     = otherwise
int FileSystem::processCommand(std::string command)
{
    std::vector<std::string> splitCommand;
    splitCommand = LibraryMethods::split(command, ' ');

    if (splitCommand.empty())
        return 0;

    if (splitCommand.at(0) == "format")
    {
        int sizeBytes = LibraryMethods::parseFSSize(splitCommand.at(1));
        if (this->createFileSystem(this->sb->name, sizeBytes) == 0)
            std::cout << "OK" << std::endl;
        else
            return -1;
    }
    else if (splitCommand.at(0) == "exit")
    {
        return -1;
    }
    else if (splitCommand.at(0) == "mkdir")
    {
        if (this->createFileOrDir(splitCommand.at(1), true) == 0)
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
                if (inodeAddress == -1)
                {
                    std::cout << "PATH NOT FOUND" << std::endl;
                    return 0;
                }
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
        if (this->createFileOrDir(splitCommand.at(2), false, file) == 0)
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
        if (splitCommand.size() > 1)
            this->info(splitCommand.at(1));
        else
            this->info("");
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
    else if (splitCommand.at(0) == "cp")
    {
        if (this->copyFile(splitCommand.at(1), splitCommand.at(2)) == 0)
            std::cout << "OK" << std::endl;
    }
    else if (splitCommand.at(0) == "load")
    {
        std::ifstream loadFile;
        loadFile.open(splitCommand.at(1));

        if (loadFile.fail())
        {
            std::cout << "FILE NOT FOUND" << std::endl;
            return 1;
        }

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

// The main shell loop. Takes commands from CLI and calls the method, processing them.
// 0    = exit
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



