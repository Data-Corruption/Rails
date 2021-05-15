#pragma once

#include <string>

constexpr auto info = 0;
constexpr auto warn = 1;
constexpr auto error = 2;

void log(int level, std::string msg);
void clearConsole();