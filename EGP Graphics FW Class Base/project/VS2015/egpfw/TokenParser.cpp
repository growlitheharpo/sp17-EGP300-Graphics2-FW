#include "TokenParser.h"
#include "IToken.h"
#include <iostream>

TokenParser::unexpected_token::unexpected_token(IToken* t)
{
	std::cout << "ERROR PARSING FILE: UNEXPECTED TOKEN: ";
	t->debugPrint();
	std::cout << std::endl;
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
		throw unexpected_token(peekToken);
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
	mGLVersion = glVersion;
	IToken* peekToken;

	//Luckily, our shader language has a very clearly defined syntax.
	//First, we clear (and emit, to make sure line numbers match) anything
	//at the beginning until we reach the shadertype token.
	consumeWhitespace(tokens);
	peekToken = tokens.get();

	//Leading whitespace has been cleared. Let's make sure we have a shadertype symbol.
	if (!checkForSymbol(peekToken, "shadertype"))
		throw unexpected_token(peekToken);

	consumeWhitespace(tokens);
	peekToken = tokens.get();

	if (checkForSymbol(peekToken, "vertex"))
		mShaderType = VERTEX;
	else if (checkForSymbol(peekToken, "fragment"))
		mShaderType = FRAGMENT;
	else if (checkForSymbol(peekToken, "geometry"))
		mShaderType = GEOMETRY;
	else
		throw unexpected_token(peekToken);

	//normally, we would emit the tokens we just consumed, but there is no equivelent of shadertype. we'll emit our GLSL version instead.
	emit("#version " + std::to_string(mGLVersion));

	//TEST OUTPUT: Just loop through the rest of the file and emit every token that we find to make sure it's working correctly.
	while (!tokens.eof())
	{
		peekToken = tokens.get();

		switch (peekToken->getType())
		{
			case IToken::SYMBOL: 
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
		}

		//Peek token can be the following at the root level:
		//#
		//in
		//uniform
		//out
		//const
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
	}
}
