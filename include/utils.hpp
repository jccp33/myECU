#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdlib>
#include <random>

#define TXT_RESET   "\033[0m"
#define TXT_RED     "\033[31m"
#define TXT_GREEN   "\033[32m"
#define TXT_YELLOW  "\033[33m"
#define TXT_BLUE    "\033[34m"
#define TXT_CYAN    "\033[36m"
#define TXT_PURPLE  "\033[35m"

void cleanScreen();

uint64_t get_timestamp_ms();

std::mt19937& get_generator();

int randomInt(int min, int max);

float randomFloat(float min, float max);

bool isNumber(const std::string &str);

#endif
