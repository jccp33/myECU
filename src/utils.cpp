#include "../include/utils.hpp"
#include <chrono>

void cleanScreen(){
    #ifdef _WIN32
        std::system("cls");
    #else
        std::system("clear");
    #endif
}

uint64_t get_timestamp_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

std::mt19937& get_generator() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return gen;
}

int randomInt(int min, int max) {
    std::uniform_int_distribution<int> distr(min, max);
    return distr(get_generator());
}

float randomFloat(float min, float max) {
    std::uniform_real_distribution<float> distr(min, max);
    return distr(get_generator());
}
