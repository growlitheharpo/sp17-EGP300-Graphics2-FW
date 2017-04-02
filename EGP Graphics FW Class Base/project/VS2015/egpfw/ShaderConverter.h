#pragma once
#include <string>
#include <egpfw/egpfw/utils/egpfwFileUtils.h>

class ShaderConverter
{
	private:
		static int kGLVersion;

	public:
		static void initGLVersion(int version = -1);
		static egpFileInfo load(const std::string& filename);
};
