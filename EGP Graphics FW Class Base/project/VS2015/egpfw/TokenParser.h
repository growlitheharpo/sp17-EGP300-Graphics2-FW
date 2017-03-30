#pragma once
#include <string>
#include <vector>
#include "TokenStream.h"

class IToken;

class TokenParser
{
	public:
		struct unexpected_token
		{
			explicit unexpected_token(IToken* t);
		};
		struct parse_error
		{
			explicit parse_error(std::string s);
		};

	private:
		enum ShaderType
		{
			UNKNOWN,
			VERTEX,
			FRAGMENT,
			GEOMETRY,
		};

		ShaderType mShaderType;
		std::string mOutput, mCurrentLine;
		size_t mLineCount;
		int mGLVersion;

		void emit(std::string addition);

		void consumeWhitespace(TokenStream& stream);
		
		static bool checkForSymbol(IToken* token, std::string value);
		static bool checkForPunctuation(IToken* token, std::string value);
		
	public:
		TokenParser();
		~TokenParser();

		void parseTokens(TokenStream& tokens, int glVersion);

		std::string getValue() const { return mOutput; }
};
