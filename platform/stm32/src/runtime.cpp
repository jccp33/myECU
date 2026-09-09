#include <cstddef>

extern "C" void *memset(void *destination, int value, std::size_t count){
    unsigned char *dest = static_cast<unsigned char *>(destination);
    const unsigned char byteValue = static_cast<unsigned char>(value);
    for(std::size_t i=0U; i<count; ++i){
        dest[i] = byteValue;
    }
    return destination;
}
