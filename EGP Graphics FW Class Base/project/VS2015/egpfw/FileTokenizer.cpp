#include "FileTokenizer.h"
#include <iostream>
#include "IToken.h"

using namespace std;

IToken* FileTokenizer::handleComment(std::fstream& fin)
{
	char peekNext = fin.peek();

	if (peekNext != '/')
		throw invalid_argument("Next item in fstream is not a comment marker!");

	fin.get(); //consume the slash
	peekNext = fin.peek();

	if (peekNext == '/') //one line comment
	{
		while (!EOLToken::characterMatches(peekNext)) //consume up to (but not including) the EOL marker
		{
			fin.get();
			peekNext = fin.peek();
		}

		return new EOLToken();
	}

	if (peekNext == '*') //multi-line comment. fuck fuck fuck
	{
		IToken* newToken = nullptr;

		while (true)
		{
			fin.get(); //consume the next item that was peeked.
			peekNext = fin.peek();

			if (fin.eof())
				throw ios_base::failure("Error parsing file: block comment not terminated before EOF.");

			if (EOLToken::characterMatches(peekNext))
			{ 
				newToken = new EOLToken();
				newToken->parseToken(fin); //let the token consume whatever combination of \n and \r we have.
				mTokens.push_back(newToken);
				
				peekNext = fin.peek();
			}

			if (peekNext == '*')
			{
				fin.get(); //consume the asterix
				peekNext = fin.peek();

				if (peekNext == '/') //we found a */, that means we reached the end!
					return new WhitespaceToken(); //leave the / in the buffer for the whitespace to consume
			}
		}
	}
	
	fin.seekg(-1, ios_base::cur); //move back one. It's not a comment.
	return new PunctuationToken();
}

FileTokenizer::FileTokenizer()
{
}

FileTokenizer::FileTokenizer(const string& filename, bool tokenizeImmediately)
{
}

FileTokenizer::~FileTokenizer()
{
	for (auto iter = mTokens.begin(); iter != mTokens.end(); ++iter)
		delete *iter;

	mTokens.clear();
}

void FileTokenizer::tokenize(const string& filename)
{
	fstream fin;
	fin.open(filename.c_str(), ios_base::in);

	tokenize(fin);

	fin.close();
}

void FileTokenizer::tokenize(fstream& fin)
{
	IToken* newToken;

	while (!fin.eof())
	{
		newToken = nullptr;
		char peekNext = fin.peek();

		if (peekNext == EOF)
			break;

		if (peekNext == '/') //Shit. Check for a comment before we do anything else.
			newToken = handleComment(fin);
		else if (EOLToken::characterMatches(peekNext))
			newToken = new EOLToken();
		else if (NumberLiteralToken::characterMatches(peekNext, fin))
			newToken = new NumberLiteralToken();
		else if (SymbolToken::characterMatches(peekNext))
			newToken = new SymbolToken();
		else if (PunctuationToken::characterMatches(peekNext))
			newToken = new PunctuationToken();
		else if (WhitespaceToken::characterMatches(peekNext))
			newToken = new WhitespaceToken();

		if (newToken == nullptr)
			std::cout << "ERROR: Could not parse symbol: " << peekNext << endl;

		newToken->parseToken(fin);
		mTokens.push_back(newToken);
	}

	mTokens.push_back(new EOFToken());
}

void FileTokenizer::printTokens() const
{
	for (auto& iter : mTokens)
	{
		if (iter->getType() != IToken::WHITESPACE)
		{
			iter->debugPrint();
			cout << "\n";
		}
	}
}
