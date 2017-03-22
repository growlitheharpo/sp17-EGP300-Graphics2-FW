/*
	Deferred Lighting Composite
	By Dan Buckstein
	Fragment shader that composites precomputed lighting with surface color.
	
	Modified by: ______________________________________________________________
*/

// version
#version 410


// ****
// varyings
in vec2 passTexcoord;


// ****
// uniforms
uniform sampler2D img_depth_sample;
uniform sampler2D img1;
uniform sampler2D img2;
uniform sampler2D img3;
uniform sampler2D img4;


// ****
// target
layout (location = 0) out vec4 fragColor;

float lerp(in float v0, in float v1, in float t)
{
	return v0 + (v1 - v0) * t;
}

vec4 lerpVec(in vec4 v0, in vec4 v1, in float t)
{
	return v0 + (v1 - v0) * t;
}

// shader function
void main()
{
	// ****
	// get object texture coordinates to sample surface textures, 
	//	and also sample results from light pre-pass
	/*vec4 texcoord = texture(img_texcoord, passTexcoord);

	vec4 diffuseSample = texture(tex_dm, texcoord.xy);
	vec4 specularSample = texture(tex_sm, texcoord.xy);

	vec4 diffuseShading = texture(img_light_diffuse, passTexcoord);
	vec4 specularShading = texture(img_light_specular, passTexcoord);

	fragColor = diffuseSample*diffuseShading + specularSample*specularShading;*/

	float depthVal = texture(img_depth_sample, passTexcoord).x;

	depthVal *= depthVal;
	depthVal *= depthVal;
	depthVal *= depthVal;
	//fragColor = vec4(depthVal, depthVal, depthVal, 1);

	fragColor = lerpVec(vec4(0, 0, 0, 1), vec4(1, 1, 1, 1), depthVal);

	vec4 blur0Sample = texture(img1, passTexcoord);
	vec4 blur2Sample = texture(img2, passTexcoord);
	vec4 blur4Sample = texture(img3, passTexcoord);
	vec4 blur8Sample = texture(img4, passTexcoord);

	//fragColor = lerpVec(noblurSample, allblurSample, depthVal);
	if (depthVal < 0.25)
		fragColor = lerpVec(blur0Sample, blur2Sample, depthVal / 0.25);
	else if (depthVal < 0.5)
		fragColor = lerpVec(blur2Sample, blur4Sample, (depthVal - 0.25) / 0.25);
	else if (depthVal < 0.75)
		fragColor = lerpVec(blur4Sample, blur8Sample, (depthVal - 0.5) / 0.25);
	else
		fragColor = blur8Sample;
}