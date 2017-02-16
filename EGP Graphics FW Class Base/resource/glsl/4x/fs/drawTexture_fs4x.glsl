/*
	Draw Texture
	By Dan Buckstein
	Fragment shader that displays a sample from a texture.
	
	Modified by: ______________________________________________________________
*/

// version
#version 410


// ****
// varyings

in v2f {
	vec2 texcoord;
} passData;

// ****
// uniforms: 
// in GLSL 4.3+ you can hard-set texture slots: 
//		layout (binding = <texture index>) uniform <sampler type> <name>;
// ...otherwise they are declared just like other uniforms: 
//		uniform <sampler type> <name>;

uniform sampler2D tex_dm;

// ****
// target

layout (location = 0) out vec4 fragColor;

// shader function
void main()
{
	// ****
	// output: this example: sample texture, copy to target
	fragColor = texture(tex_dm, passData.texcoord);
	
	//float mag = (fragColor.r + fragColor.g + fragColor.b) / 3;
	//mag *= mag;
	//fragColor.r = mag;
	//fragColor.g = mag;
	//fragColor.b = mag;
}