#include "TokenParseDelegates.h"
#include "TokenStream.h"
#include "TokenParser.h"
#include "IToken.h"
#include <iostream>

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

unsigned getLaneToken(TokenStream& in, const emitter_delegate& d)
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

	in.get();

	if (laneToken->getType() == IToken::NUMBER_LITERAL)
	{
		return stoul(static_cast<NumberLiteralToken*>(laneToken)->getValue());
	}
	
	if (TokenParser::checkForSymbol(laneToken, "default"))
	{
		d.incrementDefaultLane();
		return d.getCurrentDefaultLane() - 1;
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

	t = in.get();

	//Great, we're inside the struct. Time to start emitting.
	while (!TokenParser::checkForPunctuation(t, "}"))
	{
		switch (t->getType())
		{
			case IToken::SYMBOL:				//we hit a symbol. It SHOULD be a typename.
				out.emit("attribute ");
				out.emit(getEmissionString(t)); //should output the typename.
				out.consumeWhitespace(in);		//clear space between typename and lane.
				getLaneToken(in, out);			//will clear the "@lane(x)" portion.
				emitUntilPunctuation(out, in, ";", true);	//emit the variable name and the ;
				break;

			default: 
				out.emit(getEmissionString(t));
				break;
		}

		t = in.get();
	}
}

void parse_VertexIn130(const emitter_delegate& out, TokenStream& in, EmitableToken* startToken)
{
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

	t = in.get();

	//Great, we're inside the struct. Time to start emitting.
	while (!TokenParser::checkForPunctuation(t, "}"))
	{
		switch (t->getType())
		{
		case IToken::SYMBOL:				//we hit a symbol. It SHOULD be a typename.
			out.emit("in ");
			out.emit(getEmissionString(t)); //should output the typename.
			out.consumeWhitespace(in);		//clear space between typename and lane.
			getLaneToken(in, out);			//will clear the "@lane(x)" portion.
			emitUntilPunctuation(out, in, ";", true);	//emit the variable name and the ;
			break;

		default:
			out.emit(getEmissionString(t));
			break;
		}

		t = in.get();
	}
}

void parse_VertexIn330(const emitter_delegate& out, TokenStream& in, EmitableToken* startToken)
{
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

	t = in.get();

	//Great, we're inside the struct. Time to start emitting.
	while (!TokenParser::checkForPunctuation(t, "}"))
	{
		switch (t->getType())
		{
			case IToken::SYMBOL:	//we hit a symbol. It SHOULD be a typename.
			{
				//Get the data we need first.
				std::string type = getEmissionString(t);
				out.consumeWhitespace(in);
				unsigned location = getLaneToken(in, out);

				//Emit the layout location and type, then emit till the end to get the variable names.
				out.emit("layout (location = " + std::to_string(location) + ") in ");
				out.emit(type + " ");

				emitUntilPunctuation(out, in, ";", true);	//emit the variable name and the ;
			}
			break;

			default:
				out.emit(getEmissionString(t));
				break;
		}

		t = in.get();
	}
}

void parse_VertexOut100(const emitter_delegate& out, TokenStream& in, EmitableToken* startToken)
{
	IToken* t;

	//We got the "out" already. Clear until we hit the "varying" symbol.
	out.consumeWhitespace(in);
	t = in.get();

	if (!TokenParser::checkForSymbol(t, "varying"))
		throw TokenParser::unexpected_token(t);

	//Consumed the "varying". Let's find our pass struct name.
	out.consumeWhitespace(in);
	t = in.get();

	if (!t->getType() == IToken::SYMBOL)
		throw TokenParser::unexpected_token(t);
	out.setVaryingPrefix(static_cast<SymbolToken*>(t)->getValue());
	
	out.consumeWhitespace(in);
	t = in.get();

	if (!TokenParser::checkForPunctuation(t, "{"))
		throw TokenParser::unexpected_token(t);

	t = in.get();

	//Great, we're inside the struct. Time to start emitting.
	while (!TokenParser::checkForPunctuation(t, "}"))
	{
		switch (t->getType())
		{
		case IToken::SYMBOL:				//we hit a symbol. It SHOULD be a typename.
			out.emit("varying ");
			out.emit(getEmissionString(t)); //should output the typename.
			out.consumeWhitespace(in);		//move up to the variable name.
			t = in.get();
			out.emit(out.getVaryingPrefix() + "__" + getEmissionString(t));
			emitUntilPunctuation(out, in, ";", true);	//emit any whitespace and the ;
			break;

		default:
			out.emit(getEmissionString(t));
			break;
		}

		t = in.get();
	}
}

void parse_VertexOut130(const emitter_delegate& out, TokenStream& in, EmitableToken* startToken)
{
	IToken* t;

	//We got the "out" already. Clear until we hit the "varying" symbol.
	out.consumeWhitespace(in);
	t = in.get();

	if (!TokenParser::checkForSymbol(t, "varying"))
		throw TokenParser::unexpected_token(t);

	//Consumed the "varying". Let's find our pass struct name.
	out.consumeWhitespace(in);
	t = in.get();

	if (!t->getType() == IToken::SYMBOL)
		throw TokenParser::unexpected_token(t);
	out.setVaryingPrefix(static_cast<SymbolToken*>(t)->getValue());

	out.consumeWhitespace(in);
	t = in.get();

	if (!TokenParser::checkForPunctuation(t, "{"))
		throw TokenParser::unexpected_token(t);

	t = in.get();

	//Great, we're inside the struct. Time to start emitting.
	while (!TokenParser::checkForPunctuation(t, "}"))
	{
		switch (t->getType())
		{
		case IToken::SYMBOL:				//we hit a symbol. It SHOULD be a typename.
			out.emit("out ");
			out.emit(getEmissionString(t)); //should output the typename.
			out.consumeWhitespace(in);		//move up to the variable name.
			t = in.get();
			out.emit(out.getVaryingPrefix() + "__" + getEmissionString(t));
			emitUntilPunctuation(out, in, ";", true);	//emit any whitespace and the ;
			break;

		default:
			out.emit(getEmissionString(t));
			break;
		}

		t = in.get();
	}

}

void parse_VertexOut330(const emitter_delegate& out, TokenStream& in, EmitableToken* startToken)
{
	IToken* t;

	//We got the "out" already. Clear until we hit the "varying" symbol.
	out.consumeWhitespace(in);
	t = in.get();

	if (!TokenParser::checkForSymbol(t, "varying"))
		throw TokenParser::unexpected_token(t);

	out.emit(" out VertexData ");

	//Consumed the "varying". Let's find our pass struct name.
	out.consumeWhitespace(in);
	t = in.get();

	if (!t->getType() == IToken::SYMBOL)
		throw TokenParser::unexpected_token(t);
	out.setVaryingPrefix(static_cast<SymbolToken*>(t)->getValue());

	out.consumeWhitespace(in);
	t = in.get();

	if (!TokenParser::checkForPunctuation(t, "{"))
		throw TokenParser::unexpected_token(t);

	//Great, we're inside the struct. Time to start emitting.
	while (!TokenParser::checkForPunctuation(t, "}"))
	{
		switch (t->getType())
		{
			case IToken::SYMBOL:				//we hit a symbol. It SHOULD be a typename.
				out.emit(getEmissionString(t)); //should output the typename.
				out.consumeWhitespace(in);		//move up to the variable name.
				out.emit(getEmissionString(in.get()));
				emitUntilPunctuation(out, in, ";", true);	//emit any whitespace and the ;
				break;

			default:
				out.emit(getEmissionString(t));
				break;
		}

		t = in.get();
	}

	out.emit(getEmissionString(t) + " " + out.getVaryingPrefix() + ";"); //emit the close brace
}



void parse_FragmentIn100(const emitter_delegate& out, TokenStream& in, EmitableToken* startToken)
{
	IToken* t;

	//We got the "in" already. Clear until we hit the "varying" symbol.
	out.consumeWhitespace(in);
	t = in.get();

	if (!TokenParser::checkForSymbol(t, "varying"))
		throw TokenParser::unexpected_token(t);

	//Consumed the "varying". Let's find our pass struct name.
	out.consumeWhitespace(in);
	t = in.get();

	if (!t->getType() == IToken::SYMBOL)
		throw TokenParser::unexpected_token(t);
	out.setVaryingPrefix(static_cast<SymbolToken*>(t)->getValue());

	out.consumeWhitespace(in);
	t = in.get();

	if (!TokenParser::checkForPunctuation(t, "{"))
		throw TokenParser::unexpected_token(t);

	t = in.get();

	//Great, we're inside the struct. Time to start emitting.
	while (!TokenParser::checkForPunctuation(t, "}"))
	{
		switch (t->getType())
		{
		case IToken::SYMBOL:				//we hit a symbol. It SHOULD be a typename.
			out.emit("varying ");
			out.emit(getEmissionString(t)); //should output the typename.
			out.consumeWhitespace(in);		//move up to the variable name.
			t = in.get();
			out.emit(out.getVaryingPrefix() + "__" + getEmissionString(t));
			emitUntilPunctuation(out, in, ";", true);	//emit any whitespace and the ;
			break;

		default:
			out.emit(getEmissionString(t));
			break;
		}

		t = in.get();
	}
}

void parse_FragmentIn130(const emitter_delegate& out, TokenStream& in, EmitableToken* startToken)
{
	IToken* t;

	//We got the "in" already. Clear until we hit the "varying" symbol.
	out.consumeWhitespace(in);
	t = in.get();

	if (!TokenParser::checkForSymbol(t, "varying"))
		throw TokenParser::unexpected_token(t);

	//Consumed the "varying". Let's find our pass struct name.
	out.consumeWhitespace(in);
	t = in.get();

	if (!t->getType() == IToken::SYMBOL)
		throw TokenParser::unexpected_token(t);
	out.setVaryingPrefix(static_cast<SymbolToken*>(t)->getValue());

	out.consumeWhitespace(in);
	t = in.get();

	if (!TokenParser::checkForPunctuation(t, "{"))
		throw TokenParser::unexpected_token(t);

	t = in.get();

	//Great, we're inside the struct. Time to start emitting.
	while (!TokenParser::checkForPunctuation(t, "}"))
	{
		switch (t->getType())
		{
		case IToken::SYMBOL:				//we hit a symbol. It SHOULD be a typename.
			out.emit("in ");
			out.emit(getEmissionString(t)); //should output the typename.
			out.consumeWhitespace(in);		//move up to the variable name.
			t = in.get();
			out.emit(out.getVaryingPrefix() + "__" + getEmissionString(t));
			emitUntilPunctuation(out, in, ";", true);	//emit any whitespace and the ;
			break;

		default:
			out.emit(getEmissionString(t));
			break;
		}

		t = in.get();
	}
}

void parse_FragmentIn330(const emitter_delegate& out, TokenStream& in, EmitableToken* startTokent)
{
	IToken* t;

	//We got the "in" already. Clear until we hit the "varying" symbol.
	out.consumeWhitespace(in);
	t = in.get();

	if (!TokenParser::checkForSymbol(t, "varying"))
		throw TokenParser::unexpected_token(t);

	out.emit(" in VertexData ");

	//Consumed the "varying". Let's find our pass struct name.
	out.consumeWhitespace(in);
	t = in.get();

	if (!t->getType() == IToken::SYMBOL)
		throw TokenParser::unexpected_token(t);
	out.setVaryingPrefix(static_cast<SymbolToken*>(t)->getValue());

	out.consumeWhitespace(in);
	t = in.get();

	if (!TokenParser::checkForPunctuation(t, "{"))
		throw TokenParser::unexpected_token(t);

	//Great, we're inside the struct. Time to start emitting.
	while (!TokenParser::checkForPunctuation(t, "}"))
	{
		switch (t->getType())
		{
		case IToken::SYMBOL:				//we hit a symbol. It SHOULD be a typename.
			out.emit(getEmissionString(t)); //should output the typename.
			out.consumeWhitespace(in);		//move up to the variable name.
			out.emit(getEmissionString(in.get()));
			emitUntilPunctuation(out, in, ";", true);	//emit any whitespace and the ;
			break;

		default:
			out.emit(getEmissionString(t));
			break;
		}

		t = in.get();
	}

	out.emit(getEmissionString(t) + " " + out.getVaryingPrefix() + ";"); //emit the close brace
}

void parse_FragmentOut100(const emitter_delegate& out, TokenStream& in, EmitableToken* startToken)
{
	//Alright, we're cheating a bit here. Technically GLSL didn't have fragment outputs until 130. 
	//If the user has anything in the "target" struct, we'll emit a global variable
	//as well as a warning that their fragment shader might not function properly.

	IToken* t;

	//We got the "out" already. Clear until we hit the "target" symbol.
	out.consumeWhitespace(in);
	t = in.get();

	if (!TokenParser::checkForSymbol(t, "target"))
		throw TokenParser::unexpected_token(t);

	//Consumed the "target". Let's find the open bracket.
	out.consumeWhitespace(in);
	t = in.get();

	if (!TokenParser::checkForPunctuation(t, "{"))
		throw TokenParser::unexpected_token(t);

	t = in.get();

	//Great, we're inside the struct. Time to start emitting.
	while (!TokenParser::checkForPunctuation(t, "}"))
	{
		switch (t->getType())
		{
		case IToken::SYMBOL:				//we hit a symbol. It SHOULD be a typename.
			out.emit(getEmissionString(t)); //output the typename.
			out.consumeWhitespace(in);		//clear space between typename and lane.
			getLaneToken(in, out);			//will clear the "@lane(x)" portion.
			emitUntilPunctuation(out, in, ";", true);	//emit the variable name and the ;

			std::cout << "WARNING: GLSL < 130 did not support Fragment shader outputs. Consider using gl_FragColor.\n";

			break;

		default:
			out.emit(getEmissionString(t));
			break;
		}

		t = in.get();
	}
}

void parse_FragmentOut130(const emitter_delegate& out, TokenStream& in, EmitableToken* startToken)
{
	IToken* t;

	//We got the "out" already. Clear until we hit the "target" symbol.
	out.consumeWhitespace(in);
	t = in.get();

	if (!TokenParser::checkForSymbol(t, "target"))
		throw TokenParser::unexpected_token(t);

	//Consumed the "target". Let's find the open bracket.
	out.consumeWhitespace(in);
	t = in.get();

	if (!TokenParser::checkForPunctuation(t, "{"))
		throw TokenParser::unexpected_token(t);

	t = in.get();

	//Great, we're inside the struct. Time to start emitting.
	while (!TokenParser::checkForPunctuation(t, "}"))
	{
		switch (t->getType())
		{
		case IToken::SYMBOL:				//we hit a symbol. It SHOULD be a typename.
			out.emit("out ");
			out.emit(getEmissionString(t)); //should output the typename.
			out.consumeWhitespace(in);		//clear space between typename and lane.
			getLaneToken(in, out);			//will clear the "@lane(x)" portion.
			emitUntilPunctuation(out, in, ";", true);	//emit the variable name and the ;
			break;

		default:
			out.emit(getEmissionString(t));
			break;
		}

		t = in.get();
	}
}

void parse_FragmentOut330(const emitter_delegate& out, TokenStream& in, EmitableToken* startToken)
{
	IToken* t;

	//We got the "in" already. Clear until we hit the "target" symbol.
	out.consumeWhitespace(in);
	t = in.get();

	if (!TokenParser::checkForSymbol(t, "target"))
		throw TokenParser::unexpected_token(t);

	//Consumed the "target". Let's find the open bracket.
	out.consumeWhitespace(in);
	t = in.get();

	if (!TokenParser::checkForPunctuation(t, "{"))
		throw TokenParser::unexpected_token(t);

	t = in.get();

	//Great, we're inside the struct. Time to start emitting.
	while (!TokenParser::checkForPunctuation(t, "}"))
	{
		switch (t->getType())
		{
		case IToken::SYMBOL:	//we hit a symbol. It SHOULD be a typename.
		{
			//Get the data we need first.
			std::string type = getEmissionString(t);
			out.consumeWhitespace(in);
			unsigned location = getLaneToken(in, out);

			//Emit the layout location and type, then emit till the end to get the variable names.
			out.emit("layout (location = " + std::to_string(location) + ") out ");
			out.emit(type + " ");

			emitUntilPunctuation(out, in, ";", true);	//emit the variable name and the ;
		}
		break;

		default:
			out.emit(getEmissionString(t));
			break;
		}

		t = in.get();
	}
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



void parse_Uniform(const emitter_delegate& out, TokenStream& in, EmitableToken* startToken)
{
	//As simple as it gets. We'll get "in attributes { ... }" and just need to emit everything in the braces with "in" before it.
	IToken* t;

	//We got the "uniform" already. Clear until we hit the open bracket.
	out.consumeWhitespace(in);
	t = in.get();

	if (!TokenParser::checkForPunctuation(t, "{"))
		throw TokenParser::unexpected_token(t);

	t = in.get();

	//Great, we're inside the struct. Time to start emitting.
	while (!TokenParser::checkForPunctuation(t, "}"))
	{
		switch (t->getType())
		{
		case IToken::SYMBOL:				//we hit a symbol. It SHOULD be a typename.
			out.emit("uniform ");
			out.emit(getEmissionString(t)); //should output the typename.
			out.consumeWhitespace(in);		//clear space between typename and lane.
			emitUntilPunctuation(out, in, ";", true);	//emit the variable name and the ;
			break;

		default:
			out.emit(getEmissionString(t));
			break;
		}

		t = in.get();
	}
}
