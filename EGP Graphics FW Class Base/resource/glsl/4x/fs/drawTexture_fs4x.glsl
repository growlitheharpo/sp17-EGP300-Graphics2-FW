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


// ****
// uniforms: 
// in GLSL 4.3+ you can hard-set texture slots: 
//		layout (binding = <texture index>) uniform <sampler type> <name>;
// ...otherwise they are declared just like other uniforms: 
//		uniform <sampler type> <name>;


// ****
// target


// shader function
void main()
{
	// ****
	// output: this example: sample texture, copy to target
}