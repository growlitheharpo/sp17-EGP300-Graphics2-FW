#include "TokenParser.h"
#include "IToken.h"
#include <iostream>
#include <array>

//Token can be the following at the root level:
	//in
	//uniform
	//out
	//const
	//#
/* VARIABLES AND RETURN TYPES:
	bool int uint float double
	bvec2 bvec3 bvec4
	ivec2 ivec3 ivec4
	uvec2 uvec3 uvec4
	dvec2 dvec3 dvec4
	vec2 vec3 vec4
	mat2 mat2x2 mat2x3 mat2x4
	mat3 mat3x2 mat3x3 mat3x4
	mat4 mat4x2 mat4x3 mat4x4
	// JUST VARIABLES:
	sampler1D sampler2D sampler3D samplerCube sampler2DRect sampler1DArray sampler2DArray samplerBuffer sampler2DMS sampler2DMSArray
	isampler1D isampler2D isampler3D isamplerCube isampler2DRect isampler1DArray isampler2DArray isamplerBuffer isampler2DMS isampler2DMSArray
	usampler1D usampler2D usampler3D usamplerCube usampler2DRect usampler1DArray usampler2DArray usamplerBuffer usampler2DMS usampler2DMSArray
	sampler1DShadow sampler2DShadow samplerCubeShadow sampler2DRectShadow sampler1DArrayShadow sampler2DArrayShadow samplerCubeArrayShadow
	image1D image2D image3D imageCube image2DRect image1DArray image2DArray imageCubeArray imageBuffer image2DMS image2DMSArray
	iimage1D iimage2D iimage3D iimageCube iimage2DRect iimage1DArray iimage2DArray iimageCubeArray iimageBuffer iimage2DMS iimage2DMSArray
	uimage1D uimage2D uimage3D uimageCube uimage2DRect uimage1DArray uimage2DArray uimageCubeArray uimageBuffer uimage2DMS uimage2DMSArray
	atomic_uint
*/

const std::array<std::string, 3> ROOT_LEVEL_SYMBOLS = { "in","uniform", "out" }; //any others will just be directly emited for GL to determine the validity of.

TokenParser::unexpected_token::unexpected_token(IToken* t, int line)
{
	std::cout << "ERROR PARSING FILE: UNEXPECTED TOKEN: ";
	t->debugPrint();
	std::cout << " at line " << (line + 1) << std::endl;
}

TokenParser::parse_error::parse_error(std::string msg)
{
	std::cout << "ERROR: " << msg << std::endl;
	throw std::runtime_error(msg);
}

void TokenParser::emit(std::string addition)
{
	mCurrentLine += addition;

	if (addition == "\n" || addition == "\r\n" || addition == "\r")
	{
		std::string::size_type numTabs = 7 - mCurrentLine.size() / 10;
		mOutput += std::string(numTabs, '\t') + "//LINE " + std::to_string(++mLineCount);

		addition = "\n";
		mCurrentLine = "";
	}

	mOutput += addition;
}

bool TokenParser::checkForSymbol(IToken* token, std::string value)
{
	SymbolToken* st = dynamic_cast<SymbolToken*>(token);

	if (st != nullptr)
		return st->getValue() == value;

	return false;
}

bool TokenParser::checkForPunctuation(IToken* token, std::string value)
{
	PunctuationToken* pt = dynamic_cast<PunctuationToken*>(token);

	if (pt != nullptr)
		return pt->getValue() == value;

	return false;
}

void TokenParser::consumeWhitespace(TokenStream& stream)
{
	IToken* peekToken = stream.peek();

	if (peekToken->getType() != IToken::END_OF_LINE && peekToken->getType() != IToken::WHITESPACE)
	{
		throw unexpected_token(peekToken, mLineCount);
	}

	while (true)
	{
		if (peekToken->getType() == IToken::END_OF_LINE)
			emit("\r\n");
		else if (peekToken->getType() == IToken::WHITESPACE)
			emit(" ");
		else
			return;

		stream.get();
		peekToken = stream.peek();
	}
}

void TokenParser::handleRootSymbolToken(SymbolToken* t, TokenStream& stream, const std::string& val)
{
	token_delegate_t* parser;

	if (val == "in")
		parser = getInTokenParser();
	else if (val == "uniform")
		parser = getUniformTokenParser();
	else if (val == "out")
		parser = getOutTokenParser();
	else
		throw std::invalid_argument("Tried to handle a special token, but it was invalid!");

	if (parser == nullptr)
		throw parse_error("Could not find a proper GL configuration for the symbol \"" + t->getValue() + " at line " + std::to_string(mLineCount));

	try
	{
		parser((emitter_delegate(*this, &TokenParser::emit, &TokenParser::consumeWhitespace)), stream, t);
	}
	catch (unexpected_token e)
	{
		throw unexpected_token(e._t, mLineCount);
	}
}

void TokenParser::handleVaryingPrefixInProgram(SymbolToken* token, TokenStream& tokens)
{
	IToken* next = tokens.peek();

	if (next->getType() == IToken::PUNCTUATION && static_cast<PunctuationToken*>(next)->getValue() == ".")
	{
		tokens.get(); //consume the "."
		emit(mVaryingPrefix + "__");
	}
	else
		emit(token->getValue());
}

token_delegate_t* TokenParser::getInTokenParser() const
{
	switch (mShaderType)
	{
		case VERTEX:
		{
			if (mGLVersion < 130)
				return &parse_VertexIn100;
			if (mGLVersion < 330)
				return &parse_VertexIn130;
			if (mGLVersion >= 330)
				return &parse_VertexIn330;
		}
		break;

		case FRAGMENT:
		{
			if (mGLVersion < 130)
				return &parse_FragmentIn100;
			if (mGLVersion < 330)
				return &parse_FragmentIn130;
			if (mGLVersion >= 330)
				return &parse_FragmentIn330;
		}
		break;

		case GEOMETRY:
		{
			if (mGLVersion < 330)
				return &parse_GeometryIn320;
			if (mGLVersion >= 330)
				return &parse_GeometryIn330;
		}
		break;

		default:
			return nullptr;
	}

	return nullptr;
}

token_delegate_t* TokenParser::getUniformTokenParser() const
{
	return &parse_Uniform;
}

token_delegate_t* TokenParser::getOutTokenParser() const
{
	switch (mShaderType)
	{
	case VERTEX:
	{
		if (mGLVersion < 130)
			return &parse_VertexOut100;
		if (mGLVersion < 330)
			return &parse_VertexOut130;
		if (mGLVersion >= 330)
			return &parse_VertexOut330;
	}
	break;

	case FRAGMENT:
	{
		if (mGLVersion < 130)
			return &parse_FragmentOut100;
		if (mGLVersion < 330)
			return &parse_FragmentOut130;
		if (mGLVersion >= 330)
			return &parse_FragmentOut330;
	}
	break;

	case GEOMETRY:
	{
		if (mGLVersion < 330)
			return &parse_GeometryOut320;
		if (mGLVersion >= 330)
			return &parse_GeometryOut330;
	}
	break;

	default:
		return nullptr;
	}

	return nullptr;
}

TokenParser::TokenParser()
{
	mLineCount = 0;
	mShaderType = UNKNOWN;
	mGLVersion = -1;
}

TokenParser::~TokenParser()
{
}

void TokenParser::parseTokens(TokenStream& tokens, int glVersion)
{
	mCurrentDefaultLane = 0;
	mVaryingPrefix = "";
	mGLVersion = glVersion;
	IToken* peekToken;

	//Luckily, our shader language has a very clearly defined syntax.
	//First, we clear (and emit, to make sure line numbers match) anything
	//at the beginning until we reach the shadertype token.
	consumeWhitespace(tokens);
	peekToken = tokens.get();

	//Leading whitespace has been cleared. Let's make sure we have a shadertype symbol.
	if (!checkForSymbol(peekToken, "shadertype"))
		throw unexpected_token(peekToken, mLineCount);

	consumeWhitespace(tokens);
	peekToken = tokens.get();

	if (checkForSymbol(peekToken, "vertex"))
		mShaderType = VERTEX;
	else if (checkForSymbol(peekToken, "fragment"))
		mShaderType = FRAGMENT;
	else if (checkForSymbol(peekToken, "geometry") && mGLVersion >= 320)
		mShaderType = GEOMETRY;
	else
		throw unexpected_token(peekToken, mLineCount);

	//normally, we would emit the tokens we just consumed, but there is no equivelent of shadertype. we'll emit our GLSL version instead.
	emit("#version " + std::to_string(mGLVersion));

	std::string peekValue;

	while (!tokens.eof())
	{
		peekToken = tokens.get();

		switch (peekToken->getType())
		{
			case IToken::SYMBOL:
				peekValue = static_cast<SymbolToken*>(peekToken)->getValue();

				if (find(ROOT_LEVEL_SYMBOLS.begin(), ROOT_LEVEL_SYMBOLS.end(), peekValue) != ROOT_LEVEL_SYMBOLS.end())
					handleRootSymbolToken(static_cast<SymbolToken*>(peekToken), tokens, peekValue);
				else if (peekValue == mVaryingPrefix && mGLVersion < 330)
					handleVaryingPrefixInProgram(static_cast<SymbolToken*>(peekToken), tokens);
				else
					emit(static_cast<SymbolToken*>(peekToken)->getValue());

				break;
			case IToken::WHITESPACE:
				emit(" ");
				break;
			case IToken::PUNCTUATION:
				emit(static_cast<PunctuationToken*>(peekToken)->getValue());
				break;
			case IToken::NUMBER_LITERAL:
				emit(static_cast<NumberLiteralToken*>(peekToken)->getValue());
				break;
			case IToken::END_OF_LINE:
				emit("\r\n");
				break;
			case IToken::END_OF_FILE:
				break;
			default:
				throw unexpected_token(peekToken, mLineCount);
		}

	}
}
