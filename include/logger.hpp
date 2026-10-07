#ifndef LOGGER_HPP
#define LOGGER_HPP
#include <cstdio>

class Logger{
    private:
        FILE* file;
    public:
        Logger();
        ~Logger();
        bool open(const char* filename);
        void write(const char* message);
        void close();
};

#endif
