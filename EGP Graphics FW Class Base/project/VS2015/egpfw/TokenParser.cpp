#include "TokenParser.h"
#include "IToken.h"

void TokenParser::emit(const std::string& addition)
{
	mOutput += addition;
}

TokenParser::TokenParser()
{

}

TokenParser::~TokenParser()
{
}

void TokenParser::parseTokens(TokenStream& tokens, int glVersion)
{
}
