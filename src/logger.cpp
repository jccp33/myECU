#include "logger.hpp"

Logger::Logger() : file(0) {}

Logger::~Logger(){
    close();
}

bool Logger::open(const char *filename){
    if(file != 0) return false;
    file = std::fopen(filename, "a");
    return file != 0;
}

void Logger::write(const char *message){
    if(file == 0 || message == 0) return;
    std::fprintf(file, "%s\n", message);
    std::fflush(file);
}

void Logger::close(){
    if(file == 0) return;
    std::fclose(file);
    file = 0;
}
