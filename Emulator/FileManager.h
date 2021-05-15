#pragma once

#include <string>

class FileManager {
public:
	static std::string Read(std::string path);
	static void Write(std::string *data, std::string path); // warning overwrites file
	static bool exists(std::string path);
};