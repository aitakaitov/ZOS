#ifndef ZOS_LIBRARYMETHODS_H
#define ZOS_LIBRARYMETHODS_H

#include <vector>

class LibraryMethods {
public:
    static int parseFSSize(std::string fsSize);
    static char toggleBit(char byte, int pos);
    static int checkBit(char byte, int pos);
    static std::vector<std::string> split(std::string str, char delim);
    static bool fileExists(const std::string &path);
};


#endif //ZOS_LIBRARYMETHODS_H
