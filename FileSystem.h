#ifndef ZOS_FILESYSTEM_H
#define ZOS_FILESYSTEM_H


#include <string>
#include <array>
#include <fstream>
#include <vector>

const int32_t BYTES_PER_INODE = 1024; // there will be 1 i-node per BYTES_PER_INODE bytes in FS
const int32_t BLOCK_SIZE = 2048;      // data block size in bytes
const int32_t FS_NAME_LENGTH = 12;

struct superblock {
    char name[FS_NAME_LENGTH];     // file system name                                                                // +------------------------+
    int32_t diskSize;              // total FS size in bytes                                                          // |       SUPERBLOCK       |     44 bytes
    int32_t blockSize;             // data block size in bytes                                                        // +------------------------+
    int32_t blockCount;            // data block count                                                                // |     INODE BITMAP       |     1 bit per inode
    int32_t inodeCount;            // number of inodes                                                                // +------------------------+
    int32_t inodeMapStartAddress;  // i-node bitmap start address                                                     // |         INODES         |     40 bytes per inode
    int32_t blockMapStartAddress;  // data block bitmap start address                                                 // +------------------------+
    int32_t inodeStartAddress;     // i-node start address                                                            // |      BLOCK BITMAP      |     1 bit per block
    int32_t blockStartAddress;     // data block start address                                                        // +------------------------+
};                                                                                                                    // |         BLOCKS         |     BLOCK_SIZE per block
                                                                                                                      // +------------------------+

struct inode {
    int32_t nodeid;                 // i-node ID, free i-node has nodeid == ID_ITEM_FREE
    bool isDirectory;               // is a directory?
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
    superblock *sb;                 // superblock structure
    std::fstream fsFile;            // file with the filesystem
    std::string currentPath = "/";  // out current path as a string
    int currentInodeAddress;        // the current inode's address in FS

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

    // createDirectoryItem.cpp
    int createDirectoryItem(std::string path, bool isDirectory, FILE *file = NULL);

    // list.cpp
    int list(int inodeAddress);
    std::vector<directoryItem> getAllDirItemsFromDirect(int blockAddress);

    // removeDirectory.cpp
    int removeDirectory(std::string path);

    // findDirItemInInode.cpp
    int findDirItemInInode(const std::string& name, inode ind);
    int searchDirect(int address, const char *name);

    // addDirItemToInode.cpp
    int addDirItemToInode(char *name, char *extension, int inodeAddress, int inodeReference);
    int addDirItemToDirect(char *name, char *extension, int blockAddress, int inodeReference);

    // removeDirItemFromInode.cpp
    void removeDirItemFromInode(const std::string& name, int inodeAddress);

    // createInode.cpp
    int createInode(int inodeIndex, int blockIndex, int parentInodeAddress, FILE *bytes);
    int fillBlock(int blockAddress, char *bytes, int length);

    // bitmapOperations.cpp
    int toggleBitInBitmap(int bitIndex, int bitmapAddress, int bitmapSize);

    // ln.cpp
    int ln(std::string pathToFile, std::string pathToLink);

    // getInodeAddressForPath.cpp
    int getInodeAddressForPath(std::string path);

    // getFreeInode.cpp
    int getFreeInode();
    int getFreeInodesNum();

    // getFreeBlock.cpp
    int getFreeBlock();
    int getFreeBlocksNum();

    // removeFile.cpp
    int removeFile(std::string path);

    // outcp.cpp
    int outcp(std::string filePath, const std::string& outputPath);

    // cd.cpp
    int cd(std::string path);

    // getAbsolutePathToInode.cpp
    std::string getAbsolutePathToInode(int inodeAddress);

    // info.cpp
    int info(std::string path);

    // cat.cpp
    int cat(std::string path);

    // moveFile.cpp
    int moveFile(std::string filePath, std::string newPath);

    // copyFile.cpp
    int copyFile(std::string sourcePath, std::string destinationPath);
};



#endif //ZOS_FILESYSTEM_H
