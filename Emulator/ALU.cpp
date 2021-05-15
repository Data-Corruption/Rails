#include "ALU.h"

#include <cmath>

bool ALU::carryFlag = false;

int ALU::add(int a, int b) {
	return vecToInt(binAdder(intToVector(a), intToVector(b), 0));
}

int ALU::addc(int a, int b) {
	return vecToInt(binAdder(intToVector(a), intToVector(b), carryFlag));
}

int ALU::sub(int a, int b) {
	std::vector<bool> _b = intToVector(b);
	for (auto b : _b) {
		b = !b;
	}
	return vecToInt(binAdder(intToVector(a), _b, 1));
}

int ALU::swb(int a, int b) {
	std::vector<bool> _b = intToVector(b);
	for (auto b : _b) {
		b = !b;
	}
	return vecToInt(binAdder(intToVector(a), _b, !carryFlag));
}

std::vector<bool> ALU::binAdder(std::vector<bool> a, std::vector<bool> b, bool carryIn) {
	std::vector<bool> result;
	bool c = carryIn;
	bool p, g = 0;
	for (int i = 0; i <= 7; i++) {
		p = a[i] ^ b[i];
		g = a[i] & b[i];
		result.push_back(p ^ c);
		c = g || (p & c);
	}
	carryFlag = c;
	return result;
}

std::vector<bool> ALU::intToVector(int x) {
	std::vector<bool> result;
	for (int i = 7; i >= 0; i--) {
		int p = pow(2, i);
		if (x >= p) {
			x -= p;
			result.push_back(true);
		} else {
			result.push_back(false);
		}
	}
	std::reverse(result.begin(), result.end());
	return result;
}

int ALU::vecToInt(std::vector<bool> x) {
	int result = 0;
	for (int i = 0; i <= 7; i++) {
		if (x[i])
			result += pow(2, i);
	}
	return result;
}