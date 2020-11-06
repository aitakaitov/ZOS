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

    if (splitCommand.at(0) == "format")                                                         // FORMAT
    {
        int sizeBytes = LibraryMethods::parseFSSize(splitCommand.at(1));
        if (this->createFileSystem(this->sb->name, sizeBytes) == 0)
            std::cout << "OK" << std::endl;
        else
            return -1;
    }
    else if (splitCommand.at(0) == "exit")                                                      // EXIT
    {
        return -1;
    }
    else if (splitCommand.at(0) == "mkdir")                                                     // MKDIR
    {
        if (this->createDirectoryItem(splitCommand.at(1), true) == 0)
            std::cout << "OK" << std::endl;
    }
    else if (splitCommand.at(0) == "ls")                                                        // LS
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
    else if (splitCommand.at(0) == "rmdir")                                                     // RMDIR
    {
        if (this->removeDirectory(splitCommand.at(1)) == 0)
            std::cout << "OK" << std::endl;
    }
    else if (splitCommand.at(0) == "rm")                                                        // RM
    {
        if (this->removeFile(splitCommand.at(1)) == 0)
            std::cout << "OK" << std::endl;
    }
    else if (splitCommand.at(0) == "incp")                                                      // INCP
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
    else if (splitCommand.at(0) == "outcp")                                                     // OUTCP
    {
        if (this->outcp(splitCommand.at(1), splitCommand.at(2)) == 0)
            std::cout << "OK" << std::endl;
    }
    else if (splitCommand.at(0) == "freeb")                                                     // FREEB (get free blocks count)
    {
        std::cout << this->getFreeBlocksNum() << std::endl;
    }
    else if (splitCommand.at(0) == "freei")                                                     // FREEI (get fre inodes count)
    {
        std::cout << this->getFreeInodesNum() << std::endl;
    }
    else if (splitCommand.at(0) == "cd")                                                        // CD
    {
        this->cd(splitCommand.at(1));
    }
    else if (splitCommand.at(0) == "pwd")                                                       // PWD
    {
        std::cout << this->currentPath << std::endl;
    }
    else if (splitCommand.at(0) == "info")                                                      // INFO
    {
        this->info(splitCommand.at(1));
    }
    else if (splitCommand.at(0) == "cat")                                                       // CAT
    {
        this->cat(splitCommand.at(1));
    }
    else if (splitCommand.at(0) == "mv")                                                        // MV
    {
        if (this->moveFile(splitCommand.at(1), splitCommand.at(2)) == 0)
            std::cout << "OK" << std::endl;
    }
    else if (splitCommand.at(0) == "ln")                                                        // LN
    {
        if (this->ln(splitCommand.at(1), splitCommand.at(2)) == 0)
            std::cout << "OK" << std::endl;
    }
    else if (splitCommand.at(0) == "cp")                                                        // CP
    {
        if (this->copyFile(splitCommand.at(1), splitCommand.at(2)) == 0)
            std::cout << "OK" << std::endl;
    }
    else if (splitCommand.at(0) == "load")                                                      // LOAD
    {
        std::ifstream loadFile;
        loadFile.open(splitCommand.at(1));

        if (loadFile.fail())
        {
            std::cout << "FILE NOT FOUND" << std::endl;
            return 0;
        }

        std::string loadCommand;
        while (std::getline(loadFile, loadCommand))
            this->processCommand(loadCommand);

        loadFile.close();
        std::cout << "OK" << std::endl;
    }
    else                                                                                            // DEFAULT
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



