#pragma once

#include <vector>

class ALU {
public:
	static int add(int a, int b);
	static int addc(int a, int b);
	static int sub(int a, int b);
	static int swb(int a, int b);
private:
	static bool carryFlag;
	static std::vector<bool> binAdder(std::vector<bool> a, std::vector<bool> b, bool carryIn);
	static std::vector<bool> intToVector(int x);
	static int vecToInt(std::vector<bool> x);
};