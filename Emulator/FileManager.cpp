#include "FileManager.h"

#include <sstream>
#include <fstream>
#include <filesystem>
#include <iostream>

#include "Log.h"

std::string FileManager::Read(std::string path) {
	std::stringstream stream;
	std::ifstream file;

	if (!exists(path))
		return "";

	try {
		file.open(path);
		stream << file.rdbuf();
		file.close();
		return stream.str();
	}
	catch (std::ifstream::failure e) {
		std::cout << "ERROR: File not succesfully read! \n\n" << e.what() << "\n\n" << e.code() << "\n\n";
		return "";
	}
}

void FileManager::Write(std::string *data, std::string path) {
	std::ofstream outfile(path);
	outfile << *data << std::endl;
	outfile.close();
}

bool FileManager::exists(std::string path) {
	if (std::filesystem::exists(path))
		return 1;
	return 0;
}