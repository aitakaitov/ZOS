#ifndef ZOS_FILESYSTEM_H
#define ZOS_FILESYSTEM_H


#include <string>
#include <array>
#include <fstream>
#include <vector>

const int32_t ID_ITEM_FREE = 0;     // i-node is free if it has this as inodeid

const int32_t BYTES_PER_INODE = 1024; // there will be 1 i-node per BYTES_PER_INODE bytes in FS
const int32_t BLOCK_SIZE = 2048;      // data block size in bytes
const int32_t FS_NAME_LENGTH = 12;

struct superblock {
    char name[FS_NAME_LENGTH];     // file system name                                                                // +------------------------+
    int32_t diskSize;              // total FS size in bytes                                                          // |       SUPERBLOCK       |     44 bytes
    int32_t blockSize;             // data block size in bytes                                                        // +------------------------+
    int32_t blockCount;            // data block count                                                                // |     INODE BITMAP       |     1 bit per inode
    int32_t inodeCount;                                                                                               // +------------------------+
    int32_t inodeMapStartAddress;  // i-node bitmap start address                                                     // |         INODES         |     40 bytes per inode
    int32_t blockMapStartAddress;  // data block bitmap start address                                                 // +------------------------+
    int32_t inodeStartAddress;     // i-node start address                                                            // |      BLOCK BITMAP      |     1 bit per block
    int32_t blockStartAddress;     // data block start address                                                        // +------------------------+
};                                                                                                                    // |         BLOCKS         |     BLOCK_SIZE per block
                                                                                                                      // +------------------------+

struct inode {
    int32_t nodeid;                 // i-node ID, free i-node has nodeid == ID_ITEM_FREE
    bool isDirectory;
    int8_t references;              // number of references to the i-node
    int32_t fileSize;               // file size in bytes
    int32_t direct1;                // direct references to data blocks
    int32_t direct3;
    int32_t direct2;
    int32_t direct4;
    int32_t direct5;
    int32_t indirect1;              // 1.st order indirect reference to data block ( this -> data block)
    int32_t indirect2;              // 2.nd order indirect reference to data block ( this -> data block -> datablock)
};


struct directoryItem {
    int32_t inode;                  // i-node reference
    char itemName[12];              // 8 bytes name, 3 bytes extension, 1 byte \0
};

class FileSystem {
public:
    superblock *sb;
    std::fstream fsFile;
    std::string currentPath = "/";
    int currentInodeAddress;

    // fsIO.cpp
    int writeToFS(char *bytes, int length, int32_t address);
    int readFromFS(char *bytes, int length, int32_t address);

    // fsInit.cpp
    int createRoot();
    int loadFileSystem(std::string path);
    int createFileSystem(std::string path, int sizeBytes);

    // cli.cpp
    int commandLineLoop();
    int processCommand(std::string command);


    int createDirectoryItem(std::string path, bool isDirectory, FILE *file = NULL);
    std::vector<directoryItem> getAllDirItemsFromDirect(int blockAddress);
    int removeDirectory(std::string path);
    int findDirItemInInode(const std::string& name, inode ind);
    int addDirItemToInode(char *name, char *extension, int inodeAddress, int inodeReference);
    int addDirItemToDirect(char *name, char *extension, int blockAddress, int inodeReference);
    void removeDirItemFromInode(const std::string& name, int inodeAddress);

    int createInode(int inodeIndex, int blockIndex, int parentInodeAddress, FILE *bytes);
    int fillBlock(int blockAddress, char *bytes, int length);

    // bitmapOperations.cpp
    int toggleBitInBitmap(int bitIndex, int bitmapAddress, int bitmapSize);

    int list(int inodeAddress);

    int ln(std::string pathToFile, std::string pathToLink);

    int getInodeAddressForPath(std::string path);

    int searchDirect(int address, const char *name);
    int getFreeInode();
    int getFreeBlock();
    int getFreeBlocksNum();
    int getFreeInodesNum();

    int removeFile(std::string path);

    int outcp(std::string filePath, const std::string& outputPath);

    int cd(std::string path);

    std::string getAbsolutePathToInode(int inodeAddress);

    int info(std::string path);

    int cat(std::string path);

    int moveFile(std::string filePath, std::string newPath);

    int copyFile(std::string sourcePath, std::string destinationPath);
};



#endif //ZOS_FILESYSTEM_H
