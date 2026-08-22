#include "../include/utils.hpp"
#include <chrono>
#include <sstream>

// functions

void cleanScreen(){
    #ifdef _WIN32
        std::system("cls");
    #else
        std::system("clear");
    #endif
}

uint64_t get_timestamp_ms() {
    const std::chrono::milliseconds elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
        );
    const std::chrono::milliseconds::rep count = elapsed.count();
    if (count < 0) {
        return 0U;
    }
    return static_cast<uint64_t>(count);
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

bool isNumber(const std::string &str) {
    if (str.empty()) return false;
    std::stringstream iss(str);
    double valor;
    iss >> valor;
    // Verifica si extrajo todo el string y no hubo errores
    return iss.eof() && !iss.fail();
}
