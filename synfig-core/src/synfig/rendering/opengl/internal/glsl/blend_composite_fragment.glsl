#version 330 core

const float zero = 0;
const float one = 1;
const float epsilon = 1e-6;  

uniform float amount;
uniform sampler2D sampler_dest;
uniform sampler2D sampler_src;
layout(location = 0) out vec4 out_color;

void main()
{
	ivec2 coord = ivec2(floor(gl_FragCoord)); 
	vec4 dest = texelFetch(sampler_dest, coord, 0);
	vec4 src = texelFetch(sampler_src, coord, 0);

	src.a *= amount;
	src.rgb *= src.a;
	dest.rgb *= dest.a;
	dest = src + dest*(one - src.a);
	
	if (dest.a > epsilon) dest.rgb /= dest.a; else dest = vec4(zero); 

	out_color = dest;
}