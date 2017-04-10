#include "ShaderConverter.h"
#include "FileTokenizer.h"
#include "TokenParser.h"
#include "TokenStream.h"
#include <algorithm>
#include <GL/glew.h>
#include <sstream>

int ShaderConverter::kGLVersion = -1;


void ShaderConverter::initGLVersion(int version)
{
	if (version > 0)
		kGLVersion = version;
	else
	{
		std::stringstream iss(reinterpret_cast<const char*>(glGetString(GL_VERSION)));

		std::string portion1, portion2, portion3;
		getline(iss, portion1, '.');
		getline(iss, portion2, '.');
		getline(iss, portion3, ' ');

		kGLVersion = stoi(portion1) * 100 + stoi(portion2) * 10 + stoi(portion3);
	}
}

egpFileInfo ShaderConverter::load(const std::string& filename)
{
	egpFileInfo result{ nullptr, 0 };

	std::string filetype = filename.substr(filename.size() - 4);
	transform(filetype.begin(), filetype.end(), filetype.begin(), tolower);

	if (filetype == "glsl")
		return egpLoadFileContents(filename.c_str());
	
	if (filetype == "jksl")
	{
		std::string convertedText;

		FileTokenizer tokenizer;
		TokenParser	parser;

		tokenizer.tokenize(filename);

		TokenStream stream(tokenizer.getTokens());

		parser.parseTokens(stream, kGLVersion);
		convertedText = parser.getValue();

		result.length = convertedText.size() + 1;
		result.contents = static_cast<const char *>(malloc(sizeof(char) * result.length));
		strcpy(const_cast<char*>(result.contents), convertedText.c_str());

		parser.saveOutputFile(filename + "_PARSED.glsl");
	}


	return result;
}
