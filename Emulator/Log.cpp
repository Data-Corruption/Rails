#include "Log.h"

#include <Windows.h>
#include <iostream>

void log(int level, std::string msg) {
	std::string i;
	switch (level) {
	case 0:
		std::cout << "INFO: " << msg << std::endl;
		break;
	case 1:
		std::cout << "WARN: " << msg << std::endl;
		break;
	case 2:
		std::cout << "ERROR: " << msg << std::endl;
		exit(1);
		break;
	default:
		std::cout << "LOGGER: log function called with no message level!" << std::endl;
		break;
	}
}

#ifdef _WIN32
void clearConsole() {
	COORD topLeft = { 0, 0 };
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO screen;
	DWORD written;

	GetConsoleScreenBufferInfo(console, &screen);
	FillConsoleOutputCharacterA(
		console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written
	);
	FillConsoleOutputAttribute(
		console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
		screen.dwSize.X * screen.dwSize.Y, topLeft, &written
	);
	SetConsoleCursorPosition(console, topLeft);
}
#else
void clearConsole() {
	std::cout << "\x1B[2J\x1B[H";
}
#endif