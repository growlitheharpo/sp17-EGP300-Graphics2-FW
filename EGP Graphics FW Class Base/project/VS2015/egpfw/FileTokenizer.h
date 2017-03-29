#pragma once
#include <vector>
#include <fstream>

class IToken;

class FileTokenizer
{
	private:
		std::vector<IToken*> mTokens;
		IToken* handleComment(std::fstream& fin);

	public:
		FileTokenizer();
		FileTokenizer(const std::string& filename, bool tokenizeImmediately = true);
		~FileTokenizer();

		void tokenize(const std::string& filename);
		void tokenize(std::fstream& fin);

		std::vector<IToken*>& getTokens() { return mTokens; }
		std::vector<IToken*> const& getTokens() const { return mTokens; }

		void printTokens() const;
};
