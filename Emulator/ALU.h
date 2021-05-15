#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

class ALU {
public:
	static uint8_t add(int a, int b);
	static uint8_t addc(int a, int b);
	static uint8_t sub(int a, int b);
	static uint8_t swb(int a, int b);
private:
	static bool carryFlag;
	static std::vector<bool> binAdder(std::vector<bool> a, std::vector<bool> b, bool carryIn);
	static std::vector<bool> intToVector(int x);
	static int vecToInt(std::vector<bool> x);
};