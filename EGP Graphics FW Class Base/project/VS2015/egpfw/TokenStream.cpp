#include "TokenStream.h"

TokenStream::TokenStream(std::vector<IToken*> const& tokens)
{
	mTokens = tokens;
	mOffset = 0;
}

TokenStream::TokenStream(std::vector<IToken>&& tokens)
{
	std::move(tokens.begin(), tokens.end(), mTokens);
	mOffset = 0;
}

IToken* TokenStream::get()
{
	if (++mOffset > mTokens.size())
		throw new std::out_of_range("Stream moved out of range!");
	
	return mTokens[mOffset - 1];
}

IToken* TokenStream::peek()
{
	return mTokens[mOffset];
}

void TokenStream::seek(std::streampos pos, std::ios_base::seek_dir dir)
{
	switch (dir)
	{
		case std::ios_base::beg:
			mOffset = pos;
			break;
		case std::ios_base::cur:
			mOffset += pos;
			break;
		case std::ios_base::end:
			mOffset = mTokens.size() - 1 - pos;
			break;
		default: 
			throw std::invalid_argument("Seek direction was invalid!");
	}
}
