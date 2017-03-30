#pragma once
#include <string>
#include <vector>
#include "TokenStream.h"

class IToken;

class TokenParser
{
	private:
		std::string mOutput;

		void emit(const std::string& addition);
		
	public:
		TokenParser();
		~TokenParser();

		void parseTokens(TokenStream& tokens, int glVersion);

		std::string getValue() const { return mOutput; }
};
