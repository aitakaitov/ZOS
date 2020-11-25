#ifndef ZOS_LIBRARYMETHODS_H
#define ZOS_LIBRARYMETHODS_H

#include <vector>

class LibraryMethods {
public:
    // LibraryMethods.cpp
    static int parseFSSize(std::string fsSize);
    static char toggleBit(char byte, int pos);
    static int checkBit(char byte, int pos);
    static std::vector<std::string> split(const std::string& str, char delim);
    static bool fileExists(const std::string &path);
    static void parseName(const std::string& name, char *cName, char *cExtension, bool isDirectory);
};


#endif //ZOS_LIBRARYMETHODS_H
