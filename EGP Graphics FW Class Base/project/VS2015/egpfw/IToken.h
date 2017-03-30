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

class SymbolToken : public IToken
{
	private:
		std::string mSymbol;

	public:
		SymbolToken() : IToken(SYMBOL) {}
		~SymbolToken() = default;

		void parseToken(std::fstream& fin) override;
		void debugPrint() const override;

		std::string getValue() const { return mSymbol; }

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

class PunctuationToken : public IToken
{
	private:
		std::string mPunctuation;

	public:
		PunctuationToken() : IToken(PUNCTUATION) {}
		~PunctuationToken() = default;

		void parseToken(std::fstream& fin) override;
		void debugPrint() const override;

		std::string getValue() const { return mPunctuation; }

		static bool characterMatches(char c);
};

class NumberLiteralToken : public IToken
{
	private:
		std::string mNumber;

	public:
		NumberLiteralToken() : IToken(NUMBER_LITERAL) {}
		~NumberLiteralToken() = default;

		void parseToken(std::fstream& fin) override;
		void debugPrint() const override;

		std::string getValue() const { return mNumber; }

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
