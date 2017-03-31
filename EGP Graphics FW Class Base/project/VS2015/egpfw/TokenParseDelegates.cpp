#include "TokenParseDelegates.h"
#include "TokenStream.h"
#include "TokenParser.h"
#include "IToken.h"

std::string getEmissionString(IToken* t)
{
	EmitableToken* e = dynamic_cast<EmitableToken*>(t);
	if (e != nullptr)
		return e->getValue();

	if (t->getType() == IToken::END_OF_LINE)
		return "\n";

	if (t->getType() == IToken::WHITESPACE)
		return " ";

	return "";
}

unsigned getLaneToken(TokenStream& in)
{
	IToken* laneToken;
	IToken* t = in.peek();
	if (!TokenParser::checkForPunctuation(t, "@"))
		throw TokenParser::unexpected_token(t);

	in.get();
	t = in.peek();
	if (!TokenParser::checkForSymbol(t, "lane"))
		throw TokenParser::unexpected_token(t);

	in.get();
	t = in.peek();
	if (!TokenParser::checkForPunctuation(t, "("))
		throw TokenParser::unexpected_token(t);

	in.get();
	laneToken = in.peek();

	in.get();
	t = in.peek();
	if (!TokenParser::checkForPunctuation(t, ")"))
		throw TokenParser::unexpected_token(t);

	if (laneToken->getType() == IToken::NUMBER_LITERAL)
	{
		return stoul(static_cast<NumberLiteralToken*>(laneToken)->getValue());
	}
	
	if (TokenParser::checkForSymbol(laneToken, "default"))
	{
		//eventually do some stuff here to calculate a default layout location.
		return 0;
	}
	else
		throw TokenParser::unexpected_token(laneToken);

	return 0;
}

void emitUntilPunctuation(const emitter_delegate& out, TokenStream& in, std::string punc, bool emitPunc)
{
	IToken* peekToken;

	do {
		peekToken = in.get();

		if (peekToken->getType() == IToken::PUNCTUATION && static_cast<PunctuationToken*>(peekToken)->getValue() == punc)
		{
			if (emitPunc)
				out.emit(getEmissionString(peekToken));

			break;
		}

		out.emit(getEmissionString(peekToken));

	} while (!in.eof());
}

void parse_VertexIn100(const emitter_delegate& out, TokenStream& in, EmitableToken* startToken)
{
	//As simple as it gets. We'll get "in attributes { ... }" and just need to emit everything in the braces with "in" before it.
	IToken* t;
	
	//We got the "in" already. Clear until we hit the "attribute" symbol.
	out.consumeWhitespace(in);
	t = in.get();
	
	if (!TokenParser::checkForSymbol(t, "attribute"))
		throw TokenParser::unexpected_token(t);

	//Consumed the "attribute". Let's find the open bracket.
	out.consumeWhitespace(in);
	t = in.get();

	if (!TokenParser::checkForPunctuation(t, "{"))
		throw TokenParser::unexpected_token(t);

	//Great, we're inside the struct. Time to start emitting.
	do
	{
		t = in.get();

		switch (t->getType())
		{
			case IToken::SYMBOL:				//we hit a symbol. It SHOULD be a typename.
				out.emit("attribute ");
				out.emit(getEmissionString(t)); //should output the typename.
				getLaneToken(in);				//will clear the "@lane(x)" portion.
				emitUntilPunctuation(out, in, ";", true);	//emit the variable name and the ;
				break;

			default: 
				out.emit(getEmissionString(t));
				break;
		}

	} while (!TokenParser::checkForPunctuation(t, "}"));
}

void parse_VertexIn130(const emitter_delegate& out, TokenStream& in, EmitableToken* t)
{
}

void parse_VertexIn330(const emitter_delegate& out, TokenStream& in, EmitableToken* t)
{
}

void parse_VertexOut100(const emitter_delegate& out, TokenStream& in, EmitableToken* t)
{
}

void parse_VertexOut130(const emitter_delegate& out, TokenStream& in, EmitableToken* t)
{
}

void parse_VertexOut330(const emitter_delegate& out, TokenStream& in, EmitableToken* t)
{
}

void parse_FragmentIn100(const emitter_delegate& out, TokenStream& in, EmitableToken* t)
{
}

void parse_FragmentIn130(const emitter_delegate& out, TokenStream& in, EmitableToken* t)
{
}

void parse_FragmentIn330(const emitter_delegate& out, TokenStream& in, EmitableToken* t)
{
}

void parse_FragmentOut100(const emitter_delegate& out, TokenStream& in, EmitableToken* t)
{
}

void parse_FragmentOut130(const emitter_delegate& out, TokenStream& in, EmitableToken* t)
{
}

void parse_FragmentOut330(const emitter_delegate& out, TokenStream& in, EmitableToken* t)
{
}

void parse_GeometryIn320(const emitter_delegate& out, TokenStream& in, EmitableToken* t)
{
}

void parse_GeometryIn330(const emitter_delegate& out, TokenStream& in, EmitableToken* t)
{
}

void parse_GeometryOut320(const emitter_delegate& out, TokenStream& in, EmitableToken* t)
{
}

void parse_GeometryOut330(const emitter_delegate& out, TokenStream& in, EmitableToken* t)
{
}

void parse_Uniform(const emitter_delegate& out, TokenStream& in, EmitableToken* t)
{
}
