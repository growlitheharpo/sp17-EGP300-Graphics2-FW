#include "IToken.h"
#include <iostream>
#include <string>

using namespace std;

void SymbolToken::parseToken(fstream& fin)
{
	fin.seekg(-1, ios_base::cur); //move back 1 character
	char currentChar;

	do
	{
		currentChar = fin.get();
		mSymbol += currentChar;

		currentChar = fin.peek();
	} while (isalnum(currentChar) || currentChar == '_');
}

void SymbolToken::debugPrint() const
{
	cout << "Token: [symbol, \"" << mSymbol << "\"]";
}

void WhitespaceToken::parseToken(std::fstream& fin)
{

}

void WhitespaceToken::debugPrint() const
{
	cout << "Token: [whitespace, \"" << mWhitespace << "\"]";
}

void PunctuationToken::parseToken(std::fstream& fin)
{

}

void PunctuationToken::debugPrint() const
{
	cout << "Token: [punctuation, \"" << mPunctuation << "\"]";
}

void NumberLiteralToken::parseToken(std::fstream& fin)
{

}

void NumberLiteralToken::debugPrint() const
{
	cout << "Token: [number_literal, \"" << mNumber << "\"]";
}

void EOLToken::parseToken(std::fstream& fin)
{

}

void EOLToken::debugPrint() const
{
	cout << "Token: [end_of_line, \"" << "" << "\"]";
}

void EOFToken::parseToken(std::fstream& fin)
{

}

void EOFToken::debugPrint() const
{
	cout << "Token: [end_of_file, \"" << "" << "\"]";
}
