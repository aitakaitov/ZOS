#include <iostream>
#include "FileSystem.h"
#include "LibraryMethods.h"

// Main file of the application
int main(int argc, char** argv) {

    if (argc != 2)
    {
        std::cout << "Invalid argument count\nExiting" << std::endl;
        return 1;
    }

    std::string fsName = argv[1];

    if (LibraryMethods::fileExists(fsName))
    {
        std::cout << "File exists, attempting to load file system" << std::endl;
        FileSystem fs;
        if (fs.loadFileSystem(fsName) != 0)
            return 1;

        fs.commandLineLoop();
        fs.fsFile.close();
    }
    else
        {
            std::string formatCommand;
            getline(std::cin, formatCommand);

            if (formatCommand.substr(0, 6) == "format" && LibraryMethods::parseFSSize(formatCommand.substr(7)) > 1)
            {
                int fsSizeBytes = LibraryMethods::parseFSSize(formatCommand.substr(7));
                FileSystem fs;
                if (fs.createFileSystem(fsName, fsSizeBytes) != 0)
                    return 1;

                std::cout << "OK" << std::endl;
                fs.commandLineLoop();
                fs.fsFile.close();
            }
        }

    return 0;
}