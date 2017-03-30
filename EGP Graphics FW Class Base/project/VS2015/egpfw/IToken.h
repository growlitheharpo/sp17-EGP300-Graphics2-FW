/***************************************************************************************** /
 *
 * Many, many many thanks to the author of the following post, which was instrumental
 * in even just getting started with this project.
 * http://www.dreamincode.net/forums/topic/153718-fundamentals-of-parsing/
 * 
 ******************************************************************************************/
#pragma once
#include <fstream>

class IToken
{
	public:
		enum Type
		{
			INVALID = 0,
			SYMBOL,
			WHITESPACE,
			PUNCTUATION,
			NUMBER_LITERAL,
			END_OF_LINE,
			END_OF_FILE,
		};

	private:
		Type mType;

	public:
		explicit IToken(Type t) : mType(t) {}
		virtual ~IToken() = default;

		virtual void parseToken(std::fstream& fin) = 0;
		virtual void debugPrint() const = 0;

		Type getType() const { return mType; }
};

class EmitableToken : public IToken
{
	public:
		EmitableToken(Type t) : IToken(t) {}
		virtual ~EmitableToken() = default;

		virtual std::string getValue() const = 0;
};

class SymbolToken : public EmitableToken
{
	private:
		std::string mSymbol;

	public:
		SymbolToken() : EmitableToken(SYMBOL) {}
		~SymbolToken() = default;

		void parseToken(std::fstream& fin) override;
		void debugPrint() const override;

		std::string getValue() const override { return mSymbol; }

		static bool characterMatches(char c);
};

class WhitespaceToken : public IToken
{
	private:
		std::string mWhitespace;

	public:
		WhitespaceToken() : IToken(WHITESPACE) {}
		~WhitespaceToken() = default;

		void parseToken(std::fstream& fin) override;
		void debugPrint() const override;

		static bool characterMatches(char c);
};

class PunctuationToken : public EmitableToken
{
	private:
		std::string mPunctuation;

	public:
		PunctuationToken() : EmitableToken(PUNCTUATION) {}
		~PunctuationToken() = default;

		void parseToken(std::fstream& fin) override;
		void debugPrint() const override;

		std::string getValue() const override { return mPunctuation; }

		static bool characterMatches(char c);
};

class NumberLiteralToken : public EmitableToken
{
	private:
		std::string mNumber;

	public:
		NumberLiteralToken() : EmitableToken(NUMBER_LITERAL) {}
		~NumberLiteralToken() = default;

		void parseToken(std::fstream& fin) override;
		void debugPrint() const override;

		std::string getValue() const override { return mNumber; }

		static bool characterMatches(char c);
};

class EOLToken : public IToken
{
	public:
		EOLToken() : IToken(END_OF_LINE) {}
		~EOLToken() = default;

		void parseToken(std::fstream& fin) override;
		void debugPrint() const override;

		static bool characterMatches(char c);
};

class EOFToken : public IToken
{
	public:
		EOFToken() : IToken(END_OF_FILE) {}
		~EOFToken() = default;

		void parseToken(std::fstream& fin) override;
		void debugPrint() const override;

		static bool characterMatches(char c);
};
