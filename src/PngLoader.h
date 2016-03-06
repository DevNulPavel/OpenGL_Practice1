#ifndef PNG_LOADER
#define PNG_LOADER

#include <stdlib.h>

struct ImageData{
    bool loaded;
    size_t dataSize;
    char* data;
    int width;
    int height;

    // constructor
    ImageData(size_t inDataSize, char* inData, int inWidth, int inHeight);
    // destructor
    ~ImageData();
};


ImageData loadPngImage(const char* fileName);

#endif
