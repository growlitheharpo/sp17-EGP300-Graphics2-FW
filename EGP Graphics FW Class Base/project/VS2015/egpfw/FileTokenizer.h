#pragma once
#include <vector>
#include <fstream>

class IToken;

class FileTokenizer
{
	private:
		std::vector<IToken*> mTokens;

	public:
		FileTokenizer();
		FileTokenizer(const std::string& filename, bool tokenizeImmediately = true);
		~FileTokenizer() = default;

		void tokenize(const std::string& filename);
		void tokenize(std::fstream& fin);

		std::vector<IToken*>& getTokens() { return mTokens; }
		std::vector<IToken*> const& getTokens() const { return mTokens; }

		void printTokens() const;
};
