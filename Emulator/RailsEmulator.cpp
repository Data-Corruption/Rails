#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <X11/Xlib.h>
#include "X11/keysym.h"
#endif

#include <unordered_map>
#include <sstream>
#include <string>
#include <vector>

#include "FileManager.h"
#include "ALU.h"
#include "Log.h"

struct program {
	std::unordered_map<std::string, int> lineTags;
	std::vector<std::vector<int>> instructions;
};

struct virtualMachine {
	bool carry = false;
	std::vector<int> registers;
	std::vector<int> in_out_registers;
	std::vector<int> ram;
};

const char* art =
"    ___       ___       ___       ___       ___   \n"
"   /\\  \\     /\\  \\     /\\  \\     /\\__\\     /\\  \\  \n"
"  /::\\  \\   /::\\  \\   _\\:\\  \\   /:/  /    /::\\  \\ \n"
" /::\\:\\__\\ /::\\:\\__\\ /\\/::\\__\\ /:/__/    /\\:\\:\\__\\\n"
" \\;:::/  / \\/\\::/  / \\::/\\/__/ \\:\\  \\    \\:\\:\\/__/\n"
"  |:\\/__/    /:/  /   \\:\\__\\    \\:\\__\\    \\::/  / \n"
"   \\|__|     \\/__/     \\/__/     \\/__/     \\/__/  \n";

std::unordered_map<std::string, int> opCodes({
	{"NULL", 20},
	{"ADD",  0},
	{"ADDC", 1},
	{"SUB",  2},
	{"SWB",  3},
	{"NAND", 4},
	{"RSFT", 5},
	{"LIMM", 6},
	{"LD",   7},
	{"LDIM", 8},
	{"ST",   9},
	{"STIM", 10},
	{"BEQ",  11},
	{"BGT",  12},
	{"JMPL", 13},
	{"IN",   14},
	{"OUT",  15},
	// Pseudo Instructions
	{"NOP",  0},
	{"MOV",  0},
	{"JMP",  11},
	{"EXIT", 13}
	});

std::vector<int> threeOPs, twoOPs, immOPs;
int linenumber = 0;

program currentProgram;
virtualMachine vm;

bool containsInt(std::vector<int> haystack, int needle) {
	if (std::find(haystack.begin(), haystack.end(), needle) != haystack.end())
		return true;
	return false;
}

bool containsString(std::string haystack, std::string needle) {
	if (haystack.find(needle) != std::string::npos)
		return true;
	return false;
}

void cleanVector(std::vector<std::string>* result) {
	auto it = result->begin();
	while (it != result->end()) {
		if ((*it == "") || (*it == "\t")) {
			it = result->erase(it);
		} else {
			++it;
		}
	}
}

void splitString(std::vector<std::string>* result, std::string input, char target) {
	result->clear();
	std::stringstream ss(input);
	std::string segment;

	while (std::getline(ss, segment, target))
		result->push_back(segment);
}

int parseReg(std::string s) {
	std::erase(s, 'r');
	if (stoi(s) > 15)
		log(error, "register index out of range! Line number: " + std::to_string(linenumber));
	return stoi(s);
}

int parseImm(std::string s) {
	if (containsString(s, ":"))
		return currentProgram.lineTags[s];
	if (stoi(s) > 255)
		log(error, "immediate out of range! Line number: " + std::to_string(linenumber));
	return stoi(s);
}

int loadAssembly(std::string path) {
	std::string sourceFile = FileManager::Read(path);

	// split by line
	std::vector<std::string> lines;
	splitString(&lines, sourceFile, '\n');

	linenumber = 0;

	// load tags
	for (auto s : lines) {
		if (containsString(s, "#"))
			continue;
		std::vector<std::string> args;
		splitString(&args, s, ' ');
		cleanVector(&args);
		if (containsString(args[0], ":"))
			currentProgram.lineTags[args[0]] = linenumber;
		linenumber++;
	}

	// loop through lines / load data to program
	linenumber = 0;
	for (auto s : lines) {
		std::vector<int> cinst;
		int i = 0;
		if (containsString(s, "#"))
			continue;
		std::vector<std::string> args;
		splitString(&args, s, ' ');
		cleanVector(&args);

		if (containsString(args[i], ":"))
			i++;

		std::string opCode = args[i];
		cinst.push_back(opCodes[opCode]);

		if (opCode == "NOP") {
			cinst.push_back(0);
			cinst.push_back(0);
			cinst.push_back(0);
			opCode = "NULL";
		}
		if (opCode == "MOV") {
			cinst.push_back(0);
			cinst.push_back(parseReg(args[i + 1]));
			cinst.push_back(parseReg(args[i + 2]));
			opCode = "NULL";
		}
		if (opCode == "JMP") {
			cinst.push_back(parseImm(args[i + 1]));
			cinst.push_back(15);
			opCode = "NULL";
		}
		if (opCode == "EXIT") {
			cinst.push_back(0);
			cinst.push_back(0);
			opCode = "NULL";
		}

		if (containsInt(threeOPs, opCodes[opCode])) {
			cinst.push_back(parseReg(args[i + 1]));
			cinst.push_back(parseReg(args[i + 2]));
			cinst.push_back(parseReg(args[i + 3]));
		}
		if (containsInt(twoOPs, opCodes[opCode])) {
			cinst.push_back(parseReg(args[i + 1]));
			cinst.push_back(parseReg(args[i + 2]));
		}
		if (containsInt(immOPs, opCodes[opCode])) {
			cinst.push_back(parseImm(args[i + 1]));
			cinst.push_back(parseReg(args[i + 2]));
		}

		currentProgram.instructions.push_back(cinst);
		linenumber++;
	}
	return true;
}

void printInOutRegisters() {
	int i = 0;
	std::cout << art << std::endl;
	std::vector<std::string> out;
	out.resize(16);

	// very gross, this just formats the output to look nice
	for (auto r : vm.in_out_registers) {
		out[i] = "R:";
		out[i] += std::to_string(i);
		if (i >= 10) {
			out[i] += " ";
		} else {
			out[i] += "  ";
		}
		out[i] += std::to_string(r);
		if (r >= 100) {
			out[i] += "  ";
		} else {
			if (r >= 10) {
				out[i] += "   ";
			}
			else {
				out[i] += "    ";
			}
		}
		i++;
	}
	for (int x = 0; x < 8; x++) {
		std::cout << out[x] << out[x + 8] << std::endl;
	}
}

#ifdef _WIN32
bool exitKeyPressed() {
	if ((GetAsyncKeyState('Q') & 0x8000) & (GetAsyncKeyState(VK_CONTROL) & 0x8000))
		return true;
	return false;
}
#else
bool key_is_pressed(KeySym ks) {
	Display* dpy = XOpenDisplay(":0");
	char keys_return[32];
	XQueryKeymap(dpy, keys_return);
	KeyCode kc2 = XKeysymToKeycode(dpy, ks);
	bool isPressed = !!(keys_return[kc2 >> 3] & (1 << (kc2 & 7)));
	XCloseDisplay(dpy);
	return isPressed;
}

bool exitKeyPressed() {
	return key_is_pressed(XK_q) & (key_is_pressed(XK_Control_L) || key_is_pressed(XK_Control_R));
}
#endif

void emulateProgram() {
	int pc = 0;
	int temp = 0;
	while (true) {
		if (exitKeyPressed())
			return;

		if (pc + 1 > currentProgram.instructions.size())
			break;
		std::vector<int> i = currentProgram.instructions[pc];
		switch (i[0])
		{
		case 0: // add
			vm.registers[i[3]] = ALU::add(vm.registers[i[1]], vm.registers[i[2]]);
			break;
		case 1: // add with carry
			vm.registers[i[3]] = ALU::addc(vm.registers[i[1]], vm.registers[i[2]]);
			break;
		case 2: // sub
			vm.registers[i[3]] = ALU::sub(vm.registers[i[1]], vm.registers[i[2]]);
			break;
		case 3: // sub with borrow?
			vm.registers[i[3]] = ALU::swb(vm.registers[i[1]], vm.registers[i[2]]);
			break;
		case 4: // nand
			vm.registers[i[3]] = ~(vm.registers[i[1]] & vm.registers[i[2]]);
			break;
		case 5: // right shift
			vm.registers[i[2]] = vm.registers[i[1]] >> 1;
			break;
		case 6: // Load immediate
			vm.registers[i[2]] = i[1];
			break;
		case 7: // Load from ram
			if (vm.registers[i[1]] > 255)
				log(error, "ram index out of range! Line number: " + std::to_string(pc));
			vm.registers[i[2]] = vm.ram[vm.registers[i[1]]];
			break;
		case 8: // Load from ram using imm
			if (i[1] > 255)
				log(error, "ram index out of range! Line number: " + std::to_string(pc));
			vm.registers[i[2]] = vm.ram[i[1]];
			break;
		case 9: // Stores to ram "A B"
			if (vm.registers[i[1]] > 255)
				log(error, "ram index out of range! Line number: " + std::to_string(pc));
			vm.ram[vm.registers[i[1]]] = vm.registers[i[2]];
			break;
		case 10: // Stores to ram using imm
			if (i[1] > 255)
				log(error, "ram index out of range! Line number: " + std::to_string(pc));
			vm.ram[i[1]] = vm.registers[i[2]];
			break;
		case 11: // Branch if equal
			if (vm.registers[i[2]] == vm.registers[15]) {
				pc = i[1];
				continue;
			}
			break;
		case 12: // Branch if greater than
			if (vm.registers[i[2]] > vm.registers[15]) {
				pc = i[1];
				continue;
			}
			break;
		case 13: // Jump and link / EXIT
			if ((i[1] == 0) & (i[2] == 0)) {
				log(0, "Program has finished executing!");
				return;
			}
			temp = pc;
			pc = vm.registers[i[1]];
			vm.registers[i[2]] = temp;
			break;
		case 14: // In
			vm.registers[i[2]] = vm.in_out_registers[i[1]];
			break;
		case 15: // Out
			vm.in_out_registers[i[1]] = vm.registers[i[2]];
			break;
		default:
			break;
		}
		vm.registers[0] = 0;
		pc++;
	}
}

int main(int argc, char** argv) {
	clearConsole();
	if (argc == 1)
		log(error, "missing command line arguments");

	threeOPs = { 0, 1, 2, 3, 4 };
	twoOPs = { 5, 7, 13, 14, 9, 15 };
	immOPs = { 6, 8, 10, 11, 12 };

	vm.in_out_registers.resize(16);
	vm.registers.resize(16);
	vm.ram.resize(256);

	// parse i/o set arguments
	if (argc >= 3) {
		for (int i = 2; i < argc; i++) {
			std::vector<std::string> _regSetArg;
			splitString(&_regSetArg, argv[i], '=');
			if (stoi(_regSetArg[1]) > 255)
				log(error, "i/o set register command line argument out of range!");
			vm.in_out_registers[parseReg(_regSetArg[0])] = stoi(_regSetArg[1]);
		}
	}

	std::string path = argv[1];

	if (!FileManager::exists(path))
		log(error, "File " + path + " does not exist or is empty!\n");

	loadAssembly(path);
	emulateProgram();
	clearConsole();
	printInOutRegisters();
}