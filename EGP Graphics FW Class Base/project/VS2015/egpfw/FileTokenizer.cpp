#include "FileTokenizer.h"
#include <iostream>

FileTokenizer::FileTokenizer()
{
}

FileTokenizer::FileTokenizer(const std::string& filename, bool tokenizeImmediately)
{
}

void FileTokenizer::tokenize(const std::string& filename)
{
	std::fstream fin;
	fin.open(filename.c_str(), std::ios_base::in);

	fin.ignore(std::numeric_limits<std::streamsize>::max());
	std::streamsize size = fin.gcount();

	fin.clear();
	fin.seekg(0, std::ios_base::beg);

	char* buffer = new char[size];
	fin.read(buffer, size);

	std::cout << buffer;

	while (std::cin.get() != ' ') {}
}

void FileTokenizer::tokenize(std::fstream& fin)
{

}

void FileTokenizer::printTokens() const
{

}
