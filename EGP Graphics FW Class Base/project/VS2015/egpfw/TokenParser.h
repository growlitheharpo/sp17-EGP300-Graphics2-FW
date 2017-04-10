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

		std::string mOutput, mCurrentLine, mVaryingPrefix;
		unsigned mCurrentDefaultLane;
		ShaderType mShaderType;
		size_t mLineCount;
		int mGLVersion;

		void emit(std::string addition);

		void consumeWhitespace(TokenStream& stream);

		void handleRootSymbolToken(SymbolToken* t, TokenStream& stream, const std::string& val);
		void handleVaryingPrefixInProgram(SymbolToken* token, TokenStream& tokens);

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

		void setVaryingPrefix(const std::string& s) { mVaryingPrefix = s; }
		std::string getVaryingPrefix() const { return mVaryingPrefix; }

		void incrementDefaultLane() { ++mCurrentDefaultLane; }
		unsigned getDefaultLane() const { return mCurrentDefaultLane; }

		void saveOutputFile(const std::string& file) const;
};

typedef void (TokenParser::*emiter_function)(std::string);
typedef void (TokenParser::*consumer_function)(TokenStream&);

struct emitter_delegate
{
	private:
		TokenParser& p;
		emiter_function f1;
		consumer_function f2;

	public:
		emitter_delegate(TokenParser& a, emiter_function b, consumer_function c) : p(a), f1(b), f2(c) {}

		void emit(std::string s) const
		{
			((p).*(f1))(s);
		}

		void consumeWhitespace(TokenStream& s) const
		{
			((p).*(f2))(s);
		}

		void setVaryingPrefix(const std::string& s) const
		{
			p.setVaryingPrefix(s);
		}

		std::string getVaryingPrefix() const
		{
			return p.getVaryingPrefix();
		}

		void incrementDefaultLane() const
		{
			p.incrementDefaultLane();
		}

		unsigned getCurrentDefaultLane() const
		{
			return p.getDefaultLane();
		}
};