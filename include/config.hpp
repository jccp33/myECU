#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "data_types.hpp"

SystemConfig getSystemConfig();
bool isSystemConfigValid(const SystemConfig& config);

#endif
