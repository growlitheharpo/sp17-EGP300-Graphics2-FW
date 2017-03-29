#include "IToken.h"
#include <iostream>
#include <string>

using namespace std;

void SymbolToken::parseToken(fstream& fin)
{
	mSymbol = "";
	char currentChar;
	
	do {
		currentChar = fin.get();
		mSymbol += currentChar;

		currentChar = fin.peek();
	} while (characterMatches(currentChar));
}

void SymbolToken::debugPrint() const
{
	cout << "Token: [symbol, \"" << mSymbol << "\"]";
}

bool SymbolToken::characterMatches(char c)
{
	return isalnum(c) || c == '_';
}

void WhitespaceToken::parseToken(std::fstream& fin)
{
	mWhitespace = "";
	char currentChar;

	do {
		currentChar = fin.get();
		mWhitespace += currentChar;

		currentChar = fin.peek();
	} while (characterMatches(currentChar));
}

void WhitespaceToken::debugPrint() const
{
	cout << "Token: [whitespace, \"" << mWhitespace << "\"]";
}

bool WhitespaceToken::characterMatches(char c)
{
	return isspace(c);
}

void PunctuationToken::parseToken(std::fstream& fin)
{
	char currentChar = fin.get();
	mPunctuation = currentChar;

	switch (currentChar)
	{
		case '%': // look for % or %=
		case '*': // look for * or *=
		case '/': // look for / or /=
		case '!': // look for ! or !=
		case '<': // look for < or <=
		case '=': // look for = or ==
		case '>': // look for > or >=
			if (fin.peek() == '=')
				mPunctuation += fin.get();
			return;
		case '&':
			if (fin.peek() == '=' || fin.peek() == '&')
				mPunctuation += fin.get();
			return;
		case '+':
			if (fin.peek() == '=' || fin.peek() == '+')
				mPunctuation += fin.get();
			return;
		case '-': 
			if (fin.peek() == '=' || fin.peek() == '-') //if this was C++, we'd also need to check for pointer ops (->)
				mPunctuation += fin.get();
			return;
		case '|':
			if (fin.peek() == '=' || fin.peek() == '|')
				mPunctuation += fin.get();
		default:
			break;
	}
}

void PunctuationToken::debugPrint() const
{
	cout << "Token: [punctuation, \"" << mPunctuation << "\"]";
}

bool PunctuationToken::characterMatches(char c)
{
	return ispunct(c);
}

void NumberLiteralToken::parseToken(std::fstream& fin)
{
	mNumber = "";
	char currentChar = fin.peek();
	bool foundDot = false;

	do 
	{
		if (currentChar == '.')
		{
			if (foundDot)
				return;

			foundDot = true;
		}

		currentChar = fin.get();
		mNumber += currentChar;

		currentChar = fin.peek();
	} while (characterMatches(currentChar));
}

void NumberLiteralToken::debugPrint() const
{
	cout << "Token: [number_literal, \"" << mNumber << "\"]";
}

bool NumberLiteralToken::characterMatches(char c)
{
	return isxdigit(c) || c == '.' || c == 'f' || c == 'F';
}

void EOLToken::parseToken(std::fstream& fin)
{
	char c = fin.peek();
	while (characterMatches(c))
	{
		fin.get();
		c = fin.peek();
	}
}

void EOLToken::debugPrint() const
{
	cout << "Token: [end_of_line, \"" << "" << "\"]";
}

bool EOLToken::characterMatches(char c)
{
	return c == '\n' || c == '\r';
}

void EOFToken::parseToken(std::fstream& fin)
{
}

void EOFToken::debugPrint() const
{
	cout << "Token: [end_of_file, \"" << "" << "\"]";
}

bool EOFToken::characterMatches(char c)
{
	return false;
}
