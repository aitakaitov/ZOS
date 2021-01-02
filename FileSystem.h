#ifndef ZOS_FILESYSTEM_H
#define ZOS_FILESYSTEM_H

#include <string>
#include <array>
#include <fstream>
#include <vector>

#define FILENAME_MAX_SIZE 8
#define EXTENSION_MAX_SIZE 3

const int32_t BYTES_PER_INODE = 8192; // there will be 1 inode per BYTES_PER_INODE bytes in FS
const int32_t BLOCK_SIZE = 2048;      // data block size in bytes
const int32_t FS_NAME_LENGTH = 12;

struct superblock {
    char name[FS_NAME_LENGTH];     // file system name                                                               // +------------------------+
    int32_t diskSize;              // total FS size in bytes                                                         // |       SUPERBLOCK       |     44 bytes
    int32_t blockSize;             // data block size in bytes                                                       // +------------------------+
    int32_t blockCount;            // data block count                                                               // |     INODE BITMAP       |     1 bit per inode
    int32_t inodeCount;            // number of inodes                                                               // +------------------------+
    int32_t inodeMapStartAddress;  // inode bitmap start address                                                     // |         INODES         |     40 bytes per inode
    int32_t blockMapStartAddress;  // data block bitmap start address                                                // +------------------------+
    int32_t inodeStartAddress;     // inode start address                                                            // |      BLOCK BITMAP      |     1 bit per block
    int32_t blockStartAddress;     // data block start address                                                       // +------------------------+
};                                                                                                                   // |         BLOCKS         |     BLOCK_SIZE per block
                                                                                                                     // +------------------------+

struct inode {
    int32_t nodeid;                 // inode ID
    bool isDirectory;               // is it a directory?
    int8_t references;              // number of references to the inode
    int32_t fileSize;               // file size in bytes
    int32_t direct1;                // direct references to data blocks
    int32_t direct3;
    int32_t direct2;
    int32_t direct4;
    int32_t direct5;
    int32_t indirect1;              // 1.st order indirect reference to data block ( this -> data block)
    int32_t indirect2;              // 2.nd order indirect reference to data block ( this -> data block -> data block)
};


struct directoryItem {
    int32_t inode;                  // i-node reference
    char itemName[FILENAME_MAX_SIZE + EXTENSION_MAX_SIZE + 1];  // 8 bytes name, 3 bytes extension, 1 byte \0
};

class FileSystem {
public:
    superblock *sb;                 // Superblock loaded into memory
    std::fstream fsFile;            // File with the FS
    std::string currentPath = "/";  // We always start in root directory
    int currentInodeAddress;        // Address of inode we are currently in

    // fsIO.cpp
    int writeToFS(char *bytes, int length, int32_t address);    // Writes to FS
    int readFromFS(char *bytes, int length, int32_t address);   // Reads from FS

    // fsInit.cpp
    int createRoot();                                       // Creates root directory when the FS is created
    int loadFileSystem(std::string path);                   // Loads the file system if it exists
    int createFileSystem(std::string path, int sizeBytes);  // Creates and loads the file system if it does not exist yet

    // cli.cpp
    int commandLineLoop();                      // Infinite command loop
    int processCommand(std::string command);    // Processes commands

    // createFileOrDir.cpp
    int createFileOrDir(std::string path, bool isDirectory, FILE *file = NULL); // Creates a file or a directory

    // list.cpp
    std::vector<directoryItem> getAllDirItemsFromDirect(int blockAddress);  // Fetches all directoryItems from a directly referenced block
    int list(int inodeAddress);                                             // Lists items in a directory

    // removeDirectory.cpp
    int removeDirectory(std::string path);  // removes a directory

    // findDirITemInInode.cpp
    int findDirItemInInode(const std::string& name, inode ind); // returns an address of a directory item in an inode
    int searchDirect(int address, const char *name);    // searches a direct block for directoryItem

    // addDirItemToInode.cpp
    int addDirItemToInode(char *name, char *extension, int inodeAddress, int inodeReference);   // Adds a directoryItem to an inode
    int addDirItemToDirect(char *name, char *extension, int blockAddress, int inodeReference);  // Finds an empty place in a direct block and places a directoryItem in it

    // removeDirItemFromInode.cpp
    void removeDirItemFromInode(const std::string& name, int inodeAddress); // Removes a directoryItem from inode

    // createInode.cpp
    int createInode(int inodeIndex, int blockIndex, int parentInodeAddress, FILE *bytes, bool isDirectory);      // Creates an inode, be it file or directory
    int fillBlock(int blockAddress, char *bytes, int length);                                                    // Fills a block with data from an external file
    int getBlocksNecessary(int sizeBytes);                                                                       // Calculates number of blocks necessary to store a file

    // bitmapOperations.cpp
    int toggleBitInBitmap(int bitIndex, int bitmapAddress, int bitmapSize);

    // ln.cpp
    int ln(std::string pathToFile, std::string pathToLink); // creates a hardlink

    // getInodeAddressForPath.cpp
    int getInodeAddressForPath(std::string path); // returns address of an inode specified by a path

    // getFreeInode.cpp
    int getFreeInode(); // returns an index of a free inode
    int getFreeInodesNum(); // returns count of free inodes

    // getFreeBlock.cpp
    int getFreeBlock(); // returns an index of a free block
    int getFreeBlocksNum(); // returns count of free blocks

    // removeFile.cpp
    int removeFile(std::string path);   // removes a file, check for number of references
    int fillDirectWithDI(int blockAddress, std::vector<directoryItem> &dirItems); // Fills a direct block with directoryItems
    int defragmentDirects(inode &ind);      // Defragments the direct blocks of an directory inode

    // outcp.cpp
    int outcp(std::string filePath, const std::string& outputPath); // Copies a file to an external location

    // cd.cpp
    int cd(std::string path);   // changes working directory

    // getAbsolutePathToInode.cpp
    std::string getAbsolutePathToInode(int inodeAddress);   // returns an absolute path to an inode, if the inode exists

    // info.cpp
    int info(std::string path); // prints out info about a file/dir

    // cat.cpp
    int cat(std::string path);  // Write out a file

    // moveFile.cpp
    int moveFile(std::string filePath, std::string newPath);    // moves a file

    // copyFile.cpp
    int copyFile(std::string sourcePath, std::string destinationPath);  // copies a file
};



#endif //ZOS_FILESYSTEM_H
