#pragma once
#include "TokenStream.h"
#include "TokenParseDelegates.h"

class IToken;
class SymbolToken;
class PunctuationToken;
class NumberLiteralToken;

class TokenParser
{
	public:
		struct unexpected_token
		{
			IToken* _t;

			explicit unexpected_token(IToken* t) : _t(t) {}
			explicit unexpected_token(IToken* t, int line);
		};
		struct parse_error
		{
			explicit parse_error(std::string s);
		};

	private:
		enum ShaderType
		{
			UNKNOWN = -1,
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

		void handleRootSymbolToken(SymbolToken* t, TokenStream& stream, const std::string& val);

		token_delegate_t* getInTokenParser() const;
		token_delegate_t* getUniformTokenParser() const;
		token_delegate_t* getOutTokenParser() const;
		
		
	public:
		TokenParser();
		~TokenParser();

		void parseTokens(TokenStream& tokens, int glVersion);

		std::string getValue() const { return mOutput; }

		static bool checkForSymbol(IToken* token, std::string value);
		static bool checkForPunctuation(IToken* token, std::string value);
};
