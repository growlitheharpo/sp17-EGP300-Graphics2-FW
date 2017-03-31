#pragma once
#include <vector>
#include <ios>

class IToken;

class TokenStream
{
	private:
		std::vector<IToken*> mTokens;
		size_t mOffset;

	public:
		TokenStream(std::vector<IToken*> const& tokens);

		IToken* get();
		IToken* peek() const;

		void seek(std::streampos pos, std::ios_base::seek_dir dir);

		bool eof() const { return mOffset == mTokens.size(); }
		bool eos() const { return eof(); }
		bool atEnd() const { return eof(); }
};
